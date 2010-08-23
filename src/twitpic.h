/*
 * This file is part of maemo-sharing-twitter-multi
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

#ifndef __TWITPIC_H__
#define __TWITPIC_H__

#include <hildon/hildon.h>
#include <sharing-plugin-interface.h>

G_BEGIN_DECLS

typedef enum {
  SERVICE_TWITPIC,
  SERVICE_MOBYPICTURE,
  SERVICE_TWITGOO
} TwitterPicService;

gboolean
twitpic_account_setup                   (SharingAccount *account,
                                         GtkWindow      *parent);

gboolean
twitpic_account_validate                (SharingAccount *account,
                                         gboolean       *dead_mans_switch);

SharingPluginInterfaceSendResult
twitpic_share_file                      (SharingTransfer *transfer,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch);

SharingPluginInterfaceEditAccountResult
twitpic_account_edit                    (GtkWindow       *parent,
                                         SharingAccount  *account,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch);

G_END_DECLS

#endif /* __TWITPIC_H__ */
