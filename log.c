/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2003 Adam Rutter, Justin Hammond
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

#include "stats.h"
#include "conf.h"
#include "hash.h"
#include "log.h"
#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

const char* CoreLogFileName="NeoStats";
/*Original NeoStats logging format*/
char* LogFileNameFormat="-%m-%d";
/*test logging format*/
/*char* LogFileNameFormat="%Y%M%d";*/

const char *loglevels[10] = {
	"CRITICAL",
	"ERROR",
	"WARNING",
	"NOTICE",
	"NORMAL",
	"INFO",
	"DEBUG1",
	"DEBUG2",
	"DEBUG3",
	"INSANE"
};
struct logs_ {
	FILE *logfile;
	char name[30];
	unsigned int flush;
} logs_;

hash_t *logs;

void *close_logs ();

/** @brief Initilize the logging functions 
 */
int
init_logs ()
{
	SET_SEGV_LOCATION();
	logs = hash_create (-1, 0, 0);
	if (!logs) {
		printf ("ERROR: Can't Initilize Log SubSystem. Exiting!");
		return -1;
	}
	return 0;
}

/** @brief Occasionally flush log files out 
 */


void *
close_logs ()
{
	hscan_t hs;
	hnode_t *hn;
	struct logs_ *logentry;

	SET_SEGV_LOCATION();
	hash_scan_begin (&hs, logs);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		logentry = hnode_get (hn);
		fflush (logentry->logfile);
		logentry->flush = 0;
#ifdef DEBUG
		printf ("Closing Logfile %s (%s)\n", logentry->name, (char *) hnode_getkey (hn));
#endif
		fclose (logentry->logfile);
		hash_scan_delete (logs, hn);
		hnode_destroy (hn);
		free (logentry);
	}
	return NULL;
}


/** @Configurable logging function
 */
void
nlog (int level, int scope, char *fmt, ...)
{
	va_list ap;
	char buf[512], fmttime[80];
	int gotlog;
	hnode_t *hn;
	struct logs_ *logentry;
	time_t ts = time (NULL);

	if (level <= config.debug) {
		/* if scope is > 0, then log to a diff file */
		if (scope > 0) {
			if (strlen (segvinmodule) > 1) {
				hn = hash_lookup (logs, segvinmodule);
			} else {
				nlog (LOG_ERROR, LOG_CORE, "Warning, nlog called with LOG_MOD, but segvinmodule is blank! Logging to Core");
				hn = hash_lookup (logs, CoreLogFileName);
			}
		} else {
			hn = hash_lookup (logs, CoreLogFileName);
		}
		if (hn) {
			/* we found our log entry */
			logentry = hnode_get (hn);
			gotlog = 1;
		} else {
			/* log file not found */
			if ((strlen (segvinmodule) <= 1) && (scope > 0)) {
#ifdef DEBUG
				printf ("segvinmodule is blank, but scope is for Modules!\n");
#endif
				/* bad, but hey ! */
				scope = 0;
			}
			logentry = malloc (sizeof (struct logs_));
			strncpy (logentry->name, scope > 0 ? segvinmodule : CoreLogFileName, 30);
			snprintf (buf, 40, "logs/%s.log", logentry->name);
			logentry->logfile = fopen (buf, "a");
			logentry->flush = 0;
			hn = hnode_create (logentry);
			hash_insert (logs, hn, logentry->name);
		}

		if (!logentry->logfile) {

#ifdef DEBUG
			printf ("%s\n", strerror (errno));
			do_exit (0);
#endif
		}
		strftime (fmttime, 80, "%d/%m/%Y[%H:%M]", localtime (&ts));
		va_start (ap, fmt);
		vsnprintf (buf, 512, fmt, ap);

		fprintf (logentry->logfile, "(%s) %s %s - %s\n", fmttime, loglevels[level - 1], scope > 0 ? segvinmodule : "CORE", buf);
		logentry->flush = 1;
#ifndef DEBUG
		if (config.foreground)
#endif
			printf ("%s %s - %s\n", loglevels[level - 1], scope > 0 ? segvinmodule : "CORE", buf);
		va_end (ap);
	}
}
void
ResetLogs ()
{
	char tmp[255], tmp2[255];
	time_t t = time (NULL);
	hscan_t hs;
	hnode_t *hn;
	struct logs_ *logentry;


	SET_SEGV_LOCATION();
	hash_scan_begin (&hs, logs);
	while ((hn = hash_scan_next (&hs)) != NULL) {
		logentry = hnode_get (hn);
		if (logentry->flush > 0) {
			fflush (logentry->logfile);
			logentry->flush = 0;
#ifdef DEBUG
			printf ("Closing Logfile %s (%s)\n", logentry->name, (char *) hnode_getkey (hn));
#endif
			fclose (logentry->logfile);
			
			/* rename log file and open new file */
			strftime (tmp2, 255, LogFileNameFormat, localtime (&t));
			snprintf (tmp, 255, "logs/%s%s.log", logentry->name, tmp2);
			snprintf (tmp2, 255, "logs/%s.log", logentry->name);
			rename (tmp2, tmp);
			logentry->logfile = fopen (tmp2, "a");		
		}
	}
}



/* this is for printing out details during a assertion failure */
extern void
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

	nlog (LOG_CRITICAL, LOG_CORE, "Assertion Failure!!!!!!!!!!!");
	nlog (LOG_CRITICAL, LOG_CORE, "Function: %s (%s:%d)", infunk, file, line);
	nlog (LOG_CRITICAL, LOG_CORE, "Expression: %s", expr);
#ifdef HAVE_BACKTRACE
	for (i = 1; i < size; i++) {
		nlog (LOG_CRITICAL, LOG_CORE, "BackTrace(%d): %s", i - 1, strings[i]);
	}
#endif
	nlog (LOG_CRITICAL, LOG_CORE, "Shutting Down!");
	exit (-1);
}
