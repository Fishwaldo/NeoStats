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
#include <assert.h>


struct edit_data {
    GtkCTree *ctree;

    GtkWidget *dialog;
    GtkWidget *typemenu;
    GtkWidget *keyentry;
    GtkWidget *valueentry;
};

typedef struct {
    const char *name;
    int value;
} OptmenuItem;

static OptmenuItem typeopts[] = 
{
    { "String",    KPVAL_STRING },
    { "Integer",   KPVAL_INT    },
    { "Float",     KPVAL_FLOAT  },
    { "Data",      KPVAL_DATA   },

    { NULL, 0 }
};

static int get_option_index(OptmenuItem *opts, int val)
{
    int i;

    for(i = 0; opts->name != NULL; opts++, i++) {
        if(opts->value == val)
            return i;
    }

    return 0;
}


static GtkWidget *create_optionmenu(OptmenuItem *opts)
{
    GtkWidget *optmenu, *menu, *menuitem;

    menu = gtk_menu_new();
    for(;opts->name != NULL; opts++) {
        menuitem = gtk_menu_item_new_with_label(opts->name);
        gtk_object_set_user_data(GTK_OBJECT(menuitem), opts);
        gtk_menu_append(GTK_MENU(menu), menuitem);
        gtk_widget_show(menuitem);
    }
    gtk_widget_show(menu);

    optmenu = gtk_option_menu_new();
    gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu), menu);

    return optmenu;
}


static int get_data_value(char *val, unsigned char **datap, 
                          unsigned int *lenp)
{
    unsigned char *data;
    unsigned int len;
    unsigned int datasize;
    char *beg, *end;
    unsigned int sd;

    data = NULL;
    len = 0;
    datasize = 0;
        
    for(beg = val; *beg; beg = end) {
        sd = (unsigned int) strtol(beg, &end, 0);
        if(beg == end)
            return -1;

        if(len >= datasize) {
            datasize += 16;
            data = (unsigned char *) realloc(data, datasize);
            assert(data != NULL);
        }
        data[len++] = (unsigned char) sd;
    }

    *datap = data;
    *lenp = len;
    
    return 0;
}

static int get_escaped_char(unsigned char **sp)
{
    unsigned char *s;
    int val;

    s = *sp;

    switch(*s) {
    case 'b':
        val = '\b';
        s++;
        break;

    case 'f':
        val = '\f';
        s++;
        break;
        
    case 'n':
        val = '\n';
        s++;
        break;
        
    case 'r':
        val = '\r';
        s++;
        break;
        
    case 't':
        val =  '\t';
        s++;
        break;
        
    case 'v':
        val = '\v';
        s++;
        break;
        
    default:
        if(*s >= '0' && *s < '8') {
            val = *s - '0';
            s++;
            if(*s >= '0' && *s < '8') {
                val = val * 8 + *s - '0';
                s++;
                if(*s >= '0' && *s < '8') {
                    val = val * 8 + *s - '0';
                    s++;
                }
            }
        }
        else {
            val = *s;
            s++;
        }
    }

    *sp = s;

    return val;
}

static int decode_string(char *s, int quoted, char *ds)
{
    int val;

    while(*s != '\0') {
        if(*s == '"' && quoted)
            return -1;
        if(*s != '\\') 
            val = *s++;
        else {
            s++;
            val = get_escaped_char((unsigned char **) &s);
        }

        *ds++ = val;
    }
    *ds = '\0';

    return 0;
}

static char *get_string_value(char *buf)
{
    char *decoded;
    int quoted;
    int res;

    if(buf[0] == '"')
        quoted = 1;
    else
        quoted = 0;

    if(quoted) {
        if(buf[strlen(buf) - 1] != '"')
            return NULL;

        buf[strlen(buf) - 1] = '\0';
        buf++;
    }

    decoded = g_malloc(strlen(buf) + 1);

    res = decode_string(buf, quoted, decoded);
    if(res != 0) {
        g_free(decoded);
        return NULL;
    }

    return decoded;
}


static void edit_ok(GtkWidget *widget, struct edit_data *edit)
{
    char *val;
    char *end;
    kpval_t type;
    char *key;
    GtkWidget *menu, *active;
    OptmenuItem *opt;
    int res;
    int set;
    int intval;
    double floatval;
    unsigned char *data;
    unsigned int len;
    char *strval;
    
    menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(edit->typemenu));
    active = gtk_menu_get_active(GTK_MENU(menu));
    opt = (OptmenuItem *) gtk_object_get_user_data(GTK_OBJECT(active));

    val = gtk_editable_get_chars(GTK_EDITABLE(edit->valueentry), 0, -1);
    type = opt->value;

    key = gtk_editable_get_chars(GTK_EDITABLE(edit->keyentry), 0, -1);

    res = 0;
    set = 1;
    switch(type) {
    case KPVAL_STRING:
        g_strstrip(val);
        strval = get_string_value(val);
        if(strval == NULL) {
            set = 0;
            simple_dialog(DI_OK, "Bad string value", "Error", 0, NULL);
        }
        else {
            res = kp_set_string(key, strval);
            g_free(strval);
        }
        break;
        
    case KPVAL_INT:
        g_strstrip(val);
        intval = strtol(val, &end, 0);
        if(*end != '\0') {
            set = 0;
            simple_dialog(DI_OK, "Bad integer value", "Error", 0, NULL);
        }
        else
            res = kp_set_int(key, intval);
        break;
        
    case KPVAL_FLOAT:
        g_strstrip(val);
        floatval = strtod(val, &end);
        if(*end != '\0') {
            set = 0;
            simple_dialog(DI_OK, "Bad float value", "Error", 0, NULL);
        }
        else
            res = kp_set_float(key, floatval);
        break;
        
    case KPVAL_DATA:
        g_strstrip(val);
        res = get_data_value(val, &data, &len);
        if(res != 0) {
            set = 0;
            simple_dialog(DI_OK, "Bad data value", "Error", 0, NULL);
        }
        else {
            res = kp_set_data(key, data, len);
            free(data);
        }
        break;
        
    default:
        set = 0;
    }
    
    if(set) {
        if(res == 0) {
            kp_flush();
            reload_tree();
        }
        else {
            char *msg = g_strdup_printf("Error setting key: %s\n",
                                        kp_strerror(res));
            simple_dialog(DI_OK, msg, "Error", 0, NULL);
            g_free(msg);
        }
    }
    
    g_free(key);
    g_free(val);
}


struct edit_data *create_editdialog(void)
{
    struct edit_data *edit;
    GtkWidget *dialog_vbox;
    GtkWidget *table;
    GtkWidget *label;
    GtkWidget *dialog_action_area;
    GtkWidget *hbuttonbox;
    GtkWidget *button;

    edit = g_new(struct edit_data, 1);
    assert(edit != NULL);
    
    edit->dialog = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (edit->dialog), "Edit");
    gtk_window_set_policy (GTK_WINDOW (edit->dialog), TRUE, TRUE, FALSE);
    gtk_window_set_position(GTK_WINDOW(edit->dialog), GTK_WIN_POS_MOUSE);
    gtk_object_set_data_full (GTK_OBJECT (edit->dialog), "edit", edit,
                              g_free);
    
    dialog_vbox = GTK_DIALOG (edit->dialog)->vbox;
    
    table = gtk_table_new (4, 2, FALSE);
    gtk_box_pack_start (GTK_BOX (dialog_vbox), table, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (table), 20);
    gtk_table_set_row_spacings (GTK_TABLE (table), 10);
    gtk_table_set_col_spacings (GTK_TABLE (table), 10);
    
    label = gtk_label_new ("key");
    gtk_table_attach (GTK_TABLE (table), label, 0, 2, 0, 1,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    
    edit->keyentry = gtk_entry_new ();
    gtk_table_attach (GTK_TABLE (table), edit->keyentry, 0, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    
    edit->valueentry = gtk_entry_new ();
    gtk_table_attach (GTK_TABLE (table), edit->valueentry, 1, 2, 3, 4,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    
    edit->typemenu = create_optionmenu(typeopts);
    gtk_table_attach (GTK_TABLE (table), edit->typemenu, 0, 1, 3, 4,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    
    label = gtk_label_new ("type");
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    
    label = gtk_label_new ("value");
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 2, 3,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    
    dialog_action_area = GTK_DIALOG (edit->dialog)->action_area;
    gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area), 10);
    
    hbuttonbox = gtk_hbutton_box_new ();
    gtk_box_pack_start (GTK_BOX (dialog_action_area), hbuttonbox,
                        TRUE, TRUE, 0);
    
    button = gtk_button_new_with_label ("OK");
    gtk_container_add (GTK_CONTAINER (hbuttonbox), button);
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_signal_connect(GTK_OBJECT(button), "clicked", 
                       GTK_SIGNAL_FUNC(edit_ok), edit);
    gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(edit->dialog));

    gtk_widget_grab_default (button);
    
    button = gtk_button_new_with_label ("Cancel");
    gtk_container_add (GTK_CONTAINER (hbuttonbox), button);
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
                              GTK_SIGNAL_FUNC(gtk_widget_destroy),
                              GTK_OBJECT(edit->dialog));

    gtk_widget_show_all(edit->dialog);
    
    return edit;
}



void edit_key(GtkCTree *ctree, GtkCTreeNode *node)
{
    struct nodeinfo *ni = (struct nodeinfo *)
        gtk_ctree_node_get_row_data(ctree, GTK_CTREE_NODE(node));

    if(ni->type == KPVAL_DIR)
        simple_dialog(DI_OK, "You cannot edit a directory", "Info",
                      0, NULL);
    else {
        struct edit_data *edit;
        unsigned int i;
        char *val;
        
        edit = create_editdialog();
        edit->ctree = ctree;

        gtk_entry_set_text(GTK_ENTRY(edit->keyentry), ni->key);
        gtk_entry_set_editable(GTK_ENTRY(edit->keyentry), FALSE);
        /* gtk_widget_set_state(edit->keyentry, GTK_STATE_INSENSITIVE); */

        i = get_option_index(typeopts, ni->type);
        gtk_option_menu_set_history(GTK_OPTION_MENU(edit->typemenu), i);

        val = get_value_string(ni);
        gtk_entry_set_text(GTK_ENTRY(edit->valueentry), val);
        g_free(val);
    }
    
}


void new_key(GtkCTree *ctree, GtkCTreeNode *node)
{
    struct edit_data *edit;
    unsigned int i;
    char *subkey;
    
    struct nodeinfo *ni = (struct nodeinfo *)
        gtk_ctree_node_get_row_data(ctree, GTK_CTREE_NODE(node));

    if(ni->type != KPVAL_DIR) {
        node = GTK_CTREE_ROW(node)->parent;
        ni = (struct nodeinfo *)
            gtk_ctree_node_get_row_data(ctree, GTK_CTREE_NODE(node));
    }
        
    edit = create_editdialog();
    edit->ctree = ctree;

    subkey = g_strdup_printf("%s/", ni->key);
    gtk_entry_set_text(GTK_ENTRY(edit->keyentry), subkey);
    g_free(subkey);

    i = get_option_index(typeopts, KPVAL_STRING);
    gtk_option_menu_set_history(GTK_OPTION_MENU(edit->typemenu), i);

    gtk_entry_set_text(GTK_ENTRY(edit->valueentry), "");
}
