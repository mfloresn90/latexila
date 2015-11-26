SUBDIRS = build_tools icons images templates

dist_pkgdata_DATA =	\
	completion.xml	\
	latexhelp.html

desktopdir = $(datadir)/applications
desktop_DATA = org.gnome.latexila.desktop
@INTLTOOL_DESKTOP_RULE@

@INTLTOOL_XML_RULE@
appdatadir = $(datadir)/appdata
appdata_DATA = org.gnome.latexila.appdata.xml

gsettings_SCHEMAS = org.gnome.latexila.gschema.xml
@GSETTINGS_RULES@

EXTRA_DIST =					\
	org.gnome.latexila.appdata.xml.in	\
	org.gnome.latexila.desktop.in		\
	org.gnome.latexila.gschema.xml.in

CLEANFILES =				\
	org.gnome.latexila.appdata.xml	\
	org.gnome.latexila.desktop	\
	org.gnome.latexila.gschema.xml

-include $(top_srcdir)/git.mk
