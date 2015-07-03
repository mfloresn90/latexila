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
#include "latexila-templates-common.h"
#include "latexila-templates-default.h"
#include "latexila-templates-personal.h"
#include "latexila-utils.h"

static void
init_open_dialog (GtkDialog   *dialog,
                  GtkTreeView *default_view,
                  GtkTreeView *personal_view)
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
                     GTK_WIDGET (default_view));

  component = latexila_utils_get_dialog_component (_("Default Templates"), scrolled_window);
  gtk_container_add (hgrid, component);

  /* Personal templates */

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
  gtk_widget_set_size_request (scrolled_window, 250, 200);

  gtk_container_add (GTK_CONTAINER (scrolled_window),
                     GTK_WIDGET (personal_view));

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
  LatexilaTemplatesDefault *default_store;
  LatexilaTemplatesPersonal *personal_store;
  GtkTreeView *default_view;
  GtkTreeView *personal_view;
  GtkTreeSelection *default_selection;
  GtkTreeSelection *personal_selection;
  gint response;
  gchar *contents = NULL;

  g_return_val_if_fail (GTK_IS_WINDOW (parent_window), NULL);

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

  default_store = latexila_templates_default_get_instance ();
  personal_store = latexila_templates_personal_get_instance ();

  default_view = latexila_templates_get_view (GTK_LIST_STORE (default_store));
  personal_view = latexila_templates_get_view (GTK_LIST_STORE (personal_store));

  init_open_dialog (dialog, default_view, personal_view);

  /* Selection: at most one selected template in both GtkTreeViews. */
  default_selection = gtk_tree_view_get_selection (default_view);
  personal_selection = gtk_tree_view_get_selection (personal_view);

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
  g_signal_connect (default_view,
                    "row-activated",
                    G_CALLBACK (row_activated_cb),
                    dialog);

  g_signal_connect (personal_view,
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
          contents = latexila_templates_default_get_contents (default_store, path);
        }

      else if (gtk_tree_selection_count_selected_rows (personal_selection) > 0)
        {
          selected_rows = gtk_tree_selection_get_selected_rows (personal_selection, NULL);
          g_assert (g_list_length (selected_rows) == 1);

          path = selected_rows->data;
          contents = latexila_templates_personal_get_contents (personal_store, path);
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

/**
 * latexila_templates_dialogs_create_template:
 * @parent_window: transient parent window of the dialog.
 * @template_contents: the template's contents.
 *
 * Runs a #GtkDialog to create a new template. The template's contents is given.
 * The #GtkDialog asks the template's name and icon.
 */
void
latexila_templates_dialogs_create_template (GtkWindow   *parent_window,
                                            const gchar *template_contents)
{
  GtkDialog *dialog;
  GtkBox *content_area;
  GtkEntry *entry;
  GtkWidget *component;
  LatexilaTemplatesDefault *default_store;
  GtkTreeView *default_view;
  GtkWidget *scrolled_window;

  g_return_if_fail (GTK_IS_WINDOW (parent_window));
  g_return_if_fail (template_contents != NULL);

  dialog = g_object_new (GTK_TYPE_DIALOG,
                         "use-header-bar", TRUE,
                         "title", _("New Template..."),
                         "destroy-with-parent", TRUE,
                         "transient-for", parent_window,
                         NULL);

  gtk_dialog_add_buttons (dialog,
                          _("_Cancel"), GTK_RESPONSE_CANCEL,
                          _("Crea_te"), GTK_RESPONSE_OK,
                          NULL);

  gtk_dialog_set_default_response (dialog, GTK_RESPONSE_OK);

  content_area = GTK_BOX (gtk_dialog_get_content_area (dialog));

  /* Name */
  entry = GTK_ENTRY (gtk_entry_new ());
  gtk_widget_set_hexpand (GTK_WIDGET (entry), TRUE);
  component = latexila_utils_get_dialog_component (_("Name of the new template"),
                                                   GTK_WIDGET (entry));
  gtk_box_pack_start (content_area, component, FALSE, TRUE, 0);

  /* Icon.
   * Take the default store because it contains all the icons.
   */
  default_store = latexila_templates_default_get_instance ();
  default_view = latexila_templates_get_view (GTK_LIST_STORE (default_store));

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_size_request (scrolled_window, 250, 200);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (scrolled_window),
                     GTK_WIDGET (default_view));

  component = latexila_utils_get_dialog_component (_("Choose an icon"), scrolled_window);
  gtk_box_pack_start (content_area, component, TRUE, TRUE, 0);

  gtk_widget_show_all (GTK_WIDGET (content_area));

  while (gtk_dialog_run (dialog) == GTK_RESPONSE_OK)
    {
      GtkTreeSelection *selection;
      GList *selected_rows;
      GtkTreePath *path;
      GtkTreeIter iter;
      gchar *config_icon_name = NULL;
      const gchar *name = NULL;
      LatexilaTemplatesPersonal *personal_store;
      GError *error = NULL;

      /* If no name specified. */
      if (gtk_entry_get_text_length (entry) == 0)
        continue;

      selection = gtk_tree_view_get_selection (default_view);

      /* If no icons selected. */
      if (gtk_tree_selection_count_selected_rows (selection) == 0)
        continue;

      /* Get config icon name. */
      selected_rows = gtk_tree_selection_get_selected_rows (selection, NULL);
      g_assert (g_list_length (selected_rows) == 1);

      path = selected_rows->data;

      if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (default_store), &iter, path))
        {
          g_warning ("Create template dialog: invalid path");
          break;
        }

      gtk_tree_model_get (GTK_TREE_MODEL (default_store), &iter,
                          LATEXILA_TEMPLATES_COLUMN_CONFIG_ICON_NAME, &config_icon_name,
                          -1);

      name = gtk_entry_get_text (entry);

      personal_store = latexila_templates_personal_get_instance ();

      latexila_templates_personal_create (personal_store,
                                          name,
                                          config_icon_name,
                                          template_contents,
                                          &error);

      g_list_free_full (selected_rows, (GDestroyNotify) gtk_tree_path_free);
      g_free (config_icon_name);

      if (error != NULL)
        {
          GtkWidget *error_dialog;

          error_dialog = gtk_message_dialog_new (GTK_WINDOW (dialog),
                                                 GTK_DIALOG_MODAL |
                                                 GTK_DIALOG_DESTROY_WITH_PARENT |
                                                 GTK_DIALOG_USE_HEADER_BAR,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 "%s", _("Impossible to create the personal template."));

          gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (error_dialog),
                                                    "%s", error->message);

          gtk_dialog_run (GTK_DIALOG (error_dialog));
          gtk_widget_destroy (error_dialog);

          g_error_free (error);
          continue;
        }

      break;
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}
