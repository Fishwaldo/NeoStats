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

#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include "resource.h"

extern int neostats ();
extern void read_loop ();

static char* szNeoStatsErrorTitle="NeoStats for Windows Error";
HINSTANCE hInstance = 0;
HANDLE hNeoStatsThread = 0;

#ifndef NDEBUG
void InitDebugConsole(void)
{
	long lStdHandle;
	int hConHandle;
	FILE *fp;

	AllocConsole ();        
	SetConsoleTitle ("NeoStats Debug Window");
	lStdHandle = (long)GetStdHandle (STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle (lStdHandle, _O_TEXT);
	fp = _fdopen ( hConHandle, "w" );
	*stdout = *fp;
	setvbuf (stdout, NULL, _IONBF, 0 );
	lStdHandle = (long)GetStdHandle (STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle (lStdHandle, _O_TEXT);
	fp = _fdopen ( hConHandle, "w" );
	*stderr = *fp;
	setvbuf (stderr, NULL, _IONBF, 0 );
}

void FiniDebugConsole(void)
{
	FreeConsole();
}
#endif

void ErrorMessageBox(char* error)
{
	MessageBox (NULL, error, szNeoStatsErrorTitle, MB_ICONEXCLAMATION | MB_OK);
}

BOOL CALLBACK DialogProc (HWND hwnd, 
                          UINT message, 
                          WPARAM wParam, 
                          LPARAM lParam)
{
    switch (message)
    {
		case WM_INITDIALOG:
			return TRUE;
		case WM_COMMAND:
			return TRUE;
		case WM_DESTROY:
			PostQuitMessage (0);
			return TRUE;
		case WM_CLOSE:
			if (MessageBox (hwnd, "Close NeoStats?", "Are you sure?", MB_YESNO|MB_ICONQUESTION) == IDNO)
			{
				return 0;
			}
			else 
			{
#ifndef NDEBUG
				FiniDebugConsole ();
#endif
				DestroyWindow (hwnd);				
				TerminateThread (hNeoStatsThread, 0);
				do_exit (0, "Terminated");
			}
			return TRUE;
    }
    return FALSE;
}

int WINAPI WinMain
   (HINSTANCE hInst, HINSTANCE hPrevInst, char * cmdParam, int cmdShow)
{
    DWORD dwThreadId, dwThrdParam = 1; 
    char szMsg[80];
	WSADATA WSAData;
    MSG  msg;
    HWND hDialog = 0;
	
	_fmode = _O_BINARY;
	hInstance = hInst;

	if (WSAStartup (MAKEWORD(1, 1), &WSAData) != 0)
   	{
        ErrorMessageBox ("Unable to initialize WinSock");
        return FALSE;
	}
#ifndef NDEBUG
	InitDebugConsole ();
#endif
	hDialog = CreateDialog (hInst, 
                            MAKEINTRESOURCE (IDD_DIALOG1), 
                            0, 
                            DialogProc);
    if (!hDialog)
    {
        wsprintf (szMsg, "Error x%x", GetLastError ());
        ErrorMessageBox (szMsg);
        return FALSE;
    }

	if(neostats ()!=0)
	{
        ErrorMessageBox ("NeoStats init failed");
        return FALSE;
	}

    hNeoStatsThread = CreateThread ( 
        NULL,                        // default security attributes 
        0,                           // use default stack size  
        (LPTHREAD_START_ROUTINE)read_loop,                  // thread function 
        &dwThrdParam,                // argument to thread function 
        0,                           // use default creation flags 
        &dwThreadId);                // returns the thread identifier 
 
   // Check the return value for success. 
 
   if (hNeoStatsThread == NULL) 
   {	
	   ErrorMessageBox ("CreateThread failed.");
   }

    while ((GetMessage (&msg, NULL, 0, 0)) != 0)
    {
        if (!IsDialogMessage (hDialog, & msg))
        {
            TranslateMessage ( & msg );
            DispatchMessage ( & msg );
        }
    }
    return msg.wParam;
}
