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

#include <hildon/hildon.h>
#include <sharing-plugin-interface.h>
#include <curl/curl.h>

guint
sharing_plugin_interface_init           (gboolean *dead_mans_switch)
{
  *dead_mans_switch = FALSE;
  curl_global_init (CURL_GLOBAL_ALL);
  return 0;
}

guint
sharing_plugin_interface_uninit         (gboolean *dead_mans_switch)
{
  *dead_mans_switch = FALSE;
  curl_global_cleanup ();
  return 0;
}

SharingPluginInterfaceSendResult
sharing_plugin_interface_send           (SharingTransfer *transfer,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch)
{
  return twitmulti_share_file (transfer, con, dead_mans_switch);
}

SharingPluginInterfaceAccountSetupResult
sharing_plugin_interface_account_setup  (GtkWindow       *parent,
                                         SharingService  *service,
                                         SharingAccount **worked_on,
                                         osso_context_t  *osso)
{
  if (twitmulti_account_setup (*worked_on, parent))
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
  if (twitmulti_account_validate (account, dead_mans_switch))
    return SHARING_ACCOUNT_VALIDATE_SUCCESS;
  else
    return SHARING_ACCOUNT_VALIDATE_FAILED;
}

SharingPluginInterfaceEditAccountResult
sharing_plugin_interface_edit_account   (GtkWindow       *parent,
                                         SharingAccount  *account,
                                         ConIcConnection *con,
                                         gboolean        *dead_mans_switch)
{
  return twitmulti_account_edit (parent, account, con, dead_mans_switch);
}
