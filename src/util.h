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
#include <sharing-plugin-interface.h>

G_BEGIN_DECLS

gchar *
twitter_get_auth_url                    (SharingAccount *account);

gboolean
twitter_account_validate                (SharingAccount *account);

void
twitter_account_set_pin                 (SharingAccount *account,
                                         const gchar    *pin);

G_END_DECLS

#endif /* __UTIL_H__ */
