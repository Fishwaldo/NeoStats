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
** $Id: kptool.h,v 1.2 2003/05/26 09:18:31 fishwaldo Exp $
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ALLOC_CHECK
#include <sys/types.h>
extern void *_iamalloc(size_t size, int id);
extern void *_iarealloc(void *ptr, size_t size, int id);
extern void  _iafree(void *ptr, int id);
#define malloc(size)        _iamalloc(size, 4)
#define free(ptr)           _iafree(ptr, 4)
#define realloc(ptr, size)  _iarealloc(ptr, size, 4)
#endif


/* Import, export utils */

int   kp_import (const char *filename, const char *basekey);
int   kp_export (const char *filename, const char *basekey,
                 const char *subkey);

