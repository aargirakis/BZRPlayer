/**
* 
* @file
*
* @brief  Portable Sound Format family support implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "portablesoundformat.h"
//common includes
#include <byteorder.h>
#include <crc_.h>
#include <make_ptr.h>
//library includes
#include <binary/data_builder.h>
#include <binary/input_stream.h>
#include <debug/log.h>
#include <formats/chiptune/chiptune_container.h>
#include <strings/encoding.h>
#include <strings/prefixed_index.h>
#include <strings/trim.h>
#include <time/duration.h>
//std includes
#include <cctype>
#include <set>

namespace Formats
{
namespace Chiptune
{
namespace PortableSoundFormat
{
  const Debug::Stream Dbg("Formats::Chiptune::PortableSoundFormat");

  const uint8_t SIGNATURE[] = {'P', 'S', 'F'};
  
  namespace Tags
  {
    const uint8_t SIGNATURE[] = {'[', 'T', 'A', 'G', ']'};
    const String LIB_PREFIX = "_lib";
    
    const char UTF8[] = "utf8";
    const char TITLE[] = "title";
    const char ARTIST[] = "artist";
    const char GAME[] = "game";
    const char YEAR[] = "year";
    const char GENRE[] = "genre";
    const char COMMENT[] = "comment";
    const char COPYRIGHT[] = "copyright";
    const char XSFBY_SUFFIX[] = "sfby";
    const char LENGTH[] = "length";
    const char FADE[] = "fade";
    const char VOLUME[] = "volume";
    
    static String MakeName(StringView str)
    {
      String res;
      res.reserve(str.size());
      for (const auto sym : str)
      {
        res += std::tolower(sym);
      }
      return res;
    }
  }
  
  class Format
  {
  public:
    explicit Format(const Binary::Container& data)
      : Stream(data)
    {
    }
    
    Container::Ptr Parse(Builder& target)
    {
      ParseSignature(target);
      const auto dataStart = Stream.GetPosition();
      ParseData(target);
      const auto dataEnd = Stream.GetPosition();
      ParseTags(target);
      return CreateCalculatingCrcContainer(Stream.GetReadData(), dataStart, dataEnd - dataStart);
    }
  private:
    void ParseSignature(Builder& target)
    {
      const auto sign = Stream.ReadRawData(sizeof(SIGNATURE));
      Require(0 == std::memcmp(sign, SIGNATURE, sizeof(SIGNATURE)));
      const uint_t version = Stream.ReadField<uint8_t>();
      target.SetVersion(version);
      Dbg("Version %1%", version);
    }
    
    void ParseData(Builder& target)
    {
      const auto reservedSize = fromLE(Stream.ReadField<uint32_t>());
      const auto compressedSize = fromLE(Stream.ReadField<uint32_t>());
      const auto compressedCrc = fromLE(Stream.ReadField<uint32_t>());
      if (auto reserved = Stream.ReadData(reservedSize))
      {
        Dbg("Reserved section %1% bytes", reservedSize);
        target.SetReservedSection(std::move(reserved));
      }
      if (compressedSize)
      {
        auto programPacked = Stream.ReadData(compressedSize);
        CheckCrc(*programPacked, compressedCrc);
        Dbg("Program section %1% bytes", compressedSize);
        target.SetPackedProgramSection(std::move(programPacked));
      }
    }
    
    static void CheckCrc(const Binary::Data& blob, uint32_t crc)
    {
      Require(crc == Crc32(static_cast<const uint8_t*>(blob.Start()), blob.Size()));
    }
    
    void ParseTags(Builder& target)
    {
      if (!ReadTagSignature())
      {
        return;
      };
      bool utf8 = false;
      String comment;
      while (Stream.GetRestSize())
      {
        String name;
        StringView valueView;
        if (!ReadTagVariable(name, valueView))
        {
          //Blank lines, or lines not of the form "variable=value", are ignored.
          continue;
        }
        Dbg("tags[%1%]=%2%", name, valueView);
        if (const auto num = FindLibraryNumber(name))
        {
          target.SetLibrary(num, valueView.to_string());
          continue;
        }
        else if (name == Tags::UTF8)
        {
          utf8 = true;
          continue;
        }
        const auto value = utf8
          ? valueView.to_string()
          : Strings::ToAutoUtf8(valueView);
        if (name == Tags::TITLE)
        {
          target.SetTitle(value);
        }
        else if (name == Tags::ARTIST)
        {
          target.SetArtist(value);
        }
        else if (name == Tags::GAME)
        {
          target.SetGame(value);
        }
        else if (name == Tags::YEAR)
        {
          target.SetYear(value);
        }
        else if (name == Tags::GENRE)
        {
          target.SetGenre(value);
        }
        else if (name == Tags::COMMENT)
        {
          if (comment.empty())
          {
            comment = value;
          }
          else
          {
            comment += '\n';
            comment += value;
          }
        }
        else if (name == Tags::COPYRIGHT)
        {
          target.SetCopyright(value);
        }
        else if (name.npos != name.find(Tags::XSFBY_SUFFIX))
        {
          target.SetDumper(value);
        }
        else if (name == Tags::LENGTH)
        {
          target.SetLength(ParseTime(value));
        }
        else if (name == Tags::FADE)
        {
          target.SetFade(ParseTime(value));
        }
        else if (name == Tags::VOLUME)
        {
          target.SetVolume(ParseVolume(value));
        }
        else
        {
          target.SetTag(name, value);
        }
      }
      if (!comment.empty())
      {
        target.SetComment(std::move(comment));
      }
    }
    
    bool ReadTagSignature()
    {
      if (Stream.GetRestSize() < sizeof(Tags::SIGNATURE))
      {
        return false;
      }
      const auto currentPosition = Stream.GetPosition();
      const auto sign = Stream.ReadRawData(sizeof(Tags::SIGNATURE));
      if (0 == std::memcmp(sign, Tags::SIGNATURE, sizeof(Tags::SIGNATURE)))
      {
        return true;
      }
      Stream.Seek(currentPosition);
      return false;
    }
    
    bool ReadTagVariable(String& name, StringView& value)
    {
      const auto line = Stream.ReadString();
      const auto eqPos = line.find('=');
      if (eqPos != line.npos)
      {
        name = Tags::MakeName(Strings::TrimSpaces(line.substr(0, eqPos)));
        value = Strings::TrimSpaces(line.substr(eqPos + 1));
        return true;
      }
      else
      {
        return false;
      }
    }
    
    static uint_t FindLibraryNumber(StringView tagName)
    {
      if (tagName == Tags::LIB_PREFIX)
      {
        return 1;
      }
      else
      {
        const Strings::PrefixedIndex lib(Tags::LIB_PREFIX, tagName);
        const auto num = lib.IsValid() ? lib.GetIndex() : 0;
        Require(num != 1);
        return num;
      }
    }

    static Time::Milliseconds ParseTime(String val)
    {
      //[[hours:]minutes:]seconds.decimal
      Time::Milliseconds result;
      String::size_type start = 0;
      auto end = val.find_first_of(':', start);
      while (end != String::npos)
      {
        val[end] = 0;
        result = Time::Milliseconds(result.Get() * 60 + result.PER_SECOND * std::atoi(val.c_str()  + start));
        end = val.find_first_of(':', start = end + 1);
      }
      return Time::Milliseconds(result.Get() * 60 + result.PER_SECOND * std::atof(val.c_str() + start));
    }
    
    static float ParseVolume(String val)
    {
      return std::atof(val.c_str());
    }
  private:
    Binary::InputStream Stream;
  };
  
  Container::Ptr Parse(const Binary::Container& data, Builder& target)
  {
    try
    {
      return Format(data).Parse(target);
    }
    catch (const std::exception&)
    {
      return Container::Ptr();
    }
  }

}
}
}
