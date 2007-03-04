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

void nv_printstruct(void *data, nv_list *item);


char *fldtypes[] = {
	"Pointer String",
	"String",
	"Integer",
	"Long",
	"Void",
	"String Array"
};

nv_list *FindNamedVars(char *name) {
	nv_list *nv;
	nv = hnode_find(namedvars, name);
	if (nv) {
		return nv;
	} else {
		nlog(LOG_WARNING, "FindNamedVars: Can't find NamedVar %s", name);
		return NULL;
	}
}


int nv_init() {
	namedvars = hash_create(HASHCOUNT_T_MAX, 0, 0);
	return NS_SUCCESS;
}

hash_t *nv_hash_create(hashcount_t count, hash_comp_t comp, hash_fun_t fun, char *name2, nv_struct *nvstruct, nv_flags flags) {
	nv_list *newitem;
	if (!FindNamedVars(name2)) {
		newitem = ns_malloc(sizeof(nv_list));
		newitem->name = strdup(name2);
		newitem->type = NV_TYPE_HASH;
		newitem->flags = flags;
		newitem->format = nvstruct;
		newitem->mod = GET_CUR_MODULE();
		newitem->data = (void *)hash_create(count, comp, fun);
		hnode_create_insert(namedvars, newitem, newitem->name);
		return (hash_t *) newitem->data;
	} else {
		nlog(LOG_WARNING, "Can not create NamedVars Hash %s - Already Exists", name2);
		return hash_create(count, comp, fun);
	}
}

list_t *nv_list_create(listcount_t count, char *name2, nv_struct *nvstruct, nv_flags flags) {
	nv_list *newitem;
	if (!FindNamedVars(name2)) {
		newitem = ns_malloc(sizeof(nv_list));
		newitem->name = strdup(name2);
		newitem->type = NV_TYPE_LIST;
		newitem->flags = flags;
		newitem->format = nvstruct;
		newitem->mod = GET_CUR_MODULE();
		newitem->data = (void *)list_create(count);
		hnode_create_insert(namedvars, newitem, newitem->name);
		return (list_t *) newitem->data;
	} else {
		nlog(LOG_WARNING, "Can not create NamedVars List %s - Already Exists", name2);
		return list_create(count);
	}
}


#ifndef WIN32
int dump_namedvars(char *name2)
{
	hnode_t *node, *node2;
	lnode_t *lnode;
	hscan_t scan, scan1;
	void *data;	
	nv_list *item;
	int j;;
	hash_scan_begin(&scan1, namedvars);
	while ((node = hash_scan_next(&scan1)) != NULL ) {
		item = hnode_get(node);
		printf("%s (%p) Details: Type: %d Flags: %d Module: %s\n", item->name, item, item->type, item->flags, item->mod->info->name);
		printf("Entries:\n");
		printf("===================\n");
		if (item->type == NV_TYPE_HASH) {
			j = 0;
			hash_scan_begin(&scan, (hash_t *)item->data);
			while ((node2 = hash_scan_next(&scan)) != NULL) {
				data = hnode_get(node2);
				printf("Entry %d (%d)\n", j, (int)hash_count((hash_t *)item->data));
				nv_printstruct(data, item);
				j++;
			}
		} else if (item->type == NV_TYPE_LIST) {
			j = 0;
			lnode = list_first((list_t *)item->data);
			while (lnode) {
				data = lnode_get(lnode);
				printf("Entry %d (%d)\n", j, (int)list_count((list_t *)item->data));
				nv_printstruct(data, item);
				j++;
				lnode = list_next((list_t *)item->data, lnode);
			}
		}
	}
	return NS_SUCCESS;
}

char *nv_gf_string(const void *data, const nv_list *item, const int field) {
	char *output;
	void *data2;
	if (item->format[field].type == NV_PSTR) {
		if (item->format[field].fldoffset != -1) {
			data2 = (void *)data + item->format[field].fldoffset;
			data2 = (void *)*((int *)data2);
		} else {
			data2 = (void *)data;
		}		

		output = (char *)*(int *)(data2 + item->format[field].offset);
	} else if (item->format[field].type == NV_STR) {
		if (item->format[field].fldoffset != -1) {
			data2 = (void *)data + item->format[field].fldoffset;
			data2 = (void *)*((int *)data2);
		} else {
			data2 = (void *)data;
		}		
		output = (char *)(data2 + item->format[field].offset);
	} else {
		nlog(LOG_WARNING, "nv_gf_string: Field is not a string %d", field);
		return NULL;
	}
#ifdef DEBUG
	if (!ValidateString(output)) {
		nlog(LOG_WARNING, "nv_gf_string: Field is not string %d", field);
		return NULL;
	}
#endif
	return output;
}

int nv_gf_int(const void *data, const nv_list *item, const int field) {
	int output;
	void *data2;
	if (item->format[field].type != NV_INT) {
		nlog(LOG_WARNING, "nv_gf_int: field is not a int %d", field);
		return 0;
	}
	if (item->format[field].fldoffset != -1) {
		data2 = (void *)data + item->format[field].fldoffset;
		data2 = (void *)*((int *)data2);
	} else {
		data2 = (void *)data;
	}		
	output = *((int *)(data2 + item->format[field].offset));
	return output;
}

long nv_gf_long(const void *data, const nv_list *item, const int field) {
	long output;
	void *data2;
	if (item->format[field].type != NV_LONG) {
		nlog(LOG_WARNING, "nv_gf_long: field is not a long %d", field);
		return 0;
	}
	if (item->format[field].fldoffset != -1) {
		data2 = (void *)data + item->format[field].fldoffset;
		data2 = (void *)*((int *)data2);
	} else {
		data2 = (void *)data;
	}		
	output = *((long *)(data2 + item->format[field].offset));
	return output;
}

char **nv_gf_stringa(const void *data, const nv_list *item, const int field) {
	char **output;
	void *data2;
	int k;
	if (item->format[field].type != NV_PSTRA) {
		nlog(LOG_WARNING, "nv_gf_long: field is not a string array %d", field);
		return NULL;
	}
	if (item->format[field].fldoffset != -1) {
		data2 = (void *)data + item->format[field].fldoffset;
		data2 = (void *)*((int *)data2);
	} else {
		data2 = (void *)data;
	}		
	output = (char **)*(int *)(data2 + item->format[field].offset);
#ifdef DEBUG
	k = 0;
	while (output && output[k] != NULL) {
		if (!ValidateString(output[k])) 
			return NULL;
		k++;
	}
#endif
	return output;
}

void *nv_gf_complex(const void *data, const nv_list *item, const int field) {
	void *output, *data2;
#if 0
	if (item->format[field].type != NV_COMPLEX) {
		nlog(LOG_WARNING, "nv_gf_complex: field is not complex %d", field);
		return NULL;
	}
#endif
	if (item->format[field].fldoffset != -1) {
		data2 = (void *)data + item->format[field].fldoffset;
		data2 = (void *)*((int *)data2);
	} else {
		data2 = (void *)data;
	}		
	output = (void *)data2 + item->format[field].offset;
	return output;
}

int nv_get_field(const nv_list *item, const char *name) {
	int i = 0;
	while (item->format[i].fldname != NULL) {
		if (!ircstrcasecmp(item->format[i].fldname, name)) 
			return i;
		i++;
	}
	return -1;
}

void nv_printstruct(void *data, nv_list *item) {
	int i, k;
	char **outarry;
	
	i = 0;
	while (item->format[i].fldname != NULL) {
		printf("\tField: Name: %s, Type: %s, Flags: %d ", item->format[i].fldname, fldtypes[item->format[i].type], item->format[i].flags);
		switch (item->format[i].type) {
			case NV_PSTR:
				printf("Value: %s\n", nv_gf_string(data, item, i));
				break;
			case NV_STR:
				printf("Value: %s\n", nv_gf_string(data, item, i));
				break;
			case NV_INT:
				printf("Value: %d\n", nv_gf_int(data, item, i));
				break;
			case NV_LONG:
				printf("Value: %ld\n", nv_gf_long(data, item, i));
				break;
			case NV_VOID:
				printf("Value: Complex (%p)!\n", nv_gf_complex(data, item, i));
				break;
			case NV_PSTRA:
				k = 0;
				printf("\n");
				outarry = nv_gf_stringa(data, item, i);
				while (outarry && outarry[k] != NULL) {
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


#endif /* WIN32 */
