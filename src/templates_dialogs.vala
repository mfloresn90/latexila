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

public class DeleteTemplateDialog : Dialog
{
    public DeleteTemplateDialog (MainWindow parent)
    {
        Object (use_header_bar: 1);
        title = _("Delete Template(s)...");
        set_transient_for (parent);
        destroy_with_parent = true;
        add_button (_("_Delete"), ResponseType.APPLY);

        /* List of the personal templates */

        Templates templates = Templates.get_default ();
        TreeView templates_list = templates.get_personal_templates_list ();

        TreeSelection select = templates_list.get_selection ();
        select.set_mode (SelectionMode.MULTIPLE);

        ScrolledWindow scrollbar = Utils.add_scrollbar (templates_list);
        scrollbar.set_shadow_type (ShadowType.IN);
        scrollbar.set_size_request (250, 150);
        Widget component = Latexila.utils_get_dialog_component (_("Personal Templates"),
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

        while (run () == ResponseType.APPLY)
        {
            TreeSelection select = templates_list.get_selection ();
            unowned TreeModel model;
            List<TreePath> selected_rows = select.get_selected_rows (out model);
            List<TreeRowReference> row_refs = null;

            foreach (TreePath path in selected_rows)
            {
                row_refs.prepend (new TreeRowReference (model, path));
            }

            foreach (TreeRowReference row_ref in row_refs)
            {
                TreePath path = row_ref.get_path ();
                templates.delete_personal_template (path);
                template_deleted = true;
            }
        }

        if (template_deleted)
            templates.save_rc_file ();
    }
}
