/* LOGIN_DELAY: seconds till login process times out
   KEEP_ALIVE_DELAY: how often icqtech should send the keepalive message
      to the ICQ server
   SRV_TO_USE: ICQ server to connect to
   SRV_PORT: Port of ICQ server

   MAX_MSG_LENGTH: Maximum length of messages (should not change)
   MSG_TIMEOUT: Time until user is erased from queue */
   
#define LOGIN_DELAY      30  /* seconds */
#define KEEP_ALIVE_DELAY 120 /* seconds */
#define MAX_MSG_LENGTH 450
#define MSG_TIMEOUT 120 /* in seconds */


/* icq_help.c */
const char *icq_help[];