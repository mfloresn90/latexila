/*
 * This file is part of LaTeXila.
 *
 * Copyright (C) 2014 - Sébastien Wilmet <swilmet@gnome.org>
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

/**
 * SECTION:build-view
 * @title: LatexilaBuildView
 * @short_description: Build view
 *
 * A #LatexilaBuildView is a #GtkTreeView containing the build messages.
 * The build view is contained in the bottom panel.
 */

#include "latexila-build-view.h"
#include <string.h>
#include "latexila-utils.h"
#include "latexila-enum-types.h"

struct _LatexilaBuildViewPrivate
{
  GtkTreeStore *store;
  GtkTreeModelFilter *filtered_model;

  guint show_warnings : 1;
  guint show_badboxes : 1;
  guint show_details : 1;
  guint has_details : 1;
};

enum
{
  PROP_0,
  PROP_SHOW_WARNINGS,
  PROP_SHOW_BADBOXES,
  PROP_SHOW_DETAILS,
  PROP_HAS_DETAILS
};

/* Columns for the GtkTreeView */
enum
{
  COLUMN_ICON,
  COLUMN_MESSAGE,
  COLUMN_MESSAGE_TYPE,
  COLUMN_WEIGHT,
  COLUMN_BASENAME,
  COLUMN_PATH,
  COLUMN_FILE,
  COLUMN_START_LINE,
  COLUMN_END_LINE,
  COLUMN_LINE_STR,
  NB_COLUMNS
};

enum
{
  SIGNAL_JUMP_TO_FILE,
  LAST_SIGNAL
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaBuildView, latexila_build_view, GTK_TYPE_TREE_VIEW)

static guint signals[LAST_SIGNAL];

/**
 * latexila_build_msg_reinit:
 * @build_msg: a #LatexilaBuildMsg.
 *
 * Reinitializes a #LatexilaBuildMsg.
 */
void
latexila_build_msg_reinit (LatexilaBuildMsg *build_msg)
{
  g_assert (build_msg != NULL);

  g_free (build_msg->text);
  g_free (build_msg->filename);

  memset (build_msg, 0, sizeof (LatexilaBuildMsg));

  build_msg->start_line = -1;
  build_msg->end_line = -1;
  build_msg->expand = TRUE;
}

/**
 * latexila_build_msg_new: (skip)
 *
 * Free the return value with latexila_build_msg_free() when no longer needed.
 *
 * Returns: a newly-allocated #LatexilaBuildMsg.
 */
LatexilaBuildMsg *
latexila_build_msg_new (void)
{
  LatexilaBuildMsg *build_msg;

  build_msg = g_slice_new0 (LatexilaBuildMsg);

  build_msg->start_line = -1;
  build_msg->end_line = -1;
  build_msg->expand = TRUE;

  return build_msg;
}

/**
 * latexila_build_msg_free: (skip)
 * @build_msg: a #LatexilaBuildMsg.
 *
 * Frees the @build_msg structure.
 */
void
latexila_build_msg_free (LatexilaBuildMsg *build_msg)
{
  if (build_msg != NULL)
    {
      g_free (build_msg->text);
      g_free (build_msg->filename);
      g_slice_free (LatexilaBuildMsg, build_msg);
    }
}

/**
 * latexila_build_msg_print:
 * @build_msg: a #LatexilaBuildMsg.
 *
 * Prints @build_msg on stdout, for debugging purposes.
 */
void
latexila_build_msg_print (LatexilaBuildMsg *build_msg)
{
  GEnumClass *enum_class;
  GEnumValue *enum_value;

  enum_class = g_type_class_ref (LATEXILA_TYPE_BUILD_MSG_TYPE);
  enum_value = g_enum_get_value (enum_class, build_msg->type);

  g_print ("Build message:\n");
  g_print ("\ttype: %s\n", enum_value->value_nick);
  g_print ("\ttext: %s\n", build_msg->text);
  g_print ("\tfilename: %s\n", build_msg->filename);
  g_print ("\tstart line: %d\n", build_msg->start_line);
  g_print ("\tend line: %d\n", build_msg->end_line);
  g_print ("\texpand: %d\n", build_msg->expand);
  g_print ("\n");

  g_type_class_unref (enum_class);
}

static const gchar *
get_icon_name_from_state (LatexilaBuildState state)
{
  switch (state)
    {
    case LATEXILA_BUILD_STATE_RUNNING:
      return "system-run";

    case LATEXILA_BUILD_STATE_SUCCEEDED:
      /* This stock item doesn't have an icon name replacement, but the stock-id
       * works fine. For GTK+ 4 it will probably be removed, so we can copy the
       * icon in the latexila sources.
       */
      G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      return GTK_STOCK_APPLY;
      G_GNUC_END_IGNORE_DEPRECATIONS

    case LATEXILA_BUILD_STATE_FAILED:
      return "dialog-error";

    case LATEXILA_BUILD_STATE_ABORTED:
      return "process-stop";

    default:
      g_return_val_if_reached (NULL);
    }
}

static const gchar *
get_icon_name_from_msg_type (LatexilaBuildMsgType type)
{
  switch (type)
    {
    case LATEXILA_BUILD_MSG_TYPE_JOB_SUB_COMMAND:
      return "gray-square";

    case LATEXILA_BUILD_MSG_TYPE_ERROR:
      return "dialog-error";

    case LATEXILA_BUILD_MSG_TYPE_WARNING:
      return "dialog-warning";

    case LATEXILA_BUILD_MSG_TYPE_BADBOX:
      return "badbox";

    default:
      return NULL;
    }
}

static void
latexila_build_view_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  LatexilaBuildView *build_view = LATEXILA_BUILD_VIEW (object);

  switch (prop_id)
    {
    case PROP_SHOW_WARNINGS:
      g_value_set_boolean (value, build_view->priv->show_warnings);
      break;

    case PROP_SHOW_BADBOXES:
      g_value_set_boolean (value, build_view->priv->show_badboxes);
      break;

    case PROP_SHOW_DETAILS:
      g_value_set_boolean (value, build_view->priv->show_details);
      break;

    case PROP_HAS_DETAILS:
      g_value_set_boolean (value, build_view->priv->has_details);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
latexila_build_view_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  LatexilaBuildView *build_view = LATEXILA_BUILD_VIEW (object);

  switch (prop_id)
    {
    case PROP_SHOW_WARNINGS:
      build_view->priv->show_warnings = g_value_get_boolean (value);

      if (build_view->priv->filtered_model != NULL)
        {
          gtk_tree_model_filter_refilter (build_view->priv->filtered_model);
        }
      break;

    case PROP_SHOW_BADBOXES:
      build_view->priv->show_badboxes = g_value_get_boolean (value);

      if (build_view->priv->filtered_model != NULL)
        {
          gtk_tree_model_filter_refilter (build_view->priv->filtered_model);
        }
      break;

    case PROP_SHOW_DETAILS:
      build_view->priv->show_details = g_value_get_boolean (value);
      break;

    case PROP_HAS_DETAILS:
      build_view->priv->has_details = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
latexila_build_view_dispose (GObject *object)
{
  LatexilaBuildView *build_view = LATEXILA_BUILD_VIEW (object);

  g_clear_object (&build_view->priv->store);
  g_clear_object (&build_view->priv->filtered_model);

  G_OBJECT_CLASS (latexila_build_view_parent_class)->dispose (object);
}

#if 0
static void
latexila_build_view_finalize (GObject *object)
{

  G_OBJECT_CLASS (latexila_build_view_parent_class)->finalize (object);
}
#endif

static void
latexila_build_view_class_init (LatexilaBuildViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = latexila_build_view_get_property;
  object_class->set_property = latexila_build_view_set_property;
  object_class->dispose = latexila_build_view_dispose;
  /*object_class->finalize = latexila_build_view_finalize;*/

  g_object_class_install_property (object_class,
                                   PROP_SHOW_WARNINGS,
                                   g_param_spec_boolean ("show-warnings",
                                                         "Show warnings",
                                                         "",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_SHOW_BADBOXES,
                                   g_param_spec_boolean ("show-badboxes",
                                                         "Show badboxes",
                                                         "",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_SHOW_DETAILS,
                                   g_param_spec_boolean ("show-details",
                                                         "Show details",
                                                         "",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class,
                                   PROP_HAS_DETAILS,
                                   g_param_spec_boolean ("has-details",
                                                         "Has details",
                                                         "",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_CONSTRUCT |
                                                         G_PARAM_STATIC_STRINGS));

  /**
   * LatexilaBuildView::jump-to-file:
   * @build_view: a #LatexilaBuildView.
   * @file: the file to open.
   * @start_line: the line where to jump and the start of the selection, or -1.
   * @end_line: the end of the selection, or -1.
   *
   * The ::jump-to-file signal is emitted when a row in the build view is
   * selected. The row must contain a file, otherwise the signal is not emitted.
   * The file should be opened and presented to the user. If @start_line and
   * @end_line are not -1, jump to the @start_line and select those lines. If
   * @start_line is provided, @end_line is also provided (different than -1).
   */
  signals[SIGNAL_JUMP_TO_FILE] = g_signal_new ("jump-to-file",
                                               LATEXILA_TYPE_BUILD_VIEW,
                                               G_SIGNAL_RUN_LAST,
                                               0, NULL, NULL, NULL,
                                               G_TYPE_NONE,
                                               3,
                                               G_TYPE_FILE,
                                               G_TYPE_INT,
                                               G_TYPE_INT);
}

static gboolean
visible_func (GtkTreeModel *model,
              GtkTreeIter  *iter,
              gpointer      user_data)
{
  LatexilaBuildMsgType msg_type;
  LatexilaBuildView *build_view = user_data;

  gtk_tree_model_get (model, iter,
                      COLUMN_MESSAGE_TYPE, &msg_type,
                      -1);

  switch (msg_type)
    {
    case LATEXILA_BUILD_MSG_TYPE_WARNING:
      return build_view->priv->show_warnings;

    case LATEXILA_BUILD_MSG_TYPE_BADBOX:
      return build_view->priv->show_badboxes;

    default:
      return TRUE;
    }
}

static void
init_tree_models (LatexilaBuildView *build_view)
{
  g_assert (build_view->priv->store == NULL);

  build_view->priv->store = gtk_tree_store_new (NB_COLUMNS,
                                                G_TYPE_STRING,    /* icon name */
                                                G_TYPE_STRING,    /* message */
                                                LATEXILA_TYPE_BUILD_MSG_TYPE,
                                                G_TYPE_INT,       /* weight (normal or bold) */
                                                G_TYPE_STRING,    /* basename */
                                                G_TYPE_STRING,    /* path */
                                                G_TYPE_FILE,
                                                G_TYPE_INT,       /* start line */
                                                G_TYPE_INT,       /* end line */
                                                G_TYPE_STRING);   /* line (same as start line but for display) */

  build_view->priv->filtered_model = GTK_TREE_MODEL_FILTER (
    gtk_tree_model_filter_new (GTK_TREE_MODEL (build_view->priv->store), NULL));

  gtk_tree_model_filter_set_visible_func (build_view->priv->filtered_model,
                                          visible_func,
                                          build_view,
                                          NULL);
}

/* Returns TRUE to select the row. */
static gboolean
select_row (LatexilaBuildView *build_view,
            GtkTreeModel      *model,
            GtkTreePath       *path)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (build_view);
  GtkTreeIter iter;
  GFile *file;
  gint start_line;
  gint end_line;

  if (!gtk_tree_model_get_iter (model, &iter, path))
    {
      return FALSE;
    }

  if (gtk_tree_model_iter_has_child (model, &iter))
    {
      if (gtk_tree_view_row_expanded (tree_view, path))
        {
          gtk_tree_view_collapse_row (tree_view, path);
        }
      else
        {
          gtk_tree_view_expand_to_path (tree_view, path);
        }

      return FALSE;
    }

  gtk_tree_model_get (model, &iter,
                      COLUMN_FILE, &file,
                      COLUMN_START_LINE, &start_line,
                      COLUMN_END_LINE, &end_line,
                      -1);

  if (file != NULL)
    {
      g_signal_emit (build_view,
                     signals[SIGNAL_JUMP_TO_FILE],
                     0,
                     file,
                     start_line,
                     end_line);

      g_object_unref (file);
    }

  /* Select the row, so the user can copy/paste its contents. */
  return TRUE;
}

static gboolean
select_func (GtkTreeSelection *selection,
             GtkTreeModel     *model,
             GtkTreePath      *path,
             gboolean          path_currently_selected,
             gpointer          user_data)
{
  LatexilaBuildView *build_view = user_data;

  if (path_currently_selected)
    {
      /* Always allow deselect */
      return TRUE;
    }

  return select_row (build_view, model, path);
}

static void
row_activated_cb (LatexilaBuildView *build_view,
                  GtkTreePath       *path)
{
  select_row (build_view,
              GTK_TREE_MODEL (build_view->priv->filtered_model),
              path);
}

static void
init_tree_view (LatexilaBuildView *build_view)
{
  GtkTreeView *tree_view = GTK_TREE_VIEW (build_view);
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeSelection *selection;

  gtk_tree_view_set_model (tree_view, GTK_TREE_MODEL (build_view->priv->store));
  gtk_tree_view_set_headers_visible (tree_view, FALSE);

  /* Columns, cell renderers */

  column = gtk_tree_view_column_new ();

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer, "icon-name", COLUMN_ICON);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "weight-set", TRUE,
                "editable", TRUE,
                "editable-set", TRUE,
                NULL);

  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_add_attribute (column, renderer, "text", COLUMN_MESSAGE);
  gtk_tree_view_column_add_attribute (column, renderer, "weight", COLUMN_WEIGHT);

  gtk_tree_view_append_column (tree_view, column);

  gtk_tree_view_insert_column_with_attributes (tree_view, -1, NULL,
                                               gtk_cell_renderer_text_new (),
                                               "text", COLUMN_BASENAME,
                                               NULL);

  gtk_tree_view_insert_column_with_attributes (tree_view, -1, NULL,
                                               gtk_cell_renderer_text_new (),
                                               "text", COLUMN_LINE_STR,
                                               NULL);

  gtk_tree_view_set_tooltip_column (tree_view, COLUMN_PATH);

  /* Selection */

  selection = gtk_tree_view_get_selection (tree_view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  gtk_tree_selection_set_select_function (selection, select_func, build_view, NULL);

  /* Double-click */

  g_signal_connect (build_view,
                    "row-activated",
                    G_CALLBACK (row_activated_cb),
                    NULL);
}

static void
latexila_build_view_init (LatexilaBuildView *build_view)
{
  build_view->priv = latexila_build_view_get_instance_private (build_view);

  init_tree_models (build_view);
  init_tree_view (build_view);
}

/**
 * latexila_build_view_new:
 *
 * Returns: a new #LatexilaBuildView.
 */
LatexilaBuildView *
latexila_build_view_new (void)
{
  return g_object_new (LATEXILA_TYPE_BUILD_VIEW, NULL);
}

/**
 * latexila_build_view_clear:
 * @build_view: a #LatexilaBuildView.
 *
 * Clears the build view.
 */
void
latexila_build_view_clear (LatexilaBuildView *build_view)
{
  GtkTreeSelection *selection;

  g_return_if_fail (LATEXILA_IS_BUILD_VIEW (build_view));

  /* No selection allowed when clearing the GtkTreeStore. Else, all the rows are
   * selected, and if there are warnings or errors, the program jumps to all
   * warnings/errors one by one. It's fun, but after four or five times, it
   * becomes less fun because our text cursor moves all the time ;) Another
   * means would have been to remove and re-add the select function, but I
   * prefer this hack, shorter :)
   */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (build_view));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);
  gtk_tree_store_clear (build_view->priv->store);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

  gtk_tree_view_columns_autosize (GTK_TREE_VIEW (build_view));

  build_view->priv->has_details = FALSE;
  g_object_notify (G_OBJECT (build_view), "has-details");
}

static GtkTreeIter
add_title (LatexilaBuildView    *build_view,
           const gchar          *message,
           LatexilaBuildState    state,
           LatexilaBuildMsgType  type)
{
  gboolean bold = type == LATEXILA_BUILD_MSG_TYPE_MAIN_TITLE;
  GtkTreeIter iter;
  GtkTreePath *path;

  gtk_tree_store_append (build_view->priv->store, &iter, NULL);
  gtk_tree_store_set (build_view->priv->store, &iter,
                      COLUMN_ICON, get_icon_name_from_state (state),
                      COLUMN_MESSAGE, message,
                      COLUMN_MESSAGE_TYPE, type,
                      COLUMN_WEIGHT, bold ? 800 : 400,
                      -1);

  path = gtk_tree_model_get_path (GTK_TREE_MODEL (build_view->priv->store), &iter);
  gtk_tree_view_expand_to_path (GTK_TREE_VIEW (build_view), path);
  gtk_tree_path_free (path);

  return iter;
}

/**
 * latexila_build_view_add_main_title:
 * @build_view: a #LatexilaBuildView.
 * @main_title: the title.
 * @state: the current state of the build tool.
 *
 * Adds a build tool title. There should be only one main title.
 *
 * Returns: the title ID as a #GtkTreeIter.
 */
GtkTreeIter
latexila_build_view_add_main_title (LatexilaBuildView  *build_view,
                                    const gchar        *main_title,
                                    LatexilaBuildState  state)
{
  return add_title (build_view, main_title, state, LATEXILA_BUILD_MSG_TYPE_MAIN_TITLE);
}

/**
 * latexila_build_view_add_job_title:
 * @build_view: a #LatexilaBuildView.
 * @job_title: the title.
 * @state: the current state of the build job.
 *
 * Adds a build job title.
 *
 * Returns: the title ID as a #GtkTreeIter.
 */
GtkTreeIter
latexila_build_view_add_job_title (LatexilaBuildView  *build_view,
                                   const gchar        *job_title,
                                   LatexilaBuildState  state)
{
  return add_title (build_view, job_title, state, LATEXILA_BUILD_MSG_TYPE_JOB_TITLE);
}

/**
 * latexila_build_view_set_title_state:
 * @build_view: a #LatexilaBuildView.
 * @title_id: the title ID as a #GtkTreeIter.
 * @state: the new state.
 *
 * Changes the build state of a title, represented as an icon.
 * If @title_id is the main title, @state is for the whole build tool. If
 * @title_id is for a job title, @state is for the build job.
 */
void
latexila_build_view_set_title_state (LatexilaBuildView  *build_view,
                                     GtkTreeIter        *title_id,
                                     LatexilaBuildState  state)
{
  g_return_if_fail (LATEXILA_IS_BUILD_VIEW (build_view));
  g_return_if_fail (title_id != NULL);

  gtk_tree_store_set (build_view->priv->store, title_id,
                      COLUMN_ICON, get_icon_name_from_state (state),
                      -1);
}

/**
 * latexila_build_view_append_single_message:
 * @build_view: a #LatexilaBuildView.
 * @parent: the parent row in the tree.
 * @message: the build message structure.
 *
 * Appends a single message to the build view.
 *
 * Returns: the #GtkTreeIter where the message has been added.
 */
GtkTreeIter
latexila_build_view_append_single_message (LatexilaBuildView *build_view,
                                           GtkTreeIter       *parent,
                                           LatexilaBuildMsg  *message)
{
  GFile *file = NULL;
  gchar *path = NULL;
  gchar *basename = NULL;
  gchar *line_str = NULL;
  gint end_line;
  GtkTreeIter iter;

  if (message->filename != NULL)
    {
      gchar *filename_with_tilde;

      file = g_file_new_for_path (message->filename);

      filename_with_tilde = latexila_utils_replace_home_dir_with_tilde (message->filename);
      path = g_markup_escape_text (filename_with_tilde, -1);
      g_free (filename_with_tilde);

      basename = g_file_get_basename (file);
    }

  if (message->start_line != -1)
    {
      line_str = g_strdup_printf ("%d", message->start_line);
    }

  end_line = message->end_line;
  if (message->start_line != -1 && end_line == -1)
    {
      end_line = message->start_line + 1;
    }

  gtk_tree_store_append (build_view->priv->store, &iter, parent);
  gtk_tree_store_set (build_view->priv->store, &iter,
                      COLUMN_ICON, get_icon_name_from_msg_type (message->type),
                      COLUMN_MESSAGE, message->text,
                      COLUMN_MESSAGE_TYPE, message->type,
                      COLUMN_WEIGHT, 400,
                      COLUMN_BASENAME, basename,
                      COLUMN_FILE, file,
                      COLUMN_PATH, path,
                      COLUMN_START_LINE, message->start_line,
                      COLUMN_END_LINE, end_line,
                      COLUMN_LINE_STR, line_str,
                      -1);

  if (file != NULL)
    {
      g_object_unref (file);
    }

  g_free (path);
  g_free (basename);
  g_free (line_str);

  return iter;
}

/**
 * latexila_build_view_append_messages:
 * @build_view: a #LatexilaBuildView.
 * @parent: the parent row in the tree.
 * @messages: the tree of #LatexilaBuildMsg's to append.
 * @expand: whether to expand the @parent.
 *
 * Appends a tree of messages to the build view.
 */
void
latexila_build_view_append_messages (LatexilaBuildView *build_view,
                                     GtkTreeIter       *parent,
                                     const GNode       *messages,
                                     gboolean           expand)
{
  const GNode *node;

  for (node = messages; node != NULL; node = node->next)
    {
      GtkTreeIter child;
      LatexilaBuildMsg *build_msg = node->data;

      g_assert (build_msg != NULL);

      child = latexila_build_view_append_single_message (build_view, parent, build_msg);

      if (node->children != NULL)
        {
          latexila_build_view_append_messages (build_view,
                                               &child,
                                               node->children,
                                               build_msg->expand);
        }
    }

  if (expand)
    {
      GtkTreePath *path;
      path = gtk_tree_model_get_path (GTK_TREE_MODEL (build_view->priv->store), parent);
      gtk_tree_view_expand_to_path (GTK_TREE_VIEW (build_view), path);
      gtk_tree_path_free (path);
    }
}

/**
 * latexila_build_view_remove_children:
 * @build_view: a #LatexilaBuildView.
 * @parent: the row for which the children must be removed.
 *
 * Removes the children of @parent.
 */
void
latexila_build_view_remove_children (LatexilaBuildView *build_view,
                                     GtkTreeIter       *parent)
{
  GtkTreeIter child;
  GtkTreeModel *model;

  g_return_if_fail (LATEXILA_IS_BUILD_VIEW (build_view));

  model = GTK_TREE_MODEL (build_view->priv->store);
  if (!gtk_tree_model_iter_children (model, &child, parent))
    {
      return;
    }

  while (gtk_tree_store_remove (build_view->priv->store, &child));
}
