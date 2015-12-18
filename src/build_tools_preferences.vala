/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2012-2015 Sébastien Wilmet
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

// The preferences of the default and personal build tools.
// For the configuration of a single build tool, see build_tool_dialog.vala.

using Gtk;

public class BuildToolsPreferences : GLib.Object
{
    private enum BuildToolColumn
    {
        ENABLED,
        PIXBUF,
        LABEL,
        DESCRIPTION,
        N_COLUMNS
    }

    private Dialog _dialog;
    private Gtk.ListStore _default_store;
    private Gtk.ListStore _personal_store;
    private TreeView _default_view;
    private TreeView _personal_view;

    public BuildToolsPreferences (Window parent_window)
    {
        _default_store = get_new_store ();
        _personal_store = get_new_store ();
        update_default_store ();
        update_personal_store ();

        init_views ();

        _dialog = GLib.Object.@new (typeof (Gtk.Dialog), "use-header-bar", true, null)
            as Gtk.Dialog;
        _dialog.set_transient_for (parent_window);
        _dialog.destroy_with_parent = true;
        _dialog.title = _("Build Tools");

        Grid hgrid = new Grid ();
        hgrid.set_orientation (Orientation.HORIZONTAL);
        hgrid.set_column_spacing (10);

        hgrid.add (get_default_grid ());
        hgrid.add (get_personal_grid ());

        Box content_area = _dialog.get_content_area ();
        content_area.pack_start (hgrid);
        content_area.show_all ();

        _dialog.run ();
        _dialog.destroy ();
        Latexila.BuildToolsPersonal.get_instance ().save ();
    }

    private void init_views ()
    {
        _default_view = get_new_view (_default_store,
            Latexila.BuildToolsDefault.get_instance ());

        _personal_view = get_new_view (_personal_store,
            Latexila.BuildToolsPersonal.get_instance ());

        // Only one item of the two views can be selected at once.

        TreeSelection default_select = _default_view.get_selection ();
        TreeSelection personal_select = _personal_view.get_selection ();

        default_select.changed.connect (() =>
        {
            on_view_selection_changed (default_select, personal_select);
        });

        personal_select.changed.connect (() =>
        {
            on_view_selection_changed (personal_select, default_select);
        });
    }

    private Widget get_default_grid ()
    {
        Latexila.BuildTools default_build_tools =
            Latexila.BuildToolsDefault.get_instance () as Latexila.BuildTools;
        ToolButton properties_button = get_properties_button (_default_view,
            default_build_tools, true);
        ToolButton copy_button = get_copy_button (_default_view, default_build_tools);

        Toolbar toolbar = new Toolbar ();
        toolbar.insert (properties_button, -1);
        toolbar.insert (copy_button, -1);

        Widget join = join_view_and_toolbar (_default_view, toolbar);

        return Latexila.utils_get_dialog_component (_("Default Build Tools"), join);
    }

    private Widget get_personal_grid ()
    {
        Latexila.BuildTools personal_build_tools =
            Latexila.BuildToolsPersonal.get_instance () as Latexila.BuildTools;
        ToolButton properties_button = get_properties_button (_personal_view,
            personal_build_tools, false);
        ToolButton copy_button = get_copy_button (_personal_view, personal_build_tools);

        Toolbar toolbar = new Toolbar ();
        toolbar.insert (properties_button, -1);
        toolbar.insert (copy_button, -1);
        toolbar.insert (get_add_button (), -1);
        toolbar.insert (get_remove_button (), -1);
        toolbar.insert (get_up_button (), -1);
        toolbar.insert (get_down_button (), -1);

        Widget join = join_view_and_toolbar (_personal_view, toolbar);

        return Latexila.utils_get_dialog_component (_("Personal Build Tools"), join);
    }

    private Widget join_view_and_toolbar (TreeView view, Toolbar toolbar)
    {
        view.expand = true;
        ScrolledWindow scrolled_window = Utils.add_scrollbar (view);
        scrolled_window.set_shadow_type (ShadowType.IN);
        scrolled_window.set_size_request (350, 200);

        toolbar.set_icon_size (IconSize.MENU);
        toolbar.set_style (ToolbarStyle.ICONS);

        StyleContext context = toolbar.get_style_context ();
        context.add_class (STYLE_CLASS_INLINE_TOOLBAR);

        return Latexila.utils_join_widgets (scrolled_window, toolbar);
    }

    private Gtk.ListStore get_new_store ()
    {
        return new Gtk.ListStore (BuildToolColumn.N_COLUMNS,
            typeof (bool),   // enabled
            typeof (string), // pixbuf (icon-name)
            typeof (string), // label
            typeof (string)  // description
        );
    }

    private TreeView get_new_view (Gtk.ListStore store, Latexila.BuildTools build_tools)
    {
        TreeView view = new TreeView.with_model (store);

        TreeViewColumn enabled_column = new TreeViewColumn ();
        enabled_column.set_title (_("Enabled"));
        view.append_column (enabled_column);

        CellRendererToggle toggle_renderer = new CellRendererToggle ();
        enabled_column.pack_start (toggle_renderer, false);
        enabled_column.set_attributes (toggle_renderer,
            "active", BuildToolColumn.ENABLED);

        TreeViewColumn label_column = new TreeViewColumn ();
        label_column.set_title (_("Label"));
        view.append_column (label_column);

        CellRendererPixbuf pixbuf_renderer = new CellRendererPixbuf ();
        label_column.pack_start (pixbuf_renderer, false);
        label_column.set_attributes (pixbuf_renderer,
            "icon-name", BuildToolColumn.PIXBUF);

        CellRendererText text_renderer = new CellRendererText ();
        label_column.pack_start (text_renderer, true);
        label_column.set_attributes (text_renderer,
          "text", BuildToolColumn.LABEL);

        view.set_tooltip_column (BuildToolColumn.DESCRIPTION);

        TreeSelection select = view.get_selection ();
        select.set_mode (SelectionMode.SINGLE);

        /* Enable and disable a build tool */
        toggle_renderer.toggled.connect ((path_string) =>
        {
            TreeIter iter;
            store.get_iter_from_string (out iter, path_string);

            bool enabled;
            TreeModel model = store as TreeModel;
            model.get (iter, BuildToolColumn.ENABLED, out enabled);

            enabled = ! enabled;
            store.set (iter, BuildToolColumn.ENABLED, enabled);

            int num = int.parse (path_string);
            build_tools.set_enabled (num, enabled);
        });

        /* Double-click */
        view.row_activated.connect ((path, column) =>
        {
            if (column == label_column)
            {
                int build_tool_num = path.get_indices ()[0];
                open_build_tool (build_tools, build_tool_num);
            }
        });

        return view;
    }

    private void on_view_selection_changed (TreeSelection select,
        TreeSelection other_select)
    {
        List<TreePath> selected_items = select.get_selected_rows (null);
        if (selected_items.length () > 0)
            other_select.unselect_all ();
    }

    private ToolButton get_properties_button (TreeView view,
        Latexila.BuildTools build_tools, bool read_only)
    {
        ToolButton properties_button = new ToolButton (null, null);

        if (read_only)
        {
            properties_button.set_icon_name ("edit-find-symbolic");
            properties_button.set_tooltip_text (_("View the properties (read-only)"));
        }
        else
        {
            properties_button.set_icon_name ("document-properties-symbolic");
            properties_button.set_tooltip_text (_("Edit the properties"));
        }

        set_sensitivity_on_selection (view, properties_button);

        properties_button.clicked.connect (() =>
        {
            int build_tool_num = Utils.get_selected_row (view);

            if (0 <= build_tool_num)
                open_build_tool (build_tools, build_tool_num);
        });

        return properties_button;
    }

    private ToolButton get_copy_button (TreeView view, Latexila.BuildTools build_tools)
    {
        ToolButton copy_button = new ToolButton (null, null);
        copy_button.set_icon_name ("edit-copy-symbolic");
        copy_button.set_tooltip_text (_("Create a copy"));

        set_sensitivity_on_selection (view, copy_button);

        copy_button.clicked.connect (() =>
        {
            int selected_row = Utils.get_selected_row (view);
            if (selected_row < 0)
                return;

            Latexila.BuildTool? tool = build_tools.nth (selected_row);
            return_if_fail (tool != null);

            tool = tool.clone ();
            tool.enabled = false;
            tool.label = _("%s [copy]").printf (tool.label);

            Latexila.BuildToolsPersonal personal_build_tools =
                Latexila.BuildToolsPersonal.get_instance ();
            personal_build_tools.add (tool);

            update_personal_store ();
        });

        return copy_button;
    }

    private ToolButton get_add_button ()
    {
        ToolButton add_button = new ToolButton (null, null);
        add_button.set_icon_name ("list-add-symbolic");
        add_button.set_tooltip_text (_("Add..."));

        add_button.clicked.connect (() =>
        {
            BuildToolDialog dialog = new BuildToolDialog (_dialog, true);

            if (dialog.create_personal_build_tool ())
                update_personal_store ();
        });

        return add_button;
    }

    private ToolButton get_remove_button ()
    {
        ToolButton remove_button = new ToolButton (null, null);
        remove_button.set_icon_name ("list-remove-symbolic");
        remove_button.set_tooltip_text (_("Remove"));

        set_sensitivity_on_selection (_personal_view, remove_button);

        remove_button.clicked.connect (() =>
        {
            TreeIter iter;
            int selected_row = Utils.get_selected_row (_personal_view, out iter);
            if (selected_row == -1)
                return;

            string label;
            TreeModel model = _personal_store as TreeModel;
            model.get (iter, BuildToolColumn.LABEL, out label);

            Dialog dialog = new MessageDialog (_dialog,
                DialogFlags.DESTROY_WITH_PARENT | DialogFlags.MODAL,
                MessageType.QUESTION, ButtonsType.NONE,
                _("Do you really want to delete the build tool \"%s\"?"),
                label);

            dialog.add_buttons (_("_Cancel"), ResponseType.CANCEL,
                _("_Delete"), ResponseType.YES);

            if (dialog.run () == ResponseType.YES)
            {
                _personal_store.remove (iter);
                Latexila.BuildToolsPersonal.get_instance ().delete (selected_row);
            }

            dialog.destroy ();
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

        unowned TreeSelection select = _personal_view.get_selection ();
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

            int selected_row = Utils.get_selected_row (_personal_view, out iter_selected);

            if (selected_row > 0)
            {
                TreeIter iter_up = iter_selected;
                if (Utils.tree_model_iter_prev (_personal_store, ref iter_up))
                {
                    _personal_store.swap (iter_selected, iter_up);
                    Latexila.BuildToolsPersonal.get_instance ().move_up (selected_row);

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

        unowned TreeSelection select = _personal_view.get_selection ();
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

            TreeModel model = _personal_store as TreeModel;
            int nb_rows = model.iter_n_children (null);

            down_button.set_sensitive (row_num < nb_rows - 1);
        });

        /* Behavior */

        down_button.clicked.connect (() =>
        {
            TreeIter iter_selected;

            int selected_row = Utils.get_selected_row (_personal_view, out iter_selected);

            if (selected_row >= 0)
            {
                TreeIter iter_down = iter_selected;
                if (_personal_store.iter_next (ref iter_down))
                {
                    _personal_store.swap (iter_selected, iter_down);
                    Latexila.BuildToolsPersonal.get_instance ().move_down (selected_row);

                    // Force the 'changed' signal on the selection to be emitted
                    select.changed ();
                }
            }
        });

        return down_button;
    }

    private void update_default_store ()
    {
        update_store (_default_store, Latexila.BuildToolsDefault.get_instance ());
    }

    private void update_personal_store ()
    {
        update_store (_personal_store, Latexila.BuildToolsPersonal.get_instance ());
    }

    private void update_store (Gtk.ListStore store, Latexila.BuildTools build_tools)
    {
        store.clear ();

        foreach (Latexila.BuildTool tool in build_tools.build_tools)
        {
            string description = Markup.escape_text (tool.get_description ());

            TreeIter iter;
            store.append (out iter);
            store.set (iter,
                BuildToolColumn.ENABLED, tool.enabled,
                BuildToolColumn.PIXBUF, tool.icon,
                BuildToolColumn.LABEL, tool.label,
                BuildToolColumn.DESCRIPTION, description
            );
        }
    }

    private void open_build_tool (Latexila.BuildTools build_tools, int build_tool_num)
    {
        bool editable = build_tools is Latexila.BuildToolsPersonal;
        BuildToolDialog dialog = new BuildToolDialog (_dialog, editable);

        bool edited = dialog.open_build_tool (build_tools, build_tool_num);

        // If the build tool is edited, then it is a personal build tool.
        if (edited)
            update_personal_store ();
    }

    // Set 'widget' as sensitive when there is a selection in the TreeView.
    // If no elements are selected (this is the case by default),
    // the widget is insensitive.
    private void set_sensitivity_on_selection (TreeView view, Widget widget)
    {
        widget.set_sensitive (false);

        unowned TreeSelection select = view.get_selection ();
        select.changed.connect (() =>
        {
            bool row_selected = select.count_selected_rows () > 0;
            widget.set_sensitive (row_selected);
        });
    }
}
