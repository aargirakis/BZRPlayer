/**
*
* @file
*
* @brief  Matching-only format implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "static_expression.h"
//common includes
#include <make_ptr.h>
//library includes
#include <binary/format_factories.h>

namespace Binary
{
  class MatchOnlyFormatBase : public Format
  {
  public:
    std::size_t NextMatchOffset(const Data& data) const override
    {
      return data.Size();
    }
  };

  class FuzzyMatchOnlyFormat : public MatchOnlyFormatBase
  {
  public:
    FuzzyMatchOnlyFormat(FormatDSL::StaticPattern mtx, std::size_t offset, std::size_t minSize)
      : Offset(offset)
      , MinSize(std::max(minSize, mtx.GetSize() + offset))
      , Pattern(std::move(mtx))
    {
    }

    bool Match(const Data& data) const override
    {
      if (data.Size() < MinSize)
      {
        return false;
      }
      const uint8_t* const typedData = static_cast<const uint8_t*>(data.Start()) + Offset;
      for (std::size_t idx = 0, lim = Pattern.GetSize(); idx != lim; ++idx)
      {
        if (!Pattern.Get(idx).Match(typedData[idx]))
        {
          return false;
        }
      }
      return true;
    }

    static Ptr Create(FormatDSL::StaticPattern expr, std::size_t startOffset, std::size_t minSize)
    {
      return MakePtr<FuzzyMatchOnlyFormat>(std::move(expr), startOffset, minSize);
    }
  private:
    const std::size_t Offset;
    const std::size_t MinSize;
    const FormatDSL::StaticPattern Pattern;
  };

  class ExactMatchOnlyFormat : public MatchOnlyFormatBase
  {
  public:
    typedef std::vector<uint8_t> PatternMatrix;

    ExactMatchOnlyFormat(PatternMatrix mtx, std::size_t offset, std::size_t minSize)
      : Offset(offset)
      , MinSize(std::max(minSize, mtx.size() + offset))
      , Pattern(std::move(mtx))
    {
    }

    bool Match(const Data& data) const override
    {
      if (data.Size() < MinSize)
      {
        return false;
      }
      const uint8_t* const patternStart = &Pattern.front();
      const uint8_t* const patternEnd = patternStart + Pattern.size();
      const uint8_t* const typedDataStart = static_cast<const uint8_t*>(data.Start()) + Offset;
      return std::equal(patternStart, patternEnd, typedDataStart);
    }

    static Ptr TryCreate(const FormatDSL::StaticPattern& pattern, std::size_t startOffset, std::size_t minSize)
    {
      const std::size_t patternSize = pattern.GetSize();
      PatternMatrix tmp(patternSize);
      for (std::size_t idx = 0; idx != patternSize; ++idx)
      {
        const FormatDSL::StaticPredicate& pred = pattern.Get(idx);
        if (const uint_t* single = pred.GetSingle())
        {
          tmp[idx] = *single;
        }
        else
        {
          return Ptr();
        }
      }
      return MakePtr<ExactMatchOnlyFormat>(std::move(tmp), startOffset, minSize);
    }
  private:
    const std::size_t Offset;
    const std::size_t MinSize;
    const PatternMatrix Pattern;
  };

  Format::Ptr CreateMatchingFormatFromPredicates(const FormatDSL::Expression& expr, std::size_t minSize)
  {
    FormatDSL::StaticPattern pattern(expr.Predicates());
    const std::size_t startOffset = expr.StartOffset();
    if (Format::Ptr exact = ExactMatchOnlyFormat::TryCreate(pattern, startOffset, minSize))
    {
      return exact;
    }
    else
    {
      return FuzzyMatchOnlyFormat::Create(std::move(pattern), startOffset, minSize);
    }
  }
}

namespace Binary
{
  Format::Ptr CreateMatchOnlyFormat(const std::string& pattern)
  {
    return CreateMatchOnlyFormat(pattern, 0);
  }

  Format::Ptr CreateMatchOnlyFormat(const std::string& pattern, std::size_t minSize)
  {
    const FormatDSL::Expression::Ptr expr = FormatDSL::Expression::Parse(pattern);
    return CreateMatchingFormatFromPredicates(*expr, minSize);
  }
}
