/* NetStats - IRC Statistical Services
** Copyright (c) 1999 Adam Rutter, Justin Hammond
** http://codeworks.kamserve.com
*
** Based from GeoStats 1.1.0 by Johnathan George net@lite.net
*
** NetStats CVS Identification
** $Id: stats.c,v 1.2 2000/02/05 04:54:00 fishwaldo Exp $
*/

#include "statserv.h"

SStats *Shead;
TLD *tldhead;
/* static Stats *new_stats(const char *); */



/* "net" -- not DOT"net" */
TLD *findtld(char *tld)
{
	TLD *t;

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
		log("Unable to find TLD entry for %s (%s)", u->nick, m);
		log("*** NOTICE *** Please send a copy of this logfile to "
			"net@lite.net");
		return NULL;
	}
	t->users++;

	return t;
}

void LoadTLD()
{
	register int i;
	FILE *fp;
	char buf[BUFSIZE], buf2[BUFSIZE];
	char *tmp = NULL, *tmp2 = NULL;
	TLD *t;

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

		if ((tmp2 = strchr(buf2, '\n')))	*tmp2 = '\0';

		t = smalloc(sizeof(TLD));
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

	for (t = tldhead; t; t = t->next) {
		t->users = 0;
	}
}

/* static Stats *new_stats(const char *name)
{
	Stats *s = calloc(sizeof(Stats), 1);

#ifdef DEBUG
	log("new_stats(%s)", name);
#endif

	if (!s) {
		log("Out of memory.");
		exit(0);
	}

	memcpy(s->name, name, MAXHOST);
	s->numsplits = 0;
	s->maxusers = 0L;
	s->t_maxusers = time(NULL);
	s->t_maxopers = time(NULL);
	s->maxopers = 0;
	s->totusers = 0;
	s->daily_totusers = 0;
	s->lastseen = time(NULL);
	s->starttime = time(NULL);

	if (!shead) {
		shead = s;
		shead->next = NULL;
	} else {
		s->next = shead;
		shead = s;
	}
	return s;
}

void AddStats(SStats *s)
{
	Stats *st = findstats(s->name);

#ifdef DEBUG
	log("AddStats(%s)", s->name);
#endif

	if (!st) {
		st = new_stats(s->name);
		s->stats = st;
	} else {
		s->stats = st;
		st->lastseen = time(NULL);
	}
}
*/

SStats *findstats(char *name)
{
	SStats *t;
#ifdef DEBUG
	log("findstats(%s)", name);
#endif
	for (t = Shead; t; t = t->next) {
		if (!strcasecmp(name, t->name))
			return t;
	}
	return NULL;
}

/*

void SaveStats()
{
	FILE *fp = fopen("data/stats.db", "w");
	Stats *s;

	if (!fp) {
		log("Unable to open stats.db for writing.");
		return;
	}

	for (s = shead; s; s = s->next) {
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
	fprintf(fp, "%d %ld %d %ld %ld %ld %ld\n", stats_network.maxopers, stats_network.maxusers,
		stats_network.maxservers, stats_network.t_maxopers, stats_network.t_maxusers, stats_network.t_maxservers, stats_network.totusers);
	fclose(fp);
}
*/
void LoadStats()
{
	FILE *fp = fopen("data/nstats.db", "r");
	SStats *s;
	char buf[BUFSIZE];
	char *tmp;
	char *name, *numsplits, *maxusers, *t_maxusers,
		*maxopers, *t_maxopers, *lastseen, *starttime,
		*operkills, *serverkills, *totusers;
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
