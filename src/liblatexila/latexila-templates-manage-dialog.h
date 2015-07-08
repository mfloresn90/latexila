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

#ifndef __LATEXILA_TEMPLATES_MANAGE_DIALOG_H__
#define __LATEXILA_TEMPLATES_MANAGE_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LATEXILA_TYPE_TEMPLATES_MANAGE_DIALOG latexila_templates_manage_dialog_get_type ()

G_DECLARE_FINAL_TYPE (LatexilaTemplatesManageDialog,
                      latexila_templates_manage_dialog,
                      LATEXILA,
                      TEMPLATES_MANAGE_DIALOG,
                      GtkDialog)

GtkDialog *   latexila_templates_manage_dialog_new    (GtkWindow *parent_window);

G_END_DECLS

#endif /* __LATEXILA_TEMPLATES_MANAGE_DIALOG_H__ */
