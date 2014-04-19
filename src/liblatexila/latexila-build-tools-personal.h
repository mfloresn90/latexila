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

#ifndef __LATEXILA_BUILD_TOOLS_PERSONAL_H__
#define __LATEXILA_BUILD_TOOLS_PERSONAL_H__

#include <glib-object.h>
#include "latexila-build-tools.h"
#include "latexila-types.h"

G_BEGIN_DECLS

#define LATEXILA_TYPE_BUILD_TOOLS_PERSONAL             (latexila_build_tools_personal_get_type ())
#define LATEXILA_BUILD_TOOLS_PERSONAL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LATEXILA_TYPE_BUILD_TOOLS_PERSONAL, LatexilaBuildToolsPersonal))
#define LATEXILA_BUILD_TOOLS_PERSONAL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LATEXILA_TYPE_BUILD_TOOLS_PERSONAL, LatexilaBuildToolsPersonalClass))
#define LATEXILA_IS_BUILD_TOOLS_PERSONAL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LATEXILA_TYPE_BUILD_TOOLS_PERSONAL))
#define LATEXILA_IS_BUILD_TOOLS_PERSONAL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LATEXILA_TYPE_BUILD_TOOLS_PERSONAL))
#define LATEXILA_BUILD_TOOLS_PERSONAL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LATEXILA_TYPE_BUILD_TOOLS_PERSONAL, LatexilaBuildToolsPersonalClass))

typedef struct _LatexilaBuildToolsPersonalClass   LatexilaBuildToolsPersonalClass;
typedef struct _LatexilaBuildToolsPersonalPrivate LatexilaBuildToolsPersonalPrivate;

struct _LatexilaBuildToolsPersonal
{
  LatexilaBuildTools parent;

  LatexilaBuildToolsPersonalPrivate *priv;
};

struct _LatexilaBuildToolsPersonalClass
{
  LatexilaBuildToolsClass parent_class;
};

GType               latexila_build_tools_personal_get_type            (void) G_GNUC_CONST;

LatexilaBuildToolsPersonal *
                    latexila_build_tools_personal_get_instance        (void);

void                latexila_build_tools_personal_save                (LatexilaBuildToolsPersonal *build_tools);

void                latexila_build_tools_personal_move_up             (LatexilaBuildToolsPersonal *build_tools,
                                                                       guint                       tool_num);

void                latexila_build_tools_personal_move_down           (LatexilaBuildToolsPersonal *build_tools,
                                                                       guint                       tool_num);

void                latexila_build_tools_personal_delete              (LatexilaBuildToolsPersonal *build_tools,
                                                                       guint                       tool_num);

void                latexila_build_tools_personal_add                 (LatexilaBuildToolsPersonal *build_tools,
                                                                       LatexilaBuildTool          *new_build_tool);

void                latexila_build_tools_personal_insert              (LatexilaBuildToolsPersonal *build_tools,
                                                                       LatexilaBuildTool          *new_build_tool,
                                                                       guint                       position);

void                latexila_build_tools_personal_replace             (LatexilaBuildToolsPersonal *build_tools,
                                                                       LatexilaBuildTool          *new_build_tool,
                                                                       guint                       position);

G_END_DECLS

#endif /* __LATEXILA_BUILD_TOOLS_PERSONAL_H__ */
