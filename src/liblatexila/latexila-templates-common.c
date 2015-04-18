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

/* Common functions between default and personal templates. */

#include "latexila-templates-common.h"

void
latexila_templates_init_store (GtkListStore *store)
{
  GType types[] = {G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FILE};

  gtk_list_store_set_column_types (store,
                                   LATEXILA_TEMPLATES_N_COLUMNS,
                                   types);
}

/* For compatibility reasons. @config_icon_name is the string stored in the rc
 * file, and the return value is the theme icon name used for the pixbuf. If we
 * store directly the theme icon names in the rc file, old rc files must be
 * modified via a script for example, but it's simpler like that.
 * The config_icon_name can also be seen as the template _type_.
 */
static const gchar *
get_pixbuf_icon_name (const gchar *config_icon_name)
{
  g_return_val_if_fail (config_icon_name != NULL, NULL);

  if (g_str_equal (config_icon_name, "empty"))
    return "text-x-preview";

  if (g_str_equal (config_icon_name, "article"))
    return "text-x-generic";

  if (g_str_equal (config_icon_name, "report"))
    return "x-office-document";

  if (g_str_equal (config_icon_name, "book"))
    return "accessories-dictionary";

  if (g_str_equal (config_icon_name, "letter"))
    return "emblem-mail";

  if (g_str_equal (config_icon_name, "beamer"))
    return "x-office-presentation";

  g_return_val_if_reached (NULL);
}

void
latexila_templates_add_template (GtkListStore *store,
                                 const gchar  *name,
                                 const gchar  *config_icon_name,
                                 GFile        *file)
{
  GtkTreeIter iter;
  const gchar *pixbuf_icon_name;

  pixbuf_icon_name = get_pixbuf_icon_name (config_icon_name);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      LATEXILA_TEMPLATES_COLUMN_PIXBUF_ICON_NAME, pixbuf_icon_name,
                      LATEXILA_TEMPLATES_COLUMN_CONFIG_ICON_NAME, config_icon_name,
                      LATEXILA_TEMPLATES_COLUMN_NAME, name,
                      LATEXILA_TEMPLATES_COLUMN_FILE, file,
                      -1);
}

/**
 * latexila_templates_get_view:
 * @store: the #LatexilaTemplatesDefault or #LatexilaTemplatesPersonal instance.
 *
 * Returns: (transfer floating): a beautiful #GtkTreeView, just for you.
 */
GtkTreeView *
latexila_templates_get_view (GtkListStore *store)
{
  GtkTreeView *view;
  GtkTreeSelection *selection;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  view = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (store)));
  gtk_tree_view_set_headers_visible (view, FALSE);
  gtk_widget_set_hexpand (GTK_WIDGET (view), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (view), TRUE);

  selection = gtk_tree_view_get_selection (view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

  /* Icon */
  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (renderer, "stock-size", GTK_ICON_SIZE_BUTTON, NULL);

  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "icon-name",
                                                     LATEXILA_TEMPLATES_COLUMN_PIXBUF_ICON_NAME,
                                                     NULL);

  gtk_tree_view_append_column (view, column);

  /* Name */
  renderer = gtk_cell_renderer_text_new ();

  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "text",
                                                     LATEXILA_TEMPLATES_COLUMN_NAME,
                                                     NULL);

  gtk_tree_view_append_column (view, column);

  return view;
}
