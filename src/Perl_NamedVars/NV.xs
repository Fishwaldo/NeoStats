#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "neostats.h"


MODULE = NeoStats::NV		PACKAGE = NeoStats::NV		

void
new(class,dev=NULL)
   char      *class;
   char      *dev;
PREINIT:
   SV        *ret;
   HV        *stash;
   HV        *hash;
   HV        *tie;
   SV        *dsv;
   SV        *tieref;
   char      *p;
   char      *n;
   char      *t;
   char       errstr[1024];
   int        i, fd, ii, count;
PPCODE:
   hash   = newHV();
   ret    = (SV*)newRV_noinc((SV*)hash);

   stash  = gv_stashpv(class,TRUE);
   sv_bless(ret,stash);

   stash  = gv_stashpv(class,TRUE);
   sv_bless(ret,stash);

   tie = newHV();
   tieref = newRV_noinc((SV*)tie);
   sv_bless(tieref, gv_stashpv("NeoStats::NV::User", TRUE));
   hv_magic(hash, (GV*)tieref, 'P'); 

   dsv = newSViv((IV)i);
   sv_magic(SvRV(tieref), dsv, '~', 0, 0);
   SvREFCNT_dec(dsv);

   EXTEND(SP,1);
   PUSHs(sv_2mortal(ret));

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
   mg = mg_find(SvRV(self), 'P');
   if(!mg) { croak("lost P magic"); }
   ref = mg->mg_obj;
   PUSHMARK(SP);
   XPUSHs(ref);
   for(i=2; i<items; i++)
      XPUSHs(ST(i));
   call_method(SvPV(prop,PL_na), G_SCALAR);

void
DESTROY(self)
   SV           *self;
CODE:

MODULE = NeoStats::NV		PACKAGE = NeoStats::NV::User

void
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
PPCODE:
   hash = (HV*)SvRV(self);
   k    = SvPV(key, klen);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   croak("kernel variable %s does not exist", k);
   EXTEND(SP,1);
   PUSHs(sv_2mortal(ret));
   
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


bool
EXISTS(self, key)
   SV   *self;
   SV   *key;
PREINIT:
   HV   *hash;
   char *k;
CODE:
   hash = (HV*)SvRV(self);
   k    = SvPV(key, PL_na);
   RETVAL = hv_exists_ent(hash, key, 0);
OUTPUT:
   RETVAL

SV*
FIRSTKEY(self)
   SV *self;
PREINIT:
   HV *hash;
   HE *he;
PPCODE:
   hash = (HV*)SvRV(self);
   hv_iterinit(hash);
   if (he = hv_iternext(hash)) {
      EXTEND(sp, 1);
      PUSHs(hv_iterkeysv(he));
   }

SV*
NEXTKEY(self, lastkey)
   SV *self;
   SV *lastkey;
PREINIT:
   HV *hash;
   HE *he;
PPCODE:
   hash = (HV*)SvRV(self);
   if (he = hv_iternext(hash)) {
      EXTEND(sp, 1);
      PUSHs(hv_iterkeysv(he));
   }

SV*
DELETE(self, key)
   SV        *self;
   SV        *key;
PREINIT:
   HV *hash;
   HE *he;
CODE:
   hash = (HV*)SvRV(self);
   croak("DELETE functions is not implemented");
OUTPUT:
   RETVAL

void
CLEAR(self)
   SV *self;
PREINIT:
   HV *hash;
CODE:
   hash = (HV*)SvRV(self);
   croak("CLEAR function is not implemented");

void
_lookup(self,prop,...)
   SV     *self;
   SV     *prop;
ALIAS:
#   NeoStats::NV::User::size       = F_SIZE
#   NeoStats::NV::User::bind       = F_BIND
#   NeoStats::NV::User::type       = F_TYPE
#   NeoStats::NV::User::visibility = F_VISB
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
