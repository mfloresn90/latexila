/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2017 Sébastien Wilmet
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

public class TabLabel : Tepl.TabLabel
{
    public TabLabel (DocumentTab tab)
    {
        Object (tab: tab);

        Document doc = tab.get_buffer () as Document;
        doc.notify["project-id"].connect (update_tooltip);
    }

    public override string get_tooltip_markup ()
    {
        string base_tooltip = base.get_tooltip_markup ();

        Document doc = tab.get_buffer () as Document;
        File? location = doc.get_file ().get_location ();
        if (location == null)
            return base_tooltip;

        Project? project = doc.get_project ();
        if (project == null)
            return base_tooltip;

        if (base_tooltip == null)
            base_tooltip = "";

        if (project.main_file.equal (location))
            return base_tooltip + Markup.printf_escaped ("\n<b>%s</b>",
                _("Project main file"));

        return base_tooltip + Markup.printf_escaped ("\n<b>%s</b> %s",
            _("Project main file:"), get_main_file_relative_path ());
    }

    private string? get_main_file_relative_path ()
    {
        Document doc = tab.get_buffer () as Document;
        Project? project = doc.get_project ();
        if (project == null)
            return "";

        File origin = doc.get_file ().get_location ();
        File target = project.main_file;
        File common_dir = project.directory;

        return Utils.get_relative_path (origin, target, common_dir);
    }
}
