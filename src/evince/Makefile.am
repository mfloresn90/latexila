noinst_LTLIBRARIES = libevince.la

libevince_la_CFLAGS =		\
	$(LATEXILA_CFLAGS)	\
	$(WARN_CFLAGS)

libevince_la_LIBADD =		\
	$(LATEXILA_LIBS)

libevince_built_sources =		\
	evince-gdbus-generated.c	\
	evince-gdbus-generated.h

nodist_libevince_la_SOURCES = $(libevince_built_sources)

BUILT_SOURCES = $(libevince_built_sources)

evince-gdbus-generated.c evince-gdbus-generated.h: evince-gdbus.xml Makefile
	$(AM_V_GEN) $(GDBUS_CODEGEN) \
			--interface-prefix=org.gnome.evince. \
			--c-namespace=Evince \
			--generate-c-code evince-gdbus-generated \
			$(srcdir)/evince-gdbus.xml

EXTRA_DIST = evince-gdbus.xml

CLEANFILES = $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
