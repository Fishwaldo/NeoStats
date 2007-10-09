/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2007 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2001 ^Enigma^
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
** $Id: update.c 3198 2007-08-19 06:23:08Z Fish $
*/

/*  TODO:
 *  - Nothing at present
 */

#include "neostats.h"
#include "mrss.h"
#include "update.h"
#include "services.h"


/** local structures */
static int update_set_updateenabled_cb( const CmdParams *cmdparams, SET_REASON reason );
static int update_set_updateurl_cb( const CmdParams *cmdparams, SET_REASON reason );
static int update_check_umode( const CmdParams *cmdparams );
static int update_check_smode( const CmdParams *cmdparams );


/** Configuration structure */
static struct update_cfg 
{ 
	char updateurl[BUFSIZE];
	unsigned int enabled;
} update_cfg;

/** Copyright info */
static const char *update_copyright[] = 
{
	"Copyright (c) 1999-2007, NeoStats",
	"http://www.neostats.net/",
	NULL
};

static const char *update_about[] = 
{
	"Automatically notify operators of New Versions"
	"of NeoStats and its Modules",
	NULL
};

/** Module info */
ModuleInfo module_info = 
{
	"Update",
	"Update Notification Module",
	update_copyright,
	update_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	0,
};

/** Bot setting table */
static bot_setting update_settings[] =
{
	{"UPDATEURL",		&update_cfg.updateurl,	SET_TYPE_STRING,	0, BUFSIZE, 	NS_ULEVEL_ADMIN, NULL,	update_help_set_updateurl, update_set_updateurl_cb, ( void *)"http://localhost/~justin/download/feed.php" },
	{"UPDATEENABLED",	&update_cfg.enabled,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	update_help_set_updateenabled, update_set_updateenabled_cb, ( void* )1 },
	NS_SETTING_END()
};

ModuleEvent module_events[] = 
{
	{EVENT_UMODE,		update_check_umode,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SMODE,		update_check_smode,		EVENT_FLAG_EXCLUDE_ME},
	NS_EVENT_END()
};

unsigned int string_to_ver(const char *str)
{
    static char lookup[] = "abcdef0123456789";
    unsigned int maj = 0, min = 0, rev = 0, ver;
    unsigned char *cptr, *idx;
    int bits;

    // do the major number
    cptr = (unsigned char *)str;
    for (; *cptr; cptr++)
    {
	if (*cptr == '.' || *cptr == '_')
	{
	    cptr++;
	    break;
	}
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;
	
	maj = (maj << 4) | ((char *)idx - lookup);
    }
    
    // do the minor number
    for (bits = 2; *cptr && *cptr != '.' && *cptr != '_' && bits > 0; cptr++)
    {
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;
	
	min = (min << 4) | ((char *)idx - lookup);
	bits--;
    }
    
    // do the revision number
    for (bits = 4; *cptr && bits > 0; cptr++)
    {
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;

	rev = (rev << 4) | ((char *)idx - lookup);
	bits--;
    }

    ver = (maj << 24) | (min << 16) | (rev << (4*bits));

    return ver;
}
static int BuildMods( Module *mod_ptr, void *v) {
	hash_t *loadedmods = (hash_t *)v;
	hnode_create_insert(loadedmods, mod_ptr, mod_ptr->info->name);
	return NS_FALSE;
}

static void CheckModVersions(mrss_item_t *item, Module *mod) {
	mrss_tag_t *othertags;
	char **av;
	int ac = 0;
	othertags = item->other_tags;
	while (othertags) {
		if (!ircstrcasecmp(othertags->name, "Version")) {
			if (mod) {
				/* av[0] - Main Version av[2] - SVN revison if ac > 1 */
				ac = split_buf((char *)mod->info->version, &av);
				irc_chanalert(ns_botptr, "%x %x", string_to_ver(othertags->value), string_to_ver(av[0]));
			} else {
				ac = split_buf(me.version, &av);
				irc_chanalert(ns_botptr, "%x(%s) %x(%s)", string_to_ver(othertags->value), othertags->value, string_to_ver(av[0]), av[0]);
			}
			ns_free(av);
		}			
		othertags = othertags->next;
	}
	irc_chanalert(ns_botptr, "%s", item->title);
	irc_chanalert(ns_botptr, "%s", item->description);
	irc_chanalert(ns_botptr, "%s", item->link);
}


static void UpdateRSSHandler(void *userptr, int status, char *data, int datasize)
{
	mrss_error_t ret;
	mrss_t *mrss;
	mrss_item_t *item;
	mrss_tag_t *othertags;
	hash_t *loadedmods;
	hnode_t *node;
	Module *mod;
	SET_SEGV_LOCATION();


	if (status != NS_SUCCESS) {
		nlog(LOG_WARNING, "RSS Update Feed download failed: %s", data);
		return;
	}
	mrss = NULL;
	mrss_new(&mrss);
	ret = mrss_parse_buffer(data, datasize, &mrss);
	if (ret != MRSS_OK) {
		nlog(LOG_WARNING, "RSS Update Feed Parse failed: %s", mrss_strerror(ret));
		mrss_free(mrss);
		return;
	}

	/* build a hash of modules 
	 * we do this every check rather than at Init/Sych time
	 * as we might have loaded/unloaded modules 
	 */
	loadedmods = hash_create(HASHCOUNT_T_MAX, 0, 0);
	ProcessModuleList(BuildMods, loadedmods);

	item = mrss->item;
	while (item)
	{
		othertags = item->other_tags;
		while (othertags) {
			if (!ircstrcasecmp(othertags->name, "Module")) {
				mod = (Module *)hnode_find(loadedmods, othertags->value);
				if (mod) {
					CheckModVersions(item, mod);
				} else {
					/* check if its our Core! */
					if (!ircstrcasecmp(othertags->value, "NeoStats")) {
						CheckModVersions(item, NULL);
					}
				}
			}			
			othertags = othertags->next;
		}
		item = item->next;
	}
	mrss_free(mrss);
	hash_free_nodes(loadedmods);
}

static int update_check_updates(void *unused)
{
	if (new_transfer(update_cfg.updateurl, NULL, NS_MEMORY, "", NULL, UpdateRSSHandler) != NS_SUCCESS) {
		nlog(LOG_WARNING, "Download Update RSS Feed Failed");
	}
}

/** @brief ModInit
 *
 *  Init handler
 *  Loads connectserv configuration
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( void )
{
	SET_SEGV_LOCATION();
	/* Load stored configuration */
	ModuleConfig( update_settings );
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *  Introduce bot onto network
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch( void )
{
	SET_SEGV_LOCATION();

	if (add_services_set_list(update_settings) != NS_SUCCESS) {
		return NS_FAILURE;
	} 
	if (update_cfg.enabled == 1) {
		/* updates are enabled. Setup Timer and load first update */
		AddTimer(TIMER_TYPE_INTERVAL, update_check_updates, "CheckUpdates", 86400, NULL);
		update_check_updates(NULL);
	}
#if 0
	/* we ride on the main NeoStats Bot */
	cs_bot = AddBot( &update_botinfo );
	/* If failed to create bot, module will terminate */
	if( !cs_bot ) 
		return NS_FAILURE;
#endif
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModFini( void )
{
	SET_SEGV_LOCATION();
	return NS_SUCCESS;
}

static void update_report_update( const CmdParams *cmdparams) {

}

/** @brief update_check_uoper
 *
 *  umode event handler
 *  report umode changes
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int update_check_umode( const CmdParams *cmdparams )
{
	/* Mask of modes we will handle */
	static const unsigned int OperUmodes = 
		UMODE_NETADMIN |
		UMODE_TECHADMIN |
		UMODE_ADMIN |
		UMODE_COADMIN |
		UMODE_SADMIN |
		UMODE_OPER |
		UMODE_LOCOP |
		UMODE_SERVICES;
	unsigned int mask;
	int add = 1;
	const char *modes;

	SET_SEGV_LOCATION();
	modes = cmdparams->param;
	while( *modes != '\0' )
	{
		switch( *modes ) 
		{
			case '+':
				add = 1;
				break;
			case '-':
				add = 0;
				break;
			default:
				mask = UmodeCharToMask( *modes );
				if( (OperUmodes & mask) && (add == 1))
					update_report_update(cmdparams);
				break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/** @brief update_check_smode
 *
 *  smode event handler
 *  report smode changes
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int update_check_smode( const CmdParams *cmdparams )
{
	/* Mask of modes we will handle */
	static const unsigned int OperSmodes =
		SMODE_NETADMIN |
		SMODE_CONETADMIN |
		SMODE_TECHADMIN |
		SMODE_COTECHADMIN |
		SMODE_ADMIN |
		SMODE_COADMIN |
		SMODE_GUESTADMIN;
	unsigned int mask;
	int add = 1;
	const char *modes;

	SET_SEGV_LOCATION();
	modes = cmdparams->param;
	while( *modes != '\0' )
	{
		switch( *modes ) 
		{
			case '+':
				add = 1;
				break;
			case '-':
				add = 0;
				break;
			default:
				mask = SmodeCharToMask( *modes );
				if( (OperSmodes & mask) && (add == 1))
					update_report_update(cmdparams);
				break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/** @brief update_set_updateurl_cb
 *
 *  Set callback for updateurl
 *  Check the supplied URL is valid
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int update_set_updateurl_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_VALIDATE )
	{
	}
	return NS_SUCCESS;
}

/** @brief update_set_updateenabled_cb
 *
 *  Set callback for updateenabled
 *  Turn on/off the timer
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int update_set_updateenabled_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_VALIDATE )
	{
	}
	return NS_SUCCESS;
}
