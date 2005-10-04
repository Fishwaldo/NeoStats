/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2005 Adam Rutter, Justin Hammond, Mark Hetherington
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

#define	WM_USER_SHELLICON WM_USER + 1 

extern int neostats();
extern void read_loop();
extern void do_exit( int exitcode, char *quitmsg );

static char* szNeoStatsErrorTitle="NeoStats for Windows Error";
HINSTANCE hInstance = 0;
HANDLE hNeoStatsThread = 0;
HWND hDialog = 0;
HICON hIcon;
NOTIFYICONDATA nsNotifyIconData;
HMENU hTrayPopMenu;
POINT TrayPoint;

#ifndef NDEBUG
void InitDebugConsole( void )
{
	HANDLE lStdHandle;
	int hConHandle;
	FILE *fp;

	AllocConsole();        
	SetConsoleTitle( "NeoStats Debug Window" );
	lStdHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
	lStdHandle = GetStdHandle( STD_ERROR_HANDLE );
	hConHandle = _open_osfhandle( (intptr_t)lStdHandle, _O_TEXT );
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
}

void FiniDebugConsole( void )
{
	FreeConsole();
}
#endif

void ErrorMessageBox( char* error )
{
	MessageBox( NULL, error, szNeoStatsErrorTitle, MB_ICONEXCLAMATION | MB_OK );
}

INT_PTR CALLBACK DialogProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch( message )
    {
		case WM_INITDIALOG:
			return TRUE;

		case WM_USER_SHELLICON: 	
			switch( LOWORD( lParam ) ) 
			{ 
				case WM_LBUTTONDBLCLK:
					ShowWindow(hDialog, SW_NORMAL);
					return TRUE; 

				case WM_RBUTTONDOWN: 
					GetCursorPos( &TrayPoint );			
					hTrayPopMenu = CreatePopupMenu();
					InsertMenu( hTrayPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_OPEN, "&Open NeoStats");
					InsertMenu( hTrayPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_CLOSE, "&Exit NeoStats");										
					SetForegroundWindow( hDialog );
					TrackPopupMenu( hTrayPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, TrayPoint.x, TrayPoint.y, 0, hDialog, NULL);
					SendMessage( hDialog, WM_NULL, 0, 0 );			
					return TRUE; 
			} 
			break; 

		case WM_COMMAND:
			switch( LOWORD( wParam ) )
			{
				case ID_OPEN:
					ShowWindow( hDialog, SW_NORMAL ); 
					break;

				case ID_CLOSE:
					Shell_NotifyIcon( NIM_DELETE, &nsNotifyIconData );
					DestroyWindow( hDialog );
					PostQuitMessage( 0 );
					break;
			}

			return TRUE;

		case WM_DESTROY:
			PostQuitMessage( 0 );
			return TRUE;

		case WM_CLOSE:
			if( MessageBox( hwnd, "Close NeoStats?", "Are you sure?", MB_YESNO|MB_ICONQUESTION ) == IDNO )
			{
				return 0;
			}
			else 
			{
				Shell_NotifyIcon( NIM_DELETE, &nsNotifyIconData );
#ifndef NDEBUG
				FiniDebugConsole();
#endif
				DestroyWindow( hwnd );				
				TerminateThread( hNeoStatsThread, 0 );
				do_exit( 0, "Terminated" );
			}
			return TRUE;

		case WM_SYSCOMMAND: 
			
			if(SC_MINIMIZE == wParam) 
			{ 
				ShowWindow( hDialog, SW_HIDE );
				return TRUE; 
			}
			break;
    }
    return FALSE;
}

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, char * cmdParam, int cmdShow )
{
    DWORD dwThreadId, dwThrdParam = 1; 
    char szMsg[80];
	WSADATA WSAData;
    MSG msg;
	
	_fmode = _O_BINARY;
	hInstance = hInst;

	if( WSAStartup( MAKEWORD( 2, 0 ), &WSAData ) != 0 )
   	{
        ErrorMessageBox( "Unable to initialize WinSock" );
		WSACleanup();
        return FALSE;
	}
	if (LOBYTE( WSAData.wVersion ) != 2 || HIBYTE( WSAData.wVersion ) != 0 ) {
        ErrorMessageBox( "Unable to initialize WinSock, need version 2.0 or higher" );
		WSACleanup();
        return FALSE;
	}

#ifndef NDEBUG
	InitDebugConsole();
#endif
	hDialog = CreateDialog( hInst, MAKEINTRESOURCE( IDD_DIALOG1 ), 0, DialogProc );
    if( !hDialog )
    {
        wsprintf( szMsg, "Error x%x", GetLastError() );
        ErrorMessageBox( szMsg );
		WSACleanup();
        return FALSE;
    }
	if( neostats() != 0 )
	{
		ErrorMessageBox( "NeoStats init failed" );
		WSACleanup();
		return FALSE;
	}

	hIcon = LoadIcon( hInstance,( LPCTSTR )MAKEINTRESOURCE( IDI_ICON1 ) );
	nsNotifyIconData.cbSize = sizeof( NOTIFYICONDATA );
	nsNotifyIconData.hWnd = hDialog;
	nsNotifyIconData.uID = IDI_ICON1;
	nsNotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	strcpy( nsNotifyIconData.szTip, "NeoStats IRC Services 3.0" );
	nsNotifyIconData.hIcon = hIcon;
	nsNotifyIconData.uCallbackMessage = WM_USER_SHELLICON; 
	Shell_NotifyIcon( NIM_ADD, &nsNotifyIconData ); 

	hNeoStatsThread = CreateThread( NULL, 0, ( LPTHREAD_START_ROUTINE )read_loop, &dwThrdParam, 0, &dwThreadId );
 
	// Check the return value for success. 
 	if( hNeoStatsThread == NULL ) 
	{	
		ErrorMessageBox( "CreateThread failed." );
		WSACleanup();
		return FALSE;
	}

    while( ( GetMessage( &msg, NULL, 0, 0 ) ) != 0 )
    {
        if( !IsDialogMessage( hDialog, &msg ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }
	WSACleanup();
    return (int) msg.wParam;
}
