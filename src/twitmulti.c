/*
 * This file is part of sharing-twitter-multi
 *
 * Copyright (C) 2010 Igalia, S.L.
 * Authors: Alberto Garcia <agarcia@igalia.com>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 3 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>
 */

#include "twitmulti.h"
#include "util.h"

#include <hildon-mime.h>
#include <sharing-http.h>
#include <sharing-tag.h>
#include <libxml/parser.h>

#define TWITPIC_API_KEY                 "1f9ce28260728df0a40cafe3506a9407"
#define MOBYPICTURE_API_KEY             "L5RL7tAoAAsgZqKP"
#define YFROG_API_KEY                   "45ENOPRS1f5a98c9f89b523a8465d2a3ad0602a3"

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
    case SERVICE_IMGLY:
    case SERVICE_POSTEROUS:
      rootid = "image";
      urlid  = "url";
      break;
    case SERVICE_MOBYPICTURE:
    case SERVICE_TWITGOO:
    case SERVICE_YFROG:
      rootid = "rsp";
      urlid  = "mediaurl";
      break;
    default:
      g_return_val_if_reached (NULL);
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
open_auth_url_cb                        (const gchar *url,
                                         gpointer     data)
{
  if (url)
    hildon_uri_open (url, NULL, NULL);
  gtk_dialog_response (GTK_DIALOG (data), url ? GTK_RESPONSE_YES : GTK_RESPONSE_NO);
}

static gboolean
open_auth_url                           (SharingAccount  *account,
                                         GtkWindow       *parent,
                                         ConIcConnection *con)
{
  GtkWidget *d;
  gboolean response;

  d = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (d), "Opening web browser ...");
  gtk_window_set_transient_for (GTK_WINDOW (d), parent);
  hildon_gtk_window_set_progress_indicator (GTK_WINDOW (d), TRUE);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (d)->vbox),
                     gtk_label_new ("Opening web browser, please wait ..."));
  gtk_widget_show_all (d);

  twitter_get_auth_url (account, con, open_auth_url_cb, d);

  do
    {
      response = gtk_dialog_run (GTK_DIALOG (d));
    }
  while (response != GTK_RESPONSE_YES && response != GTK_RESPONSE_NO);

  gtk_widget_destroy (d);

  return (response == GTK_RESPONSE_YES);
}

static void
register_account_clicked                (GtkWidget *button,
                                         gpointer   data)
{
  gtk_dialog_response (GTK_DIALOG (gtk_widget_get_toplevel (button)),
                       GTK_RESPONSE_ACCEPT);
}

static void
text_changed_cb                         (GObject    *entry,
                                         GParamSpec *pspec,
                                         gpointer    data)
{
  const gchar *text = gtk_entry_get_text (GTK_ENTRY (entry));
  while (*text != '\0' && g_ascii_isspace (*text))
    text++;
  gtk_dialog_set_response_sensitive (GTK_DIALOG (data), GTK_RESPONSE_ACCEPT,
                                     *text != '\0');
}

static gboolean
twitmulti_account_enter_pin             (SharingAccount *account,
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
  gtk_dialog_set_response_sensitive (GTK_DIALOG (d), GTK_RESPONSE_ACCEPT, FALSE);

  label = gtk_label_new ("Enter PIN number:");
  entry = hildon_entry_new (HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);
  gtk_entry_set_width_chars (GTK_ENTRY (entry), 10);

  g_signal_connect (entry, "notify::text", G_CALLBACK (text_changed_cb), d);

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
twitmulti_account_setup                 (SharingAccount *account,
                                         GtkWindow      *parent)
{
  gint response;
  gboolean success = FALSE;
  GtkWidget *d, *label, *button;
  GtkContainer *vbox;

  d = gtk_dialog_new ();
  vbox = GTK_CONTAINER (GTK_DIALOG (d)->vbox);
  gtk_window_set_title (GTK_WINDOW (d), "Account setup - Twitter");
  gtk_window_set_transient_for (GTK_WINDOW (d), parent);

  label = gtk_label_new ("Open the Twitter web page in the browser using the button below\n"
                         "and authorize the Nokia N900 to post to your account.\n\n"
                         "After that, you'll see a PIN number. You have to come back here\n"
                         "and enter that number to continue this setup process.\n\n"
                         "If you don't have a Twitter account yet, you'll be offered\n"
                         "the option to create one.");
  button = gtk_button_new_with_label ("Open Twitter web page");

  hildon_gtk_widget_set_theme_size (button, HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);

  gtk_container_add (vbox, label);
  gtk_container_add (vbox, button);

  g_signal_connect (button, "clicked", G_CALLBACK (register_account_clicked), NULL);

  gtk_widget_show_all (d);
  response = gtk_dialog_run (GTK_DIALOG (d));
  gtk_widget_destroy (d);

  if (response == GTK_RESPONSE_ACCEPT)
    {
      ConIcConnection *con = con_ic_connection_new ();
      if (open_auth_url (account, parent, con))
        success = twitmulti_account_enter_pin (account, parent);
      g_object_unref (con);
    }

  return success;
}


gboolean
twitmulti_account_validate              (SharingAccount *account,
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

TwitterPicService
get_twitter_pic_service                 (SharingEntry *entry)
{
  const gchar *servicename;
  TwitterPicService service = SERVICE_TWITPIC;

  servicename = sharing_entry_get_option (entry, "service");
  if (servicename)
    {
      if (g_str_equal (servicename, "twitgoo"))
        service = SERVICE_TWITGOO;
      else if (g_str_equal (servicename, "mobypicture"))
        service = SERVICE_MOBYPICTURE;
      else if (g_str_equal (servicename, "imgly"))
        service = SERVICE_IMGLY;
      else if (g_str_equal (servicename, "posterous"))
        service = SERVICE_POSTEROUS;
      else if (g_str_equal (servicename, "yfrog"))
        service = SERVICE_YFROG;
    }

  return service;
}

SharingPluginInterfaceSendResult
twitmulti_share_file                    (SharingTransfer *transfer,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch)
{
  SharingPluginInterfaceSendResult retval;
  SharingEntry *entry;
  const GSList *l;
  gboolean post_to_twitter = TRUE;
  TwitterPicService service = SERVICE_TWITPIC;

  retval = SHARING_SEND_SUCCESS;
  *dead_mans_switch = FALSE;
  sharing_transfer_set_progress (transfer, 0.0);

  entry = sharing_transfer_get_entry (transfer);
  l = sharing_entry_get_media (entry);

  service = get_twitter_pic_service (entry);

  if (g_strcmp0 (sharing_entry_get_option (entry, "posttotwitter"), "no") == 0)
    post_to_twitter = FALSE;

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

      if (sharing_entry_media_get_sent (media))
        {
          /* Do nothing, this file has already been sent */
        }
      else if (!path || !mime)
        {
          /* This is not supposed to happen... */
          retval = SHARING_SEND_ERROR_UNKNOWN;
        }
      else if (!twitter_account_validate (account))
        {
          retval = SHARING_SEND_ERROR_AUTH;
        }
      else
        {
          const gchar *posturl;
          const gchar *verify_url = TWITTER_VERIFY_CREDENTIALS_JSON;
          gchar *hdr, *title;
          const gchar *description;
          SharingHTTP *http = sharing_http_new ();
          SharingHTTPRunResponse httpret;
          UploadProgressData data;

          data.transfer = transfer;
          data.dead_mans_switch = dead_mans_switch;
          data.size = sharing_entry_media_get_size (media);

          title = sharing_entry_media_get_title (media);
          description = sharing_entry_media_get_desc (media);

          if (title)
            {
              g_strstrip (title);
            }
          else if (description)
            {
              /* If the title field is empty, use the description instead */
              title = g_strstrip (g_strdup (description));
            }

          /* We support Mobypicture tags */
          if (service == SERVICE_MOBYPICTURE)
            {
              const GSList *taglist = sharing_entry_media_get_tags (media);
              if (taglist != NULL)
                {
                  GString *tags = g_string_sized_new (100);
                  while (taglist != NULL)
                    {
                      SharingTag *tag = (SharingTag *) taglist->data;
                      g_string_append (tags, sharing_tag_get_word (tag));
                      taglist = taglist->next;
                      if (taglist != NULL)
                        g_string_append_c (tags, ',');
                    }
                  sharing_http_add_req_multipart_data (http, "tags", tags->str, -1, "text/plain");
                  g_string_free (tags, TRUE);
                }
            }

          sharing_http_set_progress_callback (http, upload_progress_cb, &data);
          sharing_http_add_req_multipart_file (http, "media", path, mime);
          if (title)
            sharing_http_add_req_multipart_data (http, "message", title, -1, "text/plain");

          switch (service)
            {
            case SERVICE_TWITPIC:
              sharing_http_add_req_multipart_data (http, "key", TWITPIC_API_KEY, -1, "text/plain");
              posturl = "http://api.twitpic.com/2/upload.xml";
              break;
            case SERVICE_MOBYPICTURE:
              if (description)
                sharing_http_add_req_multipart_data (http, "description", description, -1, "text/plain");
              sharing_http_add_req_multipart_data (http, "key", MOBYPICTURE_API_KEY, -1, "text/plain");
              posturl = "https://api.mobypicture.com/2.0/upload.xml";
              break;
            case SERVICE_TWITGOO:
              posturl = "http://twitgoo.com/api/upload";
              break;
            case SERVICE_IMGLY:
              posturl = "http://img.ly/api/2/upload.xml";
              break;
            case SERVICE_POSTEROUS:
              posturl = "https://posterous.com/api2/upload.xml";
              break;
            case SERVICE_YFROG:
              sharing_http_add_req_multipart_data (http, "key", YFROG_API_KEY, -1, "text/plain");
              posturl = "https://yfrog.com/api/xauth_upload";
              verify_url = TWITTER_VERIFY_CREDENTIALS_XML;
              break;
            default:
              g_return_val_if_reached (SHARING_SEND_ERROR_UNKNOWN);
            }

          hdr = twitter_get_verify_credentials_header (account, verify_url);
          sharing_http_add_req_header_line (http, hdr);
          g_free (hdr);

          sharing_http_add_req_header (http, "X-Auth-Service-Provider", verify_url);
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
                    gchar *tweet = g_strconcat (title ? title : "Photo:", " ", img_url, NULL);

                    if (!post_to_twitter || twitter_update_status (tweet, account))
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

static gboolean
clear_switch                            (gpointer data)
{
  gboolean *dead_mans_switch = data;
  *dead_mans_switch = FALSE;
  return TRUE;
}

SharingPluginInterfaceEditAccountResult
twitmulti_account_edit                  (GtkWindow       *parent,
                                         SharingAccount  *account,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch)
{
  SharingPluginInterfaceEditAccountResult retval = SHARING_EDIT_ACCOUNT_ERROR_UNKNOWN;
  gint response;
  GtkWidget *d, *label;
  enum { RESPONSE_EDIT, RESPONSE_REMOVE };
  guint timeout_id;

  g_return_val_if_fail (account && dead_mans_switch, retval);

  timeout_id = gdk_threads_add_timeout_seconds (20, clear_switch, dead_mans_switch);

  d = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (d), "Edit account - Twitter");
  gtk_window_set_transient_for (GTK_WINDOW (d), parent);
  gtk_dialog_add_button (GTK_DIALOG (d), GTK_STOCK_REMOVE, RESPONSE_REMOVE);
  gtk_dialog_add_button (GTK_DIALOG (d), GTK_STOCK_EDIT, RESPONSE_EDIT);

  label = gtk_label_new ("Press 'Edit' to open the Twitter web page\n"
                         "and authorize the Nokia N900 again.\n\n"
                         "After that, you'll see a PIN number.\n"
                         "Enter the PIN here to confirm your changes.");

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (d)->vbox), label);

  gtk_widget_show_all (d);
  response = gtk_dialog_run (GTK_DIALOG (d));
  gtk_widget_destroy (d);

  switch (response)
    {
    case RESPONSE_EDIT:
      if (open_auth_url (account, parent, con) &&
          twitmulti_account_enter_pin (account, parent))
        {
          retval = SHARING_EDIT_ACCOUNT_SUCCESS;
        }
      else
        {
          retval = SHARING_EDIT_ACCOUNT_CANCELLED;
        }
      break;
    case RESPONSE_REMOVE:
      retval = SHARING_EDIT_ACCOUNT_DELETE;
      break;
    case GTK_RESPONSE_DELETE_EVENT:
      retval = SHARING_EDIT_ACCOUNT_NOT_STARTED;
      break;
    default:
      g_return_val_if_reached (SHARING_EDIT_ACCOUNT_ERROR_UNKNOWN);
    }

  g_source_remove (timeout_id);

  return retval;
}
