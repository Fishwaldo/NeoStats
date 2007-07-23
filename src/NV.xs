#include "neostats.h"
#include "namedvars.h"
#undef _
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

/* XXX TODO: implement svREADONLY */

HV *perl_encode_namedvars(nv_list *nv, void *data) {
	HV *ret;
	int i =0;
	ret = newHV();
	while (nv->format[i].fldname != NULL) {
		switch(nv->format[i].type) {
			case NV_PSTR:
			case NV_STR:
				hv_store(ret, nv->format[i].fldname, strlen(nv->format[i].fldname),
					newSVpv(nv_gf_string(data, nv, i), strlen(nv_gf_string(data, nv, i))), 0);
				break;
			case NV_INT:
			case NV_LONG:
				hv_store(ret, nv->format[i].fldname, strlen(nv->format[i].fldname),
					newSViv(nv_gf_int(data, nv, i)), 0);
				break;
			case NV_VOID:
			case NV_PSTRA:
				nlog(LOG_WARNING, "perl_encode_namedvars: void/string todo!");
				break;
		}
	i++;
	}
	return ret;
}

void perl_store_namedvars(nv_list *nv, char *key, HV *values) {
	SV *value;
	int i;
	i = 0;
	while (nv->format[i].fldname != NULL) {
		printf("fld: %s %d\n", nv->format[i].fldname, i);
		if (hv_exists(values, nv->format[i].fldname, strlen(nv->format[i].fldname))) {
			value = *hv_fetch(values, nv->format[i].fldname, strlen(nv->format[i].fldname), FALSE);
		} else {
			i++;
			continue;
		}
		switch (nv->format[i].type) {
			case NV_PSTR:
			case NV_STR:
				printf("Value: %s\n", SvPV_nolen(value));
				break;
			default:
				printf("Value: Unhandled!\n");
				break;
		}
		i++;
	}

	return;
}

int perl_candelete_namedvars(nv_list *nv, char *key) {
	/* XXX - TODO */
	return NS_FAILURE;
}
void perl_delete_namedvars(nv_list *nv, char *key) {
	/* XXX - TODO */
	return;
}


#define	RETURN_UNDEF_IF_FAIL { if ((int)RETVAL < 0) XSRETURN_UNDEF; }

MODULE = NeoStats::NV		PACKAGE = NeoStats::NV		

void
new(class, varname)
   char      *varname;
   char      *class;
PREINIT:
   SV        *ret;
   HV        *stash;
   HV        *hash;
   HV        *tie;
   SV        *nv_link;
   SV        *tieref;
   nv_list   *nv;
PPCODE:
   nv = FindNamedVars(varname);
   if (!nv) {
	nlog(LOG_WARNING, "Perl NV: Can't find NamedVar list %s", varname);
     	ret = &PL_sv_undef;
   } else {
	/* this returns a "empty" hash that is tied to the FETCH/STORE etc functions below */
   	hash   = newHV();
  	ret    = (SV*)newRV_noinc((SV*)hash);
	stash  = gv_stashpv(class ,TRUE);
   	sv_bless(ret,stash);

	/* tie the hash to the package (FETCH/STORE) below */
   	tie = newHV();
	tieref = newRV_noinc((SV*)tie);
   	sv_bless(tieref, gv_stashpv("NeoStats::NV::HashVars", TRUE));
	hv_magic(hash, (GV*)tieref, 'P'); 
	/* this one allows us to store a "pointer" 
         */
   	nv_link = newSViv((int)nv);
   	sv_magic(SvRV(tieref), nv_link, '~', 0, 0);
   	SvREFCNT_dec(nv_link);
   }
   /* return the hash */
   EXTEND(SP,1);
   PUSHs(sv_2mortal(ret));


#when we destroy the "hash" in perl, ie, it goes out of scope

#void
#DESTROY(self)
#   SV           *self 
#CODE:
#printf("DESTROY\n");

#void
#FETCH(self)
#  SV	*self;
#CODE:
#printf("FETCH\n");

MODULE = NeoStats::NV		PACKAGE = NeoStats::NV::HashVars

#/* get a individual entry. self points to what we set with sv_magic in
#* new function above */

HV *
FETCH(self, key)
   SV           *self;
   SV           *key;
PREINIT:
   HV           *hash;
   char         *k;
   STRLEN        klen;
   MAGIC        *mg;
   nv_list	*nv;
   void 	*data;
   int		pos, i;
   lnode_t 	*lnode;
CODE:
   RETVAL = (HV *)-1;
   /* find our magic */
   hash = (HV*)SvRV(self);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   /* this is the nv_hash we are point at */
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* make sure its a hash, not a list */
   if (nv->type == NV_TYPE_HASH) {
	   /* get the "key" they want */
	   k    = SvPV(key, klen);
	   /* search for the key */
	   data = hnode_find((hash_t *)nv->data, k);
	   if (!data) {
		RETVAL = (HV *)-1;
	   } else {
		RETVAL = (HV *)perl_encode_namedvars(nv, data);	   
	   }
   } else if (nv->type == NV_TYPE_LIST) {
	   /* get the position */
	   pos = SvIV(key);
	   lnode = list_first((list_t *)nv->data);;
	   for (i = 0; i == pos; i++) {
			lnode = list_next((list_t *)nv->data, lnode);
	   }
	   if (lnode) {
		   RETVAL = perl_encode_namedvars(nv, lnode_get(lnode));
	   } else
		   RETVAL = (HV *)-1;

   }
POSTCALL:
	RETURN_UNDEF_IF_FAIL;
OUTPUT:
	RETVAL


#/* store a update. should check if its RO or RW though */

void
STORE(self, key, value)
   SV         *self;
   SV         *key;
   HV         *value;
PREINIT:
   HV         *hash;
   char       *k;
   STRLEN      klen;
   MAGIC      *mg;
   nv_list    *nv;
CODE:
   /* find our magic */
   hash = (HV*)SvRV(self);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   /* this is the nv_hash we are point at */
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* make sure its a hash, not a list */
   if (nv->type == NV_TYPE_HASH) {
	nlog(LOG_WARNING, "NamedVars is not a hash?!?!");
   } else if (nv->type == NV_TYPE_LIST) {
	   k    = SvPV(key, klen);
	   perl_store_namedvars(nv, k, value);
   }	      


# /* check if a entry is in the hash */

bool
EXISTS(self, key)
   SV   *self;
   SV   *key;
PREINIT:
   HV   *hash;
   char *k;
   nv_list *nv;
   MAGIC        *mg;
   STRLEN        klen;
   char 	*data;
   int 		pos;
CODE:
   RETVAL = 0;
   hash = (HV*)SvRV(self);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   /* this is the nv_hash we are point at */
   k    = SvPV(key, PL_na);
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* make sure its a hash, not a list */
   if (nv->type == NV_TYPE_HASH) {
	   /* get the "key" they want */
	   k    = SvPV(key, klen);
	   /* search for the key */
	   data = hnode_find((hash_t *)nv->data, k);
	   if (!data) {
		RETVAL = 0;
	   } else {
		RETVAL = 1;
	   }
   } else if (nv->type == NV_TYPE_LIST) {
	   pos = SvIV(key);
	   if (pos > list_count((list_t *)nv->data)) 
		RETVAL = 0;
	   else
		RETVAL = 1;
   }
OUTPUT:
   RETVAL

#/* get the first entry from a hash */

SV*
FIRSTKEY(self)
   SV *self;
PREINIT:
   HV *hash;
   MAGIC *mg;
   nv_list *nv;
   hnode_t *node;
CODE:
   RETVAL = &PL_sv_undef;
   hash = (HV*)SvRV(self);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* make sure its a hash, not a list */
   if (nv->type == NV_TYPE_HASH) {
	hash_scan_begin( &nv->iter.hscan, (hash_t *)nv->data);
	node = hash_scan_next(&nv->iter.hscan);
	nv->itercount = 0;
	if (!node) {
		RETVAL = &PL_sv_undef;
	} else {
		RETVAL = newSVpv(hnode_getkey(node), 0);
	}
   } else if (nv->type == NV_TYPE_LIST) {
	nv->iter.node = list_first((list_t *)nv->data);
	nv->itercount = 0;
	RETVAL = newSVpv("0", 0);
   }
POSTCALL:
	RETURN_UNDEF_IF_FAIL;
OUTPUT:
	RETVAL

#/* get the next entry from a cache */

SV*
NEXTKEY(self, lastkey)
   SV *self;
PREINIT:
   HV *hash;
   MAGIC *mg;
   hnode_t *node;
   nv_list *nv;
   char tmpstr[BUFSIZE];
CODE:
   RETVAL = &PL_sv_undef;
   hash = (HV*)SvRV(self);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* make sure its a hash, not a list */
   if (nv->type == NV_TYPE_HASH) {
	node = hash_scan_next(&nv->iter.hscan);
	nv->itercount++;
	if (!node) {
		RETVAL = &PL_sv_undef;
	} else {
		RETVAL = newSVpv(hnode_getkey(node), 0);
	}
   } else if (nv->type == NV_TYPE_LIST) {
	nv->itercount++;
	if (nv->itercount >= list_count((list_t *)nv->data)) {
		RETVAL = &PL_sv_undef;
	} else {
		ircsnprintf(tmpstr, BUFSIZE, "%d", nv->itercount);
		RETVAL =newSVpv(tmpstr,0);
	}
   }
POSTCALL:
	RETURN_UNDEF_IF_FAIL;
OUTPUT:
	RETVAL

#/* delete a entry */

HV*
DELETE(self, key)
   SV        *self;
   SV        *key;
PREINIT:
   HV *hash;
   MAGIC *mg;
   nv_list *nv;
   void *data;
   char *k;
   STRLEN klen;
CODE:
   hash = (HV*)SvRV(self);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* make sure its a hash, not a list */
   if (nv->type != NV_TYPE_HASH) {
	nlog(LOG_WARNING, "Del: NamedVars is not a hash?!?!");
	RETVAL = (HV *)-1;
   } else {
	   /* get the "key" they want */
	   k    = SvPV(key, klen);
	   /* search for the key */
	   data = hnode_find((hash_t *)nv->data, k);
	   if (!data) {
		RETVAL = (HV *)-1;
	   } else {
		if (perl_candelete_namedvars(nv, k)) {
			RETVAL = (HV *)perl_encode_namedvars(nv, data);	   
			perl_delete_namedvars(nv, k);
		} else {
			RETVAL = (HV *)-1;
		}
	   }
   }
POSTCALL:
	RETURN_UNDEF_IF_FAIL;
OUTPUT:
   RETVAL

#/* clear the hash */

void
CLEAR(self)
   SV *self;
PREINIT:
   HV *hash;
CODE:
printf("CLEAR\n");
   hash = (HV*)SvRV(self);
   croak("CLEAR function is not implemented");

