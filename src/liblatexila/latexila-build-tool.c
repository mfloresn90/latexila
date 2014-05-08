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
#include <string.h>
#include <glib/gi18n.h>
#include "latexila-build-job.h"
#include "latexila-build-view.h"
#include "latexila-utils.h"

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

  /* Used for running the build tool. */
  GTask *task;
  GFile *file;
  LatexilaBuildView *build_view;
  GtkTreeIter main_title;
  GList *current_job;

  gchar **files_to_open_split;
  gchar **current_file_to_open; /* Position in files_to_open_split */
  GtkTreeIter open_file_job_title;

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

/* Prototypes */
static void run_job (LatexilaBuildTool *build_tool);
static void open_file (LatexilaBuildTool *build_tool);

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

  /* The build tool can not be modified when it is running. */
  g_return_if_fail (build_tool->priv->task == NULL);

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

      g_strfreev (build_tool->priv->files_to_open_split);
      build_tool->priv->files_to_open_split = NULL;
      if (build_tool->priv->files_to_open != NULL)
        {
          build_tool->priv->files_to_open_split = g_strsplit (build_tool->priv->files_to_open, " ", -1);
        }
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

  g_clear_object (&build_tool->priv->task);
  g_clear_object (&build_tool->priv->file);
  g_clear_object (&build_tool->priv->build_view);

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
  g_strfreev (build_tool->priv->files_to_open_split);

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
 * latexila_build_tool_clone:
 * @build_tool: the build tool to clone.
 *
 * Clones a build tool (deep copy).
 *
 * Returns: (transfer full): the cloned build tool.
 */
LatexilaBuildTool *
latexila_build_tool_clone (LatexilaBuildTool *build_tool)
{
  LatexilaBuildTool *new_build_tool;
  GList *l;

  g_return_val_if_fail (LATEXILA_IS_BUILD_TOOL (build_tool), NULL);

  new_build_tool = g_object_new (LATEXILA_TYPE_BUILD_TOOL,
                                 "label", build_tool->priv->label,
                                 "description", build_tool->priv->description,
                                 "extensions", build_tool->priv->extensions,
                                 "icon", build_tool->priv->icon,
                                 "files-to-open", build_tool->priv->files_to_open,
                                 "enabled", build_tool->priv->enabled,
                                 "id", build_tool->priv->id,
                                 NULL);

  for (l = build_tool->priv->jobs->head; l != NULL; l = l->next)
    {
      LatexilaBuildJob *build_job = l->data;
      LatexilaBuildJob *new_build_job = latexila_build_job_clone (build_job);

      latexila_build_tool_add_job (new_build_tool, new_build_job);
      g_object_unref (new_build_job);
    }

  return new_build_tool;
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

  /* The build tool can not be modified when it is running. */
  g_return_if_fail (build_tool->priv->task == NULL);

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

static void
failed (LatexilaBuildTool *build_tool)
{
  GCancellable *cancellable;
  LatexilaBuildState state;

  cancellable = g_task_get_cancellable (build_tool->priv->task);
  if (g_cancellable_is_cancelled (cancellable))
    {
      state = LATEXILA_BUILD_STATE_ABORTED;
    }
  else
    {
      state = LATEXILA_BUILD_STATE_FAILED;
    }

  latexila_build_view_set_title_state (build_tool->priv->build_view,
                                       &build_tool->priv->main_title,
                                       state);

  g_task_return_boolean (build_tool->priv->task, FALSE);
}

static void
query_exists_cb (GFile             *file,
                 GAsyncResult      *result,
                 LatexilaBuildTool *build_tool)
{
  GFileInfo *info;
  GCancellable *cancellable;
  gboolean file_exists;
  gchar *uri = NULL;
  GError *error = NULL;

  info = g_file_query_info_finish (file, result, NULL);

  cancellable = g_task_get_cancellable (build_tool->priv->task);
  if (g_cancellable_is_cancelled (cancellable))
    {
      latexila_build_view_set_title_state (build_tool->priv->build_view,
                                           &build_tool->priv->open_file_job_title,
                                           LATEXILA_BUILD_STATE_ABORTED);
      failed (build_tool);
      goto out;
    }

  file_exists = info != NULL;
  g_clear_object (&info);

  uri = g_file_get_uri (file);

  if (!file_exists)
    {
      LatexilaBuildMsg *msg;

      latexila_build_view_set_title_state (build_tool->priv->build_view,
                                           &build_tool->priv->open_file_job_title,
                                           LATEXILA_BUILD_STATE_FAILED);

      msg = latexila_build_msg_new ();
      msg->text = g_strdup_printf (_("The file '%s' doesn't exist."), uri);
      msg->type = LATEXILA_BUILD_MSG_TYPE_ERROR;

      latexila_build_view_append_single_message (build_tool->priv->build_view,
                                                 &build_tool->priv->open_file_job_title,
                                                 msg);

      latexila_build_msg_free (msg);
      failed (build_tool);
      goto out;
    }

  /* Show URI */

  gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (build_tool->priv->build_view)),
                uri,
                GDK_CURRENT_TIME,
                &error);

  if (error != NULL)
    {
      LatexilaBuildMsg *msg;

      latexila_build_view_set_title_state (build_tool->priv->build_view,
                                           &build_tool->priv->open_file_job_title,
                                           LATEXILA_BUILD_STATE_FAILED);

      msg = latexila_build_msg_new ();
      msg->text = g_strdup_printf (_("Failed to open '%s':"), uri);
      msg->type = LATEXILA_BUILD_MSG_TYPE_ERROR;

      latexila_build_view_append_single_message (build_tool->priv->build_view,
                                                 &build_tool->priv->open_file_job_title,
                                                 msg);

      g_free (msg->text);
      msg->text = g_strdup (error->message);
      msg->type = LATEXILA_BUILD_MSG_TYPE_INFO;

      latexila_build_view_append_single_message (build_tool->priv->build_view,
                                                 &build_tool->priv->open_file_job_title,
                                                 msg);

      latexila_build_msg_free (msg);
      g_error_free (error);

      failed (build_tool);
      goto out;
    }

  latexila_build_view_set_title_state (build_tool->priv->build_view,
                                       &build_tool->priv->open_file_job_title,
                                       LATEXILA_BUILD_STATE_SUCCEEDED);

  build_tool->priv->current_file_to_open++;
  open_file (build_tool);

out:
  g_object_unref (file);
  g_free (uri);
}

static void
open_file (LatexilaBuildTool *build_tool)
{
  const gchar *file_to_open;
  gchar *filename;
  gchar *shortname;
  gchar *uri;
  gchar *basename;
  gchar *message;
  GFile *file;

  while (TRUE)
    {
      if (build_tool->priv->current_file_to_open == NULL ||
          build_tool->priv->current_file_to_open[0] == NULL)
        {
          /* Finished */
          latexila_build_view_set_title_state (build_tool->priv->build_view,
                                               &build_tool->priv->main_title,
                                               LATEXILA_BUILD_STATE_SUCCEEDED);

          g_task_return_boolean (build_tool->priv->task, TRUE);
          return;
        }

      /* Check if the file to open is an empty string. It happens if there are
       * two contiguous spaces in priv->files_to_open for example.
       */
      if (build_tool->priv->current_file_to_open[0][0] == '\0')
        {
          build_tool->priv->current_file_to_open++;
        }
      else
        {
          break;
        }
    }

  file_to_open = build_tool->priv->current_file_to_open[0];

  /* Replace placeholders */

  filename = g_file_get_uri (build_tool->priv->file);
  shortname = latexila_utils_get_shortname (filename);

  if (strstr (file_to_open, "$filename") != NULL)
    {
      uri = latexila_utils_str_replace (file_to_open, "$filename", filename);
    }
  else if (strstr (file_to_open, "$shortname") != NULL)
    {
      uri = latexila_utils_str_replace (file_to_open, "$shortname", shortname);
    }
  else
    {
      uri = g_strdup_printf ("file://%s", file_to_open);
    }

  /* Add job title in the build view */

  basename = g_path_get_basename (uri);
  message = g_strdup_printf (_("Open %s"), basename);

  build_tool->priv->open_file_job_title = latexila_build_view_add_job_title (build_tool->priv->build_view,
                                                                             message,
                                                                             LATEXILA_BUILD_STATE_RUNNING);

  /* Check if the file exists */

  file = g_file_new_for_uri (uri);

  g_file_query_info_async (file,
                           G_FILE_ATTRIBUTE_STANDARD_TYPE,
                           G_FILE_QUERY_INFO_NONE,
                           G_PRIORITY_DEFAULT,
                           g_task_get_cancellable (build_tool->priv->task),
                           (GAsyncReadyCallback) query_exists_cb,
                           build_tool);

  g_free (filename);
  g_free (shortname);
  g_free (uri);
  g_free (basename);
  g_free (message);
}

static void
open_files (LatexilaBuildTool *build_tool)
{
  build_tool->priv->current_file_to_open = build_tool->priv->files_to_open_split;
  open_file (build_tool);
}

static void
run_job_cb (LatexilaBuildJob  *build_job,
            GAsyncResult      *result,
            LatexilaBuildTool *build_tool)
{
  gboolean success;

  success = latexila_build_job_run_finish (build_job, result);

  if (success)
    {
      build_tool->priv->current_job = build_tool->priv->current_job->next;
      run_job (build_tool);
    }
  else
    {
      failed (build_tool);
    }
}

static void
run_job (LatexilaBuildTool *build_tool)
{
  LatexilaBuildJob *build_job;

  if (g_task_return_error_if_cancelled (build_tool->priv->task))
    {
      return;
    }

  if (build_tool->priv->current_job == NULL)
    {
      open_files (build_tool);
      return;
    }

  build_job = build_tool->priv->current_job->data;

  latexila_build_job_run_async (build_job,
                                build_tool->priv->file,
                                build_tool->priv->build_view,
                                g_task_get_cancellable (build_tool->priv->task),
                                (GAsyncReadyCallback) run_job_cb,
                                build_tool);
}

/**
 * latexila_build_tool_run_async:
 * @build_tool: a build tool.
 * @file: a file.
 * @build_view: a build view.
 * @cancellable: a #GCancellable object.
 * @callback: the callback to call when the operation is finished.
 * @user_data: the data to pass to the callback function.
 *
 * Run a build tool on a file with the messages displayed in a build view.
 */
void
latexila_build_tool_run_async (LatexilaBuildTool   *build_tool,
                               GFile               *file,
                               LatexilaBuildView   *build_view,
                               GCancellable        *cancellable,
                               GAsyncReadyCallback  callback,
                               gpointer             user_data)
{
  g_return_if_fail (LATEXILA_IS_BUILD_TOOL (build_tool));
  g_return_if_fail (G_IS_FILE (file));
  g_return_if_fail (LATEXILA_IS_BUILD_VIEW (build_view));
  g_return_if_fail (build_tool->priv->task == NULL);

  build_tool->priv->task = g_task_new (build_tool, cancellable, callback, user_data);

  g_clear_object (&build_tool->priv->file);
  build_tool->priv->file = g_object_ref (file);

  g_clear_object (&build_tool->priv->build_view);
  build_tool->priv->build_view = g_object_ref (build_view);

  latexila_build_view_clear (build_view);

  build_tool->priv->main_title = latexila_build_view_add_main_title (build_view,
                                                                     build_tool->priv->label,
                                                                     LATEXILA_BUILD_STATE_RUNNING);

  build_tool->priv->current_job = build_tool->priv->jobs->head;
  run_job (build_tool);
}

/**
 * latexila_build_tool_run_finish:
 * @build_tool: a build tool.
 * @result: a #GAsyncResult.
 *
 * Finishes the operation started with latexila_build_tool_run_async().
 */
void
latexila_build_tool_run_finish (LatexilaBuildTool *build_tool,
                                GAsyncResult      *result)
{
  GCancellable *cancellable;

  g_return_if_fail (g_task_is_valid (result, build_tool));

  cancellable = g_task_get_cancellable (G_TASK (result));
  if (g_cancellable_is_cancelled (cancellable))
    {
      latexila_build_view_set_title_state (build_tool->priv->build_view,
                                           &build_tool->priv->main_title,
                                           LATEXILA_BUILD_STATE_ABORTED);
    }

  g_task_propagate_boolean (G_TASK (result), NULL);
  g_clear_object (&build_tool->priv->task);
}
