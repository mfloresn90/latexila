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

public enum PostProcessorType
{
    ALL_OUTPUT = 0,
    LATEX,
    LATEXMK,
    NO_OUTPUT,
    N_POST_PROCESSORS
}

public struct BuildJob
{
    PostProcessorType post_processor;
    string command;
}

public struct BuildTool
{
    string _description;
    string extensions;
    string label;
    string icon;
    string files_to_open;
    bool enabled;
    Gee.ArrayList<BuildJob?> jobs;

    // The ID is used only by the default build tools.
    // It is used to save those that are enabled or disabled.
    int id;

    public BuildTool ()
    {
        _description = "";
        extensions = "";
        label = "";
        icon = "";
        files_to_open = "";
        enabled = false;
        jobs = new Gee.ArrayList<BuildJob?> ();
        id = 0;
    }

    public bool has_jobs ()
    {
        return jobs.size > 0;
    }

    public void set_description (string description)
    {
        _description = description;
    }

    public string get_description ()
    {
        if (_description == null || _description == "")
            return label;

        return _description;
    }
}

public abstract class BuildTools : GLib.Object
{
    private static string[] _post_processor_names =
    {
        // Same order as the PostProcessorType enum.
        "all-output",
        "latex",
        "latexmk",
        "no-output"
    };

    protected Gee.LinkedList<BuildTool?> _build_tools;
    protected File _xml_file = null;

    // Used during the XML file parsing to load the build tools.
    private BuildTool _cur_tool;
    private BuildJob _cur_job;

    public signal void modified ();

    public BuildTool? get_build_tool (int tool_num)
    {
        return_val_if_fail (is_valid_index (tool_num), null);

        return _build_tools[tool_num];
    }

    // Support the foreach loop
    public Gee.Iterator<BuildTool?> iterator ()
    {
        return _build_tools.iterator ();
    }

    public bool is_empty ()
    {
        return _build_tools.size == 0;
    }

    public void set_enabled (int tool_num, bool enabled)
    {
        return_if_fail (is_valid_index (tool_num));

        BuildTool tool = _build_tools[tool_num];
        if (tool.enabled != enabled)
        {
            tool.enabled = enabled;
            _build_tools[tool_num] = tool;
            modified ();
        }
    }

    public static PostProcessorType? get_post_processor_type_from_name (string name)
    {
        for (int type = 0 ; type < PostProcessorType.N_POST_PROCESSORS ; type++)
        {
            if (_post_processor_names[type] == name)
                return (PostProcessorType) type;
        }

        return null;
    }

    public static string? get_post_processor_name_from_type (PostProcessorType type)
    {
        return_val_if_fail (type != PostProcessorType.N_POST_PROCESSORS, null);

        return _post_processor_names[type];
    }

    protected bool is_valid_index (int tool_num)
    {
        return 0 <= tool_num && tool_num < _build_tools.size;
    }

    protected void load ()
    {
        _build_tools = new Gee.LinkedList<BuildTool?> ();

        return_if_fail (_xml_file != null);

        if (! _xml_file.query_exists ())
            return;

        string? contents = Utils.load_file (_xml_file);
        if (contents == null)
            return;

        try
        {
            MarkupParser parser =
                { parser_start, parser_end, parser_text, null, null };
            MarkupParseContext context =
                new MarkupParseContext (parser, 0, this, null);
            context.parse (contents, -1);
        }
        catch (GLib.Error e)
        {
            warning ("Impossible to load build tools: %s", e.message);
        }
    }

    private void parser_start (MarkupParseContext context, string name,
        string[] attr_names, string[] attr_values) throws MarkupError
    {
        switch (name)
        {
            case "tools":
            case "label":
            case "description":
            case "open":
                return;

            case "tool":
                _cur_tool = BuildTool ();

                for (int i = 0 ; i < attr_names.length ; i++)
                {
                    switch (attr_names[i])
                    {
                        case "id":
                            _cur_tool.id = int.parse (attr_values[i]);
                            break;

                        // 'show' was the previous name of 'enabled'
                        case "show":
                        case "enabled":
                            _cur_tool.enabled = bool.parse (attr_values[i]);
                            break;

                        case "extensions":
                            _cur_tool.extensions = attr_values[i];
                            break;

                        case "icon":
                            _cur_tool.icon = attr_values[i];
                            break;

                        default:
                            throw new MarkupError.UNKNOWN_ATTRIBUTE (
                                "unknown attribute \"" + attr_names[i] + "\"");
                    }
                }
                break;

            case "job":
                _cur_job = BuildJob ();
                for (int i = 0 ; i < attr_names.length ; i++)
                {
                    switch (attr_names[i])
                    {
                        case "postProcessor":
                            PostProcessorType? post_processor_type =
                                get_post_processor_type_from_name (attr_values[i]);

                            if (post_processor_type == null)
                                throw new MarkupError.INVALID_CONTENT (
                                    "unknown post processor \"" + attr_values[i] + "\"");

                            _cur_job.post_processor = post_processor_type;
                            break;

                        // for compatibility (no longer used)
                        case "mustSucceed":
                            break;

                        default:
                            throw new MarkupError.UNKNOWN_ATTRIBUTE (
                                "unknown attribute \"" + attr_names[i] + "\"");
                    }
                }
                break;

            default:
                throw new MarkupError.UNKNOWN_ELEMENT (
                    "unknown element \"" + name + "\"");
        }
    }

    private void parser_end (MarkupParseContext context, string name) throws MarkupError
    {
        switch (name)
        {
            case "tools":
            case "label":
            case "description":
            case "open":
                return;

            case "tool":
                _build_tools.add (_cur_tool);
                break;

            case "job":
                _cur_tool.jobs.add (_cur_job);
                break;

            default:
                throw new MarkupError.UNKNOWN_ELEMENT (
                    "unknown element \"" + name + "\"");
        }
    }

    private void parser_text (MarkupParseContext context, string text, size_t text_len)
        throws MarkupError
    {
        switch (context.get_element ())
        {
            case "job":
                _cur_job.command = text.strip ();
                break;

            case "label":
                _cur_tool.label = _(text.strip ());
                break;

            case "description":
                _cur_tool.set_description (_(text.strip ()));
                break;

            case "open":
                _cur_tool.files_to_open = text.strip ();
                break;
        }
    }
}

public class DefaultBuildTools : BuildTools
{
    private static DefaultBuildTools _instance = null;

    private DefaultBuildTools ()
    {
        string path = Path.build_filename (Config.DATA_DIR, "build_tools.xml");
        _xml_file = File.new_for_path (path);

        load ();
        load_enable_setting ();
        modified.connect (save_enable_setting);
    }

    public static DefaultBuildTools get_default ()
    {
        if (_instance == null)
            _instance = new DefaultBuildTools ();

        return _instance;
    }

    // Enable or disable the build tools.
    // There are two lists: the enabled build tools IDs, and the disabled build tools IDs.
    // By default, the two lists are empty. If an ID is in a list, it will erase the
    // default value found in the XML file. So when a new default build tool is added,
    // it is not present in the lists, and it automatically gets the default value from
    // the XML file.
    private void load_enable_setting ()
    {
        GLib.Settings settings =
            new GLib.Settings ("org.gnome.latexila.preferences.latex");

        /* Get the enabled build tool IDs */

        int[] enabled_tool_ids = {};

        Variant enabled_tools = settings.get_value ("enabled-default-build-tools");
        VariantIter iter;
        enabled_tools.get ("ai", out iter);

        int enabled_tool_id;
        while (iter.next ("i", out enabled_tool_id))
            enabled_tool_ids += enabled_tool_id;

        /* Enable the build tools to enable */

        int tool_num = 0;
        foreach (BuildTool build_tool in _build_tools)
        {
            if (build_tool.id in enabled_tool_ids)
                set_enabled (tool_num, true);

            tool_num++;
        }

        /* Get the disabled build tool IDs */

        int[] disabled_tool_ids = {};

        Variant disabled_tools = settings.get_value ("disabled-default-build-tools");
        disabled_tools.get ("ai", out iter);

        int disabled_tool_id;
        while (iter.next ("i", out disabled_tool_id))
            disabled_tool_ids += disabled_tool_id;

        /* Disable the build tools to disable */

        tool_num = 0;
        foreach (BuildTool build_tool in _build_tools)
        {
            if (build_tool.id in disabled_tool_ids)
                set_enabled (tool_num, false);

            tool_num++;
        }
    }

    private void save_enable_setting ()
    {
        VariantBuilder builder_enabled = new VariantBuilder (VariantType.ARRAY);
        VariantBuilder builder_disabled = new VariantBuilder (VariantType.ARRAY);

        foreach (BuildTool build_tool in _build_tools)
        {
            if (build_tool.enabled)
                builder_enabled.add ("i", build_tool.id);
            else
                builder_disabled.add ("i", build_tool.id);
        }

        Variant enabled_tools = builder_enabled.end ();
        Variant disabled_tools = builder_disabled.end ();

        GLib.Settings settings =
            new GLib.Settings ("org.gnome.latexila.preferences.latex");

        settings.set_value ("enabled-default-build-tools", enabled_tools);
        settings.set_value ("disabled-default-build-tools", disabled_tools);
    }
}

public class PersonalBuildTools : BuildTools
{
    private static PersonalBuildTools _instance = null;

    private bool _modified = false;

    private PersonalBuildTools ()
    {
        _xml_file = get_user_config_file ();
        load ();

        modified.connect (() => _modified = true);
    }

    public static PersonalBuildTools get_default ()
    {
        if (_instance == null)
            _instance = new PersonalBuildTools ();

        return _instance;
    }

    public void move_up (int tool_num)
    {
        return_if_fail (0 < tool_num && tool_num < _build_tools.size);
        swap (tool_num, tool_num - 1);
    }

    public void move_down (int tool_num)
    {
        return_if_fail (0 <= tool_num && tool_num < _build_tools.size - 1);
        swap (tool_num, tool_num + 1);
    }

    private void swap (int tool_num1, int tool_num2)
    {
        BuildTool tool = _build_tools[tool_num1];
        _build_tools.remove_at (tool_num1);
        _build_tools.insert (tool_num2, tool);
        modified ();
    }

    public void delete (int tool_num)
    {
        return_if_fail (is_valid_index (tool_num));

        _build_tools.remove_at (tool_num);
        modified ();
    }

    public void add (BuildTool tool)
    {
        insert (_build_tools.size, tool);
    }

    public void insert (int pos, BuildTool tool)
    {
        return_if_fail (0 <= pos && pos <= _build_tools.size);

        _build_tools.insert (pos, tool);
        modified ();
    }

    public void update (int num, BuildTool tool)
    {
        return_if_fail (is_valid_index (num));

        BuildTool current_tool = _build_tools[num];

        if (! is_equal (current_tool, tool))
        {
            _build_tools[num] = tool;
            modified ();
        }
    }

    private File get_user_config_file ()
    {
        string path = Path.build_filename (Environment.get_user_config_dir (),
            "latexila", "build_tools.xml");

        return File.new_for_path (path);
    }

    public void save ()
    {
        if (! _modified)
            return;

        string content = "<tools>";

        foreach (BuildTool tool in _build_tools)
        {
            content += "\n  <tool enabled=\"%s\"".printf (tool.enabled.to_string ());
            content += " extensions=\"%s\"".printf (tool.extensions);
            content += " icon=\"%s\">\n".printf (tool.icon);

            content += Markup.printf_escaped ("    <label>%s</label>\n", tool.label);
            content += Markup.printf_escaped ("    <description>%s</description>\n",
                tool.get_description ());

            foreach (BuildJob job in tool.jobs)
            {
                content += "    <job postProcessor=\"%s\">".printf (
                    get_post_processor_name_from_type (job.post_processor));

                content += Markup.printf_escaped ("%s</job>\n", job.command);
            }

            content += Markup.printf_escaped ("    <open>%s</open>\n",
                tool.files_to_open);

            content += "  </tool>\n";
        }

        content += "</tools>\n";

        // save the file
        File file = get_user_config_file ();
        Utils.save_file (file, content, true);
    }

    private bool is_equal (BuildTool tool1, BuildTool tool2)
    {
        if (tool1.enabled != tool2.enabled
            || tool1.label != tool2.label
            || tool1.get_description () != tool2.get_description ()
            || tool1.extensions != tool2.extensions
            || tool1.icon != tool2.icon
            || tool1.files_to_open != tool2.files_to_open
            || tool1.jobs.size != tool2.jobs.size)
        {
            return false;
        }

        for (int job_num = 0 ; job_num < tool1.jobs.size ; job_num++)
        {
            BuildJob job1 = tool1.jobs[job_num];
            BuildJob job2 = tool2.jobs[job_num];

            if (job1.command != job2.command
                || job1.post_processor != job2.post_processor)
            {
                return false;
            }
        }

        return true;
    }
}
