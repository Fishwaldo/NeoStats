/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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

#include "neostats.h"
#include "hash.h"
#include "log.h"
#include "services.h"
#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

typedef struct LogEntry {
	FILE *logfile;
	char name[MAX_MOD_NAME];
	char logname[MAXPATH];
	unsigned int flush;
} LogEntry;

const char* CoreLogFileName="NeoStats";
char LogFileNameFormat[MAX_LOGFILENAME]="-%m-%d";

const char *loglevels[LOG_LEVELMAX] = {
	"CRITICAL",
	"ERROR",
	"WARNING",
	"NOTICE",
	"NORMAL",
	"INFO",
};

const char *dloglevels[DEBUGMAX] = {
	"DEBUGRX",
	"DEBUGTX",
	"DEBUG1",
	"DEBUG2",
	"DEBUG3",
	"DEBUG4",
	"DEBUG5",
	"DEBUG6",
	"DEBUG7",
	"DEBUG8",
	"DEBUG9",
	"DEBUG10",
};

static char log_buf[BUFSIZE];
static char log_fmttime[TIMEBUFSIZE];
static hash_t *logs;

/** @brief initialize the logging functions 
 */
int
InitLogs (void)
{
	SET_SEGV_LOCATION();
	logs = hash_create (-1, 0, 0);
	if (!logs) {
		printf ("ERROR: Can't initialize log subsystem. Exiting!");
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief Occasionally flush log files out 
 */
void
CloseLogs (void)
{
	hscan_t hs;
	hnode_t *hn;
	LogEntry *logentry;

	SET_SEGV_LOCATION();
	hash_scan_begin (&hs, logs);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		logentry = hnode_get (hn);
		logentry->flush = 0;
#ifdef DEBUG
		printf ("Closing Logfile %s (%s)\n", logentry->name, (char *) hnode_getkey (hn));
#endif
		if(logentry->logfile)
		{
			fflush (logentry->logfile);
			fclose (logentry->logfile);
			logentry->logfile = NULL;
		}
		hash_scan_delete (logs, hn);
		hnode_destroy (hn);
		sfree (logentry);
	}
}

void 
FiniLogs (void) 
{
	CloseLogs();
	hash_destroy(logs);
}

void make_log_filename(char* modname, char *logname)
{
	time_t t = time(NULL);
	strftime (log_fmttime, TIMEBUFSIZE, LogFileNameFormat, localtime (&t));
	strlwr(modname);
	ircsnprintf (logname, MAXPATH, "logs/%s%s.log", modname, log_fmttime);
}

static void
debuglog (const char* time, const char* level, const char *line)
{
	static int chanflag = 0;
	FILE *logfile;

	logfile = fopen ("logs/debug.log", "a");
	if (logfile) {
		fprintf (logfile, "%s %s %s - %s\n", time, level, GET_CUR_MODNAME(), line);
		fclose (logfile);
	}
	if(config.debugtochan&&!chanflag) {
		chanflag = 1;
		irc_chanprivmsg(ns_botptr, config.debugchan, "%s %s %s - %s\n", time, level, GET_CUR_MODNAME(), line);
		chanflag = 0;
	}
}

/** @Configurable logging function
 */
void
dlog (DEBUG_LEVEL level, char *fmt, ...)
{
	va_list ap;
	
#ifndef WIN32
	if (level <= config.debuglevel) {
		/* Support for module specific only debug info */
		if(ircstrcasecmp(config.debugmodule, "all")== 0 || ircstrcasecmp(config.debugmodule, GET_CUR_MODNAME()) ==0)
		{
#endif
			/* we update me.now here, because some functions might be busy and not call the loop a lot */
			me.now = time(NULL);
			ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%lu", me.now);
			strftime (log_fmttime, TIMEBUFSIZE, "%d/%m/%Y[%H:%M:%S]", localtime (&me.now));
			va_start (ap, fmt);
			ircvsnprintf (log_buf, BUFSIZE, fmt, ap);
			va_end (ap);
#ifdef WIN32
			printf ("%s %s - %s\n", dloglevels[level - 1], GET_CUR_MODNAME(), log_buf);
#else
			if (config.foreground) {
				printf ("%s %s - %s\n", dloglevels[level - 1], GET_CUR_MODNAME(), log_buf);
			}
#endif
			debuglog (log_fmttime, dloglevels[level - 1], log_buf);
#ifndef WIN32
		}
	}
#endif
}

/** @Configurable logging function
 */
void
nlog (LOG_LEVEL level, char *fmt, ...)
{
	va_list ap;
	LogEntry *logentry;
	
	if (level <= config.loglevel) {
		logentry = (LogEntry *)hnode_find (logs, GET_CUR_MODNAME());
		if (logentry) {
			/* we found our log entry */
			if(!logentry->logfile)
				logentry->logfile = fopen (logentry->logname, "a");
		} else {
			logentry = smalloc (sizeof (LogEntry));
			strlcpy (logentry->name, GET_CUR_MODNAME() , MAX_MOD_NAME);
			make_log_filename(logentry->name, logentry->logname);
			logentry->logfile = fopen (logentry->logname, "a");
			logentry->flush = 0;
			hnode_create_insert (logs, logentry, logentry->name);
		}

#ifdef DEBUG
		if (!logentry->logfile) {
			printf ("%s\n", strerror (errno));
			do_exit (NS_EXIT_NORMAL, NULL);
		}
#endif
		/* we update me.now here, because some functions might be busy and not call the loop a lot */
		me.now = time(NULL);
		ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%lu", me.now);
		strftime (log_fmttime, TIMEBUFSIZE, "%d/%m/%Y[%H:%M:%S]", localtime (&me.now));
		va_start (ap, fmt);
		ircvsnprintf (log_buf, BUFSIZE, fmt, ap);
		va_end (ap);
		fprintf (logentry->logfile, "(%s) %s %s - %s\n", log_fmttime, loglevels[level - 1], GET_CUR_MODNAME(), log_buf);
		logentry->flush = 1;
		if (config.foreground) {
			printf ("%s %s - %s\n", loglevels[level - 1], GET_CUR_MODNAME(), log_buf);
		}
	}
#ifndef WIN32
	if (config.debuglevel) {
#endif
		debuglog (log_fmttime, loglevels[level - 1], log_buf);
#ifndef WIN32
	}
#endif
}

/** rotate logs, called at midnight
 */
void
ResetLogs (void)
{
	hscan_t hs;
	hnode_t *hn;
	LogEntry *logentry;

	SET_SEGV_LOCATION();
	hash_scan_begin (&hs, logs);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		logentry = hnode_get (hn);
		/* If file handle is vald we must have used the log */
		if(logentry->logfile) {
			if (logentry->flush > 0) {
				fflush (logentry->logfile);
			}
			fclose (logentry->logfile);		
			logentry->logfile = NULL;
		}
		logentry->flush = 0;
#ifdef DEBUG
		printf ("Closing Logfile %s (%s)\n", logentry->name, (char *) hnode_getkey (hn));
#endif
		/* make new file name but do not open until needed to avoid 0 length files*/
		make_log_filename(logentry->name, logentry->logname);
	}
}

/* this is for printing out details during an assertion failure */
void
nassert_fail (const char *expr, const char *file, const int line, const char *infunk)
{
#ifdef HAVE_BACKTRACE
	void *array[50];
	size_t size;
	char **strings;
	size_t i;
/* thanks to gnulibc libary for letting me find this usefull function */
	size = backtrace (array, 10);
	strings = backtrace_symbols (array, size);
#endif

	nlog (LOG_CRITICAL, "Assertion Failure!!!!!!!!!!!");
	nlog (LOG_CRITICAL, "Function: %s (%s:%d)", infunk, file, line);
	nlog (LOG_CRITICAL, "Expression: %s", expr);
#ifdef HAVE_BACKTRACE
	for (i = 1; i < size; i++) {
		nlog (LOG_CRITICAL, "BackTrace(%d): %s", i - 1, strings[i]);
	}
#endif
	nlog (LOG_CRITICAL, "Shutting Down!");
	exit (EXIT_FAILURE);
}

#if SQLSRV
/* this is for sqlserver logging callback */
void sqlsrvlog(char *logline) 
{
	dlog(DEBUG1, "SqlSrv: %s", logline);
}
#endif
