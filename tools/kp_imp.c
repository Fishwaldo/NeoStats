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
** $Id: kp_imp.c,v 1.2 2003/05/26 09:18:31 fishwaldo Exp $
*/
/*
 * kp_imp.c: import a standard file format into the keeper database
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
#include <stdarg.h>
#include <errno.h>

enum TokenType {
    TOK_ERROR = -1,
    TOK_NONE  = 0,
    TOK_STRING,
    TOK_EQUALS,
    TOK_OPENPAREN,
    TOK_CLOSEPAREN,
    TOK_OPENBRACE,
    TOK_CLOSEBRACE,
    TOK_ENDTERM
};

typedef struct {
    enum TokenType t;
    char *s;
    unsigned int l;
    unsigned int _size;
} Token;

typedef struct {
    FILE *fp;
    int linenum;
    int res;
} File;

static void add_err(File *file, const char *format, ...)
{
    va_list ap;
    
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    file->res = -1;
}

static void free_token(Token *tok)
{
    free(tok->s);
    
    tok->s = NULL;
    tok->l = 0;
}

static void add_char(int cval, Token *tok)
{
    if(tok->l + 1 >= tok->_size) {
        tok->_size += 16;
        tok->s = (char *) realloc(tok->s, tok->_size);
        assert(tok->s != NULL);
    }

    tok->s[tok->l] = (char) cval;
    tok->l++;
    tok->s[tok->l] = '\0';

}

static int get_escaped_char(File *file, int c)
{
    int cval;

    switch(c) {
    case 'b':
        cval = '\b';
        break;
        
    case 'f':
        cval = '\f';
        break;
        
    case 'n':
        cval = '\n';
        break;
        
    case 'r':
        cval = '\r';
        break;
        
    case 't':
        cval = '\t';
        break;
        
    case 'v':
        cval = '\v';
        break;
        
    default:
        if(c >= '0' && c < '8') {
            cval = c - '0';
            c = getc(file->fp);
            if(c != EOF) {
                if(c >= '0' && c < '8') {
                    cval = cval * 8 + c - '0';
                    
                    c = getc(file->fp);
                    if(c != EOF) {
                        if(c >= '0' && c < '8') {
                            cval = cval * 8 + c - '0';
                        }
                        else 
                            ungetc(c, file->fp);
                    }
                }
                else 
                    ungetc(c, file->fp);
            }
        }
        else
            cval = c;
    }

    return cval;
}

static int get_token(File *file, Token *tok)
{
    int c;
    int incomm = 0;
    int inquote = 0;
    int endoftok = 0;
    int cval;

    tok->t = TOK_NONE;
    tok->s = NULL;
    tok->l = 0;
    tok->_size = 0;

    while(!endoftok) {
        c = getc(file->fp);
        if(c == EOF) {
            endoftok = 1;
            if(inquote) {
                tok->t = TOK_ERROR;
                add_err(file, "Parse error [%i]: Unexpected end of file\n",
                        file->linenum);
            }
            continue;
        }

        if(c == '\n') {
            file->linenum ++;
            
            if(incomm)
                incomm = 0;
        }

        if(incomm) 
            continue;
                
        if(inquote) {
            if(c == '"') {
                endoftok = 1;
                continue;
            }
        }
        else {
            if(isspace(c)) {
                if(tok->t != TOK_NONE)
                    endoftok = 1;

                continue;
            }

            if(tok->t != TOK_NONE) {
                switch(c) {
                case '#':
                case '=':
                case '(':
                case ')':
                case '{':
                case '}':
                case ';':
                case '"':
                    endoftok = 1;
                    ungetc(c, file->fp);
                    continue;
                    
                }
            }

            switch(c) {
            case '#':
                incomm = 1;
                continue;

            case '=':
                endoftok = 1;
                tok->t = TOK_EQUALS;
                continue;

            case '(':
                endoftok = 1;
                tok->t = TOK_OPENPAREN;
                continue;

            case ')':
                endoftok = 1;
                tok->t = TOK_CLOSEPAREN;
                continue;

            case '{':
                endoftok = 1;
                tok->t = TOK_OPENBRACE;
                continue;

            case '}':
                endoftok = 1;
                tok->t = TOK_CLOSEBRACE;
                continue;
                
            case ';':
                endoftok = 1;
                tok->t = TOK_ENDTERM;
                continue;

            case '"':
                inquote = 1;
                tok->t = TOK_STRING;
                continue;
            }
            
            if(c < 32 || c == 127) {
                endoftok = 1;
                tok->t = TOK_ERROR;
                add_err(file,
                        "Parse error [%i]: illegal char '\\%03o'\n", 
                        file->linenum, c);
                continue;
            }
            
            tok->t = TOK_STRING;
        }

        if(c == '\\') {
            c = getc(file->fp);

            if(c == EOF) {
                endoftok = 1;
                tok->t = TOK_ERROR;
                add_err(file, "Parse error [%i]: Unexpected end of file\n",
                        file->linenum);
                continue;
            }
            
            cval = get_escaped_char(file, c);
        }
        else 
            cval = c;

        if(cval > 255) {
            endoftok = 1;
            tok->t = TOK_ERROR;
            add_err(file,
                    "Parse error [%i]: Escape sequence out of range\n", 
                    file->linenum);
            continue;
        }
        
        add_char(cval, tok);
    }    

    if(tok->t == TOK_ERROR) {
        free_token(tok);
        
        return 0;
    }

    if(tok->t == TOK_NONE)
        return 0;
    
    /* FIXME: Ugly hack... */
    if(tok->t == TOK_STRING && tok->l == 0) {
        add_char(0, tok);
        tok->l--;
    }

    return 1;
}

static kpval_t decode_type(char *str)
{
    if(strlen(str) != 1)
        return KPVAL_UNKNOWN;
    
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
        return KPVAL_UNKNOWN;
    }
}

static void free_tokens(Token *ts, int n)
{
    int i;

    if(ts == NULL)
        return;

    for(i = 0; i < n; i++)
        free_token(&ts[i]);
    
    free(ts);
}

static int get_line(File *file, Token **tokens)
{
    Token *ts;
    int n;
    int ts_size;
    int ok;
    Token tok;

    n = 0;
    ts = NULL;
    ts_size = 0;

    *tokens = NULL;

    while(1) {
        ok = get_token(file, &tok);
        if(!ok) {
            if(tok.t == TOK_ERROR)
                return -1;
            if(n > 0) {
                add_err(file, "Parse error [%i]: Unexpected end of file\n",
                        file->linenum);

                free_tokens(ts, n);
                return -1;
            }

            return 0;
        }

        if(n >= ts_size) {
            ts_size += 16;
            ts = (Token *) realloc(ts, ts_size * sizeof(Token));
            assert(ts != NULL);
        }
        ts[n++] = tok;
        
        if(tok.t == TOK_OPENBRACE || tok.t == TOK_CLOSEBRACE ||
           tok.t == TOK_ENDTERM)
            break;
    }

    *tokens = ts;
    return n;
}

static int set_value(File *file, const char *key,  kpval_t type, char *val)
{
    int ival;
    double fval;
    char *beg, *end;
    unsigned char *data;
    unsigned int datalen, datasize;
    int res;
    unsigned int sd;

    switch(type) {
    case KPVAL_DATA:
        data = NULL;
        datalen = 0;
        datasize = 0;
        
        for(beg = val; *beg; beg = end) {
            sd = (unsigned int) strtol(beg, &end, 0);
            if(beg == end) {
                add_err(file, "Parse error [%i]: value '%s' is not data\n", 
                        file->linenum, val);
                return -1;
            }
            if(datalen >= datasize) {
                datasize += 16;
                data = (unsigned char *) realloc(data, datasize);
                assert(data != NULL);
            }
            data[datalen++] = (unsigned char) sd;
        }

        res = kp_set_data(key, data, datalen);
        free(data);
        break;

    case KPVAL_STRING:
        res = kp_set_string(key, val);
        break;

    case KPVAL_INT:
        ival = (int) strtol(val, &end, 0);
        if(*end != '\0') {
            add_err(file, "Parse error [%i]: value '%s' is not integer\n", 
                    file->linenum, val);
            return -1;
        }
        res = kp_set_int(key, ival);
        break;
        

    case KPVAL_FLOAT:
        fval = strtod(val, &end);
        if(*end != '\0') {
            add_err(file, "Parse error [%i]: value '%s' is not float\n", 
                    file->linenum, val);
            return -1;
        }
        res = kp_set_float(key, fval);
        break;

    default:
        res = 0;
    }

    if(res != 0)
        add_err(file, "Could not set key '%s': %s\n", key,
                kp_strerror(res));

    return 0;
}

static int get_term(File *file, KPDIR *base, Token *ts, int n)
{
    kpval_t type;
    char *val;
    char *key;
    int res;

    if(n == 1) {
        /* empty term */
        return 0;
    }

    if(n < 3 || ts[0].t != TOK_STRING) {
        add_err(file, "Parse error [%i]: Bad syntax\n", file->linenum);
        return -1;
    }
    key = ts[0].s;

    if(ts[1].t == TOK_EQUALS) {
        if(n != 4 || ts[2].t != TOK_STRING) {
            add_err(file, "Parse error [%i]: Bad syntax\n", file->linenum);
            return -1;
        }
        
        type = KPVAL_STRING;
        val = ts[2].s;
    }
    else {
        if(n != 7 || ts[0].t != TOK_STRING || ts[1].t != TOK_OPENPAREN ||
           ts[2].t != TOK_STRING || ts[3].t != TOK_CLOSEPAREN ||
           ts[4].t != TOK_EQUALS || ts[5].t != TOK_STRING) {
            add_err(file, "Parse error [%i]: Bad syntax\n", file->linenum);
            return -1;
        }

        type = decode_type(ts[2].s);
        if(type == KPVAL_UNKNOWN) {
            add_err(file, "Parse error [%i]: Bad type: (%s)\n", 
                    file->linenum, ts[2].s);
            return -1;
        }

        val = ts[5].s;
    }

    res = set_value(file, KP_P(base, key), type, val);

    return res;
}

static int get_entry(File *file, KPDIR *base)
{
    Token *ts;
    int n;
    int ret = 0;

    while(1) {
        n = get_line(file, &ts);
        if(n <= 0)
            return n;

        if(n == 1 && ts[0].t == TOK_CLOSEBRACE) {
            ret = 1;
            break;
        }

        if(ts[n-1].t == TOK_OPENBRACE) {
            KPDIR *newbase;

            if(n != 3 || ts[0].t != TOK_STRING || ts[1].t != TOK_EQUALS) {
                add_err(file, "Parse error [%i]: Bad syntax\n",
			file->linenum);
                ret = -1;
                break;
            }
            
            newbase = kp_dir_open(KP_P(base, ts[0].s));
            ret = get_entry(file, newbase);
            kp_dir_close(newbase);
            
            if(ret < 0)
                break;

            if(ret == 0) {
                add_err(file, "Parse error [%i]: Missing closing brace\n",
                        file->linenum);
                ret = -1;
                break;
            }
        }
        else if(ts[n-1].t == TOK_ENDTERM) {
            ret = get_term(file, base, ts, n);
            if(ret < 0)
                break;
        }
        else {
            add_err(file, "Parse error [%i]: Missing semicolon\n",
		    file->linenum);
            ret = -1;
            break;
        }

        free_tokens(ts, n);
    }

    free_tokens(ts, n);
    return ret;
}

int kp_import(const char *filename, const char *basekey)
{
    KPDIR *bd;
    File file;
    int res;

    if(filename != NULL) {
	file.fp = fopen(filename, "r");
	if(file.fp == NULL) {
	    add_err(&file, "Could not open '%s': %s\n", filename,
		    strerror(errno));
	    return -1;
	}
    }
    else
	file.fp = stdin;

    if(basekey != NULL)
	bd = kp_dir_open(basekey);
    else
	bd = NULL;

    file.linenum = 0;
    file.res = 0;

    get_entry(&file, bd);

    if(bd != NULL)
	kp_dir_close(bd);

    if(filename != NULL)
        fclose(file.fp);

    res = kp_flush();
    if(res != 0)
        add_err(&file, "Flush failed: %s\n", kp_strerror(res));

    return file.res;
}

