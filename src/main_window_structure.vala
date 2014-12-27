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

// MainWindow stuff for the structure (the list of chapters, sections, ... of a document).

public class MainWindowStructure
{
    private const Gtk.ActionEntry[] _action_entries =
    {
        { "Structure", null, N_("S_tructure") },

        { "StructureCut", "edit-cut", N_("Cu_t"), "",
            N_("Cut the selected structure item"), on_cut },

        { "StructureCopy", "edit-copy", N_("_Copy"), "",
            N_("Copy the selected structure item"), on_copy },

        { "StructureDelete", "edit-delete", N_("_Delete"), "",
            N_("Delete the selected structure item"), on_delete },

        { "StructureSelect", "edit-select-all", N_("_Select"), "",
            N_("Select the contents of the selected structure item"), on_select },

        { "StructureComment", null, N_("_Comment"), null,
            N_("Comment the selected structure item"), on_comment },

        { "StructureShiftLeft", "go-previous", N_("Shift _Left"), "",
            N_("Shift the selected structure item to the left (e.g. section → chapter)"),
            on_shift_left },

        { "StructureShiftRight", "go-next", N_("Shift _Right"), "",
            N_("Shift the selected structure item to the right (e.g. chapter → section)"),
            on_shift_right },

        { "StructureOpenFile", "document-open", N_("_Open File"), "",
            N_("Open the file referenced by the selected structure item"),
            on_open_file }
    };

    private UIManager _ui_manager;
    private Structure _structure;

    public MainWindowStructure (UIManager ui_manager)
    {
        _ui_manager = ui_manager;

        Gtk.ActionGroup action_group = new Gtk.ActionGroup ("StructureActionGroup");
        action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
        action_group.add_actions (_action_entries, this);

        ui_manager.insert_action_group (action_group, 0);
    }

    public void set_structure (Structure structure)
    {
        _structure = structure;

        structure.show_popup_menu.connect (show_popup_menu);
        structure.hide.connect (set_menu_insensitive);
        structure.no_items_selected.connect (set_menu_insensitive);
        structure.item_selected.connect (set_actions_sensitivity);
    }

    public void save_state ()
    {
        return_if_fail (_structure != null);
        _structure.save_state ();
    }

    public void refresh ()
    {
        return_if_fail (_structure != null);
        _structure.refresh ();
    }

    private void show_popup_menu (Gdk.EventButton? event)
    {
        Gtk.Menu popup_menu = _ui_manager.get_widget ("/StructurePopup") as Gtk.Menu;

        if (event != null)
            popup_menu.popup (null, null, null, event.button, event.time);
        else
            popup_menu.popup (null, null, null, 0, get_current_event_time ());
    }

    private void set_menu_insensitive ()
    {
        return_if_fail (_ui_manager != null);

        Gtk.Action menu = _ui_manager.get_action ("/MainMenu/Structure");
        menu.sensitive = false;
    }

    private void set_actions_sensitivity (StructType type)
    {
        Gtk.Action menu = _ui_manager.get_action ("/MainMenu/Structure");
        menu.sensitive = true;

        Gtk.Action shift_left =
            _ui_manager.get_action ("/StructurePopup/StructureShiftLeft");

        shift_left.sensitive = StructType.PART < type && type <= StructType.SUBPARAGRAPH;

        Gtk.Action shift_right =
            _ui_manager.get_action ("/StructurePopup/StructureShiftRight");

        shift_right.sensitive = StructType.PART <= type && type < StructType.SUBPARAGRAPH;

        Gtk.Action open_file =
            _ui_manager.get_action ("/StructurePopup/StructureOpenFile");

        open_file.sensitive = type == StructType.INCLUDE || type == StructType.IMAGE;
    }

    /* Gtk.Action callbacks */

    public void on_cut ()
    {
        return_if_fail (_structure != null);
        _structure.do_action (StructAction.CUT);
    }

    public void on_copy ()
    {
        return_if_fail (_structure != null);
        _structure.do_action (StructAction.COPY);
    }

    public void on_delete ()
    {
        return_if_fail (_structure != null);
        _structure.do_action (StructAction.DELETE);
    }

    public void on_select ()
    {
        return_if_fail (_structure != null);
        _structure.do_action (StructAction.SELECT);
    }

    public void on_comment ()
    {
        return_if_fail (_structure != null);
        _structure.do_action (StructAction.COMMENT);
    }

    public void on_shift_left ()
    {
        return_if_fail (_structure != null);
        _structure.do_action (StructAction.SHIFT_LEFT);
    }

    public void on_shift_right ()
    {
        return_if_fail (_structure != null);
        _structure.do_action (StructAction.SHIFT_RIGHT);
    }

    public void on_open_file ()
    {
        return_if_fail (_structure != null);
        _structure.do_action (StructAction.OPEN_FILE);
    }
}
