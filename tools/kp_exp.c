/*
 * kp_exp.c: export a standard file format from the keeper database
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
 * MA 02111-1307, USA
 */

#include "kptool.h"
#include <keeper.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


static const char *code_type(kpval_t type)
{
    switch(type) {
    case KPVAL_UNKNOWN:
        return "UNKNOWN";
    case KPVAL_DIR:
        return "DIR";
    case KPVAL_DATA:
        return "D";
    case KPVAL_STRING:
        return "S";
    case KPVAL_INT:
        return "I";
    case KPVAL_FLOAT:
        return "F";
    }
    return "???";
}


static void print_indent(FILE *fp, int depth)
{
    int i;

    for(i = 0; i < depth; i++)
        fprintf(fp, "    ");
}

static void print_string(FILE *fp, const char *str)
{
    const unsigned char *s;

    fprintf(fp, "\"");
    for(s = (const unsigned char *) str; *s; s++) {
        switch(*s) {
        case '\b':
            fprintf(fp, "\\b");
            break;

        case '\f':
            fprintf(fp, "\\f");
            break;
            
        case '\n':
            fprintf(fp, "\\n");
            break;

        case '\r':
            fprintf(fp, "\\r");
            break;
            
        case '\t':
            fprintf(fp, "\\t");
            break;

        case '\v':
            fprintf(fp, "\\v");
            break;

        case '\\':
            fprintf(fp, "\\\\");
            break;
            
        case '"':
            fprintf(fp, "\\\"");
            break;
            
        default:
            if(*s < 32 || (*s >= 127 && *s <= 160))
                fprintf(fp, "\\%03o", *s);
            else
                fprintf(fp, "%c", *s);
        }
    }
    fprintf(fp, "\"");
}

static void print_name(FILE *fp, const char *name, kpval_t type)
{
    const unsigned char *s;
    int isplain;

    isplain = 1;
    for(s = (const unsigned char *) name; *s; s++) {
        if(*s < 33 || *s >= 127)
            isplain = 0;
        else switch(*s) {
        case '#':
        case '=':
        case '(':
        case ')':
        case '{':
        case '}':
        case ';':
        case '"':
        case '\\':
            isplain = 0;
            break;
        }
        
        if(!isplain)
            break;
    }

    if(isplain) 
        fprintf(fp, "%s", name);
    else
        print_string(fp, name);

    fprintf(fp, " (%s) = ", code_type(type));
}

static int print_key(FILE *fp, const char *key, const char *name,
                     kpval_t type, int depth)
{
    int i;
    int res = 0;
    unsigned int datalen;
    void *rawdata;
    char *stringdata;
    int intdata;
    double floatdata;

    switch(type) {
    case KPVAL_DATA:
        res = kp_get_data(key, &rawdata, &datalen);
        if(res == 0) {
            print_indent(fp, depth);
            print_name(fp, name, type);
            fprintf(fp, "\"");
            for(i = 0; i < (int) datalen; i++) {
                if(i != 0)
                    fprintf(fp, " ");
                fprintf(fp, "0x%02x", ((unsigned char *) rawdata)[i]);
            }
            fprintf(fp, "\";\n");
            free(rawdata);
        }
        break;
        
    case KPVAL_STRING:
        res = kp_get_string(key, &stringdata);
        if(res == 0) {
            print_indent(fp, depth);
            print_name(fp, name, type);
            print_string(fp, stringdata);
            fprintf(fp, ";\n");
            free(stringdata);
        }
        break;
        
    case KPVAL_INT:
        res = kp_get_int(key, &intdata);
        if(res == 0) {
            print_indent(fp, depth);
            print_name(fp, name, type);
            fprintf(fp, "%i;\n", intdata);
        }
        break;
        
    case KPVAL_FLOAT:
        res = kp_get_float(key, &floatdata);
        if(res == 0) {
            print_indent(fp, depth);
            print_name(fp, name, type);
            fprintf(fp, "%.20g;\n", floatdata);
        }
        break;

    default:
        fprintf(stderr, "%s: Unknown type\n", key);
        res = 0;
    }

    return res;
}

static void get_recursive(FILE *fp, const char *key, const char *name,
                          int depth)
{
    int res;
    kpval_t type;
    char **keys;
    unsigned int numkeys;
    
    res = kp_get_type(key, &type);
    if(res != 0)
        fprintf(stderr, "%s: %s\n", name, kp_strerror(res));
    else {
        if(type == KPVAL_DIR) {
            res = kp_get_dir(key, &keys, &numkeys);
            if(res == 0) {
                char **kp;
                KPDIR *dir;

                kp_sort_keys(keys, numkeys);
                dir = kp_dir_open(key);

                print_indent(fp, depth);
                fprintf(fp, "%s = {\n", name);

                for(kp = keys; *kp != NULL; kp++)
                    get_recursive(fp, KP_P(dir, *kp), *kp, depth+1);

                print_indent(fp, depth);
                fprintf(fp, "}\n");

                kp_dir_close(dir);
                free(keys);
            }
        }
        else 
            res = print_key(fp, key, name, type, depth);

        if(res != 0) 
            fprintf(stderr, "%s: Get value failed: %s\n",
                    name, kp_strerror(res));
    }
}


int kp_export (const char *filename, const char *basekey, const char *subkey)
{
    FILE *fp;
    char *key;
    unsigned int len;
    
    if(filename != NULL) {
        fp = fopen(filename, "w");
        if(fp == NULL) {
            fprintf(stderr, "Could not open '%s': %s\n", filename,
                    strerror(errno));
            return -1;
        }
    }
    else
        fp = stdout;

    len = 0;
    if(basekey != NULL)
        len += strlen(basekey);
    if(subkey != NULL)
        len += strlen(subkey);

    key = (char *) malloc(len + 1);
    assert(key != NULL);
    
    key[0] = '\0';
    if(basekey != NULL)
        strcat(key, basekey);
    if(subkey != NULL)
        strcat(key, subkey);

    get_recursive(fp, key, subkey, 0);
    
    free(key);
    if(filename != NULL)
        fclose(fp);
    else
        fflush(stdout);
    
    return 0;
}
