/**
* 
* @file
*
* @brief  ProSoundCreator support implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "prosoundcreator.h"
#include "formats/chiptune/chiptune_container.h"
//common includes
#include <byteorder.h>
#include <contract.h>
#include <indices.h>
#include <iterator.h>
#include <make_ptr.h>
#include <range_checker.h>
//library includes
#include <binary/format_factories.h>
#include <binary/typed_container.h>
#include <debug/log.h>
#include <strings/format.h>
#include <strings/optimize.h>
//std includes
#include <array>
#include <cctype>
#include <cstring>
//boost includes
#include <boost/bind.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
//text includes
#include <formats/text/chiptune.h>

namespace Formats
{
namespace Chiptune
{
  namespace ProSoundCreator
  {
    const Debug::Stream Dbg("Formats::Chiptune::ProSoundCreator");

    const std::size_t MIN_MODULE_SIZE = 256;
    const std::size_t MAX_MODULE_SIZE = 0x4200;
    const std::size_t MAX_SAMPLES_COUNT = 32;
    const std::size_t MAX_SAMPLE_SIZE = 32;
    const std::size_t MAX_ORNAMENTS_COUNT = 32;
    const std::size_t MAX_ORNAMENT_SIZE = 32;
    const std::size_t MIN_PATTERN_SIZE = 1;//???
    const std::size_t MAX_PATTERN_SIZE = 64;//???
    const std::size_t MAX_PATTERNS_COUNT = 32;
    const std::size_t MAX_POSITIONS_COUNT = 100;

    /*
      Typical module structure

      Header
      samples offsets
      ornaments offsets  <- Starts from Header.OrnamentsTableOffset
      <unused idx>       <- Header.SamplesStart points here
      sample0            <- Header.SamplesOffsets[0] points here (*)
      <unused idx>
      sample1
      ...
      ff
      <unused idx>
      ornament0          <- Header.OrnamentsOffsets[0] points here (*)
      <unused idx>
      ornament1
      ...
      ff
      patterns
      ff
      position0
      ...
      limiter

      samples/ornaments offsets pointed to raw data
      each sample/ornament preceeds of unused index

      * for ver > 3.xx, each sample/ornament offset is relative to offsets' table start
    */

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
    const uint8_t PSC_ID_0[] =
    {
      'P', 'S', 'C', ' ', 'V'
    };
    const uint8_t PSC_ID_1[] =
    {
      ' ', 'C', 'O', 'M', 'P', 'I', 'L', 'A', 'T', 'I', 'O', 'N', ' ', 'O', 'F', ' '
    };

    const Char BY_DELIMITER[] =
    {
      'B', 'Y', 0
    };

    PACK_PRE struct RawId
    {
      uint8_t Identifier1[5]; //'PSC V'
      std::array<char, 4> Version;        //x.xx
      uint8_t Identifier2[16];//' COMPILATION OF '
      std::array<char, 20> Title;
      std::array<char, 4> Identifier3;//' BY ' or smth similar
      std::array<char, 20> Author;

      bool Check() const
      {
        static_assert(sizeof(PSC_ID_0) == sizeof(Identifier1), "Invalid layout");
        static_assert(sizeof(PSC_ID_1) == sizeof(Identifier2), "Invalid layout");
        return 0 == std::memcmp(Identifier1, PSC_ID_0, sizeof(Identifier1))
            && 0 == std::memcmp(Identifier2, PSC_ID_1, sizeof(Identifier2));
      }

      bool HasAuthor() const
      {
        const auto trimId = boost::algorithm::trim_copy_if(StringView(Identifier3), boost::algorithm::is_from_range(' ', ' '));
        return boost::algorithm::iequals(trimId, BY_DELIMITER);
      }

      uint_t GetVersion() const
      {
        if (std::isdigit(Version[0])
            && Version[1] == '.'
            && std::isdigit(Version[2])
            && std::isdigit(Version[3]))
        {
          return 100 * (Version[0] - '0') + 10 * (Version[2] - '0') + (Version[3] - '0');
        }
        else
        {
          return 0;
        }
      }
    } PACK_POST;

    PACK_PRE struct RawHeader
    {
      RawId Id;
      uint16_t SamplesStart;
      uint16_t PositionsOffset;
      uint8_t Tempo;
      uint16_t OrnamentsTableOffset;
    } PACK_POST;

    const uint8_t END_POSITION_MARKER = 0xff;

    PACK_PRE struct RawPattern
    {
      uint8_t Index;
      uint8_t Size;
      std::array<uint16_t, 3> Offsets;//from start of patterns
    } PACK_POST;

    PACK_PRE struct LastRawPattern
    {
      uint8_t LoopPositionIndex;
      uint8_t Marker;
      uint16_t LoopPatternPtr;
    } PACK_POST;

    PACK_PRE struct RawOrnament
    {
      PACK_PRE struct Line
      {
        //BEFooooo
        uint8_t LoopAndNoiseOffset;
        //OOOOOOOO
        int8_t NoteOffset;

        //o - noise offset (signed)
        //B - loop begin
        //E - loop end
        //F - finished
        //O - note offset (signed)

        bool IsLoopBegin() const
        {
          return 0 == (LoopAndNoiseOffset & 128);
        }

        bool IsLoopEnd() const
        {
          return 0 == (LoopAndNoiseOffset & 64);
        }

        bool IsFinished() const
        {
          return 0 == (LoopAndNoiseOffset & 32);
        }

        uint_t GetNoiseOffset() const
        {
          return LoopAndNoiseOffset & 31;
        }

        int_t GetNoteOffset() const
        {
          return NoteOffset;
        }
      } PACK_POST;
      Line Data[1];
    } PACK_POST;

    PACK_PRE struct RawSample
    {
      PACK_PRE struct Line
      {
        uint16_t Tone;
        //signed
        int8_t Adding;
        //xxxxLLLL
        uint8_t Level;
        //BEFeNDUT
        //D - vol down
        //U - vol up
        //e - envelope
        //N - noise mask
        //B - loop begin(0)
        //E - loop end (0)
        //F - finish (0)
        //T - tone mask
        uint8_t Flags;
        uint8_t Padding;

        bool IsLoopBegin() const
        {
          return 0 == (Flags & 128);
        }

        bool IsLoopEnd() const
        {
          return 0 == (Flags & 64);
        }

        bool IsFinished() const
        {
          return 0 == (Flags & 32);
        }

        uint_t GetTone() const
        {
          return fromLE(Tone);
        }

        int_t GetAdding() const
        {
          return Adding;
        }

        uint_t GetLevel() const
        {
          return Level & 15;
        }

        bool GetNoiseMask() const
        {
          return 0 != (Flags & 8);
        }

        bool GetToneMask() const
        {
          return 0 != (Flags & 1);
        }

        bool GetEnableEnvelope() const
        {
          return 0 == (Flags & 16);
        }

        int_t GetVolumeDelta() const
        {
          return int_t(0 != (Flags & 2)) - int_t(0 != (Flags & 4));
        }
      } PACK_POST;
      Line Data[1];
    } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

    static_assert(sizeof(RawId) == 69, "Invalid layout");
    static_assert(sizeof(RawHeader) == sizeof(RawId) + 7, "Invalid layout");
    static_assert(sizeof(RawPattern) == 8, "Invalid layout");
    static_assert(sizeof(RawOrnament) == 2, "Invalid layout");
    static_assert(sizeof(RawSample) == 6, "Invalid layout");

    class StubBuilder : public Builder
    {
    public:
      MetaBuilder& GetMetaBuilder() override
      {
        return GetStubMetaBuilder();
      }
      void SetInitialTempo(uint_t /*tempo*/) override {}
      void SetSample(uint_t /*index*/, Sample /*sample*/) override {}
      void SetOrnament(uint_t /*index*/, Ornament /*ornament*/) override {}
      void SetPositions(Positions /*positions*/) override {}
      PatternBuilder& StartPattern(uint_t /*index*/) override
      {
        return GetStubPatternBuilder();
      }
      void StartChannel(uint_t /*index*/) override {}
      void SetRest() override {}
      void SetNote(uint_t /*note*/) override {}
      void SetSample(uint_t /*sample*/) override {}
      void SetOrnament(uint_t /*ornament*/) override {}
      void SetVolume(uint_t /*vol*/) override {}
      void SetEnvelope(uint_t /*type*/, uint_t /*tone*/) override {}
      void SetEnvelope() override {}
      void SetNoEnvelope() override {}
      void SetNoiseBase(uint_t /*val*/) override {}
      void SetBreakSample() override {}
      void SetBreakOrnament() override {}
      void SetNoOrnament() override {}
      void SetGliss(uint_t /*val*/) override {}
      void SetSlide(int_t /*steps*/) override {}
      void SetVolumeSlide(uint_t /*period*/, int_t /*delta*/) override {}
    };

    class StatisticCollectingBuilder : public Builder
    {
    public:
      explicit StatisticCollectingBuilder(Builder& delegate)
        : Delegate(delegate)
        , UsedPatterns(0, MAX_PATTERNS_COUNT - 1)
        , UsedSamples(0, MAX_SAMPLES_COUNT - 1)
        , UsedOrnaments(0, MAX_ORNAMENTS_COUNT - 1)
      {
        UsedSamples.Insert(0);
        UsedOrnaments.Insert(0);
      }

      MetaBuilder& GetMetaBuilder() override
      {
        return Delegate.GetMetaBuilder();
      }

      void SetInitialTempo(uint_t tempo) override
      {
        return Delegate.SetInitialTempo(tempo);
      }

      void SetSample(uint_t index, Sample sample) override
      {
        assert(UsedSamples.Contain(index));
        return Delegate.SetSample(index, std::move(sample));
      }

      void SetOrnament(uint_t index, Ornament ornament) override
      {
        assert(UsedOrnaments.Contain(index));
        return Delegate.SetOrnament(index, std::move(ornament));
      }

      void SetPositions(Positions positions) override
      {
        UsedPatterns.Assign(positions.Lines.begin(), positions.Lines.end());
        Require(!UsedPatterns.Empty());
        return Delegate.SetPositions(std::move(positions));
      }

      PatternBuilder& StartPattern(uint_t index) override
      {
        assert(UsedPatterns.Contain(index));
        return Delegate.StartPattern(index);
      }

      void StartChannel(uint_t index) override
      {
        return Delegate.StartChannel(index);
      }

      void SetRest() override
      {
        return Delegate.SetRest();
      }

      void SetNote(uint_t note) override
      {
        return Delegate.SetNote(note);
      }

      void SetSample(uint_t sample) override
      {
        UsedSamples.Insert(sample);
        return Delegate.SetSample(sample);
      }

      void SetOrnament(uint_t ornament) override
      {
        UsedOrnaments.Insert(ornament);
        return Delegate.SetOrnament(ornament);
      }

      void SetVolume(uint_t vol) override
      {
        return Delegate.SetVolume(vol);
      }

      void SetEnvelope(uint_t type, uint_t tone) override
      {
        return Delegate.SetEnvelope(type, tone);
      }

      void SetEnvelope() override
      {
        return Delegate.SetEnvelope();
      }

      void SetNoEnvelope() override
      {
        return Delegate.SetNoEnvelope();
      }

      void SetNoiseBase(uint_t val) override
      {
        return Delegate.SetNoiseBase(val);
      }

      void SetBreakSample() override
      {
        return Delegate.SetBreakSample();
      }

      void SetBreakOrnament() override
      {
        return Delegate.SetBreakOrnament();
      }

      void SetNoOrnament() override
      {
        return Delegate.SetNoOrnament();
      }

      void SetGliss(uint_t absStep) override
      {
        return Delegate.SetGliss(absStep);
      }

      void SetSlide(int_t delta) override
      {
        return Delegate.SetSlide(delta);
      }

      void SetVolumeSlide(uint_t period, int_t delta) override
      {
        return Delegate.SetVolumeSlide(period, delta);
      }

      const Indices& GetUsedPatterns() const
      {
        return UsedPatterns;
      }

      const Indices& GetUsedSamples() const
      {
        return UsedSamples;
      }

      const Indices& GetUsedOrnaments() const
      {
        return UsedOrnaments;
      }
    private:
      Builder& Delegate;
      Indices UsedPatterns;
      Indices UsedSamples;
      Indices UsedOrnaments;
    };

    class RangesMap
    {
    public:
      explicit RangesMap(std::size_t limit)
        : ServiceRanges(RangeChecker::CreateShared(limit))
        , TotalRanges(RangeChecker::CreateSimple(limit))
        , FixedRanges(RangeChecker::CreateSimple(limit))
      {
      }

      void AddService(std::size_t offset, std::size_t size) const
      {
        Require(ServiceRanges->AddRange(offset, size));
        Add(offset, size);
      }

      void AddFixed(std::size_t offset, std::size_t size) const
      {
        Require(FixedRanges->AddRange(offset, size));
        Add(offset, size);
      }

      void Add(std::size_t offset, std::size_t size) const
      {
        Dbg(" Affected range %1%..%2%", offset, offset + size);
        Require(TotalRanges->AddRange(offset, size));
      }

      std::size_t GetSize() const
      {
        return TotalRanges->GetAffectedRange().second;
      }

      RangeChecker::Range GetFixedArea() const
      {
        return FixedRanges->GetAffectedRange();
      }
    private:
      const RangeChecker::Ptr ServiceRanges;
      const RangeChecker::Ptr TotalRanges;
      const RangeChecker::Ptr FixedRanges;
    };

    struct Traits
    {
      String Program;
      std::size_t OrnamentsBase;
      std::size_t SamplesBase;
    };

    Traits GetOldVersionTraits(const RawHeader& hdr)
    {
      const String programName = hdr.Id.Check()
        ? Strings::Format(Text::PROSOUNDCREATOR_EDITOR, StringView(hdr.Id.Version))
        : Text::PROSOUNDCREATOR_EDITOR_OLD;
      const Traits res = {programName, 0, 0};
      return res;
    }

    Traits GetNewVersionTraits(const RawHeader& hdr)
    {
      const String programName = hdr.Id.Check()
        ? Strings::Format(Text::PROSOUNDCREATOR_EDITOR, StringView(hdr.Id.Version))
        : Text::PROSOUNDCREATOR_EDITOR_NEW;
      const Traits res = {programName, fromLE(hdr.OrnamentsTableOffset), sizeof(hdr)};
      return res;
    }

    Traits GetTraits(const RawHeader& hdr)
    {
      const uint_t vers = hdr.Id.GetVersion();
      if (vers == 0 || vers >= 103)
      {
        return GetNewVersionTraits(hdr);
      }
      else
      {
        return GetOldVersionTraits(hdr);
      }
    }

    class PatternsSet
    {
    public:
      uint_t Add(const RawPattern& pat)
      {
        const ContainerType::const_iterator begin = Container.begin(), end = Container.end();
        const ContainerType::const_iterator it = std::find_if(begin, end,
          boost::bind(&ArePatternsEqual, pat, _1));
        if (it != end)
        {
          return std::distance(begin, it);
        }
        const std::size_t newIdx = Container.size();
        RawPattern toStore(pat);
        toStore.Index = newIdx;
        Container.push_back(toStore);
        return newIdx;
      }

      typedef std::vector<RawPattern> ContainerType;

      typedef RangeIterator<ContainerType::const_iterator> Iterator;

      Iterator Get() const
      {
        return Iterator(Container.begin(), Container.end());
      }
    private:
      static bool ArePatternsEqual(const RawPattern& lh, const RawPattern& rh)
      {
        return lh.Size == rh.Size
            && lh.Offsets[0] == rh.Offsets[0]
            && lh.Offsets[1] == rh.Offsets[1]
            && lh.Offsets[2] == rh.Offsets[2]
            ;
      }
      ContainerType Container;
    };

    class Format
    {
    public:
      Format(const Binary::TypedContainer& data)
        : Delegate(data)
        , Ranges(data.GetSize())
        , Source(GetServiceObject<RawHeader>(0))
        , Trait(GetTraits(Source))
      {
      }

      void ParseCommonProperties(Builder& builder) const
      {
        const uint_t tempo = Source.Tempo;
        builder.SetInitialTempo(tempo);
        MetaBuilder& meta = builder.GetMetaBuilder();
        meta.SetProgram(Trait.Program);
        if (Source.Id.Check())
        {
          if (Source.Id.HasAuthor())
          {
            meta.SetTitle(Strings::OptimizeAscii(Source.Id.Title));
            meta.SetAuthor(Strings::OptimizeAscii(Source.Id.Author));
          }
          else
          {
            meta.SetTitle(Strings::OptimizeAscii(StringView(Source.Id.Title.data(), &Source.Id.Author.back() + 1)));
          }
        }
      }

      PatternsSet ParsePositions(Builder& builder) const
      {
        PatternsSet patterns;
        Positions positions;
        for (std::size_t offset = fromLE(Source.PositionsOffset);; offset += sizeof(RawPattern))
        {
          const LastRawPattern* const last = Delegate.GetField<LastRawPattern>(offset);
          Require(last != nullptr);
          if (last->Marker == END_POSITION_MARKER)
          {
            const std::size_t tailSize = std::min(Delegate.GetSize() - offset, sizeof(RawPattern));
            Ranges.AddService(offset, tailSize);
            positions.Loop = std::min<uint_t>(last->LoopPositionIndex, positions.GetSize() - 1);
            break;
          }
          const RawPattern& pat = GetServiceObject<RawPattern>(offset);
          const uint_t patIndex = patterns.Add(pat);
          positions.Lines.push_back(patIndex);
        }
        Dbg("Positions: %1% entries, loop to %2%", positions.GetSize(), positions.GetLoop());
        builder.SetPositions(std::move(positions));
        return patterns;
      }

      void ParsePatterns(const PatternsSet& patterns, Builder& builder) const
      {
        bool hasValidPatterns = false;
        for (PatternsSet::Iterator it = patterns.Get(); it; ++it)
        {
          const RawPattern& pat = *it;
          Dbg("Parse pattern %1%", pat.Index);
          if (ParsePattern(pat, builder))
          {
            hasValidPatterns = true;
          }
        }
        Require(hasValidPatterns);
      }

      void ParseSamples(const Indices& samples, Builder& builder) const
      {
        Dbg("Samples: %1% to parse", samples.Count());
        const std::size_t samplesTableStart = sizeof(RawHeader);
        for (Indices::Iterator it = samples.Items(); it; ++it)
        {
          const uint_t samIdx = *it;
          Dbg("Parse sample %1%", samIdx);
          const std::size_t offsetAddr = samplesTableStart + samIdx * sizeof(uint16_t);
          const std::size_t sampleAddr = Trait.SamplesBase + fromLE(GetServiceObject<uint16_t>(offsetAddr));
          builder.SetSample(samIdx, ParseSample(sampleAddr));
        }
      }

      void ParseOrnaments(const Indices& ornaments, Builder& builder) const
      {
        Dbg("Ornaments: %1% to parse", ornaments.Count());
        //Some of the modules (e.g. Story Map.psc) references more ornaments than really stored
        const std::size_t ornamentsTableStart = fromLE(Source.OrnamentsTableOffset);
        const std::size_t ornamentsTableEnd = fromLE(Source.SamplesStart);
        const std::size_t maxOrnaments = (ornamentsTableEnd - ornamentsTableStart) / sizeof(uint16_t);
        for (Indices::Iterator it = ornaments.Items(); it; ++it)
        {
          const uint_t ornIdx = *it;
          if (ornIdx < maxOrnaments)
          {
            Dbg("Parse ornament %1%", ornIdx);
            const std::size_t offsetAddr = ornamentsTableStart + ornIdx * sizeof(uint16_t);
            const std::size_t ornamentAddr = Trait.OrnamentsBase + fromLE(GetServiceObject<uint16_t>(offsetAddr));
            builder.SetOrnament(ornIdx, ParseOrnament(ornamentAddr));
          }
          else
          {
            Dbg("Parse stub ornament %1%", ornIdx);
            builder.SetOrnament(ornIdx, Ornament());
          }
        }
      }

      std::size_t GetSize() const
      {
        return Ranges.GetSize();
      }

      RangeChecker::Range GetFixedArea() const
      {
        return Ranges.GetFixedArea();
      }
    private:
      template<class T>
      const T& GetObject(std::size_t offset) const
      {
        const T* const src = Delegate.GetField<T>(offset);
        Require(src != nullptr);
        Ranges.Add(offset, sizeof(T));
        return *src;
      }

      template<class T>
      const T& GetServiceObject(std::size_t offset) const
      {
        const T* const src = Delegate.GetField<T>(offset);
        Require(src != nullptr);
        Ranges.AddService(offset, sizeof(T));
        return *src;
      }

      uint8_t PeekByte(std::size_t offset) const
      {
        const uint8_t* const data = Delegate.GetField<uint8_t>(offset);
        Require(data != nullptr);
        return *data;
      }

      struct DataCursors : std::array<std::size_t, 3>
      {
        explicit DataCursors(const RawPattern& pat)
        {
          std::transform(pat.Offsets.begin(), pat.Offsets.end(), begin(), &fromLE<uint16_t>);
        }
      };

      struct ParserState
      {
        struct ChannelState
        {
          std::size_t Offset;
          uint_t Period;
          uint_t Counter;

          ChannelState()
            : Offset()
            , Period()
            , Counter()
          {
          }

          void Skip(uint_t toSkip)
          {
            Counter -= toSkip;
          }


          static bool CompareByCounter(const ChannelState& lh, const ChannelState& rh)
          {
            return lh.Counter < rh.Counter;
          }
        };

        std::array<ChannelState, 3> Channels;

        explicit ParserState(const DataCursors& src)
          : Channels()
        {
          for (std::size_t idx = 0; idx != src.size(); ++idx)
          {
            Channels[idx].Offset = src[idx];
          }
        }

        uint_t GetMinCounter() const
        {
          return std::min_element(Channels.begin(), Channels.end(), &ChannelState::CompareByCounter)->Counter;
        }

        void SkipLines(uint_t toSkip)
        {
          std::for_each(Channels.begin(), Channels.end(), std::bind2nd(std::mem_fun_ref(&ChannelState::Skip), toSkip));
        }
      };

      bool ParsePattern(const RawPattern& pat, Builder& builder) const
      {
        PatternBuilder& patBuilder = builder.StartPattern(pat.Index);
        const DataCursors rangesStarts(pat);
        ParserState state(rangesStarts);
        uint_t lineIdx = 0;
        for (; lineIdx < pat.Size; ++lineIdx)
        {
          //skip lines if required
          if (const uint_t linesToSkip = state.GetMinCounter())
          {
            state.SkipLines(linesToSkip);
            lineIdx += linesToSkip - 1;
          }
          else
          {
            patBuilder.StartLine(lineIdx);
            ParseLine(state, patBuilder, builder);
          }
        }
        patBuilder.Finish(pat.Size);
        for (uint_t chanNum = 0; chanNum != rangesStarts.size(); ++chanNum)
        {
          const std::size_t start = rangesStarts[chanNum];
          if (start >= Delegate.GetSize())
          {
            Dbg("Invalid offset (%1%)", start);
          }
          else
          {
            const std::size_t stop = std::min(Delegate.GetSize(), state.Channels[chanNum].Offset + 1);
            Ranges.AddFixed(start, stop - start);
          }
        }
        return lineIdx >= MIN_PATTERN_SIZE;
      }

      void ParseLine(ParserState& src, PatternBuilder& patBuilder, Builder& builder) const
      {
        for (uint_t chan = 0; chan < 3; ++chan)
        {
          ParserState::ChannelState& state = src.Channels[chan];
          if (state.Counter)
          {
            --state.Counter;
            continue;
          }
          builder.StartChannel(chan);
          ParseChannel(chan, state, patBuilder, builder);
          state.Counter = state.Period;
        }
      }

      void ParseChannel(uint_t chan, ParserState::ChannelState& state, PatternBuilder& patBuilder, Builder& builder) const
      {
        while (state.Offset < Delegate.GetSize())
        {
          const uint_t cmd = PeekByte(state.Offset++);
          if (cmd >= 0xc0) //0xc0..0xff
          {
            state.Period = cmd - 0xc0;
            break;
          }
          else if (cmd >= 0xa0) //0xa0..0xbf
          {
            builder.SetOrnament(cmd - 0xa0);
          }
          else if (cmd >= 0x80) //0x7e..0x9f
          {
            builder.SetSample(cmd - 0x80);
          }
          //0x7e,0x7f
          else if (cmd == 0x7d)
          {
            builder.SetBreakSample();
          }
          else if (cmd == 0x7c)
          {
            builder.SetRest();
          }
          else if (cmd == 0x7b)
          {
            if (chan == 1)//only for B
            {
              const uint_t noise = PeekByte(state.Offset++);
              builder.SetNoiseBase(noise);
            }
          }
          else if (cmd == 0x7a)
          {
            if (chan == 1)//only for B
            {
              const uint_t envType = PeekByte(state.Offset++) & 15;
              const uint_t envTone = (uint_t(PeekByte(state.Offset + 1)) << 8) | PeekByte(state.Offset);
              builder.SetEnvelope(envType, envTone);
              state.Offset += 2;
            }
          }
          else if (cmd == 0x71)
          {
            builder.SetBreakOrnament();
            ++state.Offset;
          }
          else if (cmd == 0x70)
          {
            const uint8_t val = PeekByte(state.Offset++);
            const uint_t period = 0 != (val & 64) ? -static_cast<int8_t>(val | 128) : val;
            const int_t step = 0 != (val & 64) ? -1 : +1;
            builder.SetVolumeSlide(period, step);
          }
          else if (cmd == 0x6f)
          {
            builder.SetNoOrnament();
            ++state.Offset;
          }
          else if (cmd == 0x6e)
          {
            const uint_t tempo = PeekByte(state.Offset++);
            patBuilder.SetTempo(tempo);
          }
          else if (cmd == 0x6d)
          {
            builder.SetGliss(PeekByte(state.Offset++));
          }
          else if (cmd == 0x6c)
          {
            builder.SetSlide(-static_cast<int8_t>(PeekByte(state.Offset++)));
          }
          else if (cmd == 0x6b)
          {
            builder.SetSlide(PeekByte(state.Offset++));
          }
          else if (cmd >= 0x58 && cmd <= 0x66)
          {
            builder.SetVolume(cmd - 0x57);
            builder.SetNoEnvelope();
          }
          else if (cmd == 0x57)
          {
            builder.SetVolume(0xf);
            builder.SetEnvelope();
          }
          else if (cmd <= 0x56)
          {
            builder.SetNote(cmd);
          }
        }
      }

      Sample ParseSample(std::size_t offset) const
      {
        const RawSample& src = GetObject<RawSample>(offset);
        Sample result;
        const std::size_t availSize = (Delegate.GetSize() - offset) / sizeof(RawSample::Line);
        for (std::size_t idx = 0, lim = std::min(availSize, MAX_SAMPLE_SIZE); idx != lim; ++idx)
        {
          const RawSample::Line& srcLine = src.Data[idx];
          const Sample::Line& dstLine = ParseSampleLine(srcLine);
          result.Lines.push_back(dstLine);
          if (srcLine.IsFinished())
          {
            break;
          }
        }
        Ranges.Add(offset, result.Lines.size() * sizeof(RawSample::Line));
        return result;
      }

      static Sample::Line ParseSampleLine(const RawSample::Line& src)
      {
        Sample::Line result;
        result.Level = src.GetLevel();
        result.Tone = src.GetTone();
        result.ToneMask = src.GetToneMask();
        result.NoiseMask = src.GetNoiseMask();
        result.Adding = src.GetAdding();
        result.EnableEnvelope = src.GetEnableEnvelope();
        result.VolumeDelta = src.GetVolumeDelta();
        result.LoopBegin = src.IsLoopBegin();
        result.LoopEnd = src.IsLoopEnd();
        return result;
      }

      Ornament ParseOrnament(std::size_t offset) const
      {
        const RawOrnament& src = GetObject<RawOrnament>(offset);
        Ornament result;
        const std::size_t availSize = (Delegate.GetSize() - offset) / sizeof(RawOrnament::Line);
        for (std::size_t idx = 0, lim = std::min(availSize, MAX_ORNAMENT_SIZE); idx != lim; ++idx)
        {
          const RawOrnament::Line& srcLine = src.Data[idx];
          const Ornament::Line& dstLine = ParseOrnamentLine(srcLine);
          result.Lines.push_back(dstLine);
          if (srcLine.IsFinished())
          {
            break;
          }
        }
        Ranges.Add(offset, result.Lines.size() * sizeof(RawOrnament::Line));
        return result;
      }

      static Ornament::Line ParseOrnamentLine(const RawOrnament::Line& src)
      {
        Ornament::Line result;
        result.NoteAddon = src.GetNoteOffset();
        result.NoiseAddon = src.GetNoiseOffset();
        result.LoopBegin = src.IsLoopBegin();
        result.LoopEnd = src.IsLoopEnd();
        return result;
      }
    private:
      const Binary::TypedContainer& Delegate;
      RangesMap Ranges;
      const RawHeader& Source;
      const Traits Trait;
    };

    enum AreaTypes
    {
      HEADER,
      SAMPLES_OFFSETS,
      ORNAMENTS_OFFSETS,
      SAMPLES,
      ORNAMENTS,
      PATTERNS,
      POSITIONS,
      END
    };

    struct Areas : public AreaController
    {
      Areas(const Binary::TypedContainer& data)
      {
        const RawHeader& header = *data.GetField<RawHeader>(0);
        const Traits traits = GetTraits(header);
        const std::size_t samplesOffsets = sizeof(header);
        const std::size_t ornamentsOffsets = fromLE(header.OrnamentsTableOffset);
        const std::size_t positionsOffset = fromLE(header.PositionsOffset);
        AddArea(HEADER, 0);
        AddArea(SAMPLES_OFFSETS, samplesOffsets);
        AddArea(ORNAMENTS_OFFSETS, ornamentsOffsets);
        AddArea(POSITIONS, positionsOffset);
        AddArea(END, data.GetSize());
        if (const uint16_t* firstSample = data.GetField<uint16_t>(samplesOffsets))
        {
          const std::size_t firstSampleStart = fromLE(*firstSample) + traits.SamplesBase;
          if (firstSampleStart == std::size_t(fromLE(header.SamplesStart) + 1))
          {
            AddArea(SAMPLES, firstSampleStart);
          }
        }
        if (const uint16_t* firstOrnament = data.GetField<uint16_t>(ornamentsOffsets))
        {
          AddArea(ORNAMENTS, fromLE(*firstOrnament) + traits.OrnamentsBase);
        }
        if (const RawPattern* firstPosition = data.GetField<RawPattern>(positionsOffset))
        {
          AddArea(PATTERNS, std::min(fromLE(firstPosition->Offsets[0]), std::min(fromLE(firstPosition->Offsets[1]), fromLE(firstPosition->Offsets[2]))));
        }
      }

      bool CheckHeader() const
      {
        if (sizeof(RawHeader) != GetAreaSize(HEADER) || Undefined != GetAreaSize(END))
        {
          return false;
        }
        return GetAreaAddress(SAMPLES_OFFSETS) < GetAreaAddress(ORNAMENTS_OFFSETS)
            && GetAreaAddress(SAMPLES) < GetAreaAddress(ORNAMENTS);
      }

      bool CheckSamples() const
      {
        const std::size_t offsetsSize = GetAreaSize(SAMPLES_OFFSETS);
        if (offsetsSize == Undefined || offsetsSize < sizeof(uint16_t) || 0 != offsetsSize % sizeof(uint16_t))
        {
          return false;
        }
        const std::size_t samplesSize = GetAreaSize(SAMPLES);
        if (samplesSize == Undefined || samplesSize < sizeof(RawSample))
        {
          return false;
        }
        return true;
      }

      bool CheckOrnaments() const
      {
        const std::size_t offsetsSize = GetAreaSize(ORNAMENTS_OFFSETS);
        if (offsetsSize == Undefined || offsetsSize < sizeof(uint16_t))
        {
          return false;
        }
        const std::size_t samplesSize = GetAreaSize(ORNAMENTS);
        if (samplesSize == Undefined || samplesSize < sizeof(RawOrnament))
        {
          return false;
        }
        return true;
      }

      bool CheckPositions() const
      {
        const std::size_t size = GetAreaSize(POSITIONS);
        return size != Undefined && size >= sizeof(RawPattern) + sizeof(LastRawPattern);
      }

      bool CheckPatterns() const
      {
        const std::size_t size = GetAreaSize(PATTERNS);
        return size != Undefined;
      }
    };

    bool FastCheck(const Binary::TypedContainer& data)
    {
      const Areas areas(data);
      if (!areas.CheckHeader())
      {
        return false;
      }
      if (!areas.CheckSamples())
      {
        return false;
      }
      //TODO: check if samples offsets are sequenced
      if (!areas.CheckOrnaments())
      {
        return false;
      }
      //TODO: check if ornaments offsets are sequenced
      if (!areas.CheckPositions())
      {
        return false;
      }
      if (!areas.CheckPatterns())
      {
        return false;
      }
      return true;
    }

    Binary::TypedContainer CreateContainer(const Binary::Container& data)
    {
      return Binary::TypedContainer(data, std::min(data.Size(), MAX_MODULE_SIZE));
    }

    bool FastCheck(const Binary::Container& rawData)
    {
      const Binary::TypedContainer data = CreateContainer(rawData);
      return FastCheck(data);
    }

    const std::string FORMAT(
      "?{69}"   //Id
      "?00"     //uint16_t SamplesStart;TODO
      "?03-3f"  //uint16_t PositionsOffset;
      "03-1f"   //uint8_t Tempo;
      "50-9000" //uint16_t OrnamentsTableOffset;
      "08-cf00" //first sample
    );

    class Decoder : public Formats::Chiptune::Decoder
    {
    public:
      Decoder()
        : Format(Binary::CreateFormat(FORMAT, MIN_MODULE_SIZE))
      {
      }

      String GetDescription() const override
      {
        return Text::PROSOUNDCREATOR_DECODER_DESCRIPTION;
      }

      Binary::Format::Ptr GetFormat() const override
      {
        return Format;
      }

      bool Check(const Binary::Container& rawData) const override
      {
        return Format->Match(rawData) && FastCheck(CreateContainer(rawData));
      }

      Formats::Chiptune::Container::Ptr Decode(const Binary::Container& rawData) const override
      {
        if (!Format->Match(rawData))
        {
          return Formats::Chiptune::Container::Ptr();
        }
        Builder& stub = GetStubBuilder();
        return Parse(rawData, stub);
      }
    private:
      const Binary::Format::Ptr Format;
    };

    Formats::Chiptune::Container::Ptr Parse(const Binary::Container& rawData, Builder& target)
    {
      const Binary::TypedContainer data = CreateContainer(rawData);

      if (!FastCheck(data))
      {
        return Formats::Chiptune::Container::Ptr();
      }

      try
      {
        const Format format(data);
        format.ParseCommonProperties(target);

        StatisticCollectingBuilder statistic(target);
        const PatternsSet& usedPatterns = format.ParsePositions(statistic);
        format.ParsePatterns(usedPatterns, statistic);
        const Indices& usedSamples = statistic.GetUsedSamples();
        format.ParseSamples(usedSamples, target);
        const Indices& usedOrnaments = statistic.GetUsedOrnaments();
        format.ParseOrnaments(usedOrnaments, target);

        Require(format.GetSize() >= MIN_MODULE_SIZE);
        const Binary::Container::Ptr subData = rawData.GetSubcontainer(0, format.GetSize());
        const RangeChecker::Range fixedRange = format.GetFixedArea();
        return CreateCalculatingCrcContainer(subData, fixedRange.first, fixedRange.second - fixedRange.first);
      }
      catch (const std::exception&)
      {
        Dbg("Failed to create");
        return Formats::Chiptune::Container::Ptr();
      }
    }

    Builder& GetStubBuilder()
    {
      static StubBuilder stub;
      return stub;
    }
  }//namespace ProSoundCreator

  Decoder::Ptr CreateProSoundCreatorDecoder()
  {
    return MakePtr<ProSoundCreator::Decoder>();
  }
}//namespace Chiptune
}//namespace Formats
