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

#ifndef __LATEXILA_BUILD_TOOLS_DEFAULT_H__
#define __LATEXILA_BUILD_TOOLS_DEFAULT_H__

#include <glib-object.h>
#include "latexila-build-tools.h"
#include "latexila-types.h"

G_BEGIN_DECLS

#define LATEXILA_TYPE_BUILD_TOOLS_DEFAULT             (latexila_build_tools_default_get_type ())
#define LATEXILA_BUILD_TOOLS_DEFAULT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LATEXILA_TYPE_BUILD_TOOLS_DEFAULT, LatexilaBuildToolsDefault))
#define LATEXILA_BUILD_TOOLS_DEFAULT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LATEXILA_TYPE_BUILD_TOOLS_DEFAULT, LatexilaBuildToolsDefaultClass))
#define LATEXILA_IS_BUILD_TOOLS_DEFAULT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LATEXILA_TYPE_BUILD_TOOLS_DEFAULT))
#define LATEXILA_IS_BUILD_TOOLS_DEFAULT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LATEXILA_TYPE_BUILD_TOOLS_DEFAULT))
#define LATEXILA_BUILD_TOOLS_DEFAULT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LATEXILA_TYPE_BUILD_TOOLS_DEFAULT, LatexilaBuildToolsDefaultClass))

typedef struct _LatexilaBuildToolsDefaultClass   LatexilaBuildToolsDefaultClass;
typedef struct _LatexilaBuildToolsDefaultPrivate LatexilaBuildToolsDefaultPrivate;

struct _LatexilaBuildToolsDefault
{
  LatexilaBuildTools parent;

  LatexilaBuildToolsDefaultPrivate *priv;
};

struct _LatexilaBuildToolsDefaultClass
{
  LatexilaBuildToolsClass parent_class;
};

GType               latexila_build_tools_default_get_type           (void);

LatexilaBuildToolsDefault *
                    latexila_build_tools_default_get_instance       (void);

G_END_DECLS

#endif /* __LATEXILA_BUILD_TOOLS_DEFAULT_H__ */
