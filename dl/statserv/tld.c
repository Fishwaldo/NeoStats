/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      tld.c, 
** Version: 3.1
** Date:    08/03/2002
*/

#include "statserv.h"



void DelTLD(User *u) {
    TLD *t = NULL;
    char *m;

    strcpy(segv_location, "StatServ-DelTLD");
    

    m = strrchr(u->hostname, '.');

    if (!m)
        t = findtld("num");
    else
        m++;

    if (!t) {
        if (!isdigit(*m))
            t = findtld(m);
        else
            t = findtld("num");
    }

    if (!t) {
        log("Unable to find TLD entry for %s (%s), damn vhosts!", u->nick, m);
        return;
    }
    t->users--;
}    

/* "net" -- not DOT"net" */
TLD *findtld(char *tld)
{
    TLD *t;

    strcpy(segv_location, "StatServ-findtld");


    for (t = tldhead; t; t = t->next) {
        if (!strcasecmp(t->tld, tld))
            return t;
    }

    return NULL;
}

TLD *AddTLD(User *u)
{
    TLD *t = NULL;
    char *m;

    strcpy(segv_location, "StatServ-AddTLD");

    m = strrchr(u->hostname, '.');

    if (!m)
        t = findtld("num");
    else
        m++;

    if (!t) {
        if (!isdigit(*m))
            t = findtld(m);
        else
            t = findtld("num");
    }

    if (!t) {
        log("Unable to find TLD entry for %s (%s), damn vhosts!", u->nick, m);
        return NULL;
    }
    t->users++;
    t->daily_users++;

    return t;
}

void LoadTLD()
{
    register int i;
    FILE *fp;
    char buf[BUFSIZE], buf2[BUFSIZE];
    char *tmp = NULL, *tmp2 = NULL;
    TLD *t;

    strcpy(segv_location, "StatServ-LoadTLD");


    if ((fp = fopen("data/tlds.nfo", "r")) == NULL) {
        log("Top Level Domain Statistics not loaded: file not found.");
        return;
    }

    while (fgets(buf, BUFSIZE, fp)) {
        memcpy(buf2, buf, sizeof(buf));
        tmp = strrchr(buf, '(');
        tmp++;
        tmp[strlen(tmp) - 1] = '\0';
        tmp[strlen(tmp) - 1] = '\0';
        for (i = 0; buf[i] != '('; i++)
            buf2[strlen(buf2) + 1] = buf[i];

        if ((tmp2 = strchr(buf2, '\n')))    *tmp2 = '\0';

        t = malloc(sizeof(TLD));
        t->users = 0;
        t->country = sstrdup(buf2);
        memcpy(t->tld, tmp, sizeof(t->tld));

        if (!tldhead) {
            tldhead = t;
            tldhead->next = NULL;
        } else {
            t->next = tldhead;
            tldhead = t;
        }
    }
    fclose(fp);
}

void init_tld()
{
    TLD *t;

    strcpy(segv_location, "StatServ-init_tld");


    for (t = tldhead; t; t = t->next) {
        t->users = 0;
    }
}

