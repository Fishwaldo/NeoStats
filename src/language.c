/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
** Portions copyright (c) 1996-2004 Andrew Church.
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
#include "language.h"

#define DEF_LANGUAGE LANG_EN

/*************************************************************************/

/* Indexes of available languages (exported), terminated by -1: */
int langlist[NUM_LANGS+1];

/* The list of lists of messages. */
static char **langtexts[NUM_LANGS];

/* Order in which languages should be displayed: (alphabetical) */
static int langorder[] = {
    LANG_EN,		/* English (US) */
    LANG_NL,		/* Dutch */
    LANG_FR,		/* French */
    LANG_DE,		/* German */
    LANG_HU,		/* Hungarian */
    LANG_IT,		/* Italian */
/*    LANG_JA_JIS,*/	/* Japanese (JIS encoding) */
    LANG_JA_EUC,	/* Japanese (EUC encoding) */
    LANG_JA_SJIS,	/* Japanese (SJIS encoding) */
    LANG_PT,		/* Portugese */
    LANG_ES,		/* Spanish */
    LANG_TR,		/* Turkish */
};

/* Filenames for language files: */
static struct {
    int num;
    const char *filename;
} filenames[] = {
    { LANG_EN,	"en_us" },
    { LANG_NL,		"nl" },
    { LANG_FR,		"fr" },
    { LANG_DE,		"de" },
    { LANG_HU,		"hu" },
    { LANG_IT,		"it" },
/*    { LANG_JA_JIS,	"ja_jis" },*/
    { LANG_JA_EUC,	"ja_euc" },
    { LANG_JA_SJIS,	"ja_sjis" },
    { LANG_PT,		"pt" },
    { LANG_ES,		"es" },
    { LANG_TR,		"tr" },
    { -1, NULL }
};

/* Mapping of language strings (to allow on-the-fly replacement of strings) */
static int *langmap;

/* Array indicating which languages were actually loaded (needed since NULL
 * langtexts[] pointers are redirected to DEF_LANGUAGE) */
static int is_loaded[NUM_LANGS];

/*************************************************************************/

/* Load a language file. */

static int read_int32(int *ptr, FILE *f)
{
    int a = fgetc(f);
    int b = fgetc(f);
    int c = fgetc(f);
    int d = fgetc(f);
    if (a == EOF || b == EOF || c == EOF || d == EOF)
		return -1;
    *ptr = a<<24 | b<<16 | c<<8 | d;
    return 0;
}

static void load_lang(int index, const char *filename)
{
    char buf[256];
    FILE *f;
    int num, size, i;
    char *data = NULL;

	dlog(DEBUG4, "debug: Loading language %d from file `languages/%s'", index, filename);
    snprintf(buf, sizeof(buf), "languages/%s", filename);
    if (!(f = fopen(buf, "r"))) {
		nlog(LOG_ERROR, "Failed to load language %d (%s)", index, filename);
		return;
    } else if (read_int32(&num, f) < 0) {
		nlog(LOG_ERROR, "Failed to read number of strings for language %d (%s)", index, filename);
		return;
    } else if (read_int32(&size, f) < 0) {
		nlog(LOG_ERROR, "Failed to read data size for language %d (%s)", index, filename);
		return;
    } else if (num != NUM_STRINGS) {
		nlog(LOG_WARNING, "Warning: Bad number of strings (%d, wanted %d) for language %d (%s)", num, NUM_STRINGS, index, filename);
    }
    langtexts[index] = scalloc((sizeof(char *) * NUM_STRINGS+1));
    if (num > NUM_STRINGS)
		num = NUM_STRINGS;
    langtexts[index][NUM_STRINGS] = data = smalloc(size);
    if (fread(data, size, 1, f) != 1) {
		nlog(LOG_ERROR, "Failed to read language data for language %d (%s)", index, filename);
		goto fail;
    }
    for (i = 0; i < num; i++) {
		int pos;
		if (read_int32(&pos, f) < 0) {
			nlog(LOG_ERROR, "Failed to read entry %d in language %d (%s) TOC",
			i, index, filename);
			goto fail;
		}
		if (pos == -1) {
			langtexts[index][i] = NULL;
		} else {
			langtexts[index][i] = data + pos;
		}
    }
    fclose(f);
    is_loaded[index] = 1;
    return;

  fail:
    free(data);
    free(langtexts[index]);
    langtexts[index] = NULL;
    return;
}

/*************************************************************************/

/* Initialize list of lists. */

int lang_init()
{
    int i, j, n = 0;

    for (i = 0; i < arraylen(langorder); i++) {
		for (j = 0; filenames[j].num >= 0; j++) {
			if (filenames[j].num == langorder[i])
			break;
	}
	if (filenames[j].num >= 0)
	    load_lang(langorder[i], filenames[j].filename);
	else
	    nlog(LOG_ERROR, "BUG: lang_init(): no filename entry for language %d!",
		langorder[i]);
    }
    langmap = scalloc((sizeof(int)* NUM_STRINGS));
    for (i = 0; i < NUM_STRINGS; i++)
		langmap[i] = i;

    if (!langtexts[DEF_LANGUAGE]) {
		nlog(LOG_ERROR, "Unable to load default language");
		return 0;
    }

    for (i = 0; i < arraylen(langorder); i++) {
		if (langtexts[langorder[i]] != NULL) {
			for (j = 0; j < NUM_STRINGS; j++) {
			if (!langtexts[langorder[i]][j])
				langtexts[langorder[i]][j] = langtexts[DEF_LANGUAGE][j];
			if (!langtexts[langorder[i]][j])
				langtexts[langorder[i]][j] = langtexts[LANG_EN][j];
			}
			langlist[n++] = langorder[i];
		}
    }
    while (n < arraylen(langlist))
		langlist[n++] = -1;

    for (i = 0; i < NUM_LANGS; i++) {
		if (!langtexts[i])
			langtexts[i] = langtexts[DEF_LANGUAGE];
    }

    return 1;
}

/*************************************************************************/

/* Clean up language data. */

void lang_cleanup(void)
{
    int i;

    /* First clear out any languages borrowing from the default language */
    for (i = 0; i < NUM_LANGS; i++) {
		if (i != DEF_LANGUAGE && langtexts[i] == langtexts[DEF_LANGUAGE])
			langtexts[DEF_LANGUAGE] = NULL;
    }

    /* Now free everything left */
    for (i = 0; i < NUM_LANGS; i++) {
		if (langtexts[i]) {
			free(langtexts[i][NUM_STRINGS]);
			free(langtexts[i]);
			langtexts[i] = NULL;
		}
    }
    free(langmap);
}

/*************************************************************************/
/*************************************************************************/

/* Return true if the given language is loaded, false otherwise. */

int have_language(int language)
{
    return language >= 0 && language < NUM_LANGS && is_loaded[language];
}

/*************************************************************************/

/* Retrieve a message text using the language selected */

const char *getstring(int language, int index)
{
    if (index < 0 || index >= NUM_STRINGS) {
		nlog(LOG_ERROR, "getstring(): BUG: index (%d) out of range!", index);
		return NULL;
    }
    return langtexts[language][langmap[index]];
}

/*************************************************************************/

/* Retrieve a message text using the given language. */

const char *getstring_lang(int language, int index)
{
    if (language < 0 || language >= NUM_LANGS) {
		nlog(LOG_ERROR, "getstring_lang(): BUG: language (%d) out of range!", language);
		language = DEF_LANGUAGE;
    } else if (index < 0 || index >= NUM_STRINGS) {
		nlog(LOG_ERROR, "getstring(): BUG: index (%d) out of range!", index);
		return NULL;
    }
    return langtexts[language][langmap[index]];
}

/*************************************************************************/

/* Set string number `old' to number `new' and return the number of the
 * message that used to be stored there.
 */

int setstring(int old, int new)
{
    int prev = langmap[old];
    langmap[old] = langmap[new];
    return prev;
}

/*************************************************************************/

/* Format a string in a strftime()-like way, but heed the nick group's
 * language setting for month and day names and adjust the time for the
 * nick group's time zone setting.  The string stored in the buffer will
 * always be null-terminated, even if the actual string was longer than the
 * buffer size.  Also note that the format parameter is a message number
 * rather than a literal string, and the time parameter is a time_t, not a
 * struct tm *.
 * Assumption: No month or day name has a length (including trailing null)
 * greater than BUFSIZE or contains the '%' character.
 */
#if 0
int strftime_lang(char *buf, int size, int language, int format, time_t time)
{
    char tmpbuf[BUFSIZE], buf2[BUFSIZE];
    char *s;
    int i, ret;
    struct tm *tm;

    strscpy(tmpbuf, langtexts[language][format], sizeof(tmpbuf));
    if (ngi_is_valid && ngi->timezone != TIMEZONE_DEFAULT) {
	time += ngi->timezone*60;
	tm = gmtime(&time);
	/* Remove "%Z" (timezone) specifiers */
	while ((s = strstr(tmpbuf, "%Z")) != NULL) {
	    char *end = s+2;
	    while (s > tmpbuf && s[-1] == ' ')
		s--;
	    strmove(s, end);
	}
    } else {
	tm = localtime(&time);
    }
    if ((s = langtexts[language][STRFTIME_DAYS_SHORT]) != NULL) {
	for (i = 0; i < tm->tm_wday; i++)
	    s += strcspn(s, "\n")+1;
	i = strcspn(s, "\n");
	strncpy(buf2, s, i);
	buf2[i] = 0;
	strnrepl(tmpbuf, sizeof(tmpbuf), "%a", buf2);
    }
    if ((s = langtexts[language][STRFTIME_DAYS_LONG]) != NULL) {
	for (i = 0; i < tm->tm_wday; i++)
	    s += strcspn(s, "\n")+1;
	i = strcspn(s, "\n");
	strncpy(buf2, s, i);
	buf2[i] = 0;
	strnrepl(tmpbuf, sizeof(tmpbuf), "%A", buf2);
    }
    if ((s = langtexts[language][STRFTIME_MONTHS_SHORT]) != NULL) {
	for (i = 0; i < tm->tm_mon; i++)
	    s += strcspn(s, "\n")+1;
	i = strcspn(s, "\n");
	strncpy(buf2, s, i);
	buf2[i] = 0;
	strnrepl(tmpbuf, sizeof(tmpbuf), "%b", buf2);
    }
    if ((s = langtexts[language][STRFTIME_MONTHS_LONG]) != NULL) {
	for (i = 0; i < tm->tm_mon; i++)
	    s += strcspn(s, "\n")+1;
	i = strcspn(s, "\n");
	strncpy(buf2, s, i);
	buf2[i] = 0;
	strnrepl(tmpbuf, sizeof(tmpbuf), "%B", buf2);
    }
    ret = strftime(buf, size, tmpbuf, tm);
    if (ret == size)
	buf[size-1] = 0;
    return ret;
}
#endif
/*************************************************************************/

/* Generates a string describing the given length of time to one unit
 * (e.g. "3 days" or "10 hours"), or two units (e.g. "5 hours 25 minutes")
 * if the MT_DUALUNIT flag is specified.  The minimum resolution is one
 * minute, unless the MT_SECONDS flag is specified; the returned time is
 * rounded up if in the minimum unit, else rounded to the nearest integer.
 * The returned buffer is a static buffer which will be overwritten on the
 * next call to this routine.
 *
 * The MT_* flags (passed in the `flags' parameter) are defined in
 * language.h.
 */

char *maketime(int language, time_t time, int flags)
{
	static char buf[BUFSIZE];
#if 0
    int unit;

    if (time < 1)  /* Enforce a minimum of one second */
	time = 1;

    if ((flags & MT_SECONDS) && time <= 59) {

	unit = (time==1 ? STR_SECOND : STR_SECONDS);
	snprintf(buf, sizeof(buf), "%ld%s", (long)time, getstring(ngi,unit));

    } else if (!(flags & MT_SECONDS) && time <= 59*60) {

	time = (time+59) / 60;
	unit = (time==1 ? STR_MINUTE : STR_MINUTES);
	snprintf(buf, sizeof(buf), "%ld%s", (long)time, getstring(ngi,unit));

    } else if (flags & MT_DUALUNIT) {

	time_t time2;
	int unit2;

	if (time <= 59*60+59) {  /* 59 minutes, 59 seconds */
	    time2 = time % 60;
	    unit2 = (time2==1 ? STR_SECOND : STR_SECONDS);
	    time = time / 60;
	    unit = (time==1 ? STR_MINUTE : STR_MINUTES);
	} else if (time <= (23*60+59)*60+30) {  /* 23 hours, 59.5 minutes */
	    time = (time+30) / 60;
	    time2 = time % 60;
	    unit2 = (time2==1 ? STR_MINUTE : STR_MINUTES);
	    time = time / 60;
	    unit = (time==1 ? STR_HOUR : STR_HOURS);
	} else {
	    time = (time+(30*60)) / (60*60);
	    time2 = time % 24;
	    unit2 = (time2==1 ? STR_HOUR : STR_HOURS);
	    time = time / 24;
	    unit = (time==1 ? STR_DAY : STR_DAYS);
	}
	if (time2)
	    snprintf(buf, sizeof(buf), "%ld%s%s%ld%s", (long)time,
		     getstring(language,unit), getstring(language,STR_TIMESEP),
		     (long)time2, getstring(language,unit2));
	else
	    snprintf(buf, sizeof(buf), "%ld%s", (long)time,
		     getstring(ngi,unit));

    } else {  /* single unit */

	if (time <= 59*60+30) {  /* 59 min 30 sec; MT_SECONDS known true */
	    time = (time+30) / 60;
	    unit = (time==1 ? STR_MINUTE : STR_MINUTES);
	} else if (time <= (23*60+30)*60) {  /* 23 hours, 30 minutes */
	    time = (time+(30*60)) / (60*60);
	    unit = (time==1 ? STR_HOUR : STR_HOURS);
	} else {
	    time = (time+(12*60*60)) / (24*60*60);
	    unit = (time==1 ? STR_DAY : STR_DAYS);
	}
	snprintf(buf, sizeof(buf), "%ld%s", (long)time, getstring(language,unit));

    }
#endif
    return buf;
}

/*************************************************************************/

/* Generates a description for the given expiration time in the form of
 * days, hours, minutes, seconds and/or a combination thereof.  May also
 * return "does not expire" or "expires soon" messages if the expiration
 * time given is zero or earlier than the current time, respectively.
 * String is truncated if it would exceed `size' bytes (including trailing
 * null byte).
 */

void expires_in_lang(char *buf, int size, int language, time_t expires)
{
#if 0
	time_t seconds = expires - time(NULL);

    if (expires == 0) {
	strscpy(buf, getstring(ngi,EXPIRES_NONE), size);
    } else if (seconds <= 0) {
	strscpy(buf, getstring(ngi,EXPIRES_SOON), size);
    } else {
	snprintf(buf, size, getstring(ngi,EXPIRES_IN),
		 maketime(ngi,seconds,MT_DUALUNIT));
    }
#endif
}

/*************************************************************************/
/*************************************************************************/

/* Send a syntax-error message to the user. */

void syntax_error(const char *service, const User *u, const char *command, int msgnum)
{
#if 0
	char buf[BUFSIZE];
    snprintf(buf, sizeof(buf), getstring(u->ngi, msgnum), command);
    notice_lang(service, u, SYNTAX_ERROR, buf);
    notice_lang(service, u, MORE_INFO, service, command);
#endif
}

/*************************************************************************/
