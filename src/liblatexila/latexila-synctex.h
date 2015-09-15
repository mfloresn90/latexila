/*
 * This file is part of LaTeXila.
 *
 * Copyright (C) 2014 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef __LATEXILA_SYNCTEX_H__
#define __LATEXILA_SYNCTEX_H__

#include "latexila-types.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define LATEXILA_TYPE_SYNCTEX             (latexila_synctex_get_type ())
#define LATEXILA_SYNCTEX(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LATEXILA_TYPE_SYNCTEX, LatexilaSynctex))
#define LATEXILA_SYNCTEX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LATEXILA_TYPE_SYNCTEX, LatexilaSynctexClass))
#define LATEXILA_IS_SYNCTEX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LATEXILA_TYPE_SYNCTEX))
#define LATEXILA_IS_SYNCTEX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LATEXILA_TYPE_SYNCTEX))
#define LATEXILA_SYNCTEX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LATEXILA_TYPE_SYNCTEX, LatexilaSynctexClass))

typedef struct _LatexilaSynctexClass   LatexilaSynctexClass;
typedef struct _LatexilaSynctexPrivate LatexilaSynctexPrivate;

struct _LatexilaSynctex
{
  GObject parent;

  LatexilaSynctexPrivate *priv;
};

struct _LatexilaSynctexClass
{
  GObjectClass parent_class;
};

GType             latexila_synctex_get_type                       (void);

LatexilaSynctex * latexila_synctex_get_instance                   (void);

void              latexila_synctex_connect_evince_window          (LatexilaSynctex *synctex,
                                                                   const gchar     *pdf_uri);

void              latexila_synctex_forward_search                 (LatexilaSynctex *synctex,
                                                                   GtkTextBuffer   *buffer,
                                                                   GFile           *buffer_location,
                                                                   GFile           *main_tex_file,
                                                                   guint            timestamp);

G_END_DECLS

#endif /* __LATEXILA_SYNCTEX_H__ */
