/*
 * This file is part of LaTeXila.
 *
 * Copyright (C) 2015 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * LaTeXila is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LaTeXila is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LaTeXila.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:templates-personal
 * @title: LatexilaTemplatesPersonal
 * @short_description: Personal templates
 * @see_also: #LatexilaTemplatesDefault
 *
 * #LatexilaTemplatesPersonal is a singleton class that stores information about
 * pesonal templates.
 *
 * Personal templates are stored in the ~/.local/share/latexila/ directory.
 * There is a templatesrc file that stores the list of names and icons. And the
 * templates' contents are stored in 0.tex, 1.tex, 2.tex, etc, in the same order
 * as in the templatesrc file.
 */

#include "config.h"
#include "latexila-templates-personal.h"
#include "latexila-templates-common.h"

struct _LatexilaTemplatesPersonal
{
  GtkListStore parent;
};

G_DEFINE_TYPE (LatexilaTemplatesPersonal, latexila_templates_personal, GTK_TYPE_LIST_STORE)

static void
latexila_templates_personal_class_init (LatexilaTemplatesPersonalClass *klass)
{
}

static GFile *
get_rc_file (void)
{
  gchar *path;
  GFile *rc_file;

  path = g_build_filename (g_get_user_data_dir (), "latexila", "templatesrc", NULL);
  rc_file = g_file_new_for_path (path);

  g_free (path);
  return rc_file;
}

static GFile *
get_personal_template_file (gint template_num)
{
  gchar *filename;
  gchar *path;
  GFile *template_file;

  filename = g_strdup_printf ("%d.tex", template_num);
  path = g_build_filename (g_get_user_data_dir (), "latexila", filename, NULL);
  template_file = g_file_new_for_path (path);

  g_free (filename);
  g_free (path);
  return template_file;
}

static void
rc_file_contents_loaded_cb (GFile                     *rc_file,
                            GAsyncResult              *result,
                            LatexilaTemplatesPersonal *templates)
{
  gchar *contents = NULL;
  gsize length;
  GKeyFile *key_file = NULL;
  gchar **names = NULL;
  gchar **icons = NULL;
  gsize n_names;
  gsize n_icons;
  gint i;
  GError *error = NULL;

  g_file_load_contents_finish (rc_file, result, &contents, &length, NULL, &error);

  if (error != NULL)
    {
      /* If the rc file doesn't exist, it means that there is no personal
       * templates.
       */
      if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
        {
          g_error_free (error);
          error = NULL;
        }

      goto out;
    }

  key_file = g_key_file_new ();
  g_key_file_load_from_data (key_file, contents, length, G_KEY_FILE_NONE, &error);

  if (error != NULL)
    goto out;

  names = g_key_file_get_string_list (key_file, PACKAGE_NAME, "names", &n_names, &error);

  if (error != NULL)
    goto out;

  icons = g_key_file_get_string_list (key_file, PACKAGE_NAME, "icons", &n_icons, &error);

  if (error != NULL)
    goto out;

  g_return_if_fail (n_names == n_icons);

  for (i = 0; i < n_names; i++)
    {
      GFile *template_file;

      template_file = get_personal_template_file (i);

      latexila_templates_add_template (GTK_LIST_STORE (templates),
                                       names[i],
                                       icons[i],
                                       template_file);

      g_object_unref (template_file);
    }

out:

  if (error != NULL)
    {
      g_warning ("The loading of personal templates failed: %s", error->message);
      g_error_free (error);
    }

  g_free (contents);
  g_strfreev (names);
  g_strfreev (icons);

  if (key_file != NULL)
    g_key_file_unref (key_file);

  /* Async operation finished. */
  g_object_unref (templates);
}

static void
latexila_templates_personal_init (LatexilaTemplatesPersonal *templates)
{
  GFile *rc_file;

  latexila_templates_init_store (GTK_LIST_STORE (templates));

  rc_file = get_rc_file ();

  /* Prevent @templates from being destroyed during the async operation. */
  g_object_ref (templates);

  g_file_load_contents_async (rc_file,
                              NULL,
                              (GAsyncReadyCallback) rc_file_contents_loaded_cb,
                              templates);
}

/**
 * latexila_templates_personal_get_instance:
 *
 * Gets the instance of the #LatexilaTemplatesPersonal singleton.
 *
 * Returns: (transfer none): the instance of #LatexilaTemplatesPersonal.
 */
LatexilaTemplatesPersonal *
latexila_templates_personal_get_instance (void)
{
  static LatexilaTemplatesPersonal *instance = NULL;

  if (instance == NULL)
    instance = g_object_new (LATEXILA_TYPE_TEMPLATES_PERSONAL, NULL);

  return instance;
}

/**
 * latexila_templates_personal_get_contents:
 * @templates: the #LatexilaTemplatesPersonal instance.
 * @path: the #GtkTreePath of a personal template.
 *
 * Gets the contents of a personal template.
 *
 * TODO load contents asynchronously, with a #GtkSourceFileLoader.
 *
 * Returns: the personal template's contents. Free with g_free().
 */
gchar *
latexila_templates_personal_get_contents (LatexilaTemplatesPersonal *templates,
                                          GtkTreePath               *path)
{
  GtkTreeIter iter;
  GFile *file;
  gchar *contents = NULL;
  GError *error = NULL;

  g_return_val_if_fail (LATEXILA_IS_TEMPLATES_PERSONAL (templates), NULL);

  gtk_tree_model_get_iter (GTK_TREE_MODEL (templates),
                           &iter,
                           path);

  gtk_tree_model_get (GTK_TREE_MODEL (templates),
                      &iter,
                      LATEXILA_TEMPLATES_COLUMN_FILE, &file,
                      -1);

  g_return_val_if_fail (G_IS_FILE (file), NULL);

  g_file_load_contents (file, NULL, &contents, NULL, NULL, &error);

  if (error != NULL)
    {
      g_warning ("Error when loading personal template contents: %s", error->message);
      g_error_free (error);
    }

  g_object_unref (file);
  return contents;
}
