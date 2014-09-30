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

  /* Used for running the build job. */
  GTask *task;
  GFile *file;
  LatexilaBuildView *build_view;
  GtkTreeIter job_title;
  LatexilaPostProcessor *post_processor;
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

  /* The build job can not be modified when it is running. */
  g_return_if_fail (build_job->priv->task == NULL);

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
  LatexilaBuildJob *build_job = LATEXILA_BUILD_JOB (object);

  g_clear_object (&build_job->priv->task);
  g_clear_object (&build_job->priv->file);

  latexila_build_job_clear (build_job);

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

static gchar **
get_command_argv (LatexilaBuildJob  *build_job,
                  gboolean           for_printing,
                  GError           **error)
{
  gchar **argv;
  gchar *base_filename;
  gchar *base_shortname;
  gint i;

  /* Separate arguments */
  if (!g_shell_parse_argv (build_job->priv->command, NULL, &argv, error) ||
      argv == NULL)
    {
      return NULL;
    }

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
  base_filename = g_file_get_basename (build_job->priv->file);
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
get_command_name (LatexilaBuildJob *build_job)
{
  gchar **argv;
  gchar *command_name;

  argv = get_command_argv (build_job, TRUE, NULL);

  if (argv == NULL || argv[0] == NULL || argv[0][0] == '\0')
    {
      command_name = NULL;
    }
  else
    {
      command_name = g_strdup (argv[0]);
    }

  g_strfreev (argv);
  return command_name;
}

static void
display_error (LatexilaBuildJob *build_job,
               const gchar      *message,
               GError           *error)
{
  LatexilaBuildMsg *build_msg;

  g_assert (error != NULL);

  latexila_build_view_set_title_state (build_job->priv->build_view,
                                       &build_job->priv->job_title,
                                       LATEXILA_BUILD_STATE_FAILED);

  build_msg = latexila_build_msg_new ();
  build_msg->text = (gchar *) message;
  build_msg->type = LATEXILA_BUILD_MSG_TYPE_ERROR;
  latexila_build_view_append_single_message (build_job->priv->build_view,
                                             &build_job->priv->job_title,
                                             build_msg);

  build_msg->text = g_strdup (error->message);
  build_msg->type = LATEXILA_BUILD_MSG_TYPE_INFO;
  latexila_build_view_append_single_message (build_job->priv->build_view,
                                             &build_job->priv->job_title,
                                             build_msg);

  /* If the command doesn't seem to be installed, display a more understandable
   * message.
   */
  if (error->domain == G_SPAWN_ERROR &&
      error->code == G_SPAWN_ERROR_NOENT)
    {
      gchar *command_name = get_command_name (build_job);

      if (command_name != NULL)
        {
          g_free (build_msg->text);
          build_msg->text = g_strdup_printf (_("%s doesn't seem to be installed."), command_name);

          latexila_build_view_append_single_message (build_job->priv->build_view,
                                                     &build_job->priv->job_title,
                                                     build_msg);

          g_free (command_name);
        }
    }

  g_error_free (error);
  latexila_build_msg_free (build_msg);
  g_task_return_boolean (build_job->priv->task, FALSE);
}

/* Returns TRUE on success. */
static gboolean
display_command_line (LatexilaBuildJob *build_job)
{
  gchar **argv;
  gchar *command_line;
  GError *error = NULL;

  argv = get_command_argv (build_job, TRUE, &error);

  if (error != NULL)
    {
      build_job->priv->job_title = latexila_build_view_add_job_title (build_job->priv->build_view,
                                                                      build_job->priv->command,
                                                                      LATEXILA_BUILD_STATE_FAILED);

      display_error (build_job, "Failed to parse command line:", error);
      return FALSE;
    }

  command_line = g_strjoinv (" ", argv);

  build_job->priv->job_title = latexila_build_view_add_job_title (build_job->priv->build_view,
                                                                  command_line,
                                                                  LATEXILA_BUILD_STATE_RUNNING);

  g_strfreev (argv);
  g_free (command_line);
  return TRUE;
}

static void
show_details_notify_cb (LatexilaBuildView *build_view,
                        GParamSpec        *pspec,
                        LatexilaBuildJob  *build_job)
{
  const GList *messages;
  gboolean show_details;

  latexila_build_view_remove_children (build_view,
                                       &build_job->priv->job_title);

  g_object_get (build_view, "show-details", &show_details, NULL);

  messages = latexila_post_processor_get_messages (build_job->priv->post_processor,
                                                   show_details);

  latexila_build_view_append_messages (build_view,
                                       &build_job->priv->job_title,
                                       messages,
                                       TRUE);
}

static void
post_processor_cb (LatexilaPostProcessor *pp,
                   GAsyncResult          *result,
                   LatexilaBuildJob      *build_job)
{
  gboolean has_details;

  latexila_post_processor_process_finish (pp, result);

  g_object_get (pp, "has-details", &has_details, NULL);

  if (has_details)
    g_object_set (build_job->priv->build_view,
                  "has-details", TRUE,
                  NULL);

  g_signal_connect (build_job->priv->build_view,
                    "notify::show-details",
                    G_CALLBACK (show_details_notify_cb),
                    build_job);

  show_details_notify_cb (build_job->priv->build_view,
                          NULL,
                          build_job);
}

static void
subprocess_wait_cb (GSubprocess      *subprocess,
                    GAsyncResult     *result,
                    LatexilaBuildJob *build_job)
{
  gboolean ret;
  LatexilaBuildState state;

  ret = g_subprocess_wait_finish (subprocess, result, NULL);

  if (!ret)
    {
      state = LATEXILA_BUILD_STATE_ABORTED;
      g_subprocess_force_exit (subprocess);
    }
  else if (g_subprocess_get_successful (subprocess))
    {
      state = LATEXILA_BUILD_STATE_SUCCEEDED;
    }
  else
    {
      ret = FALSE;
      state = LATEXILA_BUILD_STATE_FAILED;
    }

  latexila_build_view_set_title_state (build_job->priv->build_view,
                                       &build_job->priv->job_title,
                                       state);

  g_task_return_boolean (build_job->priv->task, ret);
  g_object_unref (subprocess);
}

static void
launch_subprocess (LatexilaBuildJob *build_job)
{
  GSubprocessLauncher *launcher;
  GSubprocess *subprocess;
  GFile *parent_dir;
  gchar *working_directory;
  gchar **argv;
  GError *error = NULL;

  if (build_job->priv->post_processor_type == LATEXILA_POST_PROCESSOR_TYPE_NO_OUTPUT)
    {
      launcher = g_subprocess_launcher_new (G_SUBPROCESS_FLAGS_STDOUT_SILENCE |
                                            G_SUBPROCESS_FLAGS_STDERR_SILENCE);
    }
  else
    {
      launcher = g_subprocess_launcher_new (G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                                            G_SUBPROCESS_FLAGS_STDERR_MERGE);
    }

  parent_dir = g_file_get_parent (build_job->priv->file);
  working_directory = g_file_get_path (parent_dir);
  g_object_unref (parent_dir);

  g_subprocess_launcher_set_cwd (launcher, working_directory);
  g_free (working_directory);

  /* The error is already catched in display_command_line(). */
  argv = get_command_argv (build_job, FALSE, NULL);

  subprocess = g_subprocess_launcher_spawnv (launcher, (const gchar * const *) argv, &error);
  g_strfreev (argv);
  g_object_unref (launcher);

  if (error != NULL)
    {
      display_error (build_job, "Failed to launch command:", error);
      return;
    }

  g_clear_object (&build_job->priv->post_processor);

  switch (build_job->priv->post_processor_type)
    {
    case LATEXILA_POST_PROCESSOR_TYPE_NO_OUTPUT:
      break;

    case LATEXILA_POST_PROCESSOR_TYPE_ALL_OUTPUT:
      build_job->priv->post_processor = latexila_post_processor_all_output_new ();
      break;

    case LATEXILA_POST_PROCESSOR_TYPE_LATEX:
      build_job->priv->post_processor = latexila_post_processor_latex_new ();
      break;

    case LATEXILA_POST_PROCESSOR_TYPE_LATEXMK:
      build_job->priv->post_processor = latexila_post_processor_latexmk_new ();
      break;

    default:
      g_return_if_reached ();
    }

  if (build_job->priv->post_processor != NULL)
    {
      latexila_post_processor_process_async (build_job->priv->post_processor,
                                             build_job->priv->file,
                                             g_subprocess_get_stdout_pipe (subprocess),
                                             g_task_get_cancellable (build_job->priv->task),
                                             (GAsyncReadyCallback) post_processor_cb,
                                             build_job);
    }

  g_subprocess_wait_async (subprocess,
                           g_task_get_cancellable (build_job->priv->task),
                           (GAsyncReadyCallback) subprocess_wait_cb,
                           build_job);
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
  g_return_if_fail (LATEXILA_IS_BUILD_JOB (build_job));
  g_return_if_fail (G_IS_FILE (file));
  g_return_if_fail (LATEXILA_IS_BUILD_VIEW (build_view));
  g_return_if_fail (build_job->priv->task == NULL);

  build_job->priv->task = g_task_new (build_job, cancellable, callback, user_data);

  g_clear_object (&build_job->priv->file);
  build_job->priv->file = g_object_ref (file);

  latexila_build_job_clear (build_job);

  build_job->priv->build_view = g_object_ref (build_view);

  if (!display_command_line (build_job))
    {
      return;
    }

  if (!g_task_return_error_if_cancelled (build_job->priv->task))
    {
      launch_subprocess (build_job);
    }
}

/**
 * latexila_build_job_run_finish:
 * @build_job: a build job.
 * @result: a #GAsyncResult.
 *
 * Finishes the operation started with latexila_build_job_run_async().
 *
 * Returns: %TRUE if the build job has run successfully.
 */
gboolean
latexila_build_job_run_finish (LatexilaBuildJob *build_job,
                               GAsyncResult     *result)
{
  GCancellable *cancellable;
  gboolean succeed;

  g_return_if_fail (g_task_is_valid (result, build_job));

  cancellable = g_task_get_cancellable (G_TASK (result));
  if (g_cancellable_is_cancelled (cancellable))
    {
      latexila_build_view_set_title_state (build_job->priv->build_view,
                                           &build_job->priv->job_title,
                                           LATEXILA_BUILD_STATE_ABORTED);
      succeed = FALSE;
    }
  else
    {
      succeed = g_task_propagate_boolean (G_TASK (result), NULL);
    }

  g_clear_object (&build_job->priv->task);
  g_clear_object (&build_job->priv->file);

  return succeed;
}

/**
 * latexila_build_job_clear:
 * @build_job: a build job.
 *
 * Clears the last run operation. latexila_build_job_run_finish() must have been
 * called before calling this function.
 */
void
latexila_build_job_clear (LatexilaBuildJob *build_job)
{
  if (build_job->priv->build_view != NULL)
    g_signal_handlers_disconnect_by_func (build_job->priv->build_view,
                                          show_details_notify_cb,
                                          build_job);

  g_clear_object (&build_job->priv->build_view);
  g_clear_object (&build_job->priv->post_processor);
}
