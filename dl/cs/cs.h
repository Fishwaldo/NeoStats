/* NeoStats - IRC Statistical Services Copryight (c) 1999-2002 NeoStats Group.
*
** Module: ConnectServ
** Description: Network Connection & Mode Monitoring Service
** Version: 1.3
** Authors: ^Enigma^ & Shmad
**
** Flags for IRCd's mode types.
**
*/


/* 
** If we're compiled for Ultimate 3.x.x IRCd, use these modes and flags
*/
#ifdef ULTIMATE3
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define GUESTADMIN_MODE 'j'
  #define COSERVERADMIN_MODE 'J'
  #define SERVERADMIN_MODE 'A'
  #define CONETADMIN_MODE 't'
  #define NETADMIN_MODE 'T'
  #define TECHADMIN_MODE '7'		/* Set to a number as we dont use */
  #define SERVICESADMIN_MODE 'P'
  #define NETSERVICE_MODE 'S'
  #define INVISIBLE_MODE '8'		/* Set to a number as we dont use */
  #define BOT_MODE '9'			/* Set to a number as we dont use */
#elif ULTIMATE
/*
** If we are compiled to Use Ultimate 2.8.x use these modes and flags
*/
  #define LOCOP_MODE 'O'
  #define OPER_MODE, 'o'
  #define GUESTADMIN_MODE '7'		/* Set to a number as we dont use */
  #define COSERVERADMIN_MODE 'J'
  #define SERVERADMIN_MODE 'A'
  #define CONETADMIN_MODE 't'
  #define NETADMIN_MODE 'N'
  #define TECHADMIN_MODE 'T'
  #define SERVICESADMIN_MODE 'P'
  #define NETSERVICE_MODE 'S'
  #define INVISIBLE_MODE '8'            /* Set to a number as we dont use */
  #define BOT_MODE 'B'
#endif

#ifdef UNREAL
  #define LOCOP_MODE 'O'
  #define OPER_MODE 'o'
  #define GUESTADMIN_MODE '7'		/* Set to a number we dont use */
  #define COSERVERADMIN_MODE 'C'
  #define SERVERADMIN_MODE 'A'
  #define CONETADMIN_MODE '8'		/* Set to a number we dont use */
  #define NETADMIN_MODE 'N'
  #define TECHADMIN_MODE 'T'
  #define SERVICESADMIN_MODE 'A'
  #define NETSERVICE_MODE 'S'
  #define INVISIBLE_MODE 'I'
  #define BOT_MODE 'B'
#endif
