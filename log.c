/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: log.c,v 1.5 2003/04/17 00:16:26 fishwaldo Exp $
*/

#include "stats.h"
#include "conf.h"
#include "hash.h"
#include "log.h"

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
} logs_ ;

hash_t *logs;

void *close_logs();

/** @brief Initilize the logging functions 
 */
void init_logs() {
	logs = hash_create(-1, 0, 0);
	if (!logs) {
		printf("ERROR: Can't Initilize Log SubSystem. Exiting!");
		/* if this fails, no need to call do_exit, as this is the first thing that runs... so nothing to do! */
		exit(-1);
	}
}
/** @brief Occasionally flush log files out 
 */
 

void *close_logs() {
	hscan_t hs;
	hnode_t *hn;
	struct logs_ *logentry;

	hash_scan_begin(&hs, logs);
	while ((hn = hash_scan_next(&hs)) != NULL) {
		logentry = hnode_get(hn);
		if (logentry->flush > 0) {
			fflush(logentry->logfile);
			logentry->flush = 0;
		}
#ifdef DEBUG
		printf("Closing Logfile %s (%s)\n", logentry->name, (char *)hnode_getkey(hn));
#endif
		fclose(logentry->logfile);
		hash_scan_delete(logs, hn);
		hnode_destroy(hn);
		free(logentry);
	}		
	return NULL;
}


/** @Configurable logging function
 */
void nlog(int level, int scope, char *fmt, ...) {
	va_list ap;
	char buf[512], fmttime[80];
	int gotlog;
	hnode_t *hn;
	struct logs_ *logentry;
	time_t ts = time(NULL);

	if (level <= config.debug) {	
		/* if scope is > 0, then log to a diff file */
		if (scope > 0) {
			if (strlen(segvinmodule) > 1) {
				hn = hash_lookup(logs, segvinmodule);
			} else {
				nlog(LOG_ERROR, LOG_CORE, "Warning, nlog called with LOG_MOD, but segvinmodule is blank! Logging to Core");
				hn = hash_lookup(logs, "core");
			}
		} else {
			hn = hash_lookup(logs, "core");
		}
		if (hn) {
		/* we found our log entry */
			logentry = hnode_get(hn);
			gotlog = 1;
		} else {
		/* log file not found */
			if ((strlen(segvinmodule) <= 1) && (scope > 0)) {
#ifdef DEBUG
				printf("segvinmodule is blank, but scope is for Modules!\n");
#endif
				/* bad, but hey !*/
				scope = 0;
			}
			logentry = malloc(sizeof(struct logs_));
			strncpy(logentry->name, scope > 0 ? segvinmodule : "core", 30);
			snprintf(buf, 40, "logs/%s.log", scope > 0 ? segvinmodule : "NeoStats");
			logentry->logfile = fopen(buf, "a");
			logentry->flush = 0;
			hn = hnode_create(logentry);
			hash_insert(logs, hn, logentry->name);
		}

		if (!logentry->logfile) {

#ifdef DEBUG
			printf("%s\n", strerror(errno));
			do_exit(0);
#endif
		}
		strftime(fmttime, 80, "%d/%m/%Y[%H:%M]", localtime(&ts));
		va_start(ap, fmt);
		vsnprintf(buf, 512, fmt, ap);
	
		fprintf(logentry->logfile, "(%s) %s %s - %s\n", fmttime, loglevels[level-1], scope > 0 ? segvinmodule : "CORE" , buf);
		logentry->flush = 1;
#ifndef DEBUG	
		if (config.foreground)
#endif
			printf("%s %s - %s\n", loglevels[level-1], scope > 0 ? segvinmodule : "CORE", buf);
		va_end(ap);
	}
}		
