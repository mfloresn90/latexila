SUBDIRS = build_tools icons images templates

dist_pkgdata_DATA =	\
	completion.xml	\
	latexhelp.html

desktopdir = $(datadir)/applications
desktop_DATA = org.gnome.latexila.desktop
@INTLTOOL_DESKTOP_RULE@

@INTLTOOL_XML_RULE@

appstream_XML = org.gnome.latexila.appdata.xml
@APPSTREAM_XML_RULES@

gsettings_SCHEMAS = org.gnome.latexila.gschema.xml
@GSETTINGS_RULES@

dbusservicedir = $(datadir)/dbus-1/services
dbusservice_DATA = org.gnome.latexila.service

org.gnome.latexila.service: Makefile
	$(AM_V_GEN) (echo '[D-BUS Service]'; \
		     echo 'Name=org.gnome.latexila'; \
		     echo 'Exec=${bindir}/latexila --gapplication-service') > $@.tmp && \
		     mv $@.tmp $@

EXTRA_DIST =					\
	org.gnome.latexila.appdata.xml.in	\
	org.gnome.latexila.desktop.in.in	\
	org.gnome.latexila.gschema.xml.in

DISTCLEANFILES =			\
	org.gnome.latexila.desktop.in	\
	org.gnome.latexila.gschema.xml

CLEANFILES =				\
	org.gnome.latexila.appdata.xml	\
	org.gnome.latexila.desktop	\
	org.gnome.latexila.service

-include $(top_srcdir)/git.mk
