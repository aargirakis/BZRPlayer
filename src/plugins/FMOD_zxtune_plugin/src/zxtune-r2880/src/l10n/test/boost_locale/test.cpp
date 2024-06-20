/**
*
* @file
*
* @brief  L10n test
*
* @author vitamin.caig@gmail.com
*
**/

#include <l10n/api.h>
#include <strings/strings_format.h>
#include <pointers.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

namespace
{
  const std::string Domain("test");

  Dump OpenFile(const std::string& name)
  {
    std::ifstream stream(name.c_str(), std::ios::binary);
    if (!stream)
    {
      throw std::runtime_error("Failed to open " + name);
    }
    stream.seekg(0, std::ios_base::end);
    const std::size_t size = stream.tellg();
    stream.seekg(0);
    Dump tmp(size);
    stream.read(safe_ptr_cast<char*>(&tmp[0]), tmp.size());
    return tmp;
  }

  std::string Test(const std::string& domain)
  {
    const L10n::TranslateFunctor translate(domain);
    std::ostringstream str;
    str << translate("Just a message") << std::endl;
    str << translate("context", "Just a message with context") << std::endl;
    for (uint_t idx = 0; idx != 5; ++idx)
    {
      str << Strings::Format(translate("Single form for %1%", "Plural form for %1%", idx), idx) << std::endl;
      str << Strings::Format(translate("another context", "Single form for %1% with context", "Plural form for %1% with context", idx), idx) << std::endl;
    }
    return str.str();
  }

  void Test(const Dump& ref)
  {
    const std::string val = Test(Domain);
    std::cout << val;
    if (ref.size() != val.size() || 0 != std::memcmp(&ref[0], val.data(), ref.size()))
    {
      std::cout << "Failed!" << std::endl;
      throw 1;
    }
  }
}

int main()
{
  try
  {
    L10n::Library& library = L10n::Library::Instance();
    const L10n::Translation eng = {Domain, "en", "mo", OpenFile("en/test.mo")};
    library.AddTranslation(eng);
    const L10n::Translation rus = {Domain, "ru", "mo", OpenFile("ru/test.mo")};
    library.AddTranslation(rus);
    
    std::cout << "Test Default translation" << std::endl;
    Test(OpenFile("default.res"));
    std::cout << "Test English translation" << std::endl;
    library.SelectTranslation("en");
    Test(OpenFile("english.res"));
    std::cout << "Test Russian translation" << std::endl;
    library.SelectTranslation("ru");
    Test(OpenFile("russian.res"));
    std::cout << "Test nonexisting translation" << std::endl;
    library.SelectTranslation("zz");
    Test(OpenFile("default.res"));
    return 0;
  }
  catch (...)
  {
    return 1;
  }
}
