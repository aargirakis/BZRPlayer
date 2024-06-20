/*
 * Copyright (c) 2007 Thomas Pfaff <thomaspfaff@users.sourceforge.net>
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

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#include <ao/ao.h>
#include <pac.h>

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

/* Default output parameters. */
#define OUT_RATE 44100L
#define OUT_BITS 16
#define OUT_CHANNELS 2

static long out_rate = OUT_RATE;
static int out_bits = OUT_BITS;
static int out_channels = OUT_CHANNELS;
static unsigned char out_buffer[BUFSIZ];

static int quiet;
static volatile sig_atomic_t playing;

static int ao_id = -1;
static ao_device *ao_dev = NULL;
static ao_sample_format ao_fmt;

static void sig_int (int);

int
main (int argc, char *argv[])
{
   int c;
   long n;
   struct pac_module *m;

   while ((c = getopt (argc, argv, "r:b:c:hq")) != -1) {
      switch (c) {
      case 'r':
         n = strtol (optarg, NULL, 10);
         if (n < PAC_RATE_MIN || n > PAC_RATE_MAX) {
            fprintf (stderr, "pacplay: rate must be between %ld and %ld\n",
               PAC_RATE_MIN, PAC_RATE_MAX);
            exit (EXIT_FAILURE);
         }
         out_rate = n;
         break;
      case 'b':
         n = strtol (optarg, NULL, 10);
         if (n != 8 && n != 16) {
            fprintf (stderr, "pacplay: bits must be 8 or 16\n");
            exit (EXIT_FAILURE);
         }
         out_bits = n;
         break;
      case 'c':
         n = strtol (optarg, NULL, 10);
         if (n != 1 && n != 2) {
            fprintf (stderr, "pacplay: channels must be 1 or 2\n");
            exit (EXIT_FAILURE);
         }
         out_channels = n;
         break;
      case 'h':
         printf ("usage: pacplay [-rbchq] file.pac [-]\n");
         exit (EXIT_SUCCESS);
         break;
      case 'q':
         quiet = 1;
         break;
      default:
         fprintf (stderr, "usage: pacplay [-rbchq] file.pac [-]\n"); 
         exit (EXIT_FAILURE);
         break;
      }
   }
   argv += optind;
   argc -= optind;

   if (*argv == NULL) {
      fprintf (stderr, "pacplay: no input file\n");
      exit (EXIT_FAILURE);
   }

   /* Check and initialize audio output destination. */
   if (argv[1] == NULL || *argv[1] != '-') {
      ao_initialize ();
      ao_id = ao_default_driver_id ();
      if (ao_id == -1) {
         fprintf (stderr, "pacplay: could not find usable output device\n");
         exit (EXIT_FAILURE);
      }
      ao_fmt.bits = out_bits;
      ao_fmt.rate = out_rate;
      ao_fmt.channels = out_channels;
      ao_fmt.byte_format = AO_FMT_LITTLE; /* XXX make libpac handle _NATIVE */
      ao_dev = ao_open_live (ao_id, &ao_fmt, NULL);
      if (ao_dev == NULL) {
         fprintf (stderr, "pacplay: failed to open output device\n");
         ao_shutdown ();
         exit (EXIT_FAILURE);
      }
   }
   else {
      if (*argv[1] == '-' && isatty (1)) {
         fprintf (stderr, "pacplay: not writing to a tty\n");
         exit (EXIT_FAILURE);
      }
   }

   /* Initialize libpac and open file. */
   if (pac_init (out_rate, out_bits, out_channels) != 0) {
      fprintf (stderr, "pacplay: init failed. %s\n", pac_strerror (errno));
      ao_shutdown ();
      exit (EXIT_FAILURE);
   }
   if ((m = pac_open (argv[0])) == NULL) {
      fprintf (stderr, "pacplay: open failed. %s\n", pac_strerror (errno));
      pac_exit ();
      ao_shutdown ();
      exit (EXIT_FAILURE);
   }

   /* Start the decoding loop and stop on SIGINT or EOF. */
   if (signal (SIGINT, sig_int) == SIG_ERR)
      fprintf (stderr, "pacplay: signal. %s\n", strerror (errno));

/* XXX ignore futher arguments after infile if not '-' ... */
   playing = 1;
   while (playing && (n = pac_read (m, out_buffer, sizeof out_buffer)) > 0) {
      if (argv[1] == NULL || *argv[1] != '-')
         ao_play (ao_dev, out_buffer, n);
      else
         fwrite (out_buffer, n, 1, stdout);

      if (!quiet) {
         n = pac_tell (m) / out_rate;
         fprintf (stderr, "\rPlaying \"%s\" [%.2ld:%.2ld] (%.1f%%)",
            pac_title (m), n / 60, n % 60,
            pac_tell (m) * 100.0 / pac_length (m));
      }
   }
   fputc ('\n', stderr);

   /* Clean up and exit. */
   if (argv[1] == NULL || *argv[1] != '-') {
      ao_close (ao_dev);
      ao_shutdown ();
   }
   pac_exit ();
   exit (EXIT_SUCCESS);
}

static void
sig_int (int sig)
{
   playing = 0;
}
