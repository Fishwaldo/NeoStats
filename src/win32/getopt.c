#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"

int	opterr = 1;
int optind = 1;
int optopt;
char* optarg;

#define BADCH	(int)
#define EMSG	""
#define report(s)  \
{ \
	fputs(*argv,stderr); \
	fputs(s,stderr); \
	fputc(optopt,stderr); \
	fputc('\n',stderr); \
	return('?'); \
}

int getopt(int argc, char * const *argv, const char *optstring)
{
	static char	*place = EMSG;	/* option letter processing */
	register char	*oli;		/* option letter list index */
	
	if(!*place) {			/* update scanning pointer */
		if(optind >= argc || *(place = argv[optind]) != '-' || !*++place) 
			return(EOF);
		if (*place == '-') {	/* found "--" */
			++optind;
			return(EOF);
		}
	}				/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' || !(oli = strchr(optstring,optopt))) 
	{
		if(!*place) ++optind;
		report(": illegal option -- ");
	}
	if (*++oli != ':') {		/* don't need argument */
		optarg = NULL;
		if (!*place) ++optind;
	}
	else {				/* need an argument */
		if (*place) optarg = place;	/* no white space */
		else if (argc <= ++optind) {	/* no arg */
			place = EMSG;
			report(": option requires an argument -- ");
		}
		else optarg = argv[optind];	/* white space */
		place = EMSG;
		++optind;
	}
	return(optopt);			/* dump back option letter */
}
