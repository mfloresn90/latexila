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
typedef struct _LatexilaBuildJob          LatexilaBuildJob;
typedef struct _LatexilaBuildTool         LatexilaBuildTool;

/**
 * LatexilaBuildTools:
 * @build_tools: a list of #LatexilaBuildTool's. External code should just read
 * the list, not modify it.
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
};

/**
 * LatexilaPostProcessorType:
 * @LATEXILA_POST_PROCESSOR_TYPE_NO_OUTPUT: no output.
 * @LATEXILA_POST_PROCESSOR_TYPE_ALL_OUTPUT: all output.
 * @LATEXILA_POST_PROCESSOR_TYPE_LATEX: for a LaTeX command.
 * @LATEXILA_POST_PROCESSOR_TYPE_LATEXMK: for the latexmk command.
 *
 * Types of post-processors.
 */
typedef enum
{
  LATEXILA_POST_PROCESSOR_TYPE_NO_OUTPUT,
  LATEXILA_POST_PROCESSOR_TYPE_ALL_OUTPUT,
  LATEXILA_POST_PROCESSOR_TYPE_LATEX,
  LATEXILA_POST_PROCESSOR_TYPE_LATEXMK
} LatexilaPostProcessorType;

/**
 * LatexilaBuildJob:
 * @post_processor_type: the post-processor type.
 * @command: the command to run.
 */
struct _LatexilaBuildJob
{
  LatexilaPostProcessorType post_processor_type;
  gchar *command;
};

/**
 * LatexilaBuildTool:
 * @label: the label.
 * @description: the description.
 * @extensions: the extensions.
 * @icon: the icon.
 * @files_to_open: the files to open.
 * @jobs: a list of #LatexilaBuildJob's.
 * @id: ID of the build tool. It is used only by the default build tools, for
 * saving in #GSettings the lists of enabled/disabled build tools.
 * @enabled: whether the build tool is enabled (for showing it in the UI).
 */
struct _LatexilaBuildTool
{
  gchar *label;
  gchar *description;
  gchar *extensions;
  gchar *icon;
  gchar *files_to_open;
  GSList *jobs;
  gint id;
  guint enabled : 1;
};

GType                 latexila_build_tools_get_type                 (void) G_GNUC_CONST;

void                  latexila_build_tool_free                      (LatexilaBuildTool *build_tool);

const gchar *         latexila_build_tool_get_description           (LatexilaBuildTool *build_tool);

gboolean              latexila_get_post_processor_type_from_name    (const gchar               *name,
                                                                     LatexilaPostProcessorType *type);

const gchar *         latexila_get_post_processor_name_from_type    (LatexilaPostProcessorType type);

void                  latexila_build_tools_load                     (LatexilaBuildTools *build_tools,
                                                                     GFile              *xml_file);

void                  latexila_build_tools_set_enabled              (LatexilaBuildTools *build_tools,
                                                                     guint               tool_num,
                                                                     gboolean            enabled);

G_END_DECLS

#endif /* __LATEXILA_BUILD_TOOLS_H__ */
