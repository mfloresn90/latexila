/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2010-2011 Sébastien Wilmet
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

public class DocumentView : Gtk.SourceView
{
    public const double SCROLL_MARGIN = 0.02;

    private GLib.Settings _editor_settings;
    private Pango.FontDescription _font_desc;
    private Gspell.Checker? _spell_checker = null;
    private Gspell.InlineCheckerGtv? _inline_spell_checker = null;

    public DocumentView (Document doc)
    {
        this.buffer = doc;

        doc.notify["readonly"].connect ((d, p) =>
        {
            this.editable = ! ((Document) d).readonly;
        });

        wrap_mode = WrapMode.WORD;
        auto_indent = true;
        indent_width = -1;

        /* settings */
        _editor_settings = new GLib.Settings ("org.gnome.latexila.preferences.editor");

        _editor_settings.bind ("forget-no-tabs", this, "smart-backspace",
            SettingsBindFlags.GET);

        set_font_from_settings ();

        // tab width
        uint tmp;
        _editor_settings.get ("tabs-size", "u", out tmp);
        tab_width = tmp;

        insert_spaces_instead_of_tabs = _editor_settings.get_boolean ("insert-spaces");
        show_line_numbers = _editor_settings.get_boolean ("display-line-numbers");
        highlight_current_line = _editor_settings.get_boolean ("highlight-current-line");
        doc.highlight_matching_brackets =
            _editor_settings.get_boolean ("bracket-matching");
        doc.set_style_scheme_from_string (_editor_settings.get_string ("scheme"));
        set_smart_home_end (SourceSmartHomeEndType.AFTER);

        // completion
        try
        {
            CompletionProvider provider = CompletionProvider.get_default ();
            completion.add_provider (provider);
            completion.remember_info_visibility = true;
            completion.show_headers = false;
            completion.auto_complete_delay = 0;
            completion.accelerators = 0;

            hide_completion_calltip_when_needed ();
        }
        catch (GLib.Error e)
        {
            warning ("Completion: %s", e.message);
        }

        // spell checking
        if (_editor_settings.get_boolean ("spell-checking"))
            activate_spell_checking ();

        // forward search
        button_release_event.connect (on_button_release_event);
    }

    public void scroll_to_cursor (double margin = 0.25)
    {
        scroll_to_mark (this.buffer.get_insert (), margin, false, 0, 0);
    }

    public void cut_selection ()
    {
        return_if_fail (this.buffer != null);
        Clipboard clipboard = get_clipboard (Gdk.SELECTION_CLIPBOARD);
        this.buffer.cut_clipboard (clipboard, ! ((Document) this.buffer).readonly);
        scroll_to_cursor (SCROLL_MARGIN);
        grab_focus ();
    }

    public void copy_selection ()
    {
        return_if_fail (this.buffer != null);
        Clipboard clipboard = get_clipboard (Gdk.SELECTION_CLIPBOARD);
        this.buffer.copy_clipboard (clipboard);
        grab_focus ();
    }

    public void my_paste_clipboard ()
    {
        return_if_fail (this.buffer != null);
        Clipboard clipboard = get_clipboard (Gdk.SELECTION_CLIPBOARD);
        this.buffer.paste_clipboard (clipboard, null,
            ! ((Document) this.buffer).readonly);
        scroll_to_cursor (SCROLL_MARGIN);
        grab_focus ();
    }

    public void delete_selection ()
    {
        return_if_fail (this.buffer != null);
        this.buffer.delete_selection (true, ! ((Document) this.buffer).readonly);
        scroll_to_cursor (SCROLL_MARGIN);
    }

    public void my_select_all ()
    {
        return_if_fail (this.buffer != null);
        TextIter start, end;
        this.buffer.get_bounds (out start, out end);
        this.buffer.select_range (start, end);
    }

    public void set_font_from_settings ()
    {
        string font;
        if (_editor_settings.get_boolean ("use-default-font"))
            font = AppSettings.get_default ().system_font;
        else
            font = _editor_settings.get_string ("editor-font");

        set_font_from_string (font);
    }

    public void set_font_from_string (string font)
    {
        _font_desc = Pango.FontDescription.from_string (font);
        override_font (_font_desc);
    }

    public void enlarge_font ()
    {
        // this is not saved in the settings
        _font_desc.set_size (_font_desc.get_size () + Pango.SCALE);
        override_font (_font_desc);
    }

    public void shrink_font ()
    {
        // this is not saved in the settings
        _font_desc.set_size (_font_desc.get_size () - Pango.SCALE);
        override_font (_font_desc);
    }

    public string get_indentation_style ()
    {
        if (insert_spaces_instead_of_tabs)
            return string.nfill (tab_width, ' ');
        return "\t";
    }

    public void activate_spell_checking ()
    {
        if (_spell_checker == null)
        {
            _spell_checker = new Gspell.Checker (null);
        }

        if (_spell_checker.get_language () != null)
        {
            attach_spell_checker ();
            return;
        }

        MessageDialog dialog = new MessageDialog (this.get_toplevel () as Window,
            DialogFlags.DESTROY_WITH_PARENT,
            MessageType.ERROR,
            ButtonsType.NONE,
            "%s", _("No dictionaries available for the spell checking."));

        dialog.add_buttons (_("_Help"), ResponseType.HELP,
            _("_OK"), ResponseType.OK,
            null);

        int response = dialog.run ();

        if (response == ResponseType.HELP)
        {
            try
            {
                show_uri (this.get_screen (), "help:latexila/spell_checking",
                    Gdk.CURRENT_TIME);
            }
            catch (Error e)
            {
                warning ("Impossible to open the documentation: %s", e.message);
            }
        }

        dialog.destroy ();

        _editor_settings.set_boolean ("spell-checking", false);
    }

    private void attach_spell_checker ()
    {
        return_if_fail (_spell_checker != null);

        if (_inline_spell_checker == null)
        {
            _inline_spell_checker =
                new Gspell.InlineCheckerGtv (this.buffer as Gtk.SourceBuffer,
                    _spell_checker);
        }

        _inline_spell_checker.attach_view (this);
    }

    public void disable_spell_checking ()
    {
        if (_inline_spell_checker != null)
        {
            _inline_spell_checker.detach_view (this);
            _inline_spell_checker = null;
        }
    }

    private bool on_button_release_event (Gdk.EventButton event)
    {
        // Forward search on Ctrl + left click
        if (event.button == 1 &&
            Gdk.ModifierType.CONTROL_MASK in event.state)
        {
            Latexila.Synctex synctex = Latexila.Synctex.get_instance ();
            Document doc = this.buffer as Document;
            synctex.forward_search (this.buffer, doc.location, doc.get_main_file (),
                event.time);
        }

        // propagate the event further
        return false;
    }

    private void hide_completion_calltip_when_needed ()
    {
        buffer.notify["cursor-position"].connect (() =>
        {
            CompletionProvider provider = CompletionProvider.get_default ();
            provider.hide_calltip_window ();
        });
    }
}
