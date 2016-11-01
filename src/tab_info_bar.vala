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
        Box content_area = get_content_area () as Box;

        set_message_type (msg_type);

        // icon
        string icon_name;
        switch (msg_type)
        {
            case MessageType.ERROR:
                icon_name = "dialog-error";
                break;
            case MessageType.QUESTION:
                icon_name = "dialog-question";
                break;
            case MessageType.WARNING:
                icon_name = "dialog-warning";
                break;
            case MessageType.INFO:
            default:
                icon_name = "dialog-information";
                break;
        }

        Image image = new Image.from_icon_name (icon_name, IconSize.DIALOG);
        image.set_valign (Align.START);
        content_area.pack_start (image, false, false, 0);

        // text
        Grid grid = new Grid ();
        grid.orientation = Orientation.VERTICAL;
        grid.set_row_spacing (6);

        Label primary_label = Gtef.InfoBar.create_label ();
        primary_label.set_markup ("<b>" + primary_msg + "</b>");
        grid.add (primary_label);

        Label secondary_label = Gtef.InfoBar.create_label ();
        secondary_label.set_markup ("<small>" + secondary_msg + "</small>");
        grid.add (secondary_label);

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
