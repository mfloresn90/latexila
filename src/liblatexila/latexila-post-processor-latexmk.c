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
 * SECTION:post-processor-latexmk
 * @title: LatexilaPostProcessorLatexmk
 * @short_description: latexmk post-processor
 *
 * A post-processor used for the latexmk command. Internally, this
 * post-processor uses #LatexilaPostProcessorLatex.
 */

#include "latexila-post-processor-latexmk.h"
#include "latexila-post-processor-latex.h"
#include "latexila-build-view.h"

typedef enum
{
  /* Try to find something like:
   * ------------
   * Run number 1 of rule 'pdflatex'
   * ------------
   */
  STATE_SUB_TITLE_START,
  STATE_SUB_TITLE_END,

  /* Try to find something like:
   * ------------
   * Running 'pdflatex  -synctex=1 -recorder  "test.tex"'
   * ------------
   */
  STATE_SUB_COMMAND_START,
  STATE_SUB_COMMAND_END,

  /* Fetch the sub-command output. */
  STATE_SUB_COMMAND_OUTPUT_START,
  STATE_SUB_COMMAND_OUTPUT_IN,

  /* Fetch the Latexmk messages after a command output. */
  STATE_LATEXMK_MESSAGES,

  /* If no sub-titles have been found, display almost all output. */
  STATE_FALLBACK_START,
  STATE_FALLBACK_IN
} State;

struct _LatexilaPostProcessorLatexmkPrivate
{
  /* The tree of all messages. */
  GQueue *messages;

  State state;

  /* Number of separators encountered for the current state. */
  gint separator_count;

  /* We run the 'latex' post-processor only on the last latex or pdflatex rule.
   * The first latex rules most probably have warnings that are fixed in the
   * last latex rule.
   * @last_latex_sub_command and @last_latex_messages are simple pointers inside
   * @messages, so don't free them.
   */
  LatexilaBuildMsg *last_latex_sub_command;
  GQueue *last_latex_lines;
  GList *last_latex_messages;

  GFile *file;

  GQueue *all_lines;

  guint last_rule_is_latex_rule : 1;
  guint store_all_lines : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaPostProcessorLatexmk,
                            latexila_post_processor_latexmk,
                            LATEXILA_TYPE_POST_PROCESSOR)

static void
latexila_post_processor_latexmk_start (LatexilaPostProcessor *post_processor,
                                       GFile                 *file)
{
  LatexilaPostProcessorLatexmk *pp = LATEXILA_POST_PROCESSOR_LATEXMK (post_processor);

  g_clear_object (&pp->priv->file);
  pp->priv->file = g_object_ref (file);
}

static void
add_top_level_message (LatexilaPostProcessorLatexmk *pp,
                       LatexilaBuildMsg             *msg)
{
  g_assert (msg != NULL);

  msg->type = LATEXILA_BUILD_MSG_TYPE_JOB_SUB_COMMAND;

  /* Do not expand the row, so the user has first a global view of what have
   * been executed.
   */
  msg->expand = FALSE;

  g_queue_push_tail (pp->priv->messages, msg);

  pp->priv->store_all_lines = FALSE;
}

static void
add_sub_message (LatexilaPostProcessorLatexmk *pp,
                 LatexilaBuildMsg             *sub_msg)
{
  LatexilaBuildMsg *last_toplevel_msg;

  g_assert (sub_msg != NULL);

  last_toplevel_msg = g_queue_peek_tail (pp->priv->messages);

  if (last_toplevel_msg == NULL)
    {
      g_warning ("PostProcessorLatexmk: try to add a sub-message without a top-level message.");
      latexila_build_msg_free (sub_msg);
      return;
    }

  sub_msg->type = LATEXILA_BUILD_MSG_TYPE_INFO;

  if (last_toplevel_msg->children == NULL)
    last_toplevel_msg->children = g_queue_new ();

  g_queue_push_tail (last_toplevel_msg->children, sub_msg);
}

static void
set_last_latex_sub_command (LatexilaPostProcessorLatexmk *pp,
                            LatexilaBuildMsg             *msg)
{
  if (pp->priv->last_latex_lines != NULL)
    g_queue_free_full (pp->priv->last_latex_lines, g_free);

  pp->priv->last_latex_lines = g_queue_new ();
  pp->priv->last_latex_sub_command = msg;
}

static gboolean
is_separator (const gchar *line)
{
  return g_str_has_prefix (line, "------------");
}

static void
fetch_sub_title (LatexilaPostProcessorLatexmk *pp,
                 gchar                        *line)
{
  static GRegex *regex_sub_title = NULL;
  GMatchInfo *match_info;

  g_assert (pp->priv->state == STATE_SUB_TITLE_START ||
            pp->priv->state == STATE_SUB_TITLE_END);

  if (G_UNLIKELY (regex_sub_title == NULL))
    {
      GError *error = NULL;

      regex_sub_title = g_regex_new ("Run number \\d+ of rule '(?P<rule>.*)'",
                                     G_REGEX_OPTIMIZE,
                                     0,
                                     &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatexmk: %s", error->message);
          g_error_free (error);
          return;
        }
    }

  if (pp->priv->state == STATE_SUB_TITLE_END)
    {
      g_assert (pp->priv->separator_count == 1);

      if (is_separator (line))
        {
          pp->priv->separator_count = 0;
          pp->priv->state = STATE_SUB_COMMAND_START;
        }

      goto end;
    }

  if (is_separator (line))
    {
      pp->priv->separator_count++;

      if (pp->priv->separator_count == 2)
        {
          pp->priv->separator_count = 0;
          g_warning ("PostProcessorLatexmk: fetch sub-title failed, try again.");
        }

      goto end;
    }

  if (pp->priv->separator_count != 1)
    goto end;

  g_regex_match (regex_sub_title, line, 0, &match_info);

  if (g_match_info_matches (match_info))
    {
      LatexilaBuildMsg *msg;
      gchar *rule;

      msg = latexila_build_msg_new ();
      msg->text = line;
      line = NULL;

      add_top_level_message (pp, msg);
      pp->priv->state = STATE_SUB_TITLE_END;

      rule = g_match_info_fetch_named (match_info, "rule");

      pp->priv->last_rule_is_latex_rule = (g_strcmp0 (rule, "latex") == 0 ||
                                           g_strcmp0 (rule, "pdflatex") == 0);

      if (pp->priv->last_rule_is_latex_rule)
        set_last_latex_sub_command (pp, msg);

      g_free (rule);
    }

  g_match_info_free (match_info);

end:

  if (pp->priv->store_all_lines)
    {
      if (pp->priv->all_lines == NULL)
        pp->priv->all_lines = g_queue_new ();

      g_queue_push_tail (pp->priv->all_lines, line);
    }
  else
    {
      g_free (line);
    }
}

static void
fetch_sub_command (LatexilaPostProcessorLatexmk *pp,
                   gchar                        *line)
{
  static GRegex *regex_sub_command = NULL;
  GMatchInfo *match_info;

  g_assert (pp->priv->state == STATE_SUB_COMMAND_START ||
            pp->priv->state == STATE_SUB_COMMAND_END);

  if (G_UNLIKELY (regex_sub_command == NULL))
    {
      GError *error = NULL;

      regex_sub_command = g_regex_new ("Running '(?P<command>.*)'",
                                       G_REGEX_OPTIMIZE,
                                       0,
                                       &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatexmk: %s", error->message);
          g_error_free (error);
          return;
        }
    }

  if (pp->priv->state == STATE_SUB_COMMAND_END)
    {
      g_assert (pp->priv->separator_count == 1);

      if (is_separator (line))
        {
          pp->priv->separator_count = 0;
          pp->priv->state = STATE_SUB_COMMAND_OUTPUT_START;
        }

      goto end;
    }

  if (is_separator (line))
    {
      pp->priv->separator_count++;

      if (pp->priv->separator_count == 2)
        {
          pp->priv->separator_count = 0;
          g_warning ("PostProcessorLatexmk: fetch sub-command failed, try again.");
        }

      goto end;
    }

  if (pp->priv->separator_count != 1)
    goto end;

  g_regex_match (regex_sub_command, line, 0, &match_info);

  if (g_match_info_matches (match_info))
    {
      LatexilaBuildMsg *msg;
      gchar *command;

      msg = latexila_build_msg_new ();

      command = g_match_info_fetch_named (match_info, "command");
      msg->text = g_strdup_printf ("$ %s", command);
      g_free (command);

      add_sub_message (pp, msg);
      pp->priv->state = STATE_SUB_COMMAND_END;
    }

  g_match_info_free (match_info);

end:
  g_free (line);
}

static void
fetch_latexmk_messages (LatexilaPostProcessorLatexmk *pp,
                        gchar                        *line)
{
  LatexilaBuildMsg *msg;

  g_assert (pp->priv->state == STATE_LATEXMK_MESSAGES);

  if (is_separator (line))
    {
      pp->priv->state = STATE_SUB_TITLE_START;
      fetch_sub_title (pp, line);
      return;
    }

  msg = latexila_build_msg_new ();
  msg->text = line;
  add_sub_message (pp, msg);
}

static void
fetch_sub_command_output (LatexilaPostProcessorLatexmk *pp,
                          gchar                        *line)
{
  static GRegex *regex_for_rule = NULL;
  static GRegex *regex_rule = NULL;
  GError *error = NULL;

  g_assert (pp->priv->state == STATE_SUB_COMMAND_OUTPUT_START ||
            pp->priv->state == STATE_SUB_COMMAND_OUTPUT_IN);

  if (G_UNLIKELY (regex_for_rule == NULL))
    {
      regex_for_rule = g_regex_new ("^For rule '.*', running",
                                    G_REGEX_OPTIMIZE,
                                    0,
                                    &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatexmk: %s", error->message);
          g_error_free (error);
          return;
        }
    }

  if (G_UNLIKELY (regex_rule == NULL))
    {
      regex_rule = g_regex_new ("^Rule '.*':",
                                G_REGEX_OPTIMIZE,
                                0,
                                &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatexmk: %s", error->message);
          g_error_free (error);
          return;
        }
    }

  if (pp->priv->state == STATE_SUB_COMMAND_OUTPUT_START)
    {
      if (g_str_has_prefix (line, "Latexmk: applying rule") ||
          g_regex_match (regex_for_rule, line, 0, NULL))
        goto end;

      pp->priv->state = STATE_SUB_COMMAND_OUTPUT_IN;
    }

  if (g_str_has_prefix (line, "Latexmk:") ||
      g_regex_match (regex_rule, line, 0, NULL))
    {
      LatexilaBuildMsg *msg;

      pp->priv->state = STATE_LATEXMK_MESSAGES;

      msg = latexila_build_msg_new ();
      /* The latex command output is in English, so for consistency no need to
       * translate this.
       */
      msg->text = g_strdup ("Latexmk messages");
      add_top_level_message (pp, msg);

      fetch_latexmk_messages (pp, line);
      return;
    }

  /* If the rule is latex or pdflatex, we store the output. */
  if (pp->priv->last_rule_is_latex_rule)
    {
      g_queue_push_tail (pp->priv->last_latex_lines, line);
      line = NULL;
    }

  /* If it's another rule (bibtex, makeindex, etc), we show all output. */
  else
    {
      LatexilaBuildMsg *msg;

      msg = latexila_build_msg_new ();
      msg->text = line;
      line = NULL;

      add_sub_message (pp, msg);
    }

end:
  g_free (line);
}

static void
process_line_fallback (LatexilaPostProcessorLatexmk *pp,
                       gchar                        *line)
{
  g_assert (pp->priv->state == STATE_FALLBACK_START ||
            pp->priv->state == STATE_FALLBACK_IN);

  if (pp->priv->state == STATE_FALLBACK_START &&
      !g_str_has_prefix (line, "Latexmk: This is Latexmk") &&
      !g_str_has_prefix (line, "**** Report bugs"))
    pp->priv->state = STATE_FALLBACK_IN;

  if (pp->priv->state == STATE_FALLBACK_IN)
    {
      LatexilaBuildMsg *msg;

      msg = latexila_build_msg_new ();
      msg->type = LATEXILA_BUILD_MSG_TYPE_INFO;
      msg->text = line;
      line = NULL;

      g_queue_push_tail (pp->priv->messages, msg);
    }

  g_free (line);
}

static void
latexila_post_processor_latexmk_process_line (LatexilaPostProcessor *post_processor,
                                              gchar                 *line)
{
  LatexilaPostProcessorLatexmk *pp = LATEXILA_POST_PROCESSOR_LATEXMK (post_processor);

  switch (pp->priv->state)
    {
    case STATE_SUB_TITLE_START:
    case STATE_SUB_TITLE_END:
      fetch_sub_title (pp, line);
      break;

    case STATE_SUB_COMMAND_START:
    case STATE_SUB_COMMAND_END:
      fetch_sub_command (pp, line);
      break;

    case STATE_SUB_COMMAND_OUTPUT_START:
    case STATE_SUB_COMMAND_OUTPUT_IN:
      fetch_sub_command_output (pp, line);
      break;

    case STATE_LATEXMK_MESSAGES:
      fetch_latexmk_messages (pp, line);
      break;

    case STATE_FALLBACK_START:
    case STATE_FALLBACK_IN:
      process_line_fallback (pp, line);
      break;

    default:
      g_return_if_reached ();
    }
}

static void
run_latex_post_processor (LatexilaPostProcessorLatexmk *pp,
                          gboolean                      succeeded)
{
  LatexilaPostProcessor *pp_latex;
  LatexilaBuildMsg *msg;
  GQueue *initial_children;
  GList *l;
  gboolean has_details;
  gint latex_errors_count;

  g_assert (pp->priv->last_latex_sub_command != NULL);
  g_assert (pp->priv->last_latex_lines != NULL);

  pp_latex = latexila_post_processor_latex_new ();

  latexila_post_processor_start (pp_latex, pp->priv->file);

  for (l = pp->priv->last_latex_lines->head; l != NULL; l = l->next)
    latexila_post_processor_process_line (pp_latex, l->data);

  g_queue_free (pp->priv->last_latex_lines);
  pp->priv->last_latex_lines = NULL;

  latexila_post_processor_end (pp_latex, succeeded);

  msg = pp->priv->last_latex_sub_command;

  /* The initial children contain the command, for example. */
  initial_children = msg->children;

  msg->children = latexila_post_processor_take_messages (pp_latex);

  pp->priv->last_latex_messages = msg->children->head;

  if (initial_children != NULL)
    {
      for (l = initial_children->tail; l != NULL; l = l->prev)
        g_queue_push_head (msg->children, l->data);

      g_queue_free (initial_children);
    }

  /* Expand only the last latex command. */
  msg->expand = TRUE;

  /* Set has-details.
   * Almost all the time, the user wants to see only the latex output.
   */

  /* Another solution is to know if the last rule was a latex rule, but it
   * doesn't work well. For example if a BibTeX error occurs, Latexmk run the
   * latex command a second time, so the last command is latex, but the latex
   * output has no errors, the error is only in BibTeX.
   * So it's probably better to check the number of latex errors, as is done
   * below.
   */

  latex_errors_count = latexila_post_processor_latex_get_errors_count (LATEXILA_POST_PROCESSOR_LATEX (pp_latex));

  has_details = succeeded || latex_errors_count > 0;
  g_object_set (pp, "has-details", has_details, NULL);

  g_object_unref (pp_latex);
}

static void
process_all_output (LatexilaPostProcessorLatexmk *pp)
{
  LatexilaPostProcessor *post_processor = LATEXILA_POST_PROCESSOR (pp);
  GList *l;

  g_assert (pp->priv->messages->length == 0);
  g_assert (pp->priv->store_all_lines);

  if (pp->priv->all_lines == NULL)
    return;

  pp->priv->state = STATE_FALLBACK_START;
  pp->priv->store_all_lines = FALSE;

  for (l = pp->priv->all_lines->head; l != NULL; l = l->next)
    latexila_post_processor_latexmk_process_line (post_processor, l->data);

  g_queue_free (pp->priv->all_lines);
  pp->priv->all_lines = NULL;
}

static void
latexila_post_processor_latexmk_end (LatexilaPostProcessor *post_processor,
                                     gboolean               succeeded)
{
  LatexilaPostProcessorLatexmk *pp = LATEXILA_POST_PROCESSOR_LATEXMK (post_processor);

  if (pp->priv->last_latex_sub_command != NULL)
    run_latex_post_processor (pp, succeeded);

  if (pp->priv->messages->length == 0)
    process_all_output (pp);
}

static const GList *
latexila_post_processor_latexmk_get_messages (LatexilaPostProcessor *post_processor,
                                              gboolean               show_details)
{
  LatexilaPostProcessorLatexmk *pp = LATEXILA_POST_PROCESSOR_LATEXMK (post_processor);
  gboolean has_details;

  g_object_get (pp, "has-details", &has_details, NULL);

  if (has_details && !show_details)
    return pp->priv->last_latex_messages;

  return pp->priv->messages != NULL ? pp->priv->messages->head : NULL;
}

static GQueue *
latexila_post_processor_latexmk_take_messages (LatexilaPostProcessor *post_processor)
{
  LatexilaPostProcessorLatexmk *pp = LATEXILA_POST_PROCESSOR_LATEXMK (post_processor);
  GQueue *ret;

  ret = pp->priv->messages;

  pp->priv->messages = NULL;
  pp->priv->last_latex_messages = NULL;
  pp->priv->last_latex_sub_command = NULL;

  return ret;
}

static void
latexila_post_processor_latexmk_dispose (GObject *object)
{
  LatexilaPostProcessorLatexmk *pp = LATEXILA_POST_PROCESSOR_LATEXMK (object);

  g_clear_object (&pp->priv->file);

  G_OBJECT_CLASS (latexila_post_processor_latexmk_parent_class)->dispose (object);
}

static void
latexila_post_processor_latexmk_finalize (GObject *object)
{
  LatexilaPostProcessorLatexmk *pp = LATEXILA_POST_PROCESSOR_LATEXMK (object);

  if (pp->priv->messages != NULL)
    g_queue_free_full (pp->priv->messages, (GDestroyNotify) latexila_build_msg_free);

  if (pp->priv->last_latex_lines != NULL)
    g_queue_free_full (pp->priv->last_latex_lines, g_free);

  if (pp->priv->all_lines != NULL)
    g_queue_free_full (pp->priv->all_lines, g_free);

  G_OBJECT_CLASS (latexila_post_processor_latexmk_parent_class)->finalize (object);
}

static void
latexila_post_processor_latexmk_class_init (LatexilaPostProcessorLatexmkClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  LatexilaPostProcessorClass *pp_class = LATEXILA_POST_PROCESSOR_CLASS (klass);

  object_class->dispose = latexila_post_processor_latexmk_dispose;
  object_class->finalize = latexila_post_processor_latexmk_finalize;

  pp_class->start = latexila_post_processor_latexmk_start;
  pp_class->process_line = latexila_post_processor_latexmk_process_line;
  pp_class->end = latexila_post_processor_latexmk_end;
  pp_class->get_messages = latexila_post_processor_latexmk_get_messages;
  pp_class->take_messages = latexila_post_processor_latexmk_take_messages;
}

static void
latexila_post_processor_latexmk_init (LatexilaPostProcessorLatexmk *pp)
{
  pp->priv = latexila_post_processor_latexmk_get_instance_private (pp);

  pp->priv->messages = g_queue_new ();
  pp->priv->state = STATE_SUB_TITLE_START;
  pp->priv->store_all_lines = TRUE;
}

/**
 * latexila_post_processor_latexmk_new:
 *
 * Returns: a new #LatexilaPostProcessorLatexmk object.
 */
LatexilaPostProcessor *
latexila_post_processor_latexmk_new (void)
{
  return g_object_new (LATEXILA_TYPE_POST_PROCESSOR_LATEXMK, NULL);
}
