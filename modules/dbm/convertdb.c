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
** $Id: main.c 2974 2005-12-06 21:39:11Z Mark $
*/

#include "neostats.h"


Module *RunModule[10];
int RunLevel = 0;

/** @brief print_copyright
 *
 *  print copyright notice
 *  NeoStats core use only.
 *  Not used on Win32
 *
 *  @param none
 *
 *  @return none
 */

static void print_copyright( void )
{
	printf( "-----------------------------------------------\n" );
	printf( "Copyright: NeoStats Group. 2000-2006\n" );
	printf( "Justin Hammond (fish@neostats.net)\n" );
	printf( "-----------------------------------------------\n\n" );
}

/** @brief get_options
 *
 *  Processes command line options
 *  NeoStats core use only.
 *  Not used in Win32.
 *
 *  @param argc count of command line parameters
 *  @param argv array of command line parameters
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int get_options( int argc, char **argv )
{
	int c;
	int level;

	while( ( c = getopt( argc, argv, "hv" ) ) != -1 ) {
		switch( c ) {
		case 'h':
			printf( "%s: Usage: \"%s [options]\"\n", basename(argv[0]), basename(argv[0]));
			printf( "     -h (Show this screen)\n" );
			printf( "     -v (Show version number)\n" );
			return NS_FAILURE;
		case 'v':
			printf( "NeoStats: http://www.neostats.net\n" );
			printf( "Version:  1\n");
			print_copyright();
			return NS_FAILURE;
			break;
		default:
			printf( "Unknown command line switch %c\n", optopt );
		}
	}
	return NS_SUCCESS;
}


/** @brief main
 *
 *  Program entry point
 *  NeoStats core use only.
 *  Under Win32 this is used purely as a startup function and not 
 *  the main entry point
 *
 *  @param argc count of command line parameters
 *  @param argv array of command line parameters
 *
 *  @return EXIT_SUCCESS if succeeds, EXIT_FAILURE if not 
 */

int main( int argc, char *argv[] )
{
	/* get our commandline options */
	if( get_options( argc, argv ) != NS_SUCCESS )
		return EXIT_FAILURE;
	return 1;
}

