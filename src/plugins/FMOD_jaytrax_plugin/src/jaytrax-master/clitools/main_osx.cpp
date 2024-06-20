#include <stdio.h>
#include <stdint.h>
#include <signal.h>

#include "audio_output.h"

#include "jxs.h"
#include "jaytrax.h"

#define SAMPLE_RATE 44100 /* 22050 - test low-pass capability of blep synthesizer */

static int running;

static JT1Player * player;
static volatile int fade_start, fade_length;
static volatile int max_channels;

void signal_handler(int sig)
{
	running = 0;
    signal(sig, signal_handler);
}

void fade_buffer(signed short *buffer, unsigned int count, int fade_start, int fade_length)
{
    unsigned int i;
    for (i = 0; i < count; i++)
    {
        if (fade_start < fade_length)
        {
            buffer[ i * 2 + 0 ] = (int64_t)((int64_t)buffer[ i * 2 + 0 ] * ( fade_length - fade_start )) / fade_length;
            buffer[ i * 2 + 1 ] = (int64_t)((int64_t)buffer[ i * 2 + 1 ] * ( fade_length - fade_start )) / fade_length;
            fade_start++;
        }
        else
        {
            buffer[ i * 2 + 0 ] = 0;
            buffer[ i * 2 + 1 ] = 0;
        }
    }
}

void render(void * unused, short * samples, uint32_t sampleCount)
{
    int nch = 0;
    int chanNr = player->subsong->nrofchans;
    jaytrax_renderChunk(player, samples, sampleCount, SAMPLE_RATE);
    if (!player->playFlg) running = 0;
    if (player->loopCnt >= 2)
    {
        fade_buffer( samples, sampleCount, fade_start, fade_length );
        fade_start += sampleCount;
    }
    for (int ic = 0; ic < chanNr; ++ic)
    {
        if (player->voices[ic].instrument != -1)
            nch++;
    }
    fprintf(stderr, "\ro: %3u - r: %2u - c: %2u (%2u)", player->playPosition, player->playStep, nch, nch > max_channels ? max_channels = nch : max_channels);
}

int main(int argc, const char* const* argv)
{
    JT1Song * song;
    CoreAudioStream * output;

	if (argc < 2 || argc > 3)
	{
		fprintf(stderr, "Usage:\t%s <song> [songnumber]\n", argv[0]);
		return 1;
	}
    
    jxsfile_readSong(argv[1], &song);
    if (!song)
    {
        fprintf(stderr, "Invalid song:\t%s\n", argv[1]);
        return 1;
    }

    player = jaytrax_init();
    if (!player)
    {
        fprintf(stderr, "Out of memory.\n");
        jxsfile_freeSong(song);
        return 1;
    }
    
    if (jaytrax_loadSong(player, song) <= 0)
    {
        fprintf(stderr, "Out of memory.\n");
        jaytrax_free(player);
        jxsfile_freeSong(song);
        return 1;
    }

    int track_num = 0;
    if (argc == 3)
    {
        char * end;
        track_num = strtoul(argv[2], &end, 0);
        if (!end || *end || track_num >= song->nrofsongs)
            track_num = 0;
    }

    jaytrax_changeSubsong(player, track_num);

    int nch = 0;
    int chanNr = player->subsong->nrofchans;
    for (int ic = 0; ic < chanNr; ++ic)
    {
        if (player->voices[ic].instrument != -1)
            nch++;
    }

    fprintf(stderr, "Syntrax test player v0.0001 || %i/%i\n", player->subsongNr+1, player->song->nrofsongs);
    fprintf(stderr, "Title: %s\n", player->subsong->name);
    fprintf(stderr, "o: %3u - r: %2u - c: %2u (%2u)", player->playPosition, player->playStep, nch, nch > max_channels ? max_channels = nch : max_channels);

    signal(SIGINT, signal_handler);

    output = new CoreAudioStream(render, 0, SAMPLE_RATE);
    
	if ( output )
	{
        fade_start = 0; fade_length = SAMPLE_RATE * 10;
        max_channels = 0;
        running = 1;
        output->start();
        while ( running && fade_start < fade_length )
		{
            usleep(10000);
		}
        output->close();
        fprintf(stderr, "\n");
	}

    delete output;
    
    jaytrax_free(player);
    jxsfile_freeSong(song);

	return 0;
}
