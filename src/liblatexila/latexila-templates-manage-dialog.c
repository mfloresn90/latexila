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
 * SECTION:templates-manage-dialog
 * @title: LatexilaTemplatesManageDialog
 * @short_description: Dialog to manage personal templates
 * @see_also: #LatexilaTemplatesPersonal
 *
 * #LatexilaTemplatesManageDialog is a #GtkDialog to manage personal templates.
 * The implemented actions: delete.
 */

#include "latexila-templates-manage-dialog.h"
#include <glib/gi18n.h>
#include "latexila-templates-personal.h"
#include "latexila-templates-common.h"
#include "latexila-utils.h"

struct _LatexilaTemplatesManageDialog
{
  GtkDialog parent;

  GtkTreeView *templates_view;

  GtkToolButton *delete_button;
};

G_DEFINE_TYPE (LatexilaTemplatesManageDialog, latexila_templates_manage_dialog, GTK_TYPE_DIALOG)

static void
delete_button_clicked_cb (GtkToolButton                 *delete_button,
                          LatexilaTemplatesManageDialog *manage_dialog)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  LatexilaTemplatesPersonal *templates_store;
  GtkTreeIter iter;
  gchar *name;
  GtkDialog *confirm_dialog;
  gint response;

  selection = gtk_tree_view_get_selection (manage_dialog->templates_view);

  if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    g_return_if_reached ();

  templates_store = latexila_templates_personal_get_instance ();
  g_return_if_fail (GTK_TREE_MODEL (templates_store) == model);

  gtk_tree_model_get (model, &iter,
                      LATEXILA_TEMPLATES_COLUMN_NAME, &name,
                      -1);

  confirm_dialog = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW (manage_dialog),
                                                       GTK_DIALOG_MODAL |
                                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                                       GTK_MESSAGE_QUESTION,
                                                       GTK_BUTTONS_NONE,
                                                       _("Do you really want to delete the template “%s”?"),
                                                       name));

  gtk_dialog_add_buttons (confirm_dialog,
                          _("_Cancel"), GTK_RESPONSE_CANCEL,
                          _("_Delete"), GTK_RESPONSE_YES,
                          NULL);

  response = gtk_dialog_run (confirm_dialog);
  gtk_widget_destroy (GTK_WIDGET (confirm_dialog));

  if (response == GTK_RESPONSE_YES)
    {
      GError *error = NULL;

      latexila_templates_personal_delete (templates_store, &iter, &error);

      if (error != NULL)
        {
          GtkWidget *error_dialog;

          error_dialog = gtk_message_dialog_new (GTK_WINDOW (manage_dialog),
                                                 GTK_DIALOG_MODAL |
                                                 GTK_DIALOG_DESTROY_WITH_PARENT |
                                                 GTK_DIALOG_USE_HEADER_BAR,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_OK,
                                                 _("Error when deleting the template “%s”."),
                                                 name);

          gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (error_dialog),
                                                    "%s", error->message);

          gtk_dialog_run (GTK_DIALOG (error_dialog));
          gtk_widget_destroy (error_dialog);

          g_error_free (error);
        }
    }

  g_free (name);
}

static GtkToolbar *
init_toolbar (LatexilaTemplatesManageDialog *manage_dialog)
{
  GtkToolbar *toolbar;
  GtkStyleContext *context;

  toolbar = GTK_TOOLBAR (gtk_toolbar_new ());
  gtk_toolbar_set_icon_size (toolbar, GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_toolbar_set_style (toolbar, GTK_TOOLBAR_ICONS);

  context = gtk_widget_get_style_context (GTK_WIDGET (toolbar));
  gtk_style_context_add_class (context, GTK_STYLE_CLASS_INLINE_TOOLBAR);

  /* Delete */
  manage_dialog->delete_button = GTK_TOOL_BUTTON (gtk_tool_button_new (NULL, NULL));
  gtk_tool_button_set_icon_name (manage_dialog->delete_button, "list-remove-symbolic");
  gtk_widget_set_tooltip_text (GTK_WIDGET (manage_dialog->delete_button), _("Delete"));

  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (manage_dialog->delete_button), -1);

  g_signal_connect (manage_dialog->delete_button,
                    "clicked",
                    G_CALLBACK (delete_button_clicked_cb),
                    manage_dialog);

  return toolbar;
}

static void
update_buttons_sensitivity (LatexilaTemplatesManageDialog *manage_dialog)
{
  GtkTreeSelection *selection;
  gint n_selected_rows;

  selection = gtk_tree_view_get_selection (manage_dialog->templates_view);
  n_selected_rows = gtk_tree_selection_count_selected_rows (selection);

  gtk_widget_set_sensitive (GTK_WIDGET (manage_dialog->delete_button),
                            n_selected_rows > 0);
}

static void
latexila_templates_manage_dialog_class_init (LatexilaTemplatesManageDialogClass *klass)
{
}

static void
latexila_templates_manage_dialog_init (LatexilaTemplatesManageDialog *manage_dialog)
{
  LatexilaTemplatesPersonal *templates_store;
  GtkWidget *scrolled_window;
  GtkToolbar *toolbar;
  GtkWidget *dialog_content;
  GtkBox *content_area;
  GtkTreeSelection *selection;

  templates_store = latexila_templates_personal_get_instance ();
  manage_dialog->templates_view = latexila_templates_get_view (GTK_LIST_STORE (templates_store));

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_size_request (scrolled_window, 350, 300);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (scrolled_window),
                     GTK_WIDGET (manage_dialog->templates_view));

  toolbar = init_toolbar (manage_dialog);

  dialog_content = latexila_utils_join_widgets (scrolled_window, GTK_WIDGET (toolbar));

  content_area = GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (manage_dialog)));
  gtk_box_pack_start (content_area, dialog_content, TRUE, TRUE, 0);
  gtk_widget_show_all (GTK_WIDGET (content_area));

  selection = gtk_tree_view_get_selection (manage_dialog->templates_view);

  g_signal_connect_swapped (selection,
                            "changed",
                            G_CALLBACK (update_buttons_sensitivity),
                            manage_dialog);

  update_buttons_sensitivity (manage_dialog);
}

/**
 * latexila_templates_manage_dialog_new:
 * @parent_window: transient parent window of the dialog.
 *
 * Returns: a #GtkDialog to manage personal templates.
 */
GtkDialog *
latexila_templates_manage_dialog_new (GtkWindow *parent_window)
{
  g_return_val_if_fail (GTK_IS_WINDOW (parent_window), NULL);

  return g_object_new (LATEXILA_TYPE_TEMPLATES_MANAGE_DIALOG,
                       "title", _("Manage Personal Templates"),
                       "transient-for", parent_window,
                       "modal", TRUE,
                       "destroy-with-parent", TRUE,
                       "use-header-bar", TRUE,
                       "border-width", 6,
                       NULL);
}
