/* NeoStats - IRC Statistical Services Copyright (c) 1999-2002 NeoStats Group Inc.
** Adam Rutter, Justin Hammond & 'Niggles' http://www.neostats.net
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NeoStats Identification:
** ID:      database.c, 
** Version: 1.0
** Date:    08/03/2002
*/


#include "stats.h"
#include "statserv.h"


void SaveStats()
{
    FILE *fp = fopen("data/stats.db", "w");
    SStats *s;

    strcpy(segv_location, "StatServ-SaveStats");


    if (!fp) {
        log("Unable to open stats.db for writing.");
        return;
    }

    for (s = Shead; s; s = s->next) {
#ifdef DEBUG
    log("Writing statistics to database for %s", s->name);
#endif
        fprintf(fp, "%s %d %ld %ld %d %ld %ld %ld %d %d %ld\n", s->name,
            s->numsplits, s->maxusers, s->t_maxusers, s->maxopers,
            s->t_maxopers, s->lastseen, s->starttime, s->operkills,
            s->serverkills, s->totusers);
    }
    fclose(fp);
    if ((fp = fopen("data/nstats.db", "w")) == NULL) {
        log("Unable to open nstats.db for writing.");
        return;
    }
    fprintf(fp, "%d %ld %d %ld %ld %ld %ld\n", stats_network.maxopers, stats_network.maxusers, stats_network.maxservers, stats_network.t_maxopers, stats_network.t_maxusers, stats_network.t_maxservers, stats_network.totusers);
    fclose(fp);
}

void LoadStats()
{
    FILE *fp = fopen("data/nstats.db", "r");
    SStats *s;
    char buf[BUFSIZE];
    char *tmp;
    char *name, *numsplits, *maxusers, *t_maxusers,
        *maxopers, *t_maxopers, *lastseen, *starttime,
        *operkills, *serverkills, *totusers;

    strcpy(segv_location, "StatServ-LoadStats");


    if (fp) {
    while (fgets(buf, BUFSIZE, fp)) {
        stats_network.maxopers = atoi(strtok(buf, " "));
        stats_network.maxusers = atol(strtok(NULL, " "));
        stats_network.maxservers = atoi(strtok(NULL, " "));
        stats_network.t_maxopers = atoi(strtok(NULL, " "));
        stats_network.t_maxusers = atol(strtok(NULL, " "));
        stats_network.t_maxservers = atoi(strtok(NULL, " "));
        tmp = strtok(NULL, "");
        if (tmp==NULL) {
            fprintf(stderr, "Detected Old Version of Network Database, Upgrading\n");
            stats_network.totusers = stats_network.maxusers;
        } else {
            stats_network.totusers = atoi(tmp);
        }
    }
    fclose(fp);
    }

    if ((fp = fopen("data/stats.db", "r")) == NULL)
        return;

    memset(buf, '\0', BUFSIZE);
    while (fgets(buf, BUFSIZE, fp)) {
        s = smalloc(sizeof(SStats));
        name = strtok(buf, " ");
        numsplits = strtok(NULL, " ");
        maxusers = strtok(NULL, " ");
        t_maxusers = strtok(NULL, " ");
        maxopers = strtok(NULL, " ");
        t_maxopers = strtok(NULL, " ");
        lastseen = strtok(NULL, " ");
        starttime = strtok(NULL, " ");
        operkills = strtok(NULL, " ");
        serverkills = strtok(NULL, " ");
        totusers = strtok(NULL, " ");

        memcpy(s->name, name, MAXHOST);
        s->numsplits = atoi(numsplits);
        s->maxusers = atol(maxusers);
        s->t_maxusers = atol(t_maxusers);
        s->maxopers = atoi(maxopers);
        s->t_maxopers = atol(t_maxopers);
        s->lastseen = atol(lastseen);
        s->starttime = atol(starttime);
        s->operkills = atoi(operkills);
        s->serverkills = atol(serverkills);
        s->users = 0;
        s->opers = 0;
        s->daily_totusers = 0;
        if (totusers==NULL) {
            s->totusers = 0;
            fprintf(stderr, "Detected Old Version of Server Database, Upgrading\n");
        } else {
            s->totusers = atol(totusers);
        }

#ifdef DEBUG
    log("LoadStats(): Loaded statistics for %s", s->name);
#endif
        if (!Shead) {
            Shead = s;
            Shead->next = NULL;
        } else {
            s->next = Shead;
            Shead = s;
        }
    }
    fclose(fp);
}

