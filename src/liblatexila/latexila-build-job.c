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

/**
 * SECTION:build-job
 * @title: LatexilaBuildJob
 * @short_description: Build job
 *
 * A build job. It contains a command (as a string) and a post-processor type.
 */

#include "latexila-build-job.h"
#include "latexila-enum-types.h"

struct _LatexilaBuildJobPrivate
{
  gchar *command;
  LatexilaPostProcessorType post_processor_type;
};

enum
{
  PROP_0,
  PROP_COMMAND,
  PROP_POST_PROCESSOR_TYPE
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaBuildJob, latexila_build_job, G_TYPE_OBJECT)

static void
latexila_build_job_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  LatexilaBuildJob *build_job = LATEXILA_BUILD_JOB (object);

  switch (prop_id)
    {
    case PROP_COMMAND:
      g_value_set_string (value, build_job->priv->command);
      break;

    case PROP_POST_PROCESSOR_TYPE:
      g_value_set_enum (value, build_job->priv->post_processor_type);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
latexila_build_job_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  LatexilaBuildJob *build_job = LATEXILA_BUILD_JOB (object);

  switch (prop_id)
    {
    case PROP_COMMAND:
      g_free (build_job->priv->command);
      build_job->priv->command = g_value_dup_string (value);
      break;

    case PROP_POST_PROCESSOR_TYPE:
      build_job->priv->post_processor_type = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
latexila_build_job_dispose (GObject *object)
{

  G_OBJECT_CLASS (latexila_build_job_parent_class)->dispose (object);
}

static void
latexila_build_job_finalize (GObject *object)
{
  LatexilaBuildJob *build_job = LATEXILA_BUILD_JOB (object);

  g_free (build_job->priv->command);

  G_OBJECT_CLASS (latexila_build_job_parent_class)->finalize (object);
}

static void
latexila_build_job_class_init (LatexilaBuildJobClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = latexila_build_job_get_property;
  object_class->set_property = latexila_build_job_set_property;
  object_class->dispose = latexila_build_job_dispose;
  object_class->finalize = latexila_build_job_finalize;

  g_object_class_install_property (object_class,
                                   PROP_COMMAND,
                                   g_param_spec_string ("command",
                                                        "Command",
                                                        "",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_POST_PROCESSOR_TYPE,
                                   g_param_spec_enum ("post-processor-type",
                                                      "Post-processor type",
                                                      "",
                                                      LATEXILA_TYPE_POST_PROCESSOR_TYPE,
                                                      LATEXILA_POST_PROCESSOR_TYPE_ALL_OUTPUT,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT |
                                                      G_PARAM_STATIC_STRINGS));
}

static void
latexila_build_job_init (LatexilaBuildJob *self)
{
  self->priv = latexila_build_job_get_instance_private (self);
}

/**
 * latexila_build_job_new:
 *
 * Returns: a new #LatexilaBuildJob object.
 */
LatexilaBuildJob *
latexila_build_job_new (void)
{
  return g_object_new (LATEXILA_TYPE_BUILD_JOB, NULL);
}

/**
 * latexila_build_job_clone:
 * @build_job: the build job to clone.
 *
 * Clones a build job (deep copy).
 *
 * Returns: (transfer full): the cloned build job.
 */
LatexilaBuildJob *
latexila_build_job_clone (LatexilaBuildJob *build_job)
{
  g_return_val_if_fail (LATEXILA_IS_BUILD_JOB (build_job), NULL);

  return g_object_new (LATEXILA_TYPE_BUILD_JOB,
                       "command", build_job->priv->command,
                       "post-processor-type", build_job->priv->post_processor_type,
                       NULL);
}

/**
 * latexila_build_job_to_xml:
 * @build_job: a #LatexilaBuildJob object.
 *
 * Returns: the XML contents of the @build_job. Free with g_free().
 */
gchar *
latexila_build_job_to_xml (LatexilaBuildJob *build_job)
{
  g_return_val_if_fail (LATEXILA_IS_BUILD_JOB (build_job), NULL);

  return g_markup_printf_escaped ("    <job postProcessor=\"%s\">%s</job>\n",
                                  latexila_post_processor_get_name_from_type (build_job->priv->post_processor_type),
                                  build_job->priv->command != NULL ? build_job->priv->command : "");
}
