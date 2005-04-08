
/*
* adns_unix_calls.c
* - Simple implementation of requiered UNIX system calls and
*   library functions.
*/
/*
*  This file is
*    Copyright (C) 2000, 2004 Jarle (jgaa) Aase <jgaa@jgaa.com>
*
*  It is part of adns, which is
*    Copyright (C) 1997-2000 Ian Jackson <ian@davenant.greenend.org.uk>
*    Copyright (C) 1999 Tony Finch <dot@dotat.at>
*  
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*  
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software Foundation,
*  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
*/

#include "adnsinternal.h"

int adns_writev(int FileDescriptor, const struct iovec * iov, int iovCount)
{
	size_t total_len = 0;
	int bytes_written = 0;
	int i = 0, r = 0;
	char *buf = NULL, *p = NULL;
	
	for(; i < iovCount; i++)
		total_len += iov[i].iov_len;
	
	p = buf = (char *)alloca(total_len);
	
	for(; i < iovCount; i++)
	{
		os_memcpy(p, iov[i].iov_base, iov[i].iov_len);
		p += iov[i].iov_len;
	}
	
	ADNS_CLEAR_ERRNO
	r = send(FileDescriptor, buf, total_len, 0);
	ADNS_CAPTURE_ERRNO;
	return r;
}

int adns_getpid()
{
	return GetCurrentProcessId();
}
