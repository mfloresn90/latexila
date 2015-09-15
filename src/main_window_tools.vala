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
        { "Tools", null, N_("_Tools") },
        { "ToolsSpellCheckerDialog", null, N_("_Check Spelling…"), null,
            N_("Check the spelling of the current document"), on_spell_checker_dialog },
        { "ToolsSetSpellLanguage", null, N_("_Set Language…"), null,
            N_("Set the language used for the spell checking"), on_set_language }
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

    /* Sensitivity */

    public void update_sensitivity ()
    {
        bool sensitive = _main_window.active_tab != null;

        string[] action_names =
        {
            "ToolsSpellCheckerDialog",
            "ToolsSetSpellLanguage"
        };

        foreach (string action_name in action_names)
        {
            Gtk.Action action = _action_group.get_action (action_name);
            action.sensitive = sensitive;
        }
    }

    /* Gtk.Action callbacks */

    public void on_spell_checker_dialog (Gtk.Action action)
    {
        return_if_fail (_main_window.active_view != null);

        _main_window.active_view.launch_spell_checker_dialog ();
    }

    public void on_set_language (Gtk.Action action)
    {
        return_if_fail (_main_window.active_view != null);

        _main_window.active_view.set_spell_language ();
    }

    public void on_inline_spell_checker (Gtk.Action action)
    {
        bool activate = (action as ToggleAction).active;

        foreach (DocumentView view in _main_window.get_views ())
        {
            if (activate)
                view.activate_inline_spell_checker ();
            else
                view.deactivate_inline_spell_checker ();
        }
    }
}
