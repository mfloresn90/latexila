/*
 * This file is part of LaTeXila.
 *
 * From gedit-utils.c:
 * Copyright (C) 1998, 1999 - Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2002 - Chema Celorio, Paolo Maggi
 * Copyright (C) 2003-2005 - Paolo Maggi
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
 * SECTION:utils
 * @title: LatexilaUtils
 * @short_description: Utilities functions
 *
 * Various utilities functions.
 */

#include "latexila-utils.h"
#include <string.h>

/**
 * latexila_utils_replace_home_dir_with_tilde:
 * @filename: the filename.
 *
 * Replaces the home directory with a tilde, if the home directory is present in
 * the @filename.
 *
 * This function comes from gedit.
 *
 * Returns: the new filename. Free with g_free().
 */
gchar *
latexila_utils_replace_home_dir_with_tilde (const gchar *filename)
{
  gchar *tmp;
  gchar *home;

  g_return_val_if_fail (filename != NULL, NULL);

  /* Note that g_get_home_dir returns a const string */
  tmp = (gchar *) g_get_home_dir ();

  if (tmp == NULL)
    {
      return g_strdup (filename);
    }

  home = g_filename_to_utf8 (tmp, -1, NULL, NULL, NULL);
  if (home == NULL)
    {
      return g_strdup (filename);
    }

  if (strcmp (filename, home) == 0)
    {
      g_free (home);
      return g_strdup ("~");
    }

  tmp = home;
  home = g_strdup_printf ("%s/", tmp);
  g_free (tmp);

  if (g_str_has_prefix (filename, home))
    {
      gchar *res = g_strdup_printf ("~/%s", filename + strlen (home));
      g_free (home);
      return res;
    }

  g_free (home);
  return g_strdup (filename);
}
