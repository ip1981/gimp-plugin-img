#include "plugin-img.h"

/*
 * Parse strings for format and color key
 **/
GimpPDBStatusType
img_read_options (ImageParasite * p, const gchar * fmt, const gchar * ckey,
                  GError ** error)
{
  gdouble r, g, b;
  guint32 R, G, B;

  if ((strcmp (fmt, FMT[0]) == 0) || (strcmp (fmt, "0") == 0))
    p->format = FMT_RGB565;
  else if ((strcmp (fmt, FMT[1]) == 0) || (strcmp (fmt, "1") == 0))
    p->format = FMT_RGB;
  else if ((strcmp (fmt, FMT[2]) == 0) || (strcmp (fmt, "2") == 0))
    p->format = FMT_RGBA;
  else
    {
      g_set_error (error, 0, 0, "Invalid file format: %s", fmt);
      return GIMP_PDB_CALLING_ERROR;
    }

  if (NULL == ckey)
    {
      p->ckey.is = 0;
      p->ckey.R = 0;
      p->ckey.G = 0;
      p->ckey.B = 0;
    }
  else
    {
      if ((strcmp (ckey, "") == 0) || (strcmp (ckey, "0") == 0) ||
          (strcmp (ckey, "no") == 0))
        {
          p->ckey.is = 0;
        }
      else if (3 == sscanf (ckey, "(%u, %u, %u)", &R, &G, &B))
        {
          if ((R < 256) && (G < 256) && (B < 256))
            {
              p->ckey.is = 1;
              p->ckey.R = R;
              p->ckey.G = G;
              p->ckey.B = B;
            }
          else
            {
              g_set_error (error, 0, 0,
                           "Invalid color key: %s; all values must be < 256",
                           ckey);
              return GIMP_PDB_CALLING_ERROR;
            }
        }
      else if (3 == sscanf (ckey, "(%lf; %lf; %lf)", &r, &g, &b))
        {
          if ((r <= 1.0) && (g <= 1.0) && (b <= 1.0) && (r >= 0.0) &&
              (g >= 0.0) && (b >= 0.0))
            {
              p->ckey.is = 1;
              p->ckey.R = (guint8) (r * 255);
              p->ckey.G = (guint8) (g * 255);
              p->ckey.B = (guint8) (b * 255);
            }
          else
            {
              g_set_error (error, 0, 0,
                           "Invalid color key: %s; example: (0.1; 0.7; 0.3), all values must be in [0; 1]",
                           ckey);
              return GIMP_PDB_CALLING_ERROR;
            }
        }
      else if (1 == sscanf (ckey, "0x%x", &R))
        {
          size_t l;

          l = strlen (ckey);
          if (5 == l)           /* e. g. 0xAF9, but not 0xAAA6 */
            {
              p->ckey.is = 1;
              G = R & 0x000F00;
              p->ckey.R = (guint8) ((G >> 4) | (G >> 8));       /* Red */
              G = R & 0x0000F0;
              p->ckey.G = (guint8) (G | (G >> 4));      /* Green */
              G = R & 0x00000F;
              p->ckey.B = (guint8) ((G << 4) | G);      /* Blue */
            }
          else if (8 == l)      /* e. g. 0xFFAA99 */
            {
              p->ckey.is = 1;
              p->ckey.R = (guint8) ((R & 0xFF0000) >> 16);      /* Red */
              p->ckey.G = (guint8) ((R & 0x00FF00) >> 8);       /* Green */
              p->ckey.B = (guint8) (R & 0x0000FF);      /* Blue */
            }
          else
            {
              g_set_error (error, 0, 0,
                           "Hex color key %s is invalid; examples are 0xFFF or 0xFF44GG",
                           ckey);
              return GIMP_PDB_CALLING_ERROR;
            }
        }
      else
        {
          g_set_error (error, 0, 0, "Invalid color key: %s", ckey);
          return GIMP_PDB_CALLING_ERROR;
        }

    }
  D (("Parsed format: %s (%1u)\n", FMT[p->format], p->format));
  D (("Parsed color key: %s (%1u, %1u, %1u)\n", (p->ckey.is) ? "yes" : "no",
      p->ckey.R, p->ckey.G, p->ckey.B));
  return GIMP_PDB_SUCCESS;
}

static inline void
rgb_to_rgb565 (const guint8 * c, guint16 * res)
{
  /*
   * How to:
   * |        24 bits       |
   * |   B  ||   G  ||  R   |
   * 11111xxx000000xx11111xxx
   * | B |   |  G |  | R |
   * 11111   000000  11111
   * |   high  ||  low   |
   * |       16 bits     |
   *
   */
  (*res) = 0;
  (*res) |= ((guint16) (c[0]) >> 3);    /* Red */
  (*res) |= ((guint16) (c[1]) >> 2) << 5;       /* Green */
  (*res) |= ((guint16) (c[2]) >> 3) << 11;      /* Blue */
}

static void
img_map_gimp_to_file (GimpImageType image_type, guint8 fmt,
                      const ColorKey * ckey, guint8 * src, guint8 * dest,
                      size_t npixels)
{
  size_t s, d;
  guint8 r, g, b;

  switch (image_type)
    {
    case GIMP_RGB_IMAGE:
      npixels *= 3;
      switch (fmt)
        {
        case FMT_RGB565:       /* RGB -> RGB565 */
          for (s = 0, d = 0; s < npixels;)
            {
              rgb_to_rgb565 (&(src[s]), (guint16 *) & (dest[d]));
              d += 2;
              s += 3;
            }
          break;

        case FMT_RGB:          /* RGB -> RGB */
          memcpy (dest, src, npixels);
          break;

        case FMT_RGBA:         /* RGB -> RGBA */
          if (ckey->is == 0)
            for (s = 0, d = 0; s < npixels;)
              {
                dest[d++] = src[s++];   /* Red */
                dest[d++] = src[s++];   /* Green */
                dest[d++] = src[s++];   /* Blue */
                dest[d++] = 255;        /* Opaque */
              }
          else                  /* Color key to Alpha */
            for (s = 0, d = 0; s < npixels;)
              {
                r = dest[d++] = src[s++];       /* Red */
                g = dest[d++] = src[s++];       /* Green */
                b = dest[d++] = src[s++];       /* Blue */
                if ((r == ckey->R) && (g == ckey->G) && (b == ckey->B))
                  dest[d++] = 0;        /* Transparent */
                else
                  dest[d++] = 255;      /* Opaque */
              }
          break;

        default:
          break;
        }
      break;

    case GIMP_RGBA_IMAGE:
      npixels *= 4;
      switch (fmt)
        {
        case FMT_RGB565:       /* RGBA -> RGB565 */
          for (s = 0, d = 0; s < npixels;)
            {
              rgb_to_rgb565 (&(src[s]), (guint16 *) & (dest[d]));
              d += 2;
              s += 4;           /* Skip Alpha */
            }
          break;

        case FMT_RGB:          /* RGBA -> RGB */
          for (s = 0, d = 0; s < npixels;)
            {
              dest[d++] = src[s++];     /* Red */
              dest[d++] = src[s++];     /* Green */
              dest[d++] = src[s++];     /* Blue */
              s++;              /* Skip Alpha */
            }
          break;

        case FMT_RGBA:         /* RGBA -> RGBA */
          memcpy (dest, src, npixels);
          break;

        default:
          break;
        }
      break;

    default:
      break;
    }
}

GimpPDBStatusType
img_sanity_check (gint32 image_ID, GError ** error)
{
  gint32 *layers;
  gint nlayers;
  gint image_width;
  gint image_height;
  gint i;
  GimpPDBStatusType status = GIMP_PDB_SUCCESS;

  if (gimp_image_base_type (image_ID) != GIMP_RGB)
    {
      g_set_error (error, 0, 0, "Can save only RGB images.");
      return GIMP_PDB_EXECUTION_ERROR;
    }

  layers = gimp_image_get_layers (image_ID, &nlayers);
  if (nlayers == 0)
    {
      g_set_error (error, 0, 0, "Can't save empty images ;-)");
      return GIMP_PDB_EXECUTION_ERROR;
    }

  image_width = gimp_image_width (image_ID);
  image_height = gimp_image_height (image_ID);

  for (i = 0; i < nlayers; i++)
    {
      if ((gimp_drawable_width (layers[i]) != image_width) ||
          (gimp_drawable_height (layers[i]) != image_height))
        {
          g_set_error (error, 0, 0,
                       "All layers must exactly match width and height of image.");
          status = GIMP_PDB_EXECUTION_ERROR;
          break;
        }
    }

  g_free (layers);
  return status;
}

GimpPDBStatusType
img_save_image (gint32 image, const ImageParasite * plugin,
                const gchar * filename, GError ** error)
{
  FILE *fp;
  GimpPixelRgn pixel_rgn;
  GimpDrawable *drawable = NULL;
  gint *layers = NULL, nlayers;
  size_t nwritten;
  guint8 *data = NULL, *pixels = NULL;
  size_t data_size, npixels;
  gint i, width, height;
  FileHeader hdr;

  /*
   * Should call sanity_check() for all secure checks
   */

  layers = gimp_image_get_layers (image, &nlayers);
  width = gimp_image_width (image);
  height = gimp_image_height (image);
  npixels = width * height;

  /*
   * Making file header
   */
  hdr.fmt = plugin->format;
  hdr.ncols = 1;                /* Always one column */
  hdr.nrows = nlayers;
  hdr.width = width * hdr.ncols;
  hdr.height = height * hdr.nrows;

  hdr.nrows = GUINT32_TO_LE (hdr.nrows);
  hdr.ncols = GUINT32_TO_LE (hdr.ncols);
  hdr.width = GUINT32_TO_LE (hdr.width);
  hdr.height = GUINT32_TO_LE (hdr.height);

  D (("*** Saving \"%s\"\n", filename));

  gimp_progress_init_printf ("Saving '%s'", gimp_filename_to_utf8 (filename));

  if (!(fp = g_fopen (filename, "wb")))
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   "Could not open '%s' for writing: %s",
                   gimp_filename_to_utf8 (filename), g_strerror (errno));
      g_free (layers);
      return GIMP_PDB_EXECUTION_ERROR;
    }

  /*
   * Write file
   */
  D (("Writing header: format: %s (%1u), frames: %ux%u, size: %ux%u\n",
      FMT[hdr.fmt], hdr.fmt, hdr.ncols, hdr.nrows, hdr.width, hdr.height));
  nwritten = fwrite ((const void *) &hdr, sizeof (hdr), 1, fp);
  if (nwritten != 1)
    {
      D (("Error: %s\n", g_strerror (errno)));
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   "Writing file header: %s\n", g_strerror (errno));
      fclose (fp);
      g_free (layers);
      return GIMP_PDB_EXECUTION_ERROR;
    }

  /*
   * File format specific preparations
   */
  data_size = width * height;
  switch (hdr.fmt)
    {
    case FMT_RGB565:
      data_size *= 2;
      break;

    case FMT_RGB:
      D (("Writing color key: %s (%1u, %1u, %1u)\n",
          (plugin->ckey.is) ? "yes" : "no", plugin->ckey.R, plugin->ckey.G,
          plugin->ckey.B));
      nwritten =
        fwrite ((const void *) &(plugin->ckey), sizeof (plugin->ckey), 1, fp);
      if (nwritten != 1)
        {
          D (("Error: %s\n", g_strerror (errno)));
          g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                       "Writing color key: %s\n", g_strerror (errno));
          fclose (fp);
          g_free (layers);
          return GIMP_PDB_EXECUTION_ERROR;
        }
      data_size *= 3;
      break;

    case FMT_RGBA:
      __DEBUG (if (plugin->ckey.is)
               D (("Will convert color key "
                   "(%1u, %1u, %1u) to full transparent\n", plugin->ckey.R,
                   plugin->ckey.G, plugin->ckey.B)););
      data_size *= 4;
      break;
    }

  /*
   * Writing each layer (frame)
   */
  data = g_new (guint8, data_size);
  pixels = g_new (guint8, npixels * 4); /* Enough for RGB or RGBA */
  for (i = 0; i < nlayers; i++)
    {
      gimp_progress_update ((gdouble) i / (gdouble) nlayers);
      drawable = gimp_drawable_get (layers[i]);
      gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0, width, height, FALSE,
                           FALSE);
      gimp_pixel_rgn_get_rect (&pixel_rgn, (guchar *) pixels, 0, 0, width,
                               height);
      D (("Writing frame #%u of %u (%lu bytes)\n", i + 1, nlayers, data_size));
      /*
       * Allow different image type for different layers
       */
      img_map_gimp_to_file (gimp_drawable_type (layers[i]), hdr.fmt,
                            &(plugin->ckey), pixels, data, npixels);

      nwritten = fwrite ((const void *) data, data_size, 1, fp);
      if (nwritten != 1)
        {
          D (("Error: %s\n", g_strerror (errno)));
          g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                       "Writing layer #%u: %s\n", i, g_strerror (errno));
          fclose (fp);
          g_free (layers);
          g_free (data);
          g_free (pixels);
          return GIMP_PDB_EXECUTION_ERROR;
        }
    }

  fclose (fp);
  g_free (layers);
  g_free (data);
  g_free (pixels);
  gimp_progress_update (1.0);
  D (("*** Written \"%s\"\n", filename));

  return GIMP_PDB_SUCCESS;
}
