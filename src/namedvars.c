/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2007 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000 - 2001 ^Enigma^
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

/** @file namedvars.c 
 *  @brief named variable support for NeoStats Hash and List features
 */ 

#include "neostats.h"
#include "namedvars.h"
#include "namedvars-core.h"

hash_t *namedvars;

nv_list *FindNamedVars(char *name) {
	hnode_t *node;
	node = hnode_find(namedvars, name);
	if (node) {
		return hnode_get(node);
	} else {
		return NULL;
	}
}


int nv_init() {
	namedvars = hash_create(HASHCOUNT_T_MAX, 0, 0);
	return NS_SUCCESS;
}

hash_t *nv_hash_create(hashcount_t count, hash_comp_t comp, hash_fun_t fun, char *name2, nv_struct *nvstruct, nv_flags flags) {
	nv_list *newitem;
	newitem = ns_malloc(sizeof(nv_list));
	newitem->name = strdup(name2);
	newitem->type = NV_TYPE_HASH;
	newitem->flags = flags;
	newitem->format = nvstruct;
	newitem->mod = GET_CUR_MODULE();
	newitem->data = (void *)hash_create(count, comp, fun);
	hnode_create_insert(namedvars, newitem, newitem->name);
	return (hash_t *) newitem->data;
}

list_t *nv_list_create(listcount_t count, char *name2, nv_struct *nvstruct, nv_flags flags) {
	nv_list *newitem;
	newitem = ns_malloc(sizeof(nv_list));
	newitem->name = strdup(name2);
	newitem->type = NV_TYPE_LIST;
	newitem->flags = flags;
	newitem->format = nvstruct;
	newitem->mod = GET_CUR_MODULE();
	newitem->data = (void *)list_create(count);
	hnode_create_insert(namedvars, newitem, newitem->name);
	return (list_t *) newitem->data;
}


#ifndef WIN32
int dump_namedvars(char *name2)
{
	hnode_t *node, *node2;
	hscan_t scan, scan1;
	void *data, *data2;	
	nv_list *item;
	int i, j, k;
	void *output;
	char **outarry;
	hash_scan_begin(&scan1, namedvars);
	while ((node = hash_scan_next(&scan1)) != NULL ) {
		item = hnode_get(node);
		printf("%s Details: Type: %d Flags: %d Module: %s\n", item->name, item->type, item->flags, item->mod->info->name);
		printf("Entries:\n");
		printf("===================\n");
/* XXX todo - Lists */
		if (item->type == NV_TYPE_HASH) {
			j = 0;
			hash_scan_begin(&scan, (hash_t *)item->data);
			while ((node2 = hash_scan_next(&scan)) != NULL) {
				data = hnode_get(node2);
				i = 0;
				printf("Entry %d (%d)\n", j, (int)hash_count((hash_t *)item->data));
				j++;
				while (item->format[i].fldname != NULL) {
					printf("\tField: Name: %s, Type: %d, Flags: %d ", item->format[i].fldname, item->format[i].type, item->format[i].flags);
					if (item->format[i].fldoffset != -1) {
						data2 = data + item->format[i].fldoffset;
						data2 = (void *)*((int *)data2);
					} else {
						data2 = data;
					}		
					switch (item->format[i].type) {
						case NV_PSTR:
							output = data2 + item->format[i].offset;
							printf("Value: %s\n", (char *)*(int *)output);
							break;
						case NV_STR:
							output = data2 + item->format[i].offset;
							printf("Value: %s\n", (char *)output);
							break;
						case NV_INT:
							output = data2 + item->format[i].offset;
							printf("Value: %d\n", *((int *)output));
							break;
						case NV_LONG:
							output = data2 + item->format[i].offset;
							printf("Value: %ld\n", *((long *)output));
							break;
						case NV_VOID:
							printf("Value: Complex!\n");
							break;
						case NV_PSTRA:
							output = data2 + item->format[i].offset;
							outarry = (char **)*(int *)output;
							k = 0;
							printf("\n");
							while (outarry[k] != NULL) {
								printf("\t\tValue [%d]: %s\n", k, outarry[k]);
								k++;
							}
							break;			
						default:
							printf("Value: Unhandled!\n");
							break;
					}
					i++;
				}
			}
		}
	}
	return NS_SUCCESS;
}
#endif /* WIN32 */
