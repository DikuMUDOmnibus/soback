#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "character.h"
#include "object.h"
#include "mobile.h"
#include "world.h"
#include "sockets.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "limits.h"
#include "variables.h"
#include "fight.h"
#include "misc.h"
#include "strings.h"
#include "affect.h"
#include "transfer.h"
#include "hash.h"
#include "queue.h"
#include "allocate.h"

void do_sneak( charType * ch, char * argument, int cmd )
{
  	affectType 		af;
  	int 			percent;

	if( IS_NPC(ch) ) return;

  	sendf( ch, "Ok, you'll try to move silently for a while." );
  	if( IS_AFFECTED(ch, AFF_SNEAK) )
    	affect_from_char(ch, SKILL_SNEAK);
  	percent = number(1,101); /* 101% is a complete failure */
  	if( percent > ch->skills[SKILL_SNEAK] + dex_app_skill[GET_DEX(ch)].sneak) return;

  	af.type = SKILL_SNEAK;
  	af.duration = GET_LEVEL(ch);
  	af.modifier = 0;
  	af.location = APPLY_NONE;
  	af.bitvector = AFF_SNEAK;
  	affect_to_char(ch, &af);
}

void do_hide(charType *ch, char *argument, int cmd)
{
  	int percent;

	if( IS_NPC(ch) ) return;

  	sendf( ch, "You attempt to hide yourself." );

  	if( IS_AFFECTED(ch, AFF_HIDE) )	REMOVE_BIT( ch->affects, AFF_HIDE );

  	percent=number(1,101); /* 101% is a complete failure */

  	if( percent > ch->skills[SKILL_HIDE] + dex_app_skill[GET_DEX(ch)].hide )
    	return;

  	SET_BIT(ch->affects, AFF_HIDE);
}

void do_steal(charType *ch, char *argument, int cmd)
{
  	charType 		*	victim;
  	objectType 		*	obj;
  	char 				victim_name[240];
  	char 				obj_name[240];
  	int 				percent;
  	int 				gold, eq_pos;
  	int 				ohoh = FALSE;

  	argument = twoArgument(argument, obj_name, victim_name );

  	if( !(victim = find_char_room(ch, victim_name)) ) 
  	{
    	sendf( ch, "Steal what from who?" ); return;
  	} 
  	else if( victim == ch ) 
  	{
    	sendf( ch, "Come on now, that's rather stupid!" ); return;
  	}
  	if( (!IS_NPC(victim))&&(nostealflag) )
  	{
    	act("Oops..", FALSE, ch,0,0,TO_CHAR);
    	act("$n tried to steal something from you!",FALSE,ch,0,victim,TO_VICT);
    	act("$n tries to rob $N.", TRUE, ch, 0, victim, TO_NOTVICT);
    	return;
  	}

  	if( !IS_NPC(victim) && !(victim->desc ) )
  	{
    	send_to_char("Steal something from the ghost? No way!\n\r", ch);
    	return;
  	}

  	percent = number(1,101)+2*GET_LEVEL(victim)-GET_LEVEL(ch)-dex_app_skill[GET_DEX(ch)].p_pocket;

  	if( stricmp(obj_name, "coins") && stricmp(obj_name,"gold") ) 
  	{
    	if( !(obj = find_obj_list( ch, obj_name, victim->carrying, 1)) ) 
		{
      		for( eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++ )
	  		{
        		if(  victim->equipment[eq_pos] 
				  && (isoneof(obj_name, victim->equipment[eq_pos]->name)) 
				  && can_see_obj(ch,victim->equipment[eq_pos]) ) 
				{
          			obj = victim->equipment[eq_pos];
          			break;
        		}
      			if( !obj ) 
	  			{
        			act("$E has not got that item.",FALSE,ch,0,victim,TO_CHAR);
        			return;
      			} 
	  			else 
				{
        			sendf( ch, "Steal the equipment? Impossible!" );
        			return;
      			}
	  		}
    	} 
		else 
		{
      		percent += obj->weight; /* Make heavy harder */
      		if( AWAKE(victim) && (percent > ch->skills[SKILL_STEAL]) ) 
	  		{
        		ohoh = TRUE;
        		act("Oops..", FALSE, ch,0,0,TO_CHAR);
        		act("$n tries to rob $N.", TRUE, ch, 0, victim, TO_NOTVICT);
        		if( !IS_NPC(victim) )
          			act("$n tried to rob you.",TRUE,ch,0,victim,TO_VICT);
      		} 
	  		else 
			{
        		if( (ch->carry_items + 1 < can_carry_number(ch)) ) 
				{
          			if( (ch->carry_weight + obj->weight) < can_carry_weight(ch) ) 
		  			{
            			obj_from_char(obj);
            			obj_to_char(obj, ch);
            			send_to_char("Got it!\n\r", ch);
            			if( !IS_NPC(victim) )
              				act("$n robbed you!",TRUE,ch,0,victim,TO_VICT);
          			}
          			else
            			sendf( ch, "You cannot carry that much weight." );
        		} 
				else
          			sendf( ch, "You cannot carry that much." );
      		}
    	}
  	} 
  	else 
  	{
    	if( AWAKE(victim) && (percent > ch->skills[SKILL_STEAL]) ) 
		{
      		ohoh = TRUE;
      		act("Oops..", FALSE, ch,0,0,TO_CHAR);
      		act("$n tries to steal gold from $N.",TRUE,ch,0,victim,TO_NOTVICT);
      		if( !IS_NPC(victim) )
        		act("$n tried to rob you.",TRUE,ch,0,victim,TO_VICT);
    	} 
		else 
		{
      		gold = (int) ( (GET_GOLD(victim)*number(10,25))/100 );
      		if( gold > 0 && (victim->gold > gold) ) 
	  		{
        		GET_GOLD(ch) += gold;
        		GET_GOLD(victim) -= gold;
        		sendf( ch, "Bingo! You got %d gold coins.", gold);

        		if(!IS_NPC(victim)) act("$n robbed you.",TRUE,ch,0,victim,TO_VICT);
      		} 
	  		else 
        		sendf( ch, "You couldn't get any gold..." );
    	}
  	}
  	if( ohoh && AWAKE(victim) )
    	if( !IS_SET(victim->act, ACT_NICE_THIEF) )
      		hit( victim, ch, TYPE_UNDEFINED );
}

static int _find_done( charType * ch, int room, char * name )
{
	if( IS_NPC( ch ) ) 	  return find_player_room_at( ch, room, name ) != 0;
	else if( OMNI( ch ) ) return find_char_room_at( ch, room, name ) != 0;
	else				  return find_mob_room_at( ch, room, name ) != 0;
}

static int _find_path( charType * ch, char * toFind, int depth, int mode, int in_zone )
{
	directionType	*	dir;
	roomType		*	start;
	roomType		*	here;
	queueType		*	queue;
	queueNode		*	node;
   	char			*	path;
   	char			*	temp = "";
   	char			*	hash;
   	int 				found 	    = 0;
   	int					passlock	= 0;
   	int					thrudoor	= 0;
   	int					toRoom;
   	int					count;
   	int					door;
   	int					i;

  	if( _find_done( ch, ch->in_room, toFind ) ) return -2;

	if( mode == 2 )
	{
		passlock = 1; 	thrudoor = 1;
	}
   	else if( mode == 1 ) 
   	{ 	
   		thrudoor = 1;
   	}

  	start = &world[ch->in_room];

	hash  = (char *)errMalloc( room_index_info.used );
	memset( hash, 0, room_index_info.used );

  	hash[ ch->in_room ] = 1;

  	queue = init_queue();
  	queue_add( queue, &world[ch->in_room], (int)strdup("") );

	for( count = 0; count < depth && !found && queue->hasNode; count++ )
	{
		node = queue->tail; 

		here = (roomType *)node->data;
		path = (char *)    node->nr;

		for( i = 0; i < 6 && !found; i++ )
		{
			if( dir = here->dirs[i], !dir || (toRoom = dir->to_room) == NOWHERE ) continue;
			if( hash[toRoom] ) continue;
			if( here->zone != start->zone && in_zone ) continue;
    		if( IS_NPC( ch ) && (world[toRoom].flags & NO_MOB) ) continue;

			if( dir->exit_info & EX_CLOSED )
			{
				if( !thrudoor ) continue;
				else			door |= 8;
			}

			if( dir->exit_info & EX_LOCKED )
			{
				if( !passlock ) continue;
				else			door |= 16;
			}

			temp = (char *)errMalloc( strlen( path ) + 2 );
			strcpy( temp, path );
			sprintf( temp + strlen( temp ), "%c", dirs[i][0] );

			if( _find_done( ch, toRoom, toFind ) )
			{
				DEBUG( "==> %s (%d) tracks %s (%d) .. (%d depth - %d trace - %d wide)",
						ch->name, world[ch->in_room].virtual,
						toFind, world[toRoom].virtual,
						strlen( temp ), count, queue->hasNode );
				found = 1;
			}
			else
			{
				hash[ toRoom ] = 1;
				queue_add( queue, &world[toRoom], (int)temp );	
			}
		}
		errFree( path );
		queue_delete_tail( queue );
	}

  	destroy_queue( queue );
  	errFree( hash );

  	if( found )
  	{
		DEBUG( "==> compiled path : %s", temp );
		switch( temp[0] )
		{
			case 'n' : return 0;
			case 'e' : return 1;
			case 's' : return 2;
			case 'w' : return 3;
			case 'u' : return 4;
			case 'd' : return 5;
		}
	}
	else if( count >= depth ) return -3;
	return -1;
}

void do_track( charType * ch, char * argument, int cmd )
{
   	char 					name[256];
   	int 					depth, code;

	if( IS_NPC( ch ) ) return;

  	oneArgument(argument, name);

	if( GET_MOVE( ch ) < 10 )
	{
		sendf( ch, "Try when you can track some mob down. You are worn-out." );
		return;
	}

 	GET_MANA( ch ) -= GET_INT( ch );

  	if( !find_char( ch, name ) ) 
  	{
    	sendf( ch, "You are unable to find traces of one." );
    	return;
  	}

   	depth = ch->skills[SKILL_TRACK];

   	if( IS_SET( ch->class, CLASS_THIEF ) ) 
   	{
     	depth *= GET_INT( ch );
   	}

  	if( OMNI( ch ) ) depth = room_index_info.used;
 
  	if( depth == 0 )
  	{
    	sendf( ch, "You don't see any traces of one." );
		return;
	}
 
   	WAIT_STATE( ch, PULSE_VIOLENCE / 4 );

    if( IS_LIGHT( ch->in_room ) || ch->level > IMO ) 
	{
  		if( ch->level >= IMO )
     		code = _find_path( ch, name, depth, 2, 0 );
  		else
     		code = _find_path( ch, name, depth, 2, 0 );

  		if( code < 0 )
  		{
  			if( code == -2 )	  sendf( ch, "Are you blind?" );
  			else if( code == -3 ) sendf( ch, "You have lost the track." );
  			else 				  sendf( ch, "You see some faint trail to the......." );
		}
		else 
       		sendf( ch, "You see traces of your quarry to the %s", dirs[code] );
    } 
	else 
	{
		sendf( ch, "It's too dark in here to track..." );
		return;
	}
}
