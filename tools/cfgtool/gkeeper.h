/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <keeper.h>
#include <gtk/gtk.h>

#ifdef ALLOC_CHECK
#include <sys/types.h>
extern void *_iamalloc(size_t size, int id);
extern void *_iarealloc(void *ptr, size_t size, int id);
extern void  _iafree(void *ptr, int id);
#define malloc(size)        _iamalloc(size, 4)
#define free(ptr)           _iafree(ptr, 4)
#define realloc(ptr, size)  _iarealloc(ptr, size, 4)
#endif

/* #define DEBUG */

#ifdef DEBUG
#define DEB(X) X
#else
#define DEB(X)
#endif

typedef enum {
    DI_OK,
    DI_CONFIRM,
    DI_MESSAGE
} dialog_t;

struct nodeinfo {
    char *key;
    char *name;
    int isfilled;
    int isbook;
    kpval_t type;
    union {
        int i;
        double f;
        char *s;
        struct {
            void *data;
            unsigned int len;
        } d;
    } val;
};

/* tree.c */
extern char *get_value_string(struct nodeinfo *ni);
extern void reload_tree();
extern void collapse(GtkCTree *ctree, GList *node, gpointer user_data);
extern void expand(GtkCTree *ctree, GList *node, gpointer user_data);
extern void del_node();
extern void new_node();
extern void edit_node();
extern void init_tree(GtkWidget *mainwin, GtkWidget *ctree);

/* dialog.c */
extern void init_dialog(GtkWidget *mainwin);
extern void simple_dialog(dialog_t type, const char *message,
                          const char *title, GtkSignalFunc do_func,
                          gpointer do_data);

/* edit.c */
extern void edit_key(GtkCTree *ctree, GtkCTreeNode *node);
extern void new_key(GtkCTree *ctree, GtkCTreeNode *node);

/* menu.c */
extern void load_menus(const char *key);
extern void save_menus(const char *key);
