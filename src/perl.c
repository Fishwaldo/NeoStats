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
		nlog(LOG_WARNING, "Perl error: %s\n", SvPV(sv, count)); 
		POPs;							  /* remove undef from the top of the stack */
	} else if (count != 1) {
		nlog(LOG_WARNING, "Perl error: expected 1 value from %s, "
						  "got: %d\n", (char *)function, count);
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
			ret = execute_perl(mod_ptr, mod_ptr->event_list[evt]->pe->callback, 3, cmdparams->channel->name, cmdparams->source->name, cmdparams->param);
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


#if 0

static int
generic_cb (int fd, int flags, void *userdata)
{
	HookData *data = (HookData *) userdata;

	dSP;
	ENTER;
	SAVETMPS;

	PUSHMARK (SP);
	XPUSHs (data->userdata);
	PUTBACK;

	call_sv (data->callback, G_EVAL);
	SPAGAIN;
	if (SvTRUE (ERRSV)) {
		xchat_printf (ph, "Error in generic callback %s", SvPV_nolen (ERRSV));
		POPs;							  /* remove undef from the top of the stack */
	}

	PUTBACK;
	FREETMPS;
	LEAVE;

	return XCHAT_EAT_ALL;
}

static int
timer_cb (void *userdata)
{
	HookData *data = (HookData *) userdata;
	int retVal = 0;
	int count = 0;

	dSP;
	ENTER;
	SAVETMPS;

	PUSHMARK (SP);
	XPUSHs (data->userdata);
	PUTBACK;

	count = call_sv (data->callback, G_EVAL);
	SPAGAIN;

	if (SvTRUE (ERRSV)) {
		xchat_printf (ph, "Error in timer callback %s", SvPV_nolen (ERRSV));
		POPs;							  /* remove undef from the top of the stack */
		retVal = XCHAT_EAT_ALL;
	} else {
		if (count != 1) {
			xchat_print (ph, "Timer handler should only return 1 value.");
			retVal = XCHAT_EAT_NONE;
		} else {
			retVal = POPi;
			if (retVal == 0) {
				/* if 0 is return the timer is going to get unhooked */
				PUSHMARK (SP);
				XPUSHs (sv_2mortal (newSVuv (PTR2UV (data->hook))));
				PUTBACK;

				call_pv ("NeoStats::unhook", G_EVAL);
				SPAGAIN;

				SvREFCNT_dec (data->callback);

				if (data->userdata) {
					SvREFCNT_dec (data->userdata);
				}
				free (data);
			}
		}

	}

	PUTBACK;
	FREETMPS;
	LEAVE;

	return retVal;
}

static int
server_cb (char *word[], char *word_eol[], void *userdata)
{
	HookData *data = (HookData *) userdata;
	int retVal = 0;
	int count = 0;

	/* these must be initialized after SAVETMPS */
	AV *wd = NULL;
	AV *wd_eol = NULL;

	dSP;
	ENTER;
	SAVETMPS;

	wd = newAV ();
	sv_2mortal ((SV *) wd);
	wd_eol = newAV ();
	sv_2mortal ((SV *) wd_eol);

	for (count = 1;
		  (count < 32) && (word[count] != NULL) && (word[count][0] != 0);
		  count++) {
		av_push (wd, newSVpv (word[count], 0));
	}

	for (count = 1; (count < 32) && (word_eol[count] != NULL)
		  && (word_eol[count][0] != 0); count++) {
		av_push (wd_eol, newSVpv (word_eol[count], 0));
	}

	/*               xchat_printf (ph, */
	/*                               "Recieved %d words in server callback", av_len (wd)); */
	PUSHMARK (SP);
	XPUSHs (newRV_noinc ((SV *) wd));
	XPUSHs (newRV_noinc ((SV *) wd_eol));
	XPUSHs (data->userdata);
	PUTBACK;

	count = call_sv (data->callback, G_EVAL);
	SPAGAIN;
	if (SvTRUE (ERRSV)) {
		xchat_printf (ph, "Error in server callback %s", SvPV_nolen (ERRSV));
		POPs;							  /* remove undef from the top of the stack */
		retVal = XCHAT_EAT_NONE;
	} else {
		if (count != 1) {
			xchat_print (ph, "Server handler should only return 1 value.");
			retVal = XCHAT_EAT_NONE;
		} else {
			retVal = POPi;
		}

	}

	PUTBACK;
	FREETMPS;
	LEAVE;

	return retVal;
}

static int
command_cb (char *word[], char *word_eol[], void *userdata)
{
	HookData *data = (HookData *) userdata;
	int retVal = 0;
	int count = 0;

	/* these must be initialized after SAVETMPS */
	AV *wd = NULL;
	AV *wd_eol = NULL;

	dSP;
	ENTER;
	SAVETMPS;

	wd = newAV ();
	sv_2mortal ((SV *) wd);
	wd_eol = newAV ();
	sv_2mortal ((SV *) wd_eol);

	for (count = 1;
		  (count < 32) && (word[count] != NULL) && (word[count][0] != 0);
		  count++) {
		av_push (wd, newSVpv (word[count], 0));
	}

	for (count = 1;
		  (count < 32) && (word_eol[count] != NULL) &&
		  (word_eol[count][0] != 0); count++) {
		av_push (wd_eol, newSVpv (word_eol[count], 0));
	}

	/*               xchat_printf (ph, "Recieved %d words in command callback", */
	/*                               av_len (wd)); */
	PUSHMARK (SP);
	XPUSHs (newRV_noinc ((SV *) wd));
	XPUSHs (newRV_noinc ((SV *) wd_eol));
	XPUSHs (data->userdata);
	PUTBACK;

	count = call_sv (data->callback, G_EVAL);
	SPAGAIN;
	if (SvTRUE (ERRSV)) {
		xchat_printf (ph, "Error in command callback %s", SvPV_nolen (ERRSV));
		POPs;							  /* remove undef from the top of the stack */
		retVal = XCHAT_EAT_NONE;
	} else {
		if (count != 1) {
			xchat_print (ph, "Command handler should only return 1 value.");
			retVal = XCHAT_EAT_NONE;
		} else {
			retVal = POPi;
		}

	}

	PUTBACK;
	FREETMPS;
	LEAVE;

	return retVal;
}

/* custom IRC perl functions for scripting */

/* NeoStats::Internal::register (scriptname, version, desc)
 *
 */
#endif

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













#if 0
static
XS (XS_Xchat_emit_print)
{
	char *event_name;
	int RETVAL;
	int count;

	dXSARGS;
	if (items < 1) {
		xchat_print (ph, "Usage: NeoStats::emit_print(event_name, ...)");
	} else {
		event_name = (char *) SvPV_nolen (ST (0));
		RETVAL = 0;

		/* we need to figure out the number of defined values passed in */
		for (count = 0; count < items; count++) {
			if (!SvOK (ST (count))) {
				break;
			}
		}

		switch (count) {
		case 1:
			RETVAL = xchat_emit_print (ph, event_name, NULL);
			break;
		case 2:
			RETVAL = xchat_emit_print (ph, event_name,
												SvPV_nolen (ST (1)), NULL);
			break;
		case 3:
			RETVAL = xchat_emit_print (ph, event_name,
												SvPV_nolen (ST (1)),
												SvPV_nolen (ST (2)), NULL);
			break;
		case 4:
			RETVAL = xchat_emit_print (ph, event_name,
												SvPV_nolen (ST (1)),
												SvPV_nolen (ST (2)),
												SvPV_nolen (ST (3)), NULL);
			break;
		case 5:
			RETVAL = xchat_emit_print (ph, event_name,
												SvPV_nolen (ST (1)),
												SvPV_nolen (ST (2)),
												SvPV_nolen (ST (3)),
												SvPV_nolen (ST (4)), NULL);
			break;

		}

		XSRETURN_UV (RETVAL);
	}
}
/* NeoStats::Internal::hook_event(name, callback, userdata) */
static
XS (XS_NeoStats_hook_event)
{

	char *name;
	int pri;
	SV *callback;
	SV *userdata;
	xchat_hook *hook;
	HookData *data;

	dXSARGS;

	if (items != 3) {
		xchat_print (ph,
						 "Usage: NeoStats::Internal::hook_event(name, callback, userdata)");
	} else {
		name = SvPV_nolen (ST (0));
		callback = ST (1);
		userdata = ST (2);
		data = NULL;
		data = malloc (sizeof (HookData));
		if (data == NULL) {
			XSRETURN_UNDEF;
		}

		data->callback = sv_mortalcopy (callback);
		SvREFCNT_inc (data->callback);
		data->userdata = sv_mortalcopy (userdata);
		SvREFCNT_inc (data->userdata);
		hook = xchat_hook_server (ph, name, pri, server_cb, data);

		XSRETURN_UV (PTR2UV (hook));
	}
}

/* NeoStats::Internal::hook_command(name, callback, help_text, userdata) */
static
XS (XS_Xchat_hook_command)
{
	char *name;
	int pri;
	SV *callback;
	char *help_text;
	SV *userdata;
	xchat_hook *hook;
	HookData *data;

	dXSARGS;

	if (items != 4) {
		xchat_print (ph,
						 "Usage: NeoStats::Internal::hook_command(name, callback, help_text, userdata)");
	} else {
		name = SvPV_nolen (ST (0));
		callback = ST (1);
		help_text = SvPV_nolen (ST (2));
		userdata = ST (3);
		data = NULL;

		data = malloc (sizeof (HookData));
		if (data == NULL) {
			XSRETURN_UNDEF;
		}

		data->callback = sv_mortalcopy (callback);
		SvREFCNT_inc (data->callback);
		data->userdata = sv_mortalcopy (userdata);
		SvREFCNT_inc (data->userdata);
		hook = xchat_hook_command (ph, name, pri, command_cb, help_text, data);

		XSRETURN_UV (PTR2UV (hook));
	}

}

/* NeoStats::Internal::hook_timer(timeout, callback, userdata) */
static
XS (XS_Xchat_hook_timer)
{
	int timeout;
	SV *callback;
	SV *userdata;
	xchat_hook *hook;
	HookData *data;

	dXSARGS;

	if (items != 3) {
		xchat_print (ph,
						 "Usage: NeoStats::Internal::hook_timer(timeout, callback, userdata)");
	} else {
		timeout = (int) SvIV (ST (0));
		callback = ST (1);
		data = NULL;
		userdata = ST (2);

		data = malloc (sizeof (HookData));
		if (data == NULL) {
			XSRETURN_UNDEF;
		}

		data->callback = sv_mortalcopy (callback);
		SvREFCNT_inc (data->callback);
		data->userdata = sv_mortalcopy (userdata);
		SvREFCNT_inc (data->userdata);
		hook = xchat_hook_timer (ph, timeout, timer_cb, data);
		data->hook = hook;

		XSRETURN_UV (PTR2UV (hook));
	}
}

static
XS (XS_Xchat_unhook)
{
	xchat_hook *hook;
	HookData *userdata;
	int retCount = 0;
	dXSARGS;
	if (items != 1) {
		xchat_print (ph, "Usage: NeoStats::unhook(hook)");
	} else {
		hook = INT2PTR (xchat_hook *, SvUV (ST (0)));
		userdata = (HookData *) xchat_unhook (ph, hook);

		if (userdata != NULL) {
			if (userdata->callback) {
				SvREFCNT_dec (userdata->callback);
			}

			if (userdata->userdata) {
				XPUSHs (sv_mortalcopy (userdata->userdata));
				SvREFCNT_dec (userdata->userdata);
				retCount = 1;
			}
		}
		free (userdata);
		XSRETURN (retCount);
	}
	XSRETURN_EMPTY;
}

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
/* #include "neostats.pm.h" */
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

	PL_perl_destruct_level = 2;
	mod->pm->my_perl = perl_alloc ();
	PL_perl_destruct_level = 2;
	PERL_SET_CONTEXT((PMI *)mod->pm->my_perl);
	perl_construct (mod->pm->my_perl);
	PL_perl_destruct_level = 2;
	perl_parse (mod->pm->my_perl, xs_init, 4, perl_args, NULL);
	/*
	   Now initialising the perl interpreter by loading the
	   perl_definition array.
	 */
	eval_pv (perl_definitions, TRUE);
	mod->insynch = 1;
	if (!execute_perl (mod, sv_2mortal (newSVpv ("NeoStats::Embed::load", 0)),
								1, (char *)filename)) {
#if 0
	if (!execute_perl (mod, sv_2mortal (newSVpv ("pkg_load", 0)),
								3, (char *)filename, (char *)filename, (char *)filename)) {
#endif
		/* XXX if we are here, check that pm->mod->info has something, otherwise the script didnt register */
		if (!mod->info->name[0]) {
			load_module_error(u, __("Perl Module %s didn't register. Unloading", u), filename);
			unload_perlmod(mod);
			free(mod);
			return NULL;
		}		
		/* it loaded ok */
		mod->synched = 1;
	} else {
		load_module_error(u, __("Errors in Perl Module %s", u), filename);
		unload_perlmod(mod);
		free(mod);
		return NULL;	
	}
	assign_mod_number(mod);

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
		PL_perl_destruct_level = 2;
		perl_destruct ((PMI *)mod->pm->my_perl);

		perl_free ((PMI *)mod->pm->my_perl);

		free((void *)mod->info->name);

		free((void *)mod->info->description);

		free((void *)mod->info->version);

		free(mod->info);
		
		free(mod->pm);


}
