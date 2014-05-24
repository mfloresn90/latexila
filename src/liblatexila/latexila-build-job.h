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

#ifndef __LATEXILA_BUILD_JOB_H__
#define __LATEXILA_BUILD_JOB_H__

#include <gio/gio.h>
#include "latexila-types.h"
#include "latexila-post-processor.h"

G_BEGIN_DECLS

#define LATEXILA_TYPE_BUILD_JOB             (latexila_build_job_get_type ())
#define LATEXILA_BUILD_JOB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LATEXILA_TYPE_BUILD_JOB, LatexilaBuildJob))
#define LATEXILA_BUILD_JOB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LATEXILA_TYPE_BUILD_JOB, LatexilaBuildJobClass))
#define LATEXILA_IS_BUILD_JOB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LATEXILA_TYPE_BUILD_JOB))
#define LATEXILA_IS_BUILD_JOB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LATEXILA_TYPE_BUILD_JOB))
#define LATEXILA_BUILD_JOB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LATEXILA_TYPE_BUILD_JOB, LatexilaBuildJobClass))

typedef struct _LatexilaBuildJobClass   LatexilaBuildJobClass;
typedef struct _LatexilaBuildJobPrivate LatexilaBuildJobPrivate;

struct _LatexilaBuildJob
{
  GObject parent;

  LatexilaBuildJobPrivate *priv;
};

struct _LatexilaBuildJobClass
{
  GObjectClass parent_class;
};

GType               latexila_build_job_get_type                   (void) G_GNUC_CONST;

LatexilaBuildJob *  latexila_build_job_new                        (void);

LatexilaBuildJob *  latexila_build_job_clone                      (LatexilaBuildJob *build_job);

gchar *             latexila_build_job_to_xml                     (LatexilaBuildJob *build_job);

void                latexila_build_job_run_async                  (LatexilaBuildJob    *build_job,
                                                                   GFile               *file,
                                                                   LatexilaBuildView   *build_view,
                                                                   GCancellable        *cancellable,
                                                                   GAsyncReadyCallback  callback,
                                                                   gpointer             user_data);

gboolean            latexila_build_job_run_finish                 (LatexilaBuildJob *build_job,
                                                                   GAsyncResult     *result);

void                latexila_build_job_clear                      (LatexilaBuildJob *build_job);

G_END_DECLS

#endif /* __LATEXILA_BUILD_JOB_H__ */
