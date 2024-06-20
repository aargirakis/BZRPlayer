/**
 *
 * @file
 *
 * @brief  WAV dumper
 *
 * @author vitamin.caig@gmail.com
 *
 **/

#include "../../utils.h"
#include <formats/chiptune/music/wav.h>
#include <strings/format.h>

namespace
{
  using namespace Formats::Chiptune;

  Char ToHex(uint_t val)
  {
    return val >= 10 ? val - 10 + 'A' : val + '0';
  }

  String ToHex(Binary::View data)
  {
    String result;
    result.reserve(data.Size() * 2);
    for (const auto *p = data.As<uint8_t>(), *lim = p + data.Size(); p != lim; ++p)
    {
      result += ToHex(*p >> 4);
      result += ToHex(*p & 15);
    }
    return result;
  }

  class WavBuilder
    : public Wav::Builder
    , public MetaBuilder
  {
  public:
    void SetProgram(StringView program) override
    {
      std::cout << "Program: " << program << std::endl;
    }

    void SetTitle(StringView title) override
    {
      std::cout << "Title: " << title << std::endl;
    }

    void SetAuthor(StringView author) override
    {
      std::cout << "Author: " << author << std::endl;
    }

    void SetStrings(const Strings::Array& strings) override
    {
      for (const auto& str : strings)
      {
        std::cout << "Strings: " << str << std::endl;
      }
    }

    MetaBuilder& GetMetaBuilder() override
    {
      return *this;
    }

    void SetProperties(uint_t formatCode, uint_t frequency, uint_t channels, uint_t bits, uint_t blocksize) override
    {
      std::cout << "Format: " << FormatToString(formatCode) << "\nFrequency: " << frequency
                << "\nChannels: " << channels << "\nBits: " << bits << "\nBlocksize: " << blocksize << std::endl;
    }

    void SetExtendedProperties(uint_t validBitsOrBlockSize, uint_t channelsMask, const Wav::Guid& formatId,
                               Binary::View restData) override
    {
      std::cout << "Extdended format: {" << ToHex(formatId)
                << "}"
                   "\nValid bits or block size: "
                << validBitsOrBlockSize << "\nChannels mask: " << channelsMask << "\nExtra data: (" << restData.Size()
                << " bytes) " << ToHex(restData) << std::endl;
    }

    void SetExtraData(Binary::View data) override
    {
      std::cout << "\nExtra data: (" << data.Size() << " bytes) " << ToHex(data) << std::endl;
    }

    void SetSamplesData(Binary::Container::Ptr data) override
    {
      std::cout << "Samples data: " << data->Size() << " bytes" << std::endl;
    }

    void SetSamplesCountHint(uint_t count) override
    {
      std::cout << "Samples: " << count << std::endl;
    }

  private:
    static const char* FormatToString(uint_t formatCode)
    {
      switch (formatCode)
      {
      case Wav::Format::PCM:
        return "pcm";
      case Wav::Format::ADPCM:
        return "pcm";
      case Wav::Format::IEEE_FLOAT:
        return "float32";
      case Wav::Format::ATRAC3:
        return "atrac3";
      case Wav::Format::EXTENDED:
        return "extended";
      default:
        return "other";
      }
    }
  };
}  // namespace

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      return 0;
    }
    std::unique_ptr<Binary::Dump> rawData(new Binary::Dump());
    Test::OpenFile(argv[1], *rawData);
    const Binary::Container::Ptr data = Binary::CreateContainer(std::move(rawData));
    WavBuilder builder;
    if (const auto result = Formats::Chiptune::Wav::Parse(*data, builder))
    {
      std::cout << result->Size() << " bytes total" << std::endl;
    }
    else
    {
      std::cout << "Failed to parse" << std::endl;
    }
    return 0;
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
    return 1;
  }
}
