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
 * There is a templatesrc file that stores the list of names, icons and
 * files.
 */

#include "config.h"
#include "latexila-templates-personal.h"
#include <string.h>
#include <stdlib.h>
#include "latexila-templates-common.h"
#include "latexila-utils.h"

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
get_personal_template_file_by_filename (const gchar *filename)
{
  gchar *path;
  GFile *template_file;

  path = g_build_filename (g_get_user_data_dir (), "latexila", filename, NULL);
  template_file = g_file_new_for_path (path);

  g_free (path);
  return template_file;
}

static GFile *
get_personal_template_file_by_index (gint template_num)
{
  gchar *filename;
  GFile *template_file;

  filename = g_strdup_printf ("%d.tex", template_num);
  template_file = get_personal_template_file_by_filename (filename);

  g_free (filename);
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
  gchar **files = NULL;
  gsize n_names;
  gsize n_icons;
  gsize n_files;
  gboolean has_files;
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

  has_files = g_key_file_has_key (key_file, PACKAGE_NAME, "files", &error);

  if (error != NULL)
    goto out;

  if (has_files)
    {
      files = g_key_file_get_string_list (key_file, PACKAGE_NAME, "files", &n_files, &error);

      if (error != NULL)
        goto out;

      g_return_if_fail (n_names == n_files);
    }

  for (i = 0; i < n_names; i++)
    {
      GFile *template_file;

      if (has_files)
        template_file = get_personal_template_file_by_filename (files[i]);
      else
        template_file = get_personal_template_file_by_index (i);

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
  g_return_val_if_fail (path != NULL, NULL);

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

static gboolean
save_rc_file (LatexilaTemplatesPersonal  *templates,
              GError                    **error)
{
  GFile *rc_file;
  gchar *rc_path = NULL;
  gint personal_templates_count;
  gchar **names = NULL;
  gchar **icons = NULL;
  gchar **files = NULL;
  GtkTreeIter iter;
  gint template_num;
  GKeyFile *key_file = NULL;
  gboolean ret = TRUE;

  rc_file = get_rc_file ();
  personal_templates_count = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (templates), NULL);

  if (personal_templates_count == 0)
    {
      GError *my_error = NULL;

      g_file_delete (rc_file, NULL, &my_error);

      if (g_error_matches (my_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
        {
          g_error_free (my_error);
          my_error = NULL;
        }
      else if (my_error != NULL)
        {
          ret = FALSE;
          g_propagate_error (error, my_error);
        }

      goto out;
    }

  names = g_new0 (gchar *, personal_templates_count + 1);
  icons = g_new0 (gchar *, personal_templates_count + 1);
  files = g_new0 (gchar *, personal_templates_count + 1);

  if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (templates), &iter))
    g_assert_not_reached ();

  template_num = 0;
  do
    {
      gchar *name;
      gchar *icon;
      GFile *file;

      gtk_tree_model_get (GTK_TREE_MODEL (templates), &iter,
                          LATEXILA_TEMPLATES_COLUMN_NAME, &name,
                          LATEXILA_TEMPLATES_COLUMN_CONFIG_ICON_NAME, &icon,
                          LATEXILA_TEMPLATES_COLUMN_FILE, &file,
                          -1);

      g_assert_cmpint (template_num, <, personal_templates_count);
      names[template_num] = name;
      icons[template_num] = icon;
      files[template_num] = g_file_get_basename (file);

      g_object_unref (file);
      template_num++;
    }
  while (gtk_tree_model_iter_next (GTK_TREE_MODEL (templates), &iter));

  g_assert_cmpint (template_num, ==, personal_templates_count);

  key_file = g_key_file_new ();

  g_key_file_set_string_list (key_file,
                              PACKAGE_NAME,
                              "names",
                              (const gchar * const *) names,
                              personal_templates_count);

  g_key_file_set_string_list (key_file,
                              PACKAGE_NAME,
                              "icons",
                              (const gchar * const *) icons,
                              personal_templates_count);

  g_key_file_set_string_list (key_file,
                              PACKAGE_NAME,
                              "files",
                              (const gchar * const *) files,
                              personal_templates_count);

  rc_path = g_file_get_path (rc_file);
  if (!g_key_file_save_to_file (key_file, rc_path, error))
    ret = FALSE;

out:
  g_object_unref (rc_file);
  g_free (rc_path);
  g_strfreev (names);
  g_strfreev (icons);
  g_strfreev (files);

  if (key_file != NULL)
    g_key_file_unref (key_file);

  return ret;
}

static gboolean
is_template_index_used (LatexilaTemplatesPersonal *templates,
                        gint                       template_num)
{
  GtkTreeIter iter;

  if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (templates), &iter))
    return FALSE;

  do
    {
      GFile *file;
      gchar *basename;
      gchar *endptr;
      glong cur_template_num;
      gboolean numeric_basename;

      gtk_tree_model_get (GTK_TREE_MODEL (templates), &iter,
                          LATEXILA_TEMPLATES_COLUMN_FILE, &file,
                          -1);

      basename = g_file_get_basename (file);
      cur_template_num = strtol (basename, &endptr, 10);
      numeric_basename = endptr != basename;

      g_object_unref (file);
      g_free (basename);

      if (numeric_basename && template_num == cur_template_num)
        return TRUE;
    }
  while (gtk_tree_model_iter_next (GTK_TREE_MODEL (templates), &iter));

  return FALSE;
}

static gint
get_first_free_template_index (LatexilaTemplatesPersonal *templates)
{
  gint template_num;

  for (template_num = 0; template_num <= G_MAXINT; template_num++)
    {
      if (!is_template_index_used (templates, template_num))
        return template_num;
    }

  g_return_val_if_reached (-1);
}

/**
 * latexila_templates_personal_create:
 * @templates: the #LatexilaTemplatesPersonal instance.
 * @name: the template's name.
 * @config_icon_name: the icon name that will be stored in the config file.
 * @contents: the template's contents.
 * @error: (out) (optional): a location to a %NULL #GError, or %NULL.
 *
 * Creates a new personal template. The new template is added at the end of the
 * list.
 *
 * Returns: %TRUE on success, %FALSE on error.
 */
gboolean
latexila_templates_personal_create (LatexilaTemplatesPersonal  *templates,
                                    const gchar                *name,
                                    const gchar                *config_icon_name,
                                    const gchar                *contents,
                                    GError                    **error)
{
  gint template_num;
  GFile *template_file = NULL;
  GFileOutputStream *stream = NULL;
  gboolean ret = TRUE;

  g_return_val_if_fail (LATEXILA_IS_TEMPLATES_PERSONAL (templates), FALSE);
  g_return_val_if_fail (name != NULL && name[0] != '\0', FALSE);
  g_return_val_if_fail (config_icon_name != NULL && config_icon_name[0] != '\0', FALSE);
  g_return_val_if_fail (contents != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  template_num = get_first_free_template_index (templates);
  g_return_val_if_fail (template_num >= 0, FALSE);

  template_file = get_personal_template_file_by_index (template_num);

  if (!latexila_utils_create_parent_directories (template_file, error))
    {
      ret = FALSE;
      goto out;
    }

  stream = g_file_create (template_file, G_FILE_CREATE_NONE, NULL, error);

  if (stream == NULL)
    {
      ret = FALSE;
      goto out;
    }

  if (!g_output_stream_write_all (G_OUTPUT_STREAM (stream),
                                  contents,
                                  strlen (contents),
                                  NULL,
                                  NULL,
                                  error))
    {
      ret = FALSE;
      goto out;
    }

  latexila_templates_add_template (GTK_LIST_STORE (templates),
                                   name,
                                   config_icon_name,
                                   template_file);

  if (!save_rc_file (templates, error))
    ret = FALSE;

out:
  g_clear_object (&template_file);
  g_clear_object (&stream);
  return ret;
}

/**
 * latexila_templates_personal_delete:
 * @templates: the #LatexilaTemplatesPersonal instance.
 * @iter: a valid #GtkTreeIter.
 * @error: (out) (optional): a location to a %NULL #GError, or %NULL.
 *
 * Deletes a personal template.
 *
 * Returns: %TRUE on success, %FALSE on error.
 */
gboolean
latexila_templates_personal_delete (LatexilaTemplatesPersonal  *templates,
                                    GtkTreeIter                *iter,
                                    GError                    **error)
{
  GFile *file = NULL;
  gboolean success = FALSE;

  g_return_val_if_fail (LATEXILA_IS_TEMPLATES_PERSONAL (templates), FALSE);
  g_return_val_if_fail (iter != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  gtk_tree_model_get (GTK_TREE_MODEL (templates),
                      iter,
                      LATEXILA_TEMPLATES_COLUMN_FILE, &file,
                      -1);

  g_return_val_if_fail (G_IS_FILE (file), FALSE);

  gtk_list_store_remove (GTK_LIST_STORE (templates), iter);

  if (!save_rc_file (templates, error))
    goto out;

  if (!g_file_delete (file, NULL, error))
    goto out;

  success = TRUE;

out:
  g_clear_object (&file);
  return success;
}
