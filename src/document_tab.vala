/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2010, 2011, 2017 Sébastien Wilmet
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

public class DocumentTab : Tepl.Tab
{
    public DocumentView document_view
    {
        get { return get_view () as DocumentView; }
    }

    public Document document
    {
        get { return get_buffer () as Document; }
    }

    private TabLabel _label;

    private bool ask_if_externally_modified = false;

    private uint auto_save_timeout;

    private uint _auto_save_interval;
    public uint auto_save_interval
    {
        get
        {
            return _auto_save_interval;
        }

        set
        {
            return_if_fail (value > 0);

            if (_auto_save_interval == value)
                return;

            _auto_save_interval = value;

            if (! _auto_save)
                return;

            if (auto_save_timeout > 0)
            {
                return_if_fail (document.location != null);
                return_if_fail (! document.readonly);
                remove_auto_save_timeout ();
                install_auto_save_timeout ();
            }
        }
    }

    private bool _auto_save;
    public bool auto_save
    {
        get
        {
            return _auto_save;
        }

        set
        {
            if (value == _auto_save)
                return;

            _auto_save = value;

            if (_auto_save && auto_save_timeout <= 0 && document.location != null
                && ! document.readonly)
            {
                install_auto_save_timeout ();
                return;
            }

            if (! _auto_save && auto_save_timeout > 0)
            {
                remove_auto_save_timeout ();
                return;
            }

            return_if_fail ((! _auto_save && auto_save_timeout <= 0)
                || document.location == null || document.readonly);
        }
    }

    public DocumentTab ()
    {
        DocumentView document_view = new DocumentView (new Document ());
        Object (view: document_view);
        initialize ();
    }

    public DocumentTab.from_location (File location)
    {
        this ();
        document.load (location);
    }

    public DocumentTab.with_view (DocumentView document_view)
    {
        Object (view: document_view);
        initialize ();
    }

    private void initialize ()
    {
        document.tab = this;

        document_view.focus_in_event.connect (view_focused_in);

        view.show_all ();

        _label = new TabLabel (this);
        _label.show ();

        /* auto save */
        GLib.Settings settings =
            new GLib.Settings ("org.gnome.latexila.preferences.editor");
        auto_save = settings.get_boolean ("auto-save");
        uint tmp;
        settings.get ("auto-save-interval", "u", out tmp);
        auto_save_interval = tmp;

        install_auto_save_timeout_if_needed ();

        document.notify["location"].connect (() =>
        {
            if (auto_save_timeout <= 0)
                install_auto_save_timeout_if_needed ();
        });
    }

    public TabLabel get_label ()
    {
        return _label;
    }

    public string get_menu_tip ()
    {
        return _("Activate '%s'").printf (document.get_uri_for_display ());
    }

    private bool view_focused_in ()
    {
        /* check if the document has been externally modified */

        // we already asked the user
        if (ask_if_externally_modified)
            return false;

        // if file was never saved or is remote we do not check
        if (! get_buffer ().get_file ().is_local ())
            return false;

        if (document.is_externally_modified ())
        {
            ask_if_externally_modified = true;

            string primary_msg = _("The file %s changed on disk.")
                .printf (document.location.get_parse_name ());

            string secondary_msg;
            if (document.get_modified ())
                secondary_msg = _("Do you want to drop your changes and reload the file?");
            else
                secondary_msg = _("Do you want to reload the file?");

            Tepl.InfoBar infobar = new Tepl.InfoBar.simple (MessageType.WARNING,
                primary_msg, secondary_msg);
            infobar.add_button (_("_Reload"), ResponseType.OK);
            infobar.add_button (_("_Cancel"), ResponseType.CANCEL);
            add_info_bar (infobar);
            infobar.show ();

            infobar.response.connect ((response_id) =>
            {
                if (response_id == ResponseType.OK)
                {
                    document.load (document.location);
                    ask_if_externally_modified = false;
                }

                infobar.destroy ();
                document_view.grab_focus ();
            });
        }

        return false;
    }

    private void install_auto_save_timeout ()
    {
        return_if_fail (auto_save_timeout <= 0);
        return_if_fail (auto_save);
        return_if_fail (auto_save_interval > 0);

        auto_save_timeout = Timeout.add_seconds (auto_save_interval * 60, on_auto_save);
    }

    private bool install_auto_save_timeout_if_needed ()
    {
        return_val_if_fail (auto_save_timeout <= 0, false);

        if (auto_save && document.location != null && ! document.readonly)
        {
            install_auto_save_timeout ();
            return true;
        }

        return false;
    }

    private void remove_auto_save_timeout ()
    {
        return_if_fail (auto_save_timeout > 0);

        Source.remove (auto_save_timeout);
        auto_save_timeout = 0;
    }

    private bool on_auto_save ()
    {
        return_val_if_fail (document.location != null, false);
        return_val_if_fail (! document.readonly, false);
        return_val_if_fail (auto_save_timeout > 0, false);
        return_val_if_fail (auto_save, false);
        return_val_if_fail (auto_save_interval > 0, false);

        if (document.get_modified ())
            document.save ();

        return true;
    }
}
