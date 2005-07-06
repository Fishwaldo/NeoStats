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
#undef _
#include <sys/types.h>
#include <dirent.h>

static int perl_load_file (char *script_name);





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

/* leave this before XSUB.h, to avoid readdir() being redefined */
static void
perl_auto_load (void)
{
	DIR *dir;
	struct dirent *ent;

	dir = opendir ("modules");
	if (dir) {
		while ((ent = readdir (dir))) {
			int len = strlen (ent->d_name);
			if (len > 3 && strcasecmp (".pl", ent->d_name + len - 3) == 0) {
				char *file = malloc (len + strlen ("modules") + 2);
				sprintf (file, "modules/%s", ent->d_name);
				dlog(DEBUG2, "Loading Perl Module %s", file);
				perl_load_file (file);
				free (file);
			}
		}
		closedir (dir);
	}
}

#include <EXTERN.h>
#define WIN32IOP_H
#include <perl.h>
#include <XSUB.h>



list_t *perlmods;

typedef struct {
	char filename[MAXPATH];
	ModuleInfo *modinfo;
	PerlInterpreter *my_perl;
} PerlModInfo;

extern void boot_DynaLoader (pTHX_ CV * cv);


/*
  this is used for autoload and shutdown callbacks
*/
static int
execute_perl (PerlModInfo *pm, SV * function, char *args)
{

	int count, ret_value = 1;
	SV *sv;

	dSP;
	ENTER;
	SAVETMPS;
	PERL_SET_CONTEXT(pm->my_perl);

	PUSHMARK (SP);
	XPUSHs (sv_2mortal (newSVpv (args, 0)));
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

	return ret_value;
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

/* NeoStats::Internal::register (scriptname, version, desc, shutdowncallback, filename)
 *
 */

static
XS (XS_Xchat_register)
{
	char *name, *version, *desc, *filename;
	void *gui_entry;
	dXSARGS;
	if (items != 4) {
		xchat_printf (ph,
						  "Usage: NeoStats::Internal::register(scriptname, version, desc, filename)");
	} else {
		name = SvPV_nolen (ST (0));
		version = SvPV_nolen (ST (1));
		desc = SvPV_nolen (ST (2));
		filename = SvPV_nolen (ST (3));

		gui_entry = xchat_plugingui_add (ph, filename, name,
													desc, version, NULL);

		XSRETURN_UV (PTR2UV (gui_entry));

	}
}

#endif
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
XS (XS_Xchat_hook_event)
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
#if 0
	HV *stash;
#endif
	/* This one allows dynamic loading of perl modules in perl
	   scripts by the 'use perlmod;' construction */
	newXS ("DynaLoader::boot_DynaLoader", boot_DynaLoader, __FILE__);
	/* load up all the custom IRC perl functions */
	newXS ("NeoStats::Internal::debug", XS_NeoStats_debug, __FILE__);
#if 0
	stash = get_hv ("NeoStats::", TRUE);
	if (stash == NULL) {
		exit (1);
	}

	newCONSTSUB (stash, "PRI_HIGHEST", newSViv (XCHAT_PRI_HIGHEST));
	newCONSTSUB (stash, "PRI_HIGH", newSViv (XCHAT_PRI_HIGH));
	newCONSTSUB (stash, "PRI_NORM", newSViv (XCHAT_PRI_NORM));
	newCONSTSUB (stash, "PRI_LOW", newSViv (XCHAT_PRI_LOW));
	newCONSTSUB (stash, "PRI_LOWEST", newSViv (XCHAT_PRI_LOWEST));

	newCONSTSUB (stash, "EAT_NONE", newSViv (XCHAT_EAT_NONE));
	newCONSTSUB (stash, "EAT_XCHAT", newSViv (XCHAT_EAT_XCHAT));
	newCONSTSUB (stash, "EAT_PLUGIN", newSViv (XCHAT_EAT_PLUGIN));
	newCONSTSUB (stash, "EAT_ALL", newSViv (XCHAT_EAT_ALL));
	newCONSTSUB (stash, "FD_READ", newSViv (XCHAT_FD_READ));
	newCONSTSUB (stash, "FD_WRITE", newSViv (XCHAT_FD_WRITE));
	newCONSTSUB (stash, "FD_EXCEPTION", newSViv (XCHAT_FD_EXCEPTION));
	newCONSTSUB (stash, "FD_NOTSOCKET", newSViv (XCHAT_FD_NOTSOCKET));
	newCONSTSUB (stash, "KEEP", newSViv (1));
	newCONSTSUB (stash, "REMOVE", newSViv (0));
#endif
}

int
Init_Perl (void)
{
	/* create the perl modules list */
	perlmods = list_create(-1);

	/* now load the modules */
	perl_auto_load();
	return NS_SUCCESS;
}


static int
perl_load_file (char *filename)
{
	char *perl_args[] = { "", "-e", "0", "-w" };
	const char perl_definitions[] = {
#include "neostats.pm.h"
	};
	PerlModInfo *pm;
	lnode_t *node;
	
	

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
			return FALSE;
		}
		FreeLibrary (lib);
	}
#endif

	pm = os_malloc(sizeof(PerlModInfo));
	pm->modinfo = os_malloc(sizeof(ModuleInfo));
	strlcpy(pm->filename, filename, MAXPATH);
	pm->my_perl = perl_alloc ();
	PL_perl_destruct_level = 1;
	perl_construct (pm->my_perl);

	perl_parse (pm->my_perl, xs_init, 4, perl_args, NULL);
	/*
	   Now initialising the perl interpreter by loading the
	   perl_definition array.
	 */
	eval_pv (perl_definitions, TRUE);

	if (!execute_perl (pm, sv_2mortal (newSVpv ("NeoStats::Embed::load", 0)),
								filename)) {
		/* it loaded ok */
		nlog(LOG_NORMAL, "Loaded Perl Module %s", filename);
	} else {
		nlog(LOG_WARNING, "Errors in Perl Module %s", filename);
		perl_destruct (pm->my_perl);
		perl_free (pm->my_perl);
		free(pm->modinfo);
		free(pm);
	}
		
	node = lnode_create(pm);
	list_append(perlmods, node);
	return NS_SUCCESS;
}

void
FiniPerl (void)
{
	lnode_t *node;
	PerlModInfo *pm;
	node = list_first(perlmods);
	while (node != NULL) {
		pm = lnode_get(node);
		execute_perl (pm, sv_2mortal (newSVpv ("NeoStats::Embed::unload", 0)), pm->filename);
		perl_destruct (pm->my_perl);
		perl_free (pm->my_perl);
		free(pm->modinfo);
		free(pm);
		node = list_next(perlmods, node);
	}
}

void ns_cmd_modperlist(CmdParams *cmd) {
	lnode_t *node;
	PerlModInfo *pm;
	node = list_first(perlmods);
	while (node != NULL) {
		pm = lnode_get(node);
		irc_prefmsg(ns_botptr, cmd->source,__("Perl Module: %s (%s)", cmd->source), pm->filename, pm->modinfo->version);
		irc_prefmsg(ns_botptr, cmd->source,"      : %s", pm->modinfo->description);
		node = list_next(perlmods, node);
	}
}
