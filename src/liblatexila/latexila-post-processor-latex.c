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
 * SECTION:post-processor-latex
 * @title: LatexilaPostProcessorLatex
 * @short_description: LaTeX post-processor
 *
 * A post-processor to filter the output of the latex or pdflatex commands.
 * There can be three types of errors: critical errors, warnings, or bad boxes.
 * There are also some other useful information: the size of the document, the
 * number of pages, and the number of errors.
 */

#include "latexila-post-processor-latex.h"
#include <stdlib.h>
#include <string.h>
#include "latexila-build-view.h"

/* TODO This file needs a really good bunch of code clean-up and unit tests. */

#define NO_LINE (-1)

/* If a message is split into several lines, we enter in a different state to
 * fetch the end of the message.
 */
typedef enum
{
  STATE_START,
  STATE_BADBOX,
  STATE_WARNING,
  STATE_ERROR,
  STATE_ERROR_SEARCH_LINE,
  STATE_FILENAME,
  STATE_FILENAME_HEURISTIC
} State;

/* Opened file. They are present in a stack. It is used to know on which file an
 * error or warning occurred.
 */
typedef struct
{
  gchar *filename;
  guint reliable : 1;

  /* Non-existent files are also pushed on the stack, because the corresponding
   * ')' will pop it. If we don't push them, wrong files are popped.
   * When a new message is added, the last _existing_ file is taken.
   */
  guint exists : 1;
} OpenedFile;

struct _LatexilaPostProcessorLatexPrivate
{
  GNode *messages;

  /* The last message is used to insert in O(1) at the end of 'messages'. */
  GNode *last_message;

  /* Current message. */
  LatexilaBuildMsg *cur_msg;

  State state;

  /* If a message is split into several lines, the lines are concatenated in
   * line_buffer.
   */
  GString *line_buffer;
  gint nb_lines;

  /* If a filename is split into several lines. */
  GString *filename_buffer;

  /* The stack containing the files that TeX is processing. The top of the stack
   * is the beginning of the list.
   * Elements type: pointer to OpenedFile
   */
  GSList *stack_files;

  /* The directory where the document is compiled. */
  gchar *directory_path;

  /* For statistics. */
  gint nb_badboxes;
  gint nb_warnings;
  gint nb_errors;
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaPostProcessorLatex,
                            latexila_post_processor_latex,
                            LATEXILA_TYPE_POST_PROCESSOR)

#if 0
static OpenedFile *
opened_file_new (void)
{
  return g_slice_new0 (OpenedFile);
}
#endif

static void
opened_file_free (OpenedFile *opened_file)
{
  if (opened_file != NULL)
    g_slice_free (OpenedFile, opened_file);
}

static gchar *
get_current_filename (LatexilaPostProcessorLatex *pp)
{
  GSList *l;

  for (l = pp->priv->stack_files; l != NULL; l = l->next)
    {
      OpenedFile *file = l->data;

      /* TODO check lazily if the file exists */
      if (file->exists)
        return g_strdup (file->filename);
    }

  return NULL;
}

static void
add_message (LatexilaPostProcessorLatex *pp,
             gboolean                    set_filename)
{
  static GRegex *regex_spaces = NULL;
  LatexilaBuildMsg *cur_msg;
  GError *error = NULL;

  cur_msg = pp->priv->cur_msg;
  g_return_if_fail (cur_msg != NULL);

  /* Exclude some useless messages. */
  if (cur_msg->type == LATEXILA_BUILD_MSG_TYPE_WARNING &&
      g_strcmp0 (cur_msg->text, "There were undefined references.") == 0)
    {
      latexila_build_msg_reinit (cur_msg);
      return;
    }

  if (set_filename)
    {
      g_free (cur_msg->filename);
      cur_msg->filename = get_current_filename (pp);
    }

  if (G_UNLIKELY (regex_spaces == NULL))
    {
      regex_spaces = g_regex_new ("\\s{2,}", G_REGEX_OPTIMIZE, 0, &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          error = NULL;
        }
    }

  if (G_LIKELY (regex_spaces != NULL))
    {
      gchar *new_text;

      /* A message on several lines is sometimes indented, so when the lines are
       * concatenated there are a lot of spaces. So multiple spaces are replaced
       * by one space.
       */
      new_text = g_regex_replace (regex_spaces,
                                  cur_msg->text,
                                  -1, 0, " ", 0,
                                  &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          error = NULL;
        }

      if (new_text != NULL)
        {
          g_free (cur_msg->text);
          cur_msg->text = new_text;
        }
    }

  switch (cur_msg->type)
    {
    case LATEXILA_BUILD_MSG_TYPE_BADBOX:
      pp->priv->nb_badboxes++;
      break;

    case LATEXILA_BUILD_MSG_TYPE_WARNING:
      pp->priv->nb_warnings++;
      break;

    case LATEXILA_BUILD_MSG_TYPE_ERROR:
      pp->priv->nb_errors++;
      break;

    default:
      break;
    }

  pp->priv->last_message = g_node_insert_data_after (pp->priv->messages,
                                                     pp->priv->last_message,
                                                     cur_msg);

  pp->priv->cur_msg = latexila_build_msg_new ();
}

static void
latexila_post_processor_latex_start (LatexilaPostProcessor *post_processor,
                                     GFile                 *file)
{
  LatexilaPostProcessorLatex *pp = LATEXILA_POST_PROCESSOR_LATEX (post_processor);
  GFile *parent;

  parent = g_file_get_parent (file);

  g_free (pp->priv->directory_path);
  pp->priv->directory_path = g_file_get_parse_name (parent);

  g_object_unref (parent);
}

static void
latexila_post_processor_latex_end (LatexilaPostProcessor *post_processor)
{
  LatexilaPostProcessorLatex *pp = LATEXILA_POST_PROCESSOR_LATEX (post_processor);
  LatexilaBuildMsg *cur_msg = pp->priv->cur_msg;

  latexila_build_msg_reinit (cur_msg);

  /* Statistics. Since all the messages printed by TeX are in English, the
   * string is not translated.
   */
  cur_msg->type = LATEXILA_BUILD_MSG_TYPE_INFO;
  cur_msg->text = g_strdup_printf ("%d %s, %d %s, %d %s",
                                   pp->priv->nb_errors,
                                   pp->priv->nb_errors == 1 ? "error" : "errors",
                                   pp->priv->nb_warnings,
                                   pp->priv->nb_warnings == 1 ? "warning" : "warnings",
                                   pp->priv->nb_badboxes,
                                   pp->priv->nb_badboxes == 1 ? "badbox" : "badboxes");

  add_message (pp, FALSE);
}

static gboolean
detect_badbox_line (LatexilaPostProcessorLatex *pp,
                    const gchar                *badbox_line,
                    gboolean                    current_line_is_empty)
{
  static GRegex *regex_badbox_lines = NULL;
  static GRegex *regex_badbox_line = NULL;
  static GRegex *regex_badbox_output = NULL;
  LatexilaBuildMsg *cur_msg = pp->priv->cur_msg;
  GError *error = NULL;

  if (G_UNLIKELY (regex_badbox_lines == NULL))
    {
      regex_badbox_lines = g_regex_new ("(.*) at lines (\\d+)--(\\d+)",
                                        G_REGEX_OPTIMIZE,
                                        0,
                                        &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          error = NULL;
          return FALSE;
        }
    }

  if (G_UNLIKELY (regex_badbox_line == NULL))
    {
      regex_badbox_line = g_regex_new ("(.*) at line (\\d+)",
                                       G_REGEX_OPTIMIZE,
                                       0,
                                       &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          error = NULL;
          return FALSE;
        }
    }

  if (G_UNLIKELY (regex_badbox_output == NULL))
    {
      regex_badbox_output = g_regex_new ("(.*)has occurred while \\\\output is active",
                                         G_REGEX_OPTIMIZE,
                                         0,
                                         &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          error = NULL;
          return FALSE;
        }
    }

  if (g_regex_match (regex_badbox_lines, badbox_line, 0, NULL))
    {
      gchar **strings;
      gint n1;
      gint n2;

      pp->priv->state = STATE_START;

      /* TODO use g_match_info_fetch_named() */
      strings = g_regex_split (regex_badbox_lines, badbox_line, 0);

      g_free (cur_msg->text);
      cur_msg->text = g_strdup (strings[1]);

      n1 = atoi (strings[2]);
      n2 = atoi (strings[3]);

      if (n1 <= n2)
        {
          cur_msg->start_line = n1;
          cur_msg->end_line = n2;
        }
      else
        {
          cur_msg->start_line = n2;
          cur_msg->end_line = n1;
        }

      g_strfreev (strings);
      return TRUE;
    }

  if (g_regex_match (regex_badbox_line, badbox_line, 0, NULL))
    {
      gchar **strings;

      pp->priv->state = STATE_START;

      strings = g_regex_split (regex_badbox_line, badbox_line, 0);

      g_free (cur_msg->text);
      cur_msg->text = g_strdup (strings[1]);

      cur_msg->start_line = atoi (strings[2]);

      g_strfreev (strings);
      return TRUE;
    }

  if (g_regex_match (regex_badbox_output, badbox_line, 0, NULL))
    {
      gchar **strings;

      pp->priv->state = STATE_START;

      strings = g_regex_split (regex_badbox_output, badbox_line, 0);

      g_free (cur_msg->text);
      cur_msg->text = g_strdup (strings[1]);
      cur_msg->start_line = NO_LINE;

      g_strfreev (strings);
      return TRUE;
    }

  if (pp->priv->nb_lines > 4 || current_line_is_empty)
    {
      pp->priv->state = STATE_START;

      g_free (cur_msg->text);
      cur_msg->text = g_strdup (badbox_line);

      cur_msg->start_line = NO_LINE;
      return TRUE;
    }

  pp->priv->state = STATE_BADBOX;
  return FALSE;
}

static gboolean
detect_badbox (LatexilaPostProcessorLatex *pp,
               const gchar                *line)
{
  static GRegex *regex_badbox = NULL;
  GError *error = NULL;

  if (G_UNLIKELY (regex_badbox == NULL))
    {
      regex_badbox = g_regex_new ("^(Over|Under)full \\\\[hv]box",
                                  G_REGEX_OPTIMIZE,
                                  0,
                                  &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          error = NULL;
          return FALSE;
        }
    }

  switch (pp->priv->state)
    {
    case STATE_START:
      if (!g_regex_match (regex_badbox, line, 0, NULL))
        return FALSE;

      pp->priv->cur_msg->type = LATEXILA_BUILD_MSG_TYPE_BADBOX;

      if (detect_badbox_line (pp, line, FALSE))
        {
          add_message (pp, TRUE);
        }
      else
        {
          if (pp->priv->line_buffer != NULL)
            g_string_free (pp->priv->line_buffer, TRUE);

          pp->priv->line_buffer = g_string_new (line);
          pp->priv->nb_lines++;
        }

      return TRUE;

    case STATE_BADBOX:
      g_string_append (pp->priv->line_buffer, line);
      pp->priv->nb_lines++;

      if (detect_badbox_line (pp,
                              pp->priv->line_buffer->str,
                              line[0] == '\0'))
        {
          add_message (pp, TRUE);
          pp->priv->nb_lines = 0;
        }

      /* The return value is not important here. */
      return TRUE;

    default:
      return FALSE;
    }
}

static gboolean
detect_warning_line (LatexilaPostProcessorLatex *pp,
                     const gchar                *warning,
                     gboolean                    current_line_is_empty)
{
  static GRegex *regex_warning_line = NULL;
  static GRegex *regex_warning_international_line = NULL;
  LatexilaBuildMsg *cur_msg = pp->priv->cur_msg;
  gint len;
  GError *error = NULL;

  if (G_UNLIKELY (regex_warning_line == NULL))
    {
      regex_warning_line = g_regex_new ("(.*) on input line (\\d+)\\.$",
                                        G_REGEX_OPTIMIZE,
                                        0,
                                        &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          return FALSE;
        }
    }

  if (G_UNLIKELY (regex_warning_international_line == NULL))
    {
      regex_warning_international_line = g_regex_new ("(.*)(\\d+)\\.$",
                                                      G_REGEX_OPTIMIZE,
                                                      0,
                                                      &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          return FALSE;
        }
    }

  if (g_regex_match (regex_warning_line, warning, 0, NULL))
    {
      gchar **strings;

      pp->priv->state = STATE_START;

      strings = g_regex_split (regex_warning_line, warning, 0);

      g_free (cur_msg->text);
      cur_msg->text = g_strdup (strings[1]);

      cur_msg->start_line = atoi (strings[2]);

      g_strfreev (strings);
      return TRUE;
    }

  if (g_regex_match (regex_warning_international_line, warning, 0, NULL))
    {
      gchar **strings;

      pp->priv->state = STATE_START;

      strings = g_regex_split (regex_warning_international_line, warning, 0);

      g_free (cur_msg->text);
      cur_msg->text = g_strdup (strings[1]);

      cur_msg->start_line = atoi (strings[2]);

      g_strfreev (strings);
      return TRUE;
    }

  len = strlen (warning);
  if (warning[len-1] == '.' || pp->priv->nb_lines > 5 || current_line_is_empty)
    {
      pp->priv->state = STATE_START;

      g_free (cur_msg->text);
      cur_msg->text = g_strdup (warning);

      cur_msg->start_line = NO_LINE;
      return TRUE;
    }

  pp->priv->state = STATE_WARNING;
  return FALSE;
}

static gboolean
detect_warning (LatexilaPostProcessorLatex *pp,
                const gchar                *line)
{
  static GRegex *regex_warning = NULL;
  static GRegex *regex_warning_no_file = NULL;
  LatexilaBuildMsg *cur_msg = pp->priv->cur_msg;
  GMatchInfo *match_info;
  GError *error = NULL;

  if (G_UNLIKELY (regex_warning == NULL))
    {
      regex_warning = g_regex_new ("^(((! )?(La|pdf)TeX)|Package|Class)"
                                   "(?P<name>.*) Warning[^:]*:\\s*(?P<contents>.*)",
                                   G_REGEX_OPTIMIZE | G_REGEX_CASELESS,
                                   0,
                                   &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          return FALSE;
        }
    }

  if (G_UNLIKELY (regex_warning_no_file == NULL))
    {
      regex_warning_no_file = g_regex_new ("(No file .*)",
                                           G_REGEX_OPTIMIZE,
                                           0,
                                           &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          return FALSE;
        }
    }

  switch (pp->priv->state)
    {
    case STATE_START:
      g_regex_match (regex_warning, line, 0, &match_info);
      if (g_match_info_matches (match_info))
        {
          gchar *contents;
          gchar *name;

          cur_msg->type = LATEXILA_BUILD_MSG_TYPE_WARNING;

          contents = g_match_info_fetch_named (match_info, "contents");
          name = g_match_info_fetch_named (match_info, "name");
          name = g_strstrip (name);

          if (name[0] != '\0')
            {
              gchar *new_contents = g_strdup_printf ("%s: %s", name, contents);
              g_free (contents);
              contents = new_contents;
            }

          if (detect_warning_line (pp, contents, FALSE))
            {
              add_message (pp, TRUE);
            }
          else
            {
              if (pp->priv->line_buffer != NULL)
                g_string_free (pp->priv->line_buffer, TRUE);

              pp->priv->line_buffer = g_string_new (contents);
              pp->priv->nb_lines++;
            }

          g_free (contents);
          g_free (name);
          g_match_info_free (match_info);
          return TRUE;
        }

      g_match_info_free (match_info);
      match_info = NULL;

      if (g_regex_match (regex_warning_no_file, line, 0, NULL))
        {
          gchar **strings;

          cur_msg->type = LATEXILA_BUILD_MSG_TYPE_WARNING;

          strings = g_regex_split (regex_warning_no_file, line, 0);

          g_free (cur_msg->text);
          cur_msg->text = g_strdup (strings[1]);

          cur_msg->start_line = NO_LINE;

          add_message (pp, TRUE);

          g_strfreev (strings);
          return TRUE;
        }

      return FALSE;

    case STATE_WARNING:
      g_string_append (pp->priv->line_buffer, line);
      pp->priv->nb_lines++;

      if (detect_warning_line (pp,
                               pp->priv->line_buffer->str,
                               line[0] == '\0'))
        {
          add_message (pp, TRUE);
          pp->priv->nb_lines = 0;
        }

      /* The return value is not important here. */
      return TRUE;

    default:
      return FALSE;
    }
}

static gboolean
detect_error (LatexilaPostProcessorLatex *pp,
              const gchar                *line)
{
  static GRegex *regex_latex_error = NULL;
  static GRegex *regex_pdflatex_error = NULL;
  static GRegex *regex_tex_error = NULL;
  static GRegex *regex_error_line = NULL;
  gboolean found;
  gchar *msg;
  gint len;
  LatexilaBuildMsg *cur_msg = pp->priv->cur_msg;
  GError *error = NULL;

  if (G_UNLIKELY (regex_latex_error == NULL))
    {
      regex_latex_error = g_regex_new ("^! LaTeX Error: (.*)$",
                                       G_REGEX_OPTIMIZE,
                                       0,
                                       &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          return FALSE;
        }
    }

  if (G_UNLIKELY (regex_pdflatex_error == NULL))
    {
      regex_pdflatex_error = g_regex_new ("^Error: pdflatex (.*)$",
                                          G_REGEX_OPTIMIZE,
                                          0,
                                          &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          return FALSE;
        }
    }

  if (G_UNLIKELY (regex_tex_error == NULL))
    {
      regex_tex_error = g_regex_new ("^! (.*)\\.$",
                                     G_REGEX_OPTIMIZE,
                                     0,
                                     &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          return FALSE;
        }
    }

  if (G_UNLIKELY (regex_error_line == NULL))
    {
      regex_error_line = g_regex_new ("^l\\.(\\d+)(.*)",
                                      G_REGEX_OPTIMIZE,
                                      0,
                                      &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          return FALSE;
        }
    }

  switch (pp->priv->state)
    {
    case STATE_START:
      found = TRUE;
      msg = NULL;

      if (g_regex_match (regex_latex_error, line, 0, NULL))
        {
          gchar **strings = g_regex_split (regex_latex_error, line, 0);
          msg = g_strdup (strings[1]);
          g_strfreev (strings);
        }
      else if (g_regex_match (regex_pdflatex_error, line, 0, NULL))
        {
          gchar **strings = g_regex_split (regex_pdflatex_error, line, 0);
          msg = g_strdup (strings[1]);
          g_strfreev (strings);
        }
      else if (g_regex_match (regex_tex_error, line, 0, NULL))
        {
          gchar **strings = g_regex_split (regex_tex_error, line, 0);
          msg = g_strdup (strings[1]);
          g_strfreev (strings);
        }
      else
        {
          found = FALSE;
        }

      if (found)
        {
          pp->priv->nb_lines++;
          cur_msg->type = LATEXILA_BUILD_MSG_TYPE_ERROR;

          len = strlen (line);

          /* The message is complete. */
          if (line[len-1] == '.')
            {
              g_free (cur_msg->text);
              cur_msg->text = msg;

              pp->priv->state = STATE_ERROR_SEARCH_LINE;
            }

          /* The message is split into several lines. */
          else
            {
              if (pp->priv->line_buffer != NULL)
                g_string_free (pp->priv->line_buffer, TRUE);

              pp->priv->line_buffer = g_string_new (msg);
              pp->priv->state = STATE_ERROR;

              g_free (msg);
            }

          return TRUE;
        }

      return FALSE;

    case STATE_ERROR:
      g_string_append (pp->priv->line_buffer, line);
      pp->priv->nb_lines++;

      len = strlen (line);

      if (line[len-1] == '.')
        {
          g_free (cur_msg->text);
          cur_msg->text = g_string_free (pp->priv->line_buffer, FALSE);
          pp->priv->line_buffer = NULL;

          pp->priv->state = STATE_ERROR_SEARCH_LINE;
        }
      else if (pp->priv->nb_lines > 4)
        {
          g_free (cur_msg->text);
          cur_msg->text = g_string_free (pp->priv->line_buffer, FALSE);
          pp->priv->line_buffer = NULL;

          cur_msg->start_line = NO_LINE;

          add_message (pp, TRUE);

          pp->priv->nb_lines = 0;
          pp->priv->state = STATE_START;
        }

      /* The return value is not important here. */
      return TRUE;

    case STATE_ERROR_SEARCH_LINE:
      pp->priv->nb_lines++;

      if (g_regex_match (regex_error_line, line, 0, NULL))
        {
          gchar **strings = g_regex_split (regex_error_line, line, 0);
          cur_msg->start_line = atoi (strings[1]);

          add_message (pp, TRUE);

          pp->priv->nb_lines = 0;
          pp->priv->state = STATE_START;

          g_strfreev (strings);
          return TRUE;
        }
      else if (pp->priv->nb_lines > 11)
        {
          cur_msg->start_line = NO_LINE;
          add_message (pp, TRUE);
          pp->priv->nb_lines = 0;
          pp->priv->state = STATE_START;
          return TRUE;
        }
      break;

    default:
      break;
    }

  return FALSE;
}

static gboolean
detect_other (LatexilaPostProcessorLatex *pp,
              const gchar                *line)
{
  static GRegex *regex_other_bytes = NULL;
  LatexilaBuildMsg *cur_msg = pp->priv->cur_msg;
  GMatchInfo *match_info;
  GError *error = NULL;

  if (G_UNLIKELY (regex_other_bytes == NULL))
    {
      regex_other_bytes = g_regex_new ("(?P<nb>\\d+) bytes",
                                       G_REGEX_OPTIMIZE,
                                       0,
                                       &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          return FALSE;
        }
    }

  if (strstr (line, "Output written on") == NULL)
    return FALSE;

  cur_msg->start_line = NO_LINE;
  cur_msg->type = LATEXILA_BUILD_MSG_TYPE_INFO;

  g_regex_match (regex_other_bytes, line, 0, &match_info);

  if (g_match_info_matches (match_info))
    {
      gchar *nb_bytes_str;
      glong nb_bytes;
      gchar *human_size;
      gchar *new_line;

      nb_bytes_str = g_match_info_fetch_named (match_info, "nb");
      g_return_val_if_fail (nb_bytes_str != NULL, FALSE);

      nb_bytes = atol (nb_bytes_str);
      human_size = g_format_size (nb_bytes);

      new_line = g_regex_replace_literal (regex_other_bytes, line, -1, 0, human_size, 0, &error);

      if (error == NULL)
        {
          g_free (cur_msg->text);
          cur_msg->text = new_line;
        }
      else
        {
          g_warning ("PostProcessorLatex: %s", error->message);

          g_free (cur_msg->text);
          cur_msg->text = g_strdup (line);
        }

      g_free (nb_bytes_str);
      g_free (human_size);
    }
  else
    {
      g_free (cur_msg->text);
      cur_msg->text = g_strdup (line);
    }

  add_message (pp, FALSE);

  g_match_info_free (match_info);
  return TRUE;
}

static void
push_file_on_stack (LatexilaPostProcessorLatex *pp,
                    const gchar                *filename,
                    gboolean                    reliable)
{
  /* TODO */
}

static void
pop_file_from_stack (LatexilaPostProcessorLatex *pp)
{
  if (pp->priv->stack_files != NULL)
    {
      OpenedFile *opened_file = pp->priv->stack_files->data;
      opened_file_free (opened_file);
    }

  pp->priv->stack_files = g_slist_remove_link (pp->priv->stack_files,
                                               pp->priv->stack_files);
}

/* Returns NULL if the filename does not exist.
 * Returns the path of the filename if it exists.
 */
static gchar *
get_path_if_file_exists (LatexilaPostProcessorLatex *pp,
                         const gchar                *filename)
{
  /* TODO */
  return NULL;
}

static gboolean
file_exists (LatexilaPostProcessorLatex *pp,
             const gchar                *filename)
{
  gchar *path;
  gboolean ret;

  path = get_path_if_file_exists (pp, filename);
  ret = path != NULL;

  g_free (path);
  return ret;
}

static void
update_stack_file_heuristic (LatexilaPostProcessorLatex *pp,
                             const gchar                *line)
{
  gboolean expect_filename;
  const gchar *start_filename = line;
  const gchar *line_pos;
  const gchar *line_pos_next;

  expect_filename = pp->priv->state == STATE_FILENAME_HEURISTIC;

  /* Handle special case. */
  if (expect_filename && line[0] == ')')
    {
      push_file_on_stack (pp, pp->priv->filename_buffer->str, FALSE);
      expect_filename = FALSE;
      pp->priv->state = STATE_START;
    }

  for (line_pos = line; line_pos[0] != '\0'; line_pos = line_pos_next)
    {
      gboolean is_last_char = FALSE;
      gboolean next_is_terminator = FALSE;
      gunichar cur_char = g_utf8_get_char (line_pos);

      line_pos_next = g_utf8_next_char (line_pos);

      /* We're expecting a filename. If a filename really ends at this position,
       * one of the following must be true:
       *  1) Next character is a space, indicating the end of a filename
       *    (yes, the path can't have spaces, this is a TeX limitation).
       *  2) We're at the end of the line, the filename is probably
       *     continued on the next line.
       *  3) The file was closed already, signalled by the ')'.
       */

      if (expect_filename)
        {
          is_last_char = line_pos_next[0] == '\0';

          if (is_last_char)
            {
              next_is_terminator = FALSE;
            }
          else
            {
              gunichar next_char = g_utf8_get_char (line_pos_next);
              next_is_terminator = (g_unichar_isspace (next_char) || next_char == ')');
            }
        }

      if (is_last_char || next_is_terminator)
        {
          gint pos;

          g_string_append_len (pp->priv->filename_buffer,
                               start_filename,
                               line_pos_next - start_filename);

          if (pp->priv->filename_buffer->len == 0)
            continue;

          pos = line_pos - line;

          /* By default, an output line is 79 characters max. */
          if ((is_last_char && pos < 78) ||
              next_is_terminator ||
              file_exists (pp, pp->priv->filename_buffer->str))
            {
              push_file_on_stack (pp, pp->priv->filename_buffer->str, FALSE);
              expect_filename = FALSE;
              pp->priv->state = STATE_START;
            }

          /* Guess the filename is continued on the next line, only if the
           * current filename does not exist.
           */
          else if (is_last_char)
            {
              if (file_exists (pp, pp->priv->filename_buffer->str))
                {
                  push_file_on_stack (pp, pp->priv->filename_buffer->str, FALSE);
                  expect_filename = FALSE;
                  pp->priv->state = STATE_START;
                }
              else
                {
                  pp->priv->state = STATE_FILENAME_HEURISTIC;
                }
            }

          /* Filename not detected. */
          else
            {
              pp->priv->state = STATE_START;

              if (pp->priv->filename_buffer != NULL)
                g_string_free (pp->priv->filename_buffer, TRUE);

              /* TODO ensure that filename_buffer is correctly initialized when
               * using it.
               */
              pp->priv->filename_buffer = g_string_new ("");

              expect_filename = FALSE;
            }
        }

      /* TeX is opening a file. */
      else if (cur_char == '(')
        {
          /* TODO */
        }

      /* TeX is closing a file.
       * If this filename was pushed on the stack by the reliable ":<+-"
       * method, don't pop, a ":<-" will follow. This helps in preventing
       * unbalanced ')' from popping filenames from the stack too soon.
       */
      else if (cur_char == ')')
        {
          /* TODO */
        }
    }
}

/* There are basically two ways to detect the current file TeX is processing:
 * 1) Use \Input (srctex or srcltx package) and \include exclusively. This will
 *    cause (La)TeX to print the line ":<+ filename" in the log file when opening
 *    a file, ":<-" when closing a file. Filenames pushed on the stack in this mode
 *    are marked as reliable.
 *
 * 2) Since people will probably also use the \input command, we also have to be
 *    to detect the old-fashioned way. TeX prints '(filename' when opening a file
 *    and a ')' when closing one. It is impossible to detect this with 100% certainty
 *    (TeX prints many messages and even text (a context) from the TeX source file,
 *    there could be unbalanced parentheses), so we use an heuristic algorithm.
 *    In heuristic mode a ')' will only be considered as a signal that TeX is closing
 *    a file if the top of the stack is not marked as "reliable".
 *
 * The method used here is almost the same as in Kile.
 */
static void
update_stack_file (LatexilaPostProcessorLatex *pp,
                   const gchar                *line)
{
  static GRegex *regex_file_pop = NULL;
  GError *error = NULL;

  if (G_UNLIKELY (regex_file_pop == NULL))
    {
      regex_file_pop = g_regex_new ("(\\) )?:<-$",
                                    G_REGEX_OPTIMIZE,
                                    0,
                                    &error);

      if (error != NULL)
        {
          g_warning ("PostProcessorLatex: %s", error->message);
          g_error_free (error);
          return;
        }
    }

  switch (pp->priv->state)
    {
    case STATE_START:
    case STATE_FILENAME_HEURISTIC:
      /* TeX is opening a file. */
      if (g_str_has_prefix (line, ":<+ "))
        {
          gchar *filename;

          if (pp->priv->filename_buffer != NULL)
            g_string_free (pp->priv->filename_buffer, TRUE);

          filename = g_strdup (line + 4);
          g_strstrip (filename);

          pp->priv->filename_buffer = g_string_new (filename);
          pp->priv->state = STATE_FILENAME;

          g_free (filename);
        }

      /* TeX closed a file. */
      else if (g_regex_match (regex_file_pop, line, 0, NULL) ||
               g_str_has_prefix (line, ":<-"))
        pop_file_from_stack (pp);

      /* Fallback to the heuristic detection of filenames. */
      else
        update_stack_file_heuristic (pp, line);
      break;

    case STATE_FILENAME:
      /* The partial filename was followed by '(', this means that TeX is
       * signalling it is opening the file. We are sure the filename is
       * complete now. Don't call update_stack_file_heuristic()
       * since we don't want the filename on the stack twice.
       */
      if (line[0] == '(' ||
          g_str_has_prefix (line, "\\openout"))
        {
          push_file_on_stack (pp, pp->priv->filename_buffer->str, TRUE);
          pp->priv->state = STATE_START;
        }

      /* The partial filename was followed by a TeX error, meaning the
       * file doesn't exist. Don't push it on the stack, instead try to
       * detect the error.
       */
      else if (line[0] == '!')
        {
          pp->priv->state = STATE_START;
          detect_error (pp, line);
        }
      else if (g_str_has_prefix (line, "No file"))
        {
          pp->priv->state = STATE_START;
          detect_warning (pp, line);
        }

      /* The filename is not complete. */
      else
        {
          gchar *line_stripped = g_strdup (line);
          g_strstrip (line_stripped);
          g_string_append (pp->priv->filename_buffer, line_stripped);
          g_free (line_stripped);
        }

    default:
      break;
    }
}

static void
process_line (LatexilaPostProcessorLatex *pp,
              const gchar                *line)
{
  g_assert (line != NULL);

  switch (pp->priv->state)
    {
    case STATE_START:
      if (line[0] == '\0')
        return;

      if (!(detect_badbox (pp, line) ||
            detect_warning (pp, line) ||
            detect_error (pp, line) ||
            detect_other (pp, line)))
        update_stack_file (pp, line);
      break;

    case STATE_BADBOX:
      detect_badbox (pp, line);
      break;

    case STATE_WARNING:
      detect_warning (pp, line);
      break;

    case STATE_ERROR:
    case STATE_ERROR_SEARCH_LINE:
      detect_error (pp, line);
      break;

    case STATE_FILENAME:
    case STATE_FILENAME_HEURISTIC:
      update_stack_file (pp, line);
      break;

    default:
      pp->priv->state = STATE_START;
      break;
    }
}

static void
latexila_post_processor_latex_process_lines (LatexilaPostProcessor  *post_processor,
                                             gchar                 **lines)
{
  LatexilaPostProcessorLatex *pp = LATEXILA_POST_PROCESSOR_LATEX (post_processor);
  gint i;

  for (i = 0; lines != NULL && lines[i] != NULL; i++)
    {
      process_line (pp, lines[i]);
      g_free (lines[i]);
    }

  g_free (lines);
}

static const GNode *
latexila_post_processor_latex_get_messages (LatexilaPostProcessor *post_processor)
{
  LatexilaPostProcessorLatex *pp = LATEXILA_POST_PROCESSOR_LATEX (post_processor);

  return pp->priv->messages->children;
}

static void
latexila_post_processor_latex_finalize (GObject *object)
{
  LatexilaPostProcessorLatex *pp = LATEXILA_POST_PROCESSOR_LATEX (object);

  latexila_build_messages_free (pp->priv->messages);

  if (pp->priv->cur_msg != NULL)
    latexila_build_msg_free (pp->priv->cur_msg);

  if (pp->priv->line_buffer != NULL)
    g_string_free (pp->priv->line_buffer, TRUE);

  if (pp->priv->filename_buffer != NULL)
    g_string_free (pp->priv->filename_buffer, TRUE);

  g_slist_free_full (pp->priv->stack_files, (GDestroyNotify) opened_file_free);

  g_free (pp->priv->directory_path);

  G_OBJECT_CLASS (latexila_post_processor_latex_parent_class)->finalize (object);
}

static void
latexila_post_processor_latex_class_init (LatexilaPostProcessorLatexClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  LatexilaPostProcessorClass *pp_class = LATEXILA_POST_PROCESSOR_CLASS (klass);

  object_class->finalize = latexila_post_processor_latex_finalize;

  pp_class->start = latexila_post_processor_latex_start;
  pp_class->end = latexila_post_processor_latex_end;
  pp_class->process_lines = latexila_post_processor_latex_process_lines;
  pp_class->get_messages = latexila_post_processor_latex_get_messages;
}

static void
latexila_post_processor_latex_init (LatexilaPostProcessorLatex *pp)
{
  pp->priv = latexila_post_processor_latex_get_instance_private (pp);

  pp->priv->messages = g_node_new (NULL);
  pp->priv->cur_msg = latexila_build_msg_new ();
  pp->priv->state = STATE_START;
}

/**
 * latexila_post_processor_latex_new:
 *
 * Returns: a new #LatexilaPostProcessorLatex object.
 */
LatexilaPostProcessor *
latexila_post_processor_latex_new (void)
{
  return g_object_new (LATEXILA_TYPE_POST_PROCESSOR_LATEX, NULL);
}
