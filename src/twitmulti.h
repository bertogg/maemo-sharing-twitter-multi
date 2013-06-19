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

#ifndef __TWITMULTI_H__
#define __TWITMULTI_H__

#include <hildon/hildon.h>
#include <sharing-plugin-interface.h>

G_BEGIN_DECLS

typedef enum {
  SERVICE_TWITTER,        /* http://dev.twitter.com/docs/api/1/post/statuses/update_with_media */
  SERVICE_TWITPIC,        /* http://dev.twitpic.com/docs/2/upload/                             */
  SERVICE_MOBYPICTURE,    /* http://developers.mobypicture.com/documentation/2-0/upload/       */
  SERVICE_IMGLY,          /* http://img.ly/api/docs                                            */
  SERVICE_YFROG           /* http://code.google.com/p/imageshackapi/wiki/TwitterAuthentication */
} TwitterPicService;

gboolean
twitmulti_account_setup                 (SharingAccount *account,
                                         GtkWindow      *parent);

gboolean
twitmulti_account_validate              (SharingAccount *account,
                                         gboolean       *dead_mans_switch);

SharingPluginInterfaceSendResult
twitmulti_share_file                    (SharingTransfer *transfer,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch);

SharingPluginInterfaceEditAccountResult
twitmulti_account_edit                  (GtkWindow       *parent,
                                         SharingAccount  *account,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch);

G_END_DECLS

#endif /* __TWITMULTI_H__ */
