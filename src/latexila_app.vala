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

public class LatexilaApp : Gtk.Application
{
    static Gtk.CssProvider? _provider = null;
    private bool _activate_called = false;

    private const GLib.ActionEntry[] _app_actions =
    {
        { "new-document", new_document_cb },
        { "new-window", new_window_cb },
        { "preferences", preferences_cb },
        { "manage-build-tools", manage_build_tools_cb },
        { "help", help_cb },
        { "fundraising", fundraising_cb },
        { "about", about_cb },
        { "quit", quit_cb }
    };

    public LatexilaApp ()
    {
        Object (application_id: "org.gnome.latexila");
        set_flags (ApplicationFlags.HANDLES_OPEN);
        Environment.set_application_name (Config.PACKAGE_NAME);

        setup_main_option_entries ();

        startup.connect (startup_cb);
        activate.connect (activate_cb);
        open.connect (open_documents);
        shutdown.connect (shutdown_cb);
    }

    public static LatexilaApp get_instance ()
    {
        return GLib.Application.get_default () as LatexilaApp;
    }

    public MainWindow? get_active_main_window ()
    {
        foreach (Gtk.Window window in get_windows ())
        {
            if (window is MainWindow)
                return window as MainWindow;
        }

        return null;
    }

    private void setup_main_option_entries ()
    {
        bool show_version = false;
        bool new_document = false;
        bool new_window = false;

        OptionEntry[] options = new OptionEntry[4];

        options[0] = { "version", 'V', 0, OptionArg.NONE, ref show_version,
            N_("Show the application's version"), null };

        options[1] = { "new-document", 'n', 0, OptionArg.NONE, ref new_document,
            N_("Create new document"), null };

        options[2] = { "new-window", 0, 0, OptionArg.NONE, ref new_window,
            N_("Create a new top-level window in an existing instance of LaTeXila"), null };

        options[3] = { null };

        add_main_option_entries (options);

        handle_local_options.connect (() =>
        {
            if (show_version)
            {
                stdout.printf ("%s %s\n", Config.PACKAGE_NAME, Config.PACKAGE_VERSION);
                return 0;
            }

            try
            {
                register ();
            }
            catch (Error e)
            {
                error ("Failed to register the application: %s", e.message);
            }

            if (new_window)
                activate_action ("new-window", null);

            if (new_document)
                activate_action ("new-document", null);

            return -1;
        });
    }

    private void startup_cb ()
    {
        hold ();

        add_action_entries (_app_actions, this);

        GLib.MenuModel manual_app_menu = get_menu_by_id ("manual-app-menu");
        if (manual_app_menu == null)
            warning ("manual-app-menu not available.");

        // The menubar contains everything, so we don't need the fallback app
        // menu on desktops that don't support app menus (e.g. on Xfce).
        if (prefers_app_menu ())
        {
            set_app_menu (manual_app_menu);
        }

        set_application_icons ();
        Latexila.utils_register_icons ();
        StockIcons.register_stock_icons ();
        setup_theme_extensions ();
        AppSettings.get_default ();
        support_backward_search ();
        Gtk.AccelMap.load (get_accel_filename ());

        release ();
    }

    private void activate_cb ()
    {
        hold ();

        if (! _activate_called)
        {
            _activate_called = true;
            create_window ();
            reopen_files ();
        }
        else
            active_window.present ();

        release ();
    }

    private void shutdown_cb ()
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
    }

    private void new_document_cb ()
    {
        if (! _activate_called)
            activate ();

        MainWindow? window = get_active_main_window ();
        return_if_fail (window != null);
        window.create_tab (true);
    }

    private void new_window_cb ()
    {
        if (_activate_called)
            create_window ();
        else
            activate ();
    }

    private void preferences_cb ()
    {
        PreferencesDialog.show_me (get_active_main_window ());
    }

    private void manage_build_tools_cb ()
    {
        new BuildToolsPreferences (get_active_main_window ());
    }

    private void help_cb ()
    {
        Gdk.Screen? screen = null;
        MainWindow? window = get_active_main_window ();
        if (window != null)
            screen = window.get_screen ();

        try
        {
            Gtk.show_uri (screen, "help:latexila", Gdk.CURRENT_TIME);
        }
        catch (Error e)
        {
            warning ("Impossible to open the documentation: %s", e.message);
        }
    }

    private void fundraising_cb ()
    {
        Finance.show_dialog (get_active_main_window (), false);
    }

    private void about_cb ()
    {
        string comments =
            _("LaTeXila is an Integrated LaTeX Environment for the GNOME Desktop");
        string copyright = "Copyright 2009-2017 – Sébastien Wilmet";

        string website = "https://wiki.gnome.org/Apps/LaTeXila";

        string[] authors =
        {
            "Sébastien Wilmet <swilmet@gnome.org>",
            null
        };

        string[] artists =
        {
            "Eric Forgeot <e.forgeot@laposte.net>",
            "Sébastien Wilmet <swilmet@gnome.org>",
            "Alexander Wilms <f.alexander.wilms@gmail.com>",
            "The Kile Team http://kile.sourceforge.net/",
            "Gedit LaTeX Plugin https://wiki.gnome.org/Apps/Gedit/LaTeXPlugin",
            null
        };

        Gdk.Pixbuf logo = null;
        try
        {
            logo = new Gdk.Pixbuf.from_file (Config.DATA_DIR + "/images/app/logo.png");
        }
        catch (Error e)
        {
            warning ("Logo: %s", e.message);
        }

        Gtk.show_about_dialog (get_active_main_window (),
            "program-name", "LaTeXila",
            "version", Config.PACKAGE_VERSION,
            "authors", authors,
            "artists", artists,
            "comments", comments,
            "copyright", copyright,
            "license-type", Gtk.License.GPL_3_0,
            "title", _("About LaTeXila"),
            "translator-credits", _("translator-credits"),
            "website", website,
            "logo", logo
        );
    }

    private void quit_cb ()
    {
        hold ();

        bool cont = true;
        while (cont)
        {
            MainWindow? main_window = get_active_main_window ();
            if (main_window == null)
                break;

            main_window.present ();
            cont = main_window.quit ();
        }

        if (cont)
        {
            while (this.active_window != null)
                this.active_window.destroy ();
        }

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
            if (window is MainWindow)
            {
                MainWindow main_window = window as MainWindow;
                all_documents.add_all (main_window.get_documents ());
            }
        }

        return all_documents;
    }

    // Get all the document views.
    public Gee.List<DocumentView> get_views ()
    {
        Gee.List<DocumentView> all_views = new Gee.LinkedList<DocumentView> ();
        foreach (Gtk.Window window in get_windows ())
        {
            if (window is MainWindow)
            {
                MainWindow main_window = window as MainWindow;
                all_views.add_all (main_window.get_views ());
            }
        }

        return all_views;
    }

    public MainWindow create_window ()
    {
        MainWindow? main_window = get_active_main_window ();
        if (main_window != null)
            main_window.save_state ();

        return new MainWindow (this);
    }

    public void open_documents (File[] files)
    {
        if (! _activate_called)
            activate ();

        MainWindow? main_window = get_active_main_window ();
        return_if_fail (main_window != null);

        bool jump_to = true;
        foreach (File file in files)
        {
            main_window.open_document (file, jump_to);
            jump_to = false;
        }

        main_window.present ();
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

            // TODO choose the right MainWindow, if tex_file is already opened
            // in another window.
            MainWindow? main_window = get_active_main_window ();
            if (main_window != null)
            {
                main_window.jump_to_file_position (tex_file, line, line);
                main_window.present_with_time (timestamp);
            }
        });
    }
}
