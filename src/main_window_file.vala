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

// The File menu of a MainWindow

public class MainWindowFile
{
    private const Gtk.ActionEntry[] _action_entries =
    {
        { "File", null, N_("_File") },

        { "FileNew", "document-new", N_("_New"), "<Control>N",
            N_("New file"), on_file_new },

        { "FileNewWindow", null, N_("New _Window"), null,
            N_("Create a new window"), on_new_window },

        { "FileOpen", "document-open", N_("_Open"), "<Control>O",
            N_("Open a file"), on_file_open },

        { "FileSave", "document-save", N_("_Save"), "<Control>S",
            N_("Save the current file"), on_file_save },

        { "FileSaveAs", "document-save-as", N_("Save _As"), "<Shift><Control>S",
            N_("Save the current file with a different name"), on_file_save_as },

        { "FileCreateTemplate", null, N_("Create _Template From Document..."), null,
            N_("Create a new template from the current document"), on_create_template },

        { "FileDeleteTemplate", null, N_("_Delete Template..."), null,
            N_("Delete personal template(s)"), on_delete_template },

        { "FileClose", "window-close", N_("_Close"), "<Control>W",
            N_("Close the current file"), on_file_close }
    };

    private unowned MainWindow _main_window;
    private Gtk.ActionGroup _action_group;

    public MainWindowFile (MainWindow main_window, UIManager ui_manager)
    {
        _main_window = main_window;

        _action_group = new Gtk.ActionGroup ("FileMenuActionGroup");
        _action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
        _action_group.add_actions (_action_entries, this);

        // recent documents
        Gtk.Action recent_action = new RecentAction ("FileOpenRecent", _("Open _Recent"),
            _("Open recently used files"), "");
        configure_recent_chooser (recent_action as RecentChooser);
        _action_group.add_action (recent_action);

        ui_manager.insert_action_group (_action_group, 0);
    }

    public ToolItem get_toolbar_open_button ()
    {
        RecentManager recent_manager = RecentManager.get_default ();
        Widget recent_menu = new RecentChooserMenu.for_manager (recent_manager);
        configure_recent_chooser (recent_menu as RecentChooser);

        MenuToolButton open_button = new MenuToolButton (null, null);
        open_button.icon_name = "document-open";
        open_button.set_menu (recent_menu);
        open_button.set_tooltip_text (_("Open a file"));
        open_button.set_arrow_tooltip_text (_("Open a recently used file"));

        Gtk.Action action = _action_group.get_action ("FileOpen");
        open_button.set_related_action (action);

        return open_button;
    }

    private void configure_recent_chooser (RecentChooser recent_chooser)
    {
        recent_chooser.set_local_only (false);
        recent_chooser.set_sort_type (RecentSortType.MRU);

        RecentFilter filter = new RecentFilter ();
        filter.add_application (Config.PACKAGE_NAME);
        recent_chooser.set_filter (filter);

        recent_chooser.item_activated.connect ((chooser) =>
        {
            string uri = chooser.get_current_uri ();
            _main_window.open_document (File.new_for_uri (uri));
        });
    }

    /* Sensitivity */

    public void update_sensitivity (bool sensitive)
    {
        string[] action_names =
        {
            "FileSave",
            "FileSaveAs",
            "FileClose",
            "FileCreateTemplate"
        };

        foreach (string action_name in action_names)
        {
            Gtk.Action action = _action_group.get_action (action_name);
            action.sensitive = sensitive;
        }
    }

    /* Gtk.Action callbacks */

    public void on_file_new ()
    {
        new OpenTemplateDialog (_main_window);
    }

    public void on_new_window ()
    {
        LatexilaApp.get_instance ().create_window ();
    }

    public void on_file_open ()
    {
        FileChooserDialog file_chooser = new FileChooserDialog (_("Open Files"),
            _main_window,
            FileChooserAction.OPEN,
            _("_Cancel"), ResponseType.CANCEL,
            _("_Open"), ResponseType.ACCEPT
        );

        // Open in the directory of the current document
        if (_main_window.active_document != null &&
            _main_window.active_document.location != null)
        {
            File location = _main_window.active_document.location;
            string dirname = location.get_parent ().get_path ();
            file_chooser.set_current_folder (dirname);
        }
        else if (_main_window.default_location != null)
            file_chooser.set_current_folder (_main_window.default_location);

        file_chooser.select_multiple = true;

        // Filter: by default show only .tex and .bib files
        FileFilter latex_filter = new FileFilter ();
        latex_filter.set_filter_name (_("All LaTeX Files"));
        latex_filter.add_pattern ("*.tex");
        latex_filter.add_pattern ("*.bib");
        file_chooser.add_filter (latex_filter);

        // All files filter
        FileFilter all_files_filter = new FileFilter ();
        all_files_filter.set_filter_name (_("All Files"));
        all_files_filter.add_pattern ("*");
        file_chooser.add_filter (all_files_filter);

        SList<File> files_to_open = null;
        if (file_chooser.run () == ResponseType.ACCEPT)
            files_to_open = file_chooser.get_files ();

        _main_window.default_location = file_chooser.get_current_folder ();
        file_chooser.destroy ();

        // We open the files after closing the dialog, because open a lot of documents can
        // take some time (this is not async).
        bool jump_to = true;
        foreach (File file in files_to_open)
        {
            _main_window.open_document (file, jump_to);
            jump_to = false;
        }
    }

    public void on_file_save ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.save_document (_main_window.active_document, false);
    }

    public void on_file_save_as ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.save_document (_main_window.active_document, true);
    }

    public void on_create_template ()
    {
        return_if_fail (_main_window.active_tab != null);

        CreateTemplateDialog dialog = new CreateTemplateDialog (_main_window);
        dialog.destroy ();
    }

    public void on_delete_template ()
    {
        DeleteTemplateDialog dialog = new DeleteTemplateDialog (_main_window);
        dialog.destroy ();
    }

    public void on_file_close ()
    {
        return_if_fail (_main_window.active_tab != null);
        _main_window.close_tab (_main_window.active_tab);
    }
}
