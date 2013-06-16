#include <ctype.h>
#include <unistd.h>

#include "character.h"
#include "world.h"
#include "mobile.h"
#include "object.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "limits.h"
#include "strings.h"
#include "utils.h"
#include "stash.h"
#include "transfer.h"

#define MAX_LOCKER_ITEM		40

void list_locker_objs( charType * ch, char * argu )
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
		"about",
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

	if( !argu || !*buf || what == -1 )
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
			sendf( ch, "Huh? try list.'" );
			return;
		}
	}

	init_search_records();

	for( i = 0, obj = ch->off_world; obj; obj = obj->next_content )
	{
		if( (what == 18)
         || (what == 0 && obj->type == ITEM_LIGHT) 
		 || (what == 19 && isoneofp( buf, obj->name ))
		 || (what > 0 && what < 18 && IS_WEAR( obj, (1 << what)) ) ) record_found_object( obj, 0 );
	}

	if( isnumstr( argu ) ) 
		if( getnumber( argu, &page ), --page < 0 ) page = 0;

	sendf( ch, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	sendf( ch, "+   Locker of %-12s                                                  +", ch->name);

	if( !objs_in_found_table ) 
	{
		sendf( ch, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
		sendf( ch, "  Nothing.  " );
		return;
	}
	else
	{
		sendf( ch, "+ %3d list of %-20s ( total %3d items in %3d pages )         +",
					 page+1, buf, objs_in_found_table, (objs_in_found_table/20 + 1)  );
		sendf( ch, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
		for( i = page * 20; i < objs_in_found_table && i < ((page + 1) * 20); i++ )
		{
			sendf( ch, "[%3d]%43s -%9s- %-16s", 
				found_objects[i].obj->what,
				found_objects[i].obj->wornd,
				itemstatus( found_objects[i].obj ),
				capitalize(strlwr( sprinttype( found_objects[i].obj->type, item_types, 0 ))) );
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

static int look_locker( charType * ch, char * argu )
{
	char	buf[MAX_INPUT_LENGTH];

	oneArgument( argu, buf );

	if( isoneofp( buf, "locker cabinet" ) )
	{
		list_locker_objs( ch, "all" );
		return 1;
	}
	return 0;
}

static void get_off_obj( charType * ch, objectType * obj )
{
    if( ch->carry_items + 1 > can_carry_number(ch) )
    { 
        sendf( ch, "You can't get %s, too many items." , oneword( obj->name ) );
        return;
    } 
      
    if( (ch->carry_weight + obj->weight) > can_carry_weight(ch) )
    { 
        sendf( ch, "You can't get %s, too much weight.", oneword( obj->name ) );
        return;
    } 
    
    detach_off_obj( &ch->off_world, obj );
    renum_off_list( ch->off_world );
      
	obj_to_in_world( obj );
    obj_to_char( obj, ch );
      
    act( "$n gets $p from $s cabinet.", FALSE, ch, obj, 0, TO_ROOM );
    sendf( ch, "You get %s from your cabinet.", obj->wornd );
}

int get_obj_from_locker( charType * ch, char * argu )
{
	char			item[MAX_INPUT_LENGTH];
	char			locker[MAX_INPUT_LENGTH];
	objectType	*	obj = 0, * obj_next;
	int				objNr, ofound;
	char		* 	what;
	int				done = 0;

	twoArgument( argu, item, locker );

	if( !isoneofp( locker, "locker cabinet" ) ) return 0;

	if( isnumstr( item ) )
	{
		if( ( getnumber( item, &objNr ), objNr <= 0 )
         || ( obj = find_off_obj( ch->off_world, objNr ), obj ) )
		{
			get_off_obj( ch, obj );
			done = 1;
		}
	}
	else
	{
		if( objNr = splitarg( item,  &what ), objNr < 0 )
		{
			sendf( ch, "Eh?? get what from your locker?" ); return 1;
		}

        for( ofound = 0, obj = ch->off_world; obj; obj = obj_next )
        {
			obj_next = obj->next_content;  

			if( (!*what || isoneof( what, obj->name )) && can_see_obj( ch, obj ) )
			{
				if( ++ofound < objNr && objNr ) continue; 

				get_off_obj( ch, obj );
				done = 1;

				if( ofound == objNr ) break;
            } 
		}
	}

	if( !done ) sendf( ch, "You don't see any such thing in your cabinet." );
	else
	{
		char	fname[200];

		sprintf( fname, "locker/%c/%s", tolower(*ch->name),  ch->name );
		strlwr( fname );
		stash_off_world( fname, ch->off_world );
	}

	return 1;
}

int count_num_objs( objectType * obj )
{
	int 	nr;

	for( nr = 0; obj; obj = obj->next_content )
	{
		if( obj->contains ) nr += count_num_objs( obj->contains );
		nr++;
	}

	return nr;
}

int put_obj_to_locker( charType * ch, char * argu )
{
	char			item[MAX_INPUT_LENGTH];
	char			locker[MAX_INPUT_LENGTH];
	objectType	*	obj, * obj_next;
	int				objNr, ofound, offNr, i;
	char		* 	what;
	int				done = 0;

	twoArgument( argu, item, locker );

	if( !isoneofp( locker, "locker cabinet" ) ) return 0;

	if( objNr = splitarg( item,  &what ), objNr < 0 )
	{
		sendf( ch, "Eh?? put what to your locker?" ); return 1;
	}

	if( offNr = count_num_objs( ch->off_world ), offNr >= MAX_LOCKER_ITEM )
	{
		sendf( ch, "Your locker is full." );
		return 1;
	}

    for( ofound = 0, obj = ch->carrying; obj; obj = obj_next )
    {
		obj_next = obj->next_content;  

		if( (!*what || isoneof( what, obj->name )) && can_see_obj( ch, obj ) )
		{
			if( ++ofound < objNr && objNr ) continue; 

			if( (offNr + (i = count_num_objs( obj->contains ) ) + 1) > MAX_LOCKER_ITEM )
			{
				sendf( ch, "%s : Your locker won't fit.", obj->wornd );
				continue;
			}

			if( !is_stashable( obj ) )
			{
				sendf( ch, "%s : You can't put this in your locker.", obj->wornd );
				continue;
			}

			offNr += i + 1;

			obj_from_char( obj );
			obj_to_off_world( obj );

			obj->next_content = ch->off_world;
			ch->off_world = obj;

			renum_off_list( ch->off_world );

			act( "$n puts $p to $s cabinet.", FALSE, ch, obj, 0, TO_ROOM );
			sendf( ch, "You put %s to your cabinet.", obj->wornd );

			done = 1;

			if( ofound == objNr ) break;
        } 
	}

	if( !done ) sendf( ch, "You don't see any such thing in your inventory." );
	else
	{
		char	fname[200];

		sprintf( fname, "locker/%c/%s", tolower(*ch->name),  ch->name );
		strlwr( fname );
		stash_off_world( fname, ch->off_world );
	}

	return 1;
}

void char_to_locker( charType * ch )
{
	char		fname[200];

	if( IS_NPC(ch) ) return;
	sprintf( fname, "locker/%c/%s", tolower(*ch->name),  ch->name );
	strlwr( fname );
	unstash_off_world( fname, &ch->off_world );
	renum_off_list( ch->off_world );
}

void wipe_locker( char * name )
{
	char		fname[200];

	sprintf( fname, "%s/locker/%c/%s", ROOM_STASH_DIR, tolower(*name), name );
	strlwr( fname );
	unlink( fname );
}

void move_locker( char * name, char * to )
{
	objectType	* 	stash;
	char			fname[200], tname[200];

	sprintf( fname, "%s/locker/%c/%s", ROOM_STASH_DIR, tolower(*name), name );
	sprintf( tname, "%s/locker/%c/%s", ROOM_STASH_DIR, tolower(*to), to );
	unstash_off_world( fname, &stash );
	stash_off_world( tname, stash );
	strlwr( fname );
	unlink( fname );
}

void char_from_locker( charType * ch )
{
	objectType	*	obj, * obj_next;

	if( IS_NPC(ch) ) return;

	for( obj = ch->off_world; obj; obj = obj_next )
	{
		obj_next = obj->next_content;
		extract_obj( obj, 0 );
	}
	ch->off_world = 0;
}

int locker_room( charType * ch, int cmd, char * argu )
{
	if( !cmd || !ch || IS_NPC( ch ) ) return 0;

	switch( cmd )
	{
		case	COM_LIST	:	list_locker_objs( ch, argu ); return 1;
		case	COM_PUT		:	return put_obj_to_locker( ch, argu );
		case	COM_GET		:	return get_obj_from_locker( ch, argu );
		case	COM_LOOK	:	return look_locker( ch, argu );
	}

	return 0;
}

void init_locker( void )
{
	int		rNr;

	if( rNr = real_roomNr( ROOM_LOCKER ), rNr != NOWHERE )
	{
		world[rNr].func = locker_room;
	}
}
