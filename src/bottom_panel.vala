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

using Gtk;

public class BottomPanel : Grid
{
    public BottomPanel (Latexila.BuildView build_view, Toolbar toolbar)
    {
        set_orientation (Orientation.HORIZONTAL);

        ScrolledWindow scrolled_window = Utils.add_scrollbar (build_view);

        scrolled_window.expand = true;
        scrolled_window.show_all ();
        scrolled_window.set_shadow_type (ShadowType.IN);
        add (scrolled_window);

        Grid grid = new Grid ();
        grid.orientation = Orientation.VERTICAL;
        grid.add (get_close_button ());

        toolbar.vexpand = true;
        grid.add (toolbar);

        grid.show_all ();
        add (grid);
    }

    private Button get_close_button ()
    {
        Button close_button = new Button ();
        close_button.relief = ReliefStyle.NONE;
        close_button.focus_on_click = false;
        close_button.tooltip_text = _("Hide panel");
        close_button.add (new Image.from_stock (Stock.CLOSE, IconSize.MENU));
        close_button.clicked.connect (() => this.hide ());

        return close_button;
    }
}
