#ifndef __IMG_SAVE_H__
#  define __IMG_SAVE_H__

GimpPDBStatusType img_read_options(ImageParasite *, const gchar *,
                                   const gchar *, GError **);

GimpPDBStatusType img_save_image(gint32, const ImageParasite *, const gchar *,
                                 GError **);
gboolean img_save_dialog(gint32, ImageParasite *);

GimpPDBStatusType img_sanity_check(gint32, GError **);
#endif
