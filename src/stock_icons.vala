/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2012 Sébastien Wilmet
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
 *
 * Author: Sébastien Wilmet
 */

// Add some icons to the stock icons, so it can be used e.g. in menus.

// See also:
// data/images/stock-icons/stock-icons.gresource.xml

public class StockIcons
{
    public StockIcons ()
    {
        register_new_stock_icons ();
    }

    private void register_new_stock_icons ()
    {
        string resource_path = "/org/gnome/latexila/stock-icons/";
        string[] icon_files;

        try
        {
            icon_files = resources_enumerate_children (resource_path, 0);
        }
        catch (Error e)
        {
            warning ("Failed to register new stock icons: %s", e.message);
            return;
        }

        Gtk.IconFactory icon_factory = new Gtk.IconFactory ();

        foreach (string icon_file in icon_files)
        {
            Gtk.IconSet icon_set = new Gtk.IconSet ();
            Gtk.IconSource icon_source = new Gtk.IconSource ();
            Gdk.Pixbuf pixbuf;

            try
            {
                pixbuf = new Gdk.Pixbuf.from_resource (resource_path + icon_file);
            }
            catch (Error e)
            {
                warning ("Failed to register stock icon: %s", e.message);
                continue;
            }

            icon_source.set_pixbuf (pixbuf);
            icon_set.add_source (icon_source);

            string icon_name = Latexila.utils_get_shortname (icon_file);
            icon_factory.add (icon_name, icon_set);
        }

        icon_factory.add_default ();
    }
}
