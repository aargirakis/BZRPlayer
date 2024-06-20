/**
 * \file YmFilters.h
 * \brief libstsound Audio Filter declaration file
 */

/*
 * libstsound is an AY-3-8192, YM2149 and clone PSG emulator
 * Copyright (C) 2008-2011 CPCSDK crew ( http://github.com/cpcsdk/ )
 * Based on ST-Sound
 * Copyright (C) 1995-1999 Arnaud Carre ( http://leonard.oxg.free.fr )
 */

#ifndef __FILTER_H_
#define __FILTER_H_

#include "YmTypes.h"

static const ymint DC_ADJUST_BUFFERLEN = 512;

/**
 * \class Filter
 * \brief Interface class for implementing audio filter
 */
class Filter
{
    public:
        Filter();
        virtual void AddSample(ymint sample) = 0;
        virtual ymint GetResult() = 0;
        virtual void Reset() = 0;
};

/**
 * \class SimpleLowPassFilter
 * \brief A very simple FIR 3-tap low pass filter.
 */
class SimpleLowPassFilter: public Filter
{
    public:
        SimpleLowPassFilter();
        void AddSample(ymint sample);
        ymint GetResult();
        void Reset();

    private:
        ymint delay_line[3]; // Delay lines for low pass filtering. 3 samples.
};

/**
 * \class DCRemover
 * \brief Removing of DC component
 * Implement has a simple average FIR DC_ADJUST_BUFFERLEN-tap high pass filter.
 */
class DCRemover: public Filter
{
    public:
        DCRemover();
        void AddSample(ymint sample);
        ymint GetResult();
        void Reset();

    private:
        ymint buffer[DC_ADJUST_BUFFERLEN];
        ymint pos;
        ymint sum;
#ifndef YM_SIMPLIFIED_FILTER
        bool full;
#endif
};

#if 0
class FIRFilter: public Filter
{
    public:
        FIRFilter();
        FIRFilter(ymint N = 512, ymint delay = 0, ymint *coef);
        ~FIRFilter();
        void AddSample(ymint sample);
        ymint GetResult();
        void Reset();

    privte:
        ymint N; // N-tap
        ymint delay;
        ymint pos;
        ymint *buffer;
        ymint *delay_line;
        ymfloat *coef;
        ymfloat coef_sum;
}
#endif // Unfinished

typedef struct _stereoSample
{
    ymint r;
    ymint l;
} stereoSample;

/**
 * \class FilterStereo
 * \brief Interface class for implementing audio filter who don't use stereo as two indepandant channels
 */
class FilterStereo
{
    public:
        FilterStereo();
        virtual void AddSample(stereoSample sample) = 0;
        virtual stereoSample GetResult() = 0;
        virtual void Reset() = 0;
};

/**
 * \class SimpleStereoEffectReducer
 * \brief A simple reducer of stereo effect, very usefull for headphone...
 */
class SimpleStereoEffectReducer: public FilterStereo
{
    public:
        SimpleStereoEffectReducer();
        void AddSample(stereoSample sample);
        stereoSample GetResult();
        void Reset();
    private:
        stereoSample buffer;
};

#endif /* #ifndef __FILTER_H_ */
