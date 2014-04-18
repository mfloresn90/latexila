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
  gint something;
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaPostProcessor, latexila_post_processor, G_TYPE_OBJECT)

static void
latexila_post_processor_dispose (GObject *object)
{

  G_OBJECT_CLASS (latexila_post_processor_parent_class)->dispose (object);
}

static void
latexila_post_processor_finalize (GObject *object)
{

  G_OBJECT_CLASS (latexila_post_processor_parent_class)->finalize (object);
}

static void
latexila_post_processor_class_init (LatexilaPostProcessorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = latexila_post_processor_dispose;
  object_class->finalize = latexila_post_processor_finalize;
}

static void
latexila_post_processor_init (LatexilaPostProcessor *self)
{
  LatexilaPostProcessorPrivate *priv = latexila_post_processor_get_instance_private (self);
}
