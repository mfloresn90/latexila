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
 * SECTION:build-tools-personal
 * @title: LatexilaBuildToolsPersonal
 * @short_description: Personal build tools
 *
 * The #LatexilaBuildToolsPersonal singleton class represents the personal build
 * tools. The personal build tools can be entirely modified. The XML file can be
 * saved with latexila_build_tools_personal_save().
 */

#include "latexila-build-tools-personal.h"
#include <gio/gio.h>

static LatexilaBuildToolsPersonal *instance = NULL;

struct _LatexilaBuildToolsPersonalPrivate
{
  /* Used for saving */
  GString *xml_file_contents;

  guint modified : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaBuildToolsPersonal, latexila_build_tools_personal, LATEXILA_TYPE_BUILD_TOOLS)

static void
latexila_build_tools_personal_finalize (GObject *object)
{
  LatexilaBuildToolsPersonal *build_tools = LATEXILA_BUILD_TOOLS_PERSONAL (object);

  if (build_tools->priv->xml_file_contents != NULL)
    {
      g_string_free (build_tools->priv->xml_file_contents, TRUE);
      build_tools->priv->xml_file_contents = NULL;
    }

  G_OBJECT_CLASS (latexila_build_tools_personal_parent_class)->finalize (object);
}

static void
latexila_build_tools_personal_class_init (LatexilaBuildToolsPersonalClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = latexila_build_tools_personal_finalize;
}

static GFile *
get_xml_file (void)
{
  gchar *path;
  GFile *file;

  path = g_build_filename (g_get_user_config_dir (),
                           "latexila",
                           "build_tools.xml",
                           NULL);

  file = g_file_new_for_path (path);
  g_free (path);

  return file;
}

static void
modified_cb (LatexilaBuildToolsPersonal *build_tools)
{
  build_tools->priv->modified = TRUE;
}

static void
latexila_build_tools_personal_init (LatexilaBuildToolsPersonal *build_tools)
{
  GFile *xml_file;

  build_tools->priv = latexila_build_tools_personal_get_instance_private (build_tools);

  g_signal_connect (build_tools,
                    "modified",
                    G_CALLBACK (modified_cb),
                    NULL);

  xml_file = get_xml_file ();
  latexila_build_tools_load (LATEXILA_BUILD_TOOLS (build_tools), xml_file);
  g_object_unref (xml_file);
}

/**
 * latexila_build_tools_personal_get_instance:
 *
 * Gets the instance of the #LatexilaBuildToolsPersonal singleton.
 *
 * Returns: the instance of #LatexilaBuildToolsPersonal.
 */
LatexilaBuildToolsPersonal *
latexila_build_tools_personal_get_instance (void)
{
  if (instance == NULL)
    {
      instance = g_object_new (LATEXILA_TYPE_BUILD_TOOLS_PERSONAL, NULL);
    }

  return instance;
}

static void
save_cb (GFile                      *xml_file,
         GAsyncResult               *result,
         LatexilaBuildToolsPersonal *build_tools)
{
  GError *error = NULL;

  g_file_replace_contents_finish (xml_file, result, NULL, &error);

  if (error != NULL)
    {
      g_warning ("Error while saving the personal build tools: %s",
                 error->message);
      g_error_free (error);
    }
  else
    {
      build_tools->priv->modified = FALSE;
    }

  g_string_free (build_tools->priv->xml_file_contents, TRUE);
  build_tools->priv->xml_file_contents = NULL;

  g_object_unref (build_tools);
}

/**
 * latexila_build_tools_personal_save:
 * @build_tools: a #LatexilaBuildToolsPersonal object.
 *
 * Saves the personal build tools into the XML file.
 */
void
latexila_build_tools_personal_save (LatexilaBuildToolsPersonal *build_tools)
{
  LatexilaBuildTools *build_tools_parent = LATEXILA_BUILD_TOOLS (build_tools);
  GString *contents;
  GList *cur_build_tool;
  GFile *xml_file;

  g_return_if_fail (LATEXILA_IS_BUILD_TOOLS_PERSONAL (build_tools));

  if (!build_tools->priv->modified)
    {
      return;
    }

  if (build_tools->priv->xml_file_contents != NULL)
    {
      g_string_free (build_tools->priv->xml_file_contents, TRUE);
    }

  contents = g_string_new ("<tools>");
  build_tools->priv->xml_file_contents = contents;

  for (cur_build_tool = build_tools_parent->build_tools;
       cur_build_tool != NULL;
       cur_build_tool = cur_build_tool->next)
    {
      LatexilaBuildTool *build_tool = cur_build_tool->data;
      gchar *escaped_text;
      GSList *cur_build_job;

      g_string_append_printf (contents,
                              "\n  <tool enabled=\"%s\" extensions=\"%s\" icon=\"%s\">\n",
                              build_tool->enabled ? "true" : "false",
                              build_tool->extensions != NULL ? build_tool->extensions : "",
                              build_tool->icon != NULL ? build_tool->icon : "");

      escaped_text = g_markup_printf_escaped ("    <label>%s</label>\n"
                                              "    <description>%s</description>\n",
                                              build_tool->label != NULL ? build_tool->label : "",
                                              build_tool->description != NULL ? build_tool->description : "");

      g_string_append (contents, escaped_text);
      g_free (escaped_text);

      for (cur_build_job = build_tool->jobs;
           cur_build_job != NULL;
           cur_build_job = cur_build_job->next)
        {
          LatexilaBuildJob *build_job = cur_build_job->data;

          escaped_text = g_markup_printf_escaped ("    <job postProcessor=\"%s\">%s</job>\n",
                                                  latexila_get_post_processor_name_from_type (build_job->post_processor_type),
                                                  build_job->command != NULL ? build_job->command : "");

          g_string_append (contents, escaped_text);
          g_free (escaped_text);
        }

      escaped_text = g_markup_printf_escaped ("    <open>%s</open>\n",
                                              build_tool->files_to_open != NULL ? build_tool->files_to_open : "");
      g_string_append (contents, escaped_text);
      g_free (escaped_text);

      g_string_append (contents, "  </tool>\n");
    }

  g_string_append (contents, "</tools>\n");

  /* Avoid finalization of build_tools during the async operation. */
  g_object_ref (build_tools);

  xml_file = get_xml_file ();

  g_file_replace_contents_async (xml_file,
                                 contents->str,
                                 contents->len,
                                 NULL,
                                 TRUE, /* make a backup */
                                 G_FILE_CREATE_NONE,
                                 NULL,
                                 (GAsyncReadyCallback) save_cb,
                                 build_tools);

  g_object_unref (xml_file);
}

/**
 * latexila_build_tools_personal_move_up:
 * @build_tools:
 * @tool_num:
 *
 * Move a build tool up. The first build tool is at the top.
 */
void
latexila_build_tools_personal_move_up (LatexilaBuildToolsPersonal *build_tools,
                                       guint                       tool_num)
{
  LatexilaBuildTools *build_tools_parent = LATEXILA_BUILD_TOOLS (build_tools);
  GList *node;
  GList *prev_node;

  g_return_if_fail (LATEXILA_IS_BUILD_TOOLS_PERSONAL (build_tools));

  node = g_list_nth (build_tools_parent->build_tools, tool_num);
  g_return_if_fail (node != NULL);

  prev_node = node->prev;
  g_return_if_fail (prev_node != NULL);

  build_tools_parent->build_tools = g_list_remove_link (build_tools_parent->build_tools, node);

  build_tools_parent->build_tools = g_list_insert_before (build_tools_parent->build_tools,
                                                          prev_node,
                                                          node->data);

  g_list_free (node);

  g_signal_emit_by_name (build_tools, "modified");
}

/**
 * latexila_build_tools_personal_move_down:
 * @build_tools:
 * @tool_num:
 *
 * Move a build tool down. The first build tool is at the top.
 */
void
latexila_build_tools_personal_move_down (LatexilaBuildToolsPersonal *build_tools,
                                         guint                       tool_num)
{
  LatexilaBuildTools *build_tools_parent = LATEXILA_BUILD_TOOLS (build_tools);
  GList *node;
  GList *next_node;

  g_return_if_fail (LATEXILA_IS_BUILD_TOOLS_PERSONAL (build_tools));

  node = g_list_nth (build_tools_parent->build_tools, tool_num);
  g_return_if_fail (node != NULL);

  next_node = node->next;
  g_return_if_fail (next_node != NULL);

  build_tools_parent->build_tools = g_list_remove_link (build_tools_parent->build_tools, node);

  build_tools_parent->build_tools = g_list_insert_before (build_tools_parent->build_tools,
                                                          next_node->next,
                                                          node->data);

  g_list_free (node);

  g_signal_emit_by_name (build_tools, "modified");
}

/**
 * latexila_build_tools_personal_delete:
 * @build_tools:
 * @tool_num:
 */
void
latexila_build_tools_personal_delete (LatexilaBuildToolsPersonal *build_tools,
                                      guint                       tool_num)
{
  LatexilaBuildTools *build_tools_parent = LATEXILA_BUILD_TOOLS (build_tools);
  GList *node;

  g_return_if_fail (LATEXILA_IS_BUILD_TOOLS_PERSONAL (build_tools));

  node = g_list_nth (build_tools_parent->build_tools, tool_num);
  g_return_if_fail (node != NULL);

  build_tools_parent->build_tools = g_list_remove_link (build_tools_parent->build_tools, node);

  g_list_free_full (node, (GDestroyNotify) latexila_build_tool_free);

  g_signal_emit_by_name (build_tools, "modified");
}

/**
 * latexila_build_tools_personal_add:
 * @build_tools:
 * @new_build_tool:
 *
 * Append the new build tool at the end.
 */
void
latexila_build_tools_personal_add (LatexilaBuildToolsPersonal *build_tools,
                                   LatexilaBuildTool          *new_build_tool)
{
  LatexilaBuildTools *build_tools_parent = LATEXILA_BUILD_TOOLS (build_tools);

  g_return_if_fail (LATEXILA_IS_BUILD_TOOLS_PERSONAL (build_tools));

  build_tools_parent->build_tools = g_list_append (build_tools_parent->build_tools,
                                                   new_build_tool);

  g_signal_emit_by_name (build_tools, "modified");
}

/**
 * latexila_build_tools_personal_insert:
 * @build_tools:
 * @new_build_tool:
 * @position:
 *
 * Inserts a new build tool at a given position.
 */
void
latexila_build_tools_personal_insert (LatexilaBuildToolsPersonal *build_tools,
                                      LatexilaBuildTool          *new_build_tool,
                                      guint                       position)
{
  LatexilaBuildTools *build_tools_parent = LATEXILA_BUILD_TOOLS (build_tools);

  g_return_if_fail (LATEXILA_IS_BUILD_TOOLS_PERSONAL (build_tools));

  build_tools_parent->build_tools = g_list_insert (build_tools_parent->build_tools,
                                                   new_build_tool,
                                                   position);

  g_signal_emit_by_name (build_tools, "modified");
}

/**
 * latexila_build_tools_personal_replace:
 * @build_tools:
 * @new_build_tool:
 * @position:
 *
 * Replaces a build tool.
 */
void
latexila_build_tools_personal_replace (LatexilaBuildToolsPersonal *build_tools,
                                       LatexilaBuildTool          *new_build_tool,
                                       guint                       position)
{
  LatexilaBuildTools *build_tools_parent = LATEXILA_BUILD_TOOLS (build_tools);
  GList *node;

  g_return_if_fail (LATEXILA_IS_BUILD_TOOLS_PERSONAL (build_tools));

  node = g_list_nth (build_tools_parent->build_tools, position);
  g_return_if_fail (node != NULL);

  if (node->data != new_build_tool)
    {
      latexila_build_tool_free (node->data);
      node->data = new_build_tool;

      g_signal_emit_by_name (build_tools, "modified");
    }
}
