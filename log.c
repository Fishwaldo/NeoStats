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
** $Id: log.c,v 1.1 2003/04/10 09:32:01 fishwaldo Exp $
*/

#include "stats.h"
#include "conf.h"

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



/** @Configurable logging function
 */
void nlog(int level, int scope, char *fmt, ...) {
	va_list ap;
	static FILE *logfile;
	char buf[512], fmttime[80];
	time_t ts = time(NULL);
	
	if (level <= config.debug) {	
		if (!logfile) 
			if ((logfile = fopen("logs/neostats.log", "a")) == NULL) return;

		strftime(fmttime, 80, "%d/%m/%Y[%H:%M]", localtime(&ts));
		va_start(ap, fmt);
		vsnprintf(buf, 512, fmt, ap);
	
		fprintf(logfile, "(%s) %s %s - %s\n", fmttime, loglevels[level-1], scope ? "core" : segvinmodule, buf);
	
		if (config.foreground)
			printf("%s %s - %s\n", loglevels[level-1], scope ? "core" : segvinmodule, buf);
		va_end(ap);
	}
}		