/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2015 Sébastien Wilmet
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

// The Tools menu of a MainWindow

public class MainWindowTools
{
    private const Gtk.ActionEntry[] _action_entries =
    {
        { "Tools", null, N_("_Tools") }
    };

    private const ToggleActionEntry[] _toggle_action_entries =
    {
        { "ToolsInlineSpellChecker", "tools-check-spelling", N_("_Highlight Misspelled Words"), null,
            N_("Highlight misspelled words in the document"), on_inline_spell_checker }
    };

    private unowned MainWindow _main_window;
    private Gtk.ActionGroup _action_group;

    public MainWindowTools (MainWindow main_window, UIManager ui_manager)
    {
        _main_window = main_window;

        _action_group = new Gtk.ActionGroup ("ToolsMenuActionGroup");
        _action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
        _action_group.add_actions (_action_entries, this);
        _action_group.add_toggle_actions (_toggle_action_entries, this);

        ui_manager.insert_action_group (_action_group, 0);

        /* Bind spell checking setting */

        ToggleAction spell_checking_action =
            _action_group.get_action ("ToolsInlineSpellChecker") as ToggleAction;

        GLib.Settings editor_settings =
            new GLib.Settings ("org.gnome.latexila.preferences.editor");

        editor_settings.bind ("spell-checking", spell_checking_action, "active",
            SettingsBindFlags.DEFAULT);
    }

    /* Gtk.Action callbacks */

    public void on_inline_spell_checker (Gtk.Action action)
    {
        bool activate = (action as ToggleAction).active;

        foreach (DocumentView view in _main_window.get_views ())
        {
            if (activate)
                view.activate_spell_checking ();
            else
                view.disable_spell_checking ();
        }
    }
}
