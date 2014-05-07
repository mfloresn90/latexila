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
 * SECTION:build-tool
 * @title: LatexilaBuildTool
 * @short_description: Build tool
 *
 * A build tool. It contains some basic properties: a label, a description, an
 * icon, etc. It contains a list of file extensions for which the build tool can
 * run on. More interestingly, it contains the list of #LatexilaBuildJob's to
 * run. And a list of files to open when the build jobs are successfully run.
 */

#include "latexila-build-tool.h"
#include "latexila-build-job.h"
#include "latexila-build-view.h"

struct _LatexilaBuildToolPrivate
{
  gchar *label;
  gchar *description;
  gchar *extensions;
  gchar *icon;
  gchar *files_to_open;
  gint id;

  /* A list of LatexilaBuildJob's. */
  GQueue *jobs;

  guint enabled : 1;
};

enum
{
  PROP_0,
  PROP_LABEL,
  PROP_DESCRIPTION,
  PROP_EXTENSIONS,
  PROP_ICON,
  PROP_FILES_TO_OPEN,
  PROP_ID,
  PROP_ENABLED
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaBuildTool, latexila_build_tool, G_TYPE_OBJECT)

static void
latexila_build_tool_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  LatexilaBuildTool *build_tool = LATEXILA_BUILD_TOOL (object);

  switch (prop_id)
    {
    case PROP_LABEL:
      g_value_set_string (value, build_tool->priv->label);
      break;

    case PROP_DESCRIPTION:
      g_value_set_string (value, build_tool->priv->description);
      break;

    case PROP_EXTENSIONS:
      g_value_set_string (value, build_tool->priv->extensions);
      break;

    case PROP_ICON:
      g_value_set_string (value, build_tool->priv->icon);
      break;

    case PROP_FILES_TO_OPEN:
      g_value_set_string (value, build_tool->priv->files_to_open);
      break;

    case PROP_ID:
      g_value_set_int (value, build_tool->priv->id);
      break;

    case PROP_ENABLED:
      g_value_set_boolean (value, build_tool->priv->enabled);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
latexila_build_tool_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  LatexilaBuildTool *build_tool = LATEXILA_BUILD_TOOL (object);

  switch (prop_id)
    {
    case PROP_LABEL:
      g_free (build_tool->priv->label);
      build_tool->priv->label = g_value_dup_string (value);
      break;

    case PROP_DESCRIPTION:
      g_free (build_tool->priv->description);
      build_tool->priv->description = g_value_dup_string (value);
      break;

    case PROP_EXTENSIONS:
      g_free (build_tool->priv->extensions);
      build_tool->priv->extensions = g_value_dup_string (value);
      break;

    case PROP_ICON:
      g_free (build_tool->priv->icon);
      build_tool->priv->icon = g_value_dup_string (value);
      break;

    case PROP_FILES_TO_OPEN:
      g_free (build_tool->priv->files_to_open);
      build_tool->priv->files_to_open = g_value_dup_string (value);
      break;

    case PROP_ID:
      build_tool->priv->id = g_value_get_int (value);
      break;

    case PROP_ENABLED:
      build_tool->priv->enabled = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
latexila_build_tool_dispose (GObject *object)
{
  LatexilaBuildTool *build_tool = LATEXILA_BUILD_TOOL (object);

  if (build_tool->priv->jobs != NULL)
    {
      g_queue_free_full (build_tool->priv->jobs, g_object_unref);
      build_tool->priv->jobs = NULL;
    }

  G_OBJECT_CLASS (latexila_build_tool_parent_class)->dispose (object);
}

static void
latexila_build_tool_finalize (GObject *object)
{
  LatexilaBuildTool *build_tool = LATEXILA_BUILD_TOOL (object);

  g_free (build_tool->priv->label);
  g_free (build_tool->priv->description);
  g_free (build_tool->priv->extensions);
  g_free (build_tool->priv->icon);
  g_free (build_tool->priv->files_to_open);

  G_OBJECT_CLASS (latexila_build_tool_parent_class)->finalize (object);
}

static void
latexila_build_tool_class_init (LatexilaBuildToolClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = latexila_build_tool_get_property;
  object_class->set_property = latexila_build_tool_set_property;
  object_class->dispose = latexila_build_tool_dispose;
  object_class->finalize = latexila_build_tool_finalize;

  g_object_class_install_property (object_class,
                                   PROP_LABEL,
                                   g_param_spec_string ("label",
                                                        "Label",
                                                        "",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_DESCRIPTION,
                                   g_param_spec_string ("description",
                                                        "Description",
                                                        "",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_EXTENSIONS,
                                   g_param_spec_string ("extensions",
                                                        "Extensions",
                                                        "",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_ICON,
                                   g_param_spec_string ("icon",
                                                        "Icon",
                                                        "",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_FILES_TO_OPEN,
                                   g_param_spec_string ("files-to-open",
                                                        "Files to open",
                                                        "",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));

  /**
   * LatexilaBuildTool:id:
   *
   * The build tool ID. It is used only by the default build tools, for saving
   * in #GSettings the lists of enabled/disabled build tools.
   */
  g_object_class_install_property (object_class,
                                   PROP_ID,
                                   g_param_spec_int ("id",
                                                     "ID",
                                                     "",
                                                     0,
                                                     G_MAXINT,
                                                     0,
                                                     G_PARAM_READWRITE |
                                                     G_PARAM_CONSTRUCT |
                                                     G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_ENABLED,
                                   g_param_spec_boolean ("enabled",
                                                         "Enabled",
                                                         "",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_STATIC_STRINGS));
}

static void
latexila_build_tool_init (LatexilaBuildTool *self)
{
  self->priv = latexila_build_tool_get_instance_private (self);

  self->priv->jobs = g_queue_new ();
}

/**
 * latexila_build_tool_new:
 *
 * Returns: a new #LatexilaBuildTool object.
 */
LatexilaBuildTool *
latexila_build_tool_new (void)
{
  return g_object_new (LATEXILA_TYPE_BUILD_TOOL, NULL);
}

/**
 * latexila_build_tool_get_description:
 * @build_tool: a #LatexilaBuildTool.
 *
 * Gets the description. The label is returned if the description is empty.
 *
 * Returns: the description.
 */
const gchar *
latexila_build_tool_get_description (LatexilaBuildTool *build_tool)
{
  if (build_tool->priv->description == NULL ||
      build_tool->priv->description[0] == '\0')
    {
      return build_tool->priv->label;
    }

  return build_tool->priv->description;
}

/**
 * latexila_build_tool_add_job:
 * @build_tool: a #LatexilaBuildTool.
 * @build_job: a #LatexilaBuildJob.
 *
 * Adds a build job at the end of the list (in O(1)).
 */
void
latexila_build_tool_add_job (LatexilaBuildTool *build_tool,
                             LatexilaBuildJob  *build_job)
{
  g_return_if_fail (LATEXILA_IS_BUILD_TOOL (build_tool));
  g_return_if_fail (LATEXILA_IS_BUILD_JOB (build_job));

  g_queue_push_tail (build_tool->priv->jobs, build_job);
  g_object_ref (build_job);
}

/**
 * latexila_build_tool_get_jobs:
 * @build_tool: a #LatexilaBuildTool.
 *
 * Returns: (element-type LatexilaBuildJob) (transfer none): the list of
 * #LatexilaBuildJob's.
 */
GList *
latexila_build_tool_get_jobs (LatexilaBuildTool *build_tool)
{
  g_return_val_if_fail (LATEXILA_IS_BUILD_TOOL (build_tool), NULL);

  return build_tool->priv->jobs->head;
}

/**
 * latexila_build_tool_to_xml:
 * @tool: a #LatexilaBuildTool object.
 *
 * Returns: the XML contents of the build tool. Free with g_free().
 */
gchar *
latexila_build_tool_to_xml (LatexilaBuildTool *tool)
{
  GString *contents;
  gchar *escaped_text;
  GList *l;

  g_return_val_if_fail (LATEXILA_IS_BUILD_TOOL (tool), NULL);

  contents = g_string_new (NULL);

  g_string_append_printf (contents,
                          "\n  <tool enabled=\"%s\" extensions=\"%s\" icon=\"%s\">\n",
                          tool->priv->enabled ? "true" : "false",
                          tool->priv->extensions != NULL ? tool->priv->extensions : "",
                          tool->priv->icon != NULL ? tool->priv->icon : "");

  escaped_text = g_markup_printf_escaped ("    <label>%s</label>\n"
                                          "    <description>%s</description>\n",
                                          tool->priv->label != NULL ? tool->priv->label : "",
                                          tool->priv->description != NULL ? tool->priv->description : "");

  g_string_append (contents, escaped_text);
  g_free (escaped_text);

  for (l = tool->priv->jobs->head; l != NULL; l = l->next)
    {
      LatexilaBuildJob *job = l->data;

      escaped_text = latexila_build_job_to_xml (job);
      g_string_append (contents, escaped_text);
      g_free (escaped_text);
    }

  escaped_text = g_markup_printf_escaped ("    <open>%s</open>\n",
                                          tool->priv->files_to_open != NULL ? tool->priv->files_to_open : "");
  g_string_append (contents, escaped_text);
  g_free (escaped_text);

  g_string_append (contents, "  </tool>\n");

  return g_string_free (contents, FALSE);
}

/**
 * latexila_build_tool_run:
 * @build_tool: a build tool.
 * @file: a file.
 * @build_view: a build view.
 *
 * Run a build tool on a file with the messages displayed in a build view.
 */
void
latexila_build_tool_run (LatexilaBuildTool *build_tool,
                         GFile             *file,
                         LatexilaBuildView *build_view)
{
  g_return_if_fail (LATEXILA_IS_BUILD_TOOL (build_tool));
  g_return_if_fail (G_IS_FILE (file));
  g_return_if_fail (LATEXILA_IS_BUILD_VIEW (build_view));

  latexila_build_view_clear (build_view);

  latexila_build_view_add_main_title (build_view,
                                      build_tool->priv->label,
                                      LATEXILA_BUILD_STATE_RUNNING);
}
