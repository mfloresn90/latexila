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
            N_("Set the language used for the spell checking for the current document"),
            on_set_language }
    };

    private const ToggleActionEntry[] _toggle_action_entries =
    {
        { "ToolsInlineSpellChecker", "tools-check-spelling", N_("_Highlight Misspelled Words"), null,
            N_("Highlight misspelled words in the current document"), on_inline_spell_checker }
    };

    private unowned MainWindow _main_window;
    private Gtk.ActionGroup _action_group;
    private GLib.Settings _editor_settings;

    public MainWindowTools (MainWindow main_window, UIManager ui_manager)
    {
        _main_window = main_window;

        _action_group = new Gtk.ActionGroup ("ToolsMenuActionGroup");
        _action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
        _action_group.add_actions (_action_entries, this);
        _action_group.add_toggle_actions (_toggle_action_entries, this);

        ui_manager.insert_action_group (_action_group, 0);

        update_inline_spell_checker_action_state ();
        _main_window.notify["active-tab"].connect (() =>
        {
            update_inline_spell_checker_action_state ();
        });

        _editor_settings = new GLib.Settings ("org.gnome.latexila.preferences.editor");

        _editor_settings.changed["highlight-misspelled-words"].connect (() =>
        {
            if (_main_window.active_view != null)
                _main_window.active_view.setup_inline_spell_checker ();

            update_inline_spell_checker_action_state ();
        });
    }

    private void update_inline_spell_checker_action_state ()
    {
        if (_main_window.active_tab == null)
            return;

        ToggleAction spell_checking_action =
            _action_group.get_action ("ToolsInlineSpellChecker") as ToggleAction;

        Gspell.InlineCheckerText inline_checker =
            Gspell.text_view_get_inline_checker (_main_window.active_view as TextView);
        spell_checking_action.active = inline_checker.enabled;
    }

    /* Sensitivity */

    public void update_sensitivity ()
    {
        bool sensitive = _main_window.active_tab != null;

        string[] action_names =
        {
            "ToolsSpellCheckerDialog",
            "ToolsSetSpellLanguage",
            "ToolsInlineSpellChecker"
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
        DocumentView? view = _main_window.active_view;
        return_if_fail (view != null);

        view.launch_spell_checker_dialog ();

        // If the spell checker is used, save the language since it's probably
        // correct. If it isn't correct, the language will be changed and the
        // metadata will also be saved.
        view.set_spell_language_metadata ();
    }

    public void on_set_language (Gtk.Action action)
    {
        DocumentView? view = _main_window.active_view;
        return_if_fail (view != null);

        view.launch_spell_language_chooser_dialog ();
        view.set_spell_language_metadata ();
    }

    public void on_inline_spell_checker (Gtk.Action action)
    {
        DocumentView? view = _main_window.active_view;
        return_if_fail (view != null);

        bool activate = (action as ToggleAction).active;

        Gspell.InlineCheckerText inline_checker =
            Gspell.text_view_get_inline_checker (view as TextView);

        // Save metadata only if property changes, because this function is
        // also called when update_inline_spell_checker_action_state() is
        // called.
        if (inline_checker.enabled != activate)
        {
            inline_checker.enabled = activate;

            update_inline_spell_checker_action_state ();

            view.set_inline_spell_metadata ();
            view.set_spell_language_metadata ();
        }
    }
}
