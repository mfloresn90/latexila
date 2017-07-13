SUBDIRS = data man po help src tests docs

@CODE_COVERAGE_RULES@

AM_DISTCHECK_CONFIGURE_FLAGS =	\
	--enable-gtk-doc	\
	--enable-introspection	\
	--enable-vala		\
	--enable-appstream-util

EXTRA_DIST =			\
	HACKING			\
	latexila.doap		\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in

DISTCLEANFILES =		\
	intltool-extract	\
	intltool-merge		\
	intltool-update

MAINTAINERCLEANFILES =		\
	$(GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL) \
	$(GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN) \
	$(GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL) \
	gtk-doc.make \
	INSTALL

-include $(top_srcdir)/git.mk
