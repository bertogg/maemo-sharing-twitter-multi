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

static void
register_account_clicked                (GtkWidget *button,
                                         gpointer   data)
{
  SharingAccount *account = data;
  gchar *url = twitter_get_auth_url (account);
  if (url)
    {
      hildon_uri_open (url, NULL, NULL);
      g_free (url);
    }
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

