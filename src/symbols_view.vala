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

// Lists of LaTeX symbols, displayed in the side panel.
// There are different categories, listed in a combo box.
// There is a special category that contains the most used symbols, with a clear button.
public class SymbolsView : Grid
{
    private unowned MainWindow _main_window;
    private ComboBox _combo_box;
    private IconView _symbol_view;
    private Button _clear_button;

    public SymbolsView (MainWindow main_window)
    {
        _main_window = main_window;

        create_combo_box ();
        create_icon_view ();
        create_clear_button ();

        // Packing widgets
        orientation = Orientation.VERTICAL;
        set_row_spacing (3);

        add (_combo_box);

        ScrolledWindow sw = Utils.add_scrollbar (_symbol_view);
        sw.set_shadow_type (ShadowType.IN);
        add (sw);

        add (_clear_button);

        show_all ();

        _combo_box.set_active (0);
    }

    private void create_combo_box ()
    {
        TreeModel categories_model = Symbols.get_default ().get_categories_model ();

        _combo_box = new ComboBox.with_model (categories_model);
        _combo_box.hexpand = true;
        _combo_box.margin_end = 3;

        CellRendererPixbuf pixbuf_renderer = new CellRendererPixbuf ();
        _combo_box.pack_start (pixbuf_renderer, false);
        _combo_box.set_attributes (pixbuf_renderer,
            "icon-name", SymbolsCategoryColumn.ICON, null);

        CellRendererText text_renderer = new CellRendererText ();
        text_renderer.ellipsize_set = true;
        text_renderer.ellipsize = Pango.EllipsizeMode.END;
        _combo_box.pack_start (text_renderer, true);
        _combo_box.set_attributes (text_renderer,
            "text", SymbolsCategoryColumn.NAME, null);

        _combo_box.changed.connect (() =>
        {
            TreeIter iter = {};

            if (! _combo_box.get_active_iter (out iter))
                return;

            ListStore store;
            SymbolsCategoryType type;

            categories_model.get (iter,
                SymbolsCategoryColumn.SYMBOLS_STORE, out store,
                SymbolsCategoryColumn.TYPE, out type
            );

            if (_symbol_view != null)
                _symbol_view.set_model (store);

            if (type == SymbolsCategoryType.MOST_USED)
                _clear_button.show ();
            else
                _clear_button.hide ();
        });
    }

    private void create_icon_view ()
    {
        /* show the symbols */
        _symbol_view = new IconView ();
        _symbol_view.set_pixbuf_column (SymbolColumn.PIXBUF);
        _symbol_view.set_tooltip_column (SymbolColumn.TOOLTIP);
        _symbol_view.set_selection_mode (SelectionMode.SINGLE);
        _symbol_view.spacing = 0;
        _symbol_view.row_spacing = 0;
        _symbol_view.column_spacing = 0;
        _symbol_view.expand = true;

        _symbol_view.selection_changed.connect (() =>
        {
            if (_main_window.active_tab == null)
            {
                _symbol_view.unselect_all ();
                return;
            }

            var selected_items = _symbol_view.get_selected_items ();

            // Unselect the symbol, so the user can insert several times the same symbol.
            _symbol_view.unselect_all ();

            TreePath path = selected_items.nth_data (0);
            TreeModel model = _symbol_view.get_model ();
            TreeIter iter = {};

            if (path != null && model.get_iter (out iter, path))
            {
                string latex_command;
                string id;

                model.get (iter,
                    SymbolColumn.COMMAND, out latex_command,
                    SymbolColumn.ID, out id
                );

                // insert the symbol in the current document
                _main_window.active_document.begin_user_action ();
                _main_window.active_document.insert_at_cursor (@"$latex_command ", -1);
                _main_window.active_document.end_user_action ();
                _main_window.active_view.grab_focus ();

                // insert to most used symbol
                MostUsedSymbols.get_default ().increment_symbol (id);
            }
        });
    }

    private void create_clear_button ()
    {
        _clear_button = new Button.with_mnemonic (_("_Clear"));
        _clear_button.margin_end = 3;
        _clear_button.tooltip_text = _("Clear most used symbols");

        _clear_button.clicked.connect (() =>
        {
            MostUsedSymbols.get_default ().clear ();
        });
    }
}
