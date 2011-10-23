/*
 * This file is part of sharing-twitter-multi
 *
 * Copyright (C) 2010-2011 Igalia, S.L.
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

#include "util.h"

#include <oauth.h>
#include <string.h>
#include <conicconnectionevent.h>

#define PARAM_ACCESS_TOKEN        "twitter-access-token"
#define PARAM_ACCESS_SECRET       "twitter-access-secret"
#define PARAM_REQUEST_TOKEN       "twitter-request-token"
#define PARAM_REQUEST_SECRET      "twitter-request-secret"
#define PARAM_PIN                 "twitter-pin"

#define CONSUMER_KEY              "WmyOdRu3svydhjw2SKgqZA"
#define CONSUMER_SECRET           "psF3jiC2uMVWG7P1sd2bVEhFmHmWJOUTcbvevqbrcc"

gboolean
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

static GHashTable *
parse_reply                             (const gchar *buffer,
                                         gsize        len)
{
  GHashTable *hash;
  gint i;
  gchar *buf2;
  gchar **lines;

  hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  if (len < 3)
    return hash;

  /* According to the doc, buffer might not be NULL terminated.
     I don't think this is likely here, but it's better to avoid surprises */
  buf2 = g_malloc (len + 1);
  memcpy (buf2, buffer, len);
  buf2[len] = '\0';

  lines = g_strsplit (buf2, "&", 0);

  for (i = 0; lines[i] != NULL; i++)
    {
      gchar **line = g_strsplit (lines[i], "=", 2);
      if (line[0] != NULL && line[1] != NULL)
        {
          gchar *key = g_strstrip (g_strdup (line[0]));
          gchar *val = g_strstrip (g_strdup (line[1]));
          if (key[0] != '\0' && val[0] != '\0')
            {
              g_hash_table_insert (hash, key, val);
            }
          else
            {
              g_free (key);
              g_free (val);
            }
        }
      g_strfreev (line);
    }
  g_strfreev (lines);

  g_free (buf2);

  return hash;
}

static void
string_replace                          (GString     *str,
                                         const gchar *old,
                                         const gchar *new)
{
  gint oldlen = strlen (old);
  gint newlen = strlen (new);
  const gchar *cur = str->str;
  while ((cur = strstr (cur, old)) != NULL)
    {
      gint position = cur - str->str;
      g_string_erase (str, position, oldlen);
      g_string_insert (str, position, new);
      cur = str->str + position + newlen;
    }
}

static gchar *
get_oauth_signature_valist              (const gchar *proto,
                                         const gchar *url,
                                         const gchar *access_secret,
                                         va_list      args)
{
  GString *base;
  gchar *retvalue;
  const gchar *param;

  base = g_string_sized_new (300);

  g_string_assign (base, proto);
  g_string_append_c (base, '&');
  g_string_append (base, url);
  string_replace (base, ":", "%3A");
  string_replace (base, "/", "%2F");
  g_string_append_c (base, '&');

  param = va_arg (args, gchar *);
  while (param != NULL)
    {
      const gchar *previous;
      const gchar *value = va_arg (args, gchar *);

      g_return_val_if_fail (value != NULL, NULL);

      g_string_append (base, param);
      g_string_append (base, "%3D");
      g_string_append (base, value);

      previous = param;
      param = va_arg (args, gchar *);

      if (param != NULL)
        {
          g_assert (g_strcmp0 (previous, param) <= 0);
          g_string_append (base, "%26");
        }
    }

  {
    gchar *secret, *str;

    secret = g_strconcat (CONSUMER_SECRET "&", access_secret, NULL);
    str = oauth_sign_hmac_sha1 (base->str, secret);
    g_free (secret);

    g_string_free (base, TRUE);

    retvalue = g_uri_escape_string (str, "", FALSE);
    g_free (str);
  }

  return retvalue;
}

static gchar *
get_oauth_signature                     (const gchar *proto,
                                         const gchar *url,
                                         const gchar *access_secret,
                                         ...)
{
  gchar *retvalue;
  va_list args;
  va_start (args, access_secret);
  retvalue = get_oauth_signature_valist (proto, url, access_secret, args);
  va_end (args);
  return retvalue;
}

/*
  Receives a URL and a NULL-terminated list of (param,value) and
  returns the full URL signed with the 'oauth_signature' parameter
 */
static gchar *
get_signed_url                          (const gchar *url,
                                         ...)
{
  GString *final_url;
  gchar *signature;
  const gchar *param;
  va_list args;

  final_url = g_string_sized_new (300);
  g_string_assign (final_url, url);
  g_string_append_c (final_url, '?');

  va_start (args, url);
  param = va_arg (args, gchar *);
  while (param != NULL)
    {
      const gchar *value = va_arg (args, gchar *);

      g_string_append (final_url, param);
      g_string_append_c (final_url, '=');
      g_string_append (final_url, value);

      param = va_arg (args, gchar *);
      g_string_append_c (final_url, '&');
    }
  va_end (args);

  va_start (args, url);
  signature = get_oauth_signature_valist ("GET", url, "", args);
  va_end (args);

  g_string_append (final_url, "oauth_signature=");
  g_string_append (final_url, signature);

  g_free (signature);

  return g_string_free (final_url, FALSE);
}

static gchar *
get_twitter_request_token_url           (void)
{
  gchar *timestamp, *nonce, *url;

  timestamp = g_strdup_printf ("%lu", time (NULL));
  nonce = oauth_gen_nonce ();

  url = get_signed_url (TWITTER_REQUEST_TOKEN_URL,
                        "oauth_callback", "oob",
                        "oauth_consumer_key", CONSUMER_KEY,
                        "oauth_nonce", nonce,
                        "oauth_signature_method", "HMAC-SHA1",
                        "oauth_timestamp", timestamp,
                        "oauth_version", "1.0",
                        NULL);

  g_free (timestamp);
  g_free (nonce);

  return url;
}

static gchar *
get_access_token_url                    (const gchar *request_token,
                                         const gchar *pin)
{
  gchar *timestamp, *nonce, *url;
  g_return_val_if_fail (request_token && pin, NULL);

  timestamp = g_strdup_printf ("%lu", time (NULL));
  nonce = oauth_gen_nonce ();

  url = get_signed_url (TWITTER_ACCESS_TOKEN_URL,
                        "oauth_callback", "oob",
                        "oauth_consumer_key", CONSUMER_KEY,
                        "oauth_nonce", nonce,
                        "oauth_signature_method", "HMAC-SHA1",
                        "oauth_timestamp", timestamp,
                        "oauth_token", request_token,
                        "oauth_verifier", pin,
                        "oauth_version", "1.0",
                        NULL);
  g_free (timestamp);
  g_free (nonce);

  return url;
}

static gboolean
get_twitter_access_token                (const gchar  *request_token,
                                         const gchar  *pin,
                                         gchar       **access_token,
                                         gchar       **access_secret,
                                         gchar       **screen_name)
{
  gchar *url;

  g_return_val_if_fail (request_token && pin, FALSE);
  g_return_val_if_fail (access_token && access_secret && screen_name, FALSE);

  *access_token = *access_secret = *screen_name = NULL;

  url = get_access_token_url (request_token, pin);

  if (url)
    {
      SharingHTTP *http;
      SharingHTTPRunResponse ret;
      http = sharing_http_new ();
      ret = sharing_http_run (http, url);

      if (ret == SHARING_HTTP_RUNRES_SUCCESS)
        {
          GHashTable *t;
          gsize len;
          const gchar *reply, *token, *secret, *name;

          reply = sharing_http_get_res_body (http, &len);
          t = parse_reply (reply, len);

          token  = g_hash_table_lookup (t, "oauth_token");
          secret = g_hash_table_lookup (t, "oauth_token_secret");
          name   = g_hash_table_lookup (t, "screen_name");

          if (token && secret && name)
            {
              *access_token  = g_strdup (token);
              *access_secret = g_strdup (secret);
              *screen_name   = g_strdup (name);
            }

          g_hash_table_destroy (t);
        }

      g_free (url);
      sharing_http_unref (http);
    }

  return (*access_token != NULL);
}

static gboolean
get_twitter_request_token               (gchar **token,
                                         gchar **secret)
{
  gchar *url;
  SharingHTTP *http;
  SharingHTTPRunResponse ret;

  g_return_val_if_fail (token != NULL && secret != NULL, FALSE);

  *token = *secret = NULL;

  url = get_twitter_request_token_url ();
  http = sharing_http_new ();
  ret = sharing_http_run (http, url);

  if (ret == SHARING_HTTP_RUNRES_SUCCESS)
    {
      GHashTable *t;
      gsize len;
      const gchar *reply, *oauth_token, *oauth_token_secret;

      reply = sharing_http_get_res_body (http, &len);
      t = parse_reply (reply, len);

      oauth_token = g_hash_table_lookup (t, "oauth_token");
      oauth_token_secret = g_hash_table_lookup (t, "oauth_token_secret");

      if (oauth_token && oauth_token_secret)
        {
          *token = g_strdup (oauth_token);
          *secret = g_strdup (oauth_token_secret);
        }

      g_hash_table_destroy (t);
    }

  g_free (url);
  sharing_http_unref (http);

  return (*token != NULL);
}

typedef struct {
  SharingAccount *account;
  gchar *token;
  gchar *secret;
  TwitterGetAuthUrlCb callback;
  gpointer cbdata;
  gboolean online;
} TwitterGetAuthUrlData;

static gboolean
twitter_get_auth_url_idle               (gpointer data)
{
  TwitterGetAuthUrlData *d = data;
  gchar *url = NULL;
  if (d->online && get_twitter_request_token (&(d->token), &(d->secret)))
    {
      url = g_strconcat (TWITTER_AUTHORIZE_URL, "?oauth_token=", d->token, NULL);
      sharing_account_set_param (d->account, PARAM_REQUEST_TOKEN, d->token);
      sharing_account_set_param (d->account, PARAM_REQUEST_SECRET, d->secret);
    }
  d->callback (url, d->cbdata);
  g_free (url);
  g_free (d->token);
  g_free (d->secret);
  g_slice_free (TwitterGetAuthUrlData, d);
  return FALSE;
}

static void
twitter_get_auth_url_connection         (ConIcConnection      *conn,
                                         ConIcConnectionEvent *event,
                                         gpointer              data)
{
  TwitterGetAuthUrlData *d = data;
  g_signal_handlers_disconnect_by_func (conn, twitter_get_auth_url_connection, data);
  d->online = con_ic_connection_event_get_status (event) == CON_IC_STATUS_CONNECTED;
  gdk_threads_add_timeout (500, twitter_get_auth_url_idle, d);
}

void
twitter_get_auth_url                    (SharingAccount      *account,
                                         ConIcConnection     *con,
                                         TwitterGetAuthUrlCb  callback,
                                         gpointer             cbdata)
{
  TwitterGetAuthUrlData *d;

  g_return_if_fail (account != NULL && callback != NULL);

  d = g_slice_new0 (TwitterGetAuthUrlData);
  d->account  = account;
  d->callback = callback;
  d->cbdata   = cbdata;

  g_signal_connect (con, "connection-event",
                    G_CALLBACK (twitter_get_auth_url_connection), d);
  con_ic_connection_connect (con, CON_IC_CONNECT_FLAG_NONE);
}

void
twitter_account_set_pin                 (SharingAccount *account,
                                         const gchar    *pin)
{
  g_return_if_fail (account != NULL);

  sharing_account_set_param (account, PARAM_PIN, pin);
  sharing_account_set_param (account, PARAM_ACCESS_TOKEN, NULL);
  sharing_account_set_param (account, PARAM_ACCESS_SECRET, NULL);
}

gboolean
twitter_account_validate                (SharingAccount *account)
{
  gboolean valid_account;

  {
    gchar *token, *secret, *name;

    token = sharing_account_get_param (account, PARAM_ACCESS_TOKEN);
    secret = sharing_account_get_param (account, PARAM_ACCESS_SECRET);
    name = sharing_account_get_username (account);

    /* An account with these three parameters is valid */
    valid_account = (token && secret && name);

    g_free (token);
    g_free (secret);
    g_free (name);
  }

  if (!valid_account)
    {
      gchar *req_token, *pin;

      req_token = sharing_account_get_param (account, PARAM_REQUEST_TOKEN);
      pin = sharing_account_get_param (account, PARAM_PIN);

      /* With the request token and the pin, we can obtain the access token */
      if (req_token && pin)
        {
          gchar *token, *secret, *name;

          get_twitter_access_token (req_token, pin, &token, &secret, &name);

          if (token && secret && name)
            {
              /* Store the new access token and the user name */
              sharing_account_set_param (account, PARAM_ACCESS_TOKEN, token);
              sharing_account_set_param (account, PARAM_ACCESS_SECRET, secret);
              sharing_account_set_username (account, name);

              /* Clear the request token and the pin, we don't need them anymore */
              sharing_account_set_param (account, PARAM_REQUEST_TOKEN, NULL);
              sharing_account_set_param (account, PARAM_REQUEST_SECRET, NULL);
              sharing_account_set_param (account, PARAM_PIN, NULL);

              valid_account = TRUE;
            }

          g_free (token);
          g_free (secret);
          g_free (name);
        }

      g_free (req_token);
      g_free (pin);
    }

  if (!valid_account)
    sharing_account_set_username (account, "(unconfigured)");

  return valid_account;
}

gchar *
twitter_get_verify_credentials_header   (SharingAccount *account,
                                         const gchar    *verify_url)
{
  gchar *token, *secret, *hdr = NULL;

  g_return_val_if_fail (account != NULL && verify_url != NULL, NULL);

  token = sharing_account_get_param (account, PARAM_ACCESS_TOKEN);
  secret = sharing_account_get_param (account, PARAM_ACCESS_SECRET);

  if (token && secret)
    {
      gchar *sig, *nonce, *timestamp;

      timestamp = g_strdup_printf ("%lu", time (NULL));
      nonce = oauth_gen_nonce ();

      sig = get_oauth_signature ("GET", verify_url, secret,
                                 "oauth_consumer_key", CONSUMER_KEY,
                                 "oauth_nonce", nonce,
                                 "oauth_signature_method", "HMAC-SHA1",
                                 "oauth_timestamp", timestamp,
                                 "oauth_token", token,
                                 "oauth_version", "1.0",
                                 NULL);

      hdr = g_strconcat ("X-Verify-Credentials-Authorization: OAuth realm=\"http://api.twitter.com/\", "
                         "oauth_consumer_key=\"" CONSUMER_KEY "\", "
                         "oauth_nonce=\"", nonce, "\", "
                         "oauth_signature_method=\"HMAC-SHA1\", "
                         "oauth_timestamp=\"", timestamp, "\", "
                         "oauth_token=\"", token, "\", "
                         "oauth_version=\"1.0\", "
                         "oauth_signature=\"", sig, "\"", NULL);

      g_free (sig);
      g_free (timestamp);
      g_free (nonce);
    }

  g_free (token);
  g_free (secret);

  return hdr;
}

SharingHTTPRunResponse
twitter_update_status                   (const gchar        *status,
                                         SharingAccount     *account,
                                         const gchar        *media,
                                         const gchar        *mime,
                                         UploadProgressData *progress_data)
{
  gchar *token, *secret;
  SharingHTTPRunResponse retvalue = SHARING_SEND_ERROR_UNKNOWN;
  const gchar *post_url;

  g_return_val_if_fail (status && account, FALSE);
  g_return_val_if_fail (!media || (mime && progress_data), FALSE);

  token = sharing_account_get_param (account, PARAM_ACCESS_TOKEN);
  secret = sharing_account_get_param (account, PARAM_ACCESS_SECRET);
  post_url = (media == NULL) ? TWITTER_UPDATE_STATUS_URL : TWITTER_UPDATE_MEDIA_STATUS_URL;

  if (token && secret)
    {
      gchar *sig, *hdr, *timestamp, *nonce;
      SharingHTTP *http;

      timestamp = g_strdup_printf ("%lu", time (NULL));
      nonce = oauth_gen_nonce ();

      sig = get_oauth_signature ("POST", post_url, secret,
                                 "oauth_callback", "oob",
                                 "oauth_consumer_key", CONSUMER_KEY,
                                 "oauth_nonce", nonce,
                                 "oauth_signature_method", "HMAC-SHA1",
                                 "oauth_timestamp", timestamp,
                                 "oauth_token", token,
                                 "oauth_version", "1.0",
                                 NULL);

      hdr = g_strconcat ("OAuth oauth_callback=\"oob\", "
                         "oauth_consumer_key=\"" CONSUMER_KEY "\", "
                         "oauth_nonce=\"", nonce, "\", "
                         "oauth_signature_method=\"HMAC-SHA1\", "
                         "oauth_timestamp=\"", timestamp, "\", "
                         "oauth_token=\"", token, "\", "
                         "oauth_version=\"1.0\", "
                         "oauth_signature=\"", sig, "\"", NULL);

      http = sharing_http_new ();
      sharing_http_add_req_multipart_data (http, "status", status, -1, "text/plain");
      sharing_http_add_req_header (http, "Authorization", hdr);
      sharing_http_add_req_header (http, "Expect", "");

      if (media != NULL)
        {
          sharing_http_add_req_multipart_file (http, "media[]", media, mime);
          sharing_http_set_progress_callback (http, upload_progress_cb, progress_data);
        }

      retvalue = sharing_http_run (http, post_url);
      sharing_http_unref (http);

      g_free (sig);
      g_free (hdr);
      g_free (timestamp);
      g_free (nonce);
    }

  g_free (token);
  g_free (secret);

  return retvalue;
}
