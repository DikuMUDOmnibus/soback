#include <string.h>
#include <ctype.h>

#include "character.h"
#include "world.h"
#include "mobile.h"
#include "object.h"
#include "zone.h"
#include "interpreter.h"
#include "comm.h"
#include "strings.h"
#include "utils.h"
#include "variables.h"
#include "edit.h"
#include "allocate.h"

#define	ZEDIT_NONE			0
#define	ZEDIT_NAME			1 
#define	ZEDIT_INFO			2 
#define	ZEDIT_PATH			3

#define	ZEDIT_LOW			6
#define	ZEDIT_HIGH			7
#define	ZEDIT_AGGR			8 
#define	ZEDIT_ALIGN			9

#define	ZEDIT_LIFE			10
#define	ZEDIT_RESET			11

#define ZEDIT_TOP			15

#define ZEDIT_QUIT			35

static void ze_print_zone( charType * ch, editUnitType * eu )
{
	zoneType		*	zi;

	zi = eu->ze;

	sendf( ch, "---------------- index %4d - virtual #%5d.", zi->nr, zi->zone );
	sendf( ch, "" );
	sendf( ch, " 1]       Name : %s", zi->name ? zi->name : "None" );
	sendf( ch, " 2]       Info : %s", zi->info ? zi->info : "None" );
	sendf( ch, " 3]       Path : %s", zi->path ? zi->path : "None" );
	sendf( ch, " 4]  for Level : %d - %d", zi->low, zi->high );
	sendf( ch, " 5] Aggressive : %s", sprinttype( zi->aggressive, zone_aggressives, 0 ) );
	sendf( ch, " 6]      Align : %s", sprinttype( zi->align,  zone_aligns, 0 ) );
	sendf( ch, " 7]   Lifespan : %d", zi->lifespan );
	sendf( ch, " 8]      Reset : %d", zi->reset_mode );
	sendf( ch, " 9]        Top : %d", zi->max );
	sendf( ch, "" );
	sendf( ch, "---------------- S), Q), X), N), P), J), K), E), H) help" );

	ed_new_prompt( eu, "Zedit> " );

	ch->sub_1 = EDIT_ZONE;
	ch->sub_2 = ZEDIT_NONE;
}

static void ze_print_name( charType * ch, editUnitType * eu )
{
	zoneType		*	zi = eu->ze;
	char				buf[100];

	sprintf( buf, "Medit %s - name > ", zi->name );	

	ed_new_prompt( eu, buf );

	ch->sub_2 = ZEDIT_NAME;
}

static void ze_get_name( charType * ch, editUnitType * eu, char * argu )
{
	zoneType		* 	zi = eu->ze;

	if( argu = skipsps( argu ), *argu )
	{
		zi->name = strdup( argu );		
	}

	ze_print_zone( ch, eu );
}

static void ze_print_info( charType * ch, editUnitType * eu )
{
	zoneType		* 	zi = eu->ze;
	char				buf[100];
	char				tmp[MAX_STRING_LENGTH+1];

	strcpy( tmp, zi->info ? zi->info : "None" );

	sendf( ch, "Zedit > %s", remove_newline(tmp) );

	sprintf( buf, "Zedit %s - info > ", zi->name );	

	ed_new_prompt( eu, buf );

	ch->sub_2 = ZEDIT_INFO;
}

static void ze_get_info( charType * ch, editUnitType * eu, char * argu )
{
	zoneType		* 	zi = eu->ze;

	if( argu = skipsps( argu ), *argu )
	{
		zi->info = strdup( argu );
	}

	ze_print_zone( ch, eu );
}

static void ze_print_path( charType * ch, editUnitType * eu )
{
	zoneType		* 	zi = eu->ze;
	char				buf[100];
	char				tmp[MAX_STRING_LENGTH+1];

	strcpy( tmp, zi->path ? zi->path : "None" );

	sendf( ch, "Zedit > %s", remove_newline(tmp) );

	sprintf( buf, "Zedit %s - path > ", zi->name );	

	ed_new_prompt( eu, buf );

	ch->sub_2 = ZEDIT_PATH;
}

static void ze_get_path( charType * ch, editUnitType * eu, char * argu )
{
	zoneType		* 	zi = eu->ze;

	if( argu = skipsps( argu ), *argu )
	{
		zi->path = strdup( argu );
	}

	ze_print_zone( ch, eu );
}

static void ze_print_align( charType * ch, editUnitType * eu )
{
	zoneType		*	zi = eu->ze;
	int					i, toggle = 0;
	char				buf[200];

	buf[0] = '\0';

	for( i = 0, toggle = 0; zone_aligns[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i, zone_aligns[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Zedit %s - align %s > ", zi->name, sprinttype( zi->align, zone_aligns, 0 ));

	ed_new_prompt( eu, buf );

	ch->sub_2 = ZEDIT_ALIGN;
}

static void ze_get_align( charType * ch, editUnitType * eu, char * argu )
{
	int			nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > 4 )
		{
			sendf( ch, "Invalid align type." ); return;
		}

		eu->ze->align = nr;
	}

	ze_print_zone( ch, eu );
}

static void ze_print_aggressive( charType * ch, editUnitType * eu )
{
	zoneType		*	zi = eu->ze;
	int					i, toggle = 0;
	char				buf[200];

	buf[0] = '\0';

	for( i = 0, toggle = 0; zone_aggressives[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i, zone_aggressives[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Zedit %s - align %s > ", zi->name, sprinttype( zi->align, zone_aggressives, 0 ));

	ed_new_prompt( eu, buf );

	ch->sub_2 = ZEDIT_AGGR;
}

static void ze_get_aggressive( charType * ch, editUnitType * eu, char * argu )
{
	int			nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > 2 )
		{
			sendf( ch, "Invalid aggressive type." ); return;
		}

		eu->ze->aggressive = nr;
	}

	ze_print_zone( ch, eu );
}

static void ze_print_reset( charType * ch, editUnitType * eu )
{
	zoneType		*	zi = eu->ze;
	int					i, toggle = 0;
	char				buf[200];

	buf[0] = '\0';

	for( i = 0, toggle = 0; zone_resets[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i, zone_resets[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Zedit %s - reset %s > ", zi->name, sprinttype( zi->reset_mode, zone_resets, 0 ));

	ed_new_prompt( eu, buf );

	ch->sub_2 = ZEDIT_RESET;
}

static void ze_get_reset( charType * ch, editUnitType * eu, char * argu )
{
	int			nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > 2 )
		{
			sendf( ch, "Invalid reset type." ); return;
		}

		eu->ze->reset_mode = nr;
	}

	ze_print_zone( ch, eu );
}

static void ze_print_life( charType * ch, editUnitType * eu )
{
	zoneType		*	zi = eu->ze;
	char				buf[300];

	sprintf( buf, "Zedit %s - lifespan %d > ", zi->name, zi->lifespan );

	ed_new_prompt( eu, buf );

	ch->sub_2 = ZEDIT_LIFE;
}

static void ze_get_life( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid lifespan." ); return;
		}

		eu->ze->lifespan = nr;
	}

	ze_print_zone( ch, eu );
}

static void ze_print_top( charType * ch, editUnitType * eu )
{
	zoneType		*	zi = eu->ze;
	char				buf[300];

	sprintf( buf, "Zedit %s - top %d ==> ", zi->name, zi->max );

	ed_new_prompt( eu, buf );

	ch->sub_2 = ZEDIT_TOP;
}

static void ze_get_top( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > ROOM_DEATH )
		{
			sendf( ch, "Invalid top of virtual number." ); return;
		}

		eu->ze->align = nr;
	}

	ze_print_zone( ch, eu );
}

static void ze_print_low( charType * ch, editUnitType * eu )
{
	zoneType		*	zi = eu->ze;
	char				buf[300];

	sprintf( buf, "Zedit %s - Low %d > ", zi->name, zi->low );

	ed_new_prompt( eu, buf );

	ch->sub_2 = ZEDIT_LOW;
}

static void ze_get_low( charType * ch, editUnitType * eu, char * argu )
{
	zoneType		*	zi = eu->ze;
	char				buf[200];
	int					nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > IMO )
		{
			sendf( ch, "Invalid level." ); return;
		}

		zi->low = nr;
	}

	sprintf( buf, "Zedit %s - High %d > ", zi->name, zi->high );

	ed_new_prompt( eu, buf );

	ch->sub_2 = ZEDIT_HIGH;
}

static void ze_get_high( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > IMO )
		{
			sendf( ch, "Invalid level." ); return;
		}

		eu->ze->high = nr;
	}

	ze_print_zone( ch, eu );
}

static void ze_quit_edit( charType * ch, editUnitType * eu )
{
	if( ch->sub_3 != EDIT_NONE )
	{
		switch( ch->sub_3 )
		{
			case	EDIT_YESNO  : break;
			case	EDIT_YES	: ed_save_edit( ch, eu ); 
								  del_editing( eu );
								  ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
								  break;
			case    EDIT_NO     : del_editing( eu );
								  ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
								  break;
		}
	}
	else if( ed_is_modified( ch, eu ) )
	{
		sendf( ch, "'%s' modified. Apply this to the table ? ", eu->ze->name );

		ch->sub_2 = ZEDIT_QUIT;
		ch->sub_3 = EDIT_YESNO;
	}
	else
	{
	  	del_editing( eu );
	  	ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
	}
}

static void ze_next_edit( charType * ch, editUnitType * eu, int mode, int prev )
{
	editUnitType	*	edit;
	int					nr;

	nr = eu->ze->nr;

	if( prev )
	{
		if( --nr <= ZONE_INTERNAL )
		{
			sendf( ch, "No previous zone in the zone table." ); return;
		}
	}
	else
	{
		if( ++nr >= zone_index_info.used )
		{
			sendf( ch, "No more zone in the zone table." ); return;
		}
	}

	if( mode && ed_is_modified( ch, eu ) )
	{
		ed_save_edit( ch, eu );
	}

	del_editing( eu );
  	ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;

	edit = new_editing( ch, EDIT_ZONE, nr );
	ze_print_zone( ch, edit );
}

static void ze_help( charType * ch )
{
	sendf( ch, "Medit help ----------" );
	sendf( ch, "# > to edit specified field." );
	sendf( ch, "S > apply edited zone to the zone table." );
	sendf( ch, "Q > quit."			  		  );
	sendf( ch, "X > quit without saving."     );
	sendf( ch, "N > edit next zone in the zone table." );
	sendf( ch, "P > edit previous zone in the zone table." );
	sendf( ch, "J > edit next zone without saving." );
	sendf( ch, "K > edit previous zone without saving." );
	sendf( ch, "E > save and effect all zone in the world." );
	sendf( ch, "H > show this page." );
}

void do_zedit( charType * ch, char * argu, int cmd )
{
	editUnitType	*	edit;
	int					nr;
	int					ret;

	if( (ch->mode && ch->mode != EDIT_MODE) || (ch->sub_1 && ch->sub_1 != EDIT_ZONE) )
	{
		sendf( ch, "You can not do that." ); return;
	}

	ch->mode = EDIT_MODE;

	if( ch->sub_1 == EDIT_NONE )
	{
		if( !*argu )
		{
			sendf( ch, "zedit # ( real number )" ); return;
		}

		if( getnumber( argu, &nr ) <= 0 || nr == 0 )
		{
			sendf( ch, "Invalid number." ); return;
		}

		if( nr <= ZONE_INTERNAL || nr > zone_index_info.used )
		{
			sendf( ch, "That zone is not found." ); return;
		}	

		if( ret = find_editing( ch, EDIT_ZONE, nr ), ret > 0 )
		{	
			if( ret == 1 )
			{
				sendf( ch, "That zone is not available now." ); return;
			}

			if( ret == 2 )
			{
				sendf( ch, "Oops.. You are already in editing list. Retry" ); 
				edit = find_editunit( ch );
				del_editing( edit );
				return;
			}
		}

		edit = new_editing( ch, EDIT_ZONE, nr );
		ze_print_zone( ch, edit );

		return;
	}

	if( edit = find_editunit( ch ), !edit )
	{
		sendf( ch, "Oops. Your edit record not found!." );
		ch->sub_3 = ch->sub_2 = ch->sub_1 = EDIT_NONE;
		ch->mode  = 0;
		return;
	}

	if( ch->sub_2 != EDIT_NONE )
	{
		argu = skipsps( argu );

		switch( ch->sub_2 )
		{
		case ZEDIT_NAME			:	ze_get_name( ch, edit, argu ); 			break;
		case ZEDIT_INFO			:	ze_get_info( ch, edit, argu );			break;
		case ZEDIT_PATH			:	ze_get_path( ch, edit, argu );			break;
		case ZEDIT_LOW			:	ze_get_low( ch, edit, argu );			break;
		case ZEDIT_HIGH			:	ze_get_high( ch, edit, argu );			break;
		case ZEDIT_AGGR			:	ze_get_aggressive( ch, edit, argu );	break;
		case ZEDIT_ALIGN		:	ze_get_align( ch, edit, argu );			break;
		case ZEDIT_LIFE			:	ze_get_life( ch, edit, argu );			break;
		case ZEDIT_RESET		:	ze_get_reset( ch, edit, argu );			break;
		case ZEDIT_TOP			:	ze_get_top( ch, edit, argu );			break;

		case ZEDIT_QUIT			:   ze_quit_edit( ch, edit );       	    break;
		}
		return;
	}

	if( getnumber( argu, &nr ) > 0 )
	{
		switch( nr )
		{
		case	 1  :	ze_print_name  		( ch, edit ); 			break;
		case	 2  :	ze_print_info  		( ch, edit ); 			break;
		case	 3  :	ze_print_path  		( ch, edit ); 			break;
		case	 4  :	ze_print_low  		( ch, edit ); 			break;
		case	 5  :	ze_print_aggressive	( ch, edit ); 			break;
		case	 6  :	ze_print_align		( ch, edit ); 			break;
		case	 7  :	ze_print_life		( ch, edit ); 			break;
		case	 8  :	ze_print_reset		( ch, edit ); 			break;
		case	 9  :	ze_print_top		( ch, edit ); 			break;
		default		:	sendf( ch, "Huh? type 'h' for help." ); 	break;
		}
	}
	else
	{
		if( !*argu )
		{
			ze_print_zone( ch, edit ); return;
		}
		else if( strlen( argu ) == 1 )
		{
			switch( *argu )
			{
			case	'S' : 	ed_save_edit( ch, edit );  					return;	 
			case	'Q' :	ze_quit_edit( ch, edit ); 	 				return;
			case 	'X'	:  	del_editing( edit ); 
				ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
																		return;
			case	'N' :   ze_next_edit( ch, edit, 1, 0 );  			return;
			case	'P' :   ze_next_edit( ch, edit, 1, 1 );  			return;
			case	'J' :   ze_next_edit( ch, edit, 0, 0 );  			return;
			case	'K' :   ze_next_edit( ch, edit, 0, 1 );  			return;
			case	'H' :	ze_help( ch );								return;
			}
		}

		interpreter( ch, argu, 0 );
	}
}

void zd_mcopy( charType * ch, char * argu, int cmd )
{
}
