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

// The Documents menu of a MainWindow

public class MainWindowDocuments
{
    private const Gtk.ActionEntry[] _action_entries =
    {
        { "Documents", null, N_("_Documents") },

        { "DocumentsSaveAll", "document-save", N_("_Save All"), "<Shift><Control>L",
            N_("Save all open files"), on_save_all },

        { "DocumentsCloseAll", "window-close", N_("_Close All"), "<Shift><Control>W",
            N_("Close all open files"), on_close_all },

        { "DocumentsPrevious", "go-previous", N_("_Previous Document"),
            "<Control>Page_Up", N_("Activate previous document"), on_previous },

        { "DocumentsNext", "go-next", N_("_Next Document"),
            "<Control>Page_Down", N_("Activate next document"), on_next },

        { "DocumentsMoveToNewWindow", null, N_("_Move to New Window"), null,
            N_("Move the current document to a new window"), on_move_to_new_window }
    };

    private unowned MainWindow _main_window;
    private UIManager _ui_manager;
    private DocumentsPanel _documents_panel;

    private Gtk.ActionGroup _static_action_group;

    // List of opened documents
    private Gtk.ActionGroup _list_action_group;
    private uint _list_id;

    public MainWindowDocuments (MainWindow main_window, UIManager ui_manager)
    {
        _main_window = main_window;
        _ui_manager = ui_manager;

        _static_action_group = new Gtk.ActionGroup ("DocumentsMenuActionGroup");
        _static_action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
        _static_action_group.add_actions (_action_entries, this);
        ui_manager.insert_action_group (_static_action_group, 0);

        _list_action_group = new Gtk.ActionGroup ("DocumentsListActionGroup");
        ui_manager.insert_action_group (_list_action_group, 0);
    }

    public void set_documents_panel (DocumentsPanel documents_panel)
    {
        _documents_panel = documents_panel;
        connect_signals ();
        update_sensitivity ();
    }

    private void connect_signals ()
    {
        return_if_fail (_documents_panel != null);

        _documents_panel.page_reordered.connect (() =>
        {
            update_sensitivity ();
            update_documents_list ();
        });

        _documents_panel.switch_page.connect ((pg, page_num) =>
        {
            set_active_document (page_num);
            update_sensitivity ();
        });

        _documents_panel.page_removed.connect (() =>
        {
            update_documents_list ();
            update_sensitivity ();
        });

        _documents_panel.page_added.connect (() =>
        {
            update_documents_list ();
            update_sensitivity ();
        });
    }

    private void update_documents_list ()
    {
        return_if_fail (_list_action_group != null);
        return_if_fail (_documents_panel != null);

        if (_list_id != 0)
            _ui_manager.remove_ui (_list_id);

        foreach (Gtk.Action action in _list_action_group.list_actions ())
        {
            action.activate.disconnect (list_action_activate);
            _list_action_group.remove_action (action);
        }

        _list_id = 0;

        int nb_docs = _documents_panel.get_n_pages ();
        if (nb_docs == 0)
            return;

        _list_id = _ui_manager.new_merge_id ();

        unowned SList<RadioAction> group = null;

        for (int doc_num = 0 ; doc_num < nb_docs ; doc_num++)
        {
            DocumentTab tab = _documents_panel.get_nth_page (doc_num) as DocumentTab;
            string action_name = get_list_action_name (doc_num);
            string name = tab.get_name ().replace ("_", "__");
            string tip = tab.get_menu_tip ();

            string accel = null;
            if (doc_num < 10)
                accel = "<alt>%d".printf ((doc_num + 1) % 10);

            RadioAction action = new RadioAction (action_name, name, tip, null, doc_num);
            if (group != null)
                action.set_group (group);

            /* group changes each time we add an action, so it must be updated */
            group = action.get_group ();

            _list_action_group.add_action_with_accel (action, accel);

            action.activate.connect (list_action_activate);

            _ui_manager.add_ui (_list_id,
                "/MainMenu/DocumentsMenu/DocumentsListPlaceholder",
                action_name, action_name, UIManagerItemType.MENUITEM, false);

            if (tab == _main_window.active_tab)
                action.set_active (true);
        }
    }

    private void set_active_document (uint doc_num)
    {
        string action_name = get_list_action_name (doc_num);
        RadioAction? action = _list_action_group.get_action (action_name) as RadioAction;

        if (action != null)
        {
            // Disconnect and reconnect the signal to avoid emitting it a second time.
            action.activate.disconnect (list_action_activate);
            action.set_active (true);
            action.activate.connect (list_action_activate);
        }
    }

    public void update_document_name (DocumentTab tab)
    {
        return_if_fail (_documents_panel != null);

        int doc_num = _documents_panel.page_num (tab);
        string action_name = get_list_action_name (doc_num);

        Gtk.Action? action = _list_action_group.get_action (action_name);
        return_if_fail (action != null);

        action.label = tab.get_name ().replace ("_", "__");
        action.tooltip = tab.get_menu_tip ();
    }

    private void list_action_activate (Gtk.Action action)
    {
        return_if_fail (_documents_panel != null);

        RadioAction radio_action = action as RadioAction;
        if (! radio_action.get_active ())
            return;

        _documents_panel.set_current_page (radio_action.get_current_value ());
    }

    private string get_list_action_name (uint doc_num)
    {
        return @"Tab_$doc_num";
    }

    /* Sensitivity */

    private void update_sensitivity ()
    {
        bool sensitive = _main_window.active_tab != null;

        string[] action_names =
        {
            "DocumentsSaveAll",
            "DocumentsCloseAll",
            "DocumentsPrevious",
            "DocumentsNext",
            "DocumentsMoveToNewWindow"
        };

        foreach (string action_name in action_names)
        {
            Gtk.Action action = _static_action_group.get_action (action_name);
            action.sensitive = sensitive;
        }

        if (sensitive)
            update_next_prev_doc_sensitivity ();
    }

    private void update_next_prev_doc_sensitivity ()
    {
        return_if_fail (_documents_panel != null);

        if (_main_window.active_tab == null)
            return;

        Gtk.Action action_prev = _static_action_group.get_action ("DocumentsPrevious");
        Gtk.Action action_next = _static_action_group.get_action ("DocumentsNext");

        int current_page = _documents_panel.page_num (_main_window.active_tab);
        action_prev.sensitive = current_page > 0;

        int nb_pages = _documents_panel.get_n_pages ();
        action_next.sensitive = current_page < nb_pages - 1;
    }

    /* Gtk.Action callbacks */

    public void on_save_all ()
    {
        return_if_fail (_main_window.active_tab != null);

        foreach (Document doc in _main_window.get_unsaved_documents ())
            doc.save ();
    }

    public void on_close_all ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.close_all_documents ();
    }

    public void on_previous ()
    {
        return_if_fail (_main_window.active_tab != null);
        return_if_fail (_documents_panel != null);
        _documents_panel.prev_page ();
    }

    public void on_next ()
    {
        return_if_fail (_main_window.active_tab != null);
        return_if_fail (_documents_panel != null);
        _documents_panel.next_page ();
    }

    public void on_move_to_new_window ()
    {
        DocumentTab tab = _main_window.active_tab;
        return_if_fail (tab != null);
        return_if_fail (_documents_panel != null);

        MainWindow new_window = LatexilaApp.get_instance ().create_window ();
        DocumentView view = tab.view;
        _documents_panel.remove_tab (tab);

        new_window.create_tab_with_view (view);
    }
}
