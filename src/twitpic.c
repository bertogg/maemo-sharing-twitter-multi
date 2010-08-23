/*
 * This file is part of maemo-sharing-twitpic
 *
 * Copyright (C) 2010 Igalia, S.L.
 * Authors: Alberto Garcia <agarcia@igalia.com>
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>
 */

#include "twitpic.h"
#include "util.h"

#include <hildon-mime.h>
#include <sharing-http.h>
#include <libxml/parser.h>

#define TWITPIC_API_KEY                 "1f9ce28260728df0a40cafe3506a9407"

static gchar *
parse_server_response                   (const gchar       *response,
                                         gsize              size,
                                         TwitterPicService  service)
{
  xmlDoc *doc;
  const gchar *rootid, *urlid;
  gchar *url = NULL;

  g_return_val_if_fail (response != NULL, NULL);

  switch (service)
    {
    case SERVICE_TWITPIC:
      rootid = "image";
      urlid  = "url";
      break;
    case SERVICE_TWITGOO:
      rootid = "rsp";
      urlid  = "mediaurl";
      break;
    }

  doc = xmlParseMemory (response, size);
  if (doc != NULL)
    {
      xmlNode *iter, *root = NULL;

      for (iter = xmlDocGetRootElement (doc); iter && !root; iter = iter->next)
        if (xmlStrEqual (iter->name, (xmlChar *) rootid))
          root = iter;

      if (root != NULL)
        for (iter = root->xmlChildrenNode; iter && !url; iter = iter->next)
          if (xmlStrEqual (iter->name, (xmlChar *) urlid))
            {
              xmlChar *s = xmlNodeGetContent (iter);
              url = g_strdup ((gchar *) s);
              xmlFree (s);
            }

      xmlFreeDoc (doc);
    }

  return url;
}

static void
open_auth_url                           (SharingAccount *account)
{
  gchar *url = twitter_get_auth_url (account);
  if (url)
    {
      hildon_uri_open (url, NULL, NULL);
      g_free (url);
    }
}

static void
register_account_clicked                (GtkWidget      *button,
                                         SharingAccount *account)
{
  open_auth_url (account);
  gtk_dialog_response (GTK_DIALOG (gtk_widget_get_toplevel (button)),
                       GTK_RESPONSE_ACCEPT);
}

static gboolean
twitpic_account_enter_pin               (SharingAccount *account,
                                         GtkWindow      *parent)
{
  gint response;
  gchar *pin = NULL;
  GtkWidget *d, *label, *entry, *hbox;

  g_return_val_if_fail (account != NULL, FALSE);

  d = gtk_dialog_new ();
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_window_set_title (GTK_WINDOW (d), "Account setup - Twitter PIN number");
  gtk_window_set_transient_for (GTK_WINDOW (d), parent);
  gtk_dialog_add_button (GTK_DIALOG (d), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);

  label = gtk_label_new ("Enter PIN number:");
  entry = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
  gtk_entry_set_width_chars (GTK_ENTRY (entry), 10);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (d)->vbox), hbox);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

  gtk_widget_show_all (d);
  response = gtk_dialog_run (GTK_DIALOG (d));

  if (response == GTK_RESPONSE_ACCEPT)
    pin = g_strstrip (g_strdup (gtk_entry_get_text (GTK_ENTRY (entry))));

  twitter_account_set_pin (account, pin);

  g_free (pin);
  gtk_widget_destroy (d);

  return (response == GTK_RESPONSE_ACCEPT);
}

gboolean
twitpic_account_setup                   (SharingAccount *account,
                                         GtkWindow      *parent)
{
  gint response;
  gboolean success = FALSE;
  GtkWidget *d, *label, *button;
  GtkContainer *vbox;

  d = gtk_dialog_new ();
  vbox = GTK_CONTAINER (GTK_DIALOG (d)->vbox);
  gtk_window_set_title (GTK_WINDOW (d), "Account setup - Twitpic");
  gtk_window_set_transient_for (GTK_WINDOW (d), parent);

  label = gtk_label_new ("You need to register a Twitter account using the web page.\n"
                         "If you don't have one, you'll be offered the option to do it.\n"
                         "You'll be given a PIN number to continue this setup.");
  button = gtk_button_new_with_label ("Register Twitter account");

  hildon_gtk_widget_set_theme_size (button, HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);

  gtk_container_add (vbox, label);
  gtk_container_add (vbox, button);

  g_signal_connect (button, "clicked", G_CALLBACK (register_account_clicked), account);

  gtk_widget_show_all (d);
  response = gtk_dialog_run (GTK_DIALOG (d));
  gtk_widget_destroy (d);

  if (response != GTK_RESPONSE_DELETE_EVENT)
    success = twitpic_account_enter_pin (account, parent);

  return success;
}


gboolean
twitpic_account_validate                (SharingAccount *account,
                                         gboolean       *dead_mans_switch)
{
  *dead_mans_switch = FALSE;
  return twitter_account_validate (account);
}

typedef struct {
  SharingTransfer *transfer;
  gboolean        *dead_mans_switch;
  guint64          size;
} UploadProgressData;

static gboolean
upload_progress_cb                      (SharingHTTP *http,
                                         guint64      bytes_sent,
                                         gpointer     user_data)
{
  UploadProgressData *data = user_data;
  gdouble progress = 0.0;
  *(data->dead_mans_switch) = FALSE;
  if (data->size > 0)
    progress = (gdouble) bytes_sent / data->size;
  sharing_transfer_set_progress (data->transfer, CLAMP (progress, 0.0, 1.0));
  return TRUE;
}

SharingPluginInterfaceSendResult
twitpic_share_file                      (SharingTransfer *transfer,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch)
{
  SharingPluginInterfaceSendResult retval;
  SharingEntry *entry;
  const GSList *l;
  TwitterPicService service = SERVICE_TWITPIC;

  retval = SHARING_SEND_SUCCESS;
  *dead_mans_switch = FALSE;
  sharing_transfer_set_progress (transfer, 0.0);

  entry = sharing_transfer_get_entry (transfer);
  l = sharing_entry_get_media (entry);

  if (g_strcmp0 (sharing_entry_get_option (entry, "service"), "twitgoo") == 0)
    service = SERVICE_TWITGOO;

  for (; l != NULL && retval == SHARING_SEND_SUCCESS; l = l->next)
    {
      const gchar *path;
      gchar *mime;
      SharingAccount *account;
      SharingEntryMedia *media = l->data;

      g_return_val_if_fail (media != NULL, SHARING_SEND_ERROR_UNKNOWN);

      path = sharing_entry_media_get_localpath (media);
      mime = sharing_entry_media_get_mime (media);
      account = sharing_entry_get_account (entry);

      if (path && mime && !sharing_entry_media_get_sent (media) &&
          twitter_account_validate (account))
        {
          const gchar *posturl;
          gchar *hdr, *title;
          SharingHTTP *http = sharing_http_new ();
          SharingHTTPRunResponse httpret;
          UploadProgressData data;

          data.transfer = transfer;
          data.dead_mans_switch = dead_mans_switch;
          data.size = sharing_entry_media_get_size (media);

          title = sharing_entry_media_get_title (media);
          if (title)
            g_strstrip (title);
          else
            title = g_strdup ("Photo: ");

          sharing_http_set_progress_callback (http, upload_progress_cb, &data);
          sharing_http_add_req_multipart_file (http, "media", path, mime);
          sharing_http_add_req_multipart_data (http, "message", title, -1, "text/plain");

          switch (service)
            {
            case SERVICE_TWITPIC:
              sharing_http_add_req_multipart_data (http, "key", TWITPIC_API_KEY, -1, "text/plain");
              posturl = "http://api.twitpic.com/2/upload.xml";
              break;
            case SERVICE_TWITGOO:
              posturl = "http://twitgoo.com/api/upload";
              break;
            default:
              g_return_val_if_reached (SHARING_SEND_ERROR_UNKNOWN);
            }

          hdr = twitter_get_verify_credentials_header (account);
          sharing_http_add_req_header_line (http, hdr);
          g_free (hdr);

          sharing_http_add_req_header_line (http, "X-Auth-Service-Provider: " TWITTER_VERIFY_CREDENTIALS_URL);
          httpret = sharing_http_run (http, posturl);

          switch (httpret)
            {
            case SHARING_HTTP_RUNRES_SUCCESS:
              {
                gsize len;
                const gchar *body;
                gchar *img_url;

                body = sharing_http_get_res_body (http, &len);
                img_url = parse_server_response (body, len, service);

                if (img_url != NULL)
                  {
                    gchar *tweet = g_strconcat (title, " ", img_url, NULL);

                    if (twitter_update_status (tweet, account))
                      sharing_entry_media_set_sent (media, TRUE);

                    g_free (img_url);
                    g_free (tweet);
                  }
                else
                  {
                    retval = SHARING_SEND_ERROR_UNKNOWN;
                  }
              }
              break;
            case SHARING_HTTP_RUNRES_CANCELLED:
              retval = SHARING_SEND_CANCELLED;
              break;
            case SHARING_HTTP_RUNRES_CONNECTION_PROBLEM:
              retval = SHARING_SEND_ERROR_CONNECTION;
              break;
            default:
              retval = SHARING_SEND_ERROR_UNKNOWN;
            }

          sharing_http_unref (http);
          g_free (title);
        }

      sharing_account_free (account);
      g_free (mime);
    }

  return retval;
}

SharingPluginInterfaceEditAccountResult
twitpic_account_edit                    (GtkWindow       *parent,
                                         SharingAccount  *account,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch)
{
  gint response;
  GtkWidget *d, *label;
  enum { RESPONSE_EDIT, RESPONSE_REMOVE };

  g_return_val_if_fail (account && dead_mans_switch, SHARING_EDIT_ACCOUNT_ERROR_UNKNOWN);

  d = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (d), "Edit account - Twitpic");
  gtk_window_set_transient_for (GTK_WINDOW (d), parent);
  gtk_dialog_add_button (GTK_DIALOG (d), GTK_STOCK_REMOVE, RESPONSE_REMOVE);
  gtk_dialog_add_button (GTK_DIALOG (d), GTK_STOCK_EDIT, RESPONSE_EDIT);

  label = gtk_label_new ("Press 'Edit' to open the Twitter web page.\n"
                         "After that, enter the PIN number here\n"
                         "to confirm your changes.");

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (d)->vbox), label);

  gtk_widget_show_all (d);
  response = gtk_dialog_run (GTK_DIALOG (d));
  gtk_widget_destroy (d);

  switch (response)
    {
    case RESPONSE_EDIT:
      open_auth_url (account);
      if (twitpic_account_enter_pin (account, parent))
        {
          return SHARING_EDIT_ACCOUNT_SUCCESS;
        }
      else
        {
          return SHARING_EDIT_ACCOUNT_CANCELLED;
        }
    case RESPONSE_REMOVE:
      return SHARING_EDIT_ACCOUNT_DELETE;
    case GTK_RESPONSE_DELETE_EVENT:
      return SHARING_EDIT_ACCOUNT_NOT_STARTED;
    }

  return SHARING_EDIT_ACCOUNT_ERROR_UNKNOWN;
}
