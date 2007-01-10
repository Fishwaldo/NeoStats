#include "neostats.h"
#include "namedvars.h"
#undef _
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"


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
   char      *p;
   char      *n;
   char      *t;
   char       errstr[1024];
   int        i, fd, ii, count;
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
   	sv_bless(tieref, gv_stashpv("NeoStats::NV::Vars", TRUE));
   	hv_magic(hash, (GV*)tieref, 'P'); 
	
	/* this one allows us to store a "pointer" 
         * at the moment, we are storing the name of the nv_list hash entry
         * but we should move this to the actual hash entry
         */
   	nv_link = newSVpv(varname, strlen(varname));
   	sv_magic(SvRV(tieref), nv_link, '~', 0, 0);
   	SvREFCNT_dec(nv_link);
   }
   /* return the hash */
   EXTEND(SP,1);
   PUSHs(sv_2mortal(ret));

# uknown when this is called at the moment

void
rAUTOLOAD(self,prop,...)
   SV           *self;
   SV           *prop;
PREINIT:
   MAGIC        *mg;
   SV           *ref;
   STRLEN        plen;
   char         *pval;
   int           i;
PPCODE:
printf("AUTOLOAD\n");
   mg = mg_find(SvRV(self), 'P');
   if(!mg) { croak("lost P magic"); }
   ref = mg->mg_obj;
   PUSHMARK(SP);
   XPUSHs(ref);
   for(i=2; i<items; i++)
      XPUSHs(ST(i));
   call_method(SvPV(prop,PL_na), G_SCALAR);

#when we destroy the "hash" in perl, ie, it goes out of scope

void
DESTROY(self)
   SV           *self;
CODE:
printf("DESTROY\n");

MODULE = NeoStats::NV		PACKAGE = NeoStats::NV::Vars

#/* get a individual entry. self points to what we set with sv_magic in
#* new function above */

HV *
FETCH(self, key)
   SV           *self;
   SV           *key;
PREINIT:
   SV           *ret;
   HV           *hash;
   char         *k;
   STRLEN        klen;
   MAGIC        *mg;
   SV          **val;
   char         *t;
   int           i;
   Client 	*u;
PPCODE:
   /* find our magic */
   hash = (HV*)SvRV(self);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   /* this is the nv_hash we are point at */
   printf("%s\n", SvPV_nolen(mg->mg_obj));
   /* get the "key" they want */
   k    = SvPV(key, klen);

   /* dummy code for now, lookup users */
   u = FindUser(k);
   if (!u) {
      ret = &PL_sv_undef;
   }  else {
	   ret = (SV *)perl_encode_client(u);
   }
   /* return a HV that cast to a SV instead */
   sv_2mortal(ret);
   EXTEND(SP, 1);
   PUSHs(newRV_noinc((SV *)ret));

#/* store a update. should check if its RO or RW though */

SV*
STORE(self, key, value)
   SV         *self;
   SV         *key;
   SV         *value;
PREINIT:
   HV         *hash;
   char       *k;
   char       *v;
   STRLEN      klen;
   STRLEN      vlen;
   MAGIC      *mg;
CODE:
   hash = (HV*)SvRV(self);
   k    = SvPV(key, klen);
   croak("STORE function is not implemented %s", k);
OUTPUT:
   RETVAL

#/* delete a entry from the hash */

void
DESTROY(self)
   SV        *self;
PREINIT:
   MAGIC     *mg;
   HV        *hash;
   HE        *hent;
   SV        *val;
   char      *k;
   I32        klen;
CODE:
   printf("DESTROY ITEM\n");
   mg = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
#   kp = (kvm_dev_t*)SvIVX(mg->mg_obj);
#   Safefree(kp);

   hash = (HV*)SvRV(self);
   hv_iterinit(hash);
   while(hent = hv_iternext(hash)) {
      k   = hv_iterkey(hent, &klen);
      val = hv_delete(hash, k, klen, 0);
   }


# /* check if a entry is in the hash */

bool
EXISTS(self, key)
   SV   *self;
   SV   *key;
PREINIT:
   HV   *hash;
   char *k;
CODE:
printf("EXISTS\n");
   hash = (HV*)SvRV(self);
   k    = SvPV(key, PL_na);
   RETVAL = hv_exists_ent(hash, key, 0);
OUTPUT:
   RETVAL

#/* get the first entry from a hash */

SV*
FIRSTKEY(self)
   SV *self;
PREINIT:
   HV *hash;
   HE *he;
PPCODE:
printf("FIRSTKEY\n");
   hash = (HV*)SvRV(self);
   hv_iterinit(hash);
   if (he = hv_iternext(hash)) {
      EXTEND(sp, 1);
      PUSHs(hv_iterkeysv(he));
   }

#/* get the next entry from a cache */

SV*
NEXTKEY(self, lastkey)
   SV *self;
   SV *lastkey;
PREINIT:
   HV *hash;
   HE *he;
PPCODE:
printf("NEXTKEY\n");
   hash = (HV*)SvRV(self);
   if (he = hv_iternext(hash)) {
      EXTEND(sp, 1);
      PUSHs(hv_iterkeysv(he));
   }

#/* delete a entry */

SV*
DELETE(self, key)
   SV        *self;
   SV        *key;
PREINIT:
   HV *hash;
   HE *he;
CODE:
printf("DELETE\n");
   hash = (HV*)SvRV(self);
   croak("DELETE functions is not implemented");
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

#/* latter */

void
_lookup(self,prop,...)
   SV     *self;
   SV     *prop;
ALIAS:
#   NeoStats::NV::Vars::size       = F_SIZE
#   NeoStats::NV::Vars::bind       = F_BIND
#   NeoStats::NV::Vars::type       = F_TYPE
#   NeoStats::NV::Vars::visibility = F_VISB
PREINIT:
   HV         *hash;
   SV        **var;
   SV         *ret;
   char       *pval;
   STRLEN      plen;
   int i;
PPCODE:
   hash = (HV*)SvRV(self);
   pval = SvPV(prop, plen);
   var  = hv_fetch(hash, pval, plen, FALSE);

   if(var) {
      ret = &PL_sv_undef;
   }
   EXTEND(SP,1);
   PUSHs(sv_2mortal(ret));
