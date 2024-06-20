/**
*
* @file
*
* @brief  L10n stub implementation
*
* @author vitamin.caig@gmail.com
*
**/

//common includes
#include <pointers.h>
//library includes
#include <l10n/src/library.h>

namespace
{
  class StubVocabulary : public L10n::Vocabulary
  {
  public:
    String GetText(const char* text) const override
    {
      return FromStdString(std::string(text));
    }

    String GetText(const char* single, const char* plural, int count) const override
    {
      return FromStdString(std::string(count == 1 ? single : plural));
    }

    String GetText(const char* /*context*/, const char* text) const override
    {
      return FromStdString(std::string(text));
    }

    String GetText(const char* /*context*/, const char* single, const char* plural, int count) const override
    {
      return FromStdString(std::string(count == 1 ? single : plural));
    }
  };

  class StubLibrary : public L10n::Library
  {
  public:
    void AddTranslation(const L10n::Translation& /*trans*/) override
    {
    }

    void SelectTranslation(const std::string& /*translation*/) override
    {
    }

    L10n::Vocabulary::Ptr GetVocabulary(const std::string& /*domain*/) const override
    {
      static StubVocabulary voc;
      return MakeSingletonPointer(voc);
    }
  };
}

namespace L10n
{
  Library& Library::Instance()
  {
    static StubLibrary instance;
    return instance;
  }

  void LoadTranslationsFromResources(Library& /*lib*/)
  {
  }
}
