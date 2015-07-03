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

#include "latexila-templates-manage-dialog.h"
#include <glib/gi18n.h>
#include "latexila-templates-personal.h"
#include "latexila-templates-common.h"
#include "latexila-utils.h"

static GtkWidget *
get_toolbar (void)
{
  GtkToolbar *toolbar;
  GtkStyleContext *context;
  GtkToolButton *delete_button;

  toolbar = GTK_TOOLBAR (gtk_toolbar_new ());
  gtk_toolbar_set_icon_size (toolbar, GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_toolbar_set_style (toolbar, GTK_TOOLBAR_ICONS);

  context = gtk_widget_get_style_context (GTK_WIDGET (toolbar));
  gtk_style_context_add_class (context, GTK_STYLE_CLASS_INLINE_TOOLBAR);

  delete_button = GTK_TOOL_BUTTON (gtk_tool_button_new (NULL, NULL));
  gtk_tool_button_set_icon_name (delete_button, "list-remove-symbolic");
  gtk_widget_set_tooltip_text (GTK_WIDGET (delete_button), _("Delete"));

  gtk_toolbar_insert (toolbar, GTK_TOOL_ITEM (delete_button), -1);

  return GTK_WIDGET (toolbar);
}

static GtkWidget *
get_dialog_content (void)
{
  LatexilaTemplatesPersonal *templates_store;
  GtkTreeView *templates_view;
  GtkWidget *scrolled_window;
  GtkWidget *toolbar;

  templates_store = latexila_templates_personal_get_instance ();
  templates_view = latexila_templates_get_view (GTK_LIST_STORE (templates_store));

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_set_size_request (scrolled_window, 350, 300);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (scrolled_window),
                     GTK_WIDGET (templates_view));

  toolbar = get_toolbar ();

  return latexila_utils_join_widgets (scrolled_window, toolbar);
}

/**
 * latexila_templates_manage_dialog:
 * @parent_window: transient parent window of the dialog.
 *
 * Runs a #GtkDialog to manage personal templates.
 */
void
latexila_templates_manage_dialog (GtkWindow *parent_window)
{
  GtkDialog *dialog;
  GtkWidget *dialog_content;
  GtkBox *content_area;

  g_return_if_fail (GTK_IS_WINDOW (parent_window));

  dialog = GTK_DIALOG (gtk_dialog_new_with_buttons (_("Manage Personal Templates"),
                                                    parent_window,
                                                    GTK_DIALOG_MODAL |
                                                    GTK_DIALOG_DESTROY_WITH_PARENT |
                                                    GTK_DIALOG_USE_HEADER_BAR,
                                                    NULL, NULL));

  gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);

  dialog_content = get_dialog_content ();

  content_area = GTK_BOX (gtk_dialog_get_content_area (dialog));
  gtk_box_pack_start (content_area, dialog_content, TRUE, TRUE, 0);
  gtk_widget_show_all (GTK_WIDGET (content_area));

  gtk_dialog_run (dialog);
  gtk_widget_destroy (GTK_WIDGET (dialog));
}
