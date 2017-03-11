/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2010-2015 Sébastien Wilmet
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

// The configuration of a single build tool.

using Gtk;

public class BuildToolDialog : GLib.Object
{
    private enum IconColumn
    {
        ICON_NAME,
        LABEL,
        N_COLUMNS
    }

    private enum JobColumn
    {
        COMMAND,
        POST_PROCESSOR,
        N_COLUMNS
    }

    private enum PostProcessorColumn
    {
        NAME,
        N_COLUMNS
    }

    private Dialog _dialog;

    private Entry _entry_label;
    private Entry _entry_desc;
    private Entry _entry_extensions;
    private Entry _entry_files_to_open;

    private Gtk.ListStore _icons_store;
    private ComboBox _icons_combobox;

    private Gtk.ListStore _jobs_store;
    private TreeView _jobs_view;

    public BuildToolDialog (Gtk.Window parent, bool editable)
    {
        _dialog = GLib.Object.@new (typeof (Gtk.Dialog), "use-header-bar", true, null)
            as Gtk.Dialog;
        _dialog.destroy_with_parent = true;
        _dialog.modal = true;
        _dialog.set_transient_for (parent);

        if (editable)
        {
            _dialog.title = _("Personal Build Tool");
            _dialog.add_button (_("_Cancel"), ResponseType.CANCEL);
            _dialog.add_button (_("_Apply"), ResponseType.APPLY);
            _dialog.set_default_response (ResponseType.APPLY);
        }
        else
            _dialog.title = _("Default Build Tool (read-only)");

        init_text_entries (editable);
        init_icons_store ();
        init_icons_combobox ();
        init_jobs (editable);

        Box content_area = _dialog.get_content_area ();
        content_area.pack_start (get_main_grid (editable));
        content_area.show_all ();
    }

    // Returns true if the build tool is edited.
    public bool open_build_tool (Latexila.BuildTools build_tools, int build_tool_num)
    {
        Latexila.BuildTool? build_tool = build_tools.nth (build_tool_num);
        return_val_if_fail (build_tool != null, false);

        set_build_tool (build_tool);

        bool apply = _dialog.run () == ResponseType.APPLY;

        if (apply)
        {
            Latexila.BuildTool new_build_tool = retrieve_build_tool ();
            new_build_tool.enabled = build_tool.enabled;

            Latexila.BuildToolsPersonal personal_build_tools =
                build_tools as Latexila.BuildToolsPersonal;
            personal_build_tools.replace (new_build_tool, build_tool_num);
        }

        _dialog.destroy ();
        return apply;
    }

    // Returns true if the build tool is created.
    // Returns false if the user has clicked on cancel.
    public bool create_personal_build_tool ()
    {
        set_new_build_tool ();

        bool apply = _dialog.run () == ResponseType.APPLY;

        if (apply)
        {
            Latexila.BuildTool new_build_tool = retrieve_build_tool ();
            new_build_tool.enabled = true;

            Latexila.BuildToolsPersonal build_tools =
                Latexila.BuildToolsPersonal.get_instance ();
            build_tools.add (new_build_tool);
        }

        _dialog.destroy ();
        return apply;
    }

    /*************************************************************************/
    // Init main widgets

    private void init_text_entries (bool editable)
    {
        _entry_label = new Entry ();
        _entry_desc = new Entry ();
        _entry_extensions = new Entry ();
        _entry_files_to_open = new Entry ();

        _entry_label.set_editable (editable);
        _entry_desc.set_editable (editable);
        _entry_extensions.set_editable (editable);
        _entry_files_to_open.set_editable (editable);
    }

    private void init_icons_store ()
    {
        _icons_store = new Gtk.ListStore (IconColumn.N_COLUMNS,
            typeof (string), // icon-name
            typeof (string)  // label
        );

        add_icon ("system-run", _("Execute"));
        add_icon ("compile_dvi", "LaTeX → DVI");
        add_icon ("compile_pdf", "LaTeX → PDF");
        add_icon ("compile_ps", "LaTeX → PS");
        add_icon (Stock.CONVERT, _("Convert")); // FIXME don't use Stock
        add_icon ("text-x-generic", _("View File"));
        add_icon ("latexila-dvi", _("View DVI"));
        add_icon ("latexila-pdf", _("View PDF"));
        add_icon ("latexila-ps", _("View PS"));
    }

    private void add_icon (string icon_name, string label)
    {
        TreeIter iter;
        _icons_store.append (out iter);
        _icons_store.set (iter,
            IconColumn.ICON_NAME, icon_name,
            IconColumn.LABEL, label
        );
    }

    private void init_icons_combobox ()
    {
        _icons_combobox = new ComboBox.with_model (_icons_store);

        CellRendererPixbuf pixbuf_renderer = new CellRendererPixbuf ();
        _icons_combobox.pack_start (pixbuf_renderer, false);
        _icons_combobox.set_attributes (pixbuf_renderer,
            "icon-name", IconColumn.ICON_NAME);

        CellRendererText text_renderer = new CellRendererText ();
        _icons_combobox.pack_start (text_renderer, true);
        _icons_combobox.set_attributes (text_renderer,
            "text", IconColumn.LABEL);
    }

    private void init_jobs (bool editable)
    {
        _jobs_store = new Gtk.ListStore (JobColumn.N_COLUMNS,
            typeof (string), // command
            typeof (string)  // post processor
        );

        _jobs_view = new TreeView.with_model (_jobs_store);

        /* Post processors list store */

        Gtk.ListStore post_processor_store = new Gtk.ListStore (PostProcessorColumn.N_COLUMNS,
            typeof (string) // the name of the post processor
        );

        for (int type = 0 ; type < Latexila.PostProcessorType.NB_TYPES ; type++)
        {
            unowned string name = Latexila.PostProcessor.get_name_from_type (
                (Latexila.PostProcessorType) type);

            TreeIter iter;
            post_processor_store.append (out iter);
            post_processor_store.set (iter, PostProcessorColumn.NAME, name);
        }

        post_processor_store.set_sort_column_id (PostProcessorColumn.NAME,
            SortType.ASCENDING);

        /* Cell renderers */

        CellRendererText text_renderer = new CellRendererText ();

        // In all cases, the text must be selectable, but there is no property
        // for that. So set as editable, but in read-only mode the edited text
        // isn't changed (see below).
        text_renderer.editable = true;

        TreeViewColumn column = new TreeViewColumn.with_attributes (_("Commands"),
            text_renderer, "text", JobColumn.COMMAND);
        column.set_resizable (true);
        _jobs_view.append_column (column);

        CellRendererCombo combo_renderer = new CellRendererCombo ();
        combo_renderer.editable = editable;
        combo_renderer.model = post_processor_store;
        combo_renderer.text_column = PostProcessorColumn.NAME;
        combo_renderer.has_entry = false;
        _jobs_view.insert_column_with_attributes (-1, _("Post Processor"),
            combo_renderer, "text", JobColumn.POST_PROCESSOR);

        /* Behavior */

        if (editable)
        {
            text_renderer.edited.connect ((path_string, new_text) =>
            {
                TreeIter iter;
                _jobs_store.get_iter_from_string (out iter, path_string);
                _jobs_store.set (iter, JobColumn.COMMAND, new_text);
            });
        }

        combo_renderer.edited.connect ((path_string, new_text) =>
        {
            TreeIter iter;
            _jobs_store.get_iter_from_string (out iter, path_string);
            _jobs_store.set (iter, JobColumn.POST_PROCESSOR, new_text);
        });
    }

    /*************************************************************************/
    // Jobs: mini-toolbar buttons (add, remove, up, down)

    private ToolButton get_add_button ()
    {
        ToolButton add_button = new ToolButton (null, null);
        add_button.set_icon_name ("list-add-symbolic");
        add_button.set_tooltip_text (_("Add..."));

        add_button.clicked.connect (() =>
        {
            Latexila.BuildJob new_job = new Latexila.BuildJob ();
            new_job.command = "";
            new_job.post_processor_type = Latexila.PostProcessorType.ALL_OUTPUT;
            add_build_job (new_job);
        });

        return add_button;
    }

    private ToolButton get_remove_button ()
    {
        ToolButton remove_button = new ToolButton (null, null);
        remove_button.set_icon_name ("list-remove-symbolic");
        remove_button.set_tooltip_text (_("Remove"));

        /* Sensitivity */

        remove_button.set_sensitive (false);

        unowned TreeSelection select = _jobs_view.get_selection ();
        select.changed.connect (() =>
        {
            bool row_selected = select.count_selected_rows () > 0;
            remove_button.set_sensitive (row_selected);
        });

        /* Behavior */

        remove_button.clicked.connect (() =>
        {
            TreeIter iter;
            int selected_row = Utils.get_selected_row (_jobs_view, out iter);
            if (selected_row >= 0)
                _jobs_store.remove (ref iter);
        });

        return remove_button;
    }

    private ToolButton get_up_button ()
    {
        ToolButton up_button = new ToolButton (null, null);
        up_button.set_icon_name ("go-up-symbolic");
        up_button.set_tooltip_text (_("Move up"));

        /* Sensitivity */

        up_button.set_sensitive (false);

        unowned TreeSelection select = _jobs_view.get_selection ();
        select.changed.connect (() =>
        {
            List<TreePath> selected_rows = select.get_selected_rows (null);

            if (selected_rows.length () == 0)
            {
                up_button.set_sensitive (false);
                return;
            }

            TreePath path_selected = selected_rows.nth_data (0);
            int row_num = path_selected.get_indices ()[0];

            up_button.set_sensitive (row_num > 0);
        });

        /* Behavior */

        up_button.clicked.connect (() =>
        {
            TreeIter iter_selected;

            int selected_row = Utils.get_selected_row (_jobs_view, out iter_selected);

            if (selected_row > 0)
            {
                TreeIter iter_up = iter_selected;
                if (Utils.tree_model_iter_prev (_jobs_store, ref iter_up))
                {
                    _jobs_store.swap (iter_selected, iter_up);

                    // Force the 'changed' signal on the selection to be emitted
                    select.changed ();
                }
            }
        });

        return up_button;
    }

    private ToolButton get_down_button ()
    {
        ToolButton down_button = new ToolButton (null, null);
        down_button.set_icon_name ("go-down-symbolic");
        down_button.set_tooltip_text (_("Move down"));

        /* Sensitivity */

        down_button.set_sensitive (false);

        unowned TreeSelection select = _jobs_view.get_selection ();
        select.changed.connect (() =>
        {
            List<TreePath> selected_rows = select.get_selected_rows (null);

            if (selected_rows.length () == 0)
            {
                down_button.set_sensitive (false);
                return;
            }

            TreePath path_selected = selected_rows.nth_data (0);
            int row_num = path_selected.get_indices ()[0];

            TreeModel model = _jobs_store as TreeModel;
            int nb_rows = model.iter_n_children (null);

            down_button.set_sensitive (row_num < nb_rows - 1);
        });

        /* Behavior */

        down_button.clicked.connect (() =>
        {
            TreeIter iter_selected;

            int selected_row = Utils.get_selected_row (_jobs_view, out iter_selected);

            if (selected_row >= 0)
            {
                TreeIter iter_down = iter_selected;
                if (_jobs_store.iter_next (ref iter_down))
                {
                    _jobs_store.swap (iter_selected, iter_down);

                    // Force the 'changed' signal on the selection to be emitted
                    select.changed ();
                }
            }
        });

        return down_button;
    }

    /*************************************************************************/
    // Set and retrieve data

    private void set_new_build_tool ()
    {
        _entry_label.text = "";
        _entry_desc.text = "";
        _entry_extensions.text = "";
        _entry_files_to_open.text = "";

        _icons_combobox.set_active (0);

        _jobs_store.clear ();
        _jobs_view.columns_autosize ();
    }

    private void set_build_tool (Latexila.BuildTool build_tool)
    {
        /* Text entries */

        _entry_label.text = build_tool.label ?? "";
        _entry_desc.text = build_tool.get_description () ?? "";
        _entry_extensions.text = build_tool.extensions ?? "";
        _entry_files_to_open.text = build_tool.files_to_open ?? "";

        /* Icon */

        _icons_combobox.set_active (0);

        TreeIter iter;
        bool ok = _icons_store.get_iter_first (out iter);
        return_if_fail (ok);

        TreeModel model = _icons_store as TreeModel;

        do
        {
            string icon_name;
            model.get (iter, IconColumn.ICON_NAME, out icon_name);

            if (icon_name == build_tool.icon)
            {
                _icons_combobox.set_active_iter (iter);
                break;
            }
        }
        while (_icons_store.iter_next (ref iter));

        /* Jobs */

        _jobs_store.clear ();
        foreach (Latexila.BuildJob build_job in build_tool.get_jobs ())
            add_build_job (build_job);

        _jobs_view.columns_autosize ();
    }

    // Retrieve the build tool from the data stored in the main widgets of the dialog.
    private Latexila.BuildTool retrieve_build_tool ()
    {
        Latexila.BuildTool tool = new Latexila.BuildTool ();

        /* Text entries */

        tool.label = _entry_label.text.strip ();
        tool.description = _entry_desc.text.strip ();
        tool.extensions = _entry_extensions.text.strip ();
        tool.files_to_open = _entry_files_to_open.text.strip ();

        /* Icon */

        TreeIter iter;
        _icons_combobox.get_active_iter (out iter);
        TreeModel model = _icons_store as TreeModel;
        string icon_name;
        model.get (iter, IconColumn.ICON_NAME, out icon_name);
        tool.icon = icon_name;

        /* Jobs */

        model = _jobs_store as TreeModel;
        bool valid = _jobs_store.get_iter_first (out iter);

        while (valid)
        {
            Latexila.BuildJob job = new Latexila.BuildJob ();
            string command;
            string post_processor_name;

            model.get (iter,
                JobColumn.COMMAND, out command,
                JobColumn.POST_PROCESSOR, out post_processor_name
            );

            job.command = command.strip ();

            Latexila.PostProcessorType type;
            if (Latexila.PostProcessor.get_type_from_name (post_processor_name, out type))
                job.post_processor_type = type;

            tool.add_job (job);

            valid = _jobs_store.iter_next (ref iter);
        }

        return tool;
    }

    /*************************************************************************/
    // Misc utilities functions

    private void add_build_job (Latexila.BuildJob job)
    {
        string post_processor_name =
            Latexila.PostProcessor.get_name_from_type (job.post_processor_type);

        TreeIter iter;
        _jobs_store.append (out iter);
        _jobs_store.set (iter,
            JobColumn.COMMAND, job.command,
            JobColumn.POST_PROCESSOR, post_processor_name
        );

        // Force the 'changed' signal on the selection to be emitted
        unowned TreeSelection select = _jobs_view.get_selection ();
        select.changed ();
    }

    /*************************************************************************/
    // Packing widgets, add section titles, tooltips, etc.

    private Grid get_main_grid (bool editable)
    {
        Grid main_grid = new Grid ();
        main_grid.set_row_spacing (5);
        main_grid.set_column_spacing (5);

        main_grid.attach (get_label_grid (), 0, 0, 1, 1);
        main_grid.attach (get_desc_grid (), 1, 0, 1, 1);
        main_grid.attach (get_extensions_grid (), 0, 1, 1, 1);
        main_grid.attach (get_icons_grid (), 1, 1, 1, 1);
        main_grid.attach (get_jobs_grid (editable), 0, 2, 2, 1);
        main_grid.attach (get_files_to_open_grid (), 0, 3, 2, 1);

        return main_grid;
    }

    private Grid get_label_grid ()
    {
        Grid grid = new Grid ();
        grid.set_row_spacing (6);
        grid.border_width = 6;

        Label title = new Label (null);
        title.set_markup ("<b>" + _("Label") + "</b>");
        title.set_halign (Align.START);
        grid.attach (title, 0, 0, 1, 1);

        Label arrow = new Label ("→");
        arrow.set_tooltip_text (_("You can select this arrow and copy/paste it!"));
        arrow.set_halign (Align.CENTER);
        arrow.set_hexpand (true);
        arrow.set_selectable (true);
        grid.attach (arrow, 1, 0, 1, 1);
        grid.set_hexpand (false);

        _entry_label.set_margin_start (12);
        grid.attach (_entry_label, 0, 1, 2, 1);

        return grid;
    }

    private Widget get_desc_grid ()
    {
        _entry_desc.hexpand = true;
        return Latexila.utils_get_dialog_component (_("Description"), _entry_desc);
    }

    private Widget get_extensions_grid ()
    {
        _entry_extensions.set_tooltip_text (
            _("File extensions for which the build tool can be executed.") + "\n" +
            _("The extensions are separated by spaces.") + "\n" +
            _("If it is empty, all extensions are allowed."));

        return Latexila.utils_get_dialog_component (_("Extensions"), _entry_extensions);
    }

    private Widget get_icons_grid ()
    {
        return Latexila.utils_get_dialog_component (_("Icon"), _icons_combobox);
    }

    private Widget get_jobs_grid (bool editable)
    {
        /* Placeholders */

        Label placeholders = new Label (_("Placeholders:"));

        Label placeholder_filename = new Label ("$filename");
        placeholder_filename.set_tooltip_text (
            _("The active document's filename.") + "\n" +
            _("If the active document belongs to a project, the main file is chosen."));

        Label placeholder_shortname = new Label ("$shortname");
        placeholder_shortname.set_tooltip_text (
            _("The active document's filename without its extension.") + "\n" +
            _("If the active document belongs to a project, the main file is chosen."));

        Grid placeholders_grid = new Grid ();
        placeholders_grid.set_orientation (Orientation.HORIZONTAL);
        placeholders_grid.set_column_spacing (10);
        placeholders_grid.add (placeholders);
        placeholders_grid.add (placeholder_filename);
        placeholders_grid.add (placeholder_shortname);

        /* Jobs tree view */

        _jobs_view.expand = true;

        ScrolledWindow scrolled_window = Utils.add_scrollbar (_jobs_view);
        scrolled_window.set_size_request (600, 120);
        scrolled_window.set_shadow_type (ShadowType.IN);

        /* Pack */

        Box jobs_box = new Box (Orientation.VERTICAL, 0);
        placeholders_grid.set_margin_bottom (8);
        jobs_box.pack_start (placeholders_grid, false);
        jobs_box.pack_start (scrolled_window);

        if (editable)
        {
            /* Toolbar */

            Toolbar toolbar = new Toolbar ();
            toolbar.insert (get_add_button (), -1);
            toolbar.insert (get_remove_button (), -1);
            toolbar.insert (get_up_button (), -1);
            toolbar.insert (get_down_button (), -1);

            toolbar.set_icon_size (IconSize.MENU);
            toolbar.set_style (ToolbarStyle.ICONS);

            jobs_box.pack_start (toolbar, false);

            /* Junction */

            StyleContext context = scrolled_window.get_style_context ();
            context.set_junction_sides (JunctionSides.BOTTOM);

            context = toolbar.get_style_context ();
            context.add_class (STYLE_CLASS_INLINE_TOOLBAR);
            context.set_junction_sides (JunctionSides.TOP);
        }

        return Latexila.utils_get_dialog_component (_("Jobs"), jobs_box);
    }

    private Widget get_files_to_open_grid ()
    {
        _entry_files_to_open.set_tooltip_text (
            _("List of files to open after executing the build jobs.") + "\n" +
            _("The files are separated by spaces.") + "\n" +
            _("You should use the placeholders to specify the files."));

        _entry_files_to_open.hexpand = true;
        return Latexila.utils_get_dialog_component (_("Files to open"), _entry_files_to_open);
    }
}
