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

#include <hildon/hildon.h>
#include <sharing-plugin-interface.h>

guint
sharing_plugin_interface_init           (gboolean *dead_mans_switch)
{
  *dead_mans_switch = FALSE;
  return 0;
}

guint
sharing_plugin_interface_uninit         (gboolean *dead_mans_switch)
{
  *dead_mans_switch = FALSE;
  return 0;
}

SharingPluginInterfaceSendResult
sharing_plugin_interface_send           (SharingTransfer *transfer,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch)
{
  SharingPluginInterfaceSendResult ret_val = SHARING_SEND_ERROR_UNKNOWN;
  return ret_val;
}

SharingPluginInterfaceAccountSetupResult
sharing_plugin_interface_account_setup  (GtkWindow       *parent,
                                         SharingService  *service,
                                         SharingAccount **worked_on,
                                         osso_context_t  *osso)
{
  if (twitpic_account_setup (*worked_on, parent))
    return SHARING_ACCOUNT_SETUP_SUCCESS;
  else
    return SHARING_ACCOUNT_SETUP_ERROR_UNKNOWN;
}

SharingPluginInterfaceAccountValidateResult
sharing_plugin_interface_account_validate
                                        (SharingAccount  *account,
                                         ConIcConnection *con,
                                         gboolean        *cont,
                                         gboolean        *dead_mans_switch)
{
  if (twitpic_account_validate (account, dead_mans_switch))
    return SHARING_ACCOUNT_VALIDATE_SUCCESS;
  else
    return SHARING_ACCOUNT_VALIDATE_ERROR_UNKNOWN;
}

SharingPluginInterfaceEditAccountResult
sharing_plugin_interface_edit_account   (GtkWindow       *parent,
                                         SharingAccount  *account,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch)
{
  SharingPluginInterfaceEditAccountResult ret = SHARING_EDIT_ACCOUNT_DELETE;
  return ret;
}
