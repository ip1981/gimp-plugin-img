#include "plugin-img.h"

static inline void
rgb565_to_rgb (const guint16 * rgb565, guint8 * res)
{
  /*
   * How to:
   * |       16 bits     |
   * |    high ||   low  |
   * 11111   111111  11111
   * | B |   |  G |  | R |
   * 111110001111110011111000
   * |   B  ||   G  ||  R   |
   * |       24 bits        |
   *
   */
  res[0] = (guint8) (((*rgb565) & 0x1F) << 3);  /* Red */
  res[1] = (guint8) (((*rgb565) & 0x7E0) >> 3); /* Green */
  res[2] = (guint8) (((*rgb565) & 0xF800) >> 8);        /* Blue */
}

static void
img_map_rgb565_to_rgb (const guint8 * src, guint8 * dest, size_t src_size)
{
  size_t s, d;

  /*
   * 5+6+5=16bits (2 bytes) per pixel
   */
  for (s = 0, d = 0; s < src_size;)
    {
      rgb565_to_rgb ((const guint16 *) &(src[s]), &(dest[d]));
      s += 2;
      d += 3;
    }
}

gint32
img_load_image (const gchar * filename, ImageParasite * meta, GError ** error)
{
  FILE *fp;
  size_t nread;
  size_t src_size;
  guint8 *src, *dest, *buf;
  FileHeader hdr;
  ColorKey ckey;
  guint32 row, col, width, height;
  gint32 frame, nframes, *frames;
  gchar layer_name[20];
  gint32 image, layer;
  GeglBuffer *gegl_buffer;
  GimpImageType layer_type;
  const Babl *format;

  ckey.is = 0;
  ckey.R = 0;
  ckey.G = 0;
  ckey.B = 0;

  gimp_progress_init_printf ("Opening '%s'",
                             gimp_filename_to_utf8 (filename));

  D (("*** Loading \"%s\"\n", filename));
  fp = g_fopen (filename, "rb");
  if (!fp)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno), "%s",
                   g_strerror (errno));
      return -1;
    }

  /*
   * Read common header
   */
  nread = fread ((void *) &hdr, sizeof (hdr), 1, fp);
  if (nread != 1)
    {
      D (("Error reading file header: %s\n", g_strerror (errno)));
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   "Error reading file header: %s\n", g_strerror (errno));
      fclose (fp);
      return -1;
    }
  hdr.nrows = GUINT32_FROM_LE (hdr.nrows);
  hdr.ncols = GUINT32_FROM_LE (hdr.ncols);
  hdr.width = GUINT32_FROM_LE (hdr.width);
  hdr.height = GUINT32_FROM_LE (hdr.height);

  /*
   * Width and height are TOTAL ones
   */
  if ((0 == hdr.ncols) || (0 == hdr.nrows) || (hdr.width % hdr.ncols) ||
      (hdr.height % hdr.nrows))
    {
      D (("Invalid file geometry: frames: %ux%u, size: %ux%u\n", hdr.ncols,
          hdr.nrows, hdr.width, hdr.height));
      g_set_error (error, 0, 0,
                   "Invalid file geometry: frames: %ux%u, size: %ux%u\n",
                   hdr.ncols, hdr.nrows, hdr.width, hdr.height);
      fclose (fp);
      return -1;
    }
  width = hdr.width / hdr.ncols;
  height = hdr.height / hdr.nrows;

  src_size = width * height;
  switch (hdr.fmt)
    {
    case FMT_RGB565:
      src_size *= 2;
      layer_type = GIMP_RGB_IMAGE;
      format = babl_format ("R'G'B' u8");
      break;
    case FMT_RGB:
      src_size *= 3;
      layer_type = GIMP_RGB_IMAGE;
      format = babl_format ("R'G'B' u8");
      /*
       * Read color key
       */
      nread = fread ((void *) &ckey, sizeof (ckey), 1, fp);
      if (nread != 1)
        {
          D (("Error reading color key: %s\n", g_strerror (errno)));
          g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                       "Error reading color key: %s\n", g_strerror (errno));
          fclose (fp);
          return -1;
        }
      if (ckey.is != 0)
        ckey.is = 1;            /* Normalize */

      break;
    case FMT_RGBA:
      src_size *= 4;            /* Alpha only for RGBA */
      layer_type = GIMP_RGBA_IMAGE;
      format = babl_format ("R'G'B'A u8");
      break;
    default:
      D (("Invalid file format: %1u\n", hdr.fmt));
      g_set_error (error, 0, 0, "Invalid file format: %1u\n", hdr.fmt);
      fclose (fp);
      return -1;
    }

  D (("Format: %s (%u), frames: %ux%u, size: %ux%u\n", FMT[hdr.fmt], hdr.fmt,
      hdr.ncols, hdr.nrows, hdr.width, hdr.height));
  __DEBUG (if (ckey.is)
           D (("Has color key: (%1u, %1u, %1u)\n", ckey.R, ckey.G, ckey.B)));

  /*
   * We are ready to make image with layers
   */
  image =
    gimp_image_new_with_precision (width, height, GIMP_RGB,
                                   GIMP_PRECISION_U8_GAMMA);
  gimp_image_undo_disable (image);
  gimp_image_set_filename (image, filename);

  frame = 0;
  nframes = hdr.nrows * hdr.ncols;
  src = g_new (guint8, src_size);
  dest = (hdr.fmt == FMT_RGB565) ? g_new (guint8, width * height * 3) : NULL;
  for (row = 1; row <= hdr.nrows; row++)
    {
      for (col = 1; col <= hdr.ncols; col++)
        {
          frame++;
          gimp_progress_update ((gdouble) frame / (gdouble) nframes);
          D (("Reading frame #%u of %u (%ux%u, %lu bytes)\n", frame, nframes,
              width, height, src_size));
          g_snprintf (layer_name, sizeof (layer_name), "#%i", frame);
          layer =
            gimp_layer_new (image, layer_name, width, height, layer_type, 100,
                            GIMP_NORMAL_MODE);
          gegl_buffer = gimp_drawable_get_buffer (layer);

          nread = fread ((char *) src, src_size, 1, fp);
          if (nread != 1)
            {
              D (("Invalid image data\n"));
              g_set_error (error, G_FILE_ERROR,
                           g_file_error_from_errno (errno),
                           "Invalid image data\n");

              if (dest)
                g_free (dest);
              g_free (src);
              fclose (fp);
              return -1;
            }

          if (hdr.fmt == FMT_RGB565)
            {
              img_map_rgb565_to_rgb (src, dest, src_size);
              buf = dest;
            }
          else
            buf = src;

          gegl_buffer_set (gegl_buffer,
                           GEGL_RECTANGLE (0, 0, width, height), 0,
                           format, buf, GEGL_AUTO_ROWSTRIDE);
          g_object_unref (gegl_buffer);
          gimp_image_insert_layer (image, layer, -1, frame - 1);
        }
    }

  frames = gimp_image_get_layers (image, (gint32 *) & nframes);
  if (nframes > 0)
    {
      gimp_image_set_active_layer (image, frames[0]);
      g_free (frames);
    }

  if (dest)
    g_free (dest);
  g_free (src);
  fclose (fp);

  meta->format = hdr.fmt;
  meta->ckey = ckey;
  gimp_progress_update (1.0);
  D (("*** Loaded \"%s\"\n", filename));

  return image;
}
