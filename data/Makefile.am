SUBDIRS = build_tools icons images templates

dist_pkgdata_DATA =	\
	completion.xml	\
	latexhelp.html

desktopdir = $(datadir)/applications
desktop_DATA = org.gnome.latexila.desktop
@INTLTOOL_DESKTOP_RULE@

@INTLTOOL_XML_RULE@
appdatadir = $(datadir)/appdata
appdata_in_files = latexila.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)

gsettings_SCHEMAS = org.gnome.latexila.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST =					\
	org.gnome.latexila.desktop.in		\
	org.gnome.latexila.gschema.xml.in	\
	$(appdata_in_files)

CLEANFILES =				\
	org.gnome.latexila.desktop	\
	org.gnome.latexila.gschema.xml	\
	$(appdata_DATA)

-include $(top_srcdir)/git.mk
