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
 * kptool: A tool for viewing and manipulating keeper database
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
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

static const char *usagestr =
"Usage: %s [options] [key]\n"
"Options:\n"
"  -h --help             Get help\n"
"  -l                    List keys (recursively)\n"
"  -s Type Value         Set value of key\n"
"  -g Type               Get value of key\n"
"  -r                    Remove key\n"
"  -t                    Get type of key\n"
"  -i [Filename]         Import database part\n"
"  --recursive-remove    Remove key recursively. Use it with care!\n"
"\n"
"Types:\n"
"  D                     Data, value is a hex byte list (e.g. 537a4d)\n"
"  S                     String\n"
"  I                     Integer\n"
"  F                     Float\n"
"  DIR                   Key directory (no set operation)\n";

static kpval_t decode_type(char *str)
{
    if(strcmp(str, "DIR") == 0)
        return KPVAL_DIR;

    if(strlen(str) != 1) {
        fprintf(stderr, "Bad type: %s\n", str);
        exit(1);
    }

    switch(str[0]) {
    case 'D':
        return KPVAL_DATA;
    case 'S':
        return KPVAL_STRING;
    case 'I':
        return KPVAL_INT;
    case 'F':
        return KPVAL_FLOAT;

    default:
        fprintf(stderr, "Bad type: %s\n", str);
        exit(1);
    }
}

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

static int get_key(const char *key, kpval_t type)
{
    int i;
    int res = 0;
    unsigned int datalen;
    void *rawdata;
    char *stringdata;
    int intdata;
    double floatdata;
    char **keys;

    switch(type) {
    case KPVAL_DATA:
        res = kp_get_data(key, &rawdata, &datalen);
        if(res == 0) {
            for(i = 0; i < (int) datalen; i++)
                printf("%02x", ((unsigned char *) rawdata)[i]);
            free(rawdata);
            printf("\n");
        }
        break;

    case KPVAL_STRING:
        res = kp_get_string(key, &stringdata);
        if(res == 0) {
            printf("%s\n", stringdata);
            free(stringdata);
        }
        break;

    case KPVAL_INT:
        res = kp_get_int(key, &intdata);
        if(res == 0)
            printf("%i\n", intdata);
        break;

    case KPVAL_FLOAT:
        res = kp_get_float(key, &floatdata);
        if(res == 0)
            printf("%.20g\n", floatdata);
        break;

    case KPVAL_DIR:
        res = kp_get_dir(key, &keys, NULL);
        if(res == 0) {
            for(i = 0; keys[i] != NULL; i++)
                printf("%s\n", keys[i]);
            free(keys);
        }
        break;

    default:
        fprintf(stderr, "Get operation is not allowed for this type\n");
        exit(1);
    }

    return res;
}


static void del_recursive(const char *key)
{
    int res;
    kpval_t type;
    char **keys;
    int i;

    res = kp_get_type(key, &type);
    if(res != 0)
        fprintf(stderr, "%s: %s\n", key, kp_strerror(res));
    else {
        if(type == KPVAL_DIR) {
            res = kp_get_dir(key, &keys, NULL);
            if(res != 0)
                fprintf(stderr, "%s: Get dir failed: %s\n",
                        key, kp_strerror(res));
            else {
                KPDIR *dir;

                dir = kp_dir_open(key);

                for(i = 0; keys[i] != NULL; i++)
                    del_recursive(KP_P(dir, keys[i]));

                kp_dir_close(dir);
                free(keys);
            }
        }
        else {
            res = kp_remove(key);
            if(res != 0)
                fprintf(stderr, "%s: Remove key failed: %s\n",
                        key, kp_strerror(res));
        }
    }
}

static void get_data_value(char *value, void **datap, unsigned int *lenp)
{
    int i;
    unsigned char *data;
    unsigned int len;
    char tmpbuf[3];
    int val;

    len = strlen(value) / 2;
    if(len * 2 != strlen(value)) {
        fprintf(stderr, "Bad data value: %s\n", value);
        exit(1);
    }

    data = (unsigned char *) malloc(len);
    assert(data != NULL);

    for(i = 0; i < (int) len; i++) {
        if(!isxdigit((int) value[2*i]) || !isxdigit((int) value[2*i+1])) {
            fprintf(stderr, "Bad data value: %s\n", value);
            exit(1);
        }

        tmpbuf[0] = value[2*i];
        tmpbuf[1] = value[2*i+1];
        tmpbuf[2] = '\0';

        sscanf(tmpbuf, "%x", &val);
        data[i] = val;
    }
}

static int set_key(const char *key, kpval_t type, char *value)
{
    int res = 0;
    void *data;
    unsigned int len;
    int intdata;
    double floatdata;
    char *end;

    switch(type) {
    case KPVAL_DATA:
        get_data_value(value, &data, &len);
        res = kp_set_data(key, data, len);
        break;

    case KPVAL_STRING:
        res = kp_set_string(key, value);
        break;

    case KPVAL_INT:
        intdata = strtol(value, &end, 0);
        if(*end != '\0') {
            fprintf(stderr, "Bad integer value: %s\n", value);
            exit(1);
        }
        res = kp_set_int(key, intdata);
        break;

    case KPVAL_FLOAT:
        floatdata = strtod(value, &end);
        if(*end != '\0') {
            fprintf(stderr, "Bad float value: %s\n", value);
            exit(1);
        }
        res = kp_set_float(key, floatdata);
        break;

    default:
        fprintf(stderr, "Set operation is not allowed for this type\n");
        exit(1);
    }

    if(res == 0) {
        res = kp_flush();
        if(res != 0)
            fprintf(stderr, "%s: Flush failed: %s\n",
                    key, kp_strerror(res));
        res = 0;
    }


    return res;
}



int main(int argc, char *argv[])
{
    int res;
    const char *key;
    char *arg;
    int argctr;
    kpval_t type;
    char *value = NULL;
    const char *filename = NULL;
    enum {
        LIST,
        SET,
        GET,
        DEL,
        TYPE,
        RDEL,
        IMP
    } cmd;

    /* change to the working directory */
    if (chdir(NEO_PREFIX) < 0) {
    	printf("kptool Could not change to %s\n", NEO_PREFIX);
    	printf("Did you 'make install' after compiling?\n");
    	printf("Error Was: %s\n", strerror(errno));
        exit(-1);
    }
                                                                                
    if(argc < 2 || strcmp(argv[1], "-h") == 0 ||
       strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, usagestr, argv[0]);
        exit(1);
    }
    
    argctr = 1;
    arg = argv[argctr];
    if(arg[0] != '-') {
        fprintf(stderr, "Missing option\n");
        exit(1);
    }
    else if(strcmp(arg, "--recursive-remove") == 0) {
        argctr ++;
        if(argc < 3) {
            fprintf(stderr, "Too few arguments\n");
            exit(1);
        }
        cmd = RDEL;
    }
    else {
        if(strlen(arg) != 2) {
            fprintf(stderr, "Bad option: %s\n", arg);
            exit(1);
        }
        argctr++;
        switch(arg[1]) {
        case 'l':
            cmd = LIST;
            break;

        case 's':
            if(argc < 5) {
                fprintf(stderr, "Too few arguments\n");
                exit(1);
            }
            cmd = SET;
            arg = argv[argctr++];
            type = decode_type(arg);
            value = argv[argctr++];
            break;

        case 'g':
            if(argc < 4) {
                fprintf(stderr, "Too few arguments\n");
                exit(1);
            }
            cmd = GET;
            arg = argv[argctr++];
            type = decode_type(arg);
            break;

        case 'r':
            if(argc < 3) {
                fprintf(stderr, "Too few arguments\n");
                exit(1);
            }
            cmd = DEL;
            break;

        case 't':
            if(argc < 3) {
                fprintf(stderr, "Too few arguments\n");
                exit(1);
            }
            cmd = TYPE;
            break;

        case 'i':
            if(argc == 2)
                filename = NULL;
            else if(argc == 3) {
                arg = argv[argctr++];

                if(strcmp(arg, "-") == 0)
                    filename = NULL;
                else
                    filename = arg;
            }
            else {
                fprintf(stderr, "Too many arguments\n");
                exit(1);
            }
            cmd = IMP;
            break;

        default:
            fprintf(stderr, "Unrecognized option: %s\n", arg);
            exit(1);
        }

    }

    if(argc > argctr)
        key = argv[argctr++];
    else
        key = NULL;

    if(argc > argctr) {
        fprintf(stderr, "Too many arguments\n");
        exit(1);
    }

    switch(cmd) {
    case LIST:
        if(key == NULL) {
            kp_export(NULL, NULL, "l");
            kp_export(NULL, NULL, "u");
            kp_export(NULL, NULL, "g");
        }
        else
            kp_export(NULL, NULL, key);
        break;

    case SET:
        res = set_key(key, type, value);
        if(res != 0)
            fprintf(stderr, "%s: Set value failed: %s\n",
                    key, kp_strerror(res));
        break;

    case GET:
        res = get_key(key, type);
        if(res != 0)
            fprintf(stderr, "%s: Get value failed: %s\n",
                    key, kp_strerror(res));
        break;

    case DEL:
        res = kp_remove(key);
        if(res != 0)
            fprintf(stderr, "%s: Remove key failed: %s\n",
                    key, kp_strerror(res));
        else {
            res = kp_flush();
            if(res != 0)
                fprintf(stderr, "%s: Flush failed: %s\n",
                        key, kp_strerror(res));
        }

        break;

    case TYPE:
        res = kp_get_type(key, &type);
        if(res != 0)
            fprintf(stderr, "%s: Get type failed: %s\n",
                    key, kp_strerror(res));
        else
            printf("%s\n", code_type(type));

        break;

    case IMP:
        kp_import(filename, NULL);
        break;

    case RDEL:
        del_recursive(key);
        res = kp_flush();
        if(res != 0)
            fprintf(stderr, "%s: Flush failed: %s\n",
                    key, kp_strerror(res));
        break;
    }

#ifdef ALLOC_CHECK
    _kp_exit();
#endif

    return 0;
}
