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

using Gtk;

private struct CmdLineData
{
    bool new_document;
    bool new_window;
    Variant? files_to_open;
}

private void init_i18n ()
{
    Intl.bindtextdomain (Config.GETTEXT_PACKAGE, Config.LOCALE_DIR);
    Intl.bind_textdomain_codeset (Config.GETTEXT_PACKAGE, "UTF-8");
    Intl.textdomain (Config.GETTEXT_PACKAGE);
}

// Put this here because inside a function is not possible...
[CCode (array_length = false, array_null_terminated = true)]
string[] files_list;

private CmdLineData parse_cmd_line_options (string[] args)
{
    bool show_version = false;
    CmdLineData data = CmdLineData ();

    /* Definition of the options */
    OptionEntry[] options = new OptionEntry[5];

    options[0] = { "version", 'V', 0, OptionArg.NONE, ref show_version,
        N_("Show the application's version"), null };

    options[1] = { "new-document", 'n', 0, OptionArg.NONE, ref data.new_document,
        N_("Create new document"), null };

    options[2] = { "new-window", 0, 0, OptionArg.NONE, ref data.new_window,
        N_("Create a new top-level window in an existing instance of LaTeXila"), null };

    options[3] = { "", 0, 0, OptionArg.FILENAME_ARRAY, ref files_list,
        null, "[FILE...]" };

    options[4] = { null };

    /* Parse the command line and extract data */
    OptionContext context =
        new OptionContext (_("- Integrated LaTeX Environment for GNOME"));
    context.add_main_entries (options, Config.GETTEXT_PACKAGE);
    context.add_group (Gtk.get_option_group (false));

    try
    {
        context.parse (ref args);
    }
    catch (OptionError e)
    {
        warning ("%s", e.message);
        stderr.printf (
            _("Run '%s --help' to see a full list of available command line options.\n"),
            args[0]);

        Process.exit (1);
    }

    if (show_version)
    {
        stdout.printf ("%s %s\n", Config.PACKAGE_NAME, Config.PACKAGE_VERSION);
        Process.exit (0);
    }

    if (files_list.length == 0)
        data.files_to_open = null;
    else
    {
        string[] uris = {};
        foreach (string filename in files_list)
        {
            // Call File.new_for_commandline_arg() here (and not in the LatexilaApp class)
            // because relative path resolution needs the right current working directory,
            // which can be different for the primary instance.
            File file = File.new_for_commandline_arg (filename);
            uris += file.get_uri ();
        }

        data.files_to_open = new Variant.strv (uris);
    }

    return data;
}

int main (string[] args)
{
    init_i18n ();

    CmdLineData data = parse_cmd_line_options (args);

    LatexilaApp app = new LatexilaApp ();

    try
    {
        app.register ();
    }
    catch (Error e)
    {
        error ("Failed to register the application: %s", e.message);
    }

    if (data.new_window)
        app.activate_action ("new-window", null);

    if (data.files_to_open != null)
        app.activate_action ("open-files", data.files_to_open);

    if (data.new_document)
        app.activate_action ("new-document", null);

    return app.run ();
}
