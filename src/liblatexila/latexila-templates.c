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
 * SECTION:templates
 * @title: LatexilaTemplates
 * @short_description: Templates
 *
 * #LatexilaTemplates is a singleton class that stores information about
 * templates. In LaTeXila, new documents are created from templates. There are a
 * few default templates available, and personal templates can be created.
 *
 * Each LaTeX user has probably different needs, and has a different set of
 * templates. So it's better to keep a short list of default templates. It would
 * be useless to have a hundred default templates, since anyway they would most
 * probably not fit many users. For example each user has a different preamble,
 * with different packages, configured differently, etc.
 *
 * Personal templates are stored in the ~/.local/share/latexila/ directory.
 * There is a templatesrc file that stores the list of names and icons. And the
 * templates' contents are stored in 0.tex, 1.tex, 2.tex, etc, in the same order
 * as in the templatesrc file.
 *
 * Default templates are a bit more complicated, since translations are
 * available. In the git repository, they are located in the data/templates/
 * directory. The templates are stored in XML, with some chunks that are
 * translatable or not. For example the babel package (or equivalent) is added
 * to the preamble when LaTeXila is run in another language than English (and if
 * a translation is available for that other language). Also, the letter
 * template can be completely translated, using even a different document class
 * that is more suitable for the target language.
 */

#include "config.h"
#include "latexila-templates.h"
#include <glib/gi18n.h>
#include <string.h>

typedef struct _LatexilaTemplatesPrivate LatexilaTemplatesPrivate;

struct _LatexilaTemplates
{
  GObject parent;
};

struct _LatexilaTemplatesPrivate
{
  /* Contains the default templates (empty, article, report, ...). */
  GtkListStore *default_store;

  /* Contains the personal templates (created by the user). */
  GtkListStore *personal_store;
};

enum
{
  COLUMN_PIXBUF_ICON_NAME,

  /* The string stored in the rc file (article, report, ...). */
  COLUMN_CONFIG_ICON_NAME,

  COLUMN_NAME,

  /* The file where is stored the contents. For a default template this is an
   * XML file, for a personal template this is a .tex file. A NULL file is
   * valid for a default template, it means an empty template.
   */
  COLUMN_FILE,

  N_COLUMNS
};

#define GET_PRIV(self) (latexila_templates_get_instance_private (self))

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaTemplates, latexila_templates, G_TYPE_OBJECT)

static GtkListStore *
create_new_store (void)
{
  return gtk_list_store_new (N_COLUMNS,
                             G_TYPE_STRING,
                             G_TYPE_STRING,
                             G_TYPE_STRING,
                             G_TYPE_FILE);
}

/* For compatibility reasons. @config_icon_name is the string stored in the rc
 * file, and the return value is the theme icon name used for the pixbuf. If we
 * store directly the theme icon names in the rc file, old rc files must be
 * modified via a script for example, but it's simpler like that.
 */
static const gchar *
get_pixbuf_icon_name (const gchar *config_icon_name)
{
  g_return_val_if_fail (config_icon_name != NULL, NULL);

  if (g_str_equal (config_icon_name, "empty"))
    return "text-x-preview";

  if (g_str_equal (config_icon_name, "article"))
    return "text-x-generic";

  if (g_str_equal (config_icon_name, "report"))
    return "x-office-document";

  if (g_str_equal (config_icon_name, "book"))
    return "accessories-dictionary";

  if (g_str_equal (config_icon_name, "letter"))
    return "emblem-mail";

  if (g_str_equal (config_icon_name, "beamer"))
    return "x-office-presentation";

  g_return_val_if_reached (NULL);
}

static void
add_template (GtkListStore *store,
              const gchar  *name,
              const gchar  *config_icon_name,
              GFile        *file)
{
  GtkTreeIter iter;

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      COLUMN_PIXBUF_ICON_NAME, get_pixbuf_icon_name (config_icon_name),
                      COLUMN_CONFIG_ICON_NAME, config_icon_name,
                      COLUMN_NAME, name,
                      COLUMN_FILE, file,
                      -1);
}

static void
add_default_template (GtkListStore *store,
                      const gchar  *name,
                      const gchar  *config_icon_name,
                      const gchar  *filename)
{
  gchar *path;
  GFile *file;

  path = g_build_filename (DATA_DIR, "templates", filename, NULL);
  file = g_file_new_for_path (path);

  add_template (store, name, config_icon_name, file);

  g_free (path);
  g_object_unref (file);
}

static void
init_default_templates (LatexilaTemplates *templates)
{
  LatexilaTemplatesPrivate *priv = GET_PRIV (templates);

  priv->default_store = create_new_store ();

  add_template (priv->default_store, _("Empty"), "empty", NULL);
  add_default_template (priv->default_store, _("Article"), "article", "article.xml");
  add_default_template (priv->default_store, _("Report"), "report", "report.xml");
  add_default_template (priv->default_store, _("Book"), "book", "book.xml");
  add_default_template (priv->default_store, _("Letter"), "letter", "letter.xml");
  add_default_template (priv->default_store, _("Presentation"), "beamer", "beamer.xml");
}

static GFile *
get_rc_file (void)
{
  gchar *path;
  GFile *rc_file;

  path = g_build_filename (g_get_user_data_dir (), "latexila", "templatesrc", NULL);
  rc_file = g_file_new_for_path (path);

  g_free (path);
  return rc_file;
}

static GFile *
get_personal_template_file (gint template_num)
{
  gchar *filename;
  gchar *path;
  GFile *template_file;

  filename = g_strdup_printf ("%d.tex", template_num);
  path = g_build_filename (g_get_user_data_dir (), "latexila", filename, NULL);
  template_file = g_file_new_for_path (path);

  g_free (filename);
  g_free (path);
  return template_file;
}

static void
rc_file_contents_loaded_cb (GFile             *rc_file,
                            GAsyncResult      *result,
                            LatexilaTemplates *templates)
{
  LatexilaTemplatesPrivate *priv = GET_PRIV (templates);
  gchar *contents = NULL;
  gsize length;
  GKeyFile *key_file = NULL;
  gchar **names = NULL;
  gchar **icons = NULL;
  gsize n_names;
  gsize n_icons;
  gint i;
  GError *error = NULL;

  g_file_load_contents_finish (rc_file, result, &contents, &length, NULL, &error);

  if (error != NULL)
    {
      /* If the rc file doesn't exist, it means that there is no personal
       * templates.
       */
      if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
        {
          g_error_free (error);
          error = NULL;
        }

      goto out;
    }

  key_file = g_key_file_new ();
  g_key_file_load_from_data (key_file, contents, length, G_KEY_FILE_NONE, &error);

  if (error != NULL)
    goto out;

  names = g_key_file_get_string_list (key_file, PACKAGE_NAME, "names", &n_names, &error);

  if (error != NULL)
    goto out;

  icons = g_key_file_get_string_list (key_file, PACKAGE_NAME, "icons", &n_icons, &error);

  if (error != NULL)
    goto out;

  g_return_if_fail (n_names == n_icons);

  for (i = 0; i < n_names; i++)
    {
      GFile *template_file;

      template_file = get_personal_template_file (i);
      add_template (priv->personal_store, names[i], icons[i], template_file);

      g_object_unref (template_file);
    }

out:

  if (error != NULL)
    {
      g_warning ("The loading of personal templates failed: %s", error->message);
      g_error_free (error);
    }

  g_free (contents);
  g_strfreev (names);
  g_strfreev (icons);

  if (key_file != NULL)
    g_key_file_unref (key_file);

  /* Async operation finished. */
  g_object_unref (templates);
}

static void
init_personal_templates (LatexilaTemplates *templates)
{
  LatexilaTemplatesPrivate *priv = GET_PRIV (templates);
  GFile *rc_file;

  priv->personal_store = create_new_store ();

  rc_file = get_rc_file ();

  /* Prevent @templates from being destroyed during the async operation. */
  g_object_ref (templates);

  g_file_load_contents_async (rc_file,
                              NULL,
                              (GAsyncReadyCallback) rc_file_contents_loaded_cb,
                              templates);
}

static void
latexila_templates_dispose (GObject *object)
{
  LatexilaTemplatesPrivate *priv = GET_PRIV (LATEXILA_TEMPLATES (object));

  g_clear_object (&priv->default_store);
  g_clear_object (&priv->personal_store);

  G_OBJECT_CLASS (latexila_templates_parent_class)->dispose (object);
}

static void
latexila_templates_class_init (LatexilaTemplatesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = latexila_templates_dispose;
}

static void
latexila_templates_init (LatexilaTemplates *templates)
{
  init_default_templates (templates);
  init_personal_templates (templates);
}

/**
 * latexila_templates_get_instance:
 *
 * Gets the instance of the #LatexilaTemplates singleton.
 *
 * Returns: (transfer none): the instance of #LatexilaTemplates.
 */
LatexilaTemplates *
latexila_templates_get_instance (void)
{
  static LatexilaTemplates *instance = NULL;

  if (instance == NULL)
    instance = g_object_new (LATEXILA_TYPE_TEMPLATES, NULL);

  return instance;
}

static GtkTreeView *
get_view (GtkListStore *store)
{
  GtkTreeView *view;
  GtkTreeSelection *selection;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  view = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (store)));
  gtk_tree_view_set_headers_visible (view, FALSE);
  gtk_widget_set_hexpand (GTK_WIDGET (view), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (view), TRUE);

  selection = gtk_tree_view_get_selection (view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

  /* Icon */
  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (renderer, "stock-size", GTK_ICON_SIZE_BUTTON, NULL);

  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "icon-name", COLUMN_PIXBUF_ICON_NAME,
                                                     NULL);

  gtk_tree_view_append_column (view, column);

  /* Name */
  renderer = gtk_cell_renderer_text_new ();

  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "text", COLUMN_NAME,
                                                     NULL);

  gtk_tree_view_append_column (view, column);

  return view;
}

/**
 * latexila_templates_get_default_templates_view:
 * @templates: the #LatexilaTemplates instance.
 *
 * Gets the view for default templates.
 *
 * Returns: (transfer floating): the #GtkTreeView containing the default
 * templates.
 */
GtkTreeView *
latexila_templates_get_default_templates_view (LatexilaTemplates *templates)
{
  LatexilaTemplatesPrivate *priv;

  g_return_val_if_fail (LATEXILA_IS_TEMPLATES (templates), NULL);

  priv = GET_PRIV (templates);

  return get_view (priv->default_store);
}

/**
 * latexila_templates_get_personal_templates_view:
 * @templates: the #LatexilaTemplates instance.
 *
 * Gets the view for personal templates.
 *
 * Returns: (transfer floating): the #GtkTreeView containing the personal
 * templates.
 */
GtkTreeView *
latexila_templates_get_personal_templates_view (LatexilaTemplates *templates)
{
  LatexilaTemplatesPrivate *priv;

  g_return_val_if_fail (LATEXILA_IS_TEMPLATES (templates), NULL);

  priv = GET_PRIV (templates);

  return get_view (priv->personal_store);
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
 * latexila_templates_get_default_template_contents:
 * @templates: the #LatexilaTemplates instance.
 * @path: the #GtkTreePath of a default template.
 *
 * Gets the contents of a default template. The @path must be obtained via the
 * #GtkTreeView returned by latexila_templates_get_default_templates_view().
 *
 * TODO load contents asynchronously.
 *
 * Returns: the default template contents. Free with g_free().
 */
gchar *
latexila_templates_get_default_template_contents (LatexilaTemplates *templates,
                                                  GtkTreePath       *path)
{
  LatexilaTemplatesPrivate *priv;
  GtkTreeIter iter;
  GFile *xml_file;
  gchar *xml_contents = NULL;
  gsize xml_length;
  GString *template_contents = NULL;
  GMarkupParser parser = { NULL, NULL, parser_text, NULL, NULL };
  GMarkupParseContext *context = NULL;
  GError *error = NULL;

  g_return_val_if_fail (LATEXILA_IS_TEMPLATES (templates), NULL);

  priv = GET_PRIV (templates);

  gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->default_store),
                           &iter,
                           path);

  gtk_tree_model_get (GTK_TREE_MODEL (priv->default_store),
                      &iter,
                      COLUMN_FILE, &xml_file,
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

/**
 * latexila_templates_get_personal_template_contents:
 * @templates: the #LatexilaTemplates instance.
 * @path: the #GtkTreePath of a personal template.
 *
 * Gets the contents of a personal template. The @path must be obtained via the
 * #GtkTreeView returned by latexila_templates_get_personal_templates_view().
 *
 * TODO load contents asynchronously, with a GtkSourceFileLoader.
 *
 * Returns: the personal template contents. Free with g_free().
 */
gchar *
latexila_templates_get_personal_template_contents (LatexilaTemplates *templates,
                                                   GtkTreePath       *path)
{
  LatexilaTemplatesPrivate *priv;
  GtkTreeIter iter;
  GFile *file;
  gchar *contents = NULL;
  GError *error = NULL;

  g_return_val_if_fail (LATEXILA_IS_TEMPLATES (templates), NULL);

  priv = GET_PRIV (templates);

  gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->personal_store),
                           &iter,
                           path);

  gtk_tree_model_get (GTK_TREE_MODEL (priv->personal_store),
                      &iter,
                      COLUMN_FILE, &file,
                      -1);

  g_return_val_if_fail (G_IS_FILE (file), NULL);

  g_file_load_contents (file, NULL, &contents, NULL, NULL, &error);

  if (error != NULL)
    {
      g_warning ("Error when loading personal template contents: %s", error->message);
      g_error_free (error);
    }

  g_object_unref (file);
  return contents;
}
