/************************************************************************
 *   IRC - Internet Relay Chat, Win32GUI.c
 *   Copyright (C) 2000-2003 David Flynn (DrBin) & Dominick Meglio (codemastr)
 *   
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <process.h>

#include "resource.h"

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
		case WM_HSCROLL:
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;
		case WM_CLOSE:
			DestroyWindow (hwnd);
			return TRUE;
    }
    return FALSE;
}

HINSTANCE hInstance = 0;

extern int neostats ();
HANDLE hNeoStatsThread = 0;
extern void read_loop ();


int WINAPI WinMain
   (HINSTANCE hInst, HINSTANCE hPrevInst, char * cmdParam, int cmdShow)
{
    DWORD dwThreadId, dwThrdParam = 1; 
    char szMsg[80];
	WSADATA WSAData;
    MSG  msg;
    HWND hDialog = 0;

	hInstance = hInst;
	if (WSAStartup(MAKEWORD(1, 1), &WSAData) != 0)
   	{
        MessageBox(NULL, "Unable to initialize WinSock", "NeoStats for Windows Error", MB_OK);
        return FALSE;
	}
	hDialog = CreateDialog (hInst, 
                            MAKEINTRESOURCE (IDD_DIALOG1), 
                            0, 
                            DialogProc);
    if (!hDialog)
    {
        wsprintf (szMsg, "Error x%x", GetLastError ());
        MessageBox (0, szMsg, "CreateDialog", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

	if(neostats ()!=0)
	{
        wsprintf (szMsg, "NeoStats init failed");
        MessageBox (0, szMsg, "CreateDialog", MB_ICONEXCLAMATION | MB_OK);
        return 1;
	}

    hNeoStatsThread = CreateThread( 
        NULL,                        // default security attributes 
        0,                           // use default stack size  
        (LPTHREAD_START_ROUTINE)read_loop,                  // thread function 
        &dwThrdParam,                // argument to thread function 
        0,                           // use default creation flags 
        &dwThreadId);                // returns the thread identifier 
 
   // Check the return value for success. 
 
   if (hNeoStatsThread == NULL) 
   {
      wsprintf( szMsg, "CreateThread failed." ); 
      MessageBox( NULL, szMsg, "main", MB_OK );
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
