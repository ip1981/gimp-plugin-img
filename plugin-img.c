#include "plugin-img.h"
#include "img-load.h"
#include "img-save.h"
#include "img-save-dialog.h"

const gchar *FMT[3] = { "RGB565", "RGB", "RGBA" };

static void
img_set_parasite (gint32 image, const ImageParasite * p)
{
  D (("Setting parasite for %u: format: %s (%1u), "
      "color key: %s (%1u, %1u, %1u)\n", image, FMT[p->format], p->format,
      (p->ckey.is) ? "yes" : "no", p->ckey.R, p->ckey.G, p->ckey.B));
  gimp_image_parasite_detach (image, PARASITE_ORIG_FILE);
  gimp_image_attach_new_parasite (image, PARASITE_ORIG_FILE,
                                  GIMP_PARASITE_PERSISTENT,
                                  sizeof (ImageParasite), p);
}

static void
img_get_parasite (gint32 image, ImageParasite * p)
{
  GimpParasite *gp = NULL;

  gp = gimp_image_parasite_find (image, PARASITE_ORIG_FILE);
  if (gp != NULL)
    {
      p->format = ((ImageParasite *) (gp->data))->format;
      p->ckey = ((ImageParasite *) (gp->data))->ckey;
      D (("Setting parasite for %u: format: %s (%1u), "
          "color key: %s (%1u, %1u, %1u)\n", image, FMT[p->format], p->format,
          (p->ckey.is) ? "yes" : "no", p->ckey.R, p->ckey.G, p->ckey.B));

      gimp_parasite_free (gp);
    }
}

static void
query (void)
{
  static const GimpParamDef load_args[] = {
    {GIMP_PDB_INT32, "run-mode", "Interactive, non-interactive"},
    {GIMP_PDB_STRING, "filename", "The name of the file to load"},
    {GIMP_PDB_STRING, "raw-filename", "The name entered"}
  };
  static const GimpParamDef load_return_vals[] = {
    {GIMP_PDB_IMAGE, "image", "Output image"},
  };

  static const GimpParamDef save_args[] = {
    {GIMP_PDB_INT32, "run-mode", "Interactive, non-interactive"},
    {GIMP_PDB_IMAGE, "image", "Input image"},
    {GIMP_PDB_DRAWABLE, "drawable", "Drawable to save"},
    {GIMP_PDB_STRING, "filename",
     "The name of the file to save the image in"},
    {GIMP_PDB_STRING, "raw-filename", "The name entered"},
    {GIMP_PDB_STRING, "format",
     "File format: RGB565 (or 0), RGB (1) or RGBA (2)"},
    {GIMP_PDB_STRING, "ckey",
     "Color key, ex.: 0, no, (23,55,56), (0.1;0.5;0.8), 0xFFF, 0xFEFEFE"}
  };

  gimp_install_procedure (LOAD_PROC, "Loads files of IMG file format",
                          "Loads files of IMG file format",
                          "Igor Pashev <pashev.igor@gmail.com>",
                          "Public Domain", "2009", "IMG", NULL, GIMP_PLUGIN,
                          G_N_ELEMENTS (load_args),
                          G_N_ELEMENTS (load_return_vals), load_args,
                          load_return_vals);

  gimp_register_file_handler_mime (LOAD_PROC, IMG_MIME);
  gimp_register_magic_load_handler (LOAD_PROC, "img", "", "");

  gimp_install_procedure (SAVE_PROC, "Saves files in IMG file format",
                          "Saves files in IMG file format",
                          "Igor Pashev <pashev.igor@gmail.com>",
                          "Public Domain", "2009", "IMG", "*", GIMP_PLUGIN,
                          G_N_ELEMENTS (save_args), 0, save_args, NULL);

  gimp_register_file_handler_mime (SAVE_PROC, IMG_MIME);
  gimp_register_save_handler (SAVE_PROC, "img", "");
}

static void
run (const gchar * name, gint nparams, const GimpParam * param,
     gint * nreturn_vals, GimpParam ** return_vals)
{
  static GimpParam values[2];
  gint32 image_ID;
  gint32 drawable_ID;
  static ImageParasite plugin;
  GimpRunMode run_mode;
  GimpPDBStatusType status = GIMP_PDB_SUCCESS;
  GimpExportReturn export = GIMP_EXPORT_CANCEL;

  GError *error = NULL;

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals = values;
  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;

  /*
   * Load image
   */
  if (strcmp (name, LOAD_PROC) == 0)
    {
      switch (run_mode)
        {
        case GIMP_RUN_INTERACTIVE:
          break;

        case GIMP_RUN_NONINTERACTIVE:
          if (nparams != 3)
            status = GIMP_PDB_CALLING_ERROR;
          break;

        default:
          break;
        }

      if (status == GIMP_PDB_SUCCESS)
        {
          image_ID = img_load_image (param[1].data.d_string, &plugin, &error);

          if (image_ID != -1)
            {
              *nreturn_vals = 2;
              values[1].type = GIMP_PDB_IMAGE;
              values[1].data.d_image = image_ID;
              img_set_parasite (image_ID, &plugin);
            }
          else
            {
              status = GIMP_PDB_EXECUTION_ERROR;
            }
        }
    }

  /*
   * Save image
   */
  else if (strcmp (name, SAVE_PROC) == 0)
    {
      gchar *filename;

      image_ID = param[1].data.d_int32;
      drawable_ID = param[2].data.d_int32;
      filename = param[3].data.d_string;

      switch (run_mode)
        {
        case GIMP_RUN_WITH_LAST_VALS:  /* Save */
        case GIMP_RUN_INTERACTIVE:     /* Save as... */
          gimp_get_data (SAVE_PROC, &plugin);   /* Get plugin recent options */
          img_get_parasite (image_ID, &plugin); /* Maybe overwrite it by image specific */
          gimp_ui_init (PLUG_IN_BINARY, FALSE);
          export =
            gimp_export_image (&image_ID, &drawable_ID, "IMG",
                               (GIMP_EXPORT_CAN_HANDLE_RGB |
                                GIMP_EXPORT_CAN_HANDLE_ALPHA |
                                GIMP_EXPORT_CAN_HANDLE_LAYERS));
          if (export == GIMP_EXPORT_CANCEL)
            {
              status = GIMP_PDB_CANCEL;
            }

          /*
           * Sanity check: RGB, geometry, layers.
           */
          if (status == GIMP_PDB_SUCCESS)
            {
              status = img_sanity_check (image_ID, &error);
            }

          /*
           * Allow user to override default values
           */
          if ((run_mode == GIMP_RUN_INTERACTIVE) &&
              (status == GIMP_PDB_SUCCESS))
            {
              if (!img_save_dialog (image_ID, &plugin))
                status = GIMP_PDB_CANCEL;
              else
                {
                  gimp_set_data (SAVE_PROC, &plugin, sizeof (ImageParasite));   /* Save plugin defaults */
                  img_set_parasite (image_ID, &plugin); /* Save the same for the image */
                }
            }
          break;

        case GIMP_RUN_NONINTERACTIVE:  /* TODO: non-interactive */
          if (nparams < 6)
            status = GIMP_PDB_CALLING_ERROR;
          if (status == GIMP_PDB_SUCCESS)
            {
              status = img_sanity_check (image_ID, &error);
            }
          if (status == GIMP_PDB_SUCCESS)
            {
              gchar *fmt, *key;

              fmt = param[5].data.d_string;
              key = (nparams > 6) ? param[6].data.d_string : NULL;
              status = img_read_options (&plugin, fmt, key, &error);
            }
          break;

        default:
          status = GIMP_PDB_CALLING_ERROR;
          break;
        }

      if (status == GIMP_PDB_SUCCESS)
        {
          status = img_save_image (image_ID, &plugin, filename, &error);
        }

      /*
       * Delete temporary image after export
       */
      if (export == GIMP_EXPORT_EXPORT)
        {
          gimp_image_delete (image_ID);
        }
    }
  else
    {
      status = GIMP_PDB_CALLING_ERROR;
    }

  if (status != GIMP_PDB_SUCCESS && error)
    {
      *nreturn_vals = 2;
      values[1].type = GIMP_PDB_STRING;
      values[1].data.d_string = error->message;
    }

  values[0].data.d_status = status;
}

const GimpPlugInInfo PLUG_IN_INFO = {
  NULL,                         /* init_proc  */
  NULL,                         /* quit_proc  */
  query,                        /* query_proc */
  run,                          /* run_proc   */
};

MAIN ()
