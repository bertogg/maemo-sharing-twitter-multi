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

#include <hildon-mime.h>

static void
register_account_clicked                (GtkWidget *button,
                                         gpointer   data)
{
  hildon_uri_open ("https://api.twitter.com/", NULL, NULL);
  gtk_dialog_response (GTK_DIALOG (data), GTK_RESPONSE_ACCEPT);
}

gboolean
twitpic_account_setup                   (GtkWindow *parent)
{
  gint response;
  GtkWidget *d, *label, *button;
  GtkContainer *vbox;

  d = gtk_dialog_new ();
  vbox = GTK_CONTAINER (GTK_DIALOG (d)->vbox);
  gtk_window_set_title (GTK_WINDOW (d), "Account setup - Twitpic");
  gtk_window_set_transient_for (GTK_WINDOW (d), parent);

  label = gtk_label_new ("You need to register a Twitter account using the web page.\n"
                         "If you don't have one, you'll be offered the option to do it.");
  button = gtk_button_new_with_label ("Register Twitter account");

  hildon_gtk_widget_set_theme_size (button, HILDON_SIZE_FINGER_HEIGHT | HILDON_SIZE_AUTO_WIDTH);

  gtk_container_add (vbox, label);
  gtk_container_add (vbox, button);

  g_signal_connect (button, "clicked", G_CALLBACK (register_account_clicked), d);

  gtk_widget_show_all (d);
  response = gtk_dialog_run (GTK_DIALOG (d));
  gtk_widget_destroy (d);

  return (response != GTK_RESPONSE_DELETE_EVENT);
}
