#include <stdio.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "object.h"
#include "strings.h"
#include "find.h"
#include "interpreter.h"
#include "magics.h"
#include "utils.h"
#include "stash.h"
#include "variables.h"
#include "comm.h"
#include "limits.h"
#include "transfer.h"
#include "spec.rooms.h"

#define	MAX_PAWN_OBJS	1500
static  objectType	*	pawn_list;
static	charType    *   keeper;
static  int				pawn_room_nr;

static void obj_to_pawn_list( objectType * obj )
{
	objectType 	* 	temp, * bad, * find, * last;
	int				i;

	if( pawn_list->what != 1 )
	{
		find = 0; last = 0;
	}
	else
	{
		for( find = pawn_list, last = 0; find; find = last->next_content )
		{
			if( find->next_content && (find->next_content->what > (find->what+1)) ) break;
			last = find;
		}
	}

	if( find )
	{
		obj->next_content  = find->next_content;
		find->next_content = obj;
		obj->what          = find->what + 1;
	}
	else if( last )
	{
		if( last->what + 1 >= MAX_PAWN_OBJS )
		{
			for( find = 0, temp = bad = pawn_list; temp; temp = temp->next_content )
			{
				if( temp->next_content && temp->next_content->cost < bad->cost ) 
				{
					find = temp;
					bad  = temp->next_content;
				}
			}
			detach_off_obj( &pawn_list, bad );
			extract_obj( bad, 0 );
			
			if( find == 0 )
			{
				obj->next_content = pawn_list;
				pawn_list         = obj;
				obj->what         = 1;
			}
			else
			{
				obj->next_content  = find->next_content;
				find->next_content = obj;
				obj->what          = find->what + 1;
			}
		}
		else
		{
			last->next_content = obj;
			obj->what          = last->what + 1;
		}
	}
	else
	{
		obj->next_content = pawn_list;
		pawn_list         = obj;
		obj->what         = 1;
	}

	init_search_records();

	for( temp = pawn_list; temp; temp = temp->next_content )
	{
		record_found_object( temp, 1 );
	}

	for( i = 0; i < objs_in_found_table; i++ )
	{
		if( found_objects[i].num > 5 ) 
		{
			for( bad = 0, find = pawn_list; find; find = find->next_content )
			{
				if( find->nr == found_objects[i].obj->nr )
				{
					if( !bad ) 	bad = find;
					else if( find->limit < bad->limit ) bad = find;
				}
			}
			if( bad )
			{
				detach_off_obj( &pawn_list, bad );
				extract_obj( bad, 0 );
			}
		}
	}
}

static int  pawn_trade_with( objectType * obj )
{
	if( !is_stashable( obj ) ) return FALSE;

	switch( obj->wear )
	{
		case	ITEM_TAKE + ITEM_THROW 		: return FALSE;
		case	ITEM_TAKE					:
					if( obj->type == ITEM_LIGHT ) return TRUE;
					return FALSE;

		case	ITEM_TAKE + ITEM_HOLD		:
			switch( obj->type )
			{
				case 	ITEM_DRINKCON	:
				case	ITEM_SCROLL		:
				case	ITEM_PEN		:
				case	ITEM_NOTE		:
				case	ITEM_BOAT		:
				case	ITEM_POTION		:
				case	ITEM_KEY		:
				case	ITEM_FOOD		:
				case	ITEM_TREASURE	:
				case	ITEM_MONEY		:
				case    ITEM_WAND       :
				case    ITEM_STAFF      :
				case	ITEM_TRASH		: 	return FALSE;
			}
	}
	return TRUE;
}

static void pawn_value( charType * ch, char * argu )
{
	objectType			*	obj;
	int						nr, value = 0;

	if( isnumstr( argu ) )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "%s tells you, 'value what? look sign first.", keeper->moved );
			return;
		}

		if( !( obj = find_off_obj( pawn_list, nr )) )
		{
			sendf( ch, "%s tells you, 'I don't have that item.. list again.", keeper->moved );
			return;
		}

		value = calc_used_value( obj );

		sendf( ch, "%s tells you, 'It const your just %d coins.", keeper->moved, value );
	}
	else
	{
		obj = find_obj_inven( ch, argu );

		if( !obj )
		{
			sendf( ch, "%s tells you, 'You don't have it.'", keeper->moved );
			return;
		}
		else
		{
			if( !pawn_trade_with( obj ) )
			{
				sendf( ch, "%s tells you, 'I don't buy that.'", keeper-> moved );
				return;
			}
			if( obj->contains )
			{
				sendf( ch, "%s tells you, 'It contains other items.'", keeper-> moved );
				return;
			}
			sendf( ch, "%s tells you, 'I'll pay you %d coins for that.'", 
														keeper->moved, calc_used_value(obj));
		}
	}
}

static void pawn_list_item( charType * ch, char * argu )
{
	static	const char * where[] =
	{
		"light",
		"finger",
		"neck", 
		"body",
		"head",
		"legs",
		"feet",
		"hands",
		"arms",
		"shield",
		"aboutbody",
		"waist",
		"wrist",
		"weapon",
		"hold",
		"",		/* throw? */
		"knee",
		"aboutlegs",
		"all",
		"name",
		"\n",
	};

	objectType		*	obj;
	char				buf[MAX_INPUT_LENGTH];
	int					what = -1;
	int					listed = 0, i, page = 0;
	
	buf[0] = 0;

	if( *argu )
	{
		argu = onefword( argu, buf );
		what = isinlistp( buf, where ); 
	}

	if( !*buf || what == -1 )
	{
		sendf( ch, 	"You can list follows.\n\r\n\r"
				   	"\tlight, finger, neck, body, legs, feet, hands\n\r"
                    "\tarms, shield, aboutbody, waist, wrist, weapon\n\r"
					"\thold, knee, aboutlegs, all, name.\n\r\n\r"
					"ex] \"list all\" to see a whole list of items.\n\r"
					"ex] \"list finger 2\" to see a second list of items for your finger.\n\r"
					"ex] \"list all 2, list fi 3, list aboutl, list he\"\n\r" 
					"ex] \"list name bangard\" or \"list na ban 2\"\n\r" );
		return;
	}

	if( what == 19 )
	{
		if( argu = onefword( argu, buf ), !buf[0] )
		{
			sendf( ch, "%s tells you, 'Huh? try list.'" , keeper->name );
			return;
		}
	}

	init_search_records();

	for( i = 0, obj = pawn_list; obj; obj = obj->next_content )
	{
		if( (what == 18)
         || (what == 0 && obj->type == ITEM_LIGHT) 
		 || (what == 19 && isoneofp( buf, obj->name ))
		 || (what > 0 && what < 18 && IS_WEAR( obj, (1 << what)) ) ) record_found_object( obj, 0 );
	}

	if( isnumstr( argu ) ) 
		if( getnumber( argu, &page ), --page < 0 ) page = 0;

	if( !objs_in_found_table ) 
	{
		sendf( ch, "%s tells you, 'I don't have that kind of items.'", keeper->name );
		return;
	}
	else
	{
		sendf( ch, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
		sendf( ch, "+ %3d list of %-20s ( total %3d items in %3d pages )         +",
					 page+1, buf, objs_in_found_table, (objs_in_found_table/20 + 1)  );
		sendf( ch, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
		for( i = page * 20; i < objs_in_found_table && i < ((page + 1) * 20); i++ )
		{
			sendf( ch, "[%3d]%43s -%9s-%10s coins", 
				found_objects[i].obj->what,
				found_objects[i].obj->wornd,
				itemstatus( found_objects[i].obj ),
				numfstr(calc_used_value( found_objects[i].obj )) );
			listed++;
		}
	}

	if( !listed ) sendf( ch, "Nothing" );
	else
	{
		sendf( ch, "There are %d more page(s).", 
               (objs_in_found_table -i)/20 + (objs_in_found_table - i > 0) );
	}
}

static void pawn_sell( charType * ch, char * argu )
{
	objectType	* 	obj;
	int				value;

	if( !argu || !*argu )
	{
		sendf( ch, "%s tells you, 'What do you wanna sell?'", keeper->moved );
		return;
	}

	if( obj = find_obj_inven( ch, argu ), !obj )
	{
		sendf( ch, "%s tells you, 'You don't have it.'", keeper->moved );
		return;
	}

	if( !pawn_trade_with( obj ) )
	{
		sendf( ch, "%s tells you, 'I don't buy it.'", keeper->moved );
		return;
	}

	if( obj->contains )
	{
		sendf( ch, "%s tells you, 'It contains other items.'", keeper-> moved );
		return;
	}

	ch->gold += value = calc_used_value( obj );

	obj_from_char( obj );
	obj_to_off_world( obj );
	obj_to_pawn_list( obj );

	stash_off_world( "pawn", pawn_list );

	act( "$n sells $p.", TRUE, ch, obj, 0, TO_ROOM );
	sendf( ch, "%s tells you, 'Here you have %d coins for that.", keeper->moved, value );
}

static void pawn_buy( charType * ch, char * argu )
{
	char			argm[MAX_INPUT_LENGTH+1];
	char			card[MAX_INPUT_LENGTH+1];
	objectType	* 	obj;
	int				nr, value;

	twoArgument( argu, argm, card );

	if( getnumber( argm, &nr ) <= 0 || !( obj = find_off_obj( pawn_list, nr )) ) 
	{
		sendf( ch, "%s tells you, 'What do you wanna buy?? Try list.'", keeper->moved );
		return;
	}

	value = calc_used_value( obj );

	if( !cost_from_player( ch, keeper, card, value, 0 ) ) return;

	detach_off_obj( &pawn_list, obj );
	obj_to_in_world( obj );

	obj_to_char( obj, ch );

	act( "$n buys $p.", TRUE, ch, obj, 0, TO_ROOM );
}

static int pawn_examine( charType * ch, char * argu )
{
	objectType		*	obj;
	int					nr;

	if( !argu || !*argu )
	{
		sendf( ch, "%s tells you, 'What do you want to examine?'", keeper->moved );
		return TRUE;
	}

	if( isnumstr( argu ) )
	{
		if( getnumber( argu, &nr ) <= 0 || !(obj = find_off_obj( pawn_list, nr) ) )
		{
			sendf( ch, "%s tells you, 'You don't have anything like that. Try list.'", keeper->moved );
			return TRUE;
		}

		spell_identify( 0, ch, 0, 0, 0, obj );

		return TRUE;
	}
	return FALSE;
}

static int pawn_func( charType * ch,  int cmd, char * argu )
{
	charType 	* 	temp;

	for( keeper = 0, temp = world[ch->in_room].people; temp; temp = temp->next_in_room )
	{
		if( IS_NPC(temp) && mobiles[temp->nr].virtual == MOBILE_PAWN ) 
		{
			keeper = temp;
			break;
		}
	}

	if( !keeper || keeper->in_room != pawn_room_nr ) return FALSE;

	switch( cmd )
	{
		case COM_EXAMINE	: return pawn_examine( ch, argu );
		case COM_LIST		: pawn_list_item( ch, argu ); 	return TRUE;
		case COM_BUY		: pawn_buy( ch, argu );		return TRUE;
		case COM_SELL		: pawn_sell( ch, argu );	return TRUE;
		case COM_VALUE		: pawn_value( ch, argu ); 	return TRUE;
	}
	return FALSE;
}

void init_pawn( void )
{
	roomType		*	pawn;
	objectType		*	obj, * obj_next;
	int					keeper_nr;

	if( pawn_room_nr = real_roomNr( ROOM_PAWN ), pawn_room_nr == NOWHERE )
	{
		DEBUG( "init_pawn> can't find pawn room" );
		return;
	}

	if( keeper_nr = real_mobileNr( MOBILE_PAWN ), keeper_nr == MOBILE_NULL )
	{
		DEBUG( "init_pawn> can't find pawn mobile" );
		return;
	}

	pawn = &world[pawn_room_nr];
	mobiles[keeper_nr].func = pawn_func;

	unstash_off_world( "pawn", &pawn_list );

	for( obj = pawn_list; obj; obj = obj_next )
	{
		obj_next = obj->next_content;

		if( obj->contains ) 
		{
			detach_off_obj( &pawn_list, obj );
			extract_obj( obj, 0 );
		}
	}
	renum_off_list( pawn_list );
}
