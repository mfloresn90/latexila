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

#ifndef __LATEXILA_BUILD_TOOLS_H__
#define __LATEXILA_BUILD_TOOLS_H__

#include <gio/gio.h>
#include "latexila-types.h"

G_BEGIN_DECLS

#define LATEXILA_TYPE_BUILD_TOOLS             (latexila_build_tools_get_type ())
#define LATEXILA_BUILD_TOOLS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LATEXILA_TYPE_BUILD_TOOLS, LatexilaBuildTools))
#define LATEXILA_BUILD_TOOLS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LATEXILA_TYPE_BUILD_TOOLS, LatexilaBuildToolsClass))
#define LATEXILA_IS_BUILD_TOOLS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LATEXILA_TYPE_BUILD_TOOLS))
#define LATEXILA_IS_BUILD_TOOLS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LATEXILA_TYPE_BUILD_TOOLS))
#define LATEXILA_BUILD_TOOLS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LATEXILA_TYPE_BUILD_TOOLS, LatexilaBuildToolsClass))

typedef struct _LatexilaBuildToolsClass   LatexilaBuildToolsClass;
typedef struct _LatexilaBuildToolsPrivate LatexilaBuildToolsPrivate;

/**
 * LatexilaBuildTools:
 * @build_tools: (element-type LatexilaBuildTool): a list of
 * #LatexilaBuildTool's. External code should just read the list, not modify it.
 */
struct _LatexilaBuildTools
{
  /*< private >*/
  GObject parent;

  /*< public >*/
  GList *build_tools;

  /*< private >*/
  LatexilaBuildToolsPrivate *priv;
};

struct _LatexilaBuildToolsClass
{
  GObjectClass parent_class;

  /* When the XML file is not found. */
  void (* handle_not_found_error) (LatexilaBuildTools *build_tools,
                                   GFile              *xml_file,
                                   GError             *error);
};

GType                 latexila_build_tools_get_type                 (void);

void                  latexila_build_tools_load                     (LatexilaBuildTools *build_tools,
                                                                     GFile              *xml_file);

LatexilaBuildTool *   latexila_build_tools_nth                      (LatexilaBuildTools *build_tools,
                                                                     guint               tool_num);

void                  latexila_build_tools_set_enabled              (LatexilaBuildTools *build_tools,
                                                                     guint               tool_num,
                                                                     gboolean            enabled);

G_END_DECLS

#endif /* __LATEXILA_BUILD_TOOLS_H__ */
