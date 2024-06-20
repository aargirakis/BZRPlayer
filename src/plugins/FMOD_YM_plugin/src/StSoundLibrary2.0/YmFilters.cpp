/**
 * \file YmFilters.cpp
 * \brief libstsound Audio Filter implementation file
 * This file implement some audio filter used for improve audio output quality
 */

/*
 * libstsound is an AY-3-8192, YM2149 and clone PSG emulator
 * Copyright (C) 2008-2011 CPCSDK crew ( http://github.com/cpcsdk/ )
 * Based on ST-Sound
 * Copyright (C) 1995-1999 Arnaud Carre ( http://leonard.oxg.free.fr )
 */

#include <cstring>

#include "YmFilters.h"

Filter::Filter()
{
}

DCRemover::DCRemover()
{
    Reset();
}

void DCRemover::Reset()
{
    memset(buffer, 0, DC_ADJUST_BUFFERLEN*sizeof(ymint));
    pos = DC_ADJUST_BUFFERLEN - 1;
    sum = 0;
    full = 0; // Calculate DC Remove only on the number of sample in buffer, can limits start clicks
}

inline void DCRemover::AddSample(ymint sample)
{
    pos = (pos+1)&(DC_ADJUST_BUFFERLEN-1);
#ifndef YM_SIMPLIFIED_FILTER
    if(!full && (pos == (DC_ADJUST_BUFFERLEN - 1)))
    {
        full = 1;
    }
#endif
    sum -= buffer[pos];
    sum += sample;

    buffer[pos] = sample;
}

inline ymint DCRemover::GetResult()
{
#ifdef YM_SIMPLIFIED_FILTER
    return buffer[pos] - (sum / DC_ADJUST_BUFFERLEN);
#else
    if(full)
    {
        return buffer[pos] - (sum / DC_ADJUST_BUFFERLEN);
    }
    else
    {
        return buffer[pos] - (sum / (pos + 1));
    }
#endif
}

SimpleLowPassFilter::SimpleLowPassFilter()
{
    Reset();
}

void SimpleLowPassFilter::Reset()
{
    delay_line[0] = 0;
    delay_line[1] = 0;
    delay_line[2] = 0;
}

inline void SimpleLowPassFilter::AddSample(ymint sample)
{
    delay_line[0] = delay_line[1];
    delay_line[1] = delay_line[2];
    delay_line[2] = sample;
}

inline ymint SimpleLowPassFilter::GetResult()
{
    return (delay_line[0] >> 2)
        + (delay_line[1] >> 1)
        + (delay_line[2] >> 2);
}

SimpleStereoEffectReducer::SimpleStereoEffectReducer()
{
    Reset();
}

void SimpleStereoEffectReducer::Reset()
{
    buffer.r = buffer.l = 0;
}

inline void SimpleStereoEffectReducer::AddSample(stereoSample sample)
{
    buffer = sample;
}

inline stereoSample SimpleStereoEffectReducer::GetResult()
{
    // Simple stereo reducer based on mid-side
    ymint m, s;
    stereoSample out;

    m = buffer.r + buffer.l;
    s = buffer.r - buffer.l;

    out.r = (3*m + s) >> 2;
    out.l = (3*m - s) >> 2;

    return out;
}

FilterStereo::FilterStereo()
{
}

#if 0
FIRFilter::FIRFilter()
    : N(512), delay(0), buffer(NULL), delay_line(NULL)
{
    Reset();
}

FIRFilter::FIRFilter(ymint N, ymint delay)
    : N(N), delay(delay), buffer(NULL), delay_line(NULL)
{
    Reset();
}

FIRFilter::~FIRFilter()
{
    free(buffer);
    buffer = NULL;
    free(delay_line);
    delay_line = NULL
}

void FIRFilter::Reset()
{
    free(buffer);
    buffer = (ymint*)malloc(N*size(ymint));
    if(buffer == NULL)
    {
        // TODO
    }
    memset(buffer, 0, N*sizeof(ymint));
    pos = N - 1;

    free(delay_line);
    delay_line = (ymint*)malloc(delay*size(ymint));
    if(buffer == NULL)
    {
        // TODO
    }
    memset(delay_line, 0, N*sizeof(ymint));
    delay_pos = delay - 1;
}

void FIRFilter::AddSample(ymint sample)
{
    delay_pos = (++delay_pos) % delay;
    pos = (++pos) % N;
    buffer[pos] = sample;
}

ymint FIRFilter::GetResult()
{
    double sum = 0;

    for(ymint i = 0; i < N; i++)
    {
        const ymint j = (pos + N - i) % N
        sum += coef[i] * buffer[j];
    }

    delay_line[delay_pos] = sum/coef_sum;

    return delay[(delay_pos + 1) % delay];
}
#endif // Unfinish
