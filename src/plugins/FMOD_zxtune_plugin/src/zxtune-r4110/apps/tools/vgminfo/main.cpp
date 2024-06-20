#include <byteorder.h>
#include <contract.h>
#include <types.h>
#include <pointers.h>
#include <strings/format.h>
#include <fstream>
#include <iostream>
#include <set>
#include <map>

namespace
{
  class Stream
  {
  public:
    explicit Stream(const std::string& filename)
      : Delegate(filename.c_str(), std::ios::binary)
    {
      Check();
    }
    
    template<class T>
    T ReadData()
    {
      T res = 0;
      Delegate.read(safe_ptr_cast<char*>(&res), sizeof(res));
      Check();
      return res;
    }
    
    uint32_t ReadDword()
    {
      return ReadData<uint32_t>();
    }
    
    uint16_t ReadWord()
    {
      return ReadData<uint16_t>();
    }
    
    uint8_t ReadByte()
    {
      return ReadData<uint8_t>();
    }
    
    void Skip(std::ptrdiff_t size)
    {
      Delegate.seekg(size, std::ios_base::cur);
      Check();
    }
    
    void Seek(std::size_t pos)
    {
      Delegate.seekg(pos);
      Check();
    }
    
    uint_t ReadVersion()
    {
      uint8_t vers[4] = {0};
      Delegate.read(safe_ptr_cast<char*>(vers), sizeof(vers));
      Check();
      return (vers[0] & 15) + 10 *(vers[0] >> 4) + 100 * (vers[1] & 15) + 1000 * (vers[1] >> 4);
    }
    
    std::size_t GetPos()
    {
      return Delegate.tellg();
    }
  private:
    void Check()
    {
      if (!Delegate)
      {
        throw std::runtime_error("Read error");
      }
    }
  private:
    std::ifstream Delegate;
  };
  
  class Header
  {
  public:
    Header()
      : Version()
      , Framerate()
    {
    }
    
    void Parse(Stream& stream)
    {
      const uint32_t signature = stream.ReadDword();
      if (signature != fromLE<uint32_t>(0x206d6756))
      {
        throw std::runtime_error("Invalid signature");
      }
      const uint32_t size = stream.ReadDword();
      Version = stream.ReadVersion();
      ParseSN76489(stream);
      ParseDevice("YM2413", stream);
      const uint32_t gd3 = stream.ReadDword();
      const uint32_t samples = stream.ReadDword();
      const uint32_t loopOffset = stream.ReadDword();
      const uint32_t loopSamples = stream.ReadDword();
      if (Version >= 101)
      {
        Framerate = stream.ReadDword();
      }
      if (Version >= 110)
      {
        stream.ReadDword();//flags for sn76489
        ParseDevice("YM2612", stream);
        ParseDevice("YM2151", stream);
      }
      if (Version >= 150)
      {
        stream.ReadDword();//vgm offset
        ParseSimpleDevice("SegaPCM", stream);
        stream.ReadDword();//segapcm flags
      }
      if (Version >= 151)
      {
        Require(0x40 == stream.GetPos());
        ParseSimpleDevice("RF5C68", stream);
        ParseDevice("YM2203", stream);
        ParseDevice("YM2608", stream);
        ParseDevice("YM2610", stream);
        ParseDevice("YM3812", stream);
        ParseDevice("YM3526", stream);
        ParseDevice("Y8950", stream);
        ParseDevice("YMF262", stream);
        ParseDevice("YMF278b", stream);
        ParseDevice("YMF271", stream);
        ParseDevice("YMZ280B", stream);
        ParseSimpleDevice("RF5C164", stream);
        ParseSimpleDevice("PWM", stream);
        ParseAY8910(stream);
        stream.ReadDword();//other flags
      }
      if (Version >= 161)
      {
        Require(0x80 == stream.GetPos());
        ParseDevice("LR3509/PAPU", stream);
        ParseDevice("2A03", stream);
        ParseDevice("MultiPCM", stream);
        ParseDevice("uPD7759", stream);
        ParseDevice("OKIM6258", stream);
        stream.ReadDword();//flags
        ParseDevice("OKIM6295", stream);
        ParseDevice("K051649", stream);
        ParseDevice("K054539", stream);
        ParseDevice("HuC6280", stream);
        ParseDevice("C140", stream);
        ParseDevice("K053260", stream);
        ParseDevice("CO12294/Pokey", stream);
        ParseSimpleDevice("QSound", stream);
      }
      if (Version >= 170)
      {
        Require(0xb8 == stream.GetPos());
        ParseDevice("SCSP", stream);
        stream.ReadDword();//extra header offset
      }
      if (Version >= 171)
      {
        Require(0xc0 == stream.GetPos());
        ParseSimpleDevice("WonderSwan", stream);
        ParseSimpleDevice("VSU", stream);
        ParseSimpleDevice("SA1099", stream);
        ParseSimpleDevice("ES5503", stream);
        ParseES5505(stream);
        stream.ReadDword();//flags
        ParseSimpleDevice("X1-010", stream);
        ParseSimpleDevice("C352", stream);
        ParseSimpleDevice("GA20", stream);
      }
      if (gd3)
      {
        stream.Seek(gd3 + 0x14);
        ParseGD3Tags(stream);
      }
    }
    
    void Dump() const
    {
      std::cout << 
        "Version: " << Version / 100 << '.' << Version % 100 << std::endl <<
        "Devices: " << std::endl;
      for (const auto& dev : Devices)
      {
        std::cout << "  " << dev.first << " (" << dev.second << "hz)" << std::endl;
      }
      if (Framerate)
      {
        std::cout << "Framerate: "<< Framerate << "hz" << std::endl;
      }
      if (!Tags.empty())
      {
        std::cout << "Tags:" << std::endl;
        for (const auto& tag : Tags)
        {
          std::cout << "  " << tag.first << ": " << tag.second << std::endl;
        }
      }
    }
  private:
    void ParseSN76489(Stream& stream)
    {
      if (const uint32_t data = stream.ReadDword())
      {
        const uint32_t flg = data & 0xc0000000;
        if (flg == 0xc0000000)
        {
          AddDevice("T6Ww28", data ^ flg);
        }
        else
        {
          AddDevice("SN76489", data);
        }
      }
    }
    
    void ParseAY8910(Stream& stream)
    {
      const char* const AY_CHIPS[] = {"AY8910", "AY8912", "AY8913", "AY8930"};
      const char* const YM_CHIPS[] = {"YM2140", "YM3439", "YMZ284", "YMZ294"};
      const uint32_t clk = stream.ReadDword();
      const uint32_t type = stream.ReadDword();
      if (clk)
      {
        const bool isYM = type & 0x10;
        const uint_t subtype = type & 0x0f;
        if (subtype < 4)
        {
          AddDevice((isYM ? AY_CHIPS : YM_CHIPS)[subtype], clk);
        }
        else
        {
          AddDevice("AY8910", clk);
        }
      }
    }
    
    void ParseES5505(Stream& stream)
    {
      if (const uint32_t data = stream.ReadDword())
      {
        const bool isES5506 = data & 0x80000000;
        AddDevice(isES5506 ? "ES5506" : "ES5505", data & 0x3fffffff);
      }
    }
    
    void ParseDevice(const String& name, Stream& stream)
    {
      if (const uint32_t data = stream.ReadDword())
      {
        AddDevice(name, data);
      }
    }
    
    void ParseSimpleDevice(const String& name, Stream& stream)
    {
      if (const uint32_t data = stream.ReadDword())
      {
        Devices[name] = data;
      }
    }
    
    void AddDevice(const String& name, uint32_t data)
    {
      const bool dual = data & 0x40000000;
      const bool pin7 = data & 0x80000000;
      const uint32_t clock = data & 0x3fffffff;
      String fullName = name;
      if (dual)
      {
        fullName += "x2";
      }
      if (pin7)
      {
        fullName += " (pin7)";
      }
      Devices[fullName] = clock;
    }
    
    void ParseGD3Tags(Stream& stream)
    {
      const uint32_t tag = stream.ReadDword();
      if (tag != fromLE<uint32_t>(0x20336447))
      {
        return;
      }
      stream.ReadDword();//ver
      stream.ReadDword();//size
      ReadTag("Title(eng)", stream);
      ReadTag("Title(jap)", stream);
      ReadTag("Game(eng)", stream);
      ReadTag("Game(jap)", stream);
      ReadTag("System(eng)", stream);
      ReadTag("System(jap)", stream);
      ReadTag("Author(eng)", stream);
      ReadTag("Author(jap)", stream);
      ReadTag("Released", stream);
      ReadTag("RippedBy", stream);
      ReadTag("Notes", stream);
    }
    
    void ReadTag(const String& name, Stream& stream)
    {
      String value;
      while (const uint16_t utf = stream.ReadWord())
      {
        if (utf <= 0x7f)
        {
          value += static_cast<Char>(utf);
        }
        else if (utf <= 0x7ff)
        {
          value += static_cast<Char>(0xc0 | ((utf & 0x3c0) >> 6));
          value += static_cast<Char>(0x80 | (utf & 0x3f));
        }
        else
        {
          value += static_cast<Char>(0xe0 | ((utf & 0xf000) >> 12));
          value += static_cast<Char>(0x80 | ((utf & 0x0fc0) >> 6));
          value += static_cast<Char>(0x80 | ((utf & 0x003f)));
        }
      }
      if (!value.empty())
      {
        Tags[name] = value;
      }
    }
  private:
    uint_t Version;
    uint_t Framerate;
    std::map<String, String> Tags;
    std::map<String, uint_t> Devices;
  };

  void DumpHeader(Stream& stream)
  {
    Header header;
    stream.Seek(0);
    header.Parse(stream);
    header.Dump();
  }
  
  class CommandsSet
  {
  public:
    bool ParseCommand(Stream& stream)
    {
      const auto offset = stream.GetPos();
      const uint8_t code = stream.ReadByte();
      if (code == 0x66)
      {
        return false;
      }
      if (ParseFixedCommand(code, stream)
          || ParseDataBlock(code, stream)
          || ParseRamWrite(code, stream)
          || ParseDacControl(code, stream)
          || ParseBuggyCommand(code, stream))
      {
        return true;
      }
      else
      {
        Add(Strings::Format("unknown 0x%02x @ 0x%x", uint_t(code), offset));
      }
    }
    
    void Dump() const
    {
      std::cout << "Used commands:" << std::endl;
      for (const auto& cmd : Commands)
      {
        std::cout << "  " << cmd << std::endl;
      }
    }
  private:
    struct FixedCmd
    {
      const uint8_t CodeMin;
      const uint8_t CodeMax;
      const std::size_t SequentSize;
      const char* const Name;
    };

    bool ParseFixedCommand(uint8_t code, Stream& stream)
    {
      if (code == 0x4f)
      {
        const uint_t mode = stream.ReadByte();
        Add(Strings::Format("gg mixer 0x%02x", mode));
        return true;
      }

      static const FixedCmd FIXED_COMMANDS[] =
      {
        {0x30, 0x30, 1, "dual sn76489"},
        {0x31, 0x3e, 1, "dual8"},
        {0x3f, 0x3f, 1, "dual T6Ww28"},
        {0x50, 0x50, 1, "sn76489"},
        {0x51, 0x51, 2, "ym2413"},
        {0x52, 0x53, 2, "ym2612"},
        {0x54, 0x54, 2, "ym2151"},
        {0x55, 0x55, 2, "ym2203"},
        {0x56, 0x57, 2, "ym2608"},
        {0x58, 0x59, 2, "ym2610"},
        {0x5a, 0x5a, 2, "ym3812"},
        {0x5b, 0x5b, 2, "ym3526"},
        {0x5c, 0x5c, 2, "y8950"},
        {0x5d, 0x5d, 2, "ymz280b"},
        {0x5e, 0x5f, 2, "ymf262"},
        {0x61, 0x61, 2, "wait"},
        {0x62, 0x63, 0, "wait"},
        {0x64, 0x64, 3, "waitOverride"},
        {0x70, 0x7f, 0, "wait"},
        {0x80, 0x8f, 0, "ym2612+wait"},
        {0xa0, 0xa0, 2, "ay8910"},
        {0xa1, 0xaf, 2, "dual16"},
        {0xb0, 0xb0, 2, "rf5c68"},
        {0xb1, 0xb1, 2, "rf5c164"},
        {0xb2, 0xb2, 2, "pwm"},
        {0xb3, 0xb3, 2, "dmg"},
        {0xb4, 0xb4, 2, "2a03"},
        {0xb5, 0xb5, 2, "multipcm"},
        {0xb6, 0xb6, 2, "upd7759"},
        {0xb7, 0xb7, 2, "okim6258"},
        {0xb8, 0xb8, 2, "okim6295"},
        {0xb9, 0xb9, 2, "huc6280"},
        {0xba, 0xba, 2, "k053260"},
        {0xbb, 0xbb, 2, "pokey"},
        {0xbc, 0xbc, 2, "wonderswan"},
        {0xbd, 0xbd, 2, "saa1099"},
        {0xbe, 0xbe, 2, "es5506"},
        {0xbf, 0xbf, 2, "ga20"},
        {0xc0, 0xc0, 3, "sega pcm/ram"},
        {0xc1, 0xc1, 3, "rf5c68/ram"},
        {0xc2, 0xc2, 3, "rf5c164/ram"},
        {0xc3, 0xc3, 3, "multipcm/offset"},
        {0xc4, 0xc4, 3, "qsound"},
        {0xc5, 0xc5, 3, "scsp"},
        {0xc6, 0xc6, 3, "wonderswan"},
        {0xc7, 0xc7, 3, "vsu"},
        {0xc8, 0xc8, 3, "x1-010"},
        {0xc9, 0xcf, 3, "reserved24"},
        {0xd0, 0xd0, 3, "ymf278b"},
        {0xd1, 0xd1, 3, "ymf271"},
        {0xd2, 0xd2, 3, "scc1"},
        {0xd3, 0xd3, 3, "k054539"},
        {0xd4, 0xd4, 3, "c140"},
        {0xd5, 0xd5, 3, "es5503"},
        {0xd6, 0xd6, 3, "es5506"},
        {0xd7, 0xdf, 3, "reserved24"},
        {0xe0, 0xe0, 4, "pcm/offset"},
        {0xe1, 0xe1, 4, "c352"},
        {0xe2, 0xff, 4, "reserved32"},
      };

      for (const auto& cmd : FIXED_COMMANDS)
      {
        Require(cmd.CodeMin <= cmd.CodeMax);
        if (code >= cmd.CodeMin && code <= cmd.CodeMax)
        {
          Add(cmd.Name);
          stream.Skip(cmd.SequentSize);
          return true;
        }
      }
      return false;
    }
    
    bool ParseDataBlock(uint8_t code, Stream& stream)
    {
      if (code != 0x67)
      {
        return false;
      }
      const uint_t compat = stream.ReadByte();
      if (0x66 != compat)
      {
        stream.Skip(-1);
        return false;
      }
      const uint8_t type = stream.ReadByte();
      uint32_t size = stream.ReadDword();
      size &= 0x7fffffff;
      
      if (type <= 0x3f)
      {
        Add("uncompressed " + GetDataBlockType(type));
      }
      else if (type <= 0x7e)
      {
        Add("compressed " + GetDataBlockType(type - 0x40));
      }
      else if (type == 0x7f)
      {
        Add("compression table");
      }
      else if (type <= 0xbf)
      {
        Add(GetRomDataType(type) + " ROM data");
      }
      else
      {
        Add(GetRamWriteType(type) + " RAM write data");
      }
      stream.Skip(size);
      return true;
    }
    
    static String GetDataBlockType(uint_t code)
    {
      switch (code)
      {
      case 0x00:
        return "YM2612 PCM";
      case 0x01:
        return "RF5C68 PCM";
      case 0x02:
        return "RF5C164 PCM";
      case 0x03:
        return "PWM PCM";
      case 0x04:
        return "OKIM6258 ADPCM";
      case 0x05:
        return "HuC6280 PCM";
      case 0x06:
        return "SCSP PCM";
      case 0x07:
        return "NES APU DPCM";
      default:
        return Strings::Format("data type=0x%02x", code);
      }
    }
    
    static String GetRomDataType(uint8_t code)
    {
      switch (code)
      {
      case 0x80:
        return "Sega PCM";
      case 0x81:
        return "YM2608 DELTA-T";
      case 0x82:
        return "YM2610 ADPCM";
      case 0x83:
        return "YM2610 DELTA-T";
      case 0x84:
        return "YMF278B";
      case 0x85:
        return "YMF271";
      case 0x86:
        return "YMZ280B";
      case 0x87:
        return "YMF278B";
      case 0x88:
        return "Y8950 DELTA-T";
      case 0x89:
        return "MultiPCM";
      case 0x8a:
        return "uPD7759";
      case 0x8b:
        return "OKIM6295";
      case 0x8c:
        return "K054539";
      case 0x8d:
        return "C140";
      case 0x8e:
        return "K053260";
      case 0x8f:
        return "Q-Sound";
      case 0x90:
        return "ES5505/ES5506";
      case 0x91:
        return "X1-010";
      case 0x92:
        return "C352";
      case 0x93:
        return "GA20";
      default:
        return Strings::Format("type=0x%02x", code);
      }
    }
    
    static String GetRamWriteType(uint8_t code)
    {
      switch (code)
      {
      case 0xc0:
        return "RF5C68";
      case 0xc1:
        return "RF5C164";
      case 0xc2:
        return "NES APU";
      case 0xe0:
        return "SCSP";
      case 0xe1:
        return "ES5503";
      default:
        return Strings::Format("type=0x%02x", code);
      }
    }
    
    bool ParseRamWrite(uint8_t code, Stream& stream)
    {
      if (code != 0x68)
      {
        return false;
      }
      const uint_t compat = stream.ReadByte();
      if (0x66 != compat)
      {
        throw std::runtime_error(Strings::Format("Invalid ram write code %1% at %2%", compat, stream.GetPos() - 1));
      }
      const uint8_t type = stream.ReadByte();
      Add(GetDataBlockType(type) + "RAM write");
      stream.Skip(6);
      const uint32_t size = (stream.ReadDword() - 1) & 0xffffff;
      stream.Skip(size);
      return true;
    }

    bool ParseDacControl(uint8_t code, Stream& stream)
    {
      if (code == 0x90)
      {
        Add("setup stream");
        stream.Skip(4);
      }
      else if (code == 0x91)
      {
        Add("set stream data");
        stream.Skip(4);
      }
      else if (code == 0x92)
      {
        Add("set stream freq");
        stream.Skip(5);
      }
      else if (code == 0x93)
      {
        Add("start stream");
        stream.Skip(10);
      }
      else if (code == 0x94)
      {
        Add("stop stream");
        stream.Skip(1);
      }
      else if (code == 0x95)
      {
        Add("start stream");
        stream.Skip(4);
      }
      else
      {
        return false;
      }
      return true;
    }
    
    bool ParseBuggyCommand(uint8_t code, Stream& stream)
    {
      if (code >= 0x40 && code <= 0x4e)
      {
        const std::size_t curPos = stream.GetPos();
        stream.Seek(8);
        const uint32_t ver = stream.ReadVersion();
        stream.Seek(curPos + 1 + (ver >= 160));
        return true;
      }
      return false;
    }
    
    void Add(const String& cmd)
    {
      Commands.insert(cmd);
    }
  private:
    std::set<String> Commands;
  };
  
  void DumpData(Stream& stream)
  {
    const std::size_t offsetPos = 0x34;
    stream.Seek(offsetPos);
    const uint32_t vgmOffset = stream.ReadDword();
    stream.Seek(vgmOffset ? vgmOffset + offsetPos : 0x40);
    CommandsSet cmds;
    try
    {
      while (cmds.ParseCommand(stream))
      {
      }
    }
    catch (const std::exception& e)
    {
      std::cerr << e.what() << std::endl;
    }
    cmds.Dump();
  }

  void GetInfo(const std::string& filename)
  {
    std::cout << filename << std::endl;
    Stream stream(filename);
    DumpHeader(stream);
    DumpData(stream);
  }
}

int main(int argc, const char* argv[])
{
  for (int arg = 1; arg < argc; ++arg)
  {
    GetInfo(argv[arg]);
  }
  return 0;
}
