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

#include "latexila-post-processor.h"

typedef struct _LatexilaPostProcessorPrivate LatexilaPostProcessorPrivate;

struct _LatexilaPostProcessorPrivate
{
  guint has_details : 1;
  guint show_details : 1;
};

enum
{
  PROP_0,
  PROP_HAS_DETAILS,
  PROP_SHOW_DETAILS
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaPostProcessor, latexila_post_processor, G_TYPE_OBJECT)

static void
latexila_post_processor_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  LatexilaPostProcessorPrivate *priv;

  g_return_if_fail (LATEXILA_IS_POST_PROCESSOR (object));

  priv = latexila_post_processor_get_instance_private (LATEXILA_POST_PROCESSOR (object));

  switch (prop_id)
    {
    case PROP_HAS_DETAILS:
      g_value_set_boolean (value, priv->has_details);
      break;

    case PROP_SHOW_DETAILS:
      g_value_set_boolean (value, priv->show_details);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
latexila_post_processor_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  LatexilaPostProcessorPrivate *priv;

  g_return_if_fail (LATEXILA_IS_POST_PROCESSOR (object));

  priv = latexila_post_processor_get_instance_private (LATEXILA_POST_PROCESSOR (object));

  switch (prop_id)
    {
    case PROP_HAS_DETAILS:
      priv->has_details = g_value_get_boolean (value);
      break;

    case PROP_SHOW_DETAILS:
      priv->show_details = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
latexila_post_processor_process_default (LatexilaPostProcessor *post_processor,
                                         const gchar           *output)
{
  g_return_if_fail (LATEXILA_IS_POST_PROCESSOR (post_processor));
}

static GSList *
latexila_post_processor_get_messages_default (LatexilaPostProcessor *post_processor)
{
  g_return_val_if_fail (LATEXILA_IS_POST_PROCESSOR (post_processor), NULL);

  return NULL;
}

static void
latexila_post_processor_class_init (LatexilaPostProcessorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = latexila_post_processor_get_property;
  object_class->set_property = latexila_post_processor_set_property;

  klass->process = latexila_post_processor_process_default;
  klass->get_messages = latexila_post_processor_get_messages_default;

  g_object_class_install_property (object_class,
                                   PROP_HAS_DETAILS,
                                   g_param_spec_boolean ("has-details",
                                                         "Has details",
                                                         "",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_SHOW_DETAILS,
                                   g_param_spec_boolean ("show-details",
                                                         "Show details",
                                                         "",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_STATIC_STRINGS));
}

static void
latexila_post_processor_init (LatexilaPostProcessor *post_processor)
{
}

LatexilaPostProcessor *
latexila_post_processor_new (void)
{
  return g_object_new (LATEXILA_TYPE_POST_PROCESSOR, NULL);
}

void
latexila_post_processor_process (LatexilaPostProcessor *post_processor,
                                 const gchar           *output)
{
  g_return_if_fail (LATEXILA_IS_POST_PROCESSOR (post_processor));

  LATEXILA_POST_PROCESSOR_GET_CLASS (post_processor)->process (post_processor, output);
}

GSList *
latexila_post_processor_get_messages (LatexilaPostProcessor *post_processor)
{
  g_return_val_if_fail (LATEXILA_IS_POST_PROCESSOR (post_processor), NULL);

  return LATEXILA_POST_PROCESSOR_GET_CLASS (post_processor)->get_messages (post_processor);
}
