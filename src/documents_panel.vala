/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2010-2011 Sébastien Wilmet
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

public class DocumentsPanel : Notebook
{
    public DocumentTab active_tab { get; private set; }
    public signal void right_click (Gdk.EventButton event);

    private unowned MainWindow main_window;

    public DocumentsPanel (MainWindow main_window)
    {
        this.main_window = main_window;
        this.scrollable = true;
        switch_page.connect ((page, page_num) =>
        {
            active_tab = (DocumentTab) get_nth_page ((int) page_num);
        });
    }

    public void add_tab (DocumentTab tab, int position, bool jump_to)
    {
        EventBox event_box = new EventBox ();
        event_box.set_visible_window (false);

        event_box.add (tab.label);
        event_box.button_press_event.connect ((event) =>
        {
            // right click
            if (event.button == 3)
            {
                set_current_page (page_num (tab));

                // show popup menu
                right_click (event);
            }

            return false;
        });

        int page_pos = this.insert_page (tab, event_box, position);
        this.set_tab_reorderable (tab, true);
        if (jump_to)
            this.set_current_page (page_pos);
    }

    public void remove_tab (DocumentTab tab)
    {
        // automatic clean-up build files
        GLib.Settings settings =
            new GLib.Settings ("org.gnome.latexila.preferences.latex");

        if (settings.get_boolean ("no-confirm-clean")
            && settings.get_boolean ("automatic-clean"))
        {
            CleanBuildFiles build_files = new CleanBuildFiles (main_window, tab.document);
            build_files.clean ();
        }

        int pos = page_num (tab);
        remove_page (pos);
    }

    public void remove_all_tabs ()
    {
        while (true)
        {
            int n = get_current_page ();
            if (n == -1)
                break;
            DocumentTab tab = (DocumentTab) get_nth_page (n);
            remove_tab (tab);
        }
    }
}
