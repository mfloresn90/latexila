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

#ifndef __LATEXILA_TEMPLATES_PERSONAL_H__
#define __LATEXILA_TEMPLATES_PERSONAL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LATEXILA_TYPE_TEMPLATES_PERSONAL latexila_templates_personal_get_type ()
G_DECLARE_FINAL_TYPE (LatexilaTemplatesPersonal, latexila_templates_personal, LATEXILA, TEMPLATES_PERSONAL, GtkListStore)

LatexilaTemplatesPersonal *
              latexila_templates_personal_get_instance          (void);

gchar *       latexila_templates_personal_get_contents          (LatexilaTemplatesPersonal *templates,
                                                                 GtkTreePath               *path);

gboolean      latexila_templates_personal_create                (LatexilaTemplatesPersonal *templates,
                                                                 const gchar               *name,
                                                                 const gchar               *config_icon_name,
                                                                 const gchar               *contents,
                                                                 GError                   **error);

G_END_DECLS

#endif /* __LATEXILA_TEMPLATES_PERSONAL_H__ */
