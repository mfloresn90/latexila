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

#include "latexila.h"
#include <gtk/gtk.h>

static void
check_build_jobs (LatexilaBuildJob *build_job1,
                  LatexilaBuildJob *build_job2)
{
  gchar *command1;
  gchar *command2;
  LatexilaPostProcessorType pp_type1;
  LatexilaPostProcessorType pp_type2;

  g_object_get (build_job1,
                "command", &command1,
                "post-processor-type", &pp_type1,
                NULL);

  g_object_get (build_job2,
                "command", &command2,
                "post-processor-type", &pp_type2,
                NULL);

  g_assert_cmpstr (command1, ==, command2);
  g_assert_cmpint (pp_type1, ==, pp_type2);

  g_free (command1);
  g_free (command2);
}

static void
check_build_tools (LatexilaBuildTool *build_tool1,
                   LatexilaBuildTool *build_tool2)
{
  gchar *label1;
  gchar *label2;
  gchar *description1;
  gchar *description2;
  gchar *extensions1;
  gchar *extensions2;
  gchar *icon1;
  gchar *icon2;
  gchar *files_to_open1;
  gchar *files_to_open2;
  gint id1;
  gint id2;
  gboolean enabled1;
  gboolean enabled2;
  GList *jobs1;
  GList *jobs2;

  g_object_get (build_tool1,
                "label", &label1,
                "description", &description1,
                "extensions", &extensions1,
                "icon", &icon1,
                "files-to-open", &files_to_open1,
                "id", &id1,
                "enabled", &enabled1,
                NULL);

  g_object_get (build_tool2,
                "label", &label2,
                "description", &description2,
                "extensions", &extensions2,
                "icon", &icon2,
                "files-to-open", &files_to_open2,
                "id", &id2,
                "enabled", &enabled2,
                NULL);

  g_assert_cmpstr (label1, ==, label2);
  g_assert_cmpstr (description1, ==, description2);
  g_assert_cmpstr (extensions1, ==, extensions2);
  g_assert_cmpstr (icon1, ==, icon2);
  g_assert_cmpstr (files_to_open1, ==, files_to_open2);
  g_assert_cmpint (id1, ==, id2);
  g_assert_cmpint (enabled1, ==, enabled2);

  jobs1 = latexila_build_tool_get_jobs (build_tool1);
  jobs2 = latexila_build_tool_get_jobs (build_tool2);

  g_assert_cmpint (g_list_length (jobs1), ==, g_list_length (jobs2));

  for (; jobs1 != NULL; jobs1 = jobs1->next, jobs2 = jobs2->next)
    {
      check_build_jobs (jobs1->data, jobs2->data);
    }

  g_free (label1);
  g_free (label2);
  g_free (description1);
  g_free (description2);
  g_free (extensions1);
  g_free (extensions2);
  g_free (icon1);
  g_free (icon2);
  g_free (files_to_open1);
  g_free (files_to_open2);
}

static void
loaded_cb (LatexilaBuildTools *build_tools)
{
  GList *list = NULL;
  LatexilaBuildTool *build_tool;
  LatexilaBuildJob *build_job;
  GList *l1;
  GList *l2;

  /* First build tool */

  build_tool = latexila_build_tool_new ();

  g_object_set (build_tool,
                "id", 1,
                "enabled", TRUE,
                "extensions", ".tex",
                "icon", "compile_pdf",
                "label", "LaTeX → PDF (Latexmk)",
                "files-to-open", "$shortname.pdf",
                NULL);

  build_job = latexila_build_job_new ();

  g_object_set (build_job,
                "post-processor-type", LATEXILA_POST_PROCESSOR_TYPE_LATEXMK,
                "command", "latexmk -pdf -synctex=1 $filename",
                NULL);

  latexila_build_tool_add_job (build_tool, build_job);
  g_object_unref (build_job);

  build_job = latexila_build_job_new ();

  g_object_set (build_job,
                "post-processor-type", LATEXILA_POST_PROCESSOR_TYPE_NO_OUTPUT,
                "command", "build job 2",
                NULL);

  latexila_build_tool_add_job (build_tool, build_job);
  g_object_unref (build_job);

  list = g_list_append (list, build_tool);

  /* Second build tool */

  build_tool = latexila_build_tool_new ();

  g_object_set (build_tool,
                "enabled", FALSE,
                "label", "build tool 2",
                NULL);

  list = g_list_append (list, build_tool);

  /* Compare */

  g_assert_cmpint (g_list_length (list), ==, g_list_length (build_tools->build_tools));

  for (l1 = list, l2 = build_tools->build_tools;
       l1 != NULL && l2 != NULL;
       l1 = l1->next, l2 = l2->next)
    {
      check_build_tools (l1->data, l2->data);
    }

  g_list_free_full (list, g_object_unref);
  gtk_main_quit ();
}

static void
test_load (void)
{
  LatexilaBuildTools *build_tools;
  gchar *path;
  GFile *xml_file;

  path = g_build_filename (SRCDIR, "build_tools_test.xml", NULL);
  xml_file = g_file_new_for_path (path);
  g_free (path);

  build_tools = g_object_new (LATEXILA_TYPE_BUILD_TOOLS, NULL);

  g_signal_connect (build_tools,
                    "loaded",
                    G_CALLBACK (loaded_cb),
                    NULL);

  latexila_build_tools_load (build_tools, xml_file);
  g_object_unref (xml_file);

  gtk_main ();
  g_object_unref (build_tools);
}

gint
main (gint    argc,
      gchar **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/build-tools/load", test_load);

  return g_test_run ();
}
