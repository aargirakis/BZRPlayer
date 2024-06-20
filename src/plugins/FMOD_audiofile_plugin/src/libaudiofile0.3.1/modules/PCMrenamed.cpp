/*
	Audio File Library
	Copyright (C) 2000, Silicon Graphics, Inc.
	Copyright (C) 2010, Michael Pruett <michael@68k.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA  02111-1307  USA.
*/

/*
	PCM.cpp - read and file write module for uncompressed data
*/

//#include "config.h"
#include "PCMrenamed.h"

#include <errno.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "FileModule.h"
#include "Track.h"
#include "afinternal.h"
#include "audiofile.h"
#include "compression.h"
#include "util.h"

#ifdef DEBUG
#define CHNK(X) (X)
#else
#define CHNK(X)
#endif

bool _af_pcm_format_ok (AudioFormat *f)
{
	assert(!isnan(f->pcm.slope));
	assert(!isnan(f->pcm.intercept));
	assert(!isnan(f->pcm.minClip));
	assert(!isnan(f->pcm.maxClip));

	return true;
}

class PCM : public FileModule
{
public:
	static Module *createCompress(Track *track, File *fh, bool canSeek,
		bool headerless, AFframecount *chunkFrames);
	static Module *createDecompress(Track *track, File *fh, bool canSeek,
		bool headerless, AFframecount *chunkFrames);

	virtual const char *name() const { return "pcm"; }
	virtual void runPull();
	virtual void reset2();
	virtual void runPush();
	virtual void sync1();
	virtual void sync2();

private:
	int m_bytesPerFrame;

	/* saved_fpos_next_frame and saved_nextfframe apply only to writing. */
	int m_saved_fpos_next_frame;
	int m_saved_nextfframe;

	PCM(Mode, Track *, File *, bool canSeek);
};

PCM::PCM(Mode mode, Track *track, File *fh, bool canSeek) :
	FileModule(mode, track, fh, canSeek),
	m_bytesPerFrame(track->f.bytesPerFrame(false)),
	m_saved_fpos_next_frame(-1),
	m_saved_nextfframe(-1)
{
	if (mode == Decompress)
		track->f.compressionParams = AU_NULL_PVLIST;
}

Module *PCM::createCompress(Track *track, File *fh, bool canSeek,
	bool headerless, AFframecount *chunkframes)
{
	return new PCM(Compress, track, fh, canSeek);
}

void PCM::runPush()
{
	AFframecount frames2write = m_inChunk->frameCount;
	AFframecount n;

	/*
		WARNING: due to the optimization explained at the end
		of arrangemodules(), the pcm file module cannot depend
		on the presence of the intermediate working buffer
		which _AFsetupmodules usually allocates for file
		modules in their input or output chunk (for reading or
		writing, respectively).

		Fortunately, the pcm module has no need for such a buffer.
	*/

	ssize_t bytesWritten = write(m_inChunk->buffer, m_bytesPerFrame * frames2write);
	n = bytesWritten >= 0 ? bytesWritten / m_bytesPerFrame : 0;

	CHNK(printf("writing %jd frames to pcm file\n", (intmax_t) frames2write));

	if (n != frames2write)
	{
		/* Report error if we haven't already. */
		if (m_track->filemodhappy)
		{
			/* I/O error */
			if (n < 0)
				_af_error(AF_BAD_WRITE,
					"unable to write data (%s) -- "
					"wrote %d out of %d frames",
					strerror(errno),
					m_track->nextfframe + n,
					m_track->nextfframe + frames2write);
			/* usual disk full error */
			else
				_af_error(AF_BAD_WRITE,
					"unable to write data (disk full) -- "
					"wrote %d out of %d frames",
					m_track->nextfframe + n,
					m_track->nextfframe + frames2write);
			m_track->filemodhappy = false;
		}
	}

	m_track->nextfframe += n;
	m_track->totalfframes = m_track->nextfframe;
	m_track->fpos_next_frame += (n>0) ? n * m_bytesPerFrame : 0;
	assert(!canSeek() || (tell() == m_track->fpos_next_frame));
}

void PCM::sync1()
{
	m_saved_fpos_next_frame = m_track->fpos_next_frame;
	m_saved_nextfframe = m_track->nextfframe;
}

void PCM::sync2()
{
	assert(!canSeek() || (tell() == m_track->fpos_next_frame));

	/* We can afford to seek because sync2 is rare. */
	m_track->fpos_after_data = tell();

	m_track->fpos_next_frame = m_saved_fpos_next_frame;
	m_track->nextfframe = m_saved_nextfframe;
}

Module *PCM::createDecompress(Track *track, File *fh, bool canSeek,
	bool headerless, AFframecount *chunkframes)
{
	return new PCM(Decompress, track, fh, canSeek);
}

void PCM::runPull()
{
	AFframecount framesToRead = m_outChunk->frameCount;

	/*
		WARNING: Due to the optimization explained at the end of
		arrangemodules(), the pcm file module cannot depend on
		the presence of the intermediate working buffer which
		_AFsetupmodules usually allocates for file modules in
		their input or output chunk (for reading or writing,
		respectively).

		Fortunately, the pcm module has no need for such a buffer.
	*/

	/*
		Limit the number of frames to be read to the number of
		frames left in the track.
	*/
	if (m_track->totalfframes != -1 &&
		m_track->nextfframe + framesToRead > m_track->totalfframes)
	{
		framesToRead = m_track->totalfframes - m_track->nextfframe;
	}

	ssize_t bytesRead = read(m_outChunk->buffer, m_bytesPerFrame * framesToRead);
	AFframecount framesRead = bytesRead >= 0 ? bytesRead / m_bytesPerFrame : 0;

	CHNK(printf("reading %jd frames from pcm file (got %jd)\n",
		(intmax_t) framesToRead, (intmax_t) framesRead));

	m_track->nextfframe += framesRead;
	m_track->fpos_next_frame += (framesRead>0) ? framesRead * m_bytesPerFrame : 0;
	assert(!canSeek() || (tell() == m_track->fpos_next_frame));

	/*
		If we got EOF from read, then we return the actual amount read.

		Complain only if there should have been more frames in the file.
	*/

	if (framesRead != framesToRead && m_track->totalfframes != -1)
	{
		/* Report error if we haven't already. */
		if (m_track->filemodhappy)
		{
			_af_error(AF_BAD_READ,
				"file missing data -- read %d frames, "
				"should be %d",
				m_track->nextfframe,
				m_track->totalfframes);
			m_track->filemodhappy = false;
		}
	}

	m_outChunk->frameCount = framesRead;
}

void PCM::reset2()
{
	m_track->fpos_next_frame = m_track->fpos_first_frame +
		m_bytesPerFrame * m_track->nextfframe;

	m_track->frames2ignore = 0;
}

Module *_AFpcminitcompress (Track *track, File *fh, bool canSeek,
	bool headerless, AFframecount *chunkFrames)
{
	return PCM::createCompress(track, fh, canSeek, headerless, chunkFrames);
}

Module *_AFpcminitdecompress (Track *track, File *fh, bool canSeek,
	bool headerless, AFframecount *chunkFrames)
{
	return PCM::createDecompress(track, fh, canSeek, headerless, chunkFrames);
}
