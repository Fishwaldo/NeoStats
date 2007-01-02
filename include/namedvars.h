/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2006 Adam Rutter, Justin Hammond, Mark Hetherington
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

#ifndef _NAMEDVARS_H_
#define _NAMEDVARS_H_
typedef enum {
	NV_STR,
	NV_PSTR,
	NV_INT,
	NV_LONG,
	NV_VOID,
	NV_PSTRA
} nv_struct_type;
typedef enum {
	NV_FLG_NONE,
	NV_FLG_RO
} nv_struct_flag;

typedef struct nv_struct {
	/* name of the field */
	char *fldname;
	/* type of filed */
	nv_struct_type type;
	/* offset of the field */
	int offset;
	/* flags of the field */
	nv_struct_flag flags;
	/* if its a substructure, the offset of the actual field, otherwise -1 */
	int fldoffset;
} nv_struct;

typedef enum {
	NV_TYPE_LIST,
	NV_TYPE_HASH
} nv_type;

typedef enum {
	NV_FLAGS_NONE,
	NV_FLAGS_RO
} nv_flags;

typedef struct nv_list {
	/* name of the list/hash */
	char *name;
	/* type of list/hash */
	nv_type type;
	/* description of the fields */
	nv_struct *format;
	/* flags */
	nv_flags flags;
	/* ptr to the list */
	void *data;
} nv_list;


extern hash_t *namedvars;

EXPORTFUNC hash_t *nv_hash_create(hashcount_t count, hash_comp_t comp, hash_fun_t fun, char *name, nv_struct *nvstruct, nv_flags flags);
int nv_init();


#endif /* _NAMEDVARS_H_ */
