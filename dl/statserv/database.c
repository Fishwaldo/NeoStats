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
    CStats *c;
    hnode_t *sn;
    lnode_t *cn;
    hscan_t ss;
    strcpy(segv_location, "StatServ-SaveStats");


    if (!fp) {
        log("Unable to open stats.db for writing.");
        return;
    }

    hash_scan_begin(&ss, Shead);
    while ((sn = hash_scan_next(&ss))) {
    	s=hnode_get(sn);
#ifdef DEBUG
    log("Writing statistics to database for %s", s->name);
#endif
        fprintf(fp, "%s %d %ld %ld %d %ld %ld %ld %d %d %ld\n", s->name,
            s->numsplits, s->maxusers, s->t_maxusers, s->maxopers,
            s->t_maxopers, s->lastseen, s->starttime, s->operkills,
            s->serverkills, s->totusers);
    }
    fclose(fp);
    if ((fp = fopen("data/cstats.db", "w")) == NULL) {
    	log("Unable to open cstats.db for writting.");
    	return;
    }
    cn = list_first(Chead);
    while (cn) {
    	c = lnode_get(cn);
#ifdef DEBUG
	log("Writting Statistics to database for %s", c->name);
#endif
	fprintf(fp, "%s %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n", c->name, c->topics, c->totmem, c->kicks, (long)c->lastseen, c->maxmems, (long)c->t_maxmems, c->maxkicks, (long)c->t_maxkicks, c->maxjoins, (long)c->t_maxjoins);
	cn = list_next(Chead, cn);
    }
    fclose(fp);
    if ((fp = fopen("data/nstats.db", "w")) == NULL) {
        log("Unable to open nstats.db for writing.");
        return;
    }
    fprintf(fp, "%d %ld %d %ld %ld %ld %ld %ld %ld\n", stats_network.maxopers, stats_network.maxusers, stats_network.maxservers, stats_network.t_maxopers, stats_network.t_maxusers, stats_network.t_maxservers, stats_network.totusers, stats_network.maxchans, stats_network.t_chans);
    fclose(fp);
}

void LoadStats()
{
    FILE *fp = fopen("data/nstats.db", "r");
    SStats *s;
    CStats *c;
    char buf[BUFSIZE];
    char *tmp;
    char *name, *numsplits, *maxusers, *t_maxusers,
        *maxopers, *t_maxopers, *lastseen, *starttime,
        *operkills, *serverkills, *totusers;
    char *topics, *totmem, *kicks, *joins, *maxmems, *t_maxmems, *maxkicks, *t_maxkicks, *maxjoins, *t_maxjoins; 


    hnode_t *sn;
    lnode_t *cn;
    int count;
    strcpy(segv_location, "StatServ-LoadStats");
    Chead = list_create(SS_CHAN_SIZE);
    Shead = hash_create(S_TABLE_SIZE,0,0);


    if (fp) {
    while (fgets(buf, BUFSIZE, fp)) {
        stats_network.maxopers = atoi(strtok(buf, " "));
        stats_network.maxusers = atol(strtok(NULL, " "));
        stats_network.maxservers = atoi(strtok(NULL, " "));
        stats_network.t_maxopers = atoi(strtok(NULL, " "));
        stats_network.t_maxusers = atol(strtok(NULL, " "));
        stats_network.t_maxservers = atoi(strtok(NULL, " "));
        tmp = strtok(NULL, " ");
        if (tmp==NULL) {
            fprintf(stderr, "Detected Old Version(1.0) of Network Database, Upgrading\n");
            stats_network.totusers = stats_network.maxusers;
        } else {
            stats_network.totusers = atoi(tmp);
        }
        tmp = strtok(NULL, " ");
        if (tmp == NULL) {
           log("Detected Old version (3.0) of Network Database, Upgrading");
           stats_network.maxchans = 0;
	   stats_network.t_chans = time(NULL);
        } else {
           stats_network.maxchans = atol(tmp);
	   tmp = strtok(NULL, "");
           stats_network.t_chans = atol(tmp);

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
	sn = hnode_create(s);
	if (hash_isfull(Shead)) {
		log("Eeek, StatServ Server Hash is Full!");
	} else {
		hash_insert(Shead, sn, s->name);
	}
    }
    fclose(fp);
    if ((fp = fopen("data/cstats.db", "r")) == NULL)
    	return;
    memset(buf, '\0', BUFSIZE);
    name = smalloc(CHANLEN);
    topics = smalloc(BUFSIZE);
    totmem = smalloc(BUFSIZE);
    kicks = smalloc(BUFSIZE);
    joins = smalloc(BUFSIZE);
    lastseen = smalloc(BUFSIZE);
    maxmems = smalloc(BUFSIZE);
    t_maxmems = smalloc(BUFSIZE);
    maxkicks = smalloc(BUFSIZE);
    t_maxkicks = smalloc(BUFSIZE);
    maxjoins = smalloc(BUFSIZE);
    t_maxjoins = smalloc(BUFSIZE);
    while (fgets(buf, BUFSIZE, fp)) {
        c = smalloc(sizeof(CStats));
        count = sscanf(buf, "%s %s %s %s %s %s %s %s %s %s %s\n", name, topics, totmem, kicks, lastseen, maxmems, t_maxmems, maxkicks, t_maxkicks, maxjoins, t_maxjoins);
        strcpy(c->name, name);
        c->topics = atol(topics);
	c->totmem = atol(totmem);
	c->kicks = atol(kicks);
	c->lastseen = (time_t)atol(lastseen);
	c->maxmems = atol(maxmems);
	c->t_maxmems = (time_t)atol(t_maxmems);
	c->maxkicks = atol(maxkicks);
	c->t_maxkicks = (time_t)atol(t_maxkicks);
	c->maxjoins = atol(maxjoins);
	c->t_maxjoins = (time_t)atol(t_maxjoins);
	c->topicstoday = 0;
	c->joinstoday = 0;
	c->members = 0;
	cn = lnode_create(c);
	if (list_isfull(Chead)) {
		log("Eeek, StatServ Channel Hash is Full!");
	} else {
#ifdef DEBUG
		log("Loading %s Channel Data", c->name);
#endif
		if ((time(NULL) - c->lastseen) <  604800) {
			list_append(Chead, cn);
		} else {
			log("Deleting Old Channel %s", c->name);
		}
	}
    }    
   fclose(fp);    
   free(name);
   free(topics);
   free(totmem);
   free(kicks);
   free(joins);
   free(lastseen);
   free(maxmems);
   free(t_maxmems);
   free(maxkicks);
   free(t_maxkicks);
   free(maxjoins);
   free(t_maxjoins);
    



}

