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

#include <stdlib.h>
#include <string.h>

struct loadinfo {
    GtkItemFactoryClass *class;
    guint baselen;
};

static void save_menu_accelerator(gpointer hash_key, gpointer value,
                                  gpointer user_data)
{
    GtkItemFactoryItem *item;
    gchar *name;
    KPDIR *dir;
    
    dir = (KPDIR *) user_data;
    
    item = value;
    if(item->modified) {
        name = gtk_accelerator_name (item->accelerator_key,
                                     item->accelerator_mods);
        
        kp_set_string(KP_P(dir, (char *) hash_key), name);
        g_free (name);
    }
}

void save_menus(const char *key)
{
    GtkItemFactoryClass *class;
    KPDIR *dir;

    dir = kp_dir_open(key);
    class = gtk_type_class (GTK_TYPE_ITEM_FACTORY);
    g_hash_table_foreach (class->item_ht, save_menu_accelerator, dir);
    kp_dir_close(dir);
    kp_flush();
}


static int load_menu_accelerator(const char *key, gpointer user_data)
{
    char *value;
    const char *path;
    char *rcstring;
    GtkItemFactoryItem *item;
    struct loadinfo *li;

    li = (struct loadinfo *) user_data;

    path = key + li->baselen + 1;

    value = NULL;
    kp_get_string(key, &value);
    if(value != NULL) {
        /* Ugly hack. GTK should export a function that accomplishes
           the same without parsing a string */
        rcstring = g_strdup_printf("(menu-path \"%s\" \"%s\")\n", path, value);
        gtk_item_factory_parse_rc_string(rcstring);
        g_free(rcstring);
        free(value);

        item = g_hash_table_lookup(li->class->item_ht, path);
        if(item)
            item->modified = FALSE;
    }
    return 0;
}

void load_menus(const char *key)
{
    struct loadinfo li;

    li.class = gtk_type_class (GTK_TYPE_ITEM_FACTORY);
    li.baselen = strlen(key);

    kp_recursive_do(key, load_menu_accelerator, 0, &li);
}

