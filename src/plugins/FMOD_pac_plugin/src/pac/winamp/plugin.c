/*
 * Copyright (c) 2005 Thomas Pfaff <thomaspfaff@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define WIN32_LEAN_AND_MEAN
#define WIN32_STRICT
#include <assert.h>
#include <windows.h>
#include <errno.h>
#include <stdio.h>
#include <pac.h>
#include <in2.h>

#define OUT_RATE 44100
#define OUT_BITS 16
#define OUT_CHANNELS 2
#define OUT_BUFSIZE 2048

#define WM_WA_MPEG_EOF WM_USER+2

static void inpac_config (HWND);
static void inpac_about (HWND);
static void inpac_init (void);
static void inpac_quit (void);
static int inpac_isourfile (char *);
static int inpac_play (char *);
static void inpac_pause (void);
static void inpac_unpause (void);
static int inpac_ispaused (void);
static void inpac_stop (void);
static int inpac_getlength (void);
static int inpac_getoutputtime (void);
static void inpac_setoutputtime (int);
static void inpac_setvolume (int);
static void inpac_setpan (int);
static void inpac_getfileinfo (char *, char *, int *);
static int inpac_infobox (char *, HWND);
static DWORD WINAPI DecodeThread (LPVOID *);

In_Module pac =
{
   IN_VER,
   "SBStudio PAC Player " PAC_VERSION,
   0,
   0,
   "PAC\0PAC Audio File (*.PAC)\0",
   1,
   1,
   inpac_config,
   inpac_about,
   inpac_init,
   inpac_quit,
   inpac_getfileinfo,
   inpac_infobox,
   inpac_isourfile,
   inpac_play,
   inpac_pause,
   inpac_unpause,
   inpac_ispaused,
   inpac_stop,
   inpac_getlength,
   inpac_getoutputtime,
   inpac_setoutputtime,
   inpac_setvolume,
   inpac_setpan,
   NULL, /* void SAVSAInit (int maxlatency, int srate) Called once in Play ()*/
   NULL, /* void SAVSADeInit (void); Called once in Stop () */
   NULL, /* void (*SAAddPCMData)(void *PCMData, int nch, int bps, int timestamp); */
   NULL, /* int (*SAGetMode)(); */
   NULL, /* void (*SAAdd)(void *data, int timestamp, int csa); */
   NULL, /* void (*VSAAddPCMData)(void *PCMData, int nch, int bps, int timestamp); */
   NULL, /* int (*VSAGetMode)(int *specNch, int *waveNch); */
   NULL, /* void (*VSAAdd)(void *data, int timestamp); filled in by winamp */
   NULL, /* void (*VSASetInfo)(int nch, int srate); call this in Play () */
   NULL, /* int (*dsp_isactive)(); filled in by winamp */
   NULL, /* int (*dsp_dosamples)(short int *s, int ns, int bps, int nch, int srate); */
   NULL, /* void (*EQSet)(int on, char data[10], int preamp); */
   NULL, /* void (*SetInfo)(int bitrate, int srate, int stereo, int synched); fibw */
   NULL, /* Out_Module *outMod; fibw */
};

static int stopping = 0;
static int paused = 0;
static int thread_alive = 0;
static HANDLE thread = NULL;
static DWORD tid = 0;
static CRITICAL_SECTION cs;
static int seek_to = -1;
static struct pac_module *pac_module = NULL;
static const char *pac_file = NULL;
static const char *about_msg = "SBStudio PAC Player by Thomas Pfaff "
   "<thomaspfaff@users.sourceforge.net>\n\nSee http://libpac.sourceforge.net "
   "for the latest release\n";

__declspec( dllexport ) In_Module * winampGetInModule2()
{
   return &pac;
}

BOOL WINAPI _DllMainCRTStartup (HANDLE hInst, ULONG ul_reason_for_call,
   LPVOID lpReserved)
{
   return TRUE;
}

static void inpac_config (HWND hwnd)
{
   MessageBox (hwnd, "Not implemented", "Configure SBStudio PAC Player", MB_OK);
}

static void inpac_about (HWND hwnd)
{
   MessageBox (hwnd, about_msg, "About SBStudio PAC Player", MB_OK);
}

static void inpac_init (void)
{
   InitializeCriticalSection (&cs);
}

static void inpac_quit (void)
{
   DeleteCriticalSection (&cs);
}

static int inpac_isourfile (char *fn)
{
   return pac_test (fn);
}

static int inpac_play (char *fn)
{
   int max;

   assert (thread_alive == 0 && stopping == 0 && paused == 0);

   if (pac_init (OUT_RATE, OUT_BITS, OUT_CHANNELS) != 0) {
      char s[512];
      sprintf (s, "Failed to initialize libpac: %s\n", pac_strerror (errno));
      MessageBox (NULL, s, "SBStudio PAC Player", MB_OK|MB_ICONSTOP);
      return 1;
   }
   pac_disable (PAC_STRICT_FORMAT);
   pac_module = pac_open (fn);
   if (pac_module == NULL) {
      char s[512];
      sprintf (s, "\"%s\": %s\n", fn, pac_strerror (errno));
      MessageBox (NULL, s, "SBStudio PAC Player", MB_OK|MB_ICONSTOP);
      pac_exit ();
      return 1;
   }
   pac_file = fn;
   if ((max = pac.outMod->Open (OUT_RATE, OUT_CHANNELS, OUT_BITS, 0, 0)) < 0) {
      MessageBox (NULL, "Failed to open output device", "SBStudio PAC Player",
         MB_OK|MB_ICONSTOP);
      pac_exit ();
      pac_module = NULL;
      pac_file = NULL;
      return 1;
   }
   pac.SetInfo ((OUT_RATE*OUT_BITS*OUT_CHANNELS)/1000, OUT_RATE/1000,
      OUT_CHANNELS, 1);
   pac.SAVSAInit (max, OUT_RATE);
   pac.VSASetInfo (OUT_CHANNELS, OUT_RATE);
   pac.outMod->SetVolume (-666);
   seek_to = -1;
   thread_alive = 1;
   thread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) DecodeThread,
      NULL, 0, &tid);
   SetThreadPriority (thread, THREAD_PRIORITY_ABOVE_NORMAL);
   return 0;
}

static void inpac_stop (void)
{
   stopping = 1;
   EnterCriticalSection (&cs);
   if (thread_alive || thread != NULL) {
      thread_alive = 0;
      WaitForSingleObject (thread, 5000);
      CloseHandle (thread);
      thread = NULL;
   }
   if (pac_module != NULL) {
      pac_exit ();
      pac_module = NULL;
      pac_file = NULL;
   }
   pac.outMod->Flush (0);
   pac.outMod->Close ();
   pac.SAVSADeInit ();
   paused = 0;
   stopping = 0;
   LeaveCriticalSection (&cs);
}

static void inpac_pause (void)
{
   paused = 1;
   pac.outMod->Pause (1);
}

static void inpac_unpause (void)
{
   paused = 0;
   pac.outMod->Pause (0);
}

static int inpac_ispaused (void)
{
   return paused;
}

static int inpac_getlength (void)
{
   int n;

   EnterCriticalSection (&cs);
   n = pac_module != NULL ? (double) pac_length (pac_module) / OUT_RATE *
      1000.0 : 0;
   LeaveCriticalSection (&cs);
   return n;
}

static int inpac_getoutputtime (void)
{
   return pac.outMod->GetOutputTime ();
}

static void inpac_setoutputtime (int time_in_ms)
{
   seek_to = time_in_ms;
}

static void inpac_setvolume (int volume)
{
   pac.outMod->SetVolume (volume);
}

static void inpac_setpan (int pan)
{
   pac.outMod->SetPan (pan);
}

static void inpac_getfileinfo (char *file, char *title, int *length_in_ms)
{
   EnterCriticalSection (&cs);
   if (file == NULL || *file == '\0') { /* Currently playing file. */
      if (length_in_ms != NULL)
         *length_in_ms = (double) pac_length (pac_module) / OUT_RATE * 1000.0;
      if (title != NULL)
         strcpy (title, pac_title (pac_module));
   }
   else { /* Some other file. */
      struct pac_module *p;
      if (pac_module == NULL)
         pac_init (44100, 16, 2);
      pac_enable (PAC_NOSOUNDS);
      p = pac_open (file);
      if (length_in_ms != NULL)
         *length_in_ms = p ? (double) pac_length (p) / OUT_RATE * 1000.0 : 0;
      if (title != NULL)
         strcpy (title, p ? pac_title (p) : "* LOAD FAILED *");
      pac_close (p);
      pac_disable (PAC_NOSOUNDS);
      if (pac_module == NULL)
         pac_exit ();
   }
   LeaveCriticalSection (&cs);
}

static int inpac_infobox (char *fn, HWND hwnd)
{
   return 0;
}

static DWORD WINAPI DecodeThread (LPVOID *lpParameter)
{
   long n = 0;
   char b[OUT_BUFSIZE*OUT_CHANNELS*OUT_BITS/8] = {0};

   assert (pac_module != NULL && pac_file != NULL);

   while (thread_alive) {
      if (!paused && pac.outMod->CanWrite () >= OUT_BUFSIZE) {
         n = 1;
         EnterCriticalSection (&cs);
         if (seek_to >= 0) {
            long frames = seek_to / 1000.0 * OUT_RATE;
            if (pac_seek (pac_module, frames, SEEK_SET) == frames)
               pac.outMod->Flush (seek_to);
            seek_to = -1;
         }
         n = pac_read (pac_module, b, OUT_BUFSIZE);
         if (n > 0) {
            long p = pac.outMod->GetWrittenTime ();
            pac.SAAddPCMData (b, OUT_CHANNELS, OUT_BITS, p);
            pac.VSAAddPCMData (b, OUT_CHANNELS, OUT_BITS, p);
            pac.outMod->Write (b, n);
         }
         LeaveCriticalSection (&cs);
         if (n <= 0) {
            while (pac.outMod->IsPlaying () && !stopping)
               Sleep (50); /* Drain output buffer. */
            if (!stopping)
               PostMessage(pac.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
            thread_alive = 0;
         }
      }
      else
         Sleep (50);
   }
   return 0;
}
