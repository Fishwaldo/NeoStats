/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2004 Adam Rutter, Justin Hammond, Mark Hetherington
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

/* @file Portability wrapper functions
 */

#include "neostats.h"
#ifdef HAVE_SYS_POLL_H 
#include <poll.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h> 
#endif

static char tempbuf[BUFSIZE*2];

int sys_mkdir (const char *filename, mode_t mode)
{
#ifdef WIN32
	return mkdir (filename);
#else
	return mkdir (filename, mode);
#endif
}

int sys_check_create_dir (const char* dirname)
{
	struct stat st;
	int res;

	/* first, make sure the logdir dir exists */
	res = stat(dirname, &st);
	if (res != 0) {
		/* hrm, error */
		if (errno == ENOENT) {
			/* ok, it doesn't exist, create it */
			res = sys_mkdir (dirname, 0700);
			if (res != 0) {
				/* error */
				nlog (LOG_CRITICAL, "Couldn't create directory: %s", strerror(errno));
				return NS_FAILURE;
			}
			nlog (LOG_NOTICE, "Created directory: %s", dirname);
		} else {
			nlog (LOG_CRITICAL, "Stat returned error: %s", strerror(errno));
			return NS_FAILURE;
		}
	} else if (!S_ISDIR(st.st_mode))	{
		nlog (LOG_CRITICAL, "%s is not a Directory", dirname);
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

FILE_HANDLE sys_file_open (const char * filename, int filemode)
{
	switch (filemode) 
	{ 
		case FILE_MODE_APPEND:
			return fopen (filename, "a");
			break;
		case FILE_MODE_READ:
			return fopen (filename, "r");
			break;
		default:
			break;
	}
	return fopen (filename, "r");
}

int sys_file_close (FILE_HANDLE handle)
{
	return fclose (handle);
}

int sys_file_seek (FILE_HANDLE handle, long offset, int origin)
{
	return fseek (handle, offset, origin);
}

long sys_file_tell (FILE_HANDLE handle)
{
	return ftell(handle);
}

int sys_file_printf (FILE_HANDLE handle, char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	ircvsnprintf (tempbuf, BUFSIZE*2, fmt, ap);
	va_end (ap);
	return fprintf (handle, "%s", tempbuf);	
}

int sys_file_read (void *buffer, size_t size, size_t count, FILE_HANDLE handle)
{
	return fread (buffer, size, count, handle);
}

char* sys_file_gets (char *string, int n, FILE_HANDLE handle)
{
	return fgets (string, n, handle);
}
int sys_file_write (const void *buffer, size_t size, size_t count, FILE_HANDLE handle)
{
	return fwrite (buffer, size, count, handle);
}

int sys_file_flush (FILE_HANDLE handle)
{
	return fflush (handle);
}

int sys_file_rename (const char* oldname, const char* newname)
{
	/*  WIN32 does not allow rename if the file exists 
	 *  Behaviour is undefined on various systems so maybe
	 *  we should remove on all platforms?
	 */
#ifdef WIN32
	remove (newname);
#endif
	return rename (oldname, newname);
}

char* sys_file_get_last_error_string (void)
{
	return strerror (errno);
}

int sys_file_get_last_error (void)
{
	return errno;
}

int sys_file_get_size (const char* filename)
{
	struct stat st;
	int res;

	res = stat(filename, &st);
	if (res != 0) {
		if (sys_file_get_last_error () == ENOENT) {
			/* wtf, this is bad */
			nlog (LOG_CRITICAL, "No such file: %s", filename);
			return -1;
		} else {
			nlog (LOG_CRITICAL, "File error: %s", sys_file_get_last_error_string ());
			return -1;
		}
	}
	return st.st_size;
}

int sys_sock_close (SYS_SOCKET sock)
{
#ifdef WIN32
	return closesocket (sock);
#else
	return close (sock);
#endif      			
}

int sys_sock_write (SYS_SOCKET s, const char* buf, int len)
{
#ifdef WIN32
	return send (s, buf, len, 0);
#else	
	return write (s, buf, len);
#endif
}

int sys_sock_read (SYS_SOCKET s, char* buf, int len)
{
#ifdef WIN32
	return recv (s, buf, len, 0);
#else
	return read (s, buf, len);
#endif
}

int sys_sock_set_nonblocking (SYS_SOCKET s)
{
	int flags;
#ifdef WIN32

	flags = 1;
	return ioctlsocket(s, FIONBIO, &flags);
#else
	flags = fcntl(s, F_GETFL, 0);
	flags |= O_NONBLOCK;
	return fcntl (s, F_SETFL, flags);
#endif
}

size_t sys_strftime (char *strDest, size_t maxsize, const char *format, const struct tm *timeptr)
{
	return strftime (strDest, maxsize, format, timeptr);
}

struct tm* sys_localtime (const time_t *timer)
{
	return localtime (timer);
}

