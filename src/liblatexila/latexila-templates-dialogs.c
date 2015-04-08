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

#include "latexila-templates-dialogs.h"
#include <glib/gi18n.h>
#include "latexila-templates.h"
#include "latexila-utils.h"

static void
init_open_dialog (GtkDialog   *dialog,
                  GtkTreeView *default_templates,
                  GtkTreeView *personal_templates)
{
  GtkContainer *hgrid;
  GtkWidget *scrolled_window;
  GtkWidget *component;
  GtkWidget *content_area;

  hgrid = GTK_CONTAINER (gtk_grid_new ());
  gtk_orientable_set_orientation (GTK_ORIENTABLE (hgrid), GTK_ORIENTATION_HORIZONTAL);
  gtk_grid_set_column_spacing (GTK_GRID (hgrid), 10);

  /* Default templates */

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
  gtk_widget_set_size_request (scrolled_window, 250, 200);

  gtk_container_add (GTK_CONTAINER (scrolled_window),
                     GTK_WIDGET (default_templates));

  component = latexila_utils_get_dialog_component (_("Default Templates"), scrolled_window);
  gtk_container_add (hgrid, component);

  /* Personal templates */

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
  gtk_widget_set_size_request (scrolled_window, 250, 200);

  gtk_container_add (GTK_CONTAINER (scrolled_window),
                     GTK_WIDGET (personal_templates));

  component = latexila_utils_get_dialog_component (_("Personal Templates"), scrolled_window);
  gtk_container_add (hgrid, component);

  content_area = gtk_dialog_get_content_area (dialog);
  gtk_box_pack_start (GTK_BOX (content_area), GTK_WIDGET (hgrid), TRUE, TRUE, 0);
  gtk_widget_show_all (content_area);
}

static void
selection_changed_cb (GtkTreeSelection *selection,
                      GtkTreeSelection *other_selection)
{
  /* Only one item of the two lists can be selected at once. */

  /* We unselect all the items of the other list only if the current list have
   * an item selected, because when we unselect all the items the "changed"
   * signal is emitted for the other list, so for the other list this function
   * is also called but no item is selected so nothing is done and the item
   * selected by the user is kept selected.
   */

  if (gtk_tree_selection_count_selected_rows (selection) > 0)
    gtk_tree_selection_unselect_all (other_selection);
}

static void
row_activated_cb (GtkTreeView       *tree_view,
                  GtkTreePath       *path,
                  GtkTreeViewColumn *column,
                  GtkDialog         *dialog)
{
  gtk_dialog_response (dialog, GTK_RESPONSE_OK);
}

/**
 * latexila_templates_dialogs_open:
 * @parent_window: transient parent window of the dialog.
 *
 * Runs a #GtkDialog to create a new document from a template. Only the
 * template's contents is returned.
 *
 * Returns: the template contents, or %NULL if no templates must be opened.
 */
gchar *
latexila_templates_dialogs_open (GtkWindow *parent_window)
{
  GtkDialog *dialog;
  LatexilaTemplates *templates;
  GtkTreeView *default_templates;
  GtkTreeView *personal_templates;
  GtkTreeSelection *default_selection;
  GtkTreeSelection *personal_selection;
  gint response;
  gchar *contents = NULL;

  dialog = g_object_new (GTK_TYPE_DIALOG,
                         "use-header-bar", TRUE,
                         "title", _("New File..."),
                         "destroy-with-parent", TRUE,
                         "transient-for", parent_window,
                         NULL);

  gtk_dialog_add_buttons (dialog,
                          _("_Cancel"), GTK_RESPONSE_CANCEL,
                          _("_New"), GTK_RESPONSE_OK,
                          NULL);

  gtk_dialog_set_default_response (dialog, GTK_RESPONSE_OK);

  templates = latexila_templates_get_instance ();
  default_templates = latexila_templates_get_default_templates_view (templates);
  personal_templates = latexila_templates_get_personal_templates_view (templates);

  init_open_dialog (dialog, default_templates, personal_templates);

  /* Selection: at most one selected template in both GtkTreeViews. */
  default_selection = gtk_tree_view_get_selection (default_templates);
  personal_selection = gtk_tree_view_get_selection (personal_templates);

  g_signal_connect_object (default_selection,
                           "changed",
                           G_CALLBACK (selection_changed_cb),
                           personal_selection,
                           0);

  g_signal_connect_object (personal_selection,
                           "changed",
                           G_CALLBACK (selection_changed_cb),
                           default_selection,
                           0);

  /* Double-click */
  g_signal_connect (default_templates,
                    "row-activated",
                    G_CALLBACK (row_activated_cb),
                    dialog);

  g_signal_connect (personal_templates,
                    "row-activated",
                    G_CALLBACK (row_activated_cb),
                    dialog);


  response = gtk_dialog_run (dialog);

  if (response == GTK_RESPONSE_OK)
    {
      GList *selected_rows = NULL;
      GtkTreePath *path;

      if (gtk_tree_selection_count_selected_rows (default_selection) > 0)
        {
          selected_rows = gtk_tree_selection_get_selected_rows (default_selection, NULL);
          g_assert (g_list_length (selected_rows) == 1);

          path = selected_rows->data;
          contents = latexila_templates_get_default_template_contents (templates, path);
        }

      else if (gtk_tree_selection_count_selected_rows (personal_selection) > 0)
        {
          selected_rows = gtk_tree_selection_get_selected_rows (personal_selection, NULL);
          g_assert (g_list_length (selected_rows) == 1);

          path = selected_rows->data;
          contents = latexila_templates_get_personal_template_contents (templates, path);
        }

      /* No templates selected. */
      else
        {
          contents = g_strdup ("");
        }

      g_list_free_full (selected_rows, (GDestroyNotify) gtk_tree_path_free);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
  return contents;
}
