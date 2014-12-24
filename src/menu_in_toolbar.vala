/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2010 Sébastien Wilmet
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

public class MenuToolAction : Gtk.Action
{
    public MenuToolAction (string name, string? label, string? tooltip, string? icon_name)
    {
        GLib.Object (name: name, label: label, tooltip: tooltip, icon_name: icon_name);
        unowned GtkActionClass ac = (GtkActionClass) get_class ();
        ac.toolbar_item_type = typeof (MenuToolButton);
    }
}
