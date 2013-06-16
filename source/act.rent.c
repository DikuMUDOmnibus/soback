#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "defines.h"
#include "character.h"
#include "object.h"
#include "world.h"
#include "mobile.h"
#include "strings.h"
#include "comm.h"
#include "find.h"
#include "stash.h"
#include "fight.h"
#include "transfer.h"

void extract_char( charType * ch )
{
    if( ch->desc ) close_socket( ch->desc ); 

    extract_all_obj( ch );
    char_from_world( ch );
   
    if( !IS_NPC(ch) ) save_char( ch, NOWHERE );
   
    free_a_char( ch );
}

void do_qui(charType *ch, char *argument, int cmd)
{
    sendf( ch, "If you want to quit, type 'quit'." );
}

void do_quit(charType *ch, char *argument, int cmd)
{
    if( IS_NPC(ch) || !ch->desc ) return;

    if( GET_POS(ch) == POSITION_FIGHTING )
    {
        sendf( ch, "No way! You are fighting." ); return;
    }   

    if( GET_POS(ch) < POSITION_STUNNED )
    {
        sendf( ch, "You die before your time!" );
        kill_char( ch, ch->level, 0, 0 );
        return;
    }

    log( "%s quit.", GET_NAME(ch));

    stash_char( ch, 0 );
    save_char( ch, NOWHERE );

    close_socket(ch->desc);
}

void do_police(charType *ch, char *argument, int cmd)
{
  	char 				name[200];
  	descriptorType 	*	d;
  	int 				target;
 
	if( IS_NPC(ch) ) return;

  	if( !*argument ) return;
 
  	oneArgument(argument, name);

  	target=atoi(name);
    
    for( d = desc_list;d; d = d->next )
    {
        if( target == d->fd )
        {   
            if( (d->connected == CON_PLYNG) && (d->character) )
            {
                if( d->character->level < ch->level )
                {
                    if( stash_char(d->character,0) < 0 ) sendf( ch, "Saving victim failed." ) ;
					wipe_all_obj( d->character );
                    extract_char( d->character );
					return;
                }
            }
            close_socket(d);
        }
    }
}

void do_purge( charType *ch, char *argument, int cmd )
{
  	charType 	*vict, *next_v;
  	objectType 	*obj, *next_o;
  	char 				name[256];

  	if( IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3) ) return;

  	oneArgument(argument, name);

  	if( *name ) 
  	{
    	if( (vict = find_char_room(ch, name)) )
		{
      		if( !IS_NPC(vict) )
	  		{
        		if( GET_LEVEL(ch) < GET_LEVEL(vict) )
				{
          			sendf( vict, "%s tried to purge you.",ch->name);
          			return;
        		}

        		if( stash_char(vict,0) < 0 ) sendf( ch, "saving victim failed." ) ;

				wipe_all_obj( vict );
        		extract_char( vict );
       			return;
      		}
      		act( "$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT );
      		act( "You disintegrate $N.", FALSE, ch, 0, vict, TO_CHAR );
      		if( IS_NPC(vict) ) extract_char( vict );
    	} 
		else if( (obj = find_obj_room( ch, name )) )
		{
      		act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      		act("You destroys $p.", FALSE, ch, obj, 0, TO_CHAR);
      		extract_obj( obj, 1 );
    	} 
		else 
		{
      		sendf( ch, "I don't know anyone or anything by that name." );
      		return;
    	}
    	sendf( ch, "Ok." );
  	}
  	else 
  	{
    	if(IS_NPC(ch)) 
		{
      		sendf( ch, "Don't... You would only kill yourself.." );
      		return;
    	}
    	act("$n gestures... You are surrounded by scorching flames!", 
      			FALSE, ch, 0, 0, TO_ROOM);
    	sendrf( 0, ch->in_room, "The world seems a little cleaner." );
    	for( vict = world[ch->in_room].people; vict; vict = next_v) 
		{
      		next_v = vict->next_in_room;

      		if(IS_NPC(vict)) extract_char( vict );
    	}
    	for( obj = world[ch->in_room].contents; obj; obj = next_o ) 
		{
      		next_o = obj->next_content;
      		extract_obj( obj, 1 );
    	}
  	}
}

void do_checkrent(charType *ch,char *argument, int cmd)
{
  	char 		stashfile[100],	name[MAX_STRING_LENGTH];
  	FILE 	*	fl;
	int			scan, i = 0;

	if( IS_NPC(ch) ) return;

  	onefword( argument, name );

  	if( !*name ) return;

	strlwr( name );

  	sprintf( stashfile, "%s/%c/%s.x", STASH_DIR, name[0], name );

  	if( fl = errOpen(stashfile,"r"), !fl )
  	{
  		strcat( stashfile, ".y" );
  		if( fl = errOpen( stashfile, "r" ), !fl )
  		{
    		sendf( ch, "%s has nothing in rent.", name );
    		return;
		}
  	}

  	while( !feof( fl ) )
  	{
  		if( fgets( name, 256, fl ) ) 
		{
			scan = 0;

			if( *name == 'I' ) 		sscanf( name, "%*c %d", &scan );
			else if( *name == 'Q' ) sscanf( name, "%*c %*d %*c %d", &scan );

			if( scan &&	(scan = real_objectNr(scan), scan != OBJECT_NULL ) )
				sendf( ch, "%3d] %35s", ++i, objects[scan].wornd );
		}
	}
  	fclose(fl);
}

void do_checklocker(charType *ch,char *argument, int cmd)
{
  	char 		stashfile[100],	name[MAX_STRING_LENGTH];
  	FILE 	*	fl;
	int			scan, i = 0;

	if( IS_NPC(ch) ) return;

  	onefword( argument, name );

  	if( !*name ) return;

	strlwr( name );

  	sprintf( stashfile, "%s/%s/%c/%s", ROOM_STASH_DIR, "locker", name[0], name );

  	if( fl = errOpen(stashfile,"r"), !fl )
  	{
  		strcat( stashfile, ".y" );
  		if( fl = errOpen( stashfile, "r" ), !fl )
  		{
    		sendf( ch, "%s has nothing in locker.", name );
    		return;
		}
  	}

  	while( !feof( fl ) )
  	{
  		if( fgets( name, 256, fl ) ) 
		{
			scan = 0;

			if( *name == 'I' ) 		sscanf( name, "%*c %d", &scan );
			else if( *name == 'Q' ) sscanf( name, "%*c %*d %*c %d", &scan );

			if( scan &&	(scan = real_objectNr(scan), scan != OBJECT_NULL ) )
				sendf( ch, "%2d] %35s", ++i, objects[scan].wornd );
		}
	}
  	fclose(fl);
}

void do_extractrent(charType *ch,char *argument, int cmd)
{
  	char 		name[MAX_INPUT_LENGTH];

	if( IS_NPC(ch) ) return;

  	onefword( argument, name );

  	if( !*name ) return;

  	unstash_char( ch, name );

  	sendf( ch, "OK." );

  	log( "%s grabbed rent for %s", GET_NAME(ch), name );
}

void do_replacerent( charType * ch, char * argument, int cmd )
{
  	char 	name[MAX_INPUT_LENGTH];

	if( IS_NPC(ch) ) return;

  	onefword( argument, name );

  	if( !*name ) return;

  	if( stash_char(ch, name ) < 0 ) 
  	{
		sendf( ch, "saving victim failed." ) ;
		return;
	}

  	sendf( ch, "OK." );

  	log( "%s replaced rent for %s", GET_NAME(ch), name );
}

void do_rent(charType *ch, char *arg, int cmd )
{
	int		save_room;

  	if( IS_NPC(ch) ) return;

  	if( cmd )
	{
    	if( !IS_SET(world[ch->in_room].flags,RENT) )
		{
      		send_to_char("You cannot rent here.\n\r",ch);
      		return;
    	}
    	sendf( ch, "You retire for the night." );
    	act( "$n retires for the night.", FALSE, ch,0,0, TO_NOTVICT );
		senddf( 0, 44, "===> renting %s.", ch->name );
  	}

  	if( stash_char( ch, 0 ) < 0 ) 
	{	
		DEBUG( "do_rent> saving %s failed.", ch->name );
	}

	wipe_all_obj( ch );

	save_room = world[ch->in_room].virtual; save_char( ch, save_room );

	char_from_world( ch );

	ch->in_room = save_room;
	  
	if( !ch->desc ) free_a_char( ch );
}
