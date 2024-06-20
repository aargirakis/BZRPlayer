/**
*
* @file
*
* @brief  Format test
*
* @author vitamin.caig@gmail.com
*
**/

#include <types.h>
#include <binary/data_adapter.h>
#include <binary/format_factories.h>
#include <binary/format/grammar.h>
#include <binary/format/syntax.h>
#include <sstream>
#include <iostream>
#include <boost/range/size.hpp>
#include <boost/range/algorithm/for_each.hpp>

namespace
{
  void Test(const std::string& msg, bool val)
  {
    std::cout << (val ? "Passed" : "Failed") << " test for " << msg << std::endl;
    if (!val)
      throw 1;
  }
  
  template<class T>
  void Test(const std::string& msg, T result, T reference)
  {
    if (result == reference)
    {
      std::cout << "Passed test for " << msg << std::endl;
    }
    else
    {
      std::cout << "Failed test for " << msg << "\ngot: '" << result << "'\nexp: '" << reference << '\'' << std::endl;
      throw 1;
    }
  }
}

namespace
{  
  const std::string TOKENS("DCMO");

  class GrammarReportCallback : public LexicalAnalysis::Grammar::Callback
  {
  public:
    explicit GrammarReportCallback(std::ostream& str)
      : Str(str)
    {
    }

    virtual void TokenMatched(const std::string& lexeme, LexicalAnalysis::TokenType type)
    {
      Str << TOKENS[type] << '(' << lexeme << ") ";
    }

    virtual void MultipleTokensMatched(const std::string& lexeme, const LexicalAnalysis::TokenTypesSet& /*types*/)
    {
      Str << "X(" << lexeme << ") ";
    }

    virtual void AnalysisError(const std::string& notation, std::size_t position)
    {
      Str << notation.substr(0, position) << " >" << notation.substr(position);
    }
  private:
    std::ostream& Str;
  };

  class SyntaxReportCallback : public Binary::FormatTokensVisitor
  {
  public:
    explicit SyntaxReportCallback(std::ostream& str)
      : Str(str)
    {
    }

    virtual void Match(const std::string& val)
    {
      Str << val << ' ';
    }

    virtual void GroupStart()
    {
      Str << "( ";
    }

    virtual void GroupEnd()
    {
      Str << ") ";
    }

    virtual void Quantor(uint_t count)
    {
      Str << '{' << count << "} ";
    }

    virtual void Operation(const std::string& op)
    {
      Str << op << ' ';
    }
  private:
    std::ostream& Str;
  };

  std::string GetGrammar(const std::string& notation)
  {
    const LexicalAnalysis::Grammar::Ptr grammar = Binary::CreateFormatGrammar();
    std::ostringstream result;
    GrammarReportCallback cb(result);
    grammar->Analyse(notation, cb);
    return result.str();
  }

  std::string GetSyntax(const std::string& notation)
  {
    std::ostringstream result;
    try
    {
      SyntaxReportCallback cb(result);
      Binary::ParseFormatNotation(notation, cb);
    }
    catch (const std::exception&)
    {
    }
    return result.str();
  }

  std::string GetSyntaxRPN(const std::string& notation)
  {
    std::ostringstream result;
    try
    {
      SyntaxReportCallback cb(result);
      Binary::ParseFormatNotationPostfix(notation, cb);
    }
    catch (const std::exception&)
    {
    }
    return result.str();
  }

  std::string GetSyntaxRPNChecked(const std::string& notation)
  {
    std::ostringstream result;
    try
    {
      SyntaxReportCallback cb(result);
      const Binary::FormatTokensVisitor::Ptr adapter = Binary::CreatePostfixSynaxCheckAdapter(cb);
      Binary::ParseFormatNotationPostfix(notation, *adapter);
    }
    catch (const std::exception&)
    {
    }
    return result.str();
  }
}

namespace
{
  struct FormatResult
  {
    bool Matched;
    std::size_t NextMatch;

    FormatResult(bool matched, std::size_t nextMatch)
      : Matched(matched)
      , NextMatch(nextMatch)
    {
    }
  };

  const FormatResult INVALID_FORMAT = FormatResult(false, 0);

  struct FormatTest
  {
    std::string Name;
    std::string Notation;
    std::string GrammarReport;
    std::string SyntaxReport;
    std::string SyntaxReportRPN;
    std::string SyntaxReportRPNChecked;
    FormatResult Result;
    FormatResult MatchOnlyResult;
  };

  const uint8_t SAMPLE[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

  FormatResult CheckFormat(const std::string& notation)
  {
    try
    {
      const Binary::Format::Ptr format = Binary::CreateFormat(notation);
      const Binary::DataAdapter sample(SAMPLE, boost::size(SAMPLE));
      return FormatResult(format->Match(sample), format->NextMatchOffset(sample));
    }
    catch (const std::exception&)
    {
      return INVALID_FORMAT;
    }
  }

  FormatResult CheckMatchOnlyFormat(const std::string& notation)
  {
    try
    {
      const Binary::Format::Ptr format = Binary::CreateMatchOnlyFormat(notation);
      const Binary::DataAdapter sample(SAMPLE, boost::size(SAMPLE));
      return FormatResult(format->Match(sample), format->NextMatchOffset(sample));
    }
    catch (const std::exception&)
    {
      return INVALID_FORMAT;
    }
  }

  FormatResult CheckCompositeFormat(const std::string& header, const std::string footer, std::size_t minSize, std::size_t maxSize)
  {
    try
    {
      const Binary::Format::Ptr hdr = Binary::CreateFormat(header, minSize);
      const Binary::Format::Ptr foot = Binary::CreateFormat(footer);
      const Binary::Format::Ptr format = Binary::CreateCompositeFormat(hdr, foot, minSize, maxSize);
      const Binary::DataAdapter sample(SAMPLE, boost::size(SAMPLE));
      return FormatResult(format->Match(sample), format->NextMatchOffset(sample));
    }
    catch (const std::exception&)
    {
      return INVALID_FORMAT;
    }
  }

  const FormatTest TESTS[] =
  {
    //invalid grammar/syntax
    {
      "invalid char",
      "@",
      " >@",
      "",
      "",
      "",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "invalid hexvalue",
      "abc",
      "C(ab) ab >c",
      "ab ",
      "ab ",
      "ab ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "invalid binvalue",
      "%10101102",
      " >%10101102",
      "",
      "",
      "",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "invalid value",
      "123z",
      "C(123) 123 >z",
      "",
      "",
      "",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "bad nibble",
      "0g",
      "C(0) 0 >g",
      "",
      "",
      "",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "incomplete nibble",
      "\?0",
      "M(\?) C(0) ",
      "\? ",
      "\? ",
      "\? ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    //invalid format
    {
      "single delimiter",
      " ",
      "D( ) ",
      "",
      "",
      "",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "multichar delimiter",
      " \t",
      "D( \t) ",
      "",
      "",
      "",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "decimal constant",
      "10",
      "C(10) ",
      "10 ",
      "10 ",
      "10 ",
      FormatResult(false, 0x10),
      FormatResult(false, 32)
    },
    {
      "hexadecimal constant",
      "af",
      "C(af) ",
      "af ",
      "af ",
      "af ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "binary constant",
      "%01010101",
      "C(%01010101) ",
      "%01010101 ",
      "%01010101 ",
      "%01010101 ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "char constant",
      "'A",
      "C('A) ",
      "'A ",
      "'A ",
      "'A ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "any mask",
      "\?",
      "M(\?) ",
      "\? ",
      "\? ",
      "\? ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "hex mask",
      "2x",
      "M(2x) ",
      "2x ",
      "2x ",
      "2x ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "bin mask",
      "%01xx1010",
      "M(%01xx1010) ",
      "%01xx1010 ",
      "%01xx1010 ",
      "%01xx1010 ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "mult mask",
      "*5",
      "M(*5) ",
      "*5 ",
      "*5 ",
      "*5 ",
      FormatResult(true, 5),
      FormatResult(true, 32)
    },
    {
      "quantor on empty",
      "{1}",
      "O({) C(1) O(}) ",
      "{1} ",
      "{1} ",
      "",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "zero quantor",
      "10{0}",
      "C(10) O({) C(0) O(}) ",
      "10 {0} ",
      "10 {0} ",
      "10 ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "invalid bracket",
      ")10",
      "O()) C(10) ",
      ") 10 ",
      "",
      "",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "brackets mismatch",
      "(10",
      "O(() C(10) ",
      "( 10 ",
      "( 10 ",
      "( 10 ",//???
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "multival operation left",
      "(0001)|02",
      "O(() C(0001) O()) O(|) C(02) ",
      "( 00 01 ) | 02 ",
      "( 00 01 ) 02 | ",
      "( 00 01 ) 02 ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "singleval operation left",
      "(00)|02",
      "O(() C(00) O()) O(|) C(02) ",
      "( 00 ) | 02 ",
      "( 00 ) 02 | ",
      "( 00 ) 02 | ",
      FormatResult(true, 2),
      FormatResult(true, 32)
    },
    {
      "multival operation right",
      "00|(0102)",
      "C(00) O(|) O(() C(0102) O()) ",
      "00 | ( 01 02 ) ",
      "00 | ( 01 02 ) ",
      "00 ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "singleval operation right",
      "00|(02)",
      "C(00) O(|) O(() C(02) O()) ",
      "00 | ( 02 ) ",
      "00 | ( 02 ) ",
      "00 ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "single char delimiters",
      ",,",
      "D(,) D(,) ",
      "",
      "",
      "",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "invalid range",
      "0x-x0",
      "M(0x) O(-) M(x0) ",
      "0x - x0 ",
      "0x x0 - ",
      "0x x0 - ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "empty range",
      "05-05",
      "C(05) O(-) C(05) ",
      "05 - 05 ",
      "05 05 - ",
      "05 05 - ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "full range",
      "00-ff",
      "C(00) O(-) C(ff) ",
      "00 - ff ",
      "00 ff - ",
      "00 ff - ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "invalid multiple range",
      "01-02-03",
      "C(01) O(-) C(02) O(-) C(03) ",
      "01 - 02 - 03 ",
      "01 02 - 03 - ",
      "01 02 - 03 - ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "empty conjunction",
      "01&10",
      "C(01) O(&) C(10) ",
      "01 & 10 ",
      "01 10 & ",
      "01 10 & ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    {
      "full disjunction",
      "00-80|80-ff",
      "C(00) O(-) C(80) O(|) C(80) O(-) C(ff) ",
      "00 - 80 | 80 - ff ",
      "00 80 - 80 ff - | ",
      "00 80 - 80 ff - | ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    //brackets are used only for grouping, not for operations reordering
    {
      "grouping",
      "(00|01)&(01|02)",
      "O(() C(00) O(|) C(01) O()) O(&) O(() C(01) O(|) C(02) O()) ",
      "( 00 | 01 ) & ( 01 | 02 ) ",
      "( 00 01 | ) & ( 01 02 | ) ",
      "( 00 01 | ) ",
      INVALID_FORMAT,
      INVALID_FORMAT
    },
    //valid tests
    {
      "whole explicit match",
      "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
      "C(000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f) ",
      "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f ",
      "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f ",
      "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f ",
      FormatResult(true, 32),
      FormatResult(true, 32)
    },
    {
      "partial explicit match",
      "000102030405",
      "C(000102030405) ",
      "00 01 02 03 04 05 ",
      "00 01 02 03 04 05 ",
      "00 01 02 03 04 05 ",
      FormatResult(true, 32),
      FormatResult(true, 32)
    },
    {
      "whole mask match",
      "\?0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
      "M(\?) C(0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f) ",
      "\? 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f ",
      "\? 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f ",
      "\? 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f ",
      FormatResult(true, 32),
      FormatResult(true, 32)
    },
    {
      "partial mask match",
      "00\?02030405",
      "C(00) M(\?) C(02030405) ",
      "00 \? 02 03 04 05 ",
      "00 \? 02 03 04 05 ",
      "00 \? 02 03 04 05 ",
      FormatResult(true, 32),
      FormatResult(true, 32)
    },
    {
      "full oversize unmatch",
      "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20",
      "C(000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20) ",
      "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 ",
      "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 ",
      "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "nibbles matched",
      "0x0x",
      "M(0x0x) ",
      "0x 0x ",
      "0x 0x ",
      "0x 0x ",
      FormatResult(true, 1),
      FormatResult(true, 32)
    },
    {
      "nibbles unmatched",
      "0x1x",
      "M(0x1x) ",
      "0x 1x ",
      "0x 1x ",
      "0x 1x ",
      FormatResult(false, 15),
      FormatResult(false, 32)
    },
    {
      "binary matched",
      "%0x0x0x0x%x0x0x0x1", 
      "M(%0x0x0x0x) M(%x0x0x0x1) ",
      "%0x0x0x0x %x0x0x0x1 ",
      "%0x0x0x0x %x0x0x0x1 ",
      "%0x0x0x0x %x0x0x0x1 ",
      FormatResult(true, 32),
      FormatResult(true, 32)
    },
    {
      "binary unmatched",
      "%00010xxx%00011xxx",
      "M(%00010xxx) M(%00011xxx) ",
      "%00010xxx %00011xxx ",
      "%00010xxx %00011xxx ",
      "%00010xxx %00011xxx ",
      FormatResult(false, 0x17),
      FormatResult(false, 32)
    },
    {
      "ranged matched",
      "00-0200-02",
      "C(00) O(-) C(0200) O(-) C(02) ",
      "00 - 02 00 - 02 ",
      "00 02 - 00 02 - ",
      "00 02 - 00 02 - ",
      FormatResult(true, 1),
      FormatResult(true, 32)
    },
    {
      "ranged unmatched",
      "10-12",
      "C(10) O(-) C(12) ",
      "10 - 12 ",
      "10 12 - ",
      "10 12 - ",
      FormatResult(false, 0x10),
      FormatResult(false, 32),
    },
    {
      "symbol unmatched",
      "'a'b'c'd'e",
      "C('a) C('b) C('c) C('d) C('e) ",
      "'a 'b 'c 'd 'e ",
      "'a 'b 'c 'd 'e ",
      "'a 'b 'c 'd 'e ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "matched from 1 with skip at begin",
      "\?0203",
      "M(\?) C(0203) ",
      "\? 02 03 ",
      "\? 02 03 ",
      "\? 02 03 ",
      FormatResult(false, 1),
      FormatResult(false, 32)
    },
    {
      "unmatched with skip at begin",
      "\?0302", 
      "M(\?) C(0302) ",
      "\? 03 02 ",
      "\? 03 02 ",
      "\? 03 02 ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "matched from 13 with skip at end",
      "0d0e\?\?111213\?\?", 
      "C(0d0e) M(\?) M(\?) C(111213) M(\?) M(\?) ",
      "0d 0e \? \? 11 12 13 \? \? ",
      "0d 0e \? \? 11 12 13 \? \? ",
      "0d 0e \? \? 11 12 13 \? \? ",
      FormatResult(false, 13),
      FormatResult(false, 32)
    },
    {
      "unmatched with skip at end",
      "0302\?\?", 
      "C(0302) M(\?) M(\?) ",
      "03 02 \? \? ",
      "03 02 \? \? ",
      "03 02 \? \? ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "partially matched at end",
      "1d1e1f20",
      "C(1d1e1f20) ",
      "1d 1e 1f 20 ",
      "1d 1e 1f 20 ",
      "1d 1e 1f 20 ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "partially matched at end with skipping",
      "\?1d1e1f202122",
      "M(\?) C(1d1e1f202122) ",
      "\? 1d 1e 1f 20 21 22 ",
      "\? 1d 1e 1f 20 21 22 ",
      "\? 1d 1e 1f 20 21 22 ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "quantor matched",
      "0x{10}",
      "M(0x) O({) C(10) O(}) ",
      "0x {10} ",
      "0x {10} ",
      "0x {10} ",
      FormatResult(true, 1),
      FormatResult(true, 32)
    },
    {
      "quantor unmatched",
      "1x{15}",
      "M(1x) O({) C(15) O(}) ",
      "1x {15} ",
      "1x {15} ",
      "1x {15} ",
      FormatResult(false, 16),
      FormatResult(false, 32)
    },
    {
      "quanted group matched",
      "(%xxxxxxx0%xxxxxxx1){3}",
      "O(() M(%xxxxxxx0) M(%xxxxxxx1) O()) O({) C(3) O(}) ",
      "( %xxxxxxx0 %xxxxxxx1 ) {3} ",
      "( %xxxxxxx0 %xxxxxxx1 ) {3} ",
      "( %xxxxxxx0 %xxxxxxx1 ) {3} ",
      FormatResult(true, 2),
      FormatResult(true, 32)
    },
    {
      "quanted group unmatched",
      "(%xxxxxxx1%xxxxxxx0){5}",
      "O(() M(%xxxxxxx1) M(%xxxxxxx0) O()) O({) C(5) O(}) ",
      "( %xxxxxxx1 %xxxxxxx0 ) {5} ",
      "( %xxxxxxx1 %xxxxxxx0 ) {5} ",
      "( %xxxxxxx1 %xxxxxxx0 ) {5} ",
      FormatResult(false, 1),
      FormatResult(false, 32)
    },
    {
      "group after range",
      "00-01(02030405)",
      "C(00) O(-) C(01) O(() C(02030405) O()) ",
      "00 - 01 ( 02 03 04 05 ) ",
      "00 01 - ( 02 03 04 05 ) ",
      "00 01 - ( 02 03 04 05 ) ",
      FormatResult(false, 1),
      FormatResult(false, 32)
    },
    {
      "group before range",
      "(00010203)04-05",
      "O(() C(00010203) O()) C(04) O(-) C(05) ",
      "( 00 01 02 03 ) 04 - 05 ",
      "( 00 01 02 03 ) 04 05 - ",
      "( 00 01 02 03 ) 04 05 - ",
      FormatResult(true, 32),
      FormatResult(true, 32)
    },
    {
      "quanted range matched",
      "00-1f{32}",
      "C(00) O(-) C(1f) O({) C(32) O(}) ",
      "00 - 1f {32} ",
      "00 1f - {32} ",
      "00 1f - {32} ",
      FormatResult(true, 32),
      FormatResult(true, 32)
    },
    {
      "multiplicity matched",
      "00010203 *2 05 *3",
      "C(00010203) D( ) M(*2) D( ) C(05) D( ) M(*3) ",
      "00 01 02 03 *2 05 *3 ",
      "00 01 02 03 *2 05 *3 ",
      "00 01 02 03 *2 05 *3 ",
      FormatResult(true, 32),
      FormatResult(true, 32)
    },
    {
      "multiplicity unmatched",
      "*2*3*4",
      "M(*2) M(*3) M(*4) ",
      "*2 *3 *4 ",
      "*2 *3 *4 ",
      "*2 *3 *4 ",
      FormatResult(false, 2),
      FormatResult(false, 32)
    },
    {
      "conjunction matched",
      "00 0x&x1",
      "C(00) D( ) M(0x) O(&) M(x1) ",
      "00 0x & x1 ",
      "00 0x x1 & ",
      "00 0x x1 & ",
      FormatResult(true, 32),
      FormatResult(true, 32)
    },
    {
      "conjunction unmatched",
      "(1x&%xxxx11xx){4}",
      "O(() M(1x) O(&) M(%xxxx11xx) O()) O({) C(4) O(}) ",
      "( 1x & %xxxx11xx ) {4} ",
      "( 1x %xxxx11xx & ) {4} ",
      "( 1x %xxxx11xx & ) {4} ",
      FormatResult(false, 0x1c),
      FormatResult(false, 32)
    },
    {
      "disjunction matched",
      "00|01 00|01",
      "C(00) O(|) C(01) D( ) C(00) O(|) C(01) ",
      "00 | 01 00 | 01 ",
      "00 01 | 00 01 | ",
      "00 01 | 00 01 | ",
      FormatResult(true, 32),
      FormatResult(true, 32)
    },
    {
      "disjunction unmatched",
      "(1x|%xxxx11xx){6}",
      "O(() M(1x) O(|) M(%xxxx11xx) O()) O({) C(6) O(}) ",
      "( 1x | %xxxx11xx ) {6} ",
      "( 1x %xxxx11xx | ) {6} ",
      "( 1x %xxxx11xx | ) {6} ",
      FormatResult(false, 12),
      FormatResult(false, 32)
    },
    {
      "complex condition matched",
      "00-01|1e-1f",
      "C(00) O(-) C(01) O(|) C(1e) O(-) C(1f) ",
      "00 - 01 | 1e - 1f ",
      "00 01 - 1e 1f - | ",
      "00 01 - 1e 1f - | ",
      FormatResult(true, 1),
      FormatResult(true, 32)
    },
    {
      "trd format expression",
      "(00|01|20-7f??????? ??? ?? ? 0x 00-a0){128}00?{224}??1601-7f?00-09100000?????????00?20-7f{8}000000",
      "O(() C(00) O(|) C(01) O(|) C(20) O(-) C(7f) M(?) M(?) M(?) M(?) M(?) M(?) M(?) D( ) "
        "M(?) M(?) M(?) D( ) M(?) M(?) D( ) M(?) D( ) M(0x) D( ) C(00) O(-) C(a0) O()) O({) C(128) O(}) C(00) M(?) O({) C(224) O(}) M(?) M(?) "
        "C(1601) O(-) C(7f) M(?) C(00) O(-) C(09100000) M(?) M(?) M(?) M(?) M(?) M(?) M(?) M(?) M(?) C(00) M(?) C(20) O(-) C(7f) O({) C(8) O(}) C(000000) ",
      "( 00 | 01 | 20 - 7f ? ? ? ? ? ? ? ? ? ? ? ? ? 0x 00 - a0 ) {128} 00 ? {224} ? ? 16 01 - 7f ? 00 - 09 10 00 00 ? ? ? ? ? ? ? ? ? 00 ? 20 - 7f {8} 00 00 00 ",
      "( 00 01 | 20 7f - | ? ? ? ? ? ? ? ? ? ? ? ? ? 0x 00 a0 - ) {128} 00 ? {224} ? ? 16 01 7f - ? 00 09 - 10 00 00 ? ? ? ? ? ? ? ? ? 00 ? 20 7f - {8} 00 00 00 ",
      "( 00 01 | 20 7f - | ? ? ? ? ? ? ? ? ? ? ? ? ? 0x 00 a0 - ) {128} 00 ? {224} ? ? 16 01 7f - ? 00 09 - 10 00 00 ? ? ? ? ? ? ? ? ? 00 ? 20 7f - {8} 00 00 00 ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    },
    {
      "pt3 format expression",
      "?{13}??{16}?{32}?{4}?{32}??01-ff???00-01(?00-bf){32}(?00-d9){16}*3&00-fe*3",
      "M(?) O({) C(13) O(}) M(?) M(?) O({) C(16) O(}) M(?) O({) C(32) O(}) M(?) O({) C(4) O(}) M(?) O({) C(32) O(}) M(?) M(?) C(01) O(-) C(ff) M(?) M(?) M(?) "
        "C(00) O(-) C(01) O(() M(?) C(00) O(-) C(bf) O()) O({) C(32) O(}) O(() M(?) C(00) O(-) C(d9) O()) O({) C(16) O(}) M(*3) O(&) C(00) O(-) C(fe) M(*3) ",
      "? {13} ? ? {16} ? {32} ? {4} ? {32} ? ? 01 - ff ? ? ? 00 - 01 ( ? 00 - bf ) {32} ( ? 00 - d9 ) {16} *3 & 00 - fe *3 ",
      "? {13} ? ? {16} ? {32} ? {4} ? {32} ? ? 01 ff - ? ? ? 00 01 - ( ? 00 bf - ) {32} ( ? 00 d9 - ) {16} *3 00 fe - & *3 ",
      "? {13} ? ? {16} ? {32} ? {4} ? {32} ? ? 01 ff - ? ? ? 00 01 - ( ? 00 bf - ) {32} ( ? 00 d9 - ) {16} *3 00 fe - & *3 ",
      FormatResult(false, 32),
      FormatResult(false, 32)
    }
  };

  void ExecuteTest(const FormatTest& tst)
  {
    std::cout << "Testing for " << tst.Name << " (#" << &tst - TESTS << ')' << std::endl;
    Test("grammar", GetGrammar(tst.Notation), tst.GrammarReport);
    Test("syntax", GetSyntax(tst.Notation), tst.SyntaxReport);
    Test("syntax RPN", GetSyntaxRPN(tst.Notation), tst.SyntaxReportRPN);
    Test("syntax RPN checked", GetSyntaxRPNChecked(tst.Notation), tst.SyntaxReportRPNChecked);
    const FormatResult res = CheckFormat(tst.Notation);
    Test("match", res.Matched, tst.Result.Matched);
    Test("next match offset", res.NextMatch, tst.Result.NextMatch);
    const FormatResult resMatched = CheckMatchOnlyFormat(tst.Notation);
    Test("match (only)", resMatched.Matched, tst.MatchOnlyResult.Matched);
    Test("next match offset (only)", resMatched.NextMatch, tst.MatchOnlyResult.NextMatch);
  }

  struct CompositeFormatTest
  {
    std::string Name;
    std::string Header;
    std::string Footer;
    std::size_t MinSize;
    std::size_t MaxFooterOffset;
    FormatResult Result;
  };

  const CompositeFormatTest COMPOSITE_TESTS[] =
  {
    {
      "whole matching",
      "0001", "1e1f",
      4, 32,
      FormatResult(true, 32)
    },
    {
      "matched from begin",
      "0001", "1011",
      4, 32,
      FormatResult(true, 32)
    },
    {
      "matched at end",
      "0203", "1e1f",
      4, 32,
      FormatResult(false, 2)
    },
    {
      "matched at middle",
      "0203", "1011",
      4, 32,
      FormatResult(false, 2)
    },
    {
      "not matched header",
      "0002", "1e1f",
      4, 32,
      FormatResult(false, 32)
    },
    {
      "not matched footer",
      "0001", "1e20",
      4, 32,
      FormatResult(false, 32)
    },
    {
      "not matched max footer offset",
      "0001", "1e1f",
      4, 29,
      FormatResult(false, 32)
    },
    {
      "matched max footer offset",
      "0001", "1e1f",
      4, 30,
      FormatResult(true, 32)
    },
    {
      "not matched minsize",
      "0001", "1e1f",
      33, 33,
      FormatResult(false, 32)
    },
    {
      "matched with overlap",
      "0203040506070809", "x8x9",
      4, 32,
      FormatResult(false, 2)
    },
    {
      "matched with iterations",
      "x2x3x4", "1e1f",
      4, 16,
      FormatResult(false, 0x12)
    },
    {
      "matched with no skip at begin",
      "00010203", "040506",
      4, 32,
      FormatResult(true, 32)
    },
    {
      "matched with no skip at middle",
      "02030405", "06070809",
      4, 7,
      FormatResult(false, 2)
    },
  };

  void ExecuteCompositeTest(const CompositeFormatTest& tst)
  {
    std::cout << "Testing for composite format: " << tst.Name << std::endl;
    const FormatResult res = CheckCompositeFormat(tst.Header, tst.Footer, tst.MinSize, tst.MaxFooterOffset);
    Test("match", res.Matched, tst.Result.Matched);
    Test("next match offset", res.NextMatch, tst.Result.NextMatch);
  }
}

int main()
{
  try
  {
    boost::for_each(TESTS, std::ptr_fun(&ExecuteTest));
    boost::for_each(COMPOSITE_TESTS, std::ptr_fun(&ExecuteCompositeTest));
  }
  catch (int code)
  {
    return code;
  }
}
