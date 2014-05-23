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

#ifndef __LATEXILA_BUILD_VIEW_H__
#define __LATEXILA_BUILD_VIEW_H__

#include <gtk/gtk.h>
#include "latexila-types.h"

G_BEGIN_DECLS

#define LATEXILA_TYPE_BUILD_VIEW             (latexila_build_view_get_type ())
#define LATEXILA_BUILD_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LATEXILA_TYPE_BUILD_VIEW, LatexilaBuildView))
#define LATEXILA_BUILD_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LATEXILA_TYPE_BUILD_VIEW, LatexilaBuildViewClass))
#define LATEXILA_IS_BUILD_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LATEXILA_TYPE_BUILD_VIEW))
#define LATEXILA_IS_BUILD_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LATEXILA_TYPE_BUILD_VIEW))
#define LATEXILA_BUILD_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LATEXILA_TYPE_BUILD_VIEW, LatexilaBuildViewClass))

typedef struct _LatexilaBuildMsg         LatexilaBuildMsg;
typedef struct _LatexilaBuildViewClass   LatexilaBuildViewClass;
typedef struct _LatexilaBuildViewPrivate LatexilaBuildViewPrivate;

/**
 * LatexilaBuildState:
 * @LATEXILA_BUILD_STATE_RUNNING: running.
 * @LATEXILA_BUILD_STATE_SUCCEEDED: succeeded.
 * @LATEXILA_BUILD_STATE_FAILED: failed.
 * @LATEXILA_BUILD_STATE_ABORTED: aborted.
 */
typedef enum
{
  LATEXILA_BUILD_STATE_RUNNING,
  LATEXILA_BUILD_STATE_SUCCEEDED,
  LATEXILA_BUILD_STATE_FAILED,
  LATEXILA_BUILD_STATE_ABORTED
} LatexilaBuildState;

/**
 * LatexilaBuildMsgType:
 * @LATEXILA_BUILD_MSG_TYPE_MAIN_TITLE: main title.
 * @LATEXILA_BUILD_MSG_TYPE_JOB_TITLE: build job title.
 * @LATEXILA_BUILD_MSG_TYPE_JOB_SUB_COMMAND: build job sub-command.
 * @LATEXILA_BUILD_MSG_TYPE_ERROR: error.
 * @LATEXILA_BUILD_MSG_TYPE_WARNING: warning.
 * @LATEXILA_BUILD_MSG_TYPE_BADBOX: badbox.
 * @LATEXILA_BUILD_MSG_TYPE_INFO: other info.
 */
typedef enum
{
  LATEXILA_BUILD_MSG_TYPE_MAIN_TITLE,
  LATEXILA_BUILD_MSG_TYPE_JOB_TITLE,
  LATEXILA_BUILD_MSG_TYPE_JOB_SUB_COMMAND,
  LATEXILA_BUILD_MSG_TYPE_ERROR,
  LATEXILA_BUILD_MSG_TYPE_WARNING,
  LATEXILA_BUILD_MSG_TYPE_BADBOX,
  LATEXILA_BUILD_MSG_TYPE_INFO
} LatexilaBuildMsgType;

/**
 * LatexilaBuildMsg:
 * @type: the message type.
 * @text: the text.
 * @filename: reference to a certain file.
 * @start_line: reference to a line in the file. -1 to unset.
 * @end_line: reference to a line in the file. -1 to unset.
 * @expand: if the message has children, whether to show them by default.
 *
 * A build message, one line in the #GtkTreeView. If a @filename is provided,
 * the file will be opened when the user clicks on the message. If @start_line
 * and @end_line are provided, the lines between the two positions will be
 * selected (or just @start_line will be selected if @end_line is -1).
 *
 * The @expand field assumes that a #LatexilaBuildMsg is included in a #GNode or
 * similar N-ary tree structure.
 */
struct _LatexilaBuildMsg
{
  LatexilaBuildMsgType type;
  gchar *text;
  gchar *filename;
  gint start_line;
  gint end_line;
  guint expand : 1;
};

struct _LatexilaBuildView
{
  GtkTreeView parent;

  LatexilaBuildViewPrivate *priv;
};

struct _LatexilaBuildViewClass
{
  GtkTreeViewClass parent_class;
};

LatexilaBuildMsg *    latexila_build_msg_new                        (void);

void                  latexila_build_msg_free                       (LatexilaBuildMsg *build_msg);

GType                 latexila_build_view_get_type                  (void) G_GNUC_CONST;

LatexilaBuildView *   latexila_build_view_new                       (void);

void                  latexila_build_view_clear                     (LatexilaBuildView *build_view);

GtkTreeIter           latexila_build_view_add_main_title            (LatexilaBuildView  *build_view,
                                                                     const gchar        *main_title,
                                                                     LatexilaBuildState  state);

GtkTreeIter           latexila_build_view_add_job_title             (LatexilaBuildView  *build_view,
                                                                     const gchar        *job_title,
                                                                     LatexilaBuildState  state);

void                  latexila_build_view_set_title_state           (LatexilaBuildView  *build_view,
                                                                     GtkTreeIter        *title_id,
                                                                     LatexilaBuildState  state);

GtkTreeIter           latexila_build_view_append_single_message     (LatexilaBuildView *build_view,
                                                                     GtkTreeIter       *parent,
                                                                     LatexilaBuildMsg  *message);

void                  latexila_build_view_append_messages           (LatexilaBuildView *build_view,
                                                                     GtkTreeIter       *parent,
                                                                     const GNode       *messages,
                                                                     gboolean           expand);

void                  latexila_build_view_remove_children           (LatexilaBuildView *build_view,
                                                                     GtkTreeIter       *parent);

G_END_DECLS

#endif /* __LATEXILA_BUILD_VIEW_H__ */
