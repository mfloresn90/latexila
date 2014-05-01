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
 * SECTION:build-tools-default
 * @title: LatexilaBuildToolsDefault
 * @short_description: Default build tools
 *
 * The #LatexilaBuildToolsDefault singleton class represents the default build
 * tools of LaTeXila. The only possible modification is to enable or disable a
 * build tool. Each default build tool has an ID. These IDs are used to load and
 * save the lists of enabled and disabled build tools. The XML file is never
 * modified by LaTeXila. But the XML file (located in data/build_tools/) can be
 * modified by a developer to change a command, add a new build tool (with a new
 * ID), etc. The changes will automatically be available to all the users when
 * upgrading to the new LaTeXila version. That's why the default build tools can
 * not be modified and are not saved to another XML file.
 */

#include "config.h"
#include "latexila-build-tools-default.h"
#include <gio/gio.h>

static LatexilaBuildToolsDefault *instance = NULL;

struct _LatexilaBuildToolsDefaultPrivate
{
  gint something; /* not used, but the struct can not be empty */
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaBuildToolsDefault, latexila_build_tools_default, LATEXILA_TYPE_BUILD_TOOLS)

static void
set_enabled_by_id (LatexilaBuildToolsDefault *build_tools,
                   gint                       build_tool_id,
                   gboolean                   enabled)
{
  LatexilaBuildTools *build_tools_parent = LATEXILA_BUILD_TOOLS (build_tools);
  GList *l;

  for (l = build_tools_parent->build_tools; l != NULL; l = l->next)
    {
      LatexilaBuildTool *build_tool = l->data;
      gint id;

      g_object_get (build_tool, "id", &id, NULL);

      if (id == build_tool_id)
        {
          g_object_set (build_tool, "enabled", enabled, NULL);
          return;
        }
    }
}

/* Enable or disable the build tools.
 * There are two lists: the enabled build tools IDs, and the disabled build tools IDs.
 * By default, the two lists are empty. If an ID is in a list, it will override the
 * default value found in the XML file. So when a new default build tool is added,
 * it is not present in the lists, and it automatically gets the default value from
 * the XML file.
 */
static void
load_settings (LatexilaBuildToolsDefault *build_tools)
{
  GSettings *settings;
  GVariant *tools;
  GVariantIter *iter;
  gint tool_id;

  settings = g_settings_new ("org.gnome.latexila.preferences.latex");

  tools = g_settings_get_value (settings, "enabled-default-build-tools");
  g_variant_get (tools, "ai", &iter);

  while (g_variant_iter_loop (iter, "i", &tool_id))
    {
      set_enabled_by_id (build_tools, tool_id, TRUE);
    }

  g_variant_iter_free (iter);
  g_variant_unref (tools);

  tools = g_settings_get_value (settings, "disabled-default-build-tools");
  g_variant_get (tools, "ai", &iter);

  while (g_variant_iter_loop (iter, "i", &tool_id))
    {
      set_enabled_by_id (build_tools, tool_id, FALSE);
    }

  g_variant_iter_free (iter);
  g_variant_unref (tools);
  g_object_unref (settings);
}

static void
save_settings (LatexilaBuildToolsDefault *build_tools)
{
  LatexilaBuildTools *build_tools_parent = LATEXILA_BUILD_TOOLS (build_tools);
  GVariantBuilder builder_enabled;
  GVariantBuilder builder_disabled;
  GVariant *enabled_tools;
  GVariant *disabled_tools;
  GSettings *settings;
  GList *l;

  g_variant_builder_init (&builder_enabled, G_VARIANT_TYPE_ARRAY);
  g_variant_builder_init (&builder_disabled, G_VARIANT_TYPE_ARRAY);

  for (l = build_tools_parent->build_tools; l != NULL; l = l->next)
    {
      LatexilaBuildTool *build_tool = l->data;
      gboolean enabled;
      gint id;

      g_object_get (build_tool,
                    "enabled", &enabled,
                    "id", &id,
                    NULL);

      if (enabled)
        {
          g_variant_builder_add (&builder_enabled, "i", id);
        }
      else
        {
          g_variant_builder_add (&builder_disabled, "i", id);
        }
    }

  enabled_tools = g_variant_builder_end (&builder_enabled);
  disabled_tools = g_variant_builder_end (&builder_disabled);

  settings = g_settings_new ("org.gnome.latexila.preferences.latex");
  g_settings_set_value (settings, "enabled-default-build-tools", enabled_tools);
  g_settings_set_value (settings, "disabled-default-build-tools", disabled_tools);

  g_variant_unref (enabled_tools);
  g_variant_unref (disabled_tools);
  g_object_unref (settings);
}

#if 0
static void
latexila_build_tools_default_finalize (GObject *object)
{

  G_OBJECT_CLASS (latexila_build_tools_default_parent_class)->finalize (object);
}
#endif

static void
latexila_build_tools_default_class_init (LatexilaBuildToolsDefaultClass *klass)
{
#if 0
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = latexila_build_tools_default_finalize;
#endif
}

static GFile *
get_xml_file (void)
{
  gchar *path;
  GFile *file;

  path = g_build_filename (DATA_DIR, "build_tools.xml", NULL);
  file = g_file_new_for_path (path);
  g_free (path);

  return file;
}

static void
latexila_build_tools_default_init (LatexilaBuildToolsDefault *build_tools)
{
  GFile *xml_file;

  build_tools->priv = latexila_build_tools_default_get_instance_private (build_tools);

  /* load_settings() will be called directly after the file is loaded. So if
   * external code connects to the "loaded" signal too, normally the settings
   * will be loaded too.
   */
  g_signal_connect (build_tools,
                    "loaded",
                    G_CALLBACK (load_settings),
                    NULL);

  g_signal_connect (build_tools,
                    "modified",
                    G_CALLBACK (save_settings),
                    NULL);

  xml_file = get_xml_file ();
  latexila_build_tools_load (LATEXILA_BUILD_TOOLS (instance), xml_file);
  g_object_unref (xml_file);
}

/**
 * latexila_build_tools_default_get_instance:
 *
 * Gets the instance of the #LatexilaBuildToolsDefault singleton.
 *
 * Returns: (transfer none): the instance of #LatexilaBuildToolsDefault.
 */
LatexilaBuildToolsDefault *
latexila_build_tools_default_get_instance (void)
{
  if (instance == NULL)
    {
      instance = g_object_new (LATEXILA_TYPE_BUILD_TOOLS_DEFAULT, NULL);
    }

  return instance;
}
