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
 *
 * Author: Sébastien Wilmet
 */

using Gtk;

public class MostUsedSymbols : GLib.Object
{
    private static MostUsedSymbols _instance = null;
    private bool _modified = false;
    private Gtk.ListStore _store;

    // The column containing the number of times a symbol has been used.
    private static const int SYMBOL_COLUMN_NUM = SymbolColumn.N_COLUMNS;

    private MostUsedSymbols ()
    {
        init_models ();
        load_data ();
    }

    // singleton
    public static MostUsedSymbols get_default ()
    {
        if (_instance == null)
            _instance = new MostUsedSymbols ();

        return _instance;
    }

    private void init_models ()
    {
        _store = new Gtk.ListStore (SymbolColumn.N_COLUMNS + 1,
            typeof (Gdk.Pixbuf),
            typeof (string), // command
            typeof (string), // tooltip
            typeof (string), // id
            typeof (int)     // number of times used
        );

        _store.set_sort_column_id (SYMBOL_COLUMN_NUM, SortType.DESCENDING);
    }

    public TreeModel get_model ()
    {
        return _store as TreeModel;
    }

    public void clear ()
    {
        _store.clear ();
    }

    public void increment_symbol (string id)
    {
        TreeIter iter;

        if (! get_iter_at_symbol_id (id, out iter))
            add_symbol (id, 1);
        else
        {
            int num;
            TreeModel model = _store as TreeModel;
            model.get (iter, SYMBOL_COLUMN_NUM, out num);

            _store.set (iter, SYMBOL_COLUMN_NUM, num + 1);
        }

        _modified = true;
    }

    private bool get_iter_at_symbol_id (string id, out TreeIter iter)
    {
        if (! _store.get_iter_first (out iter))
            return false;

        do
        {
            string cur_id;
            TreeModel model = _store as TreeModel;
            model.get (iter, SymbolColumn.ID, out cur_id);

            if (cur_id == id)
                return true;
        }
        while (_store.iter_next (ref iter));

        return false;
    }

    private void add_symbol (string id, int nb_times_used)
    {
        Gdk.Pixbuf? pixbuf = Symbols.get_pixbuf (id);
        if (pixbuf == null)
            return;

        string command;
        string tooltip;
        if (! Symbols.get_default ().get_symbol_info (id, out command, out tooltip))
            return;

        TreeIter iter;
        _store.append (out iter);
        _store.set (iter,
            SymbolColumn.PIXBUF, pixbuf,
            SymbolColumn.COMMAND, command,
            SymbolColumn.TOOLTIP, tooltip,
            SymbolColumn.ID, id,
            SYMBOL_COLUMN_NUM, nb_times_used
        );
    }

    private File get_xml_file ()
    {
        string path = Path.build_filename (Environment.get_user_data_dir (),
            "latexila", "most_used_symbols.xml");

        return File.new_for_path (path);
    }

    private void load_data ()
    {
        File file = get_xml_file ();
        if (! file.query_exists ())
            return;

        string? contents = Utils.load_file (file);
        if (contents == null)
            return;

        try
        {
            MarkupParser parser = { parser_start, null, null, null, null };
            MarkupParseContext context = new MarkupParseContext (parser, 0, this, null);
            context.parse (contents, -1);
        }
        catch (GLib.Error e)
        {
            warning ("Impossible to load the most used symbols: %s", e.message);
        }
    }

    private void parser_start (MarkupParseContext context, string name,
        string[] attr_names, string[] attr_values) throws MarkupError
    {
        switch (name)
        {
            case "symbols":
                return;

            case "symbol":
                string id = null;
                int num = 0;

                for (int i = 0 ; i < attr_names.length ; i++)
                {
                    switch (attr_names[i])
                    {
                        case "id":
                            id = attr_values[i];
                            break;

                        case "num":
                            num = int.parse (attr_values[i]);
                            break;

                        case "command":
                        case "package":
                            // Used in the past but no longer required.
                            break;

                        default:
                            throw new MarkupError.UNKNOWN_ATTRIBUTE (
                                "unknown attribute \"" + attr_names[i] + "\"");
                    }
                }

                add_symbol (id, num);
                break;

            default:
                throw new MarkupError.UNKNOWN_ELEMENT (
                    "unknown element \"" + name + "\"");
        }
    }

    public void save ()
    {
        if (! _modified)
            return;

        _modified = false;

        File file = get_xml_file ();

        TreeIter iter;
        bool is_empty = ! _store.get_iter_first (out iter);

        if (is_empty)
        {
            Utils.delete_file (file);
            return;
        }

        string content = "<symbols>\n";

        do
        {
            string id;
            int num;

            TreeModel model = _store as TreeModel;
            model.get (iter,
                SymbolColumn.ID, out id,
                SYMBOL_COLUMN_NUM, out num
            );

            content += "  <symbol id=\"%s\" num=\"%d\" />\n".printf (id, num);
        }
        while (_store.iter_next (ref iter));

        content += "</symbols>\n";

        Utils.save_file (file, content);
    }
}
