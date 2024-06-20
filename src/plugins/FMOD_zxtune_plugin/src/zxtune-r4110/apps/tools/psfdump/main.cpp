/**
*
* @file
*
* @brief  Portable Sound Format dumper utilities
*
* @author vitamin.caig@gmail.com
*
**/

//common includes
#include <make_ptr.h>
//library includes
#include <binary/data_builder.h>
#include <binary/compression/zlib_container.h>
#include <formats/chiptune/emulation/gameboyadvancesoundformat.h>
#include <formats/chiptune/emulation/nintendodssoundformat.h>
#include <formats/chiptune/emulation/playstationsoundformat.h>
#include <formats/chiptune/emulation/playstation2soundformat.h>
#include <formats/chiptune/emulation/portablesoundformat.h>
#include <formats/chiptune/emulation/ultra64soundformat.h>
#include <strings/format.h>
#include <time/duration.h>
//std includes
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>

namespace
{
  Binary::Container::Ptr OpenFile(const std::string& name)
  {
    std::ifstream stream(name.c_str(), std::ios::binary);
    if (!stream)
    {
      throw std::runtime_error("Failed to open " + name);
    }
    stream.seekg(0, std::ios_base::end);
    const std::size_t size = stream.tellg();
    stream.seekg(0);
    Binary::DataBuilder data(size);
    stream.read(static_cast<char*>(data.Allocate(size)), size);
    return data.CaptureResult();
  }

  void Write(uint_t level, const char* msg)
  {
    if (level)
    {
      std::cout << std::setw(level) << ' ';
    }
    std::cout << msg << std::endl;
  }
  
  template<class... P>
  void Write(uint_t level, const char* msg, P&&... params)
  {
    Write(level, Strings::Format(msg, params...).c_str());
  }
  
  char ToHex(uint_t nib)
  {
    return nib > 9 ? 'a' + (nib - 10) : '0' + nib;
  }
  
  char ToSym(uint_t val)
  {
    return val >= ' ' && val < 0x7f ? val : '.';
  }
  
  void DumpHex(uint_t level, const void* data, std::size_t size)
  {
    const std::size_t LINE_SIZE = 16;
    const auto DUMP_SIZE = std::min<std::size_t>(size, 256);
    for (std::size_t offset = 0; offset < DUMP_SIZE; )
    {
      const auto in = static_cast<const uint8_t*>(data) + offset;
      const auto toPrint = std::min(size - offset, LINE_SIZE);
      std::string msg(5 + 2 + LINE_SIZE * 3 + 2 + LINE_SIZE, ' ');
      msg[0] = ToHex((offset >> 12) & 15);
      msg[1] = ToHex((offset >>  8) & 15);
      msg[2] = ToHex((offset >>  4) & 15);
      msg[3] = ToHex((offset >>  0) & 15);
      msg[5] = '|';
      msg[7 + LINE_SIZE * 3] = '|';
      for (std::size_t idx = 0; idx < toPrint; ++idx)
      {
        msg[7 + idx * 3 + 0] = ToHex(in[idx] >> 4);
        msg[7 + idx * 3 + 1] = ToHex(in[idx] & 15);
        msg[7 + LINE_SIZE * 3 + 2 + idx] = ToSym(in[idx]);
      }
      Write(level, msg.c_str());
      offset += toPrint;
    }
    if (DUMP_SIZE != size)
    {
      Write(level, "<truncated>");
    }
  }

  class SectionDumper
  {
  public:
    using Ptr = std::unique_ptr<const SectionDumper>;
    
    virtual void DumpReserved(const Binary::Container& blob) const
    {
      DumpHex(2, blob.Start(), blob.Size());
    }

    virtual void DumpProgram(const Binary::Container& blob) const
    {
      DumpHex(2, blob.Start(), blob.Size());
    }
  };
  
  class PSF1Dumper : public SectionDumper
  {
  public:
    void DumpProgram(const Binary::Container& blob) const override
    {
      try
      {
        Write(2, "PS-X EXE image:");
        PSXExeDumper delegate;
        Formats::Chiptune::PlaystationSoundFormat::ParsePSXExe(blob, delegate);
      }
      catch (const std::exception&)
      {
        Write(3, "Corrupted");
      }
    }
    
  private:
    class PSXExeDumper : public Formats::Chiptune::PlaystationSoundFormat::Builder
    {
    public:
      void SetRegisters(uint32_t pc, uint32_t gp) override
      {
        Write(3, "Registers: PC=0x%1$08x GP=0x%2$08x", pc, gp);
      }
      
      void SetStackRegion(uint32_t head, uint32_t size) override
      {
        Write(3, "Stack: %1% (0x%1$08x) bytes at 0x%2$08x", size, head);
      }
      
      void SetRegion(String region, uint_t fps) override
      {
        Write(3, "Region: %1% (%2% fps)", region, fps);
      }

      void SetTextSection(uint32_t address, const Binary::Data& content)
      {
        Write(3, "Text section: %1% (0x%1$08x) bytes at 0x%2$08x", content.Size(), address);
        DumpHex(4, content.Start(), content.Size());
      };
    };
  };
  
  class PSF2Dumper : public SectionDumper
  {
  public:
    void DumpReserved(const Binary::Container& blob) const override
    {
      try
      {
        Write(2, "PS2 VFS:");
        PSF2VFSDumper delegate;
        Formats::Chiptune::Playstation2SoundFormat::ParseVFS(blob, delegate);
        if (const auto total = delegate.GetTotalSize())
        {
          Write(2, "Total: %1% bytes (%2%%% ratio)", total, 100.0f * blob.Size() / total);
        }
      }
      catch (const std::exception&)
      {
        Write(3, "Corrupted");
      }
    }
  private:
    class PSF2VFSDumper : public Formats::Chiptune::Playstation2SoundFormat::Builder
    {
    public:
      void OnFile(String path, Binary::Container::Ptr content) override
      {
        const auto fileSize = content ? content->Size() : std::size_t(0);
        Write(3, "%1%: %2% bytes", path, fileSize);
        if (fileSize)
        {
          DumpHex(4, content->Start(), fileSize);
        }
        TotalSize += fileSize;
      }
      
      std::size_t GetTotalSize() const
      {
        return TotalSize;
      }
    private:
      std::size_t TotalSize = 0;
    };
  };
  
  class USFDumper : public SectionDumper
  {
  public:
    void DumpReserved(const Binary::Container& blob) const override
    {
      try
      {
        Write(2, "Ultra64 state:");
        USFStateDumper delegate;
        Formats::Chiptune::Ultra64SoundFormat::ParseSection(blob, delegate);
        delegate.Dump();
      }
      catch (const std::exception&)
      {
        Write(3, "Corrupted");
      }
    }
  private:
    class USFStateDumper : public Formats::Chiptune::Ultra64SoundFormat::Builder
    {
    public:
      void SetRom(uint32_t offset, const Binary::Data& content) override
      {
        Rom.Account(offset, content);
      }
      
      void SetSaveState(uint32_t offset, const Binary::Data& content) override
      {
        SaveState.Account(offset, content);
      }
      
      void Dump() const
      {
        Rom.Dump("ROM");
        SaveState.Dump("SaveState");
      }
    private:
      struct Area
      {
        uint_t ChunksCount = 0;
        std::size_t ChunksSize = 0;
        uint_t End = 0;
        
        void Account(uint32_t offset, const Binary::Data& content)
        {
          ++ChunksCount;
          const auto size = content.Size();
          ChunksSize += size;
          End = std::max<uint_t>(End, offset + size);
        }
        
        void Dump(const char* tag) const
        {
          if (ChunksCount)
          {
            Write(3, "%1%: %2% chunks with %3% bytes total (%4%%% covered)", tag, ChunksCount, ChunksSize, 100.0f * ChunksSize / End);
          }
        }
      };
      Area Rom;
      Area SaveState;
    };
  };
  
  class GSFDumper : public SectionDumper
  {
  public:
    void DumpProgram(const Binary::Container& blob) const override
    {
      try
      {
        Write(2, "GBA ROM:");
        GBARomDumper delegate;
        Formats::Chiptune::GameBoyAdvanceSoundFormat::ParseRom(blob, delegate);
      }
      catch (const std::exception&)
      {
        Write(3, "Corrupted");
      }
    }
  private:
    class GBARomDumper : public Formats::Chiptune::GameBoyAdvanceSoundFormat::Builder
    {
    public:
      void SetEntryPoint(uint32_t addr) override
      {
        Write(3, "EntryPoint: 0x%1$08x", addr);
      }
      
      void SetRom(uint32_t addr, const Binary::Data& content) override
      {
        Write(3, "ROM: %1% (0x%1$08x) bytes at 0x%2$08x", content.Size(), addr);
        DumpHex(4, content.Start(), content.Size());
      }
    };
  };

  class TwoSFDumper : public SectionDumper
  {
  public:
    void DumpProgram(const Binary::Container& blob) const override
    {
      try
      {
        Write(2, "DS ROM:");
        ChunkDumper delegate;
        Formats::Chiptune::NintendoDSSoundFormat::ParseRom(blob, delegate);
      }
      catch (const std::exception&)
      {
        Write(3, "Corrupted");
      }
    }

    void DumpReserved(const Binary::Container& blob) const override
    {
      try
      {
        Write(2, "DS Savestate:");
        ChunkDumper delegate;
        Formats::Chiptune::NintendoDSSoundFormat::ParseState(blob, delegate);
      }
      catch (const std::exception&)
      {
        Write(3, "Corrupted");
      }
    }
  private:
    class ChunkDumper : public Formats::Chiptune::NintendoDSSoundFormat::Builder
    {
    public:
      void SetChunk(uint32_t offset, const Binary::Data& content) override
      {
        Write(3, "%1% (0x%1$08x) bytes at 0x%2$08x", content.Size(), offset);
        DumpHex(4, content.Start(), content.Size());
      }
    };
  };
  
  class PSFDumper : public Formats::Chiptune::PortableSoundFormat::Builder
  {
  public:
    void SetVersion(uint_t ver) override
    {
      Write(1, "Type: %1% (id=0x%2$02x)", DecodeVersion(ver), ver);
      Dumper = CreateDumper(ver);
    }

    void SetReservedSection(Binary::Container::Ptr blob) override
    {
      Write(1, "Reserved area: %1% bytes", blob->Size());
      Dumper->DumpReserved(*blob);
    }
    
    void SetPackedProgramSection(Binary::Container::Ptr blob) override
    {
      const auto packedSize = blob->Size();
      const auto unpacked = Binary::Compression::Zlib::CreateDeferredDecompressContainer(std::move(blob));
      Write(1, "Program area: %1% bytes (%2% packed, %3%%% ratio)", unpacked->Size(), packedSize, 100.0f * packedSize / unpacked->Size());
      Dumper->DumpProgram(*unpacked);
    }
    
    void SetTitle(String title) override
    {
      Write(1, "Title: %1%", title);
    }
    
    virtual void SetArtist(String artist) override
    {
      Write(1, "Artist: %1%", artist);
    }
    
    void SetGame(String game) override
    {
      Write(1, "Game: %1%", game);
    }
    
    void SetYear(String date) override
    {
      Write(1, "Year: %1%", date);
    }
    
    void SetGenre(String genre) override
    {
      Write(1, "Genre: %1%", genre);
    }
    
    void SetComment(String comment) override
    {
      Write(1, "Comment: %1%", comment);
    }
    
    void SetCopyright(String copyright) override
    {
      Write(1, "Copyright: %1%", copyright);
    }
    
    void SetDumper(String dumper) override
    {
      Write(1, "Dumper: %1%", dumper);
    }
    
    void SetLength(Time::Milliseconds duration) override
    {
      Write(1, "Length: %1%", Time::MillisecondsDuration(duration.Get(), Time::Milliseconds(1)).ToString());
    }
    
    void SetFade(Time::Milliseconds fade) override
    {
      Write(1, "Fade: %1%", Time::MillisecondsDuration(fade.Get(), Time::Milliseconds(1)).ToString());
    }
    
    void SetVolume(float vol) override
    {
      Write(1, "Volume: %1%", vol);
    }
    
    void SetTag(String name, String value) override
    {
      Write(1, "%1%: %2%", name, value);
    }

    void SetLibrary(uint_t num, String filename) override
    {
      Write(1, "Library #%1%: %2%", num, filename);
    }
    
  private:
    static std::string DecodeVersion(uint_t ver)
    {
      switch (ver)
      {
      case 0x01:
        return "Playstation (PSF1)";
      case 0x02:
        return "Playstation 2 (PSF2)";
      case 0x11:
        return "Saturn (SSF)";
      case 0x12:
        return "Dreamcast (DSF)";
      case 0x13:
        return "Sega Genesis";
      case 0x21:
        return "Nintendo 64 (USF)";
      case 0x22:
        return "GameBoy Advance (GSF)";
      case 0x23: 
        return "Super NES (SNSF)";
      case 0x24:
        return "Nintendo DS (2SF)";
      case 0x25:
        return "Nitro Composer (NCSF)";
      case 0x41:
        return "Capcom QSound (QSF)";
      default:
        return "Unknown";
      }
    }
    
    static SectionDumper::Ptr CreateDumper(uint_t ver)
    {
      switch (ver)
      {
      case Formats::Chiptune::PlaystationSoundFormat::VERSION_ID:
        return MakePtr<PSF1Dumper>();
      case Formats::Chiptune::Playstation2SoundFormat::VERSION_ID:
        return MakePtr<PSF2Dumper>();
      case Formats::Chiptune::Ultra64SoundFormat::VERSION_ID:
        return MakePtr<USFDumper>();
      case Formats::Chiptune::GameBoyAdvanceSoundFormat::VERSION_ID:
        return MakePtr<GSFDumper>();
      case Formats::Chiptune::NintendoDSSoundFormat::VERSION_ID:
        return MakePtr<TwoSFDumper>();
      default:
        return MakePtr<SectionDumper>();
      }
    }

  private:
    SectionDumper::Ptr Dumper;
  };
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      return 0;
    }
    for (int arg = 1; arg < argc; ++arg)
    {
      const std::string filename(argv[arg]);
      Write(0, "%1%:", filename);
      const auto data = OpenFile(filename);
      PSFDumper builder;
      if (const auto container = Formats::Chiptune::PortableSoundFormat::Parse(*data, builder))
      {
        Write(1, "Size: %1% bytes (file %2% bytes, %3%%% used)", container->Size(), data->Size(), 100.0f * container->Size() / data->Size());
      }
      else
      {
        Write(1, "Invalid format");
      }
    }
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
    return 1;
  }
}
