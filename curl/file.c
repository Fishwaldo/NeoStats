/***************************************************************************
 *                                  _   _ ____  _     
 *  Project                     ___| | | |  _ \| |    
 *                             / __| | | | |_) | |    
 *                            | (__| |_| |  _ <| |___ 
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2003, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 * 
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: file.c,v 1.36 2003/10/30 09:13:04 bagder Exp $
 ***************************************************************************/

#include "setup.h"

#ifndef CURL_DISABLE_FILE
/* -- WIN32 approved -- */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>

#if defined(WIN32) && !defined(__GNUC__) || defined(__MINGW32__)
#include <time.h>
#include <io.h>
#include <fcntl.h>
#else
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <sys/time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#include <sys/ioctl.h>
#include <signal.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif


#endif

#include "curl.h"
#include "urldata.h"
#include "progress.h"
#include "sendf.h"
#include "escape.h"

#define _MPRINTF_REPLACE /* use our functions only */
#include "mprintf.h"

/* The last #include file should be: */
#ifdef CURLDEBUG
#include "memdebug.h"
#endif

/* Emulate a connect-then-transfer protocol. We connect to the file here */
CURLcode Curl_file_connect(struct connectdata *conn)
{
  char *real_path = curl_unescape(conn->path, 0);
  struct FILE *file;
  int fd;
#if defined(WIN32) || defined(__EMX__)
  int i;
  char *actual_path;
#endif

  file = (struct FILE *)malloc(sizeof(struct FILE));
  if(!file)
    return CURLE_OUT_OF_MEMORY;

  memset(file, 0, sizeof(struct FILE));
  conn->proto.file = file;

#if defined(WIN32) || defined(__EMX__)
  /* If the first character is a slash, and there's
     something that looks like a drive at the beginning of
     the path, skip the slash.  If we remove the initial
     slash in all cases, paths without drive letters end up
     relative to the current directory which isn't how
     browsers work.

     Some browsers accept | instead of : as the drive letter
     separator, so we do too.

     On other platforms, we need the slash to indicate an
     absolute pathname.  On Windows, absolute paths start
     with a drive letter.
  */
  actual_path = real_path;
  if ((actual_path[0] == '/') &&
      actual_path[1] &&
      (actual_path[2] == ':' || actual_path[2] == '|'))
  {
    actual_path[2] = ':';
    actual_path++;
  }

  /* change path separators from '/' to '\\' for Windows and OS/2 */
  for (i=0; actual_path[i] != '\0'; ++i)
    if (actual_path[i] == '/')
      actual_path[i] = '\\';

  fd = open(actual_path, O_RDONLY | O_BINARY);	/* no CR/LF translation! */
#else
  fd = open(real_path, O_RDONLY);
#endif
  free(real_path);

  if(fd == -1) {
    failf(conn->data, "Couldn't open file %s", conn->path);
    return CURLE_FILE_COULDNT_READ_FILE;
  }
  file->fd = fd;

  return CURLE_OK;
}

/* This is the do-phase, separated from the connect-phase above */

CURLcode Curl_file(struct connectdata *conn)
{
  /* This implementation ignores the host name in conformance with 
     RFC 1738. Only local files (reachable via the standard file system)
     are supported. This means that files on remotely mounted directories
     (via NFS, Samba, NT sharing) can be accessed through a file:// URL
  */
  CURLcode res = CURLE_OK;
  struct stat statbuf;
  unsigned long expected_size=0;
  bool fstated=FALSE;
  ssize_t nread;
  struct SessionHandle *data = conn->data;
  char *buf = data->state.buffer;
  int bytecount = 0;
  struct timeval start = Curl_tvnow();
  struct timeval now = start;
  int fd;

  /* get the fd from the connection phase */
  fd = conn->proto.file->fd;

/*VMS?? -- This only works reliable for STREAMLF files */
  if( -1 != fstat(fd, &statbuf)) {
    /* we could stat it, then read out the size */
    expected_size = statbuf.st_size;
    fstated = TRUE;
  }

  /* If we have selected NOBODY and HEADER, it means that we only want file
     information. Which for FILE can't be much more than the file size and
     date. */
  if(data->set.no_body && data->set.include_header && fstated) {
    CURLcode result;
    sprintf(buf, "Content-Length: %lu\r\n", expected_size);
    result = Curl_client_write(data, CLIENTWRITE_BOTH, buf, 0);
    if(result)
      return result;

    sprintf(buf, "Accept-ranges: bytes\r\n");
    result = Curl_client_write(data, CLIENTWRITE_BOTH, buf, 0);
    if(result)
      return result;

#ifdef HAVE_STRFTIME
    if(fstated) {
      struct tm *tm;
#ifdef HAVE_GMTIME_R
      struct tm buffer;
      tm = (struct tm *)gmtime_r((time_t *)&statbuf.st_mtime, &buffer);
#else
      tm = gmtime((time_t *)&statbuf.st_mtime);
#endif
      /* format: "Tue, 15 Nov 1994 12:45:26 GMT" */
      strftime(buf, BUFSIZE-1, "Last-Modified: %a, %d %b %Y %H:%M:%S GMT\r\n",
               tm);
      result = Curl_client_write(data, CLIENTWRITE_BOTH, buf, 0);
    }
#endif
    return result;
  }

  /* Added by Dolbneff A.V & Spiridonoff A.V */
  if (conn->resume_from <= (long)expected_size)
    expected_size -= conn->resume_from;
  else
    /* Is this error code suitable in such situation? */
    return CURLE_FTP_BAD_DOWNLOAD_RESUME;

  if (fstated && (expected_size == 0))
    return CURLE_OK;

  /* The following is a shortcut implementation of file reading
     this is both more efficient than the former call to download() and
     it avoids problems with select() and recv() on file descriptors
     in Winsock */
  if(fstated)
    Curl_pgrsSetDownloadSize(data, (double)expected_size);

  if(conn->resume_from)
    /* Added by Dolbneff A.V & Spiridonoff A.V */
    lseek(fd, conn->resume_from, SEEK_SET);

  while (res == CURLE_OK) {
    nread = read(fd, buf, BUFSIZE-1);

    if ( nread > 0)
      buf[nread] = 0;

    if (nread <= 0)
      break;

    bytecount += nread;
    /* NOTE: The following call to fwrite does CR/LF translation on
       Windows systems if the target is stdout. Use -O or -o parameters
       to prevent CR/LF translation (this then goes to a binary mode
       file descriptor). */

    res = Curl_client_write(data, CLIENTWRITE_BODY, buf, nread);
    if(res)
      return res;

    now = Curl_tvnow();
    if(Curl_pgrsUpdate(conn))
      res = CURLE_ABORTED_BY_CALLBACK;
  }
  now = Curl_tvnow();
  if(Curl_pgrsUpdate(conn))
    res = CURLE_ABORTED_BY_CALLBACK;

  close(fd);

  return res;
}
#endif
