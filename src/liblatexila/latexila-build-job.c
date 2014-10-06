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
#include <string.h>
#include <glib/gi18n.h>
#include "latexila-build-view.h"
#include "latexila-post-processor-all-output.h"
#include "latexila-post-processor-latex.h"
#include "latexila-post-processor-latexmk.h"
#include "latexila-utils.h"
#include "latexila-enum-types.h"

struct _LatexilaBuildJobPrivate
{
  gchar *command;
  LatexilaPostProcessorType post_processor_type;
  guint running_tasks_count;
};

/* Used for running the build job. */
typedef struct _TaskData TaskData;
struct _TaskData
{
  GFile *file;
  LatexilaBuildView *build_view;
  GtkTreeIter job_title;
  LatexilaPostProcessor *post_processor;

  /* To finish the post-processor we must know if the subprocess has succeeded.
   * Since there are two different callbacks that can be called in any order, we
   * store the needed results to pass to
   * latexila_post_processor_process_finish().
   */
  GAsyncResult *post_processor_result;
  guint succeeded : 1;
  guint succeeded_set : 1;
};

enum
{
  PROP_0,
  PROP_COMMAND,
  PROP_POST_PROCESSOR_TYPE
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaBuildJob, latexila_build_job, G_TYPE_OBJECT)

static TaskData *
task_data_new (void)
{
  return g_slice_new0 (TaskData);
}

static void
task_data_free (TaskData *data)
{
  if (data != NULL)
    {
      g_clear_object (&data->file);
      g_clear_object (&data->build_view);
      g_clear_object (&data->post_processor);
      g_clear_object (&data->post_processor_result);

      g_slice_free (TaskData, data);
    }
}

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

  /* The build job can not be modified when it is running. */
  g_return_if_fail (build_job->priv->running_tasks_count == 0);

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
latexila_build_job_init (LatexilaBuildJob *build_job)
{
  build_job->priv = latexila_build_job_get_instance_private (build_job);
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

static gchar **
get_command_argv (GTask     *task,
                  gboolean   for_printing,
                  GError   **error)
{
  LatexilaBuildJob *build_job = g_task_get_source_object (task);
  TaskData *data = g_task_get_task_data (task);
  gchar **argv;
  gchar *base_filename;
  gchar *base_shortname;
  gint i;

  /* Separate arguments */
  if (!g_shell_parse_argv (build_job->priv->command, NULL, &argv, error) ||
      argv == NULL)
    return NULL;

  /* Re-add quotes if needed */
  if (for_printing)
    {
      for (i = 0; argv[i] != NULL; i++)
        {
          /* If the argument contains a space, add the quotes. */
          if (strchr (argv[i], ' ') != NULL)
            {
              gchar *new_arg = g_strdup_printf ("\"%s\"", argv[i]);
              g_free (argv[i]);
              argv[i] = new_arg;
            }
        }
    }

  /* Replace placeholders */
  base_filename = g_file_get_basename (data->file);
  base_shortname = latexila_utils_get_shortname (base_filename);

  for (i = 0; argv[i] != NULL; i++)
    {
      gchar *new_arg = NULL;

      if (strstr (argv[i], "$filename") != NULL)
        {
          new_arg = latexila_utils_str_replace (argv[i], "$filename", base_filename);
        }
      else if (strstr (argv[i], "$shortname"))
        {
          new_arg = latexila_utils_str_replace (argv[i], "$shortname", base_shortname);
        }
      else if (strstr (argv[i], "$view"))
        {
          g_warning ("Build job: the '$view' placeholder is deprecated.");
          new_arg = latexila_utils_str_replace (argv[i], "$view", "xdg-open");
        }

      if (new_arg != NULL)
        {
          g_free (argv[i]);
          argv[i] = new_arg;
        }
    }

  g_free (base_filename);
  g_free (base_shortname);
  return argv;
}

static gchar *
get_command_name (GTask *task)
{
  gchar **argv;
  gchar *command_name;

  argv = get_command_argv (task, TRUE, NULL);

  if (argv == NULL || argv[0] == NULL || argv[0][0] == '\0')
    command_name = NULL;
  else
    command_name = g_strdup (argv[0]);

  g_strfreev (argv);
  return command_name;
}

static void
display_error (GTask       *task,
               const gchar *message,
               GError      *error)
{
  TaskData *data = g_task_get_task_data (task);
  LatexilaBuildMsg *build_msg;

  g_assert (error != NULL);

  latexila_build_view_set_title_state (data->build_view,
                                       &data->job_title,
                                       LATEXILA_BUILD_STATE_FAILED);

  build_msg = latexila_build_msg_new ();
  build_msg->text = (gchar *) message;
  build_msg->type = LATEXILA_BUILD_MSG_TYPE_ERROR;
  latexila_build_view_append_single_message (data->build_view,
                                             &data->job_title,
                                             build_msg);

  build_msg->text = g_strdup (error->message);
  build_msg->type = LATEXILA_BUILD_MSG_TYPE_INFO;
  latexila_build_view_append_single_message (data->build_view,
                                             &data->job_title,
                                             build_msg);

  /* If the command doesn't seem to be installed, display a more understandable
   * message.
   */
  if (error->domain == G_SPAWN_ERROR &&
      error->code == G_SPAWN_ERROR_NOENT)
    {
      gchar *command_name = get_command_name (task);

      if (command_name != NULL)
        {
          g_free (build_msg->text);
          build_msg->text = g_strdup_printf (_("%s doesn't seem to be installed."), command_name);

          latexila_build_view_append_single_message (data->build_view,
                                                     &data->job_title,
                                                     build_msg);

          g_free (command_name);
        }
    }

  g_error_free (error);
  latexila_build_msg_free (build_msg);
  g_task_return_boolean (task, FALSE);
  g_object_unref (task);
}

/* Returns TRUE on success. */
static gboolean
display_command_line (GTask *task)
{
  LatexilaBuildJob *build_job = g_task_get_source_object (task);
  TaskData *data = g_task_get_task_data (task);
  gchar **argv;
  gchar *command_line;
  GError *error = NULL;

  argv = get_command_argv (task, TRUE, &error);

  if (error != NULL)
    {
      data->job_title = latexila_build_view_add_job_title (data->build_view,
                                                           build_job->priv->command,
                                                           LATEXILA_BUILD_STATE_FAILED);

      display_error (task, "Failed to parse command line:", error);
      return FALSE;
    }

  command_line = g_strjoinv (" ", argv);

  data->job_title = latexila_build_view_add_job_title (data->build_view,
                                                       command_line,
                                                       LATEXILA_BUILD_STATE_RUNNING);

  g_strfreev (argv);
  g_free (command_line);
  return TRUE;
}

static void
show_details_notify_cb (LatexilaBuildView *build_view,
                        GParamSpec        *pspec,
                        GTask             *task)
{
  TaskData *data = g_task_get_task_data (task);
  const GList *messages;
  gboolean show_details;

  latexila_build_view_remove_children (build_view, &data->job_title);

  g_object_get (build_view, "show-details", &show_details, NULL);

  messages = latexila_post_processor_get_messages (data->post_processor,
                                                   show_details);

  latexila_build_view_append_messages (build_view,
                                       &data->job_title,
                                       messages,
                                       TRUE);
}

static void
finish_post_processor (GTask *task)
{
  TaskData *data = g_task_get_task_data (task);
  gboolean has_details;

  g_assert (data->succeeded_set);
  g_assert (data->post_processor_result != NULL);

  latexila_post_processor_process_finish (data->post_processor,
                                          data->post_processor_result,
                                          data->succeeded);

  g_clear_object (&data->post_processor_result);

  g_object_get (data->post_processor,
                "has-details", &has_details,
                NULL);

  if (has_details)
    g_object_set (data->build_view,
                  "has-details", TRUE,
                  NULL);

  g_signal_connect_object (data->build_view,
                           "notify::show-details",
                           G_CALLBACK (show_details_notify_cb),
                           task,
                           0);

  show_details_notify_cb (data->build_view, NULL, task);
}

static void
post_processor_cb (LatexilaPostProcessor *pp,
                   GAsyncResult          *result,
                   GTask                 *task)
{
  TaskData *data = g_task_get_task_data (task);

  if (data->post_processor_result != NULL)
    {
      g_warning ("BuildJob: got two post-processor results.");
      g_object_unref (data->post_processor_result);
    }

  data->post_processor_result = g_object_ref (result);

  if (data->succeeded_set)
    finish_post_processor (task);

  g_object_unref (task);
}

static void
subprocess_wait_cb (GSubprocess  *subprocess,
                    GAsyncResult *result,
                    GTask        *task)
{
  TaskData *data = g_task_get_task_data (task);
  gboolean ret;
  LatexilaBuildState state;

  ret = g_subprocess_wait_finish (subprocess, result, NULL);

  if (data->succeeded_set)
    g_warning ("BuildJob: subprocess finished two times.");

  data->succeeded = g_subprocess_get_successful (subprocess);
  data->succeeded_set = TRUE;

  if (!ret)
    {
      state = LATEXILA_BUILD_STATE_ABORTED;
      g_subprocess_force_exit (subprocess);
    }
  else if (data->succeeded)
    {
      state = LATEXILA_BUILD_STATE_SUCCEEDED;
    }
  else
    {
      ret = FALSE;
      state = LATEXILA_BUILD_STATE_FAILED;
    }

  latexila_build_view_set_title_state (data->build_view,
                                       &data->job_title,
                                       state);

  g_task_return_boolean (task, ret);

  if (data->post_processor_result != NULL)
    finish_post_processor (task);

  g_object_unref (subprocess);
  g_object_unref (task);
}

static void
launch_subprocess (GTask *task)
{
  LatexilaBuildJob *build_job = g_task_get_source_object (task);
  TaskData *data = g_task_get_task_data (task);
  GSubprocessLauncher *launcher;
  GSubprocess *subprocess;
  GFile *parent_dir;
  gchar *working_directory;
  gchar **argv;
  GError *error = NULL;

  if (build_job->priv->post_processor_type == LATEXILA_POST_PROCESSOR_TYPE_NO_OUTPUT)
    launcher = g_subprocess_launcher_new (G_SUBPROCESS_FLAGS_STDOUT_SILENCE |
                                          G_SUBPROCESS_FLAGS_STDERR_SILENCE);
  else
    launcher = g_subprocess_launcher_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                                          G_SUBPROCESS_FLAGS_STDERR_MERGE);

  parent_dir = g_file_get_parent (data->file);
  working_directory = g_file_get_path (parent_dir);
  g_object_unref (parent_dir);

  g_subprocess_launcher_set_cwd (launcher, working_directory);
  g_free (working_directory);

  /* The error is already catched in display_command_line(). */
  argv = get_command_argv (task, FALSE, NULL);

  subprocess = g_subprocess_launcher_spawnv (launcher, (const gchar * const *) argv, &error);
  g_strfreev (argv);
  g_object_unref (launcher);

  if (error != NULL)
    {
      display_error (task, "Failed to launch command:", error);
      return;
    }

  g_clear_object (&data->post_processor);

  switch (build_job->priv->post_processor_type)
    {
    case LATEXILA_POST_PROCESSOR_TYPE_NO_OUTPUT:
      break;

    case LATEXILA_POST_PROCESSOR_TYPE_ALL_OUTPUT:
      data->post_processor = latexila_post_processor_all_output_new ();
      break;

    case LATEXILA_POST_PROCESSOR_TYPE_LATEX:
      data->post_processor = latexila_post_processor_latex_new ();
      break;

    case LATEXILA_POST_PROCESSOR_TYPE_LATEXMK:
      data->post_processor = latexila_post_processor_latexmk_new ();
      break;

    default:
      g_return_if_reached ();
    }

  if (data->post_processor != NULL)
    {
      g_object_ref (task);

      latexila_post_processor_process_async (data->post_processor,
                                             data->file,
                                             g_subprocess_get_stdout_pipe (subprocess),
                                             g_task_get_cancellable (task),
                                             (GAsyncReadyCallback) post_processor_cb,
                                             task);
    }

  g_subprocess_wait_async (subprocess,
                           g_task_get_cancellable (task),
                           (GAsyncReadyCallback) subprocess_wait_cb,
                           task);
}

/**
 * latexila_build_job_run_async:
 * @build_job: a build job.
 * @file: a file.
 * @build_view: a build view.
 * @cancellable: a #GCancellable object.
 * @callback: the callback to call when the operation is finished.
 * @user_data: the data to pass to the callback function.
 *
 * Runs asynchronously the build job on a file with the messages displayed in a
 * build view. When the operation is finished, @callback will be called. You can
 * then call latexila_build_job_run_finish().
 */
void
latexila_build_job_run_async (LatexilaBuildJob    *build_job,
                              GFile               *file,
                              LatexilaBuildView   *build_view,
                              GCancellable        *cancellable,
                              GAsyncReadyCallback  callback,
                              gpointer             user_data)
{
  GTask *task;
  TaskData *data;

  g_return_if_fail (LATEXILA_IS_BUILD_JOB (build_job));
  g_return_if_fail (G_IS_FILE (file));
  g_return_if_fail (LATEXILA_IS_BUILD_VIEW (build_view));

  task = g_task_new (build_job, cancellable, callback, user_data);
  build_job->priv->running_tasks_count++;

  data = task_data_new ();
  g_task_set_task_data (task, data, (GDestroyNotify) task_data_free);

  data->file = g_object_ref (file);
  data->build_view = g_object_ref (build_view);

  if (!display_command_line (task))
    return;

  if (g_task_return_error_if_cancelled (task))
    g_object_unref (task);
  else
    launch_subprocess (task);
}

/**
 * latexila_build_job_run_finish:
 * @build_job: a build job.
 * @result: a #GAsyncResult.
 *
 * Finishes the operation started with latexila_build_job_run_async().
 *
 * Before calling this function, you should keep a reference to @result as long
 * as the build messages are displayed in the build view.
 *
 * Returns: %TRUE if the build job has run successfully.
 */
gboolean
latexila_build_job_run_finish (LatexilaBuildJob *build_job,
                               GAsyncResult     *result)
{
  GTask *task;
  TaskData *data;
  GCancellable *cancellable;
  gboolean succeed;

  g_return_if_fail (g_task_is_valid (result, build_job));

  task = G_TASK (result);
  data = g_task_get_task_data (task);

  cancellable = g_task_get_cancellable (task);
  if (g_cancellable_is_cancelled (cancellable))
    {
      latexila_build_view_set_title_state (data->build_view,
                                           &data->job_title,
                                           LATEXILA_BUILD_STATE_ABORTED);
      succeed = FALSE;
    }
  else
    {
      succeed = g_task_propagate_boolean (task, NULL);
    }

  build_job->priv->running_tasks_count--;
  return succeed;
}
