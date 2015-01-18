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

public class SidePanel : Grid
{
    private enum SidePanelColumn
    {
        PIXBUF,
        NAME,
        N_COLUMNS
    }

    private GLib.Settings _settings;
    private Gee.ArrayList<Grid?> _components;
    private ComboBox _combo_box;
    private ListStore _list_store;
    private int _current_component = -1;

    public SidePanel ()
    {
        _settings = new GLib.Settings ("org.gnome.latexila.preferences.ui");
        _components = new Gee.ArrayList<Grid?> ();

        margin_start = 6;
        margin_top = 3;
        column_spacing = 3;
        row_spacing = 3;

        init_combo_box ();
        Button close_button = get_close_button ();

        attach (_combo_box, 0, 0, 1, 1);
        attach (close_button, 1, 0, 1, 1);
        show_all ();
    }

    private void init_combo_box ()
    {
        _list_store = new ListStore (SidePanelColumn.N_COLUMNS,
            typeof (string), // pixbuf (icon-name)
            typeof (string)  // name
        );

        _combo_box = new ComboBox.with_model (_list_store);
        _combo_box.hexpand = true;

        CellRendererPixbuf pixbuf_renderer = new CellRendererPixbuf ();
        _combo_box.pack_start (pixbuf_renderer, false);
        _combo_box.set_attributes (pixbuf_renderer,
            "icon-name", SidePanelColumn.PIXBUF, null);

        CellRendererText text_renderer = new CellRendererText ();
        text_renderer.ellipsize_set = true;
        text_renderer.ellipsize = Pango.EllipsizeMode.END;
        _combo_box.pack_start (text_renderer, true);
        _combo_box.set_attributes (text_renderer, "text", SidePanelColumn.NAME, null);

        /* signals */
        _combo_box.changed.connect (show_active_component);
    }

    private Button get_close_button ()
    {
        Button close_button = new Button ();
        close_button.relief = ReliefStyle.NONE;
        close_button.focus_on_click = false;
        close_button.tooltip_text = _("Hide panel");
        close_button.add (new Image.from_icon_name ("window-close", IconSize.MENU));
        close_button.margin_end = 3;

        close_button.clicked.connect (() => this.hide ());

        return close_button;
    }

    public void add_component (string name, string icon_name, Grid component)
    {
        TreeIter iter;
        _list_store.append (out iter);
        _list_store.set (iter,
            SidePanelColumn.PIXBUF, icon_name,
            SidePanelColumn.NAME, name);

        _components.add (component);
        attach (component, 0, _components.size, 2, 1);
    }

    public void restore_state ()
    {
        foreach (Grid component in _components)
            component.hide ();

        int num = _settings.get_int ("side-panel-component");
        num = num.clamp (0, _components.size - 1);
        _combo_box.set_active (num);

        // Save which component is displayed. Since the component can be different
        // on each window, we make only a SET (not a GET).
        // The setting is bind only after getting the old value, otherwise the old value
        // is overwritten.
        _settings.bind ("side-panel-component", _combo_box, "active",
            SettingsBindFlags.SET);
    }

    private void show_active_component ()
    {
        if (0 <= _current_component && _current_component < _components.size)
            _components[_current_component].hide ();

        _current_component = _combo_box.active;
        _components[_current_component].show ();
    }
}
