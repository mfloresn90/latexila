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

#ifndef __LATEXILA_TEMPLATES_DEFAULT_H__
#define __LATEXILA_TEMPLATES_DEFAULT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LATEXILA_TYPE_TEMPLATES_DEFAULT latexila_templates_default_get_type ()
G_DECLARE_FINAL_TYPE (LatexilaTemplatesDefault, latexila_templates_default, LATEXILA, TEMPLATES_DEFAULT, GtkListStore)

LatexilaTemplatesDefault *
              latexila_templates_default_get_instance           (void);

gchar *       latexila_templates_default_get_contents           (LatexilaTemplatesDefault *templates,
                                                                 GtkTreePath              *path);

G_END_DECLS

#endif /* __LATEXILA_TEMPLATES_DEFAULT_H__ */
