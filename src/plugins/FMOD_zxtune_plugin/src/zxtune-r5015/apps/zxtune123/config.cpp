/**
 *
 * @file
 *
 * @brief  Parsing tools implementation
 *
 * @author vitamin.caig@gmail.com
 *
 **/

// local includes
#include "config.h"
// common includes
#include <error_tools.h>
// library includes
#include <parameters/serialize.h>
#include <strings/map.h>
// std includes
#include <cctype>
#include <fstream>

#define FILE_TAG 0DBA1FA8

namespace
{
  static const Char PARAMETERS_DELIMITER = ',';

  static const Char CONFIG_FILENAME[] = "zxtune.conf";

  // try to search config in homedir, if defined
  String GetDefaultConfigFile()
  {
#ifdef _WIN32
    static const Char ENV_HOMEDIR[] = "APPDATA";
    static const Char PATH[] = "\\zxtune\\";
#else
    static const Char ENV_HOMEDIR[] = "HOME";
    static const Char PATH[] = "/.zxtune/";
#endif
    String dir;
    if (const auto* homeDir = ::getenv(ENV_HOMEDIR))
    {
      dir = String(homeDir) + PATH;
    }
    return dir + CONFIG_FILENAME;
  }

  void ParseParametersString(Parameters::Identifier prefix, const String& str, Strings::Map& result)
  {
    Strings::Map res;

    enum
    {
      IN_NAME,
      IN_VALUE,
      IN_VALSTR,
      IN_NOWHERE
    } mode = IN_NAME;

    /*
     parse strings in form name=value[,name=value...]
      name ::= [\w\d\._]*
      value ::= \"[^\"]*\"
      value ::= [^,]*
      name is prepended with prefix before insert to result
    */
    String paramName, paramValue;
    for (String::const_iterator it = str.begin(), lim = str.end(); it != lim; ++it)
    {
      bool doApply = false;
      const Char sym(*it);
      switch (mode)
      {
      case IN_NOWHERE:
        if (sym == PARAMETERS_DELIMITER)
        {
          break;
        }
        [[fallthrough]];
      case IN_NAME:
        if (sym == '=')
        {
          mode = IN_VALUE;
        }
        else if (!std::isspace(sym))
        {
          paramName += sym;
        }
        else
        {
          throw MakeFormattedError(THIS_LINE, "Invalid parameter format '%1%'.", str);
        }
        break;
      case IN_VALUE:
        if (Parameters::STRING_QUOTE == sym)
        {
          paramValue += sym;
          mode = IN_VALSTR;
        }
        else if (sym == PARAMETERS_DELIMITER)
        {
          doApply = true;
          mode = IN_NOWHERE;
          --it;
        }
        else
        {
          paramValue += sym;
        }
        break;
      case IN_VALSTR:
        paramValue += sym;
        if (Parameters::STRING_QUOTE == sym)
        {
          doApply = true;
          mode = IN_NOWHERE;
        }
        break;
      default:
        assert(!"Invalid state");
      };

      if (doApply)
      {
        res.emplace(prefix.Append(paramName), paramValue);
        paramName.clear();
        paramValue.clear();
      }
    }
    if (IN_VALUE == mode)
    {
      res.emplace(prefix.Append(paramName), paramValue);
    }
    else if (IN_NOWHERE != mode)
    {
      throw MakeFormattedError(THIS_LINE, "Invalid parameter format '%1%'.", str);
    }
    result.swap(res);
  }

  void ParseConfigFile(const String& filename, String& params)
  {
    const String configName(filename.empty() ? CONFIG_FILENAME : filename);

    typedef std::basic_ifstream<Char> FileStream;
    std::unique_ptr<FileStream> configFile(new FileStream(configName.c_str()));
    if (!*configFile)
    {
      if (!filename.empty())
      {
        throw Error(THIS_LINE, "Failed to open configuration file " + configName);
      }
      configFile.reset(new FileStream(GetDefaultConfigFile().c_str()));
    }
    if (!*configFile)
    {
      params.clear();
      return;
    }

    String lines;
    std::vector<Char> buffer(1024);
    for (;;)
    {
      configFile->getline(&buffer[0], buffer.size());
      if (const std::streamsize lineSize = configFile->gcount())
      {
        std::vector<Char>::const_iterator endof(buffer.begin() + lineSize - 1);
        auto beginof = std::find_if<std::vector<Char>::const_iterator>(buffer.begin(), endof,
                                                                       [](Char c) { return !std::isspace(c); });
        if (beginof != endof && *beginof != Char('#'))
        {
          if (!lines.empty())
          {
            lines += PARAMETERS_DELIMITER;
          }
          lines += String(beginof, endof);
        }
      }
      else
      {
        break;
      }
    }
    params = lines;
  }
}  // namespace

void ParseConfigFile(const String& filename, Parameters::Modifier& result)
{
  String strVal;
  ParseConfigFile(filename, strVal);
  if (!strVal.empty())
  {
    ParseParametersString("", strVal, result);
  }
}

void ParseParametersString(Parameters::Identifier pfx, const String& str, Parameters::Modifier& result)
{
  Strings::Map strMap;
  ParseParametersString(pfx, str, strMap);
  Parameters::Convert(strMap, result);
}
