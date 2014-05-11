/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2011 Sébastien Wilmet
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

private class LatexPostProcessor : PostProcessor
{
    private enum FilterStatus
    {
        START,
        BADBOX,
        WARNING,
        ERROR,
        ERROR_SEARCH_LINE,
        FILENAME,
        FILENAME_HEURISTIC
    }

    struct FileInStack
    {
        string filename;
        bool reliable;

        // We also push file which doesn't exist, because the corresponding ')' will pop
        // it. If we don't push them, wrong files are poped.
        // Anyway, when a new message is added, we get the last _existing_ file.
        bool exists;
    }

    private const int NO_LINE = -1;

    // the current message
    private BuildMsg msg;

    // if a message is splitted, we enter in a different status, so we fetch the end
    // of the message
    private FilterStatus status = FilterStatus.START;

    // if a message is splitted, the lines are concatenated in this buffer
    private string line_buf;
    private int nb_lines = 0;

    // if a filename is splitted into several lines
    private string filename_buf;

    // the stack containing the files that TeX is processing
    private SList<FileInStack?> stack_files = new SList<FileInStack?> ();

    // the directory where the document is compiled
    private string directory_path = null;

    // for statistics
    private int nb_badboxes = 0;
    private int nb_warnings = 0;
    private int nb_errors = 0;

    /* Regex */
    private static Regex? reg_badbox = null;
    private static Regex? reg_badbox_lines = null;
    private static Regex? reg_badbox_line = null;
    private static Regex? reg_badbox_output = null;

    private static Regex? reg_warning = null;
    private static Regex? reg_warning_no_file = null;
    private static Regex? reg_warning_line = null;
    private static Regex? reg_warning_international_line = null;

    private static Regex? reg_latex_error = null;
    private static Regex? reg_pdflatex_error = null;
    private static Regex? reg_tex_error = null;
    private static Regex? reg_error_line = null;

    private static Regex? reg_file_pop = null;
    private static Regex? reg_other_bytes = null;

    private static Regex? reg_spaces = null;

    public LatexPostProcessor ()
    {
        msg = BuildMsg ();

        if (reg_badbox != null)
            return;

        try
        {
            reg_badbox = new Regex ("^(Over|Under)full \\\\[hv]box");
            reg_badbox_lines = new Regex ("(.*) at lines (\\d+)--(\\d+)");
            reg_badbox_line = new Regex ("(.*) at line (\\d+)");
            reg_badbox_output =
                new Regex ("(.*)has occurred while \\\\output is active");

            string warning_str = "^(((! )?(La|pdf)TeX)|Package|Class)";
            warning_str += "(?P<name>.*) Warning[^:]*:\\s*(?P<contents>.*)";
            reg_warning = new Regex (warning_str,
                RegexCompileFlags.CASELESS | RegexCompileFlags.OPTIMIZE);

            reg_warning_no_file = new Regex ("(No file .*)");
            reg_warning_line = new Regex ("(.*) on input line (\\d+)\\.$");
            reg_warning_international_line = new Regex ("(.*)(\\d+)\\.$");

            reg_latex_error = new Regex ("^! LaTeX Error: (.*)$");
            reg_pdflatex_error = new Regex ("^Error: pdflatex (.*)$");
            reg_tex_error = new Regex ("^! (.*)\\.$");
            reg_error_line = new Regex ("^l\\.(\\d+)(.*)");

            reg_file_pop = new Regex ("(\\) )?:<-$");
            reg_other_bytes = new Regex ("(?P<nb>\\d+) bytes");

            reg_spaces = new Regex ("\\s{2,}");
        }
        catch (RegexError e)
        {
            warning ("LatexPostProcessor: %s", e.message);
        }
    }

    public override void process (File file, string output)
    {
        directory_path = file.get_parent ().get_parse_name ();

        string[] lines = output.split ("\n");

        foreach (string line in lines)
            latex_output_filter (line);

        // Stats.
        // Since all the messages printed by the 'latex' or 'pdflatex' command are in
        // English, it would be strange to have only this one translated.
        msg.text = "%d %s, %d %s, %d %s".printf (
            nb_errors,   nb_errors   == 1 ? "error"   : "errors",
            nb_warnings, nb_warnings == 1 ? "warning" : "warnings",
            nb_badboxes, nb_badboxes == 1 ? "badbox"  : "badboxes");
        msg.type = BuildMsgType.INFO;
        add_msg (false);
    }

    private void latex_output_filter (string line)
    {
        switch (status)
        {
            case FilterStatus.START:
                if (line.length == 0)
                    return;

                if (! (detect_badbox (line)
                       || detect_warning (line)
                       || detect_error (line)
                       || detect_other (line)))
                {
                    update_stack_file (line);
                }
                break;

            case FilterStatus.BADBOX:
                detect_badbox (line);
                break;

            case FilterStatus.WARNING:
                detect_warning (line);
                break;

            case FilterStatus.ERROR:
            case FilterStatus.ERROR_SEARCH_LINE:
                detect_error (line);
                break;

            case FilterStatus.FILENAME:
            case FilterStatus.FILENAME_HEURISTIC:
                update_stack_file (line);
                break;

            default:
                status = FilterStatus.START;
                break;
        }
    }

    private bool detect_badbox (string line)
    {
        switch (status)
        {
            case FilterStatus.START:
                if (! reg_badbox.match (line))
                    return false;

                msg.type = BuildMsgType.BADBOX;

                if (detect_badbox_line (line, false))
                    add_msg ();
                else
                {
                    line_buf = line;
                    nb_lines++;
                }

                return true;

            case FilterStatus.BADBOX:
                line_buf += line;
                nb_lines++;
                if (detect_badbox_line (line_buf, line.length == 0))
                {
                    add_msg ();
                    nb_lines = 0;
                }

                // the return value is not important here
                return true;

            default:
                return false;
        }
    }

    private bool detect_badbox_line (string badbox, bool current_line_is_empty)
    {
        if (reg_badbox_lines.match (badbox))
        {
            status = FilterStatus.START;
            string[] strings = reg_badbox_lines.split (badbox);
            msg.text = strings[1];
            int n1 = int.parse (strings[2]);
            int n2 = int.parse (strings[3]);

            if (n1 <= n2)
            {
                msg.start_line = n1;
                msg.end_line = n2;
            }
            else
            {
                msg.start_line = n2;
                msg.end_line = n1;
            }

            return true;
        }

        else if (reg_badbox_line.match (badbox))
        {
            status = FilterStatus.START;
            string[] strings = reg_badbox_line.split (badbox);
            msg.text = strings[1];
            msg.start_line = int.parse (strings[2]);
            return true;
        }

        else if (reg_badbox_output.match (badbox))
        {
            status = FilterStatus.START;
            string[] strings = reg_badbox_output.split (badbox);
            msg.text = strings[1];
            msg.start_line = NO_LINE;
            return true;
        }

        else if (nb_lines > 4 || current_line_is_empty)
        {
            status = FilterStatus.START;
            msg.text = badbox;
            msg.start_line = NO_LINE;
            return true;
        }

        status = FilterStatus.BADBOX;
        return false;
    }

    private bool detect_warning (string line)
    {
        switch (status)
        {
            case FilterStatus.START:
                MatchInfo match_info;
                if (reg_warning.match (line, 0, out match_info))
                {
                    msg.type = BuildMsgType.WARNING;

                    string contents = match_info.fetch_named ("contents");

                    string name = match_info.fetch_named ("name").strip ();
                    if (name != "")
                        contents = @"$name: $contents";

                    if (detect_warning_line (contents, false))
                        add_msg ();
                    else
                    {
                        line_buf = contents;
                        nb_lines++;
                    }

                    return true;
                }

                else if (reg_warning_no_file.match (line))
                {
                    msg.type = BuildMsgType.WARNING;
                    string[] strings = reg_warning_no_file.split (line);
                    msg.text = strings[1];
                    msg.start_line = NO_LINE;
                    add_msg ();
                    return true;
                }

                return false;

            case FilterStatus.WARNING:
                line_buf += line;
                nb_lines++;
                if (detect_warning_line (line_buf, line.length == 0))
                {
                    add_msg ();
                    nb_lines = 0;
                }

                // the return value is not important here
                return true;

            default:
                return false;
        }
    }

    private bool detect_warning_line (string warning, bool current_line_is_empty)
    {
        if (reg_warning_line.match (warning))
        {
            status = FilterStatus.START;
            string[] strings = reg_warning_line.split (warning);
            msg.text = strings[1];
            msg.start_line = int.parse (strings[2]);
            return true;
        }

        else if (reg_warning_international_line.match (warning))
        {
            status = FilterStatus.START;
            string[] strings = reg_warning_international_line.split (warning);
            msg.text = strings[1];
            msg.start_line = int.parse (strings[2]);
            return true;
        }

        else if (warning[warning.length - 1] == '.')
        {
            status = FilterStatus.START;
            msg.text = warning;
            msg.start_line = NO_LINE;
            return true;
        }

        else if (nb_lines > 5 || current_line_is_empty)
        {
            status = FilterStatus.START;
            msg.text = warning;
            msg.start_line = NO_LINE;
            return true;
        }

        status = FilterStatus.WARNING;
        return false;
    }

    private bool detect_error (string line)
    {
        switch (status)
        {
            case FilterStatus.START:
                bool found = true;
                string tmp = null;

                if (reg_latex_error.match (line))
                {
                    string[] strings = reg_latex_error.split (line);
                    tmp = strings[1];
                }

                else if (reg_pdflatex_error.match (line))
                {
                    string[] strings = reg_pdflatex_error.split (line);
                    tmp = strings[1];
                }

                else if (reg_tex_error.match (line))
                {
                    string[] strings = reg_tex_error.split (line);
                    tmp = strings[1];
                }
                else
                    found = false;

                if (found)
                {
                    nb_lines++;
                    msg.type = BuildMsgType.ERROR;

                    // the message is complete
                    if (line[line.length - 1] == '.')
                    {
                        msg.text = tmp;
                        status = FilterStatus.ERROR_SEARCH_LINE;
                    }
                    // the message is splitted
                    else
                    {
                        line_buf = tmp;
                        status = FilterStatus.ERROR;
                    }
                    return true;
                }

                return false;

            case FilterStatus.ERROR:
                line_buf += line;
                nb_lines++;

                if (line[line.length - 1] == '.')
                {
                    msg.text = line_buf;
                    status = FilterStatus.ERROR_SEARCH_LINE;
                }
                else if (nb_lines > 4)
                {
                    msg.text = line_buf;
                    msg.start_line = NO_LINE;
                    add_msg ();
                    nb_lines = 0;
                    status = FilterStatus.START;
                }

                // the return value is not important here
                return true;

            case FilterStatus.ERROR_SEARCH_LINE:
                nb_lines++;
                if (reg_error_line.match (line))
                {
                    string[] strings = reg_error_line.split (line);
                    msg.start_line = int.parse (strings[1]);
                    add_msg ();
                    nb_lines = 0;
                    status = FilterStatus.START;
                    return true;
                }
                else if (nb_lines > 11)
                {
                    msg.start_line = NO_LINE;
                    add_msg ();
                    nb_lines = 0;
                    status = FilterStatus.START;
                    return true;
                }
                break;

            default:
                break;
        }

        return false;
    }

    private bool detect_other (string line)
    {
        if (! line.contains ("Output written on"))
            return false;

        msg.start_line = NO_LINE;
        msg.type = BuildMsgType.INFO;

        MatchInfo match_info;
        if (! reg_other_bytes.match (line, 0, out match_info))
        {
            msg.text = line;
            add_msg (false);
            return true;
        }

        /* show the file size in a human readable format */
        string? nb_bytes_str = match_info.fetch_named ("nb");
        return_val_if_fail (nb_bytes_str != null, false);

        int64 nb_bytes = int64.parse (nb_bytes_str);
        string human_size = format_size (nb_bytes);

        try
        {
            string new_line = reg_other_bytes.replace_literal (line, -1, 0, human_size);
            msg.text = new_line;
        }

        // nice try!
        catch (RegexError e)
        {
            warning ("LaTeX post processor: %s", e.message);
            msg.text = line;
        }

        add_msg (false);
        return true;
    }

    // There are basically two ways to detect the current file TeX is processing:
    // 1) Use \Input (srctex or srcltx package) and \include exclusively. This will
    //    cause (La)TeX to print the line ":<+ filename" in the log file when opening
    //    a file, ":<-" when closing a file. Filenames pushed on the stack in this mode
    //    are marked as reliable.
    //
    // 2) Since people will probably also use the \input command, we also have to be
    //    to detect the old-fashioned way. TeX prints '(filename' when opening a file
    //    and a ')' when closing one. It is impossible to detect this with 100% certainty
    //    (TeX prints many messages and even text (a context) from the TeX source file,
    //    there could be unbalanced parentheses), so we use an heuristic algorithm.
    //    In heuristic mode a ')' will only be considered as a signal that TeX is closing
    //    a file if the top of the stack is not marked as "reliable".
    //
    // The method used here is almost the same as in Kile.

    private void update_stack_file (string line)
    {
        switch (status)
        {
            case FilterStatus.START:
            case FilterStatus.FILENAME_HEURISTIC:
                // TeX is opening a file
                if (line.has_prefix (":<+ "))
                {
                    filename_buf = line.substring (4).strip ();
                    status = FilterStatus.FILENAME;
                }

                // TeX closed a file
                else if (reg_file_pop.match (line) || line.has_prefix (":<-"))
                    pop_file_from_stack ();

                // fallback to the heuristic detection of filenames
                else
                    update_stack_file_heuristic (line);
                break;

            case FilterStatus.FILENAME:
                // The partial filename was followed by '(', this means that TeX is
                // signalling it is opening the file. We are sure the filename is
                // complete now. Don't call update_stack_file_heuristic()
                // since we don't want the filename on the stack twice.
                if (line[0] == '(' || line.has_prefix ("\\openout"))
                {
                    push_file_on_stack (filename_buf, true);
                    status = FilterStatus.START;
                }

                // The partial filename was followed by a TeX error, meaning the
                // file doesn't exist. Don't push it on the stack, instead try to
                // detect the error.
                else if (line[0] == '!')
                {
                    status = FilterStatus.START;
                    detect_error (line);
                }
                else if (line.has_prefix ("No file"))
                {
                    status = FilterStatus.START;
                    detect_warning (line);
                }

                // the filename is not complete
                else
                    filename_buf += line.strip ();
                break;

            default:
                break;
        }
    }

    private void update_stack_file_heuristic (string line)
    {
        bool expect_filename = status == FilterStatus.FILENAME_HEURISTIC;
        int index = 0;
        long length = line.length;

        // handle special case
        if (expect_filename && length > 0 && line[0] == ')')
        {
            push_file_on_stack (filename_buf, false);
            expect_filename = false;
            status = FilterStatus.START;
        }

        // scan for parentheses and grab filenames
        for (int i = 0 ; i < length ; i++)
        {
            /*
            We're expecting a filename. If a filename really ends at this position,
            one of the following must be true:
                1) Next character is a space, indicating the end of a filename
                  (yes, the path can't have spaces, this is a TeX limitation).
                2) We're at the end of the line, the filename is probably
                   continued on the next line.
                3) The file was closed already, signalled by the ')'.
            */

            bool is_last_char = false;
            bool next_is_terminator = false;

            if (expect_filename)
            {
                is_last_char = i + 1 == length;
                next_is_terminator =
                    is_last_char ? false : (line[i+1].isspace () || line[i+1] == ')');
            }

            if (is_last_char || next_is_terminator)
            {
                filename_buf += line[index : i + 1];

                if (filename_buf.length == 0)
                    continue;

                // by default, an output line is 79 characters max
                if ((is_last_char && i < 78)
                    || next_is_terminator
                    || file_exists (filename_buf))
                {
                    push_file_on_stack (filename_buf, false);
                    expect_filename = false;
                    status = FilterStatus.START;
                }

                // Guess the filename is continued on the next line, only if the
                // current filename does not exist
                else if (is_last_char)
                {
                    if (file_exists (filename_buf))
                    {
                        push_file_on_stack (filename_buf, false);
                        expect_filename = false;
                        status = FilterStatus.START;
                    }
                    else
                        status = FilterStatus.FILENAME_HEURISTIC;
                }

                // filename not detected
                else
                {
                    status = FilterStatus.START;
                    filename_buf = "";
                    expect_filename = false;
                }
            }

            // TeX is opening a file
            else if (line[i] == '(')
            {
                status = FilterStatus.START;
                filename_buf = "";
                // we need to extract the filename
                expect_filename = true;
                // this is where the filename is supposed to start
                index = i + 1;
            }

            // TeX is closing a file
            // If this filename was pushed on the stack by the reliable ":<+-"
            // method, don't pop, a ":<-" will follow. This helps in preventing
            // unbalanced ')' from popping filenames from the stack too soon.
            else if (line[i] == ')' && stack_files.length () > 0
                    && ! top_file_on_stack_is_reliable ())
                pop_file_from_stack ();
        }
    }

    private bool
    file_exists (string filename)
    {
        return get_path_if_file_exists (filename) != null;
    }

    // return null if the filename does not exist
    // return the path of the filename if it exists
    private string? get_path_if_file_exists (string filename)
    {
        if (Path.is_absolute (filename))
        {
            if (FileUtils.test (filename, FileTest.IS_REGULAR))
                return filename;
            else
                return null;
        }

        string full_path;
        if (filename.has_prefix ("./"))
            full_path = Path.build_filename (directory_path, filename.substring (2),
                null);
        else
            full_path = Path.build_filename (directory_path, filename, null);

        if (FileUtils.test (full_path, FileTest.IS_REGULAR))
            return full_path;

        // try to add various extensions on the filename to see if the file exists
        string[] extensions = {".tex", ".ltx", ".latex", ".dtx", ".ins"};
        foreach (string ext in extensions)
        {
            string path_with_ext = full_path + ext;
            if (FileUtils.test (path_with_ext, FileTest.IS_REGULAR))
                return path_with_ext;
        }

        return null;
    }

    // Get last existing file pushed on the stack
    private string? get_current_filename ()
    {
        unowned SList<FileInStack?> stack_file = stack_files;
        while (stack_file != null)
        {
            FileInStack file = stack_file.data;
            if (file.exists)
                return file.filename;

            stack_file = stack_file.next;
        }

        return null;
    }

    private void push_file_on_stack (string filename, bool reliable)
    {
        FileInStack file = FileInStack ();
        file.reliable = reliable;

        // handle special case when a warning message is collapsed
        string clean_filename;
        string bad_suffix = "pdfTeX";
        if (filename.has_suffix (bad_suffix))
            clean_filename = filename[0 : -bad_suffix.length];
        else
            clean_filename = filename;

        string? path = get_path_if_file_exists (clean_filename);
        if (path != null)
        {
            file.filename = path;
            file.exists = true;
        }
        else
        {
            file.filename = clean_filename;
            file.exists = false;
        }

        stack_files.prepend (file);
    }

    private void pop_file_from_stack ()
    {
        if (stack_files == null)
            return;

        stack_files.remove (stack_files.data);
    }

    private bool top_file_on_stack_is_reliable ()
    {
        return_val_if_fail (stack_files != null, true);
        return stack_files.data.reliable;
    }

    private void add_msg (bool set_filename = true)
    {
        // exclude some useless messages here
        if (msg.type == BuildMsgType.WARNING
            && msg.text == "There were undefined references.")
        {
            msg = BuildMsg ();
            return;
        }

        if (set_filename)
            msg.filename = get_current_filename ();

        try
        {
            // A message on several lines are sometimes indented, so when the lines are
            // catenated there are a lot of spaces. We replace these spaces by one space.
            msg.text = reg_spaces.replace (msg.text, -1, 0, " ");
        }
        catch (RegexError e)
        {
            warning ("Latex post processor: %s", e.message);
        }

        switch (msg.type)
        {
            case BuildMsgType.BADBOX:
                nb_badboxes++;
                break;

            case BuildMsgType.WARNING:
                nb_warnings++;
                break;

            case BuildMsgType.ERROR:
                nb_errors++;
                break;
        }

        _all_messages.add (msg);
        msg = BuildMsg ();
    }
}
