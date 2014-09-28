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
 * SECTION:post-processor-all-output
 * @title: LatexilaPostProcessorAllOutput
 * @short_description: all-output post-processor
 *
 * A post-processor that keeps all the output. Nothing is filtered.
 */

#include "latexila-post-processor-all-output.h"
#include "latexila-build-view.h"

struct _LatexilaPostProcessorAllOutputPrivate
{
  GQueue *messages;
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaPostProcessorAllOutput,
                            latexila_post_processor_all_output,
                            LATEXILA_TYPE_POST_PROCESSOR)

static void
latexila_post_processor_all_output_process_lines (LatexilaPostProcessor  *post_processor,
                                                  gchar                 **lines)
{
  LatexilaPostProcessorAllOutput *pp = LATEXILA_POST_PROCESSOR_ALL_OUTPUT (post_processor);
  gint i;

  for (i = 0; lines != NULL && lines[i] != NULL; i++)
    {
      LatexilaBuildMsg *msg = latexila_build_msg_new ();
      msg->text = lines[i];
      msg->type = LATEXILA_BUILD_MSG_TYPE_INFO;

      g_queue_push_tail (pp->priv->messages, msg);
    }

  g_free (lines);
}

static const GQueue *
latexila_post_processor_all_output_get_messages (LatexilaPostProcessor *post_processor)
{
  LatexilaPostProcessorAllOutput *pp = LATEXILA_POST_PROCESSOR_ALL_OUTPUT (post_processor);

  return pp->priv->messages;
}

static void
latexila_post_processor_all_output_finalize (GObject *object)
{
  LatexilaPostProcessorAllOutput *pp = LATEXILA_POST_PROCESSOR_ALL_OUTPUT (object);

  g_queue_free_full (pp->priv->messages, (GDestroyNotify) latexila_build_msg_free);

  G_OBJECT_CLASS (latexila_post_processor_all_output_parent_class)->finalize (object);
}

static void
latexila_post_processor_all_output_class_init (LatexilaPostProcessorAllOutputClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  LatexilaPostProcessorClass *post_processor_class = LATEXILA_POST_PROCESSOR_CLASS (klass);

  object_class->finalize = latexila_post_processor_all_output_finalize;

  post_processor_class->process_lines = latexila_post_processor_all_output_process_lines;
  post_processor_class->get_messages = latexila_post_processor_all_output_get_messages;
}

static void
latexila_post_processor_all_output_init (LatexilaPostProcessorAllOutput *pp)
{
  pp->priv = latexila_post_processor_all_output_get_instance_private (pp);

  pp->priv->messages = g_queue_new ();
}

/**
 * latexila_post_processor_all_output_new:
 *
 * Returns: a new #LatexilaPostProcessorAllOutput object.
 */
LatexilaPostProcessor *
latexila_post_processor_all_output_new (void)
{
  return g_object_new (LATEXILA_TYPE_POST_PROCESSOR_ALL_OUTPUT, NULL);
}
