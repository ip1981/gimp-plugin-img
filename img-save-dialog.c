#include "plugin-img.h"
#include <libgimp/gimpui.h>

static GtkWidget *fmt_frame, *ckey_frame, *fmt_vbox, *ckey_vbox, *ckey_align,
  *fmt_align, *hbox, *fmt_rgb565, *fmt_rgb, *fmt_rgba, *ckey_image, *ckey_use,
  *gimp_color;
static GimpRGB color;
static GdkPixbuf *pixbuf;

static GtkWidget *
img_preview_new (gint32 image_ID)
{
  GtkWidget *image, *event_box, *align;
  guint32 width, height;
  gint *frames, nframes, layer;

  frames = gimp_image_get_layers (image_ID, &nframes);
  layer = frames[g_random_int () % nframes];
  g_free (frames);
  width = gimp_drawable_width (layer);
  height = gimp_drawable_height (layer);

  pixbuf =
    gimp_drawable_get_thumbnail (layer, MIN (width, 128), MIN (height, 128),
                                 GIMP_PIXBUF_SMALL_CHECKS);

  image = gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (pixbuf);

  event_box = gtk_event_box_new ();
  gtk_widget_set_events (event_box, GDK_ALL_EVENTS_MASK);

  align = gtk_alignment_new (0.5, 0.5, 0, 0);

  gtk_widget_show (image);
  gtk_widget_show (event_box);
  gtk_container_add (GTK_CONTAINER (event_box), image);
  gtk_container_add (GTK_CONTAINER (align), event_box);

  return align;
}

static void
on_RGB (GtkWidget * widget, gpointer data)
{
  gtk_widget_set_sensitive (ckey_vbox,
                            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                                          (fmt_rgb)) ||
                            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                                          (fmt_rgba)));
}

static void
on_ckey_use (GtkWidget * widget, gpointer data)
{
  gboolean active;

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
  gtk_widget_set_sensitive (ckey_image, active);
  gtk_widget_set_sensitive (gimp_color, active);
}

static gboolean
on_image_click (GtkWidget * widget, GdkEventButton * event, gpointer data)
{
  int width, height, rowstride, n_channels;
  guchar *pixels, *p;

  n_channels = gdk_pixbuf_get_n_channels (pixbuf);
  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);
  p = pixels + (int) (event->y * rowstride) + (int) (event->x * n_channels);
  gimp_rgb_set_uchar (&color, p[0], p[1], p[2]);
  gimp_color_button_set_color ((GimpColorButton *) gimp_color, &color);
  return TRUE;
}

gboolean
img_save_dialog (gint32 image, ImageParasite * plugin)
{
  GtkWidget *dialog;
  gint response;

  gimp_ui_init (PLUG_IN_BINARY, TRUE);
  dialog =
    gimp_dialog_new ("Save as IMG", PLUG_IN_BINARY, NULL, 0,
                     gimp_standard_help_func, "plug-in-img",
                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
                     GTK_RESPONSE_OK, NULL);
  gimp_window_set_transient (GTK_WINDOW (dialog));
  gtk_widget_show (dialog);

  /*
   * Radio buttons to select file format
   */
  fmt_frame = gimp_frame_new ("Image format");
  fmt_rgb565 = gtk_radio_button_new_with_label (NULL, FMT[FMT_RGB565]);
  fmt_rgb =
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON
                                                 (fmt_rgb565), FMT[FMT_RGB]);
  fmt_rgba =
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (fmt_rgb),
                                                 FMT[FMT_RGBA]);
  gtk_widget_show (fmt_frame);
  gtk_widget_show (fmt_rgb565);
  gtk_widget_show (fmt_rgb);
  gtk_widget_show (fmt_rgba);

  /*
   * Horizontal main box
   */
  hbox = gtk_hbox_new (TRUE, 2);
  gtk_widget_show (hbox);

  /*
   * Vertical box for radio buttons
   */
  fmt_align = gtk_alignment_new (0.5, 0, 0, 0);
  gtk_widget_show (fmt_align);
  fmt_vbox = gtk_vbox_new (TRUE, 12);
  gtk_container_add (GTK_CONTAINER (fmt_align), fmt_vbox);
  gtk_container_set_border_width (GTK_CONTAINER (fmt_vbox), 12);
  gtk_widget_show (fmt_vbox);
  gtk_box_pack_start (GTK_BOX (fmt_vbox), fmt_frame, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (fmt_vbox), fmt_rgb565, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (fmt_vbox), fmt_rgb, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (fmt_vbox), fmt_rgba, TRUE, TRUE, 0);

  /*
   * Allow color key for RGB and RGBA
   */
  g_signal_connect (G_OBJECT (fmt_rgb), "clicked", G_CALLBACK (on_RGB), NULL);
  g_signal_connect (G_OBJECT (fmt_rgba), "clicked", G_CALLBACK (on_RGB),
                    NULL);

  /*
   * Vertical box for color key
   */
  ckey_align = gtk_alignment_new (0.5, 0, 0, 0);
  gtk_widget_show (ckey_align);
  ckey_vbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (ckey_align), ckey_vbox);
  gtk_container_set_border_width (GTK_CONTAINER (ckey_vbox), 12);
  ckey_frame = gimp_frame_new ("Transparent color");
  ckey_use = gtk_check_button_new_with_label ("Set transparent color");
  ckey_image = img_preview_new (image);
  gimp_rgb_set_uchar (&color, plugin->ckey.R, plugin->ckey.G, plugin->ckey.B);
  gimp_color =
    gimp_color_button_new ("Choose color", 64, 64, &color,
                           GIMP_COLOR_AREA_FLAT);
  gtk_box_pack_start (GTK_BOX (ckey_vbox), ckey_frame, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (ckey_vbox), ckey_use, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (ckey_vbox), ckey_image, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (ckey_vbox), gimp_color, TRUE, TRUE, 0);
  gtk_widget_show (gimp_color);
  gtk_widget_show (ckey_vbox);
  gtk_widget_show (ckey_frame);
  gtk_widget_show (ckey_use);
  gtk_widget_show (ckey_image);
  g_signal_connect (G_OBJECT (ckey_use), "clicked", G_CALLBACK (on_ckey_use),
                    NULL);
  g_signal_connect (G_OBJECT (ckey_image), "button-press-event",
                    G_CALLBACK (on_image_click), NULL);
  /*
   * Assemble all together
   */
  gtk_box_pack_start (GTK_BOX (hbox), fmt_align, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), ckey_align, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, TRUE, TRUE,
                      0);

  /*
   * Set initial parameters
   */
  switch (plugin->format)
    {
    case FMT_RGB565:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fmt_rgb565), TRUE);
      break;
    case FMT_RGB:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fmt_rgb), TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ckey_use),
                                    plugin->ckey.is != 0);
      break;
    case FMT_RGBA:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fmt_rgba), TRUE);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ckey_use),
                                    plugin->ckey.is != 0);
      break;
    }

  on_RGB (fmt_rgb, NULL);
  on_ckey_use (ckey_use, NULL);

  /*
   * Run the dialog
   */
  response = gimp_dialog_run (GIMP_DIALOG (dialog));

  /*
   * Get new parameters
   */
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fmt_rgb565)))
    plugin->format = FMT_RGB565;
  else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fmt_rgb)))
    plugin->format = FMT_RGB;
  else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (fmt_rgba)))
    plugin->format = FMT_RGBA;
  plugin->ckey.is =
    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ckey_use)) ? 1 : 0;
  gimp_color_button_get_color ((GimpColorButton *) gimp_color, &color);
  gimp_rgb_get_uchar (&color, &plugin->ckey.R, &plugin->ckey.G,
                      &plugin->ckey.B);
  gtk_widget_destroy (dialog);
  return (response == GTK_RESPONSE_OK);
}
