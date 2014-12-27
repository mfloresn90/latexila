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
 */

using Gtk;

// Create a new document from a template.
public class OpenTemplateDialog
{
    private unowned MainWindow _main_window;
    private Dialog _dialog;
    private TreeView _default_templates;
    private TreeView _personal_templates;

    public OpenTemplateDialog (MainWindow main_window)
    {
        _main_window = main_window;

        _dialog = new Dialog.with_buttons (_("New File..."), main_window,
            DialogFlags.DESTROY_WITH_PARENT,
            _("_Cancel"), ResponseType.REJECT,
            _("_OK"), ResponseType.ACCEPT
        );

        Box content_area = _dialog.get_content_area () as Box;

        Grid hgrid = new Grid ();
        hgrid.set_orientation (Orientation.HORIZONTAL);
        hgrid.set_column_spacing (10);
        content_area.pack_start (hgrid);

        Templates templates = Templates.get_default ();

        // List of default templates.
        _default_templates = templates.get_default_templates_list ();

        ScrolledWindow scrollbar = Utils.add_scrollbar (_default_templates);
        scrollbar.set_shadow_type (ShadowType.IN);
        scrollbar.set_size_request (250, 200);
        Widget component = Utils.get_dialog_component (_("Default templates"), scrollbar);
        hgrid.add (component);

        // List of personal templates.
        _personal_templates = templates.get_personal_templates_list ();

        scrollbar = Utils.add_scrollbar (_personal_templates);
        scrollbar.set_shadow_type (ShadowType.IN);
        scrollbar.set_size_request (250, 200);
        component = Utils.get_dialog_component (_("Your personal templates"), scrollbar);
        hgrid.add (component);

        content_area.show_all ();

        connect_to_signals ();
        run_me ();
        _dialog.destroy ();
    }

    private void connect_to_signals ()
    {
        TreeSelection default_select = _default_templates.get_selection ();
        TreeSelection personal_select = _personal_templates.get_selection ();

        default_select.changed.connect (() =>
        {
            on_list_selection_changed (default_select, personal_select);
        });

        personal_select.changed.connect (() =>
        {
            on_list_selection_changed (personal_select, default_select);
        });

        _default_templates.row_activated.connect ((path) =>
        {
            open_default_template (path);
            _dialog.destroy ();
        });

        _personal_templates.row_activated.connect ((path) =>
        {
            open_personal_template (path);
            _dialog.destroy ();
        });
    }

    private void on_list_selection_changed (TreeSelection select,
        TreeSelection other_select)
    {
        // Only one item of the two lists can be selected at once.

        // We unselect all the items of the other list only if the current list
        // have an item selected, because when we unselect all the items the
        // "changed" signal is emitted for the other list, so for the
        // other list this function is also called but no item is selected so
        // nothing is done and the item selected by the user keeps selected.

        List<TreePath> selected_items = select.get_selected_rows (null);
        if (selected_items.length () > 0)
            other_select.unselect_all ();
    }

    private void run_me ()
    {
        if (_dialog.run () != ResponseType.ACCEPT)
            return;

        // Default template selected?
        TreeSelection select = _default_templates.get_selection ();
        List<TreePath> selected_items = select.get_selected_rows (null);

        if (selected_items.length () > 0)
        {
            TreePath path = selected_items.nth_data (0);
            open_default_template (path);
            return;
        }

        // Personal template selected?
        select = _personal_templates.get_selection ();
        selected_items = select.get_selected_rows (null);
        if (selected_items.length () > 0)
        {
            TreePath path = selected_items.nth_data (0);
            open_personal_template (path);
            return;
        }

        // No template selected
        create_document ("");
    }

    private void open_default_template (TreePath path)
    {
        Templates templates = Templates.get_default ();
        create_document (templates.get_default_template_contents (path));
    }

    private void open_personal_template (TreePath path)
    {
        Templates templates = Templates.get_default ();
        create_document (templates.get_personal_template_contents (path));
    }

    private void create_document (string contents)
    {
        DocumentTab tab = _main_window.create_tab (true);
        tab.document.set_contents (contents);
    }
}

public class CreateTemplateDialog : Dialog
{
    public CreateTemplateDialog (MainWindow parent)
    {
        return_val_if_fail (parent.active_tab != null, null);

        title = _("New Template...");
        set_transient_for (parent);
        destroy_with_parent = true;
        add_button (_("_Cancel"), ResponseType.REJECT);
        add_button (_("_OK"), ResponseType.ACCEPT);

        Box content_area = get_content_area () as Box;
        content_area.homogeneous = false;

        /* name */
        Entry entry = new Entry ();
        entry.hexpand = true;
        Widget component = Utils.get_dialog_component (_("Name of the new template"),
            entry);
        content_area.pack_start (component, false);

        /* icon */
        Templates templates = Templates.get_default ();

        // Take the default store because it contains all the icons.
        TreeView templates_list = templates.get_default_templates_list ();

        ScrolledWindow scrollbar = Utils.add_scrollbar (templates_list);
        scrollbar.set_shadow_type (ShadowType.IN);
        scrollbar.set_size_request (250, 200);
        component = Utils.get_dialog_component (_("Choose an icon"), scrollbar);
        content_area.pack_start (component);

        content_area.show_all ();

        run_me (parent, entry, templates_list);
    }

    private void run_me (MainWindow parent, Entry entry, TreeView templates_list)
    {
        Templates templates = Templates.get_default ();

        while (run () == ResponseType.ACCEPT)
        {
            // if no name specified
            if (entry.text_length == 0)
                continue;

            TreeSelection select = templates_list.get_selection ();
            List<TreePath> selected_items = select.get_selected_rows (null);

            // if no icon selected
            if (selected_items.length () == 0)
                continue;

            // get the contents
            TextIter start, end;
            parent.active_document.get_bounds (out start, out end);
            string contents = parent.active_document.get_text (start, end, false);

            // get the icon id
            TreePath path = selected_items.nth_data (0);
            string icon_id = templates.get_icon_id (path);

            templates.create_personal_template (entry.text, icon_id, contents);
            break;
        }
    }
}

public class DeleteTemplateDialog : Dialog
{
    public DeleteTemplateDialog (MainWindow parent)
    {
        title = _("Delete Template(s)...");
        add_button (_("_Delete"), ResponseType.ACCEPT);
        add_button (_("_Close"), ResponseType.REJECT);
        set_transient_for (parent);
        destroy_with_parent = true;

        /* List of the personal templates */

        Templates templates = Templates.get_default ();
        TreeView templates_list = templates.get_personal_templates_list ();

        TreeSelection select = templates_list.get_selection ();
        select.set_mode (SelectionMode.MULTIPLE);

        ScrolledWindow scrollbar = Utils.add_scrollbar (templates_list);
        scrollbar.set_shadow_type (ShadowType.IN);
        scrollbar.set_size_request (250, 150);
        Widget component = Utils.get_dialog_component (_("Personal templates"),
            scrollbar);

        Box content_area = get_content_area () as Box;
        content_area.pack_start (component);
        content_area.show_all ();

        run_me (templates_list);
    }

    private void run_me (TreeView templates_list)
    {
        Templates templates = Templates.get_default ();
        bool template_deleted = false;

        while (run () == ResponseType.ACCEPT)
        {
            TreeSelection select = templates_list.get_selection ();
            List<TreePath> selected_items = select.get_selected_rows (null);
            uint nb_selected_items = selected_items.length ();

            for (int item_num = 0 ; item_num < nb_selected_items ; item_num++)
            {
                TreePath path = selected_items.nth_data (item_num);
                templates.delete_personal_template (path);
                template_deleted = true;
            }
        }

        if (template_deleted)
            templates.save_rc_file ();
    }
}
