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

#ifndef __LATEXILA_BUILD_TOOL_H__
#define __LATEXILA_BUILD_TOOL_H__

#include "latexila-types.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define LATEXILA_TYPE_BUILD_TOOL             (latexila_build_tool_get_type ())
#define LATEXILA_BUILD_TOOL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LATEXILA_TYPE_BUILD_TOOL, LatexilaBuildTool))
#define LATEXILA_BUILD_TOOL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LATEXILA_TYPE_BUILD_TOOL, LatexilaBuildToolClass))
#define LATEXILA_IS_BUILD_TOOL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LATEXILA_TYPE_BUILD_TOOL))
#define LATEXILA_IS_BUILD_TOOL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LATEXILA_TYPE_BUILD_TOOL))
#define LATEXILA_BUILD_TOOL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LATEXILA_TYPE_BUILD_TOOL, LatexilaBuildToolClass))

typedef struct _LatexilaBuildToolClass   LatexilaBuildToolClass;
typedef struct _LatexilaBuildToolPrivate LatexilaBuildToolPrivate;

struct _LatexilaBuildTool
{
  GObject parent;

  LatexilaBuildToolPrivate *priv;
};

struct _LatexilaBuildToolClass
{
  GObjectClass parent_class;
};

GType                 latexila_build_tool_get_type                  (void) G_GNUC_CONST;

LatexilaBuildTool *   latexila_build_tool_new                       (void);

LatexilaBuildTool *   latexila_build_tool_clone                     (LatexilaBuildTool *build_tool);

const gchar *         latexila_build_tool_get_description           (LatexilaBuildTool *build_tool);

void                  latexila_build_tool_add_job                   (LatexilaBuildTool *build_tool,
                                                                     LatexilaBuildJob  *build_job);

GList *               latexila_build_tool_get_jobs                  (LatexilaBuildTool *build_tool);

gchar *               latexila_build_tool_to_xml                    (LatexilaBuildTool *tool);

void                  latexila_build_tool_run_async                 (LatexilaBuildTool   *build_tool,
                                                                     GFile               *file,
                                                                     LatexilaBuildView   *build_view,
                                                                     GCancellable        *cancellable,
                                                                     GAsyncReadyCallback  callback,
                                                                     gpointer             user_data);

void                  latexila_build_tool_run_finish                (LatexilaBuildTool *build_tool,
                                                                     GAsyncResult      *result);

G_END_DECLS

#endif /* __LATEXILA_BUILD_TOOL_H__ */
