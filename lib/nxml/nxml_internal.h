/* nXml - Copyright (C) 2005 bakunin - Andrea Marchesini 
 *                                <bakunin@autistici.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __N_XML_INTERNAL_H__
#define __N_XML_INTERNAL_H__

#include "nxml.h"

/* Rule [4] */
#define __NXML_NAMESTARTCHARS \
	  ((ch=__NXML_U8()) == ':' \
	  || (ch >= 'a' && ch <= 'z') \
	  || ch=='_' \
	  || (ch >= 'A' && ch <= 'Z') \
	  || (ch >= 0xc0 && ch <= 0xd6) \
	  || (ch >= 0xd8 && ch <= 0xf6) \
	  || (ch >= 0xf8 && ch <= 0x2ff) \
	  || (ch >= 0x370 && ch <= 0x37d) \
	  || (ch >= 0x37f && ch <= 0x1fff) \
	  || (ch >= 0x200c && ch <= 0x200d) \
	  || (ch >= 0x2070 && ch <= 0x218f) \
	  || (ch >= 0x2c00 && ch <= 0x2fef) \
	  || (ch >= 0x3001 && ch <= 0xd7ff) \
	  || (ch >= 0xf900 && ch <= 0xfdcf) \
	  || (ch >= 0xfdf0 && ch <= 0xfffd) \
	  || (ch >= 0x10000 && ch <= 0xeffff))

/* Rule [4a] */
#define __NXML_NAMECHARS \
	  (__NXML_NAMESTARTCHARS \
	  || ch=='-' \
	  || ch=='.' \
	  || (ch >= '0' && ch <= '9') \
	  || ch==0xb7 \
	  || (ch >= 0x0300 && ch <= 0x036f) \
	  || (ch >= 0x203f && ch <= 0x2040))

#define __NXML_U8() __nxml_utf8((unsigned char **)buffer, size, &byte)

typedef struct __nxml_download_t__ __nxml_download_t;
typedef struct __nxml_string_t__ __nxml_string_t;

/**
 * \brief
 * For internal use only
 */
struct __nxml_download_t__
{
  char *mm;
  size_t size;
};

__nxml_download_t *		__nxml_download_file	(nxml_t *nxml,
							 char *url);

int64_t				__nxml_utf8		(unsigned char **buffer,
	       					 	 size_t *size,
							 int *byte);

int64_t				__nxml_int_charset	(int i,
							 unsigned char *buffer,
							 char *charset);

int				__nxml_utf_detection	(char *r_buffer,
							 size_t r_size,
							 char **buffer,
				   			 size_t *size,
							 nxml_charset_t *);

int				__nxml_escape_spaces	(nxml_t * doc,
							 char **buffer,
							 size_t * size);

char *				__nxml_get_value	(nxml_t * doc,
							 char **buffer,
							 size_t * size);

char *				__nxml_trim		(char *tmp);

char *				__nxml_entity_trim	(nxml_t * nxml,
							 char *str);
	

/* nxml_string.c */

/**
 * \brief
 * For internal use only
 */
struct __nxml_string_t__
{
  char *string;
  size_t size;
};

__nxml_string_t	*		__nxml_string_new	(void);

int				__nxml_string_add	(__nxml_string_t *st,
							 char *what,
							 size_t size);

char *				__nxml_string_free	(__nxml_string_t *st);


void				__nxml_namespace_parse	(nxml_t *nxml);

void				__nxml_entity_parse	(nxml_t * nxml);

#endif

/* EOF */

