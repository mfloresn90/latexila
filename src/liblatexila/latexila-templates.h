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

#ifndef __LATEXILA_TEMPLATES_H__
#define __LATEXILA_TEMPLATES_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LATEXILA_TYPE_TEMPLATES latexila_templates_get_type ()
G_DECLARE_FINAL_TYPE (LatexilaTemplates, latexila_templates, LATEXILA, TEMPLATES, GObject)

LatexilaTemplates *   latexila_templates_get_instance                           (void);

GtkTreeView *         latexila_templates_get_default_templates_view             (LatexilaTemplates *templates);

GtkTreeView *         latexila_templates_get_personal_templates_view            (LatexilaTemplates *templates);

gchar *               latexila_templates_get_default_template_contents          (LatexilaTemplates *templates,
                                                                                 GtkTreePath       *path);

gchar *               latexila_templates_get_personal_template_contents         (LatexilaTemplates *templates,
                                                                                 GtkTreePath       *path);

G_END_DECLS

#endif /* __LATEXILA_TEMPLATES_H__ */
