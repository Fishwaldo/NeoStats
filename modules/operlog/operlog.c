/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Copyright (c) 1999-2002 Adam Rutter, Justin Hammond
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
** $Id: operlog.c,v 1.6 2003/01/21 13:15:34 fishwaldo Exp $
*/

#include <stdio.h>
#include "dl.h"
#include "stats.h"
#include "ol_help.c"

const char olversion_date[] = __DATE__;
const char olversion_time[] = __TIME__;
char *s_OperLog;

extern const char *cs_help[];
int ol_chatops(char **av, int ac);

Module_Info my_info[] = { {
    "OperLog",
    "IRCOp monitoring service",
    "0.0.1"
} };

int new_m_version(char *origin, char **av, int ac) {
    snumeric_cmd(351, origin, "Module OperLog Loaded, Version: %s %s %s",my_info[0].module_version,olversion_date,olversion_time);
    return 0;
}

Functions my_fn_list[] = { 
    { "VERSION",    new_m_version,    1 },
    { NULL,        NULL,     0}
};

int __Bot_Message(char *origin, char **av, int ac)
{
    User *u;
    u = finduser(origin);

    return 1;

}

int Online(char **av, int ac) {

    if (init_bot(s_OperLog,"OperLog",me.name,"Network Connection & Mode Monitoring Service", "+oikSwgleq-x", my_info[0].module_name) == -1 ) {
        /* Nick was in use */
        s_OperLog = strcat(s_OperLog, "_");
        init_bot(s_OperLog,"OperLog",me.name,"Network Connection & Mode Monitoring Service", "+oikSwgleq-x", my_info[0].module_name);
    }
    return 1;
};

EventFnList my_event_list[] = {
    { "ONLINE", Online},
    { "CHATOPS", ol_chatops},
    { NULL, NULL}
};

Module_Info *__module_get_info() {
    return my_info;
};

Functions *__module_get_functions() {
    return my_fn_list;
};

EventFnList *__module_get_events() {
    return my_event_list;
};


void _init() {

    s_OperLog = "OperLog";

}

void _fini() {

};


/* Routine for logging items with the 'cslog' */
void ollog(char *fmt, ...)
{
        va_list ap;
        FILE *csfile = fopen("logs/ol.log", "a");
        char buf[512], fmtime[80];
        time_t tmp = time(NULL);

        va_start(ap, fmt);
        vsnprintf(buf, 512, fmt, ap);

        strftime(fmtime, 80, "%H:%M[%m/%d/%Y]", localtime(&tmp));

        if (!csfile) {
        log("Unable to open logs/ConnectServ.log for writing.");
        return;
        }

    fprintf(csfile, "(%s) %s\n", fmtime, buf);
        va_end(ap);
        fclose(csfile);

}


int ol_chatops(char **av, int ac) {
    strcpy(segv_location, "ol_chatops");
    /* Print Connection Notice */
chanalert(s_OperLog, "\2CHATOPS\2 %s", av);
  return 1;
}


