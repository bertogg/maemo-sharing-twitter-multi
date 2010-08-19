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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <glib.h>

G_BEGIN_DECLS

gchar *
get_twitter_auth_url                    (gchar **token,
                                         gchar **secret);

gboolean
get_twitter_access_token                (gchar  *request_token,
                                         gchar  *pin,
                                         gchar **access_token,
                                         gchar **access_secret,
                                         gchar **screen_name);

G_END_DECLS

#endif /* __UTIL_H__ */
