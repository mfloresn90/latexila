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

// The Edit menu of a MainWindow

public class MainWindowEdit
{
    private const Gtk.ActionEntry[] _action_entries =
    {
        { "Edit", null, N_("_Edit") },

        { "EditUndo", "edit-undo", N_("_Undo"), "<Control>Z",
            N_("Undo the last action"), on_undo },

        { "EditRedo", "edit-redo", N_("_Redo"), "<Shift><Control>Z",
            N_("Redo the last undone action"), on_redo },

        { "EditCut", "edit-cut", N_("Cu_t"), "<Control>X",
            N_("Cut the selection"), on_cut },

        { "EditCopy", "edit-copy", N_("_Copy"), "<Control>C",
            N_("Copy the selection"), on_copy },

        // No shortcut here because if the shortcut is null, Ctrl+V is used for _all_
        // the window. In this case Ctrl+V in the search text entry would be broken (the
        // text is pasted in the document instead of the entry).
        // Anyway if we press Ctrl+V when the cursor is in the document, no problem.
        { "EditPaste", "edit-paste", N_("_Paste"), "",
            N_("Paste the clipboard"), on_paste },

        { "EditDelete", "edit-delete", N_("_Delete"), null,
            N_("Delete the selected text"), on_delete },

        { "EditSelectAll", "edit-select-all", N_("Select _All"), "<Control>A",
            N_("Select the entire document"), on_select_all },

        { "EditComment", null, N_("_Comment"), "<Control>M",
            N_("Comment the selected lines (add the character \"%\")"),
            on_comment },

        { "EditUncomment", null, N_("_Uncomment"), "<Shift><Control>M",
            N_("Uncomment the selected lines (remove the character \"%\")"),
            on_uncomment },

        { "EditCompletion", null, N_("_Completion"), "<Control>space",
            N_("Complete the LaTeX command"), on_completion },

        { "EditPreferences", "preferences-system", N_("_Preferences"), null,
            N_("Configure the application"), on_open_preferences }
    };

    private const ToggleActionEntry[] _toggle_action_entries =
    {
        { "EditSpellChecking", "tools-check-spelling", N_("_Spell Check"), null,
            N_("Activate or disable the spell checking"), on_spell_checking }
    };

    private unowned MainWindow _main_window;
    private Gtk.ActionGroup _action_group;

    public MainWindowEdit (MainWindow main_window, UIManager ui_manager)
    {
        _main_window = main_window;

        _action_group = new Gtk.ActionGroup ("EditMenuActionGroup");
        _action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
        _action_group.add_actions (_action_entries, this);
        _action_group.add_toggle_actions (_toggle_action_entries, this);

        ui_manager.insert_action_group (_action_group, 0);

        /* Bind spell checking setting */

        ToggleAction spell_checking_action =
            _action_group.get_action ("EditSpellChecking") as ToggleAction;

        GLib.Settings editor_settings =
            new GLib.Settings ("org.gnome.latexila.preferences.editor");

        editor_settings.bind ("spell-checking", spell_checking_action, "active",
            SettingsBindFlags.DEFAULT);
    }

    /* Sensitivity */

    public void update_sensitivity ()
    {
        bool sensitive = _main_window.active_tab != null;

        set_edit_actions_sensitivity (sensitive);

        if (sensitive)
        {
            set_has_selection_sensitivity ();
            set_undo_sensitivity ();
            set_redo_sensitivity ();
        }
    }

    private void set_edit_actions_sensitivity (bool sensitive)
    {
        string[] action_names =
        {
            "EditUndo",
            "EditRedo",
            "EditCut",
            "EditCopy",
            "EditPaste",
            "EditDelete",
            "EditSelectAll",
            "EditComment",
            "EditUncomment",
            "EditCompletion"
        };

        foreach (string action_name in action_names)
        {
            Gtk.Action action = _action_group.get_action (action_name);
            action.sensitive = sensitive;
        }
    }

    private void set_has_selection_sensitivity ()
    {
        bool has_selection = false;

        if (_main_window.active_tab != null)
            has_selection = _main_window.active_document.has_selection;

        // Actions that must be insensitive if there is no selection.
        string[] action_names =
        {
            "EditCut",
            "EditCopy",
            "EditDelete"
        };

        foreach (string action_name in action_names)
        {
            Gtk.Action action = _action_group.get_action (action_name);
            action.sensitive = has_selection;
        }
    }

    private void set_undo_sensitivity ()
    {
        bool can_undo = false;

        if (_main_window.active_tab != null)
            can_undo = _main_window.active_document.can_undo;

        Gtk.Action action = _action_group.get_action ("EditUndo");
        action.sensitive = can_undo;
    }

    private void set_redo_sensitivity ()
    {
        bool can_redo = false;

        if (_main_window.active_tab != null)
            can_redo = _main_window.active_document.can_redo;

        Gtk.Action action = _action_group.get_action ("EditRedo");
        action.sensitive = can_redo;
    }

    /* Gtk.Action callbacks */

    public void on_undo ()
    {
        return_if_fail (_main_window.active_tab != null);

        if (_main_window.active_document.can_undo)
        {
            _main_window.active_document.undo ();
            _main_window.active_view.scroll_to_cursor ();
            _main_window.active_view.grab_focus ();
        }
    }

    public void on_redo ()
    {
        return_if_fail (_main_window.active_tab != null);

        if (_main_window.active_document.can_redo)
        {
            _main_window.active_document.redo ();
            _main_window.active_view.scroll_to_cursor ();
            _main_window.active_view.grab_focus ();
        }
    }

    public void on_cut ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.active_view.cut_selection ();
    }

    public void on_copy ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.active_view.copy_selection ();
    }

    public void on_paste ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.active_view.my_paste_clipboard ();
    }

    public void on_delete ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.active_view.delete_selection ();
    }

    public void on_select_all ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.active_view.my_select_all ();
    }

    public void on_comment ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.active_document.comment_selected_lines ();
    }

    public void on_uncomment ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.active_document.uncomment_selected_lines ();
    }

    public void on_completion ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.active_view.show_completion ();
    }

    public void on_spell_checking (Gtk.Action action)
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

    public void on_open_preferences ()
    {
        PreferencesDialog.show_me (_main_window);
    }
}
