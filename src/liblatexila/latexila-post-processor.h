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

#ifndef __LATEXILA_POST_PROCESSOR_H__
#define __LATEXILA_POST_PROCESSOR_H__

#include <gio/gio.h>
#include "latexila-types.h"

G_BEGIN_DECLS

#define LATEXILA_TYPE_POST_PROCESSOR             (latexila_post_processor_get_type ())
#define LATEXILA_POST_PROCESSOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LATEXILA_TYPE_POST_PROCESSOR, LatexilaPostProcessor))
#define LATEXILA_POST_PROCESSOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LATEXILA_TYPE_POST_PROCESSOR, LatexilaPostProcessorClass))
#define LATEXILA_IS_POST_PROCESSOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LATEXILA_TYPE_POST_PROCESSOR))
#define LATEXILA_IS_POST_PROCESSOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LATEXILA_TYPE_POST_PROCESSOR))
#define LATEXILA_POST_PROCESSOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LATEXILA_TYPE_POST_PROCESSOR, LatexilaPostProcessorClass))

typedef struct _LatexilaPostProcessorClass   LatexilaPostProcessorClass;
typedef struct _LatexilaPostProcessorPrivate LatexilaPostProcessorPrivate;

/**
 * LatexilaPostProcessorType:
 * @LATEXILA_POST_PROCESSOR_TYPE_NO_OUTPUT: no output.
 * @LATEXILA_POST_PROCESSOR_TYPE_ALL_OUTPUT: all output.
 * @LATEXILA_POST_PROCESSOR_TYPE_LATEX: for a LaTeX command.
 * @LATEXILA_POST_PROCESSOR_TYPE_LATEXMK: for the latexmk command.
 * @LATEXILA_POST_PROCESSOR_TYPE_NB_TYPES: number of post-processor types.
 *
 * Types of post-processors.
 */
typedef enum
{
  LATEXILA_POST_PROCESSOR_TYPE_NO_OUTPUT,
  LATEXILA_POST_PROCESSOR_TYPE_ALL_OUTPUT,
  LATEXILA_POST_PROCESSOR_TYPE_LATEX,
  LATEXILA_POST_PROCESSOR_TYPE_LATEXMK,
  LATEXILA_POST_PROCESSOR_TYPE_NB_TYPES
} LatexilaPostProcessorType;

struct _LatexilaPostProcessor
{
  GObject parent;

  LatexilaPostProcessorPrivate *priv;
};

struct _LatexilaPostProcessorClass
{
  GObjectClass parent_class;

  void (* start) (LatexilaPostProcessor *pp,
                  GFile                 *file);

  void (* process_line) (LatexilaPostProcessor *pp,
                         gchar                 *line);

  void (* end) (LatexilaPostProcessor *pp);

  const GQueue * (* get_messages) (LatexilaPostProcessor *pp);
};

GType                   latexila_post_processor_get_type              (void) G_GNUC_CONST;

gboolean                latexila_post_processor_get_type_from_name    (const gchar               *name,
                                                                       LatexilaPostProcessorType *type);

const gchar *           latexila_post_processor_get_name_from_type    (LatexilaPostProcessorType type);

void                    latexila_post_processor_process_async         (LatexilaPostProcessor *pp,
                                                                       GFile                 *file,
                                                                       GInputStream          *stream,
                                                                       GCancellable          *cancellable,
                                                                       GAsyncReadyCallback    callback,
                                                                       gpointer               user_data);

void                    latexila_post_processor_process_finish        (LatexilaPostProcessor *pp,
                                                                       GAsyncResult          *result);

void                    latexila_post_processor_start                 (LatexilaPostProcessor *pp,
                                                                       GFile                 *file);

void                    latexila_post_processor_process_line          (LatexilaPostProcessor *pp,
                                                                       gchar                 *line);

void                    latexila_post_processor_end                   (LatexilaPostProcessor *pp);

const GQueue *          latexila_post_processor_get_messages          (LatexilaPostProcessor *pp);

G_END_DECLS

#endif /* __LATEXILA_POST_PROCESSOR_H__ */
