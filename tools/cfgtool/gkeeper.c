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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MENUPATH "ug/SerSoft/gkeeper:/menus"

static void about(void);
static void quit(void);

static GtkItemFactoryEntry menu_items[] = {
    {"/_File",             NULL,         0,              0, "<Branch>"},
    {"/File/_Reload",      "<control>L", reload_tree,    0, NULL},
    {"/File/sep1",         NULL,         0,              0, "<Separator>"},
    {"/File/_Quit",        "<control>Q", quit,           0, NULL},
    {"/_Edit",             NULL,         0,              0, "<Branch>"},
    {"/Edit/_Edit",        "<control>E", edit_node,      0, NULL},
    {"/Edit/_New",         "<control>N", new_node,       0, NULL},
    {"/Edit/_Delete",      "<control>D", del_node,       0, NULL},
    {"/_Help",             NULL,         0,              0, "<LastBranch>"},
    {"/Help/About",        NULL,         about,          0, NULL}
};

static void quit()
{
    save_menus(MENUPATH);

#ifdef ALLOC_CHECK
    _kp_exit();
#endif

    gtk_exit(0);
}

static void about()
{
    simple_dialog(DI_MESSAGE,
                  "configtoolis a graphical tool for viewing\n"
                  "and manipulating the internal configuration\n"
                  "of NeoStats and its Modules\n"
                  "\n"
                  "It is based on a tool that comes from the Keeper Libary\n"
                  "called gkeeper. The Keeper Library can be found at:\n"
                  "http://www.inf.bme.hu/~mszeredi/keeper/\n"
                  "\n"
                  "Author: Justin Hammond <justin@dynam.ac>\n",
                  "About", 0, NULL);
}


static GtkWidget *setup_main_menu(GtkWidget *window)
{
    unsigned int nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);
    GtkItemFactory *item_factory;
    GtkAccelGroup *accel_group;
    GtkWidget *menubar;

    accel_group = gtk_accel_group_new();

    item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
                                        accel_group);

    gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, NULL);

    gtk_accel_group_attach (accel_group, GTK_OBJECT (window));

    menubar = gtk_item_factory_get_widget(item_factory, "<main>");

    return menubar;
}

static void setup_window(int argc, char *argv[])
{
    GtkWidget *main_window;
    GtkWidget *scrolled_win;
    GtkWidget *menubar;
    GtkWidget *main_vbox;
    GtkWidget *ctree;
    char *title[] = { "Key" , "Value" };

    gtk_init(&argc, &argv);
    load_menus(MENUPATH);

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW (main_window), "ConfigTool");
    gtk_signal_connect (GTK_OBJECT (main_window), "destroy",
			GTK_SIGNAL_FUNC (quit), NULL);


    main_vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_border_width(GTK_CONTAINER(main_vbox), 1);
    gtk_container_add(GTK_CONTAINER(main_window), main_vbox);

    menubar = setup_main_menu(main_window);
    gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);

    scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_widget_set_usize (scrolled_win, 500, 400);
    gtk_container_border_width(GTK_CONTAINER(scrolled_win), 0);

    gtk_box_pack_start(GTK_BOX (main_vbox), scrolled_win, TRUE, TRUE, 0);

    ctree = gtk_ctree_new_with_titles(2, 0, title);
    gtk_clist_set_column_auto_resize (GTK_CLIST (ctree), 0, TRUE);
    gtk_clist_set_column_width (GTK_CLIST (ctree), 1, 200);
    gtk_clist_set_selection_mode (GTK_CLIST (ctree), GTK_SELECTION_EXTENDED);
    gtk_clist_column_titles_passive (GTK_CLIST (ctree));
    gtk_ctree_set_line_style (GTK_CTREE(ctree), GTK_CTREE_LINES_DOTTED);
    gtk_container_border_width(GTK_CONTAINER(ctree), 1);

    gtk_signal_connect(GTK_OBJECT(ctree), "tree_expand",
                       GTK_SIGNAL_FUNC(expand), NULL);

    gtk_signal_connect(GTK_OBJECT(ctree), "tree_collapse",
                       GTK_SIGNAL_FUNC(collapse), NULL);

    gtk_container_add(GTK_CONTAINER(scrolled_win), ctree);

    gtk_widget_realize(main_window);

    init_dialog(main_window);
    init_tree(main_window, ctree);

    gtk_widget_show_all (main_window);
}

int main(int argc, char *argv[])
{
    setup_window(argc, argv);
    gtk_main();

    return 0;
}

