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

#include "gkeeper.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "folder_open.xpm"
#include "folder_closed.xpm"
#include "book_open.xpm"
#include "book_closed.xpm"
#include "string.xpm"
#include "data.xpm"
#include "float.xpm"
#include "num.xpm"

static GtkWidget *ctree_widget;
static GtkCTreeNode *lnode;
static GtkCTreeNode *unode;
static GtkCTreeNode *gnode;

struct pix_info {
    GdkPixmap *pixmap;
    GdkBitmap *mask;
};

enum {
    PM_FOLDER_OPEN,
    PM_FOLDER_CLOSED,
    PM_BOOK_OPEN,
    PM_BOOK_CLOSED,
    PM_STRING,
    PM_DATA,
    PM_FLOAT,
    PM_NUM,
    PM_WARN,

    PM_LAST
};


struct pix_info pixmaps[PM_LAST];

struct entry {
    char *name;
    kpval_t type;
};

static void pixmaps_add(GdkWindow *win, int index, char **data)
{
    GdkColor transparent = { 0 };

    pixmaps[index].pixmap =
        gdk_pixmap_create_from_xpm_d (win, &pixmaps[index].mask,
                                      &transparent, data);
}

static void free_nodeinfo(gpointer data)
{
    struct nodeinfo *ni = (struct nodeinfo *) data;

    if(ni->type == KPVAL_STRING)
        g_free(ni->val.s);
    else if(ni->type == KPVAL_DATA)
        g_free(ni->val.d.data);

    g_free(ni->key);
    g_free(ni);
}


static int dir_sort(const void *ptr1, const void *ptr2)
{
    const struct entry *e1 = (const struct entry *) ptr1;
    const struct entry *e2 = (const struct entry *) ptr2;

    if(e1->type == KPVAL_DIR && e2->type != KPVAL_DIR)
        return -1;
    if(e1->type != KPVAL_DIR && e2->type == KPVAL_DIR)
        return 1;

    return strcmp(e1->name, e2->name);
}



static void remove_selected(GtkWidget *widget, gpointer data)
{
    GtkCTree *ctree;
    GtkCList *clist;
    GtkCTreeNode *node;
    struct nodeinfo *ni;
    GList *sel;
    int res;

    ctree = GTK_CTREE(ctree_widget);
    clist = GTK_CLIST(ctree);

    for(sel = g_list_copy(clist->selection); sel != NULL; sel = sel->next) {
        node = (GtkCTreeNode *) sel->data;

        ni = (struct nodeinfo *)
            gtk_ctree_node_get_row_data(ctree, node);
        
        res = kp_recursive_do(ni->key, (kp_func) kp_remove, 0, NULL);
        if(res != 0) {
            char *msg = g_strdup_printf("Error removing key: %s\n",
                                        kp_strerror(res));
            simple_dialog(DI_OK, msg, "Error", 0, NULL);
            g_free(msg);
        }
    }
    kp_flush();
    reload_tree();
}

static char *code_string(const char *str)
{
    char *coded, *cs;
    const unsigned char *s;

    coded = g_malloc(strlen(str) * 4 + 64);
    assert(coded != NULL);
    cs = coded;

    *cs++ = '"';
    for(s = (const unsigned char *) str; *s; s++) {
        switch(*s) {
        case '\b':
            cs += sprintf(cs, "\\b");
            break;

        case '\f':
            cs += sprintf(cs, "\\f");
            break;
            
        case '\n':
            cs += sprintf(cs, "\\n");
            break;

        case '\r':
            cs += sprintf(cs, "\\r");
            break;
            
        case '\t':
            cs += sprintf(cs, "\\t");
            break;

        case '\v':
            cs += sprintf(cs, "\\v");
            break;

        case '\\':
            cs += sprintf(cs, "\\\\");
            break;
            
        case '"':
            cs += sprintf(cs, "\\\"");
            break;
            
        default:
            if(*s < 32 || (*s >= 127 && *s <= 160))
                cs += sprintf(cs, "\\%03o", *s);
            else
                *cs++ = *s;
        }
    }
    *cs++ = '"';
    *cs = '\0';

    return coded;
}


char *get_value_string(struct nodeinfo *ni)
{
    int i;
    char *newval;
    char buf[32];
    char *val;
    unsigned char *data;
    
    switch(ni->type) {
    case KPVAL_STRING:
        val = code_string(ni->val.s);
        break;
        
    case KPVAL_INT:
        val = g_strdup_printf("%i", ni->val.i);
        break;
        
    case KPVAL_FLOAT:
        for(i = 13; i < 20; i++) {
            val = g_strdup_printf("%.*g", i, ni->val.f);
            if(strtod(val, NULL) == ni->val.f)
                break;
            g_free(val);
        }
        break;
        
    case KPVAL_DATA:
        data = (unsigned char *) ni->val.d.data;
        val = g_strdup("");
        for(i = 0; i < ni->val.d.len; i++) {
            sprintf(buf, "0x%02x ", (int) data[i]);
            newval = g_strconcat(val, buf, NULL);
            g_free(val);
            val = newval;
        }
        val[strlen(val)-1] = '\0';
        break;
        
    default:
        val = NULL;
    }
    
    return val;
}


static struct pix_info *get_type_pixmap(kpval_t type)
{
    struct pix_info *pm;

    switch(type) {
    case KPVAL_STRING:
        pm = &pixmaps[PM_STRING];
        break;

    case KPVAL_INT:
        pm = &pixmaps[PM_NUM];
        break;

    case KPVAL_FLOAT:
        pm = &pixmaps[PM_FLOAT];
        break;

    case KPVAL_DATA:
        pm = &pixmaps[PM_DATA];
        break;
        
    default:
        pm = NULL;
    }

    return pm;
}

static int get_value(const char *key, struct nodeinfo *ni, int reload,
                     int *changedp)
{
    char *s;
    int i;
    double f;
    void *data;
    unsigned int len;
    int res;
    int changed = 0;

    switch(ni->type) {
    case KPVAL_STRING:
        res = kp_get_string(key, &s);
        if(res == 0) {
            if(!reload || strcmp(ni->val.s, s) != 0) {
                changed = 1;
                g_free(ni->val.s);
                ni->val.s = g_strdup(s);
                assert(ni->val.s != NULL);
            }
            free(s);
        }
        break;

    case KPVAL_INT:
        res = kp_get_int(key, &i);
        if(res == 0) {
            if(!reload || ni->val.i != i) {
                changed = 1;
                ni->val.i = i;
            }
        }
        break;
        
    case KPVAL_FLOAT:
        res = kp_get_float(key, &f);
        if(res == 0) {
            if(!reload || ni->val.f != f) {
                changed = 1;
                ni->val.f = f;
            }
        }
        break;

    case KPVAL_DATA:
        res = kp_get_data(key, &data, &len);
        if(res == 0) {
            if(!reload || ni->val.d.len != len ||
               memcmp(ni->val.d.data, data, len) != 0) {
                changed = 1;
                g_free(ni->val.d.data);
                ni->val.d.len = len;
                ni->val.d.data = g_memdup(data, len);
                assert(ni->val.d.data != NULL);
            }
            free(data);
        }
        break;

    default:
        res = KPERR_BADTYPE;
    }

    if(changedp != NULL)
        *changedp = changed;
    
    return res;
}

static GtkCTreeNode *add_dir_node(GtkCTree *ctree, GtkCTreeNode *parent,
                                  GtkCTreeNode *sibling, struct nodeinfo *ni)
{
    char *text[2];
    struct pix_info *pm1, *pm2;
    GtkCTreeNode *node = NULL;

    text[0] = ni->name;
    text[1] = "";

    if(!ni->isbook) {
        pm1 = &pixmaps[PM_FOLDER_CLOSED];
        pm2 = &pixmaps[PM_FOLDER_OPEN];
    }
    else {
        pm1 = &pixmaps[PM_BOOK_CLOSED];
        pm2 = &pixmaps[PM_BOOK_OPEN];
    }

    DEB(g_print("+ %s\n", ni->key));
    node = gtk_ctree_insert_node (ctree, parent, sibling, text, 5,
                                  pm1->pixmap, pm1->mask,
                                  pm2->pixmap, pm2->mask,
                                  FALSE, FALSE);
    
    text[0] = "DUMMY";
    text[1] = "";
    
    /* Insert dummy node, so that this becomes expandable */
    gtk_ctree_insert_node (ctree, node, NULL, text, 5,
                           NULL, NULL,
                           NULL, NULL, TRUE, FALSE);
    
    return node;
}


static GtkCTreeNode *add_val_node(GtkCTree *ctree, GtkCTreeNode *parent,
                                  GtkCTreeNode *sibling, struct nodeinfo *ni)
{
    gchar *text[2];
    struct pix_info *pm;
    GtkCTreeNode *node = NULL;
    char *val;
    int res;
        
    res = get_value(ni->key, ni, 0, NULL);
    if(res == 0) {
        val = get_value_string(ni);
        pm = get_type_pixmap(ni->type);

        text[0] = ni->name;
        text[1] = val;
        
        DEB(g_print("+ %s\n", ni->key));
        node = gtk_ctree_insert_node (ctree, parent, sibling, text, 5,
                                      pm->pixmap, pm->mask,
                                      NULL, NULL, TRUE, FALSE);
        g_free(val);
    }

    return node;
}


static int check_val_node(GtkCTree *ctree, GtkCTreeNode *parent,
                           GtkCTreeNode *node, struct nodeinfo *ni)
{
    int changed;
    char *val;
    int res;

    res = get_value(ni->key, ni, 1, &changed);
    if(res == 0 && changed) {
        val = get_value_string(ni);
        
        DEB(g_print("! %s\n", ni->key));
        gtk_ctree_node_set_text(ctree, node, 1, val);
        g_free(val);
    }

    return res;
}

static GtkCTreeNode *add_keeper_node(GtkCTree *ctree,  GtkCTreeNode *parent,
                                     GtkCTreeNode *sibling,
                                     const char *key, const char *name,
                                     kpval_t type, int isbook)
{
    struct nodeinfo *ni;
    GtkCTreeNode *node;

    ni = g_malloc0(sizeof(struct nodeinfo));
    assert(ni != NULL);

    ni->key = g_strdup(key);
    assert(ni->key != NULL);

    ni->name = g_strdup(name);
    assert(ni->name != NULL);

    ni->isfilled = 0;

    if(!isbook && name[strlen(name)-1] == ':')
        ni->isbook = 1;
    else
        ni->isbook = isbook;

    ni->type = type;

    if(ni->type == KPVAL_DIR)
        node = add_dir_node(ctree, parent, sibling, ni);
    else
        node = add_val_node(ctree, parent, sibling, ni);

    if(node != NULL)
        gtk_ctree_node_set_row_data_full(ctree, node, ni, free_nodeinfo);
    else
        free_nodeinfo(ni);

    return node;
}

static void reload_subkeys(const char *, GtkCTree *, GtkCTreeNode *, int);


static void get_ents(GtkCTree *ctree, GtkCTreeNode *parent, KPDIR *dir,
                     struct entry *ents, unsigned int numkeys, int isbook)
{
    GtkCTreeNode *node;
    GtkCTreeNode *oldnode;
    struct nodeinfo *ni = NULL;
    struct entry *e;
    int i;

    oldnode = NULL;
    node = GTK_CTREE_ROW(parent)->children;
    i = 0;
    while(i < numkeys || node != NULL) {
        if(i < numkeys)
            e = &ents[i];
        else
            e = NULL;

        if(node != oldnode && node != NULL)
            ni = (struct nodeinfo *)
                gtk_ctree_node_get_row_data(ctree, GTK_CTREE_NODE(node));
        oldnode = node;

        if(node != NULL && i < numkeys &&
           strcmp(ni->name, e->name) == 0 && ni->type == e->type) {
            node = GTK_CTREE_ROW(node)->sibling;
            if(ni->type != KPVAL_DIR)
                check_val_node(ctree, parent, oldnode, ni);
            else
                reload_subkeys(ni->key, ctree, oldnode, ni->isbook);
            i++;
        }
        else {
            if(node != NULL &&
               (e == NULL ||
                (ni->type == KPVAL_DIR && e->type != KPVAL_DIR) ||
                strcmp(ni->name, e->name) < 0)) {

                DEB(g_print("- %s\n", ni->key));
                node = GTK_CTREE_ROW(node)->sibling;
                gtk_ctree_remove_node(ctree, oldnode);
            }
            else {
                add_keeper_node(ctree, parent, node, KP_P(dir, e->name),
                                e->name, e->type, isbook);
                i++;
            }
        }
    }
}

static void get_subkeys(const char *key, GtkCTree *ctree,
                        GtkCTreeNode *parent, int isbook)
{
    struct entry *ents = NULL;
    char **keys = NULL;
    KPDIR *dir;
    int res;
    unsigned int numents = 0;
    unsigned int numkeys;
        

    dir = kp_dir_open(key);

    res = kp_get_dir(key, &keys, &numkeys);
    if(res == 0) {
        int i;

        if(numkeys != 0) {
            ents = g_malloc(sizeof(struct entry) * numkeys);
            assert(ents != NULL);

            for(i = 0; i < numkeys; i++) {
                kpval_t type;

                res = kp_get_type(KP_P(dir, keys[i]), &type);
                if(res == 0) {
                    ents[numents].name = keys[i];
                    ents[numents].type = type;
                    numents ++;
                }
            }

            if(numents != 0)
                qsort(ents, numents, sizeof(struct entry), dir_sort);
        }
    }

    get_ents(ctree, parent, dir, ents, numents, isbook);

    g_free(ents);
    free(keys);
    kp_dir_close(dir);
}

static void reload_subkeys(const char *key, GtkCTree *ctree,
                           GtkCTreeNode *node, int isbook)
{
    struct nodeinfo *ni;

    ni = (struct nodeinfo *)
        gtk_ctree_node_get_row_data(ctree, GTK_CTREE_NODE(node));

    if(GTK_CTREE_ROW(node)->expanded)
        get_subkeys(ni->key, ctree, node, ni->isbook);


    if(GTK_CTREE_ROW(node)->children == NULL)
        gtk_ctree_expand(ctree, node);
}

static void collapse_recursive_nonempty(GtkCTree *ctree, GtkCTreeNode *node)
{
    GtkCTreeNode *child;

    child = GTK_CTREE_ROW(node)->children;
    if(child != NULL) {
        while(child) {
            GtkCTreeRow *row = GTK_CTREE_ROW(child);
            if(!row->is_leaf && row->expanded)
                collapse_recursive_nonempty(ctree, child);

            gtk_ctree_unselect(ctree, child);

            child = row->sibling;
        }
        gtk_ctree_collapse(ctree, node);
    }
}

void collapse(GtkCTree *ctree, GList *node, gpointer user_data)
{
    struct nodeinfo *ni;

    ni = (struct nodeinfo *)
        gtk_ctree_node_get_row_data(ctree, GTK_CTREE_NODE(node));

    DEB(g_print("collapse: %s\n", ni->key));

    gtk_clist_freeze (GTK_CLIST (ctree));
    gtk_signal_handler_block_by_func (GTK_OBJECT(ctree),
                                      GTK_SIGNAL_FUNC(collapse), NULL);

    collapse_recursive_nonempty(ctree, GTK_CTREE_NODE(node));

    gtk_signal_handler_unblock_by_func (GTK_OBJECT(ctree),
                                        GTK_SIGNAL_FUNC(collapse), NULL);
    gtk_clist_thaw (GTK_CLIST (ctree));
}


void expand(GtkCTree *ctree, GList *node, gpointer user_data)
{
    struct nodeinfo *ni;

    ni = (struct nodeinfo *)
        gtk_ctree_node_get_row_data(ctree, GTK_CTREE_NODE(node));

    DEB(g_print("expand: %s\n", ni->key));

    gtk_clist_freeze (GTK_CLIST (ctree));
    if(!ni->isfilled) {
        gtk_signal_handler_block_by_func (GTK_OBJECT(ctree),
                                          GTK_SIGNAL_FUNC(collapse), NULL);
        /* remove dummy */
        gtk_ctree_remove_node(ctree, GTK_CTREE_ROW(node)->children);
        ni->isfilled = 1;
        gtk_signal_handler_unblock_by_func (GTK_OBJECT(ctree),
                                            GTK_SIGNAL_FUNC(collapse), NULL);
    }
    get_subkeys(ni->key, ctree, GTK_CTREE_NODE(node), ni->isbook);

    gtk_clist_thaw (GTK_CLIST (ctree));
}


void reload_tree(void)
{
    GtkCTree *ctree = GTK_CTREE(ctree_widget);

    gtk_clist_freeze (GTK_CLIST (ctree));

    reload_subkeys("l", ctree, lnode, 0);
/*    reload_subkeys("u", ctree, unode, 0); */
    reload_subkeys("g", ctree, gnode, 0);

    gtk_clist_thaw (GTK_CLIST (ctree));
}

void del_node(void)
{
    GtkCList *clist;

    clist = GTK_CLIST(ctree_widget);

    if(clist->selection != NULL)
        simple_dialog(DI_CONFIRM, "Remove the selected keys?",
                      "Confirm", remove_selected, NULL);
    else
        simple_dialog(DI_OK, "No keys are selected", "Info", 0, NULL);

}

void new_node(void)
{
    GtkCList *clist;

    clist = GTK_CLIST(ctree_widget);

    if(clist->selection != NULL) {
        new_key(GTK_CTREE(ctree_widget), 
                GTK_CTREE_NODE(clist->selection->data));
    }
    else
        simple_dialog(DI_OK, "No keys are selected", "Info", 0, NULL);

}


void edit_node(void)
{
    GtkCList *clist;

    clist = GTK_CLIST(ctree_widget);

    if(clist->selection != NULL)
        edit_key(GTK_CTREE(ctree_widget),
                 GTK_CTREE_NODE(clist->selection->data));
    else
        simple_dialog(DI_OK, "No keys are selected", "Info", 0, NULL);
}



void init_tree(GtkWidget *mainwin, GtkWidget *ctree)
{
    ctree_widget = ctree;

    pixmaps_add(mainwin->window, PM_FOLDER_OPEN,   folder_open_xpm);
    pixmaps_add(mainwin->window, PM_FOLDER_CLOSED, folder_closed_xpm);
    pixmaps_add(mainwin->window, PM_BOOK_OPEN,     book_open_xpm);
    pixmaps_add(mainwin->window, PM_BOOK_CLOSED,   book_closed_xpm);
    pixmaps_add(mainwin->window, PM_STRING,        string_xpm);
    pixmaps_add(mainwin->window, PM_DATA,          data_xpm);
    pixmaps_add(mainwin->window, PM_FLOAT,         float_xpm);
    pixmaps_add(mainwin->window, PM_NUM,           num_xpm);

    lnode = add_keeper_node(GTK_CTREE(ctree), NULL, NULL, "l", "DATA",
                            KPVAL_DIR, 0);
/*
    unode = add_keeper_node(GTK_CTREE(ctree), NULL, NULL, "u", "USER",
                            KPVAL_DIR, 0);
*/                          
    gnode = add_keeper_node(GTK_CTREE(ctree), NULL, NULL, "g", "CONFIG",
                            KPVAL_DIR, 0);

}
