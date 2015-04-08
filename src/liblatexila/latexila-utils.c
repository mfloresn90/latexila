/*
 * This file is part of LaTeXila.
 *
 * From gedit-utils.c:
 * Copyright (C) 1998, 1999 - Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2002 - Chema Celorio, Paolo Maggi
 * Copyright (C) 2003-2005 - Paolo Maggi
 *
 * Copyright (C) 2014-2015 - Sébastien Wilmet <swilmet@gnome.org>
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
 * SECTION:utils
 * @title: LatexilaUtils
 * @short_description: Utilities functions
 *
 * Various utilities functions.
 */

#include "latexila-utils.h"
#include <string.h>
#include "latexila-synctex.h"

static gint
get_extension_position (const gchar *filename)
{
  const gchar *pos;
  gint length;

  if (filename == NULL)
    return 0;

  length = strlen (filename);
  pos = filename + length;
  g_assert (pos[0] == '\0');

  while (TRUE)
    {
      pos = g_utf8_find_prev_char (filename, pos);

      if (pos == NULL || pos[0] == '/')
        break;

      if (pos[0] == '.')
        return pos - filename;
    }

  return length;
}

/**
 * latexila_utils_get_shortname:
 * @filename: a filename.
 *
 * Returns: the @filename without its extension. Free with g_free().
 */
gchar *
latexila_utils_get_shortname (const gchar *filename)
{
  return g_strndup (filename, get_extension_position (filename));
}

/**
 * latexila_utils_get_extension:
 * @filename: a filename.
 *
 * Returns: the @filename's extension with the dot, in lowercase. Free with
 * g_free().
 */
gchar *
latexila_utils_get_extension (const gchar *filename)
{
  gint pos = get_extension_position (filename);
  return g_ascii_strdown (filename + pos, -1);
}

/**
 * latexila_utils_replace_home_dir_with_tilde:
 * @filename: the filename.
 *
 * Replaces the home directory with a tilde, if the home directory is present in
 * the @filename.
 *
 * This function comes from gedit.
 *
 * Returns: the new filename. Free with g_free().
 */
gchar *
latexila_utils_replace_home_dir_with_tilde (const gchar *filename)
{
  gchar *tmp;
  gchar *home;

  g_return_val_if_fail (filename != NULL, NULL);

  /* Note that g_get_home_dir returns a const string */
  tmp = (gchar *) g_get_home_dir ();

  if (tmp == NULL)
    return g_strdup (filename);

  home = g_filename_to_utf8 (tmp, -1, NULL, NULL, NULL);
  if (home == NULL)
    return g_strdup (filename);

  if (strcmp (filename, home) == 0)
    {
      g_free (home);
      return g_strdup ("~");
    }

  tmp = home;
  home = g_strdup_printf ("%s/", tmp);
  g_free (tmp);

  if (g_str_has_prefix (filename, home))
    {
      gchar *res = g_strdup_printf ("~/%s", filename + strlen (home));
      g_free (home);
      return res;
    }

  g_free (home);
  return g_strdup (filename);
}

/**
 * latexila_utils_register_icons:
 *
 * Register the LaTeXila icons to the #GtkIconTheme as built-in icons. For
 * example the icon located at data/images/stock-icons/badbox.png in the
 * LaTeXila git repository will be available with the icon name "badbox". The
 * "stock-icons" directory name is for historical reasons and should be changed
 * when stock icons are no longer used in LaTeXila.
 *
 * TODO: prefix icon names with "latexila-", so the badbox.png would be
 * "latexila-badbox".
 */
void
latexila_utils_register_icons (void)
{
  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_default (),
                                    "/org/gnome/latexila/stock-icons/");
}

/**
 * latexila_utils_get_pixbuf_from_icon_name:
 * @icon_name: an icon name.
 * @icon_size: an icon size.
 *
 * Returns: (nullable) (transfer full): the corresponding #GdkPixbuf.
 */
GdkPixbuf *
latexila_utils_get_pixbuf_from_icon_name (const gchar *icon_name,
                                          GtkIconSize  icon_size)
{
  gint size;
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  gtk_icon_size_lookup (icon_size, &size, NULL);

  pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                     icon_name,
                                     size,
                                     0,
                                     &error);

  if (error != NULL)
    {
      g_warning ("Error when loading icon \"%s\": %s", icon_name, error->message);
      g_error_free (error);
    }

  return pixbuf;
}

/**
 * latexila_utils_str_replace:
 * @string: a string
 * @search: the search string
 * @replacement: the replacement string
 *
 * Replaces all occurences of @search by @replacement.
 *
 * Returns: A newly allocated string with the replacements. Free with g_free().
 */
gchar *
latexila_utils_str_replace (const gchar *string,
                            const gchar *search,
                            const gchar *replacement)
{
  gchar **chunks;
  gchar *ret;

  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (search != NULL, NULL);
  g_return_val_if_fail (replacement != NULL, NULL);

  chunks = g_strsplit (string, search, -1);
  if (chunks != NULL && chunks[0] != NULL)
    ret = g_strjoinv (replacement, chunks);
  else
    ret = g_strdup (string);

  g_strfreev (chunks);
  return ret;
}

/**
 * latexila_utils_file_query_exists_async:
 * @file: a #GFile.
 * @cancellable: a #GCancellable.
 * @callback: the callback to call when the operation is finished.
 * @user_data: the data to pass to the callback function.
 *
 * The asynchronous version of g_file_query_exists(). When the operation is
 * finished, @callback will be called. You can then call
 * latexila_utils_file_query_exists_finish() to get the result of the operation.
 */
void
latexila_utils_file_query_exists_async (GFile               *file,
                                        GCancellable        *cancellable,
                                        GAsyncReadyCallback  callback,
                                        gpointer             user_data)
{
  g_file_query_info_async (file,
                           G_FILE_ATTRIBUTE_STANDARD_TYPE,
                           G_FILE_QUERY_INFO_NONE,
                           G_PRIORITY_DEFAULT,
                           cancellable,
                           callback,
                           user_data);
}

/**
 * latexila_utils_file_query_exists_finish:
 * @file: a #GFile.
 * @result: a #GAsyncResult.
 *
 * Finishes the operation started with latexila_utils_file_query_exists_async().
 * There is no output #GError parameter, so you should check if the operation
 * has been cancelled (in which case %FALSE will be returned).
 *
 * Returns: %TRUE if the file exists and the operation hasn't been cancelled,
 * %FALSE otherwise.
 */
gboolean
latexila_utils_file_query_exists_finish (GFile        *file,
                                         GAsyncResult *result)
{
  GFileInfo *info = g_file_query_info_finish (file, result, NULL);

  if (info != NULL)
    {
      g_object_unref (info);
      return TRUE;
    }

  return FALSE;
}

static gboolean
default_document_viewer_is_evince (const gchar *uri)
{
  GFile *file;
  GAppInfo *app_info;
  const gchar *executable;
  gboolean ret;
  GError *error = NULL;

  file = g_file_new_for_uri (uri);
  app_info = g_file_query_default_handler (file, NULL, &error);
  g_object_unref (file);

  if (error != NULL)
    {
      g_warning ("Impossible to know if evince is the default document viewer: %s",
                 error->message);

      g_error_free (error);
      return FALSE;
    }

  executable = g_app_info_get_executable (app_info);
  ret = strstr (executable, "evince") != NULL;

  g_object_unref (app_info);
  return ret;
}

/**
 * latexila_utils_show_uri:
 * @screen: (nullable): a #GdkScreen, or %NULL.
 * @uri: the URI to show.
 * @timestamp: a timestamp.
 * @error: (out) (optional): a %NULL #GError, or %NULL.
 *
 * Shows the @uri. If the URI is a PDF file and if Evince is the default
 * document viewer, this function also connects the Evince window so the
 * backward search works (switch from the PDF to the source file).
 */
void
latexila_utils_show_uri (GdkScreen    *screen,
                         const gchar  *uri,
                         guint32       timestamp,
                         GError      **error)
{
  g_return_if_fail (uri != NULL);
  g_return_if_fail (error == NULL || *error == NULL);

  if (gtk_show_uri (screen, uri, timestamp, error))
    {
      gchar *extension = latexila_utils_get_extension (uri);

      if (g_strcmp0 (extension, ".pdf") == 0 &&
          default_document_viewer_is_evince (uri))
        {
          LatexilaSynctex *synctex = latexila_synctex_get_instance ();
          latexila_synctex_connect_evince_window (synctex, uri);
        }

      g_free (extension);
    }
}

/**
 * latexila_utils_get_dialog_component:
 * @title: the title of the dialog component.
 * @widget: the widget displayed below the title.
 *
 * Gets a #GtkDialog component. When a dialog contains several components, or
 * logical groups, this function is useful to attach the @widget with a @title.
 * The title will be in bold, left-aligned, and the widget will have a left
 * margin.
 *
 * Returns: (transfer floating): the dialog component containing the @title and
 * the @widget.
 */
GtkWidget *
latexila_utils_get_dialog_component (const gchar *title,
                                     GtkWidget   *widget)
{
  GtkContainer *grid;
  GtkWidget *label;
  gchar *markup;

  grid = GTK_CONTAINER (gtk_grid_new ());
  gtk_orientable_set_orientation (GTK_ORIENTABLE (grid), GTK_ORIENTATION_VERTICAL);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 6);
  gtk_container_set_border_width (grid, 6);

  /* Title in bold, left-aligned. */
  label = gtk_label_new (NULL);
  markup = g_strdup_printf ("<b>%s</b>", title);
  gtk_label_set_markup (GTK_LABEL (label), markup);
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_container_add (grid, label);

  /* Left margin for the widget. */
  gtk_widget_set_margin_start (widget, 12);
  gtk_container_add (grid, widget);

  g_free (markup);
  return GTK_WIDGET (grid);
}
