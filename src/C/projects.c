/* projects.c generated by valac 0.10.3, the Vala compiler
 * generated from projects.vala, do not modify */

/*
 * This file is part of LaTeXila.
 *
 * Copyright © 2010 Sébastien Wilmet
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

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <gio/gio.h>
#include <gtksourceview/gtksourceview.h>
#include <string.h>
#include <stdlib.h>
#include <gee.h>


#define TYPE_PROJECTS (projects_get_type ())
#define PROJECTS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_PROJECTS, Projects))
#define PROJECTS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_PROJECTS, ProjectsClass))
#define IS_PROJECTS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_PROJECTS))
#define IS_PROJECTS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_PROJECTS))
#define PROJECTS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_PROJECTS, ProjectsClass))

typedef struct _Projects Projects;
typedef struct _ProjectsClass ProjectsClass;
typedef struct _ProjectsPrivate ProjectsPrivate;

#define PROJECTS_TYPE_PROJECT_COLUMN (projects_project_column_get_type ())

#define TYPE_MAIN_WINDOW (main_window_get_type ())
#define MAIN_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MAIN_WINDOW, MainWindow))
#define MAIN_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MAIN_WINDOW, MainWindowClass))
#define IS_MAIN_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MAIN_WINDOW))
#define IS_MAIN_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MAIN_WINDOW))
#define MAIN_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MAIN_WINDOW, MainWindowClass))

typedef struct _MainWindow MainWindow;
typedef struct _MainWindowClass MainWindowClass;
#define _g_free0(var) (var = (g_free (var), NULL))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

#define TYPE_DOCUMENT (document_get_type ())
#define DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_DOCUMENT, Document))
#define DOCUMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DOCUMENT, DocumentClass))
#define IS_DOCUMENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_DOCUMENT))
#define IS_DOCUMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DOCUMENT))
#define DOCUMENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DOCUMENT, DocumentClass))

typedef struct _Document Document;
typedef struct _DocumentClass DocumentClass;

#define TYPE_PROJECT (project_get_type ())
typedef struct _Project Project;

#define TYPE_APP_SETTINGS (app_settings_get_type ())
#define APP_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_APP_SETTINGS, AppSettings))
#define APP_SETTINGS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_APP_SETTINGS, AppSettingsClass))
#define IS_APP_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_APP_SETTINGS))
#define IS_APP_SETTINGS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_APP_SETTINGS))
#define APP_SETTINGS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_APP_SETTINGS, AppSettingsClass))

typedef struct _AppSettings AppSettings;
typedef struct _AppSettingsClass AppSettingsClass;
typedef struct _Block9Data Block9Data;
#define _project_free0(var) ((var == NULL) ? NULL : (var = (project_free (var), NULL)))
typedef struct _Block10Data Block10Data;

struct _Projects {
	GObject parent_instance;
	ProjectsPrivate * priv;
};

struct _ProjectsClass {
	GObjectClass parent_class;
};

typedef enum  {
	PROJECTS_PROJECT_COLUMN_DIRECTORY,
	PROJECTS_PROJECT_COLUMN_MAIN_FILE,
	PROJECTS_PROJECT_COLUMN_N_COLUMNS
} ProjectsProjectColumn;

struct _Project {
	GFile* directory;
	GFile* main_file;
};

struct _Block9Data {
	int _ref_count_;
	GtkFileChooserButton* file_chooser_button1;
	GtkFileChooserButton* file_chooser_button2;
};

struct _Block10Data {
	int _ref_count_;
	GtkDialog* dialog;
	GtkListStore* store;
	GtkTreeView* treeview;
};


static gpointer projects_parent_class = NULL;

GType projects_get_type (void) G_GNUC_CONST;
enum  {
	PROJECTS_DUMMY_PROPERTY
};
static GType projects_project_column_get_type (void) G_GNUC_UNUSED;
GType main_window_get_type (void) G_GNUC_CONST;
void projects_new_project (MainWindow* main_window);
static void _lambda65_ (Block9Data* _data9_);
static void __lambda65__gtk_file_chooser_button_file_set (GtkFileChooserButton* _sender, gpointer self);
GType document_get_type (void) G_GNUC_CONST;
Document* main_window_get_active_document (MainWindow* self);
GFile* document_get_location (Document* self);
static gboolean projects_main_file_is_in_directory (GtkWindow* window, GFile* main_file, GFile* directory);
GType project_get_type (void) G_GNUC_CONST;
Project* project_dup (const Project* self);
void project_free (Project* self);
void project_copy (const Project* self, Project* dest);
void project_destroy (Project* self);
GType app_settings_get_type (void) G_GNUC_CONST;
AppSettings* app_settings_get_default (void);
gboolean app_settings_add_project (AppSettings* self, Project* new_project, GFile** conflict);
char* utils_replace_home_dir_with_tilde (const char* uri);
static Block9Data* block9_data_ref (Block9Data* _data9_);
static void block9_data_unref (Block9Data* _data9_);
gboolean projects_configure_project (GtkWindow* main_window, gint project_id);
Project* app_settings_get_project (AppSettings* self, gint id);
gboolean app_settings_project_change_main_file (AppSettings* self, gint num, GFile* new_main_file);
void projects_manage_projects (MainWindow* main_window);
static void projects_update_model (GtkListStore* model);
GtkWidget* utils_add_scrollbar (GtkWidget* child);
static void _lambda66_ (Block10Data* _data10_);
gint utils_get_selected_row (GtkTreeView* view, GtkTreeIter* iter_to_set);
static void __lambda66__gtk_button_clicked (GtkButton* _sender, gpointer self);
static void _lambda67_ (Block10Data* _data10_);
void app_settings_delete_project (AppSettings* self, gint num);
static void __lambda67__gtk_button_clicked (GtkButton* _sender, gpointer self);
static void _lambda68_ (Block10Data* _data10_);
void app_settings_clear_all_projects (AppSettings* self);
static void __lambda68__gtk_button_clicked (GtkButton* _sender, gpointer self);
static Block10Data* block10_data_ref (Block10Data* _data10_);
static void block10_data_unref (Block10Data* _data10_);
GeeLinkedList* app_settings_get_projects (AppSettings* self);
Projects* projects_new (void);
Projects* projects_construct (GType object_type);



static GType projects_project_column_get_type (void) {
	static volatile gsize projects_project_column_type_id__volatile = 0;
	if (g_once_init_enter (&projects_project_column_type_id__volatile)) {
		static const GEnumValue values[] = {{PROJECTS_PROJECT_COLUMN_DIRECTORY, "PROJECTS_PROJECT_COLUMN_DIRECTORY", "directory"}, {PROJECTS_PROJECT_COLUMN_MAIN_FILE, "PROJECTS_PROJECT_COLUMN_MAIN_FILE", "main-file"}, {PROJECTS_PROJECT_COLUMN_N_COLUMNS, "PROJECTS_PROJECT_COLUMN_N_COLUMNS", "n-columns"}, {0, NULL, NULL}};
		GType projects_project_column_type_id;
		projects_project_column_type_id = g_enum_register_static ("ProjectsProjectColumn", values);
		g_once_init_leave (&projects_project_column_type_id__volatile, projects_project_column_type_id);
	}
	return projects_project_column_type_id__volatile;
}


static gpointer _g_object_ref0 (gpointer self) {
	return self ? g_object_ref (self) : NULL;
}


static void _lambda65_ (Block9Data* _data9_) {
	GFile* dir;
	GError * _inner_error_ = NULL;
	dir = _g_object_ref0 (gtk_file_chooser_get_file ((GtkFileChooser*) _data9_->file_chooser_button1));
	{
		gtk_file_chooser_set_current_folder_file ((GtkFileChooser*) _data9_->file_chooser_button2, dir, &_inner_error_);
		if (_inner_error_ != NULL) {
			goto __catch25_g_error;
		}
	}
	goto __finally25;
	__catch25_g_error:
	{
		GError * e;
		e = _inner_error_;
		_inner_error_ = NULL;
		{
			_g_error_free0 (e);
		}
	}
	__finally25:
	if (_inner_error_ != NULL) {
		_g_object_unref0 (dir);
		g_critical ("file %s: line %d: uncaught error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
		g_clear_error (&_inner_error_);
		return;
	}
	_g_object_unref0 (dir);
}


static void __lambda65__gtk_file_chooser_button_file_set (GtkFileChooserButton* _sender, gpointer self) {
	_lambda65_ (self);
}


static Block9Data* block9_data_ref (Block9Data* _data9_) {
	g_atomic_int_inc (&_data9_->_ref_count_);
	return _data9_;
}


static void block9_data_unref (Block9Data* _data9_) {
	if (g_atomic_int_dec_and_test (&_data9_->_ref_count_)) {
		_g_object_unref0 (_data9_->file_chooser_button2);
		_g_object_unref0 (_data9_->file_chooser_button1);
		g_slice_free (Block9Data, _data9_);
	}
}


void projects_new_project (MainWindow* main_window) {
	Block9Data* _data9_;
	GtkDialog* dialog;
	GtkVBox* content_area;
	GtkHBox* hbox;
	GtkVBox* vbox1;
	GtkVBox* vbox2;
	GtkLabel* label1;
	char* _tmp0_;
	char* _tmp1_;
	GtkLabel* label2;
	char* _tmp2_;
	char* _tmp3_;
	Document* doc;
	GError * _inner_error_ = NULL;
	g_return_if_fail (main_window != NULL);
	_data9_ = g_slice_new0 (Block9Data);
	_data9_->_ref_count_ = 1;
	dialog = g_object_ref_sink ((GtkDialog*) gtk_dialog_new_with_buttons (_ ("New Project"), (GtkWindow*) main_window, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_NEW, GTK_RESPONSE_OK, NULL, NULL));
	content_area = _g_object_ref0 (GTK_VBOX (gtk_dialog_get_content_area (dialog)));
	hbox = g_object_ref_sink ((GtkHBox*) gtk_hbox_new (FALSE, 6));
	vbox1 = g_object_ref_sink ((GtkVBox*) gtk_vbox_new (TRUE, 6));
	vbox2 = g_object_ref_sink ((GtkVBox*) gtk_vbox_new (TRUE, 6));
	gtk_box_pack_start ((GtkBox*) hbox, (GtkWidget*) vbox1, FALSE, FALSE, 0);
	gtk_box_pack_start ((GtkBox*) hbox, (GtkWidget*) vbox2, TRUE, TRUE, 0);
	gtk_container_set_border_width ((GtkContainer*) hbox, (guint) 6);
	label1 = g_object_ref_sink ((GtkLabel*) gtk_label_new (NULL));
	gtk_label_set_markup (label1, _tmp1_ = g_strconcat (_tmp0_ = g_strconcat ("<b>", _ ("Directory:"), NULL), "</b>", NULL));
	_g_free0 (_tmp1_);
	_g_free0 (_tmp0_);
	label2 = g_object_ref_sink ((GtkLabel*) gtk_label_new (NULL));
	gtk_label_set_markup (label2, _tmp3_ = g_strconcat (_tmp2_ = g_strconcat ("<b>", _ ("Main File:"), NULL), "</b>", NULL));
	_g_free0 (_tmp3_);
	_g_free0 (_tmp2_);
	gtk_box_pack_start ((GtkBox*) vbox1, (GtkWidget*) label1, TRUE, TRUE, 0);
	gtk_box_pack_start ((GtkBox*) vbox1, (GtkWidget*) label2, TRUE, TRUE, 0);
	_data9_->file_chooser_button1 = g_object_ref_sink ((GtkFileChooserButton*) gtk_file_chooser_button_new (_ ("Directory"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER));
	_data9_->file_chooser_button2 = g_object_ref_sink ((GtkFileChooserButton*) gtk_file_chooser_button_new (_ ("Main File"), GTK_FILE_CHOOSER_ACTION_OPEN));
	gtk_box_pack_start ((GtkBox*) vbox2, (GtkWidget*) _data9_->file_chooser_button1, TRUE, TRUE, 0);
	gtk_box_pack_start ((GtkBox*) vbox2, (GtkWidget*) _data9_->file_chooser_button2, TRUE, TRUE, 0);
	gtk_box_pack_start ((GtkBox*) content_area, (GtkWidget*) hbox, TRUE, TRUE, 0);
	gtk_widget_show_all ((GtkWidget*) content_area);
	g_signal_connect_data (_data9_->file_chooser_button1, "file-set", (GCallback) __lambda65__gtk_file_chooser_button_file_set, block9_data_ref (_data9_), (GClosureNotify) block9_data_unref, 0);
	doc = _g_object_ref0 (main_window_get_active_document (main_window));
	if (doc != NULL) {
		{
			GFile* _tmp4_;
			gtk_file_chooser_set_file ((GtkFileChooser*) _data9_->file_chooser_button1, _tmp4_ = g_file_get_parent (document_get_location (doc)), &_inner_error_);
			_g_object_unref0 (_tmp4_);
			if (_inner_error_ != NULL) {
				goto __catch26_g_error;
			}
			gtk_file_chooser_set_file ((GtkFileChooser*) _data9_->file_chooser_button2, document_get_location (doc), &_inner_error_);
			if (_inner_error_ != NULL) {
				goto __catch26_g_error;
			}
		}
		goto __finally26;
		__catch26_g_error:
		{
			GError * e;
			e = _inner_error_;
			_inner_error_ = NULL;
			{
				_g_error_free0 (e);
			}
		}
		__finally26:
		if (_inner_error_ != NULL) {
			_g_object_unref0 (doc);
			_g_object_unref0 (label2);
			_g_object_unref0 (label1);
			_g_object_unref0 (vbox2);
			_g_object_unref0 (vbox1);
			_g_object_unref0 (hbox);
			_g_object_unref0 (content_area);
			_g_object_unref0 (dialog);
			block9_data_unref (_data9_);
			g_critical ("file %s: line %d: uncaught error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
			g_clear_error (&_inner_error_);
			return;
		}
	}
	while (TRUE) {
		GFile* directory;
		GFile* main_file;
		gboolean _tmp5_ = FALSE;
		Project project = {0};
		GFile* _tmp6_;
		GFile* _tmp7_;
		GFile* conflict;
		AppSettings* _tmp8_;
		GFile* _tmp9_ = NULL;
		gboolean _tmp10_;
		GFile* _tmp11_;
		gboolean _tmp12_;
		char* _tmp13_;
		char* _tmp14_;
		char* _tmp15_;
		GtkDialog* _tmp16_;
		GtkDialog* error_dialog;
		if (!(gtk_dialog_run (dialog) == GTK_RESPONSE_OK)) {
			break;
		}
		directory = _g_object_ref0 (gtk_file_chooser_get_file ((GtkFileChooser*) _data9_->file_chooser_button1));
		main_file = _g_object_ref0 (gtk_file_chooser_get_file ((GtkFileChooser*) _data9_->file_chooser_button2));
		if (directory == NULL) {
			_tmp5_ = TRUE;
		} else {
			_tmp5_ = main_file == NULL;
		}
		if (_tmp5_) {
			_g_object_unref0 (main_file);
			_g_object_unref0 (directory);
			continue;
		}
		if (!projects_main_file_is_in_directory ((GtkWindow*) dialog, main_file, directory)) {
			_g_object_unref0 (main_file);
			_g_object_unref0 (directory);
			continue;
		}
		memset (&project, 0, sizeof (Project));
		project.directory = (_tmp6_ = _g_object_ref0 (directory), _g_object_unref0 (project.directory), _tmp6_);
		project.main_file = (_tmp7_ = _g_object_ref0 (main_file), _g_object_unref0 (project.main_file), _tmp7_);
		conflict = NULL;
		if ((_tmp12_ = (_tmp10_ = app_settings_add_project (_tmp8_ = app_settings_get_default (), &project, &_tmp9_), conflict = (_tmp11_ = _tmp9_, _g_object_unref0 (conflict), _tmp11_), _tmp10_), _g_object_unref0 (_tmp8_), _tmp12_)) {
			_g_object_unref0 (conflict);
			project_destroy (&project);
			_g_object_unref0 (main_file);
			_g_object_unref0 (directory);
			break;
		}
		error_dialog = (_tmp16_ = (GtkDialog*) g_object_ref_sink ((GtkMessageDialog*) gtk_message_dialog_new ((GtkWindow*) dialog, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _ ("There is a conflict with the project \"%s\"."), _tmp15_ = g_strconcat (_tmp14_ = utils_replace_home_dir_with_tilde (_tmp13_ = g_file_get_parse_name (conflict)), "/", NULL))), _g_free0 (_tmp15_), _g_free0 (_tmp14_), _g_free0 (_tmp13_), _tmp16_);
		gtk_dialog_run (error_dialog);
		gtk_object_destroy ((GtkObject*) error_dialog);
		_g_object_unref0 (error_dialog);
		_g_object_unref0 (conflict);
		project_destroy (&project);
		_g_object_unref0 (main_file);
		_g_object_unref0 (directory);
	}
	gtk_object_destroy ((GtkObject*) dialog);
	_g_object_unref0 (doc);
	_g_object_unref0 (label2);
	_g_object_unref0 (label1);
	_g_object_unref0 (vbox2);
	_g_object_unref0 (vbox1);
	_g_object_unref0 (hbox);
	_g_object_unref0 (content_area);
	_g_object_unref0 (dialog);
	block9_data_unref (_data9_);
}


gboolean projects_configure_project (GtkWindow* main_window, gint project_id) {
	gboolean result = FALSE;
	AppSettings* _tmp0_;
	Project* _tmp1_;
	Project* project;
	GtkDialog* dialog;
	GtkVBox* content_area;
	char* _tmp2_;
	char* _tmp3_;
	char* _tmp4_;
	char* _tmp5_;
	GtkLabel* _tmp6_;
	GtkLabel* location;
	GtkHBox* hbox;
	GtkLabel* label;
	GtkFileChooserButton* file_chooser_button;
	gboolean ret;
	GError * _inner_error_ = NULL;
	g_return_val_if_fail (main_window != NULL, FALSE);
	project = (_tmp1_ = app_settings_get_project (_tmp0_ = app_settings_get_default (), project_id), _g_object_unref0 (_tmp0_), _tmp1_);
	g_return_val_if_fail (project != NULL, FALSE);
	dialog = g_object_ref_sink ((GtkDialog*) gtk_dialog_new_with_buttons (_ ("Configure Project"), main_window, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL, NULL));
	content_area = _g_object_ref0 (GTK_VBOX (gtk_dialog_get_content_area (dialog)));
	location = (_tmp6_ = g_object_ref_sink ((GtkLabel*) gtk_label_new (_tmp5_ = g_strdup_printf (_ ("Location of the project: %s"), _tmp4_ = g_strconcat (_tmp3_ = utils_replace_home_dir_with_tilde (_tmp2_ = g_file_get_parse_name ((*project).directory)), "/", NULL)))), _g_free0 (_tmp5_), _g_free0 (_tmp4_), _g_free0 (_tmp3_), _g_free0 (_tmp2_), _tmp6_);
	gtk_label_set_line_wrap (location, TRUE);
	gtk_box_pack_start ((GtkBox*) content_area, (GtkWidget*) location, FALSE, FALSE, (guint) 6);
	hbox = g_object_ref_sink ((GtkHBox*) gtk_hbox_new (FALSE, 6));
	gtk_box_pack_start ((GtkBox*) content_area, (GtkWidget*) hbox, TRUE, TRUE, 0);
	label = g_object_ref_sink ((GtkLabel*) gtk_label_new (_ ("Main File:")));
	gtk_box_pack_start ((GtkBox*) hbox, (GtkWidget*) label, FALSE, FALSE, 0);
	file_chooser_button = g_object_ref_sink ((GtkFileChooserButton*) gtk_file_chooser_button_new (_ ("Main File"), GTK_FILE_CHOOSER_ACTION_OPEN));
	gtk_box_pack_start ((GtkBox*) hbox, (GtkWidget*) file_chooser_button, TRUE, TRUE, 0);
	gtk_widget_show_all ((GtkWidget*) content_area);
	{
		gtk_file_chooser_set_file ((GtkFileChooser*) file_chooser_button, (*project).main_file, &_inner_error_);
		if (_inner_error_ != NULL) {
			goto __catch27_g_error;
		}
	}
	goto __finally27;
	__catch27_g_error:
	{
		GError * e;
		e = _inner_error_;
		_inner_error_ = NULL;
		{
			_g_error_free0 (e);
		}
	}
	__finally27:
	if (_inner_error_ != NULL) {
		_g_object_unref0 (file_chooser_button);
		_g_object_unref0 (label);
		_g_object_unref0 (hbox);
		_g_object_unref0 (location);
		_g_object_unref0 (content_area);
		_g_object_unref0 (dialog);
		_project_free0 (project);
		g_critical ("file %s: line %d: uncaught error: %s (%s, %d)", __FILE__, __LINE__, _inner_error_->message, g_quark_to_string (_inner_error_->domain), _inner_error_->code);
		g_clear_error (&_inner_error_);
		return FALSE;
	}
	ret = FALSE;
	while (TRUE) {
		GFile* main_file;
		AppSettings* _tmp7_;
		if (!(gtk_dialog_run (dialog) == GTK_RESPONSE_OK)) {
			break;
		}
		main_file = _g_object_ref0 (gtk_file_chooser_get_file ((GtkFileChooser*) file_chooser_button));
		if (main_file == NULL) {
			_g_object_unref0 (main_file);
			continue;
		}
		if (!projects_main_file_is_in_directory ((GtkWindow*) dialog, main_file, (*project).directory)) {
			_g_object_unref0 (main_file);
			continue;
		}
		ret = app_settings_project_change_main_file (_tmp7_ = app_settings_get_default (), project_id, main_file);
		_g_object_unref0 (_tmp7_);
		_g_object_unref0 (main_file);
		break;
	}
	gtk_object_destroy ((GtkObject*) dialog);
	result = ret;
	_g_object_unref0 (file_chooser_button);
	_g_object_unref0 (label);
	_g_object_unref0 (hbox);
	_g_object_unref0 (location);
	_g_object_unref0 (content_area);
	_g_object_unref0 (dialog);
	_project_free0 (project);
	return result;
}


static void _lambda66_ (Block10Data* _data10_) {
	gint i;
	gboolean _tmp0_ = FALSE;
	i = utils_get_selected_row (_data10_->treeview, NULL);
	if (i != (-1)) {
		_tmp0_ = projects_configure_project ((GtkWindow*) _data10_->dialog, i);
	} else {
		_tmp0_ = FALSE;
	}
	if (_tmp0_) {
		projects_update_model (_data10_->store);
	}
}


static void __lambda66__gtk_button_clicked (GtkButton* _sender, gpointer self) {
	_lambda66_ (self);
}


static void _lambda67_ (Block10Data* _data10_) {
	GtkTreeIter iter = {0};
	gint i;
	char* directory;
	GtkTreeModel* model;
	GtkDialog* delete_dialog;
	i = utils_get_selected_row (_data10_->treeview, &iter);
	if (i == (-1)) {
		return;
	}
	directory = NULL;
	model = _g_object_ref0 (GTK_TREE_MODEL (_data10_->store));
	gtk_tree_model_get (model, &iter, PROJECTS_PROJECT_COLUMN_DIRECTORY, &directory, -1, -1);
	delete_dialog = (GtkDialog*) g_object_ref_sink ((GtkMessageDialog*) gtk_message_dialog_new ((GtkWindow*) _data10_->dialog, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, _ ("Do you really want to delete the project \"%s\"?"), directory));
	gtk_dialog_add_buttons (delete_dialog, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_DELETE, GTK_RESPONSE_YES, NULL);
	if (gtk_dialog_run (delete_dialog) == GTK_RESPONSE_YES) {
		AppSettings* _tmp0_;
		gtk_list_store_remove (_data10_->store, &iter);
		app_settings_delete_project (_tmp0_ = app_settings_get_default (), i);
		_g_object_unref0 (_tmp0_);
	}
	gtk_object_destroy ((GtkObject*) delete_dialog);
	_g_object_unref0 (delete_dialog);
	_g_object_unref0 (model);
	_g_free0 (directory);
}


static void __lambda67__gtk_button_clicked (GtkButton* _sender, gpointer self) {
	_lambda67_ (self);
}


static void _lambda68_ (Block10Data* _data10_) {
	GtkDialog* clear_dialog;
	GtkButton* button;
	GtkImage* img;
	clear_dialog = (GtkDialog*) g_object_ref_sink ((GtkMessageDialog*) gtk_message_dialog_new ((GtkWindow*) _data10_->dialog, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, _ ("Do you really want to clear all projects?")));
	gtk_dialog_add_button (clear_dialog, GTK_STOCK_CANCEL, (gint) GTK_RESPONSE_CANCEL);
	button = g_object_ref_sink ((GtkButton*) gtk_button_new_with_label (_ ("Clear All")));
	img = g_object_ref_sink ((GtkImage*) gtk_image_new_from_stock (GTK_STOCK_CLEAR, GTK_ICON_SIZE_BUTTON));
	gtk_button_set_image (button, (GtkWidget*) img);
	gtk_widget_show_all ((GtkWidget*) button);
	gtk_dialog_add_action_widget (clear_dialog, (GtkWidget*) button, (gint) GTK_RESPONSE_YES);
	if (gtk_dialog_run (clear_dialog) == GTK_RESPONSE_YES) {
		AppSettings* _tmp0_;
		app_settings_clear_all_projects (_tmp0_ = app_settings_get_default ());
		_g_object_unref0 (_tmp0_);
		gtk_list_store_clear (_data10_->store);
	}
	gtk_object_destroy ((GtkObject*) clear_dialog);
	_g_object_unref0 (img);
	_g_object_unref0 (button);
	_g_object_unref0 (clear_dialog);
}


static void __lambda68__gtk_button_clicked (GtkButton* _sender, gpointer self) {
	_lambda68_ (self);
}


static Block10Data* block10_data_ref (Block10Data* _data10_) {
	g_atomic_int_inc (&_data10_->_ref_count_);
	return _data10_;
}


static void block10_data_unref (Block10Data* _data10_) {
	if (g_atomic_int_dec_and_test (&_data10_->_ref_count_)) {
		_g_object_unref0 (_data10_->treeview);
		_g_object_unref0 (_data10_->store);
		_g_object_unref0 (_data10_->dialog);
		g_slice_free (Block10Data, _data10_);
	}
}


void projects_manage_projects (MainWindow* main_window) {
	Block10Data* _data10_;
	GtkVBox* content_area;
	GtkTreeViewColumn* column;
	GtkCellRendererPixbuf* pixbuf_renderer;
	GtkCellRendererText* text_renderer;
	GtkTreeViewColumn* _tmp0_;
	GtkCellRendererPixbuf* _tmp1_;
	GtkCellRendererText* _tmp2_;
	GtkTreeSelection* select;
	GtkWidget* sw;
	GtkHBox* hbox;
	GtkButton* edit_button;
	GtkButton* delete_button;
	GtkButton* clear_all_button;
	GtkImage* image;
	g_return_if_fail (main_window != NULL);
	_data10_ = g_slice_new0 (Block10Data);
	_data10_->_ref_count_ = 1;
	_data10_->dialog = g_object_ref_sink ((GtkDialog*) gtk_dialog_new_with_buttons (_ ("Manage Projects"), (GtkWindow*) main_window, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL, NULL));
	content_area = _g_object_ref0 (GTK_VBOX (gtk_dialog_get_content_area (_data10_->dialog)));
	_data10_->store = gtk_list_store_new ((gint) PROJECTS_PROJECT_COLUMN_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
	projects_update_model (_data10_->store);
	_data10_->treeview = g_object_ref_sink ((GtkTreeView*) gtk_tree_view_new_with_model ((GtkTreeModel*) _data10_->store));
	gtk_widget_set_size_request ((GtkWidget*) _data10_->treeview, 400, 150);
	gtk_tree_view_set_rules_hint (_data10_->treeview, TRUE);
	column = g_object_ref_sink (gtk_tree_view_column_new ());
	gtk_tree_view_append_column (_data10_->treeview, column);
	gtk_tree_view_column_set_title (column, _ ("Directory"));
	pixbuf_renderer = g_object_ref_sink ((GtkCellRendererPixbuf*) gtk_cell_renderer_pixbuf_new ());
	g_object_set (pixbuf_renderer, "stock-id", GTK_STOCK_DIRECTORY, NULL);
	gtk_cell_layout_pack_start ((GtkCellLayout*) column, (GtkCellRenderer*) pixbuf_renderer, FALSE);
	text_renderer = g_object_ref_sink ((GtkCellRendererText*) gtk_cell_renderer_text_new ());
	gtk_cell_layout_pack_start ((GtkCellLayout*) column, (GtkCellRenderer*) text_renderer, TRUE);
	gtk_tree_view_column_set_attributes (column, (GtkCellRenderer*) text_renderer, "text", PROJECTS_PROJECT_COLUMN_DIRECTORY, NULL, NULL);
	column = (_tmp0_ = g_object_ref_sink (gtk_tree_view_column_new ()), _g_object_unref0 (column), _tmp0_);
	gtk_tree_view_append_column (_data10_->treeview, column);
	gtk_tree_view_column_set_title (column, _ ("Main File"));
	pixbuf_renderer = (_tmp1_ = g_object_ref_sink ((GtkCellRendererPixbuf*) gtk_cell_renderer_pixbuf_new ()), _g_object_unref0 (pixbuf_renderer), _tmp1_);
	g_object_set (pixbuf_renderer, "stock-id", GTK_STOCK_FILE, NULL);
	gtk_cell_layout_pack_start ((GtkCellLayout*) column, (GtkCellRenderer*) pixbuf_renderer, FALSE);
	text_renderer = (_tmp2_ = g_object_ref_sink ((GtkCellRendererText*) gtk_cell_renderer_text_new ()), _g_object_unref0 (text_renderer), _tmp2_);
	gtk_cell_layout_pack_start ((GtkCellLayout*) column, (GtkCellRenderer*) text_renderer, TRUE);
	gtk_tree_view_column_set_attributes (column, (GtkCellRenderer*) text_renderer, "text", PROJECTS_PROJECT_COLUMN_MAIN_FILE, NULL, NULL);
	select = _g_object_ref0 (gtk_tree_view_get_selection (_data10_->treeview));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	sw = utils_add_scrollbar ((GtkWidget*) _data10_->treeview);
	gtk_box_pack_start ((GtkBox*) content_area, sw, TRUE, TRUE, 0);
	hbox = g_object_ref_sink ((GtkHBox*) gtk_hbox_new (FALSE, 5));
	gtk_box_pack_start ((GtkBox*) content_area, (GtkWidget*) hbox, FALSE, FALSE, (guint) 5);
	edit_button = g_object_ref_sink ((GtkButton*) gtk_button_new_from_stock (GTK_STOCK_PROPERTIES));
	delete_button = g_object_ref_sink ((GtkButton*) gtk_button_new_from_stock (GTK_STOCK_DELETE));
	clear_all_button = g_object_ref_sink ((GtkButton*) gtk_button_new_with_label (_ ("Clear All")));
	image = g_object_ref_sink ((GtkImage*) gtk_image_new_from_stock (GTK_STOCK_CLEAR, GTK_ICON_SIZE_MENU));
	gtk_button_set_image (clear_all_button, (GtkWidget*) image);
	gtk_box_pack_start ((GtkBox*) hbox, (GtkWidget*) edit_button, TRUE, TRUE, 0);
	gtk_box_pack_start ((GtkBox*) hbox, (GtkWidget*) delete_button, TRUE, TRUE, 0);
	gtk_box_pack_start ((GtkBox*) hbox, (GtkWidget*) clear_all_button, TRUE, TRUE, 0);
	gtk_widget_show_all ((GtkWidget*) content_area);
	g_signal_connect_data (edit_button, "clicked", (GCallback) __lambda66__gtk_button_clicked, block10_data_ref (_data10_), (GClosureNotify) block10_data_unref, 0);
	g_signal_connect_data (delete_button, "clicked", (GCallback) __lambda67__gtk_button_clicked, block10_data_ref (_data10_), (GClosureNotify) block10_data_unref, 0);
	g_signal_connect_data (clear_all_button, "clicked", (GCallback) __lambda68__gtk_button_clicked, block10_data_ref (_data10_), (GClosureNotify) block10_data_unref, 0);
	gtk_dialog_run (_data10_->dialog);
	gtk_object_destroy ((GtkObject*) _data10_->dialog);
	_g_object_unref0 (image);
	_g_object_unref0 (clear_all_button);
	_g_object_unref0 (delete_button);
	_g_object_unref0 (edit_button);
	_g_object_unref0 (hbox);
	_g_object_unref0 (sw);
	_g_object_unref0 (select);
	_g_object_unref0 (text_renderer);
	_g_object_unref0 (pixbuf_renderer);
	_g_object_unref0 (column);
	_g_object_unref0 (content_area);
	block10_data_unref (_data10_);
}


static gboolean projects_main_file_is_in_directory (GtkWindow* window, GFile* main_file, GFile* directory) {
	gboolean result = FALSE;
	GtkDialog* error_dialog;
	g_return_val_if_fail (window != NULL, FALSE);
	g_return_val_if_fail (main_file != NULL, FALSE);
	g_return_val_if_fail (directory != NULL, FALSE);
	if (g_file_has_prefix (main_file, directory)) {
		result = TRUE;
		return result;
	}
	error_dialog = (GtkDialog*) g_object_ref_sink ((GtkMessageDialog*) gtk_message_dialog_new (window, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _ ("The Main File is not in the directory.")));
	gtk_dialog_run (error_dialog);
	gtk_object_destroy ((GtkObject*) error_dialog);
	result = FALSE;
	_g_object_unref0 (error_dialog);
	return result;
}


static glong string_get_length (const char* self) {
	glong result;
	g_return_val_if_fail (self != NULL, 0L);
	result = g_utf8_strlen (self, (gssize) (-1));
	return result;
}


static char* string_slice (const char* self, glong start, glong end) {
	char* result = NULL;
	glong string_length;
	gboolean _tmp0_ = FALSE;
	gboolean _tmp1_ = FALSE;
	const char* start_string;
	g_return_val_if_fail (self != NULL, NULL);
	string_length = string_get_length (self);
	if (start < 0) {
		start = string_length + start;
	}
	if (end < 0) {
		end = string_length + end;
	}
	if (start >= 0) {
		_tmp0_ = start <= string_length;
	} else {
		_tmp0_ = FALSE;
	}
	g_return_val_if_fail (_tmp0_, NULL);
	if (end >= 0) {
		_tmp1_ = end <= string_length;
	} else {
		_tmp1_ = FALSE;
	}
	g_return_val_if_fail (_tmp1_, NULL);
	g_return_val_if_fail (start <= end, NULL);
	start_string = g_utf8_offset_to_pointer (self, start);
	result = g_strndup (start_string, ((gchar*) g_utf8_offset_to_pointer (start_string, end - start)) - ((gchar*) start_string));
	return result;
}


static void projects_update_model (GtkListStore* model) {
	AppSettings* _tmp0_;
	GeeLinkedList* _tmp1_;
	GeeLinkedList* projects;
	g_return_if_fail (model != NULL);
	gtk_list_store_clear (model);
	projects = (_tmp1_ = app_settings_get_projects (_tmp0_ = app_settings_get_default ()), _g_object_unref0 (_tmp0_), _tmp1_);
	{
		GeeIterator* _project_it;
		_project_it = gee_abstract_collection_iterator ((GeeAbstractCollection*) projects);
		while (TRUE) {
			Project* _tmp2_;
			Project _tmp3_ = {0};
			Project _tmp4_;
			Project project;
			char* uri_directory;
			char* uri_main_file;
			char* _tmp5_;
			char* _tmp6_;
			char* dir;
			char* main_file;
			GtkTreeIter iter = {0};
			if (!gee_iterator_next (_project_it)) {
				break;
			}
			project = (_tmp4_ = (project_copy (_tmp2_ = (Project*) gee_iterator_get (_project_it), &_tmp3_), _tmp3_), _project_free0 (_tmp2_), _tmp4_);
			uri_directory = g_file_get_parse_name (project.directory);
			uri_main_file = g_file_get_parse_name (project.main_file);
			dir = (_tmp6_ = g_strconcat (_tmp5_ = utils_replace_home_dir_with_tilde (uri_directory), "/", NULL), _g_free0 (_tmp5_), _tmp6_);
			main_file = string_slice (uri_main_file, string_get_length (uri_directory) + 1, string_get_length (uri_main_file));
			gtk_list_store_append (model, &iter);
			gtk_list_store_set (model, &iter, PROJECTS_PROJECT_COLUMN_DIRECTORY, dir, PROJECTS_PROJECT_COLUMN_MAIN_FILE, main_file, -1, -1);
			_g_free0 (main_file);
			_g_free0 (dir);
			_g_free0 (uri_main_file);
			_g_free0 (uri_directory);
			project_destroy (&project);
		}
		_g_object_unref0 (_project_it);
	}
}


Projects* projects_construct (GType object_type) {
	Projects * self = NULL;
	self = (Projects*) g_object_new (object_type, NULL);
	return self;
}


Projects* projects_new (void) {
	return projects_construct (TYPE_PROJECTS);
}


static void projects_class_init (ProjectsClass * klass) {
	projects_parent_class = g_type_class_peek_parent (klass);
}


static void projects_instance_init (Projects * self) {
}


GType projects_get_type (void) {
	static volatile gsize projects_type_id__volatile = 0;
	if (g_once_init_enter (&projects_type_id__volatile)) {
		static const GTypeInfo g_define_type_info = { sizeof (ProjectsClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) projects_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (Projects), 0, (GInstanceInitFunc) projects_instance_init, NULL };
		GType projects_type_id;
		projects_type_id = g_type_register_static (G_TYPE_OBJECT, "Projects", &g_define_type_info, 0);
		g_once_init_leave (&projects_type_id__volatile, projects_type_id);
	}
	return projects_type_id__volatile;
}




