/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2010-2012 Sébastien Wilmet
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

public class LatexilaApp : Gtk.Application
{
    static Gtk.CssProvider? _provider = null;

    public LatexilaApp ()
    {
        Object (application_id: "org.gnome.latexila");
        Environment.set_application_name (Config.PACKAGE_NAME);

        connect_signals ();
        add_actions ();
    }

    private void connect_signals ()
    {
        startup.connect (init_primary_instance);

        activate.connect (() =>
        {
            hold ();
            active_window.present ();
            release ();
        });

        shutdown.connect (() =>
        {
            hold ();

            Projects.get_default ().save ();
            MostUsedSymbols.get_default ().save ();

            /* Save accel file */
            string accel_filename = get_accel_filename ();
            File accel_file = File.new_for_path (accel_filename);
            try
            {
                Latexila.utils_create_parent_directories (accel_file);
                Gtk.AccelMap.save (accel_filename);
            }
            catch (Error error)
            {
                warning ("Error when saving accel file: %s", error.message);
            }

            release ();
        });
    }

    private void add_actions ()
    {
        /* New document */
        SimpleAction new_document_action = new SimpleAction ("new-document", null);
        add_action (new_document_action);

        new_document_action.activate.connect (() =>
        {
            hold ();
            MainWindow window = active_window as MainWindow;
            window.create_tab (true);
            release ();
        });

        /* New window */
        SimpleAction new_window_action = new SimpleAction ("new-window", null);
        add_action (new_window_action);

        new_window_action.activate.connect (() =>
        {
            hold ();
            create_window ();
            release ();
        });

        /* Open files */
        VariantType strings_array = new VariantType ("as");
        SimpleAction open_files_action = new SimpleAction ("open-files", strings_array);
        add_action (open_files_action);

        open_files_action.activate.connect ((param) =>
        {
            string[] uris = param.dup_strv ();
            File[] files = {};

            foreach (string uri in uris)
            {
                if (0 < uri.length)
                    files += File.new_for_uri (uri);
            }

            open_documents (files);
        });

        /* Preferences */
        SimpleAction preferences_action = new SimpleAction ("preferences", null);
        add_action (preferences_action);

        preferences_action.activate.connect (() =>
        {
            hold ();
            PreferencesDialog.show_me (this.active_window);
            release ();
        });

        /* Manage build tools */
        SimpleAction build_tools_action = new SimpleAction ("manage-build-tools", null);
        add_action (build_tools_action);

        build_tools_action.activate.connect (() =>
        {
            hold ();
            new BuildToolsPreferences (this.active_window);
            release ();
        });

        /* Help */
        SimpleAction help_action = new SimpleAction ("help", null);
        add_action (help_action);

        help_action.activate.connect (() =>
        {
            hold ();

            try
            {
                Gtk.show_uri (this.active_window.get_screen (), "help:latexila",
                    Gdk.CURRENT_TIME);
            }
            catch (Error e)
            {
                warning ("Impossible to open the documentation: %s", e.message);
            }

            release ();
        });

        /* Fundraiser */
        SimpleAction fundraiser_action = new SimpleAction ("fundraiser", null);
        add_action (fundraiser_action);

        fundraiser_action.activate.connect (() =>
        {
            hold ();
            Finance.show_dialog (this.active_window, false);
            release ();
        });
    }

    public static LatexilaApp get_instance ()
    {
        return GLib.Application.get_default () as LatexilaApp;
    }

    private void init_primary_instance ()
    {
        hold ();
        set_application_icons ();
        Latexila.utils_register_icons ();
        StockIcons.register_stock_icons ();
        setup_theme_extensions ();

        AppSettings.get_default ();
        create_window ();
        reopen_files ();
        Gtk.AccelMap.load (get_accel_filename ());
        support_backward_search ();
        release ();
    }

    private void set_application_icons ()
    {
        string[] sizes = {"16x16", "22x22", "24x24", "32x32", "48x48"};

        List<Gdk.Pixbuf> list = null;

        foreach (string size in sizes)
        {
            string filename = Path.build_filename (Config.ICONS_DIR, size,
                "apps", "latexila.png");

            try
            {
                list.append (new Gdk.Pixbuf.from_file (filename));
            }
            catch (Error e)
            {
                warning ("Application icon: %s", e.message);
            }
        }

        Gtk.Window.set_default_icon_list ((owned) list);
    }

    private void setup_theme_extensions ()
    {
        Gtk.Settings settings = Gtk.Settings.get_default ();
        settings.notify["gtk-theme-name"].connect (update_theme);
        update_theme ();
    }

    private void update_theme ()
    {
        Gtk.Settings settings = Gtk.Settings.get_default ();
        Gdk.Screen screen = Gdk.Screen.get_default ();

        if (settings.gtk_theme_name == "Adwaita")
        {
            if (_provider == null)
            {
                _provider = new Gtk.CssProvider ();
                File file = File.new_for_uri ("resource:///org/gnome/latexila/ui/latexila.adwaita.css");
                try
                {
                    _provider.load_from_file (file);
                }
                catch (Error e)
                {
                    warning ("Cannot load CSS: %s", e.message);
                }
            }

            Gtk.StyleContext.add_provider_for_screen (screen, _provider,
                Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
        }
        else if (_provider != null)
        {
            Gtk.StyleContext.remove_provider_for_screen (screen, _provider);
            _provider = null;
        }
    }

    private void reopen_files ()
    {
        GLib.Settings editor_settings =
            new GLib.Settings ("org.gnome.latexila.preferences.editor");

        if (editor_settings.get_boolean ("reopen-files"))
        {
            GLib.Settings window_settings =
                new GLib.Settings ("org.gnome.latexila.state.window");

            string[] uris = window_settings.get_strv ("documents");
            File[] files = {};
            foreach (string uri in uris)
            {
                if (0 < uri.length)
                    files += File.new_for_uri (uri);
            }

            open_documents (files);
        }
    }

    // Get all the documents currently opened.
    public Gee.List<Document> get_documents ()
    {
        Gee.List<Document> all_documents = new Gee.LinkedList<Document> ();
        foreach (Gtk.Window window in get_windows ())
        {
            MainWindow main_window = window as MainWindow;
            all_documents.add_all (main_window.get_documents ());
        }

        return all_documents;
    }

    // Get all the document views.
    public Gee.List<DocumentView> get_views ()
    {
        Gee.List<DocumentView> all_views = new Gee.LinkedList<DocumentView> ();
        foreach (Gtk.Window window in get_windows ())
        {
            MainWindow main_window = window as MainWindow;
            all_views.add_all (main_window.get_views ());
        }

        return all_views;
    }

    public MainWindow create_window ()
    {
        if (active_window != null)
        {
            MainWindow window = active_window as MainWindow;
            window.save_state ();
        }

        return new MainWindow (this);
    }

    public void open_documents (File[] files)
    {
        bool jump_to = true;
        foreach (File file in files)
        {
            MainWindow window = active_window as MainWindow;
            window.open_document (file, jump_to);
            jump_to = false;
        }
    }

    private string get_accel_filename ()
    {
        return Path.build_filename (Environment.get_user_config_dir (),
            "latexila", "accels");
    }

    private void support_backward_search ()
    {
        Latexila.Synctex synctex = Latexila.Synctex.get_instance ();

        synctex.backward_search.connect ((tex_uri, line, timestamp) =>
        {
            File tex_file = File.new_for_uri (tex_uri);
            if (! tex_file.query_exists ())
            {
                warning (@"Backward search: the file \"$tex_uri\" doesn't exist.");
                return;
            }

            MainWindow main_window = active_window as MainWindow;
            main_window.jump_to_file_position (tex_file, line, line + 1);
            main_window.present_with_time (timestamp);
        });
    }
}
