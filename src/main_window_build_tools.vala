/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2012, 2014 Sébastien Wilmet
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

public class MainWindowBuildTools
{
    private const Gtk.ActionEntry[] _action_entries =
    {
        { "Build", null, N_("_Build") },

        { "BuildClean", "edit-clear-all", N_("Cleanup Build _Files"), null,
            N_("Clean-up build files (*.aux, *.log, *.out, *.toc, etc)"),
            on_clean },

        { "BuildStopExecution", "process-stop", N_("_Stop Execution"), null,
            N_("Stop Execution"), on_stop_execution },

        { "BuildViewLog", "view_log", N_("View _Log"), null,
            N_("View Log"), on_view_log },

        { "BuildToolsPreferences", "preferences-system", N_("_Manage Build Tools"), null,
            N_("Manage Build Tools") }
    };

    private const ToggleActionEntry[] _toggle_action_entries =
    {
        { "BuildShowDetails", "zoom-in", N_("Show _Details"), null,
            N_("Show Details"), null },

        { "BuildShowWarnings", "dialog-warning", N_("Show _Warnings"), null,
            N_("Show Warnings"), null },

        { "BuildShowBadBoxes", "badbox", N_("Show _Bad Boxes"), null,
            N_("Show Bad Boxes"), null }
    };

    private unowned MainWindow _main_window;
    private UIManager _ui_manager;
    private Latexila.BuildView _build_view;
    private BottomPanel _bottom_panel;

    // Used for running a build tool, and clear it when running another build tool.
    private Cancellable? _cancellable;
    private AsyncResult? _build_tool_result;

    private Gtk.ActionGroup _static_action_group;
    private Gtk.ActionGroup _dynamic_action_group;
    private uint _menu_ui_id;

    public MainWindowBuildTools (MainWindow main_window, UIManager ui_manager)
    {
        _main_window = main_window;
        _ui_manager = ui_manager;

        /* Static Gtk.Actions */
        _static_action_group = new Gtk.ActionGroup ("BuildMenuActionGroup");
        _static_action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
        _static_action_group.add_actions (_action_entries, this);
        _static_action_group.add_toggle_actions (_toggle_action_entries, this);

        Gtk.Action stop_exec = _static_action_group.get_action ("BuildStopExecution");
        stop_exec.sensitive = false;

        ui_manager.insert_action_group (_static_action_group, 0);

        LatexilaApp app = LatexilaApp.get_instance ();
        Amtk.utils_bind_g_action_to_gtk_action (app, "manage-build-tools",
            _static_action_group, "BuildToolsPreferences");

        /* Dynamic Gtk.Actions (the placeholder) */
        _dynamic_action_group = new Gtk.ActionGroup ("BuildToolsActions");
        ui_manager.insert_action_group (_dynamic_action_group, 0);
        update_menu ();

        Latexila.BuildToolsPersonal personal_build_tools =
            Latexila.BuildToolsPersonal.get_instance ();
        personal_build_tools.modified.connect (() => update_menu ());
        personal_build_tools.loaded.connect (() => update_menu ());

        Latexila.BuildToolsDefault default_build_tools =
            Latexila.BuildToolsDefault.get_instance ();
        default_build_tools.modified.connect (() => update_menu ());
        default_build_tools.loaded.connect (() => update_menu ());
    }

    public void set_build_view (Latexila.BuildView build_view)
    {
        _build_view = build_view;
        connect_toggle_actions ();

        _build_view.jump_to_file.connect ((file, start_line, end_line) =>
        {
            if (start_line == -1)
                _main_window.open_document (file);
            else
                _main_window.jump_to_file_position (file, start_line, end_line);
        });
    }

    public void set_bottom_panel (BottomPanel bottom_panel)
    {
        _bottom_panel = bottom_panel;
    }

    public void update_sensitivity ()
    {
        bool build_tool_is_running = _cancellable != null;

        Gtk.Action stop_exec_action = _static_action_group.get_action ("BuildStopExecution");
        stop_exec_action.set_sensitive (build_tool_is_running);

        LatexilaApp app = LatexilaApp.get_instance ();
        GLib.SimpleAction preferences_action = app.lookup_action ("manage-build-tools")
            as GLib.SimpleAction;

        // a build tool can not be modified when it is running
        preferences_action.set_enabled (! build_tool_is_running);

        Gtk.Action clean_action = _static_action_group.get_action ("BuildClean");
        Gtk.Action view_log_action = _static_action_group.get_action ("BuildViewLog");

        if (_main_window.active_tab == null)
        {
            _dynamic_action_group.set_sensitive (false);
            clean_action.set_sensitive (false);
            view_log_action.set_sensitive (false);
            return;
        }

        bool is_tex = _main_window.active_document.is_main_file_a_tex_file ();
        view_log_action.set_sensitive (is_tex);

        if (build_tool_is_running)
        {
            _dynamic_action_group.set_sensitive (false);
            clean_action.set_sensitive (false);
            return;
        }

        _dynamic_action_group.set_sensitive (true);
        clean_action.set_sensitive (is_tex);

        Latexila.BuildTools build_tools =
            Latexila.BuildToolsDefault.get_instance () as Latexila.BuildTools;

        int tool_num = 0;
        foreach (Latexila.BuildTool tool in build_tools.build_tools)
        {
            string action_name = get_default_build_tool_name (tool_num);
            update_build_tool_sensitivity (tool, action_name);
            tool_num++;
        }

        build_tools = Latexila.BuildToolsPersonal.get_instance () as Latexila.BuildTools;
        tool_num = 0;
        foreach (Latexila.BuildTool tool in build_tools.build_tools)
        {
            string action_name = get_personal_build_tool_name (tool_num);
            update_build_tool_sensitivity (tool, action_name);
            tool_num++;
        }
    }

    private string get_default_build_tool_name (int tool_num)
    {
        return @"DefaultBuildTool_$tool_num";
    }

    private string get_personal_build_tool_name (int tool_num)
    {
        return @"PersonalBuildTool_$tool_num";
    }

    private Latexila.BuildTool? get_build_tool_from_name (string action_name)
    {
        Latexila.BuildTools build_tools;

        if (action_name.has_prefix ("DefaultBuildTool_"))
            build_tools = Latexila.BuildToolsDefault.get_instance () as Latexila.BuildTools;

        else if (action_name.has_prefix ("PersonalBuildTool_"))
            build_tools = Latexila.BuildToolsPersonal.get_instance () as Latexila.BuildTools;

        else
            return_val_if_reached (null);

        string[] name = action_name.split ("_");
        return_val_if_fail (name.length == 2, null);

        int tool_num = int.parse (name[1]);

        return build_tools.nth (tool_num);
    }

    private void update_build_tool_sensitivity (Latexila.BuildTool tool,
        string action_name)
    {
        if (! tool.enabled)
            return;

        Gtk.Action action = _dynamic_action_group.get_action (action_name);

        Document active_doc = _main_window.active_document;
        bool unsaved_doc = active_doc.location == null;

        if (unsaved_doc)
        {
            action.set_sensitive (tool.get_jobs () != null);
            return;
        }

        string path = active_doc.get_main_file ().get_parse_name ();
        string ext = Latexila.utils_get_extension (path);

        string[] extensions = tool.extensions.split (" ");
        bool sensitive = tool.extensions.length == 0 || ext in extensions;
        action.set_sensitive (sensitive);
    }

    public void save_state ()
    {
        GLib.Settings settings =
            new GLib.Settings ("org.gnome.latexila.preferences.ui");

        ToggleAction action =
            _static_action_group.get_action ("BuildShowWarnings") as ToggleAction;
        settings.set_boolean ("show-build-warnings", action.active);

        action = _static_action_group.get_action ("BuildShowBadBoxes") as ToggleAction;
        settings.set_boolean ("show-build-badboxes", action.active);
    }

    private void update_menu ()
    {
        return_if_fail (_dynamic_action_group != null);

        if (_menu_ui_id != 0)
            _ui_manager.remove_ui (_menu_ui_id);

        foreach (Gtk.Action action in _dynamic_action_group.list_actions ())
        {
            action.activate.disconnect (activate_dynamic_action);
            _dynamic_action_group.remove_action (action);
        }

        Latexila.BuildTools default_build_tools =
            Latexila.BuildToolsDefault.get_instance () as Latexila.BuildTools;
        Latexila.BuildTools personal_build_tools =
            Latexila.BuildToolsPersonal.get_instance () as Latexila.BuildTools;

        /* Empty */
        if (default_build_tools.build_tools == null &&
            personal_build_tools.build_tools == null)
        {
            _menu_ui_id = 0;
            return;
        }

        _menu_ui_id = _ui_manager.new_merge_id ();

        /* Add the default build tools */
        int tool_num = 0;
        int accel_num = 2;
        foreach (Latexila.BuildTool build_tool in default_build_tools.build_tools)
        {
            string action_name = get_default_build_tool_name (tool_num);
            add_dynamic_action (build_tool, action_name, ref accel_num);
            tool_num++;
        }

        add_separator ();

        /* Add the personal build tools */
        tool_num = 0;
        foreach (Latexila.BuildTool build_tool in personal_build_tools.build_tools)
        {
            string action_name = get_personal_build_tool_name (tool_num);
            add_dynamic_action (build_tool, action_name, ref accel_num);
            tool_num++;
        }

        update_sensitivity ();
    }

    private void add_separator ()
    {
        _ui_manager.add_ui (_menu_ui_id,
            "/MainMenu/BuildMenu/BuildToolsPlaceholderMenu",
            "BuildToolsSeparator", null, UIManagerItemType.SEPARATOR, false);

        _ui_manager.add_ui (_menu_ui_id,
            "/MainToolbar/BuildToolsPlaceholderToolbar",
            "BuildToolsSeparator", null, UIManagerItemType.SEPARATOR, false);
    }

    private void add_dynamic_action (Latexila.BuildTool build_tool, string action_name,
        ref int accel_num)
    {
        if (! build_tool.enabled)
            return;

        Gtk.Action action = new Gtk.Action (action_name, build_tool.label,
            build_tool.get_description (), null);
        action.icon_name = build_tool.icon;
        action.set_always_show_image (true);

        // F2 -> F11
        // (F1 = help, F12 = show/hide side panel)
        string? accel = null;
        if (accel_num <= 11)
            accel = @"<Release>F$accel_num";

        _dynamic_action_group.add_action_with_accel (action, accel);
        action.activate.connect (activate_dynamic_action);

        _ui_manager.add_ui (_menu_ui_id,
            "/MainMenu/BuildMenu/BuildToolsPlaceholderMenu",
            action_name, action_name, UIManagerItemType.MENUITEM, false);

        _ui_manager.add_ui (_menu_ui_id,
            "/MainToolbar/BuildToolsPlaceholderToolbar",
            action_name, action_name, UIManagerItemType.TOOLITEM, false);

        accel_num++;
    }

    private void activate_dynamic_action (Gtk.Action action)
    {
        return_if_fail (_main_window.active_tab != null);
        return_if_fail (_build_view != null);
        return_if_fail (_bottom_panel != null);

        Latexila.BuildTool? tool = get_build_tool_from_name (action.name);
        return_if_fail (tool != null);

        Document active_doc = _main_window.active_document;

        if (tool.get_jobs () == null)
            return_if_fail (active_doc.location != null);

        /* Save the document if jobs are executed */
        if (tool.get_jobs () != null)
        {
            if (active_doc.location == null)
            {
                bool tmp_location_set = active_doc.set_tmp_location ();
                return_if_fail (tmp_location_set);
            }

            int project_id = active_doc.project_id;

            if (project_id == -1)
                active_doc.save ();

            // Save all the documents belonging to the project
            else
            {
                Gee.List<Document> docs = LatexilaApp.get_instance ().get_documents ();
                foreach (Document doc in docs)
                {
                    if (doc.project_id == project_id)
                        doc.save ();
                }
            }

            // Ensure that the files are correctly saved before the execution.
            Utils.flush_queue ();
        }

        /* Run the build tool */

        File main_file = active_doc.get_main_file ();
        _cancellable = new Cancellable ();
        _build_tool_result = null;
        update_sensitivity ();
        tool.run_async.begin (main_file, _build_view, _cancellable, (obj, result) =>
        {
            _build_tool_result = result;
            tool.run_async.end (result);
            _cancellable = null;
            update_sensitivity ();

            // Refresh the document structure when the build tool has finished. When the
            // build tool is running, LaTeXila is already busy parsing the command
            // outputs. On the other hand, when the build tool has finished, maybe Evince
            // is shown so LaTeXila has nothing to do.
            _main_window.get_main_window_structure ().refresh ();
        });

        _bottom_panel.show ();
    }

    private void connect_toggle_actions ()
    {
        return_if_fail (_build_view != null);

        GLib.Settings settings = new GLib.Settings ("org.gnome.latexila.preferences.ui");

        /* Show details */

        ToggleAction action_details =
            _static_action_group.get_action ("BuildShowDetails") as ToggleAction;

        action_details.active = false;

        action_details.bind_property ("active", _build_view, "show-details",
            BindingFlags.DEFAULT | BindingFlags.SYNC_CREATE);

        _build_view.bind_property ("has-details", action_details, "sensitive",
            BindingFlags.DEFAULT | BindingFlags.SYNC_CREATE);

        /* Show warnings */

        ToggleAction action_warnings =
            _static_action_group.get_action ("BuildShowWarnings") as ToggleAction;

        action_warnings.active = settings.get_boolean ("show-build-warnings");

        action_warnings.bind_property ("active", _build_view, "show-warnings",
            BindingFlags.BIDIRECTIONAL | BindingFlags.SYNC_CREATE);

        /* Show badboxes */

        ToggleAction action_badboxes =
            _static_action_group.get_action ("BuildShowBadBoxes") as ToggleAction;

        action_badboxes.active = settings.get_boolean ("show-build-badboxes");

        action_badboxes.bind_property ("active", _build_view, "show-badboxes",
            BindingFlags.BIDIRECTIONAL | BindingFlags.SYNC_CREATE);
    }

    /* Gtk.Action callbacks */

    public void on_stop_execution ()
    {
        return_if_fail (_cancellable != null);
        _cancellable.cancel ();
    }

    public void on_clean ()
    {
        return_if_fail (_main_window.active_tab != null);

        CleanBuildFiles build_files = new CleanBuildFiles (_main_window,
            _main_window.active_document);

        build_files.clean ();
    }

    public void on_view_log ()
    {
        return_if_fail (_main_window.active_tab != null);
        return_if_fail (_main_window.active_document.is_main_file_a_tex_file ());

        File mainfile = _main_window.active_document.get_main_file ();
        File directory = mainfile.get_parent ();

        string basename = Latexila.utils_get_shortname (mainfile.get_basename ()) + ".log";
        File file = directory.get_child (basename);
        DocumentTab? tab = _main_window.open_document (file);

        if (tab == null)
            warning ("Impossible to view log");
        else
            tab.document.readonly = true;
    }
}
