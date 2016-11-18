/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2010-2016 Sébastien Wilmet
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

public class DocumentView : Gtef.View
{
    private const string METADATA_ATTRIBUTE_SPELL_LANGUAGE =
        "metadata::latexila-spell-language";
    private const string METADATA_ATTRIBUTE_INLINE_SPELL =
        "metadata::latexila-inline-spell";
    private const string INLINE_SPELL_ENABLED_STR = "1";
    private const string INLINE_SPELL_DISABLED_STR = "0";

    private GLib.Settings _editor_settings;
    private Pango.FontDescription _font_desc;

    private static bool _no_spell_language_dialog_shown = false;

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
        init_spell_checking ();

        // forward search
        button_release_event.connect (on_button_release_event);
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

    /* Spell checking */

    private void init_spell_checking ()
    {
        Gspell.Checker spell_checker = new Gspell.Checker (get_spell_language ());

        unowned Gspell.TextBuffer gspell_buffer =
            Gspell.TextBuffer.get_from_gtk_text_buffer (buffer);
        gspell_buffer.set_spell_checker (spell_checker);

        setup_inline_spell_checker ();

        Gspell.TextView gspell_view =
            Gspell.TextView.get_from_gtk_text_view (this as TextView);
        gspell_view.notify["inline-spell-checking"].connect (inline_checker_enabled_notify_cb);

        Document doc = get_buffer () as Document;

        doc.notify["location"].connect (() =>
        {
            spell_checker.set_language (get_spell_language ());
            setup_inline_spell_checker ();
        });

        _editor_settings.changed["spell-checking-language"].connect (() =>
        {
            spell_checker.set_language (get_spell_language ());
        });

        _editor_settings.changed["highlight-misspelled-words"].connect (() =>
        {
            setup_inline_spell_checker ();
        });
    }

    private unowned Gspell.Language? get_spell_language ()
    {
        Document doc = get_buffer () as Document;

        string? lang_code = doc.get_metadata (METADATA_ATTRIBUTE_SPELL_LANGUAGE);
        if (lang_code == null)
            lang_code = _editor_settings.get_string ("spell-checking-language");

        if (lang_code[0] == '\0')
            return null;

        return Gspell.Language.lookup (lang_code);
    }

    private unowned Gspell.Checker? get_spell_checker ()
    {
        unowned Gspell.TextBuffer gspell_buffer =
            Gspell.TextBuffer.get_from_gtk_text_buffer (buffer);

        return gspell_buffer.get_spell_checker ();
    }

    public void setup_inline_spell_checker ()
    {
        Document doc = get_buffer () as Document;

        bool enabled;

        string? metadata = doc.get_metadata (METADATA_ATTRIBUTE_INLINE_SPELL);
        if (metadata != null)
            enabled = metadata == INLINE_SPELL_ENABLED_STR;
        else
            enabled = _editor_settings.get_boolean ("highlight-misspelled-words");

        Gspell.TextView gspell_view =
            Gspell.TextView.get_from_gtk_text_view (this as TextView);
        gspell_view.inline_spell_checking = enabled;
    }

    public void launch_spell_checker_dialog ()
    {
        Gspell.Navigator navigator = Gspell.NavigatorTextView.new (this as TextView);

        Gspell.CheckerDialog dialog =
            new Gspell.CheckerDialog (this.get_toplevel () as Window, navigator);

        dialog.show ();
    }

    public void launch_spell_language_chooser_dialog ()
    {
        Gspell.Checker? spell_checker = get_spell_checker ();
        return_if_fail (spell_checker != null);

        Gspell.LanguageChooserDialog dialog =
            new Gspell.LanguageChooserDialog (this.get_toplevel () as Window,
                spell_checker.get_language (),
                DialogFlags.DESTROY_WITH_PARENT |
                DialogFlags.MODAL |
                DialogFlags.USE_HEADER_BAR);

        dialog.run ();

        unowned Gspell.Language? lang = dialog.get_language ();
        spell_checker.set_language (lang);

        dialog.destroy ();
    }

    public void save_spell_language_metadata ()
    {
        Gspell.Checker? spell_checker = get_spell_checker ();
        return_if_fail (spell_checker != null);

        Document doc = get_buffer () as Document;

        unowned Gspell.Language? lang = spell_checker.get_language ();
        if (lang != null)
            doc.set_metadata (METADATA_ATTRIBUTE_SPELL_LANGUAGE, lang.get_code ());
        else
            doc.set_metadata (METADATA_ATTRIBUTE_SPELL_LANGUAGE, null);
    }

    public void save_inline_spell_metadata ()
    {
        Document doc = get_buffer () as Document;

        Gspell.TextView gspell_view =
            Gspell.TextView.get_from_gtk_text_view (this as TextView);

        if (gspell_view.inline_spell_checking)
        {
            doc.set_metadata (METADATA_ATTRIBUTE_INLINE_SPELL,
                INLINE_SPELL_ENABLED_STR);
        }
        else
        {
            doc.set_metadata (METADATA_ATTRIBUTE_INLINE_SPELL,
                INLINE_SPELL_DISABLED_STR);
        }
    }

    private void inline_checker_enabled_notify_cb ()
    {
        Gspell.TextView gspell_view =
            Gspell.TextView.get_from_gtk_text_view (this as TextView);
        if (! gspell_view.inline_spell_checking)
            return;

        Gspell.Checker? spell_checker = get_spell_checker ();
        return_if_fail (spell_checker != null);

        if (spell_checker.get_language () != null)
            return;

        gspell_view.inline_spell_checking = false;

        if (_no_spell_language_dialog_shown)
            return;

        _no_spell_language_dialog_shown = true;

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
    }
}
