/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2012, 2015 Sébastien Wilmet
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
            N_("Undo the last action") },

        { "EditRedo", "edit-redo", N_("_Redo"), "<Shift><Control>Z",
            N_("Redo the last undone action") },

        { "EditCut", "edit-cut", N_("Cu_t"), "<Control>X",
            N_("Cut the selection") },

        { "EditCopy", "edit-copy", N_("_Copy"), "<Control>C",
            N_("Copy the selection") },

        // No shortcut here because if the shortcut is null, Ctrl+V is used for _all_
        // the window. In this case Ctrl+V in the search text entry would be broken (the
        // text is pasted in the document instead of the entry).
        // Anyway if we press Ctrl+V when the cursor is in the document, no problem.
        { "EditPaste", "edit-paste", N_("_Paste"), "",
            N_("Paste the clipboard") },

        { "EditDelete", "edit-delete", N_("_Delete"), null,
            N_("Delete the selected text") },

        { "EditSelectAll", "edit-select-all", N_("Select _All"), "<Control>A",
            N_("Select the entire document") },

        { "EditIndent", "format-indent-more", N_("_Indent"), "Tab",
            N_("Indent the selected lines"), on_indent },

        { "EditUnindent", "format-indent-less", N_("_Unindent"), "<Shift>Tab",
            N_("Unindent the selected lines"), on_unindent },

        { "EditComment", null, N_("_Comment"), "<Control>M",
            N_("Comment the selected lines (add the character \"%\")"),
            on_comment },

        { "EditUncomment", null, N_("_Uncomment"), "<Shift><Control>M",
            N_("Uncomment the selected lines (remove the character \"%\")"),
            on_uncomment },

        { "EditCompletion", null, N_("_Completion"), "<Control>space",
            N_("Complete the LaTeX command"), on_completion },

        { "EditPreferences", "preferences-system", N_("_Preferences"), null,
            N_("Configure the application") }
    };

    private unowned MainWindow _main_window;
    private Gtk.ActionGroup _action_group;

    public MainWindowEdit (MainWindow main_window, UIManager ui_manager)
    {
        _main_window = main_window;

        _action_group = new Gtk.ActionGroup ("EditMenuActionGroup");
        _action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
        _action_group.add_actions (_action_entries, this);

        ui_manager.insert_action_group (_action_group, 0);

        LatexilaApp app = LatexilaApp.get_instance ();

        Amtk.utils_bind_g_action_to_gtk_action (main_window, "tepl-undo",
            _action_group, "EditUndo");
        Amtk.utils_bind_g_action_to_gtk_action (main_window, "tepl-redo",
            _action_group, "EditRedo");
        Amtk.utils_bind_g_action_to_gtk_action (main_window, "tepl-cut",
            _action_group, "EditCut");
        Amtk.utils_bind_g_action_to_gtk_action (main_window, "tepl-copy",
            _action_group, "EditCopy");
        Amtk.utils_bind_g_action_to_gtk_action (main_window, "tepl-paste",
            _action_group, "EditPaste");
        Amtk.utils_bind_g_action_to_gtk_action (main_window, "tepl-delete",
            _action_group, "EditDelete");
        Amtk.utils_bind_g_action_to_gtk_action (main_window, "tepl-select-all",
            _action_group, "EditSelectAll");
        Amtk.utils_bind_g_action_to_gtk_action (app, "preferences",
            _action_group, "EditPreferences");
    }

    /* Sensitivity */

    public void update_sensitivity ()
    {
        bool sensitive = _main_window.active_tab != null;

        string[] action_names =
        {
            "EditIndent",
            "EditUnindent",
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

    /* Gtk.Action callbacks */

    public void on_indent ()
    {
        DocumentTab? tab = _main_window.active_tab;
        return_if_fail (tab != null);

        TextIter start;
        TextIter end;
        tab.get_buffer ().get_selection_bounds (out start, out end);

        tab.view.indent_lines (start, end);
    }

    public void on_unindent ()
    {
        DocumentTab? tab = _main_window.active_tab;
        return_if_fail (tab != null);

        TextIter start;
        TextIter end;
        tab.get_buffer ().get_selection_bounds (out start, out end);

        tab.view.unindent_lines (start, end);
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
}
