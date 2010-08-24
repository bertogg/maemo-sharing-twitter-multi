/*
 * This file is part of maemo-sharing-twitter-multi
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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <glib.h>
#include <sharing-plugin-interface.h>
#include <conicconnection.h>

G_BEGIN_DECLS

#define TWITTER_ACCESS_TOKEN_URL        "https://api.twitter.com/oauth/access_token"
#define TWITTER_REQUEST_TOKEN_URL       "https://api.twitter.com/oauth/request_token"
#define TWITTER_AUTHORIZE_URL           "https://api.twitter.com/oauth/authorize"
#define TWITTER_UPDATE_STATUS_URL       "https://api.twitter.com/1/statuses/update.xml"
#define TWITTER_VERIFY_CREDENTIALS_URL  "https://api.twitter.com/1/account/verify_credentials.json"

typedef void
(*TwitterGetAuthUrlCb)                  (const gchar *url,
                                         gpointer     data);

void
twitter_get_auth_url                    (SharingAccount      *account,
                                         ConIcConnection     *con,
                                         TwitterGetAuthUrlCb  callback,
                                         gpointer             cbdata);

gboolean
twitter_account_validate                (SharingAccount *account);

void
twitter_account_set_pin                 (SharingAccount *account,
                                         const gchar    *pin);

gchar *
twitter_get_verify_credentials_header   (SharingAccount *account);

gboolean
twitter_update_status                   (const gchar    *status,
                                         SharingAccount *account);

G_END_DECLS

#endif /* __UTIL_H__ */
