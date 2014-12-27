/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2010-2012 Sébastien Wilmet
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

using Gtk;

namespace Utils
{
    /*************************************************************************/
    // String utilities

    public string str_middle_truncate (string str, uint max_length)
    {
        if (str.length <= max_length)
            return str;

        uint half_length = (max_length - 4) / 2;
        int l = str.length;
        return str[0:half_length] + "..." + str[l-half_length:l];
    }

    public bool char_is_escaped (string text, long char_index)
    {
        return_val_if_fail (char_index < text.length, false);

        bool escaped = false;
        int index = (int) char_index;
        unichar cur_char;
        while (text.get_prev_char (ref index, out cur_char))
        {
            if (cur_char != '\\')
                break;

            escaped = ! escaped;
        }

        return escaped;
    }

    public unowned string? get_string_from_resource (string resource_path)
    {
        try
        {
            Bytes bytes = resources_lookup_data (resource_path, 0);
            return (string) bytes.get_data ();
        }
        catch (Error e)
        {
            warning ("Failed to load data from resource '%s': %s",
                resource_path, e.message);
            return null;
        }
    }


    /*************************************************************************/
    // URI, File or Path utilities

    public string? uri_get_dirname (string uri)
    {
        return_val_if_fail (uri != null, null);
        string dir = Path.get_dirname (uri);
        if (dir == ".")
            return null;
        return Latexila.utils_replace_home_dir_with_tilde (dir);
    }

    /* Returns a string suitable to be displayed in the UI indicating
     * the name of the directory where the file is located.
     * For remote files it may also contain the hostname etc.
     * For local files it tries to replace the home dir with ~.
     */
    public string? get_dirname_for_display (File location)
    {
        try
        {
            Mount mount = location.find_enclosing_mount (null);
            string mount_name = mount.get_name ();
            string? dirname =
                uri_get_dirname (location.get_path () ?? location.get_uri ());

            if (dirname == null || dirname == ".")
                return mount_name;
            return mount_name + " " + dirname;
        }

        // local files or uri without mounts
        catch (Error e)
        {
            return uri_get_dirname (location.get_path () ?? location.get_uri ());
        }
    }

    public void delete_file (File file)
    {
        if (! file.query_exists ())
            return;

        try
        {
            file.delete ();
        }
        catch (Error e)
        {
            warning ("Delete file '%s' failed: %s", file.get_parse_name (), e.message);
        }
    }

    public bool create_parent_directories (File file)
    {
        File parent = file.get_parent ();

        if (parent == null || parent.query_exists ())
            return true;

        try
        {
            parent.make_directory_with_parents ();
        }
        catch (Error e)
        {
            warning ("Failed to create directory parents for the file '%s': %s",
                file.get_parse_name (), e.message);
            return false;
        }

        return true;
    }

    public bool save_file (File file, string contents, bool make_backup = false)
    {
        if (! create_parent_directories (file))
            return false;

        try
        {
            file.replace_contents (contents.data, null, make_backup,
                FileCreateFlags.NONE, null);
        }
        catch (Error e)
        {
            warning ("Failed to save the file '%s': %s", file.get_parse_name (),
                e.message);
            return false;
        }

        return true;
    }

    // Returns null on error.
    public string? load_file (File file)
    {
        try
        {
            uint8[] chars;
            file.load_contents (null, out chars, null);
            return (string) (owned) chars;
        }
        catch (Error e)
        {
            warning ("Failed to load the file '%s': %s", file.get_parse_name (),
                e.message);
            return null;
        }
    }

    // origin can be equal to common_dir, but target must be different
    public string? get_relative_path (File origin, File target, File common_dir)
    {
        File? origin_dir;
        if (origin.equal (common_dir))
            origin_dir = origin;
        else
            origin_dir = origin.get_parent ();

        File? target_parent = target.get_parent ();

        return_val_if_fail (origin_dir != null, null);
        return_val_if_fail (target_parent != null, null);

        // The origin is in the same directory as the target.
        if (target_parent.equal (origin_dir))
            return target.get_basename ();

        // Get a list of parent directories. Stop at the common dir.
        List<File> target_dirs = new List<File> ();
        List<File> origin_dirs = new List<File> ();

        while (target_parent != null && ! target_parent.equal (common_dir))
        {
            target_dirs.prepend (target_parent);
            target_parent = target_parent.get_parent ();
        }

        while (origin_dir != null && ! origin_dir.equal (common_dir))
        {
            origin_dirs.prepend (origin_dir);
            origin_dir = origin_dir.get_parent ();
        }

        // Get number of common dirs
        uint dir_index = 0;
        while (dir_index < target_dirs.length () && dir_index < origin_dirs.length ())
        {
            File cur_target_dir = target_dirs.nth_data (dir_index);
            File cur_origin_dir = origin_dirs.nth_data (dir_index);
            if (! cur_target_dir.equal (cur_origin_dir))
                break;

            dir_index++;
        }

        uint nb_common_dirs = dir_index;

        /* Build the relative path */
        string relative_path = "";

        // go to the common dir
        uint nb_remaining_origin_dirs = origin_dirs.length () - nb_common_dirs;
        for (uint i = 0 ; i < nb_remaining_origin_dirs ; i++)
            relative_path += "../";

        // go to the target dir
        for (uint i = nb_common_dirs ; i < target_dirs.length () ; i++)
        {
            File cur_target_dir = target_dirs.nth_data (i);
            relative_path += cur_target_dir.get_basename () + "/";
        }

        // add the target basename
        relative_path += target.get_basename ();
        return relative_path;
    }


    /*************************************************************************/
    // UI stuff

    public ScrolledWindow add_scrollbar (Widget child)
    {
        ScrolledWindow sw = new ScrolledWindow (null, null);
        sw.add (child);
        return sw;
    }

    public bool tree_model_iter_prev (TreeModel model, ref TreeIter iter)
    {
        TreePath path = model.get_path (iter);
        if (path.prev ())
        {
            bool ret = model.get_iter (out iter, path);
            return ret;
        }
        return false;
    }

    // get indice of selected row in the treeview
    // returns -1 if no row is selected
    public int get_selected_row (TreeView view, out TreeIter iter = null)
    {
        TreeSelection select = view.get_selection ();
        if (select.get_selected (null, out iter))
        {
            TreeModel model = view.get_model ();
            TreePath path = model.get_path (iter);
            return path.get_indices ()[0];
        }
        return -1;
    }

    public Grid get_dialog_component (string title, Widget widget)
    {
        Grid grid = new Grid ();
        grid.orientation = Orientation.VERTICAL;
        grid.set_row_spacing (6);
        grid.border_width = 6;

        // title in bold, left aligned
        Label label = new Label (null);
        label.set_markup ("<b>" + title + "</b>");
        label.set_halign (Align.START);
        grid.add (label);

        // left margin for the widget
        widget.set_margin_left (12);
        grid.add (widget);

        return grid;
    }

    public unowned Gtk.Window? get_toplevel_window (Widget widget)
    {
        unowned Widget toplevel = widget.get_toplevel ();

        if (toplevel is Gtk.Window)
            return toplevel as Gtk.Window;

        return null;
    }


    /*************************************************************************/
    // Misc

    public void flush_queue ()
    {
        while (Gtk.events_pending ())
            Gtk.main_iteration ();
    }
}
