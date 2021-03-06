SUBDIRS = evince liblatexila ui

bin_PROGRAMS = latexila

# --disable-since-check is used because LaTeXila often depends on unstable
# versions of GNOME libraries.
AM_VALAFLAGS =			\
	--disable-since-check	\
	--pkg gtk+-3.0		\
	--pkg gtksourceview-3.0	\
	--pkg Tepl-3		\
	--pkg gspell-1		\
	--pkg gee-0.8		\
	--pkg posix		\
	--target-glib=2.50	\
	--vapidir liblatexila	\
	--pkg latexila

# The valac command is always executed in the sources directory, not the build
# directory. If $(top_srcdir) is used, it will be relative to the build directory.
vapidir = ../vapi

vala_files =				\
	app_settings.vala		\
	bottom_panel.vala		\
	build_tool_dialog.vala		\
	build_tools_preferences.vala	\
	clean_build_files.vala		\
	completion.vala			\
	custom_statusbar.vala		\
	dialogs.vala			\
	documents_panel.vala		\
	document_structure.vala		\
	document_tab.vala		\
	document.vala			\
	document_view.vala		\
	encodings.vala			\
	error_entry.vala		\
	file_browser.vala		\
	finance.vala			\
	latexila_app.vala		\
	latex_menu.vala			\
	main.vala			\
	main_window_build_tools.vala	\
	main_window_documents.vala	\
	main_window_edit.vala		\
	main_window_file.vala		\
	main_window_structure.vala	\
	main_window_tools.vala		\
	main_window.vala		\
	menu_in_toolbar.vala		\
	most_used_symbols.vala		\
	preferences_dialog.vala		\
	project_dialogs.vala		\
	projects.vala			\
	search.vala			\
	side_panel.vala			\
	stock_icons.vala		\
	structure_model.vala		\
	structure.vala			\
	symbols.vala			\
	symbols_view.vala		\
	tab_label.vala			\
	utils.vala

latexila_SOURCES = 				\
	$(vala_files)				\
	$(vapidir)/config.vapi			\
	$(vapidir)/menu_tool_action.vapi

# -w: silence all warnings, because we already have the valac warnings. The
# generated C code currently triggers a lot of warnings because deprecated
# functions are used. So by silencing the GCC warnings, we can more easily
# scroll up the build output to see the valac warnings.
latexila_CPPFLAGS =			\
	-I$(top_srcdir)			\
	-I$(srcdir)/liblatexila		\
	-I$(builddir)/liblatexila	\
	-include config.h		\
	-w

latexila_CFLAGS =		\
	$(LATEXILA_CFLAGS)

latexila_LDADD = 			\
	$(LATEXILA_LIBS)		\
	liblatexila/liblatexila.la

# GResource

latexila-resources.c: latexila.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/latexila.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source $(srcdir)/latexila.gresource.xml

BUILT_SOURCES = latexila-resources.c
nodist_latexila_SOURCES = $(BUILT_SOURCES)

EXTRA_DIST = latexila.gresource.xml

CLEANFILES =			\
	$(BUILT_SOURCES)	\
	$(vala_files:.vala=.c)	\
	latexila_vala.stamp

-include $(top_srcdir)/git.mk
