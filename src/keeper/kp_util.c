/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond
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
** $Id$
*/
/*
 * KEEPER: A configuration reading and writing library
 *
 * Copyright (C) 1999-2000 Miklos Szeredi
 * Email: mszeredi@inf.bme.hu
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

#include "kp_util.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

/* Initialization is done */
static volatile int kp_inited = 0;

/* Only the local database is initialized */
static volatile int kp_local_inited = 0;

/* Index for the different sections */
enum {
	KPDB_GLOBAL,
	KPDB_LOCAL,
	KPDB_USER,

	KPDB_NUMDBS
};

/* String containing the temporary file name (not the path) */
static char *kp_tmpname = NULL;

/* String array containing the base path of the different sections */
static char *kp_basedirs[KPDB_NUMDBS];

#define GLOBALDIR "data/kpconf"	/* Default global database path */
#define LOCALDIR  "data/kpdata"	/* Local database path */
#define USERSUBDIR "kplang"	/* Default user database dir */
#define LOCKFILE ":lock:"	/* Lock file name */

/* ------------------------------------------------------------------------- 
 * Initialize the kp_tmpname variable with a unique hostname/pid string
 * ------------------------------------------------------------------------- */
static void kp_init_tmpname()
{
	const char *hostname;
	unsigned int pid;

	hostname = getenv("HOST");
	if (hostname == NULL)
		hostname = "localhost";

	pid = getpid();

	kp_tmpname = (char *) malloc_check(strlen(hostname) + 64);
	sprintf(kp_tmpname, ":tmp.%s.%u:", hostname, pid);
}

/* ------------------------------------------------------------------------- 
 * Create the local database dir
 * ------------------------------------------------------------------------- */
static char *kp_init_localdb(void)
{
	char *basedir;

	basedir = (char *) malloc_check(strlen(LOCALDIR) + 2);
	sprintf(basedir, "%s/", LOCALDIR);
	mkdir(basedir, 0755);

	return basedir;
}

/* ------------------------------------------------------------------------- 
 * Get the location of the user database dir, and create it
 * ------------------------------------------------------------------------- */
static char *kp_init_userdb(void)
{
#if 0
	const char *homeval;
	char *userdir;
	char *basedir;
	userdir = getenv("KEEPER_USERDIR");
	if (userdir != 0) {
		basedir = (char *) malloc_check(strlen(userdir) + 2);
		sprintf(basedir, "%s/", userdir);
	} else {
		homeval = getenv("HOME");
		if (homeval == NULL)
			homeval = "";
		/* FIXME: where to find home, if $HOME is not set? */

		basedir = (char *) malloc_check(strlen(homeval) + 1 +
						strlen(USERSUBDIR) + 2);
		sprintf(basedir, "%s/%s/", homeval, USERSUBDIR);
	}
	mkdir(basedir, 0755);

	return basedir;
#endif
	return USERSUBDIR;
}

/* ------------------------------------------------------------------------- 
 * Get the location of the global database dir, and create it
 * ------------------------------------------------------------------------- */
static char *kp_init_globaldb(void)
{
	char *basedir;
	char *globaldir;

	/* Be careful. This is a recursion */
#if 0
	globaldir = NULL;
	kp_get_string("l/keeper/globaldir", &globaldir);
	if (globaldir == NULL)
#endif
		globaldir = strdup_check(GLOBALDIR);

	basedir = (char *) malloc_check(strlen(globaldir) + 2);
	sprintf(basedir, "%s/", globaldir);
	free(globaldir);

	mkdir(basedir, 0755);

	return basedir;
}

/* ------------------------------------------------------------------------- 
 * Initialize the database, if it has not yet been initialized
 * ------------------------------------------------------------------------- */
static void kp_init(int dbindex)
{
	if (kp_inited)
		return;

	/* This is because of the recursion caused by having to read the local
	   database, before the global can be initialized */
	if (kp_local_inited && dbindex == KPDB_LOCAL)
		return;

	if (!kp_inited) {
		kp_init_tmpname();
		kp_basedirs[KPDB_LOCAL] = kp_init_localdb();
		kp_local_inited = 1;
		kp_basedirs[KPDB_USER] = kp_init_userdb();
		kp_basedirs[KPDB_GLOBAL] = kp_init_globaldb();
		kp_inited = 1;
	}
}

/* ------------------------------------------------------------------------- 
 * Free everything used by the keeper library
 * ------------------------------------------------------------------------- */
void kp_exit()
{
	free(kp_tmpname);
	free(kp_basedirs[KPDB_LOCAL]);

	_kp_clear_cache();
}

#define READ_BUF_SIZE 1024

/* ------------------------------------------------------------------------- 
 * Read a line of arbitary length. On end of file return NULL
 * ------------------------------------------------------------------------- */
static char *kp_read_line(FILE * fp)
{
	char buf[READ_BUF_SIZE];
	char *line = NULL;
	int eol;
	unsigned int linelen, buflen;

	eol = 0;
	linelen = 0;
	do {
		if (fgets(buf, READ_BUF_SIZE, fp) == NULL)
			return line;

		buflen = strlen(buf);
		if (buflen > 0 && buf[buflen - 1] == '\n') {
			buf[buflen - 1] = '\0';
			buflen--;
			eol = 1;
		}

		line = (char *)
		    check_ptr(realloc(line, linelen + buflen + 1));

		strcpy(line + linelen, buf);
		linelen += buflen;
	} while (!eol);

	return line;
}

/* ------------------------------------------------------------------------- 
 * Get an entry in the database file, the key name is the return value,
 * and the value of the key  is returned in *valuep. On end of file it
 * returns NULL
 * ------------------------------------------------------------------------- */
char *_kp_get_line(FILE * fp, char **valuep)
{
	char *buf, *s;
	int found;

	found = 0;
	do {
		buf = kp_read_line(fp);
		if (buf == NULL)
			return NULL;

		for (s = buf; *s && *s != SEP1; s++);
		if (*s)
			found = 1;
		else
			free(buf);
	} while (!found);

	*s = '\0';
	*valuep = s + 1;

	return buf;
}

/* ------------------------------------------------------------------------- 
 * Split the key path into a section, a file path, and a key name.
 * If the path contained a colon, *iskeyfile is set to 1
 * ------------------------------------------------------------------------- */
int _kp_get_path(const char *keypath, kp_path * kpp, char **keynamep,
		 int *iskeyfile)
{
	char *path;
	char *basedir;
	int dbindex;
	int ibeg;
	int i;

	switch (keypath[0]) {
	case 'g':
		dbindex = KPDB_GLOBAL;
		break;

	case 'l':
		dbindex = KPDB_LOCAL;
		break;

	case 'u':
		dbindex = KPDB_USER;
		break;

	default:
		return KPERR_NOKEY;
	}

	/* Skip over the rest of the database selection chars */
	keypath++;
	while (*keypath && *keypath != '/')
		keypath++;

	if (*keypath)
		keypath++;

	kp_init(dbindex);

	basedir = kp_basedirs[dbindex];
	kpp->dbindex = dbindex;

	ibeg = strlen(basedir);

	path = (char *) malloc_check(ibeg + strlen(keypath) + 1);
	sprintf(path, "%s%s", basedir, keypath);

	i = ibeg;
	for (; path[i]; i++) {
		if (path[i] == ':') {
			if (path[i + 1] == '/')
				*keynamep = &path[i + 2];
			else
				*keynamep = &path[i + 1];

			path[i] = '\0';
			kpp->path = path;
			if (iskeyfile != NULL)
				*iskeyfile = 1;
			return 0;
		}
	}

	kpp->path = path;
	*keynamep = &path[i];
	if (iskeyfile != NULL)
		*iskeyfile = 0;

	return 0;
}

/* ------------------------------------------------------------------------- 
 * Lock a section of the database for reading or writing. While the given
 * section is locked for write no other process can modify or read the
 * database
 * ------------------------------------------------------------------------- */
int _kp_lock_file(int dbindex, int iswrite)
{
#if 0				/* not needed for Neo? */
	struct flock flock;
	int res;
	char *basedir;
	char *lockfile;
	int lockfd;
	int mode;

	basedir = kp_basedirs[dbindex];
	lockfile =
	    (char *) malloc_check(strlen(basedir) + strlen(LOCKFILE) + 1);
	sprintf(lockfile, "%s%s", basedir, LOCKFILE);

	if (iswrite)
		mode = O_RDWR;
	else
		mode = O_RDONLY;

	lockfd = open(lockfile, O_CREAT | mode, 0644);
	free(lockfile);
	if (lockfd == -1) {
		/* Silently return if lockfile creation fails */
		return -1;
	}
	fcntl(lockfd, F_SETFD, FD_CLOEXEC);

	memset(&flock, 0, sizeof(flock));

	if (iswrite)
		flock.l_type = F_WRLCK;
	else
		flock.l_type = F_RDLCK;

	flock.l_whence = 0;
	flock.l_start = 0;
	flock.l_len = 0;

	do {
		res = fcntl(lockfd, F_SETLKW, &flock);
	}
	while (res == -1 && errno == EINTR);

	if (res == -1 && errno == EDEADLK) {
		fprintf(stderr, "keeper: Internal Error: Deadlock\n");
		abort();
	}

	return lockfd;
#endif
	return -1;
}

/* ------------------------------------------------------------------------- 
 * Unlock a section of the database
 * ------------------------------------------------------------------------- */
void _kp_unlock_file(int lockfd)
{
#if 0				/* not needed for Neo? */
	struct flock flock;

	memset(&flock, 0, sizeof(flock));

	flock.l_type = F_UNLCK;
	flock.l_whence = 0;
	flock.l_start = 0;
	flock.l_len = 0;

	fcntl(lockfd, F_SETLK, &flock);
	close(lockfd);
#endif
}

/* ------------------------------------------------------------------------- 
 * Return the length of the base directory for a given section
 * ------------------------------------------------------------------------- */
int _kp_get_ibeg(int dbindex)
{
	return strlen(kp_basedirs[dbindex]);
}

/* ------------------------------------------------------------------------- 
 * Return a temporary file path for a given section
 * ------------------------------------------------------------------------- */
char *_kp_get_tmpfile(int dbindex)
{
	char *basedir;
	char *filename;

	basedir = kp_basedirs[dbindex];
	filename =
	    (char *) malloc_check(strlen(basedir) + strlen(kp_tmpname) +
				  1);
	sprintf(filename, "%s%s", basedir, kp_tmpname);

	return filename;
}

/* ------------------------------------------------------------------------- 
 * Convert an errno value to a kperr_t value
 * ------------------------------------------------------------------------- */
int _kp_errno_to_kperr(int en)
{
	switch (en) {
	case ENAMETOOLONG:
	case EISDIR:
	case ENOTDIR:
	case ENOTEMPTY:
		return KPERR_BADKEY;

	case EACCES:
	case EROFS:
	case EPERM:
		return KPERR_NOACCES;

	case ENOENT:
		return KPERR_NOKEY;

	case EMFILE:
	case ENFILE:
	case ENOMEM:
	case ENOSPC:
		return KPERR_NOSPACE;

	case EFAULT:
		fprintf(stderr,
			"keeper: Internal Error: Function retured EFAULT\n");
		abort();

	default:
		return KPERR_BADDB;
	}
}

/* ------------------------------------------------------------------------- 
 * Add a key name to the key array, the array allocated to a size that will
 * hold the array and all the key names. But for now the key names are
 * allocated separate space.
 * ------------------------------------------------------------------------- */
static void kp_add_subkey(struct key_array *keys, char *name)
{
	char *namedup;
	int strsize;

	strsize = strlen(name) + 1;
	strsize = ROUND_TO(strsize, 8);
	keys->strsize += strsize;

	keys->num++;
	keys->array = (char **)
	    check_ptr(realloc(keys->array,
			      (keys->num + 1) * sizeof(char *) +
			      keys->strsize));

	namedup = strdup_check(name);

	keys->array[keys->num - 1] = namedup;
	keys->array[keys->num] = NULL;
}

/* ------------------------------------------------------------------------- 
 * Check if the key name already exists in the array, and if not add it
 * ------------------------------------------------------------------------- */
void _kp_add_subkey_check(struct key_array *keys, char *name)
{
	char **kp;
	unsigned int namelen;
	char *s;

	namelen = strlen(name);
	s = strchr(name, ':');
	if (s != NULL) {
		if (s[1] != '\0')
			return;
		namelen--;
	}

	for (kp = keys->array; *kp != NULL; kp++) {
		if (strncmp(*kp, name, namelen) == 0 &&
		    ((*kp)[namelen] == '\0' || (*kp)[namelen] == ':'))
			return;
	}
	kp_add_subkey(keys, name);
}

/* End of kp_util.c */
