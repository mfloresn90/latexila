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
 */

using Gtk;

public class Templates : GLib.Object
{
    private static Templates _instance = null;

    // Contains the default templates (empty, article, report, ...)
    private ListStore _default_store;

    // Contains the personal templates (created by the user)
    private ListStore _personal_store;

    private int _nb_personal_templates;

    // The contents of the personal templates are saved in the user data directory.
    // The first personal template is 0.tex, the second 1.tex, and so on.
    // The names and the icons of the personal templates are saved in an rc file.
    private File _data_dir;
    private File _rc_file;

    // Used for the construction of a default template when reading its XML file.
    private string _default_template;

    private enum TemplateColumn
    {
        PIXBUF,  // the theme icon name
        ICON_ID, // the string stored in the rc file (article, report, ...)
        NAME,
        CONTENTS,
        N_COLUMNS
    }

    /* Templates is a singleton */
    private Templates ()
    {
        _data_dir = File.new_for_path (
            Path.build_filename (Environment.get_user_data_dir (), "latexila"));

        _rc_file = _data_dir.get_child ("templatesrc");

        init_default_templates ();
        init_personal_templates ();
    }

    public static Templates get_default ()
    {
        if (_instance == null)
            _instance = new Templates ();
        return _instance;
    }

    private void init_default_templates ()
    {
        _default_store = create_new_store ();

        add_template_from_string (_default_store, _("Empty"), "empty", "");

        add_default_template (_("Article"), "article", "article.xml");
        add_default_template (_("Report"), "report", "report.xml");
        add_default_template (_("Book"), "book", "book.xml");
        add_default_template (_("Letter"), "letter", "letter.xml");
        add_default_template (_("Presentation"), "beamer", "beamer.xml");
    }

    private void init_personal_templates ()
    {
        _personal_store = create_new_store ();
        _nb_personal_templates = 0;

        // if the rc file doesn't exist, there is no personal template
        if (! _rc_file.query_exists ())
            return;

        // load the key file
        KeyFile key_file = new KeyFile ();
        string[] names;
        string[] icons;

        try
        {
            key_file.load_from_file (_rc_file.get_path (), KeyFileFlags.NONE);

            // get the names and the icons
            names = key_file.get_string_list (Config.PACKAGE_NAME, "names");
            icons = key_file.get_string_list (Config.PACKAGE_NAME, "icons");
        }
        catch (Error e)
        {
            warning ("Load templates failed: %s", e.message);
            return;
        }

        return_if_fail (names.length == icons.length);

        int nb_templates = names.length;

        for (int i = 0 ; i < nb_templates ; i++)
        {
            File file = get_personal_template_file (i);
            if (! file.query_exists ())
            {
                warning ("The template '%s' doesn't exist.", names[i]);
                continue;
            }

            if (add_template_from_file (_personal_store, names[i], icons[i], file))
                _nb_personal_templates++;
        }
    }

    private ListStore create_new_store ()
    {
        return new ListStore (TemplateColumn.N_COLUMNS,
            typeof (string), // pixbuf
            typeof (string), // icon id
            typeof (string), // name
            typeof (string)  // contents
        );
    }

    private File get_personal_template_file (int template_num)
    {
        string filename = "%d.tex".printf (template_num);
        return _data_dir.get_child (filename);
    }


    /*************************************************************************/
    // Add a default template.
    //
    // A default template (or some chunks) can be translated. The templates
    // are stored in XML files. ITS Tool is used to extract the translatable
    // strings. Here the XML files are read, the chunks are joint, and the
    // translatable chunks are translated.

    private void add_default_template (string name, string icon_id, string filename)
    {
        string path = Path.build_filename (Config.DATA_DIR, "templates", filename);
        File file = File.new_for_path (path);

        if (! file.query_exists ())
        {
            warning ("The default template '%s' doesn't exist. Path: %s", name, path);
            return;
        }

        string? contents = Utils.load_file (file);
        return_if_fail (contents != null);

        _default_template = "";

        try
        {
            MarkupParser parser =
                { null, null, parser_text, null, null };
            MarkupParseContext context =
                new MarkupParseContext (parser, 0, this, null);
            context.parse (contents, -1);
        }
        catch (GLib.Error e)
        {
            warning ("Impossible to load the default template '%s': %s",
                name, e.message);
            return;
        }

        add_template_from_string (_default_store, name, icon_id, _default_template);
    }

    private void parser_text (MarkupParseContext context, string text, size_t text_len)
        throws MarkupError
    {
        string chunk = null;

        switch (context.get_element ())
        {
            case "chunk":
                chunk = text;
                break;

            case "translatableChunk":
                chunk = _(text);
                break;

            case "babel":
                string translated_text = _(text);
                if (translated_text != text)
                    chunk = translated_text;
                break;
        }

        if (chunk == null)
            return;

        // Remove the first '\n'. Without this, the XML files would be less
        // well presented.
        if (chunk[0] == '\n')
            chunk = chunk[1 : chunk.length];

        _default_template += chunk;
    }


    /*************************************************************************/
    // Add and delete templates, save rc file.

    private void add_template_from_string (ListStore store, string name,
        string icon_id, string contents)
    {
        TreeIter iter;
        store.append (out iter);
        store.set (iter,
            TemplateColumn.PIXBUF, get_theme_icon (icon_id),
            TemplateColumn.ICON_ID, icon_id,
            TemplateColumn.NAME, name,
            TemplateColumn.CONTENTS, contents);
    }

    // Returns true on success.
    private bool add_template_from_file (ListStore store, string name,
        string icon_id, File file)
    {
        string? contents = Utils.load_file (file);
        if (contents == null)
            return false;

        add_template_from_string (store, name, icon_id, contents);

        return true;
    }

    public void delete_personal_template (TreePath template_path)
    {
        /* Delete the template from the personal store */
        TreeModel model = (TreeModel) _personal_store;
        TreeIter iter;
        model.get_iter (out iter, template_path);
        _personal_store.remove (iter);

        /* Remove the corresponding file */
        int template_num = template_path.get_indices ()[0];
        File template_file = get_personal_template_file (template_num);
        Utils.delete_file (template_file);

        /* Rename the next .tex files */
        for (int i = template_num + 1 ; i < _nb_personal_templates ; i++)
        {
            File file = get_personal_template_file (i);
            File new_file = get_personal_template_file (i-1);
            try
            {
                file.move (new_file, FileCopyFlags.OVERWRITE);
            }
            catch (Error e)
            {
                warning ("Delete personal template, move file failed: %s", e.message);
            }
        }

        _nb_personal_templates--;
    }

    public void create_personal_template (string name, string icon_id, string contents)
    {
        add_template_from_string (_personal_store, name, icon_id, contents);
        _nb_personal_templates++;

        save_rc_file ();

        File file = get_personal_template_file (_nb_personal_templates - 1);
        Utils.save_file (file, contents);
    }

    public void save_rc_file ()
    {
        if (_nb_personal_templates == 0)
        {
            Utils.delete_file (_rc_file);
            return;
        }

        // The names and the icons of all personal templates.
        string[] names = new string[_nb_personal_templates];
        string[] icons = new string[_nb_personal_templates];

        // Traverse the list store.
        TreeIter iter;
        TreeModel model = _personal_store as TreeModel;
        bool valid_iter = model.get_iter_first (out iter);
        int template_num = 0;

        while (valid_iter)
        {
            model.get (iter,
                TemplateColumn.NAME, out names[template_num],
                TemplateColumn.ICON_ID, out icons[template_num]);

            valid_iter = model.iter_next (ref iter);
            template_num++;
        }

        // Contents of the rc file
        KeyFile key_file = new KeyFile ();
        key_file.set_string_list (Config.PACKAGE_NAME, "names", names);
        key_file.set_string_list (Config.PACKAGE_NAME, "icons", icons);

        string key_file_data = key_file.to_data ();

        // Save the rc file
        Utils.save_file (_rc_file, key_file_data);
    }

    // For compatibility reasons. 'icon_id' is the string stored in the rc file,
    // and the return value is the theme icon name used for the pixbuf.
    // If we store directly the theme icon names in the rc file, old rc files must be
    // modified via a script for example, but it's simpler like that.
    private string? get_theme_icon (string icon_id)
    {
        switch (icon_id)
        {
            case "empty":
                return "text-x-preview";

            case "article":
                return "text-x-generic";

            case "report":
                return "x-office-document";

            case "book":
                return "accessories-dictionary";

            case "letter":
                return "emblem-mail";

            case "beamer":
                return "x-office-presentation";

            default:
                return_val_if_reached (null);
        }
    }


    /*************************************************************************/
    // Get templates data: icon id, contents.

    public string get_icon_id (TreePath default_template_path)
    {
        TreeModel model = _default_store as TreeModel;
        TreeIter iter;
        if (! model.get_iter (out iter, default_template_path))
        {
            warning ("Failed to get template icon id");
            return "";
        }

        string icon_id;
        model.get (iter, TemplateColumn.ICON_ID, out icon_id);

        return icon_id;
    }

    public string get_default_template_contents (TreePath path)
    {
        return get_template_contents (_default_store, path);
    }

    public string get_personal_template_contents (TreePath path)
    {
        return get_template_contents (_personal_store, path);
    }

    private string get_template_contents (ListStore store, TreePath path)
    {
        TreeIter iter;
        TreeModel model = store as TreeModel;
        if (! model.get_iter (out iter, path))
        {
            warning ("Failed to get template contents");
            return "";
        }

        string contents;
        model.get (iter, TemplateColumn.CONTENTS, out contents);

        return contents;
    }


    /*************************************************************************/
    // Create templates list for the dialog windows.

    public TreeView get_default_templates_list ()
    {
        return get_templates_list (_default_store);
    }

    public TreeView get_personal_templates_list ()
    {
        return get_templates_list (_personal_store);
    }

    private TreeView get_templates_list (ListStore store)
    {
        TreeView view = new TreeView.with_model (store);
        view.headers_visible = false;
        view.expand = true;

        TreeSelection select = view.get_selection ();
        select.set_mode (SelectionMode.SINGLE);

        // Icon
        CellRendererPixbuf pixbuf_renderer = new CellRendererPixbuf ();
        pixbuf_renderer.stock_size = IconSize.BUTTON;

        TreeViewColumn column = new TreeViewColumn.with_attributes ("Icon",
            pixbuf_renderer, "icon-name", TemplateColumn.PIXBUF, null);

        view.append_column (column);

        // Name
        CellRendererText text_renderer = new CellRendererText ();

        column = new TreeViewColumn.with_attributes ("Name",
            text_renderer, "text", TemplateColumn.NAME, null);

        view.append_column (column);

        return view;
    }
}
