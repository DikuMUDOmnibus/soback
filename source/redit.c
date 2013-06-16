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

#define	REDIT_NONE			0
#define	REDIT_NAME			1 
#define REDIT_ZONE			2
#define	REDIT_DESC			5

#define REDIT_SECTOR		10
#define REDIT_FLAGS			11

#define REDIT_D_WORD		15
#define REDIT_D_DESC		16
#define REDIT_D_EXIT		17
#define REDIT_D_KEY			18
#define REDIT_D_TO			19

#define REDIT_QUIT			35

static void re_print_one_dir( charType * ch, directionType * dir, int nr )
{
	if( !dir )
	{
	sendf( ch, "%2d] %-10s : None", nr + 7, onewordc( dirs[nr] ) );
	}
	else
	{
	sendf( ch, "%2d] %-10s : %d - %s", nr + 7, onewordc( dirs[nr] ),
						world[dir->to_room].virtual, world[dir->to_room].name );
	if( dir->keyword )
	sendf( ch, "       keyword : %s", dir->keyword );
	if( dir->description )
	sendf( ch, "          desc : %s", dir->description );
	if( dir->exit_info )
	sendf( ch, "          info : %s", sprintbit( dir->exit_info, exit_bits, 0 ) );
	if( dir->key > 0 )
	sendf( ch, "           key : %d", dir->key );
	}
}

static void re_print_room( charType * ch, editUnitType * eu )
{
	roomType		*	ri = eu->re;
	int					i;

	sendf( ch, "---------------- index %4d - virtual %5d.", ri->nr, ri->virtual );
	sendf( ch, "" );
	sendf( ch, " 1]       Zone : %d", ri->zone );
	sendf( ch, " 2]       Name : %s", ri->name  ? ri->name  : "None" );
	sendf( ch, " 3]Description : %s", ri->description ? ri->description : "None" );	
	sendf( ch, " 4]     Sector : %s", sprinttype( ri->sector, sector_types, 0 ) );
	sendf( ch, " 5]      Flags : %s", sprintbit( ri->flags, room_bits, 0 ) );
	sendf( ch, " 6] Extra desc : %s", ri->extrd ? ri->extrd->keyword : "None" );
	sendf( ch, " -- Direction --" );

	for( i = 0; i < MAX_DIRECTIONS; i++ ) re_print_one_dir( ch, ri->dirs[i], i );

	sendf( ch, "" );
	sendf( ch, "---------------- S), Q), X), N), P), J), K), E), G) #, H) help" );

	ed_new_prompt( eu, "Redit> " );

	ch->sub_1 = EDIT_ROOM;
	ch->sub_2 = REDIT_NONE;
}

static void re_print_name( charType * ch, editUnitType * eu )
{
	roomType		*	ri = eu->re;
	char				buf[100];

	sprintf( buf, "Redit %s - name > ", ri->name );	

	ed_new_prompt( eu, buf );

	ch->sub_2 = REDIT_NAME;
}

static void re_get_name( charType * ch, editUnitType * eu, char * argu )
{
	roomType		*	ri = eu->re;

	if( argu = skipsps( argu ), *argu )
	{
		ri->name = strdup( argu );		
	}

	re_print_room( ch, eu );
}

static void re_print_desc( charType * ch, editUnitType * eu )
{
	roomType		*	ri = eu->re;
	char				tmp[MAX_STRING_LENGTH+1];
	char				buf[300];

	strcpy( tmp, ri->description ? ri->description : "None" );

	sendf( ch, "Redit > %s", remove_newline( tmp ) );

	sprintf( buf, "Redit %s - description > ", ri->name );

	ed_new_prompt( eu, buf );

	ch->sub_2 = REDIT_DESC;
}

static void re_get_desc( charType * ch, editUnitType * eu, char * argu )
{
	roomType		*	ri = eu->re;

	if( argu = skipsps( argu ), *argu )
	{
		ri->description = strdup( argu );
	}

	re_print_room( ch, eu );
}

static void re_print_flags( charType * ch, editUnitType * eu )
{
	roomType		*	ri = eu->re;
	int					i, toggle = 0;
	char				buf[300];

	sprintf( buf, "%2d. %-20s", 0, "Clear" );

	for( i = 0, toggle = 1; room_bits[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i + 1, room_bits[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Redit flags > %s\n\rRedit flags > ", sprintbit( ri->flags, room_bits, 0 ) );
	
	ed_new_prompt( eu, buf );

	ch->sub_2 = REDIT_FLAGS;
}

static void re_get_flags( charType * ch, editUnitType * eu, char * argu )
{
	roomType		*	ri = eu->re;
	int					nr = 0;
	char				buf[300];

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > LAST_ROOM_FLAG )
		{
			sendf( ch, "Invalid room flag." ); return;
		}
	}

	if( *argu && nr == 0 )
	{
		ri->flags = 0;
		sprintf( buf, "Redit flags %s\n\rRedit flags > ", sprintbit( ri->flags, room_bits, 0 ) );
		ed_new_prompt( eu, buf );
	}
	else if( nr )
	{
		nr--;
		if( ri->flags & ( 1 << nr ) ) ri->flags &= ~(1 << nr);
		else 						  ri->flags |=   1 << nr ;

		sprintf( buf, "Redit flags %s\n\rRedit flags > ", sprintbit( ri->flags, room_bits, 0 ) );
		ed_new_prompt( eu, buf );
	}
	else
	{
		re_print_room( ch, eu );
	}
}

static void re_print_sector( charType * ch, editUnitType * eu )
{
	roomType			*	ri = eu->re;
	char					buf[300];
	int						i, toggle;

	buf[0] = '\0';

	for( i = 0, toggle = 0; sector_types[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i, sector_types[i] );

		if( ++toggle >= 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0, buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Redit %s - sector %s > ", ri->name, sprinttype( ri->sector, sector_types, 0 ) );

	ch->sub_2 = REDIT_SECTOR;

	ed_new_prompt( eu, buf );
}

static void re_get_sector( charType * ch, editUnitType * eu, char * argu )
{
    int         nr;

    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0 || nr > SECT_LAST )
        {
            sendf( ch, "Invalid sector type." ); return;
        }

        eu->re->sector = nr;
    }

    re_print_room( ch, eu );
}

static void re_print_extr( charType * ch, editUnitType * eu )
{
    exdescriptionType   *   ex;
    roomType            *   ri = eu->re;
    int                     i;

    ex = ri->extrd;

    if( !ex )
    {
        sendf( ch, "Redit > None" );
    }
    else
    {
        for( i = 1; ex; ex = ex->next )
            sendf( ch, "Redit %d > %s\n\r%s", i++, ex->keyword, ex->description );
    }

    sendf( ch, "Editing extra description is not yet implemented." );
    ch->sub_2 = REDIT_NONE;
}

static void re_print_zone( charType * ch, editUnitType * eu )
{   
    roomType            *   ri = eu->re;
    char                	buf[300];
    
    sprintf( buf, "Redit %s - zone %d > ", ri->name, ri->zone );
    
    ed_new_prompt( eu, buf );
    
    ch->sub_2 = REDIT_ZONE;
}
        
static void re_get_zone( charType * ch, editUnitType * eu, char * argu )
{   
    roomType            *   ri = eu->re;
    int     				nr;         

    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0 )
        {
            sendf( ch, "Invalid zone." ); return;
        }

        ri->zone = nr;
    }

    re_print_room( ch, eu );
}

static void re_print_d_word( charType * ch, editUnitType * eu )
{
    roomType            *   ri = eu->re;
	directionType		*	di;
	char					buf[200];

	di = eu->re->dirs[eu->dir] = eu->de  = errCalloc( sizeof( directionType ) );

	if( world[ri->nr].dirs[eu->dir] ) *di = *(world[ri->nr].dirs[eu->dir]);

	sprintf( buf, "Redit %s - %s - keyword %s > ", 
					ri->name, sprinttype( eu->dir, dirs, 0 ), 
					di->keyword ? di->keyword : "None" );

	ed_new_prompt( eu, buf );

	ch->sub_2 = REDIT_D_WORD;
}

static void re_print_exit_info( charType * ch, editUnitType * eu )
{
    roomType            *   ri = eu->re;
	directionType		*	di = eu->de;
	char					buf[400];
	int						i, toggle;

	buf[0] = '\0';

	for( i = 0, toggle = 0; exit_bits[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i+1, exit_bits[i] );

		if( ++toggle >= 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0, buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Redit exit > %s\n\rRedit %s - %s - exit > ", 
			sprintbit( di->exit_info, exit_bits, 0 ), ri->name, sprinttype( eu->dir, dirs, 0 ) );

	ed_new_prompt( eu, buf );

	ch->sub_2 = REDIT_D_EXIT;
}

static void re_get_d_word( charType * ch, editUnitType * eu, char * argu )
{
    roomType            *   ri = eu->re;
	directionType		*	di = eu->de;
	char					buf[200];
	
	if( *argu )
	{
		di->keyword = strdup( argu );
	}

	sendf( ch, "Redit > %s", di->description ? di->description : "None" );
	sprintf( buf, "Redit %s - %s - desc > ", 
					ri->name, sprinttype( eu->dir, dirs, 0 ) );

	ed_new_prompt( eu, buf );

	ch->sub_2 = REDIT_D_DESC;
}

static void re_get_d_desc( charType * ch, editUnitType * eu, char * argu )
{
    roomType            *   ri = eu->re;
	directionType		*	di = eu->de;
	char					buf[200];
	
	if( *argu )
	{
		di->keyword = strdup( argu );
	}

	sendf( ch, "Redit > %s", di->description ? di->description : "None" );
	sprintf( buf, "Redit %s - %s - desc > ", 
					ri->name, sprinttype( eu->dir, dirs, 0 ) );

	ed_new_prompt( eu, buf );

	ch->sub_2 = REDIT_D_EXIT;

	re_print_exit_info( ch, eu );
}

static void re_get_d_exit( charType * ch, editUnitType * eu, char * argu )
{
    roomType            *   ri = eu->re;
	directionType		*	di = eu->de;
    int                 	nr = 0;
    char                	buf[300];
   
    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0 || nr > EX_LAST )
        {
            sendf( ch, "Invalid exit flag." ); return; 
        }

		if( nr != 1 && nr != 6 && nr != 7 )
		{
			sendf( ch, "That type of exit flag could not be edited." ); return;
		}
    }
   
    if( nr )
    {
        nr--;
        if( di->exit_info & ( 1 << nr ) ) di->exit_info &= ~(1 << nr);
        else                        	  di->exit_info |=   1 << nr ;
   
		sprintf( buf, "Redit exit > %s\n\rRedit %s - %s - exit > ", 
			sprintbit( di->exit_info, exit_bits, 0 ), ri->name, sprinttype( eu->dir, dirs, 0 ) );

		ed_new_prompt( eu, buf );
    }
    else
    {
		if( di->exit_info & EX_ISDOOR )
		{
			sprintf( buf, "Redit %s - key %d > ", ri->name, di->key );
			ed_new_prompt( eu, buf );

			ch->sub_2 = REDIT_D_KEY;
		}
		else
		{
			sprintf( buf, "Redit %s - to_room %d > ", ri->name, world[di->to_room].virtual );
			ed_new_prompt( eu, buf );
			ch->sub_2 = REDIT_D_TO;
		}
    }
}

static void re_get_d_key( charType * ch, editUnitType * eu, char * argu )
{
    roomType            *   ri = eu->re;
	directionType		*	di = eu->de;
    int                 	nr = 0, index;
	char					buf[300];

    if( *argu )
    {
        if( getnumber( argu, &nr ) == 0 || nr < -1 )
        {
            sendf( ch, "Invalid key number." ); return;
        }

		if( nr != -1 && nr != 0 )
			if( index = real_objectNr( nr ), index == OBJECT_NULL || objects[index].type != ITEM_KEY )
			{
				sendf( ch, "No such item or it's not a key type." ); return;
			}

        di->key = nr;
    }

	sprintf( buf, "Redit %s - to_room %d > ", ri->name, world[di->to_room].virtual );

	ed_new_prompt( eu, buf );

	ch->sub_2 = REDIT_D_TO;
}

static void re_get_d_to( charType * ch, editUnitType * eu, char * argu )
{
	roomType			*	ri = eu->re;
	directionType		*	di = eu->de;
    int                 	nr = 0, index;

    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0 )
        {
            sendf( ch, "Invalid room number." ); return;
        }

		if( nr == 0 )
		{
			ri->dirs[eu->dir] = 0;
		}	
		else if( index = real_roomNr( nr ), index == NOWHERE )
		{
			sendf( ch, "No such room in the world." ); return;
		}
		else
        	di->to_room = index;
    }

	eu->de = 0;
	re_print_room( ch, eu );
}

static void re_quit_edit( charType * ch, editUnitType * eu )
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
		sendf( ch, "'%s' modified. Apply this to the table ? ", eu->re->name );

		ch->sub_2 = REDIT_QUIT;
		ch->sub_3 = EDIT_YESNO;
	}
	else
	{
	  	del_editing( eu );
	  	ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
	}
}

static void re_next_edit( charType * ch, editUnitType * eu, int mode, int prev )
{
	editUnitType	*	edit;
	int					nr;

	nr = eu->re->nr;

	if( prev )
	{
		if( --nr <= ROOM_INTERNAL )
		{
			sendf( ch, "No previous room in the room table." ); return;
		}
	}
	else
	{
		if( ++nr >= room_index_info.used )
		{
			sendf( ch, "No more room in the room table." ); return;
		}
	}

	if( mode && ed_is_modified( ch, eu ) )
	{
		ed_save_edit( ch, eu );
	}

	del_editing( eu );
  	ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;

	edit = new_editing( ch, EDIT_ROOM, nr );
	re_print_room( ch, edit );
}

static void re_change_edit( charType * ch, editUnitType * eu, char * argu )
{
	editUnitType	*	edit;
	int					nr;
	char				buf1[MAX_INPUT_LENGTH+1];
	char				buf2[MAX_INPUT_LENGTH+1];

	twoArgument( argu, buf1, buf2 );

	if( !isnumstr( buf2 ) )
	{
		sendf( ch, "Need number of room to change editing room." );
		return;
	}

	if( getnumber( buf2, &nr ) <= 0 )
	{
		sendf( ch, "Invalid room number." ); return;
	}

	if( nr = real_roomNr( nr ), nr == NOWHERE )
	{
		sendf( ch, "No room by that number." ); return;
	}

	if( ed_is_modified( ch, eu ) )
	{
		ed_save_edit( ch, eu );
	}

	del_editing( eu );
  	ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;

	edit = new_editing( ch, EDIT_ROOM, nr );
	re_print_room( ch, edit );
}

static void re_help( charType * ch )
{
	sendf( ch, "Medit help ----------" );
	sendf( ch, "# > to edit specified field." );
	sendf( ch, "S > apply edited room to the room table." );
	sendf( ch, "Q > quit."			  		  );
	sendf( ch, "X > quit without saving."     );
	sendf( ch, "N > edit next room in the room table." );
	sendf( ch, "P > edit previous room in the room table." );
	sendf( ch, "J > edit next room without saving." );
	sendf( ch, "K > edit previous room without saving." );
	sendf( ch, "G # > change editing room to new." );
	sendf( ch, "H > show this page." );
}

void do_redit( charType * ch, char * argu, int cmd )
{
	editUnitType	*	edit;
	int					nr;
	int					ret;

	if( (ch->mode && ch->mode != EDIT_MODE) || (ch->sub_1 && ch->sub_1 != EDIT_ROOM) )
	{
		sendf( ch, "You can not do that." ); return;
	}

	ch->mode = EDIT_MODE;

	if( ch->sub_1 == EDIT_NONE )
	{
		if( !*argu )
		{
			sendf( ch, "iedit # ( virtual number )" ); return;
		}

		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid number." ); return;
		}

		if( nr = real_roomNr( nr ), nr <= ROOM_INTERNAL )
		{
			sendf( ch, "That room is not found." ); return;
		}	

		if( ret = find_editing( ch, EDIT_ROOM, nr ), ret > 0 )
		{	
			if( ret == 1 )
			{
				sendf( ch, "That room is not available now." ); return;
			}

			if( ret == 2 )
			{
				sendf( ch, "Oops.. You are already in editing list. Retry" ); 
				edit = find_editunit( ch );
				del_editing( edit );
				return;
			}
		}

		edit = new_editing( ch, EDIT_ROOM, nr );
		re_print_room( ch, edit );

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
		case REDIT_NAME			:	re_get_name( ch, edit, argu ); 			break;
		case REDIT_DESC			:	re_get_desc( ch, edit, argu ); 			break;
		case REDIT_ZONE			:	re_get_zone( ch, edit, argu );			break;
		case REDIT_FLAGS		:	re_get_flags( ch, edit, argu );			break;
		case REDIT_SECTOR		:	re_get_sector( ch, edit, argu );		break;
		case REDIT_D_WORD		:	re_get_d_word( ch, edit, argu );		break;
		case REDIT_D_DESC		:	re_get_d_desc( ch, edit, argu );		break;
		case REDIT_D_EXIT		:	re_get_d_exit( ch, edit, argu );		break;
		case REDIT_D_KEY		:	re_get_d_key( ch, edit, argu );			break;
		case REDIT_D_TO			:	re_get_d_to( ch, edit, argu );			break;

		case REDIT_QUIT			:   re_quit_edit( ch, edit );       	    break;
		}
		return;
	}

	if( getnumber( argu, &nr ) > 0 )
	{
		switch( nr )
		{
		case	 1  :	re_print_zone		( ch, edit );			break;
		case	 2  :	re_print_name  		( ch, edit ); 			break;
		case	 3  :	re_print_desc  		( ch, edit ); 			break;
		case	 4  :	re_print_sector		( ch, edit );			break;
		case	 5  :	re_print_flags		( ch, edit );			break;
		case	 6  :	re_print_extr		( ch, edit );			break;
		case	 7  :   edit->dir = 0;	
						re_print_d_word		( ch, edit );			break;
		case	 8  :   edit->dir = 1;	
						re_print_d_word		( ch, edit );			break;
		case	 9  :   edit->dir = 2;	
						re_print_d_word		( ch, edit );			break;
		case	 10 :   edit->dir = 3;	
						re_print_d_word		( ch, edit );			break;
		case	 11 :   edit->dir = 4;	
						re_print_d_word		( ch, edit );			break;
		case	 12 :   edit->dir = 5;	
						re_print_d_word		( ch, edit );			break;
		default		:	sendf( ch, "Huh? type 'h' for help." ); 	break;
		}
	}
	else
	{
		if( !*argu ) 
		{
			re_print_room( ch, edit ); return;
		}
		else if( strlen( argu ) >= 1 )
		{
			switch( *argu )
			{
			case	'S' : 	ed_save_edit( ch, edit );  					return;	 
			case	'Q' :	re_quit_edit( ch, edit ); 	 				return;
			case 	'X'	:  	del_editing( edit ); 
				ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
																		return;
			case	'N' :   re_next_edit( ch, edit, 1, 0 );  			return;
			case	'P' :   re_next_edit( ch, edit, 1, 1 );  			return;
			case	'J' :   re_next_edit( ch, edit, 0, 0 );  			return;
			case	'K' :   re_next_edit( ch, edit, 0, 1 );  			return;
			case	'H' :	re_help( ch );								return;
			case	'G' :	re_change_edit( ch, edit, argu );			return;
			}
		}
		interpreter( ch, argu, 0 );
	}
}

void ed_rcopy( charType * ch, char * argu, int cmd )
{
}
