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

#include <assert.h>
#include <gtk/gtk.h>
#include <pac.h>
#include <pthread.h>
#include <xmms/configfile.h>
#include <xmms/plugin.h>
#include <xmms/util.h>
#include <xmms/xmmsctrl.h>

#define OUT_RATE 44100
#define OUT_BITS 16
#define OUT_CHANNELS 2
#define OUT_BUFSIZE 4096

static int out_rate = OUT_RATE;
static int out_bits = OUT_BITS;
static int out_channels = OUT_CHANNELS;

static int options = PAC_MODE_DEFAULT;
static gboolean show_playlist_info;

enum { BITS_8, BITS_16 };
static GtkWidget *bits[2];

enum { MONO, STEREO };
static GtkWidget *chan[2];

enum { RATE_11, RATE_22, RATE_44 };
static GtkWidget *rate[3];

static GtkWidget *gus_emulation;
static GtkWidget *interpolation;
static GtkWidget *strict_format;
static GtkWidget *playlist_info;
static GtkWidget *volume_reduction;

static GtkWidget *about, *config;
static pthread_t tid;
static pthread_mutex_t mutex;
static volatile int playing;
static int paused;
static int stopping;
static long seek_to = -1;
static struct pac_module *pac_module;

static void read_config (void);
static void write_config (GtkWidget *, gpointer);

static void xp_init (void);
static void xp_about (void);
static void xp_config (void);
static int xp_test (char *);
static void xp_play (char *);
static void xp_stop (void);
static void xp_pause (short);
static void xp_seek (int);
static int xp_get_time (void);
static void xp_get_volume (int *, int *);
static void xp_set_volume (int, int);
static void xp_cleanup (void);
static void xp_songinfo (char *, char **, int *);
static void *xp_thread (void *);

InputPlugin pac =
{
   NULL,
   NULL,
   "SBStudio PAC Player " PAC_VERSION,
   xp_init,
   xp_about,
   xp_config,
   xp_test,
   NULL,
   xp_play,
   xp_stop,
   xp_pause,
   xp_seek,
   NULL,
   xp_get_time,
   xp_get_volume,
   xp_set_volume,
   xp_cleanup,
   NULL,
   NULL,
   NULL,
   NULL,
   xp_songinfo,
   NULL,
   NULL
};

InputPlugin *
get_iplugin_info (void)
{
   return &pac;
}

static void
xp_init (void)
{
   read_config ();
   pthread_mutex_init (&mutex, NULL);
}

static void
xp_cleanup (void)
{
   pthread_mutex_destroy (&mutex);
}

static int
xp_test (char *filename)
{
   return pac_test (filename);
}

static void
xp_play (char *filename)
{
   long length;

   assert (tid == 0 && playing == 0 && paused == 0);

   if (pac_init (out_rate, out_bits, out_channels) != 0) {
      pac_perror ("pac_init");
      return;
   }
   pac_disable (PAC_MODE_DEFAULT);
   pac_enable (options);
   if ((pac_module = pac_open (filename)) == NULL) {
      pac_perror ("pac_open");
      pac_exit ();
      return;
   }
   length = pac_length (pac_module) / out_rate * 1000;
   pac.set_info ((char *) pac_title (pac_module), length,
      out_rate * out_channels * out_bits, out_rate, out_channels);
   pac.output->open_audio ((out_bits == 16) ? FMT_S16_LE : FMT_S8,
      out_rate, out_channels);
   playing = 1;
   if (pthread_create (&tid, NULL, xp_thread, NULL) != 0) {
      playing = 0;
      tid = 0;
      xp_stop ();
   }
}

static void
xp_stop (void)
{
   pthread_mutex_lock (&mutex);
   stopping = 1;
   if (playing) {
      playing = 0;
      paused = 0;
      pthread_join (tid, NULL);
   }
   pac.output->close_audio ();
   pac_exit ();
   pac_module = NULL;
   stopping = 0;
   seek_to = -1;
   tid = 0;
   pthread_mutex_unlock (&mutex);
}

static void
xp_pause (short p)
{
   pthread_mutex_lock (&mutex);
   if (playing) {
      pac.output->pause (p);
      paused = !paused;
   }
   pthread_mutex_unlock (&mutex);
}

static void
xp_seek (int pos)
{
   pthread_mutex_lock (&mutex);
   seek_to = pos * out_rate;
   pthread_mutex_unlock (&mutex);
}

static int
xp_get_time (void)
{
   return pac.output->output_time ();
}

static void
xp_get_volume (int *lv, int *rv)
{
   pac.output->get_volume (lv, rv);
}

static void
xp_set_volume (int lv, int rv)
{
   pac.output->set_volume (lv, rv);
}

static void
xp_songinfo (char *filename, char **title, int *length)
{
   struct pac_module *m;

   if (!show_playlist_info) {
      *length = 0;
      return;
   }
   pthread_mutex_lock (&mutex);
   if (pac_module == NULL)
      pac_init (out_rate, out_bits, out_channels);
   pac_enable (PAC_NOSOUNDS);
   if ((m = pac_open (filename)) != NULL) {
      *title = g_strdup (pac_title (m));
      *length = pac_length (m) / out_rate * 1000;
      pac_close (m);
   }
   else {
      *title = g_strdup ("* ERROR: FAILED TO LOAD FILE *");
      *length = 0;
   }
   if (pac_module == NULL)
      pac_exit ();
   pac_disable (PAC_NOSOUNDS);
   pthread_mutex_unlock (&mutex);
}

static void *
xp_thread (void *arg_not_used)
{
   long n;
   unsigned char b[OUT_BUFSIZE];

   while (playing) {
      if (pac.output->buffer_free () >= OUT_BUFSIZE) {
         pthread_mutex_lock (&mutex);
         if (seek_to != -1) {
            pac_seek (pac_module, seek_to, SEEK_SET);
            pac.output->flush (seek_to / out_rate  * 1000);
            seek_to = -1;
         }
         n = pac_read (pac_module, b, OUT_BUFSIZE);
         pthread_mutex_unlock (&mutex);
         if (n > 0) {
            pac.add_vis_pcm (pac.output->written_time (),
               (out_bits == 16) ? FMT_S16_LE : FMT_S8, out_channels, n, b);
            pac.output->write_audio (b, n);
         }
         else {
            /* Stop pre-buffering. */
            pac.output->buffer_free ();
            pac.output->buffer_free ();
            /* Drain output buffer. */
            while (playing && (pac.output->buffer_playing() || paused) &&
               !stopping)
               xmms_usleep (10 * 1000);
            /* Next song or stop. */
            playing = 0;
            if (!stopping) {
               if (xmms_remote_get_playlist_pos (0) <
                   xmms_remote_get_playlist_length (0) - 1)
                  xmms_remote_playlist_next (0);
               else
                  xmms_remote_stop (0);
            }
         }
      }
      else
         xmms_usleep (10 * 1000);
   }
   return NULL;
}

static void
xp_about (void)
{
   GtkWidget *button;
   GtkWidget *label;
   GtkWidget *vbox;
   GtkWidget *buttonbox;

   if (about != NULL)
      return;

   about = gtk_window_new (GTK_WINDOW_DIALOG);
   gtk_window_set_title (GTK_WINDOW (about), "About SBStudio PAC Player");
   gtk_window_set_policy (GTK_WINDOW (about), FALSE, FALSE, FALSE);
   gtk_window_set_position (GTK_WINDOW (about), GTK_WIN_POS_MOUSE);

   vbox = gtk_vbox_new (FALSE, 5);
   gtk_container_add (GTK_CONTAINER (about), vbox);
   label = gtk_label_new ("\n  SBStudio PAC Player by Thomas Pfaff "
      "<thomaspfaff@users.sourceforge.net>  \n  Get the latest release from "
      "http://libpac.sourceforge.net  \n");
   gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
   gtk_container_add (GTK_CONTAINER (vbox), label);

   buttonbox = gtk_hbutton_box_new ();
   gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox),
      GTK_BUTTONBOX_DEFAULT_STYLE);
   gtk_button_box_set_spacing (GTK_BUTTON_BOX (buttonbox), 5);
   gtk_box_pack_start (GTK_BOX(vbox), buttonbox, FALSE, FALSE, 0);
   gtk_signal_connect (GTK_OBJECT (about), "destroy",
      GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);

   button = gtk_button_new_with_label ("Ok");
   GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
   gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
      GTK_SIGNAL_FUNC (gtk_widget_destroy), (gpointer) about);
   gtk_box_pack_start (GTK_BOX ((buttonbox)), button, FALSE, TRUE, 5);

   gtk_widget_show_all (about);
}

static void
destroy_config (GtkWidget *widget, gpointer data)
{
   gtk_widget_destroy (config);
   config = NULL;
}

static void
xp_config (void)
{
   GtkWidget *notebook;
   GtkWidget *vbox;
   GtkTooltips *tips;

   if (config != NULL)
      return;

   config = gtk_window_new (GTK_WINDOW_DIALOG);
   gtk_window_set_title (GTK_WINDOW (config), "Configure SBStudio PAC Player");
   gtk_window_set_policy (GTK_WINDOW (config), FALSE, FALSE, FALSE);
   gtk_window_set_position (GTK_WINDOW (config), GTK_WIN_POS_MOUSE);
   gtk_container_border_width(GTK_CONTAINER (config), 5);
   gtk_signal_connect (GTK_OBJECT (config), "destroy",
      GTK_SIGNAL_FUNC (gtk_widget_destroyed), &config);

   /* Contain Notebook and Buttons. */
   vbox = gtk_vbox_new (FALSE, 5);
   gtk_container_add (GTK_CONTAINER (config), vbox);
   tips = gtk_tooltips_new ();
   notebook = gtk_notebook_new ();
   gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 5);

   /* Quality */
   {
      GSList *group;
      GtkWidget *radio;
      GtkWidget *frame;
      GtkWidget *table;
      GtkWidget *vbox;

      table = gtk_table_new (2, 2, 0);
      gtk_table_set_row_spacings (GTK_TABLE (table), 10);
      gtk_table_set_col_spacings (GTK_TABLE (table), 10);
      gtk_container_set_border_width (GTK_CONTAINER (table), 5);
      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table,
         gtk_label_new ("Quality"));

      frame = gtk_frame_new ("Resolution");
      vbox = gtk_vbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (frame), vbox);
      radio = gtk_radio_button_new_with_label (NULL, "16 bits");
      gtk_box_pack_start (GTK_BOX (vbox), radio, FALSE, FALSE, 0);
      if (out_bits == 16)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
      bits[BITS_16] = radio;
      radio = gtk_radio_button_new_with_label (gtk_radio_button_group
         (GTK_RADIO_BUTTON (radio)), "8 bits");
      gtk_box_pack_start (GTK_BOX (vbox), radio, FALSE, FALSE, 0);
      bits[BITS_8] = radio;
      if (out_bits == 8)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
      gtk_table_attach_defaults (GTK_TABLE (table), frame, 0, 1, 0, 1);

      frame = gtk_frame_new ("Channels");
      vbox = gtk_vbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (frame), vbox);
      radio = gtk_radio_button_new_with_label (NULL, "Stereo");
      gtk_box_pack_start (GTK_BOX (vbox), radio, FALSE, FALSE, 0);
      if (out_channels == 2)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
      chan[STEREO] = radio;
      radio = gtk_radio_button_new_with_label (gtk_radio_button_group
         (GTK_RADIO_BUTTON (radio)), "Mono");
      gtk_box_pack_start (GTK_BOX (vbox), radio, FALSE, FALSE, 0);
      if (out_channels == 1)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
      chan[MONO] = radio;
      gtk_table_attach_defaults (GTK_TABLE (table), frame, 1, 2, 0, 1);

      frame = gtk_frame_new ("Rate");
      vbox = gtk_vbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (frame), vbox);
      radio = gtk_radio_button_new_with_label (NULL, "44100 Hz");
      gtk_box_pack_start (GTK_BOX (vbox), radio, FALSE, FALSE, 0);
      if (out_rate == 44100)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
      rate[RATE_44] = radio;
      radio = gtk_radio_button_new_with_label (gtk_radio_button_group
         (GTK_RADIO_BUTTON (radio)), "22050 Hz");
      gtk_box_pack_start (GTK_BOX (vbox), radio, FALSE, FALSE, 0);
      if (out_rate == 22050)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
      rate[RATE_22] = radio;
      radio = gtk_radio_button_new_with_label (gtk_radio_button_group
         (GTK_RADIO_BUTTON (radio)), "11025 Hz");
      gtk_box_pack_start (GTK_BOX (vbox), radio, FALSE, FALSE, 0);
      if (out_rate == 11025)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
      rate[RATE_11] = radio;
      gtk_table_attach_defaults (GTK_TABLE (table), frame, 0, 2, 1, 2);
   }

   /* Options */
   {
      GtkWidget *page;
      GtkWidget *frame;
      GtkWidget *check;

      page = gtk_vbox_new (FALSE, 0);
      gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page,
         gtk_label_new ("Options"));

      check = gtk_check_button_new_with_label ("Emulate Gravis UltraSound");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), check,  "Emulate Gravis "
         "UltraSound hardware volume ramping for version 1.4 PAC modules "
         "using the Cxy (note-off) effect.", NULL);
      gtk_box_pack_start (GTK_BOX (page), check, FALSE, FALSE, 0);
      if (options & PAC_GUS_EMULATION)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
      gus_emulation = check;

      check = gtk_check_button_new_with_label ("Linear interpolation");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), check,  "Use linear "
         "interpolation for slightly better sample-rate conversion.", NULL);
      gtk_box_pack_start (GTK_BOX (page), check, FALSE, FALSE, 0);
      if (options & PAC_INTERPOLATION)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
      interpolation = check;

      check = gtk_check_button_new_with_label ("Volume reduction");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), check,  "Reduce mixing "
         "volume level to avoid excessive clipping.", NULL);
      gtk_box_pack_start (GTK_BOX (page), check, FALSE, FALSE, 0);
      if (options & PAC_VOLUME_REDUCTION)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
      volume_reduction = check;

      check = gtk_check_button_new_with_label ("Strict format");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), check,  "Fail loading files "
         "that have non-critical errors, such as missing one or more sounds, "
         "sheets (patterns), or channel settings.", NULL);
      gtk_box_pack_start (GTK_BOX (page), check, FALSE, FALSE, 0);
      if (options & PAC_STRICT_FORMAT)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
      strict_format = check;

      check = gtk_check_button_new_with_label ("Show song information in "
         "playlist");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), check, "Show song "
         "information for files added to the playlist.  This may slow things "
         "down as each file has to be loaded and then closed.", NULL);
      gtk_box_pack_start (GTK_BOX (page), check, FALSE, FALSE, 0);
      if (show_playlist_info)
         gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
      playlist_info = check;
   }

   /* Buttons */
   {
      GtkWidget *hbox;
      GtkWidget *button;

      hbox = gtk_hbox_new (TRUE, 5);
      button = gtk_button_new_with_label ("OK");
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
         GTK_SIGNAL_FUNC (write_config), NULL);
      button = gtk_button_new_with_label ("Cancel");
      gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
         GTK_SIGNAL_FUNC (destroy_config), NULL);
      /*gtk_signal_connect (GTK_OBJECT (button), "clicked",
        GTK_SIGNAL_FUNC (gtk_widget_destroy), &config);*/
      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
   }
   gtk_widget_show_all (config);
}

/* Called once at initialization by xp_init. */
static void
read_config (void)
{
   ConfigFile *cfg;
   gchar *filename;

   filename = g_strconcat (g_get_home_dir(), "/.xmms/config", NULL);
   if ((cfg = xmms_cfg_open_file (filename)) != NULL) {
      gboolean b = FALSE;

      /* Quality. */
      xmms_cfg_read_int (cfg, "pac", "rate", &out_rate);
      xmms_cfg_read_int (cfg, "pac", "bits", &out_bits);
      xmms_cfg_read_int (cfg, "pac", "channels", &out_channels);

      /* Options. */
      options = 0;
      if (xmms_cfg_read_boolean (cfg, "pac", "gus_emulation", &b)) {
         if (b == TRUE)
            options |= PAC_GUS_EMULATION;
      }
      else if (PAC_MODE_DEFAULT & PAC_GUS_EMULATION)
         options |= PAC_GUS_EMULATION;

      if (xmms_cfg_read_boolean (cfg, "pac", "interpolation", &b)) {
         if (b == TRUE)
            options |= PAC_INTERPOLATION;
      }
      else if (PAC_MODE_DEFAULT & PAC_INTERPOLATION)
         options |= PAC_INTERPOLATION;

      if (xmms_cfg_read_boolean (cfg, "pac", "volume_reduction", &b)) {
         if (b == TRUE)
            options |= PAC_VOLUME_REDUCTION;
      }
      else if (PAC_MODE_DEFAULT & PAC_VOLUME_REDUCTION)
         options |= PAC_VOLUME_REDUCTION;

      if (xmms_cfg_read_boolean (cfg, "pac", "strict_format", &b)) {
         if (b == TRUE)
            options |= PAC_STRICT_FORMAT;
      }
      else if (PAC_MODE_DEFAULT & PAC_STRICT_FORMAT)
         options |= PAC_STRICT_FORMAT;

      xmms_cfg_read_boolean (cfg, "pac", "playlist_info", &show_playlist_info);
   }
   g_free (filename);
}

/* Called in response to user clicking "OK" in configuration dialog. */
static void
write_config (GtkWidget *widget, gpointer data)
{
   gboolean b;
   ConfigFile *cfg = NULL;
   gchar *filename = NULL;

   filename = g_strconcat (g_get_home_dir(), "/.xmms/config", NULL);
   cfg = xmms_cfg_open_file (filename);
   if (cfg == NULL)
      cfg = xmms_cfg_new ();

   /* Quality. */
   if (GTK_TOGGLE_BUTTON (rate[RATE_11])->active)
      out_rate = 11025;
   else if (GTK_TOGGLE_BUTTON (rate[RATE_22])->active)
      out_rate = 22050;
   else
      out_rate = 44100;
   xmms_cfg_write_int (cfg, "pac", "rate", out_rate);

   out_bits = (GTK_TOGGLE_BUTTON (bits[BITS_8])->active) ? 8 : 16;
   xmms_cfg_write_int (cfg, "pac", "bits", out_bits);

   out_channels = (GTK_TOGGLE_BUTTON (chan[MONO])->active) ? 1 : 2;
   xmms_cfg_write_int (cfg, "pac", "channels", out_channels);

   /* Options. */
   options = 0;
   if (GTK_TOGGLE_BUTTON (gus_emulation)->active)
      options |= PAC_GUS_EMULATION;
   xmms_cfg_write_boolean (cfg, "pac", "gus_emulation",
      options & PAC_GUS_EMULATION);

   if (GTK_TOGGLE_BUTTON (interpolation)->active)
      options |= PAC_INTERPOLATION;
   xmms_cfg_write_boolean (cfg, "pac", "interpolation",
      options & PAC_INTERPOLATION);

   if (GTK_TOGGLE_BUTTON (volume_reduction)->active)
      options |= PAC_VOLUME_REDUCTION;
   xmms_cfg_write_boolean (cfg, "pac", "volume_reduction",
      options & PAC_VOLUME_REDUCTION);

   if (GTK_TOGGLE_BUTTON (strict_format)->active)
      options |= PAC_STRICT_FORMAT;
   xmms_cfg_write_boolean (cfg, "pac", "strict_format",
      options & PAC_STRICT_FORMAT);

   show_playlist_info = GTK_TOGGLE_BUTTON (playlist_info)->active;
   xmms_cfg_write_boolean (cfg, "pac", "playlist_info", show_playlist_info);

   xmms_cfg_write_file (cfg, filename);
   xmms_cfg_free (cfg);
   g_free (filename);

   gtk_widget_destroy (config);
   config = NULL;

   /* Apply options if playing. */
   pthread_mutex_lock (&mutex);
   if (playing) {
      pac_disable (0xff);
      pac_enable (options);
   }
   pthread_mutex_unlock (&mutex);
}
