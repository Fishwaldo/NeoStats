/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

/* @file NeoStats interface to Perl Interpreter
 *  based on perl plugin from Xchat client
 */

#include "neostats.h"
#include "services.h"
#include "modules.h"
#undef _
#define PERLDEFINES
#include "perlmod.h"
#include <sys/types.h>
#include <dirent.h>

extern void boot_DynaLoader (pTHX_ CV * cv);
static void dump_hash(HV *rethash);




#ifdef WIN32
/* TODO: Mark, I got no idea about this win32 stuff, I'll leave it here for you to look at */
static DWORD
child (char *str)
{
	MessageBoxA (0, str, "Perl DLL Error",
					 MB_OK | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);
	return 0;
}

static void
thread_mbox (char *str)
{
	DWORD tid;

	CloseHandle (CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) child,
										str, 0, &tid));
}

#endif

static void
dump_hash(HV *rethash) {
	char *key;
	SV *value;
	int keycount;
	I32 *keylen;

	keycount = hv_iterinit(rethash);
	printf("%d items in call\n",keycount);
	while(keycount-- != 0) {
		value = hv_iternextsv(rethash, &key, &keylen);
		printf("Return (%s) -> (%s)\n",key,SvPV_nolen(value));
	}
}



/*
  this is used for autoload and shutdown callbacks
*/
static int
execute_perl (Module *mod, SV * function, int numargs, ...)
{

	int count, ret_value = 1;
	SV *sv;
	va_list args;
	char *tmpstr[10];

	PERL_SET_CONTEXT((PMI *)mod->pm->my_perl);
	SET_RUN_LEVEL(mod);

	dSP;
	ENTER;
	SAVETMPS;
	PUSHMARK (SP);

	va_start(args, numargs);
	for (count = 0; count < numargs; count++) {
		tmpstr[count] = va_arg(args, char *);
		XPUSHs (sv_2mortal (newSVpv (tmpstr[count], 0)));
	}
	va_end(args);

	PUTBACK;

	count = call_sv (function, G_EVAL | G_SCALAR);
	SPAGAIN;

	sv = GvSV (gv_fetchpv ("@", TRUE, SVt_PV));
	if (SvTRUE (sv)) {
		nlog(LOG_WARNING, "Perl error: %s", SvPV(sv, count)); 
		POPs;							  /* remove undef from the top of the stack */
	} else if (count != 1) {
		nlog(LOG_WARNING, "Perl error: expected 1 value from %s, "
						  "got: %d", (char *)function, count);
	} else {
		ret_value = POPi;
	}

	PUTBACK;
	FREETMPS;
	LEAVE;
	RESET_RUN_LEVEL();
	return ret_value;
}

int
perl_sync_module(Module *mod) {
	mod->insynch = 1;
	execute_perl (mod, sv_2mortal (newSVpv ("NeoStats::Embed::sync", 0)),1, mod->pm->filename);
	mod->synched = 1;
	return NS_SUCCESS;
}

int
perl_event_cb(Event evt, CmdParams *cmdparams, Module *mod_ptr) {
	int ret = NS_FAILURE;
	switch (evt) {
		case EVENT_NULL:
			nlog(LOG_WARNING, "Ehhh, PerlModule got callback for EVENT_NULL?");
			break;
		case EVENT_MODULELOAD:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->param);
			break;
		case EVENT_MODULEUNLOAD:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->param);
			break;
		case EVENT_SERVER:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_SQUIT:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_PING:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_PONG:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_SIGNON:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_QUIT:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_NICKIP:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_KILL:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_GLOBALKILL:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_LOCALKILL:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_SERVERKILL:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_BOTKILL:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_NICK:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_AWAY:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_UMODE:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_SMODE:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_NEWCHAN:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->channel->name);
			break;
		case EVENT_DELCHAN:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->channel->name);
			break;
		case EVENT_JOIN:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->channel->name, cmdparams->source->name);
			break;
		case EVENT_PART:
			/* XXX Something wrong here */
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->channel->name, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_PARTBOT:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 3, cmdparams->channel->name, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_EMPTYCHAN:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 4, cmdparams->channel->name, cmdparams->source->name, cmdparams->bot->name, cmdparams->param);
			break;
		case EVENT_KICK:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 4, cmdparams->channel->name, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_KICKBOT:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 4, cmdparams->channel->name, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_TOPIC:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->channel->name, cmdparams->source->name);
			break;
		case EVENT_CMODE:
			dlog(DEBUG1, "EVENT_CMODE todo!");
			break;
		case EVENT_PRIVATE:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->bot->name, cmdparams->param);
			break;
		case EVENT_NOTICE:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->bot->name, cmdparams->param);
			break;
		case EVENT_CPRIVATE:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->channel->name, cmdparams->param);
			break;
		case EVENT_CNOTICE:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->channel->name, cmdparams->param);
			break;
		case EVENT_GLOBOPS:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CHATOPS:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_WALLOPS:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPVERSIONRPL:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPVERSIONREQ:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_CTCPFINGERRPL:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPFINGERREQ:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_CTCPACTIONREQ:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_CTCPTIMERPL:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPTIMEREQ:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_CTCPPINGRPL:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPPINGREQ:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_DCCSEND:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_DCCCHAT:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_DCCCHATMSG:
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_ADDBAN:
		case EVENT_DELBAN:
			dlog(DEBUG1, "EVENT_*BAN Todo!");
			break;
		case EVENT_COUNT:
			/* nothing */
			break;
			
	}
	return ret;
}

/* encode a Client Structure into a perl Hash */
HV *perl_encode_client(Client *u) {
	HV *client;
	
	client = newHV();
	
	hv_store(client, "nick", 4, 
		newSVpv(u->name, strlen(u->name)), 0);
	hv_store(client, "username", 8, 
		newSVpv(u->user->username, strlen(u->user->username)), 0);
	hv_store(client, "hostname", 8, 
		newSVpv(u->user->hostname, strlen(u->user->hostname)), 0);
	hv_store(client, "vhost", 5, 
		newSVpv(u->user->hostname, strlen(u->user->hostname)), 0);
	hv_store(client, "awaymsg", 7, 
		newSVpv(u->user->awaymsg, strlen(u->user->awaymsg)), 0);
	hv_store(client, "swhois", 6, 
		newSVpv(u->user->swhois, strlen(u->user->swhois)), 0);
	hv_store(client, "userhostmask", 12, 
		newSVpv(u->user->userhostmask, strlen(u->user->userhostmask)), 0);
	hv_store(client, "uservhostmask", 13, 
		newSVpv(u->user->uservhostmask, strlen(u->user->uservhostmask)), 0);
	hv_store(client, "server", 6, 
		newSVpv(u->uplink->name, strlen(u->uplink->name)), 0);
	hv_store(client, "is_away", 7, 
		newSViv(u->user->is_away), 0);
	hv_store(client, "umodes", 6, 
		newSVpv(u->user->modes, strlen(u->user->modes)), 0);
	hv_store(client, "Umode", 5, 
		newSViv(u->user->Umode), 0);
	hv_store(client, "smodes", 6, 
		newSVpv(u->user->smodes, strlen(u->user->smodes)), 0);
	hv_store(client, "Smode", 5, 
		newSViv(u->user->Smode), 0);
	hv_store(client, "Ulevel", 6, 
		newSViv(u->user->ulevel), 0);
	return (client);
}

/* encode a Server Structure into a perl Hash */
HV *perl_encode_server(Client *u) {
	HV *client;
	
	client = newHV();
	
	hv_store(client, "name", 4, 
		newSVpv(u->name, strlen(u->name)), 0);
	hv_store(client, "uplink", 6, 
		newSVpv(u->uplinkname, strlen(u->uplinkname)), 0);
	hv_store(client, "users", 5, 
		newSViv(u->server->users), 0);
	hv_store(client, "awaycount", 9, 
		newSViv(u->server->awaycount), 0);
	hv_store(client, "hops", 4, 
		newSViv(u->server->hops), 0);
	hv_store(client, "ping", 4, 
		newSViv(u->server->ping), 0);
	hv_store(client, "uptime", 6, 
		newSViv(u->server->uptime), 0);
	return (client);
}

/* encode a Client Structure into a perl Hash */
HV *perl_encode_channel(Channel *c) {
	HV *client;
	
	client = newHV();
	
	hv_store(client, "name", 4, 
		newSVpv(c->name, strlen(c->name)), 0);
	hv_store(client, "topic", 5, 
		newSVpv(c->topic, strlen(c->topic)), 0);
	hv_store(client, "topicowner", 10, 
		newSVpv(c->topicowner, strlen(c->topicowner)), 0);
	hv_store(client, "topictime", 9, 
		newSViv(c->topictime), 0);
	hv_store(client, "users", 5, 
		newSViv(c->users), 0);
	hv_store(client, "modes", 5, 
		newSViv(c->modes), 0);
	hv_store(client, "limit", 5, 
		newSViv(c->limit), 0);
	hv_store(client, "key", 3, 
		newSVpv(c->key, strlen(c->key)), 0);
	hv_store(client, "createtime", 10, 
		newSViv(c->creationtime), 0);
	/* XXX todo: Encode Mode Params */
	return (client);
}



/* NeoStats::Internal::register (scriptname, version, desc)
 *
 */

static
XS (XS_NeoStats_register)
{
	Module *mod;
	dXSARGS;


	if (items != 3) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::register(scriptname, version, desc)");
	} else {
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		mod->info->name = SvPV_nolen (ST (0));
		mod->info->version = SvPV_nolen (ST (1));
		mod->info->description = SvPV_nolen (ST (2));
#if 0
		name = SvPV_nolen (ST (0));
		version = SvPV_nolen (ST (1));
		desc = SvPV_nolen (ST (2));
		mod->info->name = os_malloc(strlen(name)+1);
		strlcpy((char *)mod->info->name, name, strlen(name)+1);

		mod->info->description = os_malloc(strlen(desc)+1);
		strlcpy((char *)mod->info->description, desc, strlen(desc)+1);

		mod->info->version = os_malloc(strlen(version)+1);
		strlcpy((char *)mod->info->version, version, strlen(version)+1);
#endif		

		XSRETURN_UV (PTR2UV (mod));

	}
}

/* NeoStats::debug(output) */
static
XS (XS_NeoStats_debug)
{

	char *text = NULL;

	dXSARGS;
	if (items != 1) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::print(text)");
	} else {
		text = SvPV_nolen (ST (0));
		nlog(LOG_WARNING, "%s", text);
	}
	XSRETURN_EMPTY;
}

/* NeoStats::Internal::hook_event(event, flags, callback, userdata) */
static
XS (XS_NeoStats_hook_event)
{

	ModuleEvent *evt;

	dXSARGS;
	if (items != 4) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::hook_event(event, flags, callback, userdata)");
	} else {
		evt = ns_calloc(sizeof(ModuleEvent));
		evt->pe = ns_calloc(sizeof(PerlEvent));
		evt->event = (int) SvIV (ST (0));
		evt->flags = (int) SvIV (ST (1));
		/* null, because its a perl event, which will execute via dedicated perl event handler */
		evt->handler = NULL; 
		evt->pe->callback = sv_mortalcopy(ST (2));
		SvREFCNT_inc (evt->pe->callback);
		evt->pe->userdata = sv_mortalcopy(ST (3));
		SvREFCNT_inc (evt->pe->userdata);
		/* add it as a event */
		AddEvent(evt);

		XSRETURN_UV (PTR2UV (evt));
	}
}

static
XS (XS_NeoStats_unhook_event)
{
	Module *mod;
	Event evt = -1;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items != 1) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:unhook_event(hook)");
	} else {
		evt = (int) SvIV (ST(0));
		if (mod->event_list && mod->event_list[evt]->pe) {
			SvREFCNT_dec(mod->event_list[evt]->pe->callback);
			SvREFCNT_dec(mod->event_list[evt]->pe->userdata);
			ns_free(mod->event_list[evt]->pe);
			ns_free(mod->event_list[evt]);
		}
		DeleteEvent(evt);
	}
	XSRETURN_EMPTY;
}


static
XS (XS_NeoStats_AddBot)
{
	Module *mod;
	HV * rethash;
	SV *ret;
	int flags;
	SV *value;
	BotInfo *bi;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:AddBot(botinfo, flags, data)");
	} else {
		ret = ST(0);
		flags = (int) SvIV (ST (1));
		if(SvTYPE(SvRV(ret))!=SVt_PVHV) {
			 dlog(DEBUG1, "XS_NeoStats_AddBot: unsuported input %lu, must %i", SvTYPE(SvRV(ret)), SVt_PVHV);
			 XSRETURN_EMPTY;
		}
		rethash = (HV*)SvRV(ret);
		dump_hash(rethash);
		bi = ns_malloc(sizeof(BotInfo));
		value = *hv_fetch(rethash, "nick", strlen("nick"), FALSE);
		strncpy(bi->nick, SvPV_nolen(value), MAXNICK);
		value = *hv_fetch(rethash, "altnick", strlen("altnick"), FALSE);
		strncpy(bi->altnick, SvPV_nolen(value), MAXNICK);
		value = *hv_fetch(rethash, "ident", strlen("ident"), FALSE);
		strncpy(bi->user, SvPV_nolen(value), MAXUSER);
		value = *hv_fetch(rethash, "host", strlen("host"), FALSE);
		strncpy(bi->host, SvPV_nolen(value), MAXHOST);
		value = *hv_fetch(rethash, "gecos", strlen("gecos"), FALSE);
		strncpy(bi->realname, SvPV_nolen(value), MAXREALNAME);
		bi->flags = (int) SvIV (ST (1));
		bi->bot_cmd_list = NULL;
		bi->bot_setting_list = NULL;
		if ((bot = AddBot(bi)) == NULL) {
			free(bi);
			XSRETURN_EMPTY;
		}
		bot->botinfo = bi;
		XSRETURN_UV (PTR2UV (bot));
	}
	XSRETURN_EMPTY;
}




static
XS (XS_NeoStats_DelBot)
{
	Module *mod;
	dXSARGS;
	Bot *bot;


	if (items != 2) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::DelBot(botname, quitreason)");
	} else {
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		bot = FindBot(SvPV_nolen(ST(0)));
		ns_free(bot->botinfo);
		irc_quit(bot, SvPV_nolen(ST(1)));
		XSRETURN_UV( NS_SUCCESS);
	}
}


static
XS (XS_NeoStats_FindUser)
{
	Module *mod;
	Client *u;
	SV *client;

	dXSARGS;

	if (items != 1) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::FindUser(nick)");
	} else {
		SP -= items; /* remove args from the stack */
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		u = FindUser(SvPV_nolen(ST(0)));
		if (!u) {
			XSRETURN_EMPTY;
		}
		/* create a hash with the users details filled in */
		client = (SV *)perl_encode_client(u);
		sv_2mortal(client);
		XPUSHs(newRV_noinc((SV *)client));
		PUTBACK;
		XSRETURN(1);
	}
}

static
XS (XS_NeoStats_FindServer)
{
	Module *mod;
	Client *u;
	SV *client;

	dXSARGS;

	if (items != 1) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::FindServer(name)");
	} else {
		SP -= items; /* remove args from the stack */
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		u = FindServer(SvPV_nolen(ST(0)));
		if (!u) {
			XSRETURN_EMPTY;
		}
		/* create a hash with the users details filled in */
		client = (SV *)perl_encode_server(u);
		sv_2mortal(client);
		XPUSHs(newRV_noinc((SV *)client));
		PUTBACK;
		XSRETURN(1);
	}
}


static
XS (XS_NeoStats_FindChannel)
{
	Module *mod;
	Channel *c;
	SV *client;

	dXSARGS;

	if (items != 1) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::FindServer(name)");
	} else {
		SP -= items; /* remove args from the stack */
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		c = FindChannel(SvPV_nolen(ST(0)));
		if (!c) {
			XSRETURN_EMPTY;
		}
		/* create a hash with the users details filled in */
		client = (SV *)perl_encode_channel(c);
		sv_2mortal(client);
		XPUSHs(newRV_noinc((SV *)client));
		PUTBACK;
		XSRETURN(1);
	}
}

#if 0

static
XS (XS_Xchat_get_list)
{
	SV *name;
	HV *hash;
	xchat_list *list;
	const char *const *fields;
	const char *field;
	int i = 0;						  /* field index */
	int count = 0;					  /* return value for scalar context */
	U32 context;
	dXSARGS;

	if (items != 1) {
		xchat_print (ph, "Usage: NeoStats::get_list(name)");
	} else {
		SP -= items;				  /*remove the argument list from the stack */

		name = ST (0);

		list = xchat_list_get (ph, SvPV_nolen (name));

		if (list == NULL) {
			XSRETURN_EMPTY;
		}

		context = GIMME_V;

		if (context == G_SCALAR) {
			while (xchat_list_next (ph, list)) {
				count++;
			}
			xchat_list_free (ph, list);
			XSRETURN_IV ((IV) count);
		}

		fields = xchat_list_fields (ph, SvPV_nolen (name));
		while (xchat_list_next (ph, list)) {
			i = 0;
			hash = newHV ();
			sv_2mortal ((SV *) hash);
			while (fields[i] != NULL) {
				switch (fields[i][0]) {
				case 's':
					field = xchat_list_str (ph, list, fields[i] + 1);
					if (field != NULL) {
						hv_store (hash, fields[i] + 1, strlen (fields[i] + 1),
									 newSVpvn (field, strlen (field)), 0);
						/*                                              xchat_printf (ph, */
						/*                                                      "string: %s - %d - %s",  */
						/*                                                      fields[i]+1, */
						/*                                                      strlen(fields[i]+1), */
						/*                                                      field, strlen(field) */
						/*                                                              ); */
					} else {
						hv_store (hash, fields[i] + 1, strlen (fields[i] + 1),
									 &PL_sv_undef, 0);
						/*                                              xchat_printf (ph, */
						/*                                                      "string: %s - %d - undef",       */
						/*                                                              fields[i]+1, */
						/*                                                              strlen(fields[i]+1) */
						/*                                                              ); */
					}
					break;
				case 'p':
					/*                                       xchat_printf (ph, "pointer: %s", fields[i]+1); */
					hv_store (hash, fields[i] + 1, strlen (fields[i] + 1),
								 newSVuv (PTR2UV (xchat_list_str (ph, list,
																			 fields[i] + 1)
											 )), 0);
					break;
				case 'i':
					hv_store (hash, fields[i] + 1, strlen (fields[i] + 1),
								 newSVuv (xchat_list_int (ph, list, fields[i] + 1)),
								 0);
					/*                                       xchat_printf (ph, "int: %s - %d",fields[i]+1, */
					/*                                                        xchat_list_int (ph, list, fields[i]+1) */
					/*                                                       ); */
					break;
				case 't':
					hv_store (hash, fields[i] + 1, strlen (fields[i] + 1),
								 newSVnv (xchat_list_time (ph, list, fields[i] + 1)),
								 0);
					break;
				}
				i++;
			}

			XPUSHs (newRV_noinc ((SV *) hash));

		}
		xchat_list_free (ph, list);

		PUTBACK;
		return;
	}
}

#endif

/* xs_init is the second argument perl_parse. As the name hints, it
   initializes XS subroutines (see the perlembed manpage) */
static void
xs_init (pTHX)
{
	HV *stash;
	/* This one allows dynamic loading of perl modules in perl
	   scripts by the 'use perlmod;' construction */
	newXS ("DynaLoader::boot_DynaLoader", boot_DynaLoader, __FILE__);
	/* load up all the custom IRC perl functions */
	newXS ("NeoStats::Internal::debug", XS_NeoStats_debug, __FILE__);
	newXS ("NeoStats::Internal::register", XS_NeoStats_register, __FILE__);
	newXS ("NeoStats::Internal::hook_event", XS_NeoStats_hook_event, __FILE__);
	newXS ("NeoStats::Internal::unhook_event", XS_NeoStats_unhook_event, __FILE__);
	newXS ("NeoStats::Internal::AddBot", XS_NeoStats_AddBot, __FILE__);
	newXS ("NeoStats::Internal::DelBot", XS_NeoStats_DelBot, __FILE__);
	newXS ("NeoStats::Internal::FindUser", XS_NeoStats_FindUser, __FILE__);
	newXS ("NeoStats::Internal::FindServer", XS_NeoStats_FindServer, __FILE__);
	newXS ("NeoStats::Internal::FindChannel", XS_NeoStats_FindChannel, __FILE__);

	stash = get_hv ("NeoStats::", TRUE);
	if (stash == NULL) {
		exit (1);
	}
	newCONSTSUB (stash, "EVENT_NULL", newSViv (EVENT_NULL));
	newCONSTSUB (stash, "EVENT_MODULELOAD", newSViv (EVENT_MODULELOAD));
	newCONSTSUB (stash, "EVENT_MODULEUNLOAD", newSViv (EVENT_MODULEUNLOAD));
	newCONSTSUB (stash, "EVENT_SERVER", newSViv (EVENT_SERVER));
	newCONSTSUB (stash, "EVENT_SQUIT", newSViv (EVENT_SQUIT));
	newCONSTSUB (stash, "EVENT_PING", newSViv (EVENT_PING));
	newCONSTSUB (stash, "EVENT_PONG", newSViv (EVENT_PONG));
	newCONSTSUB (stash, "EVENT_SIGNON", newSViv (EVENT_SIGNON));
	newCONSTSUB (stash, "EVENT_QUIT", newSViv (EVENT_QUIT));
	newCONSTSUB (stash, "EVENT_NICKIP", newSViv (EVENT_NICKIP));
	newCONSTSUB (stash, "EVENT_KILL", newSViv (EVENT_KILL));
	newCONSTSUB (stash, "EVENT_GLOBALKILL", newSViv (EVENT_GLOBALKILL));
	newCONSTSUB (stash, "EVENT_LOCALKILL", newSViv (EVENT_LOCALKILL));
	newCONSTSUB (stash, "EVENT_SERVERKILL", newSViv (EVENT_SERVERKILL));
	newCONSTSUB (stash, "EVENT_BOTKILL", newSViv (EVENT_BOTKILL));
	newCONSTSUB (stash, "EVENT_NICK", newSViv (EVENT_NICK));
	newCONSTSUB (stash, "EVENT_AWAY", newSViv (EVENT_AWAY));
	newCONSTSUB (stash, "EVENT_UMODE", newSViv (EVENT_UMODE));
	newCONSTSUB (stash, "EVENT_SMODE", newSViv (EVENT_SMODE));
	newCONSTSUB (stash, "EVENT_NEWCHAN", newSViv (EVENT_NEWCHAN));
	newCONSTSUB (stash, "EVENT_DELCHAN", newSViv (EVENT_DELCHAN));
	newCONSTSUB (stash, "EVENT_JOIN", newSViv (EVENT_JOIN));
	newCONSTSUB (stash, "EVENT_PART", newSViv (EVENT_PART));
	newCONSTSUB (stash, "EVENT_PARTBOT", newSViv (EVENT_PARTBOT));
	newCONSTSUB (stash, "EVENT_EMPTYCHAN", newSViv (EVENT_EMPTYCHAN));
	newCONSTSUB (stash, "EVENT_KICK", newSViv (EVENT_KICK));
	newCONSTSUB (stash, "EVENT_KICKBOT", newSViv (EVENT_KICKBOT));
	newCONSTSUB (stash, "EVENT_TOPIC", newSViv (EVENT_TOPIC));
	newCONSTSUB (stash, "EVENT_CMODE", newSViv (EVENT_CMODE));
	newCONSTSUB (stash, "EVENT_PRIVATE", newSViv (EVENT_PRIVATE));
	newCONSTSUB (stash, "EVENT_NOTICE", newSViv (EVENT_NOTICE));
	newCONSTSUB (stash, "EVENT_CPRIVATE", newSViv (EVENT_CPRIVATE));
	newCONSTSUB (stash, "EVENT_CNOTICE", newSViv (EVENT_CNOTICE));
	newCONSTSUB (stash, "EVENT_GLOBOPS", newSViv (EVENT_GLOBOPS));
	newCONSTSUB (stash, "EVENT_CHATOPS", newSViv (EVENT_CHATOPS));
	newCONSTSUB (stash, "EVENT_WALLOPS", newSViv (EVENT_WALLOPS));
	newCONSTSUB (stash, "EVENT_CTCPVERSIONRPL", newSViv (EVENT_CTCPVERSIONRPL));
	newCONSTSUB (stash, "EVENT_CTCPVERSIONREQ", newSViv (EVENT_CTCPVERSIONREQ));
	newCONSTSUB (stash, "EVENT_CTCPFINGERRPL", newSViv (EVENT_CTCPFINGERRPL));
	newCONSTSUB (stash, "EVENT_CTCPFINGERREQ", newSViv (EVENT_CTCPFINGERREQ));
	newCONSTSUB (stash, "EVENT_CTCPACTIONREQ", newSViv (EVENT_CTCPACTIONREQ));
	newCONSTSUB (stash, "EVENT_CTCPTIMERPL", newSViv (EVENT_CTCPTIMERPL));
	newCONSTSUB (stash, "EVENT_CTCPTIMEREQ", newSViv (EVENT_CTCPTIMEREQ));
	newCONSTSUB (stash, "EVENT_CTCPPINGRPL", newSViv (EVENT_CTCPPINGRPL));
	newCONSTSUB (stash, "EVENT_CTCPPINGREQ", newSViv (EVENT_CTCPPINGREQ));
	newCONSTSUB (stash, "EVENT_DCCSEND", newSViv (EVENT_DCCSEND));
	newCONSTSUB (stash, "EVENT_DCCCHAT", newSViv (EVENT_DCCCHAT));
	newCONSTSUB (stash, "EVENT_DCCCHATMSG", newSViv (EVENT_DCCCHATMSG));
	newCONSTSUB (stash, "EVENT_ADDBAN", newSViv (EVENT_ADDBAN));
	newCONSTSUB (stash, "EVENT_DELBAN", newSViv (EVENT_DELBAN));
	
	newCONSTSUB (stash, "EVENT_FLAG_DISABLED", newSViv (EVENT_FLAG_DISABLED));
	newCONSTSUB (stash, "EVENT_FLAG_IGNORE_SYNCH", newSViv (EVENT_FLAG_IGNORE_SYNCH));
	newCONSTSUB (stash, "EVENT_FLAG_USE_EXCLUDE", newSViv (EVENT_FLAG_USE_EXCLUDE));
	newCONSTSUB (stash, "EVENT_FLAG_EXCLUDE_ME", newSViv (EVENT_FLAG_EXCLUDE_ME));
	newCONSTSUB (stash, "EVENT_FLAG_EXCLUDE_MODME", newSViv (EVENT_FLAG_EXCLUDE_MODME));


	newCONSTSUB (stash, "BOT_FLAG_ONLY_OPERS", newSViv (BOT_FLAG_ONLY_OPERS));
	newCONSTSUB (stash, "BOT_FLAG_RESTRICT_OPERS", newSViv (BOT_FLAG_RESTRICT_OPERS));
	newCONSTSUB (stash, "BOT_FLAG_DEAF", newSViv (BOT_FLAG_DEAF));
	newCONSTSUB (stash, "BOT_FLAG_SERVICEBOT", newSViv (BOT_FLAG_SERVICEBOT));
	newCONSTSUB (stash, "BOT_FLAG_PERSIST", newSViv (BOT_FLAG_PERSIST));
	newCONSTSUB (stash, "BOT_FLAG_NOINTRINSICLEVELS", newSViv (BOT_FLAG_NOINTRINSICLEVELS));
	newCONSTSUB (stash, "BOT_COMMON_HOST", newSVpv (BOT_COMMON_HOST, strlen(BOT_COMMON_HOST)));



	newCONSTSUB (stash, "NS_SUCCESS", newSViv (NS_SUCCESS));
	newCONSTSUB (stash, "NS_FAILURE", newSViv (NS_FAILURE));

}

int
Init_Perl (void)
{

	return NS_SUCCESS;
}


Module *load_perlmodule (const char *filename, Client *u)
{
	char *perl_args[] = { "", "-e", "0", "-w" };
	CmdParams *cmd;
	const char perl_definitions[] = {
#include "neostats.pm.h" 
	};
	Module *mod;
	
	

#ifdef WIN32
	static HINSTANCE lib = NULL;

	if (!lib) {
		lib = LoadLibrary (PERL_DLL);
		if (!lib) {
			thread_mbox ("Cannot open " PERL_DLL "\n\n"
							 "You must have ActivePerl installed in order to\n"
							 "run perl scripts.\n\n"
							 "http://www.activestate.com/ActivePerl/\n\n"
							 "Make sure perl's bin directory is in your PATH.");
			return NULL;
		}
		FreeLibrary (lib);
	}
#endif

	mod = ns_calloc(sizeof(Module));
	mod->pm = ns_calloc(sizeof(PerlModInfo));
	mod->info = ns_calloc(sizeof(ModuleInfo));
	mod->modtype = MOD_PERL;
	strlcpy(mod->pm->filename, filename, MAXPATH);
	/*XXX  this is a temp solution till we get fully loaded. Its Bad */
	mod->info->name = ns_malloc(strlen("NeoStats")+1);
	ircsnprintf((char *)mod->info->name, strlen("NeoStats")+1, "NeoStats");

	PL_perl_destruct_level = 1;
	mod->pm->my_perl = perl_alloc ();
	PL_perl_destruct_level = 1;
	PERL_SET_CONTEXT((PMI *)mod->pm->my_perl);
	perl_construct (mod->pm->my_perl);
	PL_perl_destruct_level = 1;
	perl_parse (mod->pm->my_perl, xs_init, 4, perl_args, NULL);
	/*
	   Now initialising the perl interpreter by loading the
	   perl_definition array.
	 */
	eval_pv (perl_definitions, TRUE);
	mod->insynch = 0;
	if (!execute_perl (mod, sv_2mortal (newSVpv ("NeoStats::Embed::load", 0)),
								1, (char *)filename)) {
		/* XXX if we are here, check that pm->mod->info has something, otherwise the script didnt register */
		if (!mod->info->name[0]) {
			load_module_error(u, __("Perl Module %s didn't register. Unloading", u), filename);
			unload_perlmod(mod);
			free(mod);
			return NULL;
		}		
		/* it loaded ok */
	} else {
		load_module_error(u, __("Errors in Perl Module %s", u), filename);
		unload_perlmod(mod);
		free(mod);
		return NULL;	
	}
	assign_mod_number(mod);
	SET_RUN_LEVEL(mod);
	DBAOpenDatabase();
	RESET_RUN_LEVEL();
	insert_module(mod);

	cmd = ns_calloc (sizeof(CmdParams));
	cmd->param = (char*)mod->info->name;
	SendAllModuleEvent(EVENT_MODULELOAD, cmd);
	ns_free(cmd);

	nlog(LOG_NORMAL, "Loaded Perl Module %s (%s)", mod->info->name, mod->info->version);
         if (u) {
	         irc_prefmsg (ns_botptr, u, __("Perl Module %s loaded, %s",u), mod->info->name, mod->info->description);
                  irc_globops (NULL, _("Perl Module %s loaded"), mod->info->name);
         }
	return mod;
}

void PerlModFini(Module *mod) {
		SET_RUN_LEVEL(mod);
		if (mod->synched == 1) {
			/* only execute unload if synced */
			execute_perl (mod, sv_2mortal (newSVpv ("NeoStats::Embed::unload", 0)), 1, mod->pm->filename);
		}
		RESET_RUN_LEVEL();
}

void unload_perlmod(Module *mod) {
		PERL_SET_CONTEXT((PMI *)mod->pm->my_perl);
		/* because segv handler doesn't handle perl well yet */
		RESET_RUN_LEVEL()
		PL_perl_destruct_level = 1;
		perl_destruct ((PMI *)mod->pm->my_perl);

		perl_free ((PMI *)mod->pm->my_perl);

		free((void *)mod->info->name);

		free((void *)mod->info->description);

		free((void *)mod->info->version);

		free(mod->info);
		
		free(mod->pm);


}
