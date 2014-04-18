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

#include "latexila-post-processor-all-output.h"

struct _LatexilaPostProcessorAllOutputPrivate
{
  GSList *messages;
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaPostProcessorAllOutput, latexila_post_processor_all_output, LATEXILA_TYPE_POST_PROCESSOR)

static void
latexila_post_processor_all_output_process (LatexilaPostProcessor *post_processor,
                                            const gchar           *output)
{
  LatexilaPostProcessorAllOutput *pp = LATEXILA_POST_PROCESSOR_ALL_OUTPUT (post_processor);
  gchar **lines;
  gchar **l;

  lines = g_strsplit (output, "\n", 0);

  for (l = lines; l != NULL && *l != NULL; l++)
    {
      pp->priv->messages = g_slist_prepend (pp->priv->messages, *l);
    }

  /* Generally a single \n is present at the end of the output, so an empty line
   * is added to the list. But we don't want to display it.
   * TODO check if it is still the case in C.
   */
#if 0
  if (pp->priv->messages != NULL)
    {
      gchar *line = pp->priv->messages->data;
      g_assert (line != NULL);

      if (line[0] == '\0')
        {
          GSList *removed_element = pp->priv->messages;

          pp->priv->messages = g_slist_remove_link (pp->priv->messages, pp->priv->messages);

          g_slist_free_full (removed_element, g_free);
        }
    }
#endif

  pp->priv->messages = g_slist_reverse (pp->priv->messages);

  /* Do not use g_strfreev() because the strings are reused in the list. */
  g_free (lines);
}

static GSList *
latexila_post_processor_all_output_get_messages (LatexilaPostProcessor *post_processor)
{
  LatexilaPostProcessorAllOutput *pp = LATEXILA_POST_PROCESSOR_ALL_OUTPUT (post_processor);

  return pp->priv->messages;
}

static void
latexila_post_processor_all_output_finalize (GObject *object)
{
  LatexilaPostProcessorAllOutputPrivate *priv;

  priv = latexila_post_processor_all_output_get_instance_private (LATEXILA_POST_PROCESSOR_ALL_OUTPUT (object));

  g_slist_free_full (priv->messages, g_free);
  priv->messages = NULL;

  G_OBJECT_CLASS (latexila_post_processor_all_output_parent_class)->finalize (object);
}

static void
latexila_post_processor_all_output_class_init (LatexilaPostProcessorAllOutputClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  LatexilaPostProcessorClass *post_processor_class = LATEXILA_POST_PROCESSOR_CLASS (klass);

  object_class->finalize = latexila_post_processor_all_output_finalize;

  post_processor_class->process = latexila_post_processor_all_output_process;
  post_processor_class->get_messages = latexila_post_processor_all_output_get_messages;
}

static void
latexila_post_processor_all_output_init (LatexilaPostProcessorAllOutput *pp)
{
  pp->priv = latexila_post_processor_all_output_get_instance_private (pp);
}

LatexilaPostProcessor *
latexila_post_processor_all_output_new (void)
{
  return g_object_new (LATEXILA_TYPE_POST_PROCESSOR_ALL_OUTPUT, NULL);
}
