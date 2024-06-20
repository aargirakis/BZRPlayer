/**
*
* @file
*
* @brief  Format detector implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "details.h"
#include "static_expression.h"
//common includes
#include <contract.h>
#include <make_ptr.h>
//library includes
#include <binary/format_factories.h>
#include <math/numeric.h>
//std includes
#include <array>
#include <limits>
#include <vector>

namespace Binary
{
  class FuzzyFormat : public FormatDetails
  {
  public:
    typedef std::array<uint8_t, 256> PatternRow;
    typedef std::vector<PatternRow> PatternMatrix;

    FuzzyFormat(PatternMatrix mtx, std::size_t offset, std::size_t minSize, std::size_t minScanStep)
      : Offset(offset)
      , MinSize(std::max(minSize, mtx.size() + offset))
      , MinScanStep(minScanStep)
      , Pat(std::move(mtx))
      , PatRBegin(&Pat.back())
      , PatREnd(&Pat.front() - 1)
    {
    }

    bool Match(const Data& data) const override
    {
      if (data.Size() < MinSize)
      {
        return false;
      }
      const std::size_t endOfPat = Offset + Pat.size();
      const uint8_t* typedDataLast = static_cast<const uint8_t*>(data.Start()) + endOfPat - 1;
      return 0 == SearchBackward(typedDataLast);
    }

    std::size_t NextMatchOffset(const Data& data) const override
    {
      const std::size_t size = data.Size();
      if (size < MinSize)
      {
        return size;
      }
      const uint8_t* const typedData = static_cast<const uint8_t*>(data.Start());
      const std::size_t endOfPat = Offset + Pat.size();
      const uint8_t* const scanStart = typedData + endOfPat - 1;
      const uint8_t* const scanStop = typedData + size;
      const std::size_t firstMatch = SearchBackward(scanStart);
      const std::size_t initialOffset = firstMatch != 0 ? firstMatch : MinScanStep;
      for (const uint8_t* scanPos = scanStart + initialOffset; scanPos < scanStop; )
      {
        if (const std::size_t offset = SearchBackward(scanPos))
        {
          scanPos += offset;
        }
        else
        {
          return scanPos - scanStart;
        }
      }
      return size;
    }

    std::size_t GetMinSize() const override
    {
      return MinSize;
    }

    static Ptr Create(const FormatDSL::StaticPattern& pattern, std::size_t startOffset, std::size_t minSize)
    {
      const std::size_t patternSize = pattern.GetSize();
      PatternMatrix tmp(patternSize);
      for (uint_t sym = 0; sym != 256; ++sym)
      {
        for (std::size_t pos = 0, offset = 1; pos != patternSize; ++pos, ++offset)
        {
          const FormatDSL::StaticPredicate& pred = pattern.Get(pos);
          if (pred.Match(sym))
          {
            offset = 0;
          }
          else
          {
            Require(Math::InRange<std::size_t>(offset, 0, std::numeric_limits<PatternRow::value_type>::max()));
            tmp[pos][sym] = static_cast<PatternRow::value_type>(offset);
          }
        }
      }
      /*
       Increase offset using suffixes.
       Average offsets (standard big file test):
         without increasing: 2.17
         with increasing: 2.27
         with precise increasing: 2.28

       Due to high complexity of precise detection, simple increasing is used
      */
      const auto& offsets = pattern.GetSuffixOffsets();
      for (std::size_t pos = 0; pos != patternSize - 1; ++pos)
      {
        FuzzyFormat::PatternRow& row = tmp[pos];
        const std::size_t suffixLen = patternSize - pos - 1;
        const std::size_t offset = offsets[suffixLen];
        const std::size_t availOffset = std::min<std::size_t>(offset, std::numeric_limits<PatternRow::value_type>::max());
        for (uint_t sym = 0; sym != 256; ++sym)
        {
          if (uint8_t& curOffset = row[sym])
          {
            curOffset = std::max(curOffset, static_cast<PatternRow::value_type>(availOffset));
          }
        }
      }
      //Each matrix element specifies forward movement of reversily matched pattern for specified symbol. =0 means symbol match
      const std::size_t minScanStep = pattern.FindPrefix(patternSize);
      return MakePtr<FuzzyFormat>(std::move(tmp), startOffset, minSize, minScanStep);
    }
  private:
    std::size_t SearchBackward(const uint8_t* data) const
    {
      auto it = PatRBegin;
      if (const std::size_t offset = (*it)[*data])
      {
        return offset;
      }
      --it;
      --data;
      for (; it != PatREnd; --it, --data)
      {
        if (const std::size_t offset = (*it)[*data])
        {
          return offset;
        }
      }
      return 0;
    }
  private:
    const std::size_t Offset;
    const std::size_t MinSize;
    const std::size_t MinScanStep;
    const PatternMatrix Pat;
    const PatternRow* const PatRBegin;
    const PatternRow* const PatREnd;
  };

  class ExactFormat : public FormatDetails
  {
  public:
    typedef std::vector<uint8_t> PatternMatrix;

    ExactFormat(PatternMatrix mtx, std::size_t offset, std::size_t minSize)
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

    std::size_t NextMatchOffset(const Data& data) const override
    {
      const std::size_t size = data.Size();
      if (size < MinSize)
      {
        return size;
      }
      const uint8_t* const patternStart = &Pattern.front();
      const uint8_t* const patternEnd = patternStart + Pattern.size();
      const uint8_t* const typedDataStart = static_cast<const uint8_t*>(data.Start());
      const uint8_t* const typedDataEnd = typedDataStart + size;
      const uint8_t* const matched = std::search(typedDataStart + Offset + 1, typedDataEnd, patternStart, patternEnd);
      return matched != typedDataEnd
        ? matched - typedDataStart - Offset
        : size;
    }

    std::size_t GetMinSize() const override
    {
      return MinSize;
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
      return MakePtr<ExactFormat>(std::move(tmp), startOffset, minSize);
    }
  private:
    const std::size_t Offset;
    const std::size_t MinSize;
    const PatternMatrix Pattern;
  };

  Format::Ptr CreateScanningFormatFromPredicates(const FormatDSL::Expression& expr, std::size_t minSize)
  {
    const FormatDSL::StaticPattern pattern(expr.Predicates());
    const std::size_t startOffset = expr.StartOffset();
    if (Format::Ptr exact = ExactFormat::TryCreate(pattern, startOffset, minSize))
    {
      return exact;
    }
    else
    {
      return FuzzyFormat::Create(pattern, startOffset, minSize);
    }
  }
}

namespace Binary
{
  Format::Ptr CreateFormat(const std::string& pattern)
  {
    return CreateFormat(pattern, 0);
  }

  Format::Ptr CreateFormat(const std::string& pattern, std::size_t minSize)
  {
    const FormatDSL::Expression::Ptr expr = FormatDSL::Expression::Parse(pattern);
    return CreateScanningFormatFromPredicates(*expr, minSize);
  }
}
