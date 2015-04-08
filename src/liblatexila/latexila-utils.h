/*
 * This file is part of LaTeXila.
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

#ifndef __LATEXILA_UTILS_H__
#define __LATEXILA_UTILS_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

gchar *         latexila_utils_get_shortname                    (const gchar *filename);

gchar *         latexila_utils_get_extension                    (const gchar *filename);

gchar *         latexila_utils_replace_home_dir_with_tilde      (const gchar *filename);

void            latexila_utils_register_icons                   (void);

GdkPixbuf *     latexila_utils_get_pixbuf_from_icon_name        (const gchar *icon_name,
                                                                 GtkIconSize  icon_size);

gchar *         latexila_utils_str_replace                      (const gchar *string,
                                                                 const gchar *search,
                                                                 const gchar *replacement);

void            latexila_utils_file_query_exists_async          (GFile               *file,
                                                                 GCancellable        *cancellable,
                                                                 GAsyncReadyCallback  callback,
                                                                 gpointer             user_data);

gboolean        latexila_utils_file_query_exists_finish         (GFile        *file,
                                                                 GAsyncResult *result);

void            latexila_utils_show_uri                         (GdkScreen    *screen,
                                                                 const gchar  *uri,
                                                                 guint32       timestamp,
                                                                 GError      **error);

GtkWidget *     latexila_utils_get_dialog_component             (const gchar *title,
                                                                 GtkWidget   *widget);

G_END_DECLS

#endif /* __LATEXILA_UTILS_H__ */
