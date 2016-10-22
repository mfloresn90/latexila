/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2010-2011, 2016 Sébastien Wilmet
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

public class TabInfoBar : Gtef.InfoBar
{
    public TabInfoBar (string primary_msg, string secondary_msg, MessageType msg_type)
    {
        set_message_type (msg_type);

        Grid grid = new Grid ();
        grid.orientation = Orientation.VERTICAL;
        grid.set_row_spacing (10);

        Label primary_label = Gtef.InfoBar.create_label ();
        primary_label.set_markup ("<b>" + primary_msg + "</b>");
        grid.add (primary_label);

        Label secondary_label = Gtef.InfoBar.create_label ();
        secondary_label.set_markup ("<small>" + secondary_msg + "</small>");
        grid.add (secondary_label);

        Box content_area = get_content_area () as Box;
        content_area.pack_start (grid);

        show_all ();
    }

    public void add_ok_button ()
    {
        add_button (_("_OK"), ResponseType.OK);
        response.connect ((response_id) =>
        {
            if (response_id == ResponseType.OK)
                destroy ();
        });
    }
}
