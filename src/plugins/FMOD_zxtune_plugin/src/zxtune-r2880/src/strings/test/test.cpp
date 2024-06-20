/**
*
* @file
*
* @brief  Strings test
*
* @author vitamin.caig@gmail.com
*
**/

#include <strings/fields.h>
#include <strings/fields_filter.h>
#include <strings/map.h>
#include <strings/template.h>

#include <iostream>

namespace
{
  template<class Policy>
  class FieldsSourceFromMap : public Policy
  {
  public:
    explicit FieldsSourceFromMap(const Strings::Map& map)
      : Map(map)
    {
    }
    
    virtual String GetFieldValue(const String& name) const
    {
      const Strings::Map::const_iterator it = Map.find(name);
      return it == Map.end() ? Policy::GetFieldValue(name) : it->second;
    }
  private:
    const Strings::Map& Map;
  };
  
  void TestTemplate(const Strings::FieldsSource& source, const String& templ, const String& reference)
  {
    const String res = Strings::Template::Instantiate(templ, source);
    if (res == reference)
    {
      std::cout << "Passed test for '" << templ << '\'' << std::endl;
    }
    else
    {
      std::cout << "Failed test for '" << templ << "' (result is '" << res << "')" << std::endl;
      throw 1;
    }
  }

  template<class Policy>
  void TestTemplate(const String& templ, const Strings::Map& params, const String& reference)
  {
    const FieldsSourceFromMap<Policy> source(params);
    TestTemplate(source, templ, reference);
  }
}

int main()
{
  try
  {
    std::cout << "---- Test for string template ----" << std::endl;
    {
      TestTemplate<Strings::SkipFieldsSource>("without template", Strings::Map(), "without template");
      TestTemplate<Strings::KeepFieldsSource>("no [mapped] template", Strings::Map(), "no [mapped] template");
      TestTemplate<Strings::SkipFieldsSource>("no [nonexisting] template", Strings::Map(), "no  template");
      TestTemplate<Strings::FillFieldsSource>("no [tabulated] template", Strings::Map(), "no             template");
      Strings::Map params;
      params["name"] = "value";
      TestTemplate<Strings::SkipFieldsSource>("single [name] test", params, "single value test");
      TestTemplate<Strings::SkipFieldsSource>("duplicate [name] and [name] test", params, "duplicate value and value test");
      params["value"] = "name";
      TestTemplate<Strings::SkipFieldsSource>("multiple [name] and [value] test", params, "multiple value and name test");
      TestTemplate<Strings::SkipFieldsSource>("syntax error [name test", params, "syntax error [name test");
      TestTemplate<Strings::SkipFieldsSource>("[name] at the beginning", params, "value at the beginning");
      TestTemplate<Strings::SkipFieldsSource>("at the end [name]", params, "at the end value");
      const FieldsSourceFromMap<Strings::SkipFieldsSource> source(params);
      const Strings::FilterFieldsSource replaceToChar(source, "abcde", '%');
      TestTemplate(replaceToChar, "Replace bunch of symbols to single in [name] and [value]", "Replace bunch of symbols to single in v%lu% and n%m%");
      const Strings::FilterFieldsSource replaceToCharsSet(source, "abcde", "ABCDE");
      TestTemplate(replaceToCharsSet, "Replace bunch of symbols to multiple in [name] and [value]", "Replace bunch of symbols to multiple in vAluE and nAmE");
    }
  }
  catch (int code)
  {
    return code;
  }
}
