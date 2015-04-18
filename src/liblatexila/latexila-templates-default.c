/*
 * This file is part of LaTeXila.
 *
 * Copyright (C) 2015 - Sébastien Wilmet <swilmet@gnome.org>
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
 * SECTION:templates-default
 * @title: LatexilaTemplatesDefault
 * @short_description: Default templates
 * @see_also: #LatexilaTemplatesPersonal
 *
 * #LatexilaTemplatesDefault is a singleton class that stores information about
 * default templates. In LaTeXila, new documents are created from templates.
 * There are a few default templates available, and personal templates can be
 * created (see #LatexilaTemplatesPersonal).
 *
 * Each LaTeX user has probably different needs, and has a different set of
 * templates. So it's better to keep a short list of default templates. It would
 * be useless to have a hundred default templates, since anyway they would most
 * probably not fit many users. For example each user has a different preamble,
 * with different packages, configured differently, etc.
 *
 * In the git repository, default templates are located in the data/templates/
 * directory. The templates are stored in XML format, with some chunks that are
 * translatable or not. For example the babel package (or equivalent) is added
 * to the preamble when LaTeXila is run in another language than English (and if
 * a translation is available for that other language). Also, the letter
 * template can be completely translated, using even a different document class
 * that is more suitable for the target language.
 */

#include "config.h"
#include "latexila-templates-default.h"
#include <glib/gi18n.h>
#include "latexila-templates-common.h"

struct _LatexilaTemplatesDefault
{
  GtkListStore parent;
};

G_DEFINE_TYPE (LatexilaTemplatesDefault, latexila_templates_default, GTK_TYPE_LIST_STORE)

static void
latexila_templates_default_class_init (LatexilaTemplatesDefaultClass *klass)
{
}

static void
add_default_template (LatexilaTemplatesDefault *templates,
                      const gchar              *name,
                      const gchar              *config_icon_name,
                      const gchar              *filename)
{
  gchar *path;
  GFile *file;

  path = g_build_filename (DATA_DIR, "templates", filename, NULL);
  file = g_file_new_for_path (path);

  latexila_templates_add_template (GTK_LIST_STORE (templates),
                                   name,
                                   config_icon_name,
                                   file);

  g_free (path);
  g_object_unref (file);
}

static void
latexila_templates_default_init (LatexilaTemplatesDefault *templates)
{
  latexila_templates_init_store (GTK_LIST_STORE (templates));

  latexila_templates_add_template (GTK_LIST_STORE (templates),
                                   _("Empty"),
                                   "empty",
                                   NULL);

  add_default_template (templates, _("Article"), "article", "article.xml");
  add_default_template (templates, _("Report"), "report", "report.xml");
  add_default_template (templates, _("Book"), "book", "book.xml");
  add_default_template (templates, _("Letter"), "letter", "letter.xml");
  add_default_template (templates, _("Presentation"), "beamer", "beamer.xml");
}

/**
 * latexila_templates_default_get_instance:
 *
 * Gets the instance of the #LatexilaTemplatesDefault singleton.
 *
 * Returns: (transfer none): the instance of #LatexilaTemplatesDefault.
 */
LatexilaTemplatesDefault *
latexila_templates_default_get_instance (void)
{
  static LatexilaTemplatesDefault *instance = NULL;

  if (instance == NULL)
    instance = g_object_new (LATEXILA_TYPE_TEMPLATES_DEFAULT, NULL);

  return instance;
}

static void
parser_add_chunk (GString     *string,
                  const gchar *chunk,
                  gint         chunk_len)
{
  if (chunk == NULL)
    return;

  /* Remove the first '\n'. Without this, the XML files would be less well
   * presented.
   */
  if (chunk[0] == '\n')
    {
      chunk = chunk + 1;

      if (chunk_len != -1)
        chunk_len--;
    }

  if (chunk_len != -1)
    g_string_append_len (string, chunk, chunk_len);
  else
    g_string_append (string, chunk);
}

static void
parser_text (GMarkupParseContext  *context,
             const gchar          *text,
             gsize                 text_len,
             gpointer              user_data,
             GError              **error)
{
  GString *template_contents = user_data;
  const gchar *element;
  gchar *text_nul_terminated = NULL;

  element = g_markup_parse_context_get_element (context);

  if (g_strcmp0 (element, "chunk") == 0)
    {
      parser_add_chunk (template_contents, text, text_len);
    }

  else if (g_strcmp0 (element, "translatableChunk") == 0)
    {
      const gchar *chunk;

      text_nul_terminated = g_strndup (text, text_len);
      chunk = _(text_nul_terminated);

      parser_add_chunk (template_contents, chunk, -1);
    }

  else if (g_strcmp0 (element, "babel") == 0)
    {
      const gchar *translated_text;

      text_nul_terminated = g_strndup (text, text_len);
      translated_text = _(text_nul_terminated);

      if (translated_text != text_nul_terminated)
        parser_add_chunk (template_contents, translated_text, -1);
    }

  g_free (text_nul_terminated);
}

/**
 * latexila_templates_default_get_contents:
 * @templates: the #LatexilaTemplatesDefault instance.
 * @path: the #GtkTreePath of a default template.
 *
 * Gets the contents of a default template.
 *
 * TODO load contents asynchronously.
 *
 * Returns: the default template's contents. Free with g_free().
 */
gchar *
latexila_templates_default_get_contents (LatexilaTemplatesDefault *templates,
                                         GtkTreePath              *path)
{
  GtkTreeIter iter;
  GFile *xml_file;
  gchar *xml_contents = NULL;
  gsize xml_length;
  GString *template_contents = NULL;
  GMarkupParser parser = { NULL, NULL, parser_text, NULL, NULL };
  GMarkupParseContext *context = NULL;
  GError *error = NULL;

  g_return_val_if_fail (LATEXILA_IS_TEMPLATES_DEFAULT (templates), NULL);

  gtk_tree_model_get_iter (GTK_TREE_MODEL (templates),
                           &iter,
                           path);

  gtk_tree_model_get (GTK_TREE_MODEL (templates),
                      &iter,
                      LATEXILA_TEMPLATES_COLUMN_FILE, &xml_file,
                      -1);

  if (xml_file == NULL)
    return g_strdup ("");

  g_file_load_contents (xml_file, NULL, &xml_contents, &xml_length, NULL, &error);

  template_contents = g_string_new (NULL);

  if (error != NULL)
    goto out;

  context = g_markup_parse_context_new (&parser, 0, template_contents, NULL);
  g_markup_parse_context_parse (context, xml_contents, xml_length, &error);

out:
  g_object_unref (xml_file);
  g_free (xml_contents);

  if (context != NULL)
    g_markup_parse_context_unref (context);

  if (error != NULL)
    {
      g_warning ("Error when loading default template contents: %s", error->message);
      g_error_free (error);
    }

  return g_string_free (template_contents, FALSE);
}
