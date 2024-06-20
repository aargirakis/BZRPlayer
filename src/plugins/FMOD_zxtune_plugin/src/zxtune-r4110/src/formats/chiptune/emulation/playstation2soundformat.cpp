/**
* 
* @file
*
* @brief  PSF2 VFS parser implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "playstation2soundformat.h"
//common includes
#include <byteorder.h>
#include <make_ptr.h>
//library includes
#include <binary/compression/zlib_container.h>
#include <binary/container_factories.h>
#include <binary/input_stream.h>
#include <binary/format_factories.h>
#include <debug/log.h>
//std includes
#include <map>
//text includes
#include <formats/text/chiptune.h>

namespace Formats
{
namespace Chiptune
{
  namespace Playstation2SoundFormat
  {
    const Debug::Stream Dbg("Formats::Chiptune::PSF2");
    
    struct DirectoryEntry
    {
      static const std::size_t RAW_SIZE = 48;
    
      String Name;
      std::size_t Offset;
      std::size_t Size;
      std::size_t BlockSize;
      
      explicit DirectoryEntry(Binary::InputStream& stream)
      {
        const auto nameBegin = stream.ReadRawData(36);
        const auto nameEnd = std::find(nameBegin, nameBegin + 36, 0);
        Name.assign(nameBegin, nameEnd);
        Offset = fromLE(stream.ReadField<uint32_t>());
        Size = fromLE(stream.ReadField<uint32_t>());
        BlockSize = fromLE(stream.ReadField<uint32_t>());
      }
      
      bool IsDir() const
      {
        return Offset != 0 && Size == 0 && BlockSize == 0;
      }
    };

    using FileBlocks = std::map<std::size_t, Binary::Container::Ptr>;
    
    class ScatteredContainer : public Binary::Container
    {
    public:
      ScatteredContainer(FileBlocks blocks, std::size_t totalSize)
        : TotalSize(totalSize)
        , Blocks(std::move(blocks))
      {
        Require(Blocks.size() > 1);
      }
      
      const void* Start() const override
      {
        Flatten();
        return Flattened->data();
      }

      std::size_t Size() const override
      {
        return TotalSize;
      }
      
      Ptr GetSubcontainer(std::size_t offset, std::size_t size) const override
      {
        if (!Flattened)
        {
          const auto it = Blocks.lower_bound(offset);
          if (it == Blocks.end() || it->first + it->second->Size() <= offset)
          {
            return Ptr();
          }
          const auto& part = it->second;
          const auto partOffset = offset - it->first;
          const auto partSize = part->Size();
          if (!partOffset && partSize == size)
          {
            return part;
          }
          else if (partOffset + size <= partSize)
          {
            return part->GetSubcontainer(partOffset, size);
          }
          Flatten();
        }
        return Binary::CreateContainer(Flattened, offset, size);
      }
      
      static Ptr Create(FileBlocks blocks, std::size_t totalSize)
      {
        Require(!blocks.empty());
        if (blocks.size() == 1)
        {
          return blocks.begin()->second;
        }
        else
        {
          return MakePtr<ScatteredContainer>(std::move(blocks), totalSize);
        }
      }
    private:
      void Flatten() const
      {
        if (!Flattened)
        {
          Flattened.reset(new Dump(TotalSize));
          uint8_t* dst = Flattened->data();
          for (const auto& blk : Blocks)
          {
            const auto size = blk.second->Size();
            std::memcpy(dst, blk.second->Start(), size);
            dst += size;
          }
          Blocks.clear();
        }
      }
    private:
      const std::size_t TotalSize;
      mutable FileBlocks Blocks;
      mutable std::shared_ptr<Dump> Flattened;
    };

    class Format
    {
    public:
      explicit Format(const Binary::Container& data)
        : Stream(data)
      {
      }
      
      void Parse(Builder& target)
      {
        ParseDir(0, "/", target);
      }
    private:
      void ParseDir(uint_t depth, String path, Builder& target)
      {
        Require(depth < 10);
        const uint_t entries = fromLE(Stream.ReadField<uint32_t>());
        Dbg("%2% entries at '%1%'", path, entries);
        for (uint_t idx = 0; idx < entries; ++idx)
        {
          const auto entryPos = Stream.GetPosition();
          const DirectoryEntry entry(Stream);
          Dbg("%1% (offset=%2% size=%3% block=%4%)", entry.Name, entry.Offset, entry.Size, entry.BlockSize);
          auto entryPath = path + entry.Name;
          Stream.Seek(entry.Offset);
          if (entry.IsDir())
          {
            Require(entryPos < entry.Offset);
            ParseDir(depth + 1, entryPath + '/', target);
          }
          else if (0 == entry.Size)
          {
            //empty file may have zero offset
            target.OnFile(std::move(entryPath), Binary::Container::Ptr());
          }
          else
          {
            Require(entryPos < entry.Offset);
            auto blocks = ParseFileBlocks(entry.Size, entry.BlockSize);
            target.OnFile(std::move(entryPath), ScatteredContainer::Create(std::move(blocks), entry.Size));
          }
          Stream.Seek(entryPos + DirectoryEntry::RAW_SIZE);
        }
      }
      
      FileBlocks ParseFileBlocks(std::size_t fileSize, std::size_t blockSize)
      {
        const auto blocksCount = (fileSize + blockSize - 1) / blockSize;
        std::vector<std::size_t> blocksSizes(blocksCount);
        for (auto& size : blocksSizes)
        {
          size = fromLE(Stream.ReadField<uint32_t>());
        }
        FileBlocks result;
        std::size_t offset = 0;
        for (const auto size : blocksSizes)
        {
          const auto unpackedSize = std::min(blockSize, fileSize - offset);
          Dbg(" @%1%: %2% -> %3%", offset, size, unpackedSize);
          auto packed = Stream.ReadData(size);
          auto unpacked = Binary::Compression::Zlib::CreateDeferredDecompressContainer(std::move(packed), unpackedSize);
          result.emplace(offset, std::move(unpacked));
          offset += unpackedSize;
        }
        Require(offset == fileSize);
        return result;
      }
    private:
      Binary::InputStream Stream;
    };
    
    void ParseVFS(const Binary::Container& data, Builder& target)
    {
      Format(data).Parse(target);
    }

    const std::string FORMAT(
      "'P'S'F"
      "02"
    );
    
    class Decoder : public Formats::Chiptune::Decoder
    {
    public:
      Decoder()
        : Format(Binary::CreateMatchOnlyFormat(FORMAT))
      {
      }

      String GetDescription() const override
      {
        return Text::PLAYSTATION2SOUNDFORMAT_DECODER_DESCRIPTION;
      }

      Binary::Format::Ptr GetFormat() const override
      {
        return Format;
      }

      bool Check(const Binary::Container& rawData) const override
      {
        return Format->Match(rawData);
      }

      Formats::Chiptune::Container::Ptr Decode(const Binary::Container& /*rawData*/) const override
      {
        return Formats::Chiptune::Container::Ptr();//TODO
      }
    private:
      const Binary::Format::Ptr Format;
    };
  }

  Decoder::Ptr CreatePSF2Decoder()
  {
    return MakePtr<Playstation2SoundFormat::Decoder>();
  }
}
}
