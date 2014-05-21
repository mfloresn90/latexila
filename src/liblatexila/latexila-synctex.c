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
 * SECTION:synctex
 * @title: LatexilaSynctex
 * @short_description: SyncTeX support between LaTeXila and Evince
 *
 * The #LatexilaSynctex class (a singleton) implements the support of SyncTeX
 * between LaTeXila and the Evince PDF viewer. It is used to switch between the
 * source file(s) and the PDF, at the same position in the document. It is called
 * the forward search: source file -> PDF. And backward search: PDF -> source
 * file.
 *
 * D-Bus is used to communicate between LaTeXila and Evince. The implementation
 * uses the asynchronous gdbus generated functions.
 *
 * For the position, only the line is used, not the column. The column is a bit
 * buggy.
 */

#include "latexila-synctex.h"
#include <glib/gi18n.h>
#include "evince-gdbus-generated.h"
#include "latexila-utils.h"

struct _LatexilaSynctexPrivate
{
  /* PDF URI -> EvinceWindow object */
  GHashTable *evince_windows;
};

typedef struct
{
  GtkTextBuffer *buffer;
  GFile *buffer_location;
  gchar *pdf_uri;
} ForwardSearchData;

typedef struct
{
  gchar *pdf_uri;
  gchar *owner;
} ConnectEvinceWindowData;

enum
{
  SIGNAL_BACKWARD_SEARCH,
  LAST_SIGNAL
};

G_DEFINE_TYPE_WITH_PRIVATE (LatexilaSynctex, latexila_synctex, G_TYPE_OBJECT)

static LatexilaSynctex *instance = NULL;
static guint signals[LAST_SIGNAL];

static ForwardSearchData *
forward_search_data_new (void)
{
  return g_slice_new0 (ForwardSearchData);
}

static void
forward_search_data_free (ForwardSearchData *data)
{
  if (data != NULL)
    {
      g_clear_object (&data->buffer);
      g_clear_object (&data->buffer_location);
      g_free (data->pdf_uri);
      g_slice_free (ForwardSearchData, data);
    }
}

static ConnectEvinceWindowData *
connect_evince_window_data_new (void)
{
  return g_slice_new0 (ConnectEvinceWindowData);
}

static void
connect_evince_window_data_free (ConnectEvinceWindowData *data)
{
  if (data != NULL)
    {
      g_free (data->pdf_uri);
      g_free (data->owner);
      g_slice_free (ConnectEvinceWindowData, data);
    }
}

static void
latexila_synctex_dispose (GObject *object)
{
  LatexilaSynctex *synctex = LATEXILA_SYNCTEX (object);

  if (synctex->priv->evince_windows != NULL)
    {
      g_hash_table_unref (synctex->priv->evince_windows);
      synctex->priv->evince_windows = NULL;
    }

  G_OBJECT_CLASS (latexila_synctex_parent_class)->dispose (object);
}

static void
latexila_synctex_class_init (LatexilaSynctexClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = latexila_synctex_dispose;

  /**
   * LatexilaSynctex::backward-search:
   * @synctex: the #LatexilaSynctex instance.
   * @tex_uri: the *.tex file URI.
   * @line: the line to jump to.
   * @timestamp: timestamp of the event.
   *
   * The ::backward-search signal is emitted to perform a backward search, i.e.
   * switching from the PDF to the source *.tex file.
   */
  signals[SIGNAL_BACKWARD_SEARCH] = g_signal_new ("backward-search",
                                                  LATEXILA_TYPE_SYNCTEX,
                                                  G_SIGNAL_RUN_LAST,
                                                  0, NULL, NULL, NULL,
                                                  G_TYPE_NONE, 3,
                                                  G_TYPE_STRING,
                                                  G_TYPE_INT,
                                                  G_TYPE_UINT);
}

static void
latexila_synctex_init (LatexilaSynctex *synctex)
{
  synctex->priv = latexila_synctex_get_instance_private (synctex);

  synctex->priv->evince_windows = g_hash_table_new_full (g_str_hash,
                                                         g_str_equal,
                                                         g_free,
                                                         g_object_unref);
}

/**
 * latexila_synctex_get_instance:
 *
 * Returns: (transfer none): the #LatexilaSynctex singleton instance.
 */
LatexilaSynctex *
latexila_synctex_get_instance (void)
{
  if (instance == NULL)
    {
      instance = g_object_new (LATEXILA_TYPE_SYNCTEX, NULL);
    }

  return instance;
}

static void
show_warning (const gchar *message)
{
  GtkApplication *app;
  GtkWindow *parent;
  GtkWidget *dialog;

  app = GTK_APPLICATION (g_application_get_default ());
  parent = gtk_application_get_active_window (app);

  dialog = gtk_message_dialog_new (parent,
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_OK,
                                   "%s", _("Impossible to do the forward search."));

  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                            "%s", message);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

static gchar *
get_pdf_uri (GFile *main_tex_file)
{
  gchar *tex_uri;
  gchar *short_uri;
  gchar *pdf_uri;

  tex_uri = g_file_get_uri (main_tex_file);
  short_uri = latexila_utils_get_shortname (tex_uri);
  pdf_uri = g_strdup_printf ("%s.pdf", short_uri);

  g_free (tex_uri);
  g_free (short_uri);
  return pdf_uri;
}

static GVariant *
get_buffer_position (GtkTextBuffer *buffer)
{
  GtkTextIter iter;
  gint line;
  gint column;

  gtk_text_buffer_get_iter_at_mark (buffer,
                                    &iter,
                                    gtk_text_buffer_get_insert (buffer));

  line = gtk_text_iter_get_line (&iter) + 1;

  /* Ignore the column, it gives a better result. */
  column = -1;

  return g_variant_new ("(ii)", line, column);
}

static void
window_closed_cb (EvinceWindow *window,
                  const gchar  *pdf_uri)
{
  g_hash_table_remove (instance->priv->evince_windows, pdf_uri);
}

static void
sync_source_cb (EvinceWindow    *window,
                const gchar     *tex_uri,
                GVariant        *pos,
                guint            timestamp,
                LatexilaSynctex *synctex)
{
  gint line;
  gint column; /* not used */

  g_variant_get (pos, "(ii)", &line, &column);

  g_signal_emit (synctex,
                 signals[SIGNAL_BACKWARD_SEARCH], 0,
                 tex_uri,
                 line - 1,
                 timestamp);
}

static void
window_proxy_cb (GObject      *object,
                 GAsyncResult *result,
                 GTask        *task)
{
  EvinceWindow *window;
  ConnectEvinceWindowData *data;
  GError *error = NULL;

  window = evince_window_proxy_new_for_bus_finish (result, &error);

  if (error != NULL)
    {
      g_warning ("SyncTeX: can not connect to evince window: %s", error->message);
      g_task_return_boolean (task, FALSE);
      g_object_unref (task);
      g_error_free (error);
      return;
    }

  data = g_task_get_task_data (task);

  g_hash_table_insert (instance->priv->evince_windows, data->pdf_uri, window);

  g_signal_connect (window,
                    "closed",
                    G_CALLBACK (window_closed_cb),
                    data->pdf_uri);

  g_signal_connect (window,
                    "sync-source",
                    G_CALLBACK (sync_source_cb),
                    instance);

  data->pdf_uri = NULL;

  /* connect_evince_window_async() is finally finished! */
  g_task_return_boolean (task, TRUE);
  g_object_unref (task);
}

static void
get_window_list_cb (EvinceApplication *app,
                    GAsyncResult      *result,
                    GTask             *task)
{
  ConnectEvinceWindowData *data;
  gchar **window_list;
  gchar *window_path;
  GError *error = NULL;

  evince_application_call_get_window_list_finish (app, &window_list, result, &error);
  g_object_unref (app);

  if (error != NULL)
    {
      g_warning ("SyncTeX: can not get window list: %s", error->message);
      g_task_return_boolean (task, FALSE);
      g_object_unref (task);
      g_error_free (error);
      return;
    }

  if (window_list == NULL || window_list[0] == NULL)
    {
      g_warning ("SyncTeX: the window list is empty.");
      g_task_return_boolean (task, FALSE);
      g_object_unref (task);
      g_strfreev (window_list);
      return;
    }

  data = g_task_get_task_data (task);

  /* There is normally only one window. */
  window_path = window_list[0];

  evince_window_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                   G_DBUS_PROXY_FLAGS_NONE,
                                   data->owner,
                                   window_path,
                                   NULL,
                                   (GAsyncReadyCallback) window_proxy_cb,
                                   task);

  g_strfreev (window_list);
}

static void
application_proxy_cb (GObject      *object,
                      GAsyncResult *result,
                      GTask        *task)
{
  EvinceApplication *app;
  GError *error = NULL;

  app = evince_application_proxy_new_for_bus_finish (result, &error);

  if (error != NULL)
    {
      g_warning ("SyncTeX: can not connect to evince application: %s", error->message);
      g_task_return_boolean (task, FALSE);
      g_object_unref (task);
      g_error_free (error);
      return;
    }

  evince_application_call_get_window_list (app,
                                           NULL,
                                           (GAsyncReadyCallback) get_window_list_cb,
                                           task);
}

static void
find_document_cb (EvinceDaemon *daemon,
                  GAsyncResult *result,
                  GTask        *task)
{
  ConnectEvinceWindowData *data;
  GError *error = NULL;

  data = g_task_get_task_data (task);

  evince_daemon_call_find_document_finish (daemon, &data->owner, result, &error);
  g_object_unref (daemon);

  if (error != NULL)
    {
      g_warning ("SyncTeX: find document: %s", error->message);
      g_task_return_boolean (task, FALSE);
      g_object_unref (task);
      g_error_free (error);
      return;
    }

  evince_application_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        data->owner,
                                        "/org/gnome/evince/Evince",
                                        NULL,
                                        (GAsyncReadyCallback) application_proxy_cb,
                                        task);
}

static void
daemon_proxy_cb (GObject      *object,
                 GAsyncResult *result,
                 GTask        *task)
{
  EvinceDaemon *daemon;
  GError *error = NULL;
  ConnectEvinceWindowData *data;

  daemon = evince_daemon_proxy_new_for_bus_finish (result, &error);

  if (error != NULL)
    {
      g_warning ("SyncTeX: can not connect to the evince daemon: %s", error->message);
      g_task_return_boolean (task, FALSE);
      g_object_unref (task);
      g_error_free (error);
      return;
    }

  data = g_task_get_task_data (task);

  evince_daemon_call_find_document (daemon, data->pdf_uri, TRUE, NULL,
                                    (GAsyncReadyCallback) find_document_cb,
                                    task);
}

static void
connect_evince_window_async (LatexilaSynctex     *synctex,
                             const gchar         *pdf_uri,
                             GAsyncReadyCallback  callback,
                             gpointer             user_data)
{
  GTask *task;
  ConnectEvinceWindowData *data;

  g_return_if_fail (LATEXILA_IS_SYNCTEX (synctex));
  g_return_if_fail (pdf_uri != NULL);

  task = g_task_new (synctex, NULL, callback, user_data);

  if (g_hash_table_contains (synctex->priv->evince_windows, pdf_uri))
    {
      g_task_return_boolean (task, TRUE);
      g_object_unref (task);
      return;
    }

  data = connect_evince_window_data_new ();
  data->pdf_uri = g_strdup (pdf_uri);

  g_task_set_task_data (task, data, (GDestroyNotify) connect_evince_window_data_free);

  evince_daemon_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                   G_DBUS_PROXY_FLAGS_NONE,
                                   "org.gnome.evince.Daemon",
                                   "/org/gnome/evince/Daemon",
                                   NULL,
                                   (GAsyncReadyCallback) daemon_proxy_cb,
                                   task);
}

static void
connect_evince_window_finish (LatexilaSynctex *synctex,
                              GAsyncResult    *result)
{
  g_return_if_fail (g_task_is_valid (result, synctex));

  g_task_propagate_boolean (G_TASK (result), NULL);
}

/**
 * latexila_synctex_connect_evince_window:
 * @synctex: the #LatexilaSynctex instance.
 * @pdf_uri: the PDF URI.
 *
 * Connects asynchronously the evince window for @pdf_uri. LaTeXila will then
 * listen the signals emitted by the evince window when the user wants to switch
 * from the PDF to the corresponding *.tex file.
 */
void
latexila_synctex_connect_evince_window (LatexilaSynctex *synctex,
                                        const gchar     *pdf_uri)
{
  connect_evince_window_async (synctex,
                               pdf_uri,
                               (GAsyncReadyCallback) connect_evince_window_finish,
                               NULL);
}

static void
sync_view_cb (EvinceWindow      *evince_window,
              GAsyncResult      *result,
              ForwardSearchData *data)
{
  GError *error = NULL;

  evince_window_call_sync_view_finish (evince_window, result, &error);

  if (error != NULL)
    {
      g_warning ("SyncTeX: can not sync view: %s", error->message);
      g_error_free (error);
    }

  /* latexila_synctex_forward_search() finished! */
  forward_search_data_free (data);
}

static void
connect_evince_window_cb (LatexilaSynctex   *synctex,
                          GAsyncResult      *result,
                          ForwardSearchData *data)
{
  EvinceWindow *evince_window;
  gchar *buffer_path;

  connect_evince_window_finish (synctex, result);

  evince_window = g_hash_table_lookup (synctex->priv->evince_windows, data->pdf_uri);

  if (evince_window == NULL)
    {
      show_warning (_("Can not communicate with evince."));
      forward_search_data_free (data);
      return;
    }

  buffer_path = g_file_get_path (data->buffer_location);

  evince_window_call_sync_view (evince_window,
                                buffer_path,
                                get_buffer_position (data->buffer),
                                GDK_CURRENT_TIME,
                                NULL,
                                (GAsyncReadyCallback) sync_view_cb,
                                data);

  g_free (buffer_path);
}

static void
synctex_file_query_exists_cb (GFile             *synctex_file,
                              GAsyncResult      *result,
                              ForwardSearchData *data)
{
  gboolean synctex_file_exists;

  synctex_file_exists = latexila_utils_file_query_exists_finish (synctex_file, result);

  if (!synctex_file_exists)
    {
      gchar *basename = g_file_get_basename (synctex_file);
      gchar *message = g_strdup_printf (_("The file \"%s\" doesn't exist."), basename);

      show_warning (message);

      g_free (basename);
      g_free (message);
      g_object_unref (synctex_file);
      forward_search_data_free (data);
      return;
    }

  connect_evince_window_async (instance,
                               data->pdf_uri,
                               (GAsyncReadyCallback) connect_evince_window_cb,
                               data);

  g_object_unref (synctex_file);
}

static void
pdf_file_query_exists_cb (GFile             *pdf_file,
                          GAsyncResult      *result,
                          ForwardSearchData *data)
{
  gboolean pdf_file_exists;
  gchar *short_uri;
  gchar *synctex_uri;
  GFile *synctex_file;

  pdf_file_exists = latexila_utils_file_query_exists_finish (pdf_file, result);
  g_object_unref (pdf_file);

  if (!pdf_file_exists)
    {
      show_warning (_("The PDF file doesn't exist."));
      forward_search_data_free (data);
      return;
    }

  short_uri = latexila_utils_get_shortname (data->pdf_uri);
  synctex_uri = g_strdup_printf ("%s.synctex.gz", short_uri);
  synctex_file = g_file_new_for_uri (synctex_uri);
  g_free (short_uri);
  g_free (synctex_uri);

  latexila_utils_file_query_exists_async (synctex_file,
                                          NULL,
                                          (GAsyncReadyCallback) synctex_file_query_exists_cb,
                                          data);
}

/**
 * latexila_synctex_forward_search:
 * @synctex: the #LatexilaSynctex instance.
 * @buffer: a #GtkTextBuffer.
 * @buffer_location: the *.tex file of @buffer.
 * @main_tex_file: the main *.tex file of @buffer.
 *
 * Does a forward search, i.e. switch from the *.tex file to the PDF file at the
 * same position as the cursor position in @buffer.
 */
void
latexila_synctex_forward_search (LatexilaSynctex *synctex,
                                 GtkTextBuffer   *buffer,
                                 GFile           *buffer_location,
                                 GFile           *main_tex_file)
{
  ForwardSearchData *data;
  GFile *pdf_file;

  g_return_if_fail (LATEXILA_IS_SYNCTEX (synctex));
  g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (buffer_location == NULL || G_IS_FILE (buffer_location));
  g_return_if_fail (main_tex_file == NULL || G_IS_FILE (main_tex_file));

  if (buffer_location == NULL)
    {
      show_warning (_("The document is not saved."));
      return;
    }

  g_return_if_fail (G_IS_FILE (main_tex_file));

  data = forward_search_data_new ();
  data->buffer = g_object_ref (buffer);
  data->buffer_location = g_object_ref (buffer_location);
  data->pdf_uri = get_pdf_uri (main_tex_file);

  pdf_file = g_file_new_for_uri (data->pdf_uri);

  latexila_utils_file_query_exists_async (pdf_file,
                                          NULL,
                                          (GAsyncReadyCallback) pdf_file_query_exists_cb,
                                          data);
}
