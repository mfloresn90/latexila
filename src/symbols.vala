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

public enum SymbolsCategoryType
{
    NORMAL,
    MOST_USED
}

public enum SymbolsCategoryColumn
{
    TYPE,
    ICON, // icon-name, as a string
    NAME,
    SYMBOLS_STORE,
    N_COLUMNS
}

public enum SymbolColumn
{
    PIXBUF, // Gdk.Pixbuf
    COMMAND,
    TOOLTIP,
    ID, // used for the most used symbols
    N_COLUMNS
}

public class Symbols : GLib.Object
{
    private struct CategoryInfo
    {
        string name;
        string icon;
        string id;
    }

    private const CategoryInfo[] _normal_categories =
    {
        { N_("Greek"), "symbol_greek", "greek" },
        { N_("Arrows"), "symbol_arrows", "arrows" },
        { N_("Relations"), "symbol_relations", "relations" },
        { N_("Operators"), "symbol_operators", "operators" },
        { N_("Delimiters"), "symbol_delimiters", "delimiters" },
        { N_("Misc math"), "symbol_misc_math", "misc-math" },
        { N_("Misc text"), "symbol_misc_text", "misc-text" }
    };

    private static Symbols _instance = null;
    private ListStore _categories_store;

    // category id -> NormalSymbols
    private Gee.Map<string, NormalSymbols?> _normal_symbols_map;

    // singleton
    private Symbols ()
    {
        _categories_store = new ListStore (SymbolsCategoryColumn.N_COLUMNS,
            typeof (SymbolsCategoryType),
            typeof (string), // the icon
            typeof (string), // the name
            typeof (TreeModel)
        );

        _normal_symbols_map = new Gee.HashMap<string, NormalSymbols?> ();

        foreach (CategoryInfo info in _normal_categories)
            add_normal_category (info);
    }

    public static Symbols get_default ()
    {
        if (_instance == null)
        {
            _instance = new Symbols ();

            // This class must be instanciated before the construction of the
            // MostUsedSymbols class, because MostUsedSymbols calls
            // get_symbol_info().
            _instance.add_most_used_category ();
        }

        return _instance;
    }

    public TreeModel get_categories_model ()
    {
        return _categories_store as TreeModel;
    }

    public bool get_symbol_info (string id, out string command, out string tooltip)
    {
        command = null;
        tooltip = null;

        string[] id_components = id.split ("/");
        return_val_if_fail (id_components.length == 2, false);

        string category_id = id_components[0];
        string icon_file = id_components[1];

        return_val_if_fail (_normal_symbols_map.has_key (category_id), false);

        NormalSymbols normal_symbols = _normal_symbols_map[category_id];

        string package = null;
        bool ok = normal_symbols.get_symbol_info (icon_file, out command, out package);

        return_val_if_fail (ok, false);

        tooltip = get_tooltip (command, package);
        return true;
    }

    private void add_normal_category (CategoryInfo info)
    {
        ListStore store = new NormalSymbols (info.id);

        _normal_symbols_map[info.id] = store as NormalSymbols;

        TreeIter iter;
        _categories_store.append (out iter);
        _categories_store.set (iter,
            SymbolsCategoryColumn.TYPE, SymbolsCategoryType.NORMAL,
            SymbolsCategoryColumn.ICON, info.icon,
            SymbolsCategoryColumn.NAME, _(info.name),
            SymbolsCategoryColumn.SYMBOLS_STORE, store
        );
    }

    private void add_most_used_category ()
    {
        TreeModel model = MostUsedSymbols.get_default ().get_model ();

        TreeIter iter;
        _categories_store.append (out iter);
        _categories_store.set (iter,
            SymbolsCategoryColumn.TYPE, SymbolsCategoryType.MOST_USED,
            SymbolsCategoryColumn.ICON, "help-about",
            SymbolsCategoryColumn.NAME, _("Most Used"),
            SymbolsCategoryColumn.SYMBOLS_STORE, model
        );
    }

    public static string get_tooltip (string latex_command, string? package_required)
    {
        // Some characters ('<' for example) generate errors for the tooltip,
        // so the text must be escaped.
        string tooltip = Markup.escape_text (latex_command);

        if (package_required != null)
            tooltip += " (package %s)".printf (package_required);

        return tooltip;
    }

    public static Gdk.Pixbuf? get_pixbuf (string symbol_id)
    {
        try
        {
            return new Gdk.Pixbuf.from_resource (
                "/org/gnome/latexila/symbols/" + symbol_id);
        }
        catch (Error e)
        {
            warning ("Impossible to load the symbol '%s': %s", symbol_id, e.message);
            return null;
        }
    }
}

private class NormalSymbols : ListStore
{
    private struct SymbolInfo
    {
        string latex_command;
        string? package_required;
    }

    private string _category_id;
    private string _resource_path;

    // icon file -> symbol info
    private Gee.Map<string, SymbolInfo?> _data;

    public NormalSymbols (string category_id)
    {
        _category_id = category_id;
        _resource_path = @"/org/gnome/latexila/symbols/$category_id/";

        _data = new Gee.HashMap<string, SymbolInfo?> ();

        Type[] column_types = {
            typeof (Gdk.Pixbuf),
            typeof (string), // command
            typeof (string), // tooltip
            typeof (string)  // id
        };

        set_column_types (column_types);

        load_symbols ();
    }

    public bool get_symbol_info (string icon_file, out string command, out string package)
    {
        command = null;
        package = null;

        if (! _data.has_key (icon_file))
            return false;

        SymbolInfo info = _data[icon_file];
        command = info.latex_command;
        package = info.package_required;

        return true;
    }

    private void load_symbols ()
    {
        unowned string? contents =
            Utils.get_string_from_resource (_resource_path + "data.xml");

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
            warning ("Impossible to load the symbols: %s", e.message);
        }
    }

    private void add_symbol (string icon_file, SymbolInfo symbol)
    {
        _data[icon_file] = symbol;

        string tooltip = Symbols.get_tooltip (symbol.latex_command,
            symbol.package_required);

        string id = "%s/%s".printf (_category_id, icon_file);

        Gdk.Pixbuf? pixbuf = Symbols.get_pixbuf (id);
        if (pixbuf == null)
            return;

        TreeIter iter;
        append (out iter);
        set (iter,
            SymbolColumn.PIXBUF, pixbuf,
            SymbolColumn.COMMAND, symbol.latex_command,
            SymbolColumn.TOOLTIP, tooltip,
            SymbolColumn.ID, id
        );
    }

    private void parser_start (MarkupParseContext context, string name,
        string[] attr_names, string[] attr_values) throws MarkupError
    {
        switch (name)
        {
            case "symbols":
                return;

            case "symbol":
                SymbolInfo symbol = SymbolInfo ();
                symbol.package_required = null;
                string icon_file = null;

                for (int i = 0 ; i < attr_names.length ; i++)
                {
                    switch (attr_names[i])
                    {
                        case "file":
                            icon_file = attr_values[i];
                            break;

                        case "command":
                            symbol.latex_command = attr_values[i];
                            break;

                        case "package":
                            symbol.package_required = attr_values[i];
                            break;

                        default:
                            throw new MarkupError.UNKNOWN_ATTRIBUTE (
                                "unknown attribute \"" + attr_names[i] + "\"");
                    }
                }

                add_symbol (icon_file, symbol);
                break;

            default:
                throw new MarkupError.UNKNOWN_ELEMENT (
                    "unknown element \"" + name + "\"");
        }
    }
}
