#ifndef __IMG_H__
#define __IMG_H__

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <errno.h>
#include <string.h>
#include <glib/gstdio.h>

#define LOAD_PROC        "file-img-load"
#define SAVE_PROC        "file-img-save"
#define PLUG_IN_BINARY   "file-img"

/* Comment out to disable debug */
#define DEBUG

#ifdef DEBUG
#define __DEBUG(x) {x}
#define D(x) \
{ \
  printf("IMG plugin: "); \
  printf x; \
}
#else
#define __DEBUG(x)
#define D(x)
#endif

/* FIXME: MIME type for these files */
#define IMG_MIME        "image/x-img"

#define FMT_RGB565 0
#define FMT_RGB 1
#define FMT_RGBA 2

extern const gchar *FMT[3];

/* File structure:
 * |------------------------|
 * | File header            |
 * |------------------------|
 * | [Color key for RGB]    |
 * |------------------------|
 * | Image data             |
 * |- - - - - - - - - - - - |
 *
 * */

/* ACHTUNG: byte order in the file is LITTLE-ENDIAN - lowest byte comes first */
typedef struct _FileHeader
{
  guint8 fmt;                   /* 0 - RGB565, 1 - RGB, 2 - RGBA) */

  /*
   * ACHTUNG: do not align, or else sizeof(ftm)==4:  __attribute__ ((packed))
   */
  guint32 nrows __attribute__((packed));        /* Number of rows of frames */
  guint32 ncols __attribute__((packed));        /* Number of columns of frames */

  guint32 width __attribute__((packed));        /* Image width (total) */
  guint32 height __attribute__((packed));       /* Image height (total) */

} FileHeader;

typedef struct _ColorKey
{
  guint8 is;                    /* 0 - no color key, otherwise is one */
  guint8 R __attribute__((packed));
  guint8 G __attribute__((packed));
  guint8 B __attribute__((packed));

} ColorKey;

/* Save params for GIMP_RUN_WITH_LAST_VALS
 * for each image LOADED image or for plugin defaults *
 * */

#define PARASITE_ORIG_FILE "orig-file-info"
typedef struct _ImageParasite
{
  guint32 format;
  ColorKey ckey;
} ImageParasite;

#endif
