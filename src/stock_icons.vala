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

namespace StockIcons
{
    // It seems that GtkActionEntry doesn't support custom icons from the icon theme
    // (added with latexila_utils_register_icons()).
    // So add the latexila-specific icons to the stock-ids.
    public void register_stock_icons ()
    {
        add_theme_icon_to_stock ("accent0");
        add_theme_icon_to_stock ("accent1");
        add_theme_icon_to_stock ("accent2");
        add_theme_icon_to_stock ("accent3");
        add_theme_icon_to_stock ("accent4");
        add_theme_icon_to_stock ("accent5");
        add_theme_icon_to_stock ("accent6");
        add_theme_icon_to_stock ("accent7");
        add_theme_icon_to_stock ("accent8");
        add_theme_icon_to_stock ("accent9");
        add_theme_icon_to_stock ("accent10");
        add_theme_icon_to_stock ("accent11");
        add_theme_icon_to_stock ("accent12");
        add_theme_icon_to_stock ("accent13");
        add_theme_icon_to_stock ("accent14");
        add_theme_icon_to_stock ("accent15");
        add_theme_icon_to_stock ("badbox");
        add_theme_icon_to_stock ("blackboard");
        add_theme_icon_to_stock ("bold");
        add_theme_icon_to_stock ("character-size");
        add_theme_icon_to_stock ("compile_dvi");
        add_theme_icon_to_stock ("compile_pdf");
        add_theme_icon_to_stock ("compile_ps");
        add_theme_icon_to_stock ("completion_cmd");
        add_theme_icon_to_stock ("delimiters-left");
        add_theme_icon_to_stock ("delimiters-right");
        add_theme_icon_to_stock ("gray-square");
        add_theme_icon_to_stock ("italic");
        add_theme_icon_to_stock ("list-description-size16");
        add_theme_icon_to_stock ("list-description-size24");
        add_theme_icon_to_stock ("list-enumerate-size16");
        add_theme_icon_to_stock ("list-enumerate-size24");
        add_theme_icon_to_stock ("list-item-size16");
        add_theme_icon_to_stock ("list-itemize-size16");
        add_theme_icon_to_stock ("list-itemize-size24");
        add_theme_icon_to_stock ("math-frac");
        add_theme_icon_to_stock ("math-nth-root");
        add_theme_icon_to_stock ("math-square-root");
        add_theme_icon_to_stock ("math-subscript");
        add_theme_icon_to_stock ("math-superscript");
        add_theme_icon_to_stock ("math");
        add_theme_icon_to_stock ("mathaccent0");
        add_theme_icon_to_stock ("mathaccent1");
        add_theme_icon_to_stock ("mathaccent2");
        add_theme_icon_to_stock ("mathaccent3");
        add_theme_icon_to_stock ("mathaccent4");
        add_theme_icon_to_stock ("mathaccent5");
        add_theme_icon_to_stock ("mathaccent6");
        add_theme_icon_to_stock ("mathaccent7");
        add_theme_icon_to_stock ("mathaccent8");
        add_theme_icon_to_stock ("mathaccent9");
        add_theme_icon_to_stock ("mathaccent10");
        add_theme_icon_to_stock ("mathcal");
        add_theme_icon_to_stock ("mathfrak");
        add_theme_icon_to_stock ("references");
        add_theme_icon_to_stock ("roman");
        add_theme_icon_to_stock ("sans_serif");
        add_theme_icon_to_stock ("sectioning-size16");
        add_theme_icon_to_stock ("sectioning-size24");
        add_theme_icon_to_stock ("slanted");
        add_theme_icon_to_stock ("small_caps");
        add_theme_icon_to_stock ("symbol_arrows");
        add_theme_icon_to_stock ("symbol_delimiters");
        add_theme_icon_to_stock ("symbol_greek");
        add_theme_icon_to_stock ("symbol_misc_math");
        add_theme_icon_to_stock ("symbol_misc_text");
        add_theme_icon_to_stock ("symbol_operators");
        add_theme_icon_to_stock ("symbol_relations");
        add_theme_icon_to_stock ("table-size16");
        add_theme_icon_to_stock ("table-size24");
        add_theme_icon_to_stock ("tree_chapter");
        add_theme_icon_to_stock ("tree_label");
        add_theme_icon_to_stock ("tree_paragraph");
        add_theme_icon_to_stock ("tree_part");
        add_theme_icon_to_stock ("tree_section");
        add_theme_icon_to_stock ("tree_subsection");
        add_theme_icon_to_stock ("tree_subsubsection");
        add_theme_icon_to_stock ("tree_todo");
        add_theme_icon_to_stock ("typewriter");
        add_theme_icon_to_stock ("underline");
        add_theme_icon_to_stock ("view_dvi");
        add_theme_icon_to_stock ("view_log");
        add_theme_icon_to_stock ("view_pdf");
        add_theme_icon_to_stock ("view_ps");
    }

    private void add_theme_icon_to_stock (string icon_name)
    {
        Gtk.IconSource icon_source = new Gtk.IconSource ();
        icon_source.set_icon_name (icon_name);

        Gtk.IconSet icon_set = new Gtk.IconSet ();
        icon_set.add_source (icon_source);

        Gtk.IconFactory icon_factory = new Gtk.IconFactory ();
        icon_factory.add (icon_name, icon_set);
        icon_factory.add_default ();
    }
}
