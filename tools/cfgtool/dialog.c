/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
** http://www.neostats.net/
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** NeoStats CVS Identification
** $Id$
*/
/*
 * gkeeper: A graphical tool for viewing and manipulating keeper
 * database
 *
 * Copyright (C) 1999-2000 Miklos Szeredi
 * Email: mszeredi@inf.bme.hu
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA */

#include "gkeeper.h"

#include "warning.xpm"

static GdkPixmap *warn_pixmap;
static GdkBitmap *warn_mask;
static GtkWidget *main_window;

extern void init_dialog(GtkWidget *mainwin)
{
    GdkColor transparent = { 0 };

    warn_pixmap = gdk_pixmap_create_from_xpm_d (mainwin->window, &warn_mask,
                                                &transparent, warning_xpm);
    
    main_window = mainwin;
}

void simple_dialog(dialog_t type, const char *message, const char *title,
                   GtkSignalFunc do_func, gpointer do_data)
{
    GtkWidget *dialog, *label, *button, *buttbox, *hbox, *pixmap;

    dialog = gtk_dialog_new();
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(main_window));

    buttbox = gtk_hbutton_box_new();
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area),
                       buttbox);

    button = gtk_button_new_with_label("OK");
    GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
    gtk_box_pack_start(GTK_BOX(buttbox), button, TRUE, TRUE, 0);
    if(do_func != 0)
        gtk_signal_connect(GTK_OBJECT (button), "clicked", do_func, do_data);

    gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(dialog));

    if(type == DI_CONFIRM) {
        button = gtk_button_new_with_label("Cancel");
        GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
        gtk_box_pack_start(GTK_BOX(buttbox), button, TRUE, TRUE, 0);

        gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                                  GTK_SIGNAL_FUNC(gtk_widget_destroy),
                                  GTK_OBJECT(dialog));
    }
    gtk_widget_grab_default (button);

    hbox = gtk_hbox_new(FALSE, 10);
    gtk_container_border_width(GTK_CONTAINER(hbox), 10);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                       hbox);

    if(type == DI_OK || type == DI_CONFIRM) {
        pixmap = gtk_pixmap_new(warn_pixmap, warn_mask);
        gtk_box_pack_start(GTK_BOX(hbox), pixmap, TRUE, TRUE, 0);
    }

    label = gtk_label_new (message);
    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

    gtk_widget_show_all (dialog);
}
