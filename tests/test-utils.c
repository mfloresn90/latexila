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

static void
test_get_shortname (void)
{
  gchar *shortname;

  shortname = latexila_utils_get_shortname ("file.txt");
  g_assert_cmpstr (shortname, ==, "file");
  g_free (shortname);

  shortname = latexila_utils_get_shortname ("file.tar.gz");
  g_assert_cmpstr (shortname, ==, "file.tar");
  g_free (shortname);

  shortname = latexila_utils_get_shortname ("file");
  g_assert_cmpstr (shortname, ==, "file");
  g_free (shortname);

  shortname = latexila_utils_get_shortname ("dir.ext/blah");
  g_assert_cmpstr (shortname, ==, "dir.ext/blah");
  g_free (shortname);
}

static void
test_get_extension (void)
{
  gchar *extension;

  extension = latexila_utils_get_extension ("file.pdf");
  g_assert_cmpstr (extension, ==, ".pdf");
  g_free (extension);

  extension = latexila_utils_get_extension ("file.tar.gz");
  g_assert_cmpstr (extension, ==, ".gz");
  g_free (extension);

  extension = latexila_utils_get_extension ("file");
  g_assert_cmpstr (extension, ==, "");
  g_free (extension);
}

static void
test_replace_home_dir_with_tilde (void)
{
  const gchar *homedir = g_get_home_dir ();
  gchar *before;
  gchar *after;

  before = g_build_filename (homedir, "blah", NULL);
  after = latexila_utils_replace_home_dir_with_tilde (before);
  g_assert_cmpstr (after, ==, "~/blah");
  g_free (before);
  g_free (after);

  after = latexila_utils_replace_home_dir_with_tilde (homedir);
  g_assert_cmpstr (after, ==, "~");
  g_free (after);

  after = latexila_utils_replace_home_dir_with_tilde ("/blah");
  g_assert_cmpstr (after, ==, "/blah");
  g_free (after);
}

static void
test_str_replace (void)
{
  gchar *result;

  result = latexila_utils_str_replace ("$filename", "$filename", "blah");
  g_assert_cmpstr (result, ==, "blah");
  g_free (result);

  result = latexila_utils_str_replace ("$shortname.pdf", "$shortname", "blah");
  g_assert_cmpstr (result, ==, "blah.pdf");
  g_free (result);

  result = latexila_utils_str_replace ("abcdabcd", "ab", "r");
  g_assert_cmpstr (result, ==, "rcdrcd");
  g_free (result);

  result = latexila_utils_str_replace ("abcd", "ef", "r");
  g_assert_cmpstr (result, ==, "abcd");
  g_free (result);
}

gint
main (gint    argc,
      gchar **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/utils/get-shortname", test_get_shortname);
  g_test_add_func ("/utils/get-extension", test_get_extension);
  g_test_add_func ("/utils/replace-home-dir-with-tilde", test_replace_home_dir_with_tilde);
  g_test_add_func ("/utils/str-replace", test_str_replace);

  return g_test_run ();
}
