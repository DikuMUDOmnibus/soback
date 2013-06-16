#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "utils.h"
#include "comm.h"
#include "magics.h"
#include "find.h"
#include "limits.h"
#include "fight.h"
#include "interpreter.h"
#include "misc.h"
#include "variables.h"
#include "strings.h"
#include "transfer.h"
#include "affect.h"
#include "specials.h"

void spell_teleport( int level, charType *ch,char * arg, int type, 
		charType *victim, objectType *obj)
{
  	int to_room;

	/* choose room */
	if( IS_NPC(ch) ) return;

	do
	{
		to_room = number( 4, rooms_in_world - 1 );

	} while( world[to_room].flags & ( OFF_LIMITS + PRIVATE + NORELOCATE ) );

  	if( ch->fight ) stop_fighting(ch);
  	act("$n slowly fade out of existence.", FALSE, ch,0,0,TO_ROOM);

  	char_from_room( ch );
  	char_to_room(ch, to_room);

  	act("$n slowly fade in to existence.", FALSE, ch,0,0,TO_ROOM);
  	do_look(ch, "", 0);
}

void spell_far_look( int level, charType *ch,char * arg, int type,
  charType *victim, objectType *obj)
{
  int location,i;

  if (can_see(ch,victim)){
    i=GET_LEVEL(ch)-GET_LEVEL(victim);
    if(i < 0){
      send_to_char("Your spell is not powerful enough for that.\n\r",ch);
      return;
    } 
    location = victim->in_room;
  	} 
  	else 
  	{
    	send_to_char("No such creature around, what a waste.\n\r", ch);
    	return;
  	}
	if( IS_SET(world[location].flags, PRIVATE)    ||
		IS_SET(world[location].flags, NORELOCATE) ||
		IS_SET(world[location].flags, OFF_LIMITS) ) 
	{
		send_to_char( "You fail miserably.\n\r", ch);
		return;
	}
  	send_to_char(world[location].name,ch);
  	send_to_char("\n\r",ch);
  	send_to_char(world[location].description,ch);
  	list_obj_to_char( world[location].contents, ch, 1, FALSE );
  	list_room_chars( ch, world[location].people );
}

void spell_relocate( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  int location,i;

	if( IS_NPC(ch) ) return;

  if (can_see(ch,victim)){
    i=GET_LEVEL(ch)-GET_LEVEL(victim);
    if(i < 0){
      send_to_char("Your spell is not powerful enough for that.\n\r",ch);
      return;
    } else if(number(0,2) > i){
      send_to_char("You make a valiant effort, but barely fail.\n\r",ch);
      send_to_char("Temporary note: This means you had a real chance.\n\r",ch);
      return;
    }
    location = victim->in_room;
  } else {
    send_to_char("No such creature around, what a waste.\n\r", ch);
    return;
  }
  if (IS_SET(world[location].flags, PRIVATE) ||
      IS_SET(world[location].flags, NORELOCATE) ||
      IS_SET(world[location].flags, OFF_LIMITS)) {
    send_to_char( "You fail miserably.\n\r", ch);
    return;
  }
  act("$n disappears in a puff of purple smoke.", FALSE,ch,0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);
  act("$n appears with a modest poof.",FALSE,ch,0,0,TO_ROOM);
  do_look(ch, "",15);
}

void spell_word_of_recall( int level, charType *ch, char * arg, int type,
  								charType *victim, objectType *obj )
{   
	static	int		temple = 0;
  	int 			location;
  	bool 			found = FALSE;
  
  	if(IS_NPC(victim)) return;

	if( temple == 0 )
	{
  		int loc_nr = ROOM_TEMPLE;

  		for( location = 0; location <= rooms_in_world; location++ )
    		if( world[location].virtual == loc_nr) 
			{
      			found=TRUE;
	  			temple = location;
      			break;
    		}
	}
	else location = temple;

  	if( !temple ) 
  	{
  		DEBUG( "spell_word_of_recall> can't find temple." );
    	sendf( ch, "You are completely lost." ); return;
  	}

  	act( "$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  	char_from_room( victim );
  	char_to_room(victim, location);

  	if( ch->fight ) stop_fighting(ch);

  	act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  	do_look(victim, "",15);
  	victim->move = victim->move - 50;
}   

void spell_summon( int level, charType *ch, char * arg, int type, charType *victim, objectType *obj )
{
	roundAffType	rf;

  	if( IS_NPC(victim) )
  	{
    	if( GET_LEVEL(victim) > MIN(20,level+3) ) 
		{
      		sendf( ch, "You failed." );
      		return;
    	}
  	} 
  	else 
  	{
    	if( GET_LEVEL(victim) > GET_LEVEL(ch) )
		{
      		sendf( ch, "You are too humble a soul." );
      		return;
    	}
  	}

  	if( IS_SET(world[victim->in_room].flags, NOSUMMON)
        || IS_SET(world[ch->in_room].flags, NOSUMMON)
        || world[victim->in_room].virtual <= 1 )
  	{
    	sendf( ch, "You nearly succeed, but not quite." );
    	return;
  	}

  	if( IS_NPC(victim) && saves_spell(ch, victim, SAVING_SPELL) )
  	{
    	sendf( ch, "You failed." );
    	return;
  	}

	if( victim->desc && victim->desc->wait > 1 )
	{
    	memset( &rf, 0, sizeof( rf ) );
        
    	rf.type      = SPELL_SUMMON + ROUND_SKILL_TYPE;
    	rf.duration  = 0;

		ch->hunt = victim;

		roundAff_to_char( ch, &rf );

        act( "$n concentrates $s all energy, and reach for the sky.", TRUE, ch, 0, 0, TO_ROOM );
        act( "You concentrate your all energy, and reach for the sky.", TRUE, ch, 0, 0, TO_CHAR );
    }
    else
    {
        int         was_room;
 
        act( "$n murmurs, and reach for the sky.",  TRUE, ch, 0, 0, TO_ROOM );
        act( "You murmur, and reach for the sky.", TRUE, ch, 0, 0, TO_CHAR );
    
        act( "$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM );
 
        was_room = ch->in_room;
   
        char_from_room( victim );
        char_to_room( victim, was_room );

        if( victim->was_in_room != NOWHERE ) victim->was_in_room = was_room ;

		act( "$n arrives suddenly.", TRUE,  victim, 0, 0,  TO_ROOM );
		act( "$n has summoned you!", FALSE, ch, 0, victim, TO_VICT );
		do_look( victim, "", COM_LOOK );
	}
}

void spell_portal( int level, charType *ch, char * arg, int type, charType *victim, objectType *obj )
{
	objectType		* to, * from;

	if( ch == victim || IS_NPC(victim) )
	{
        sendf( ch, "You failed." );
        return;
    }
    else
    {
        if( GET_LEVEL(victim) > GET_LEVEL(ch) )
        {
            sendf( ch, "You are too humble a soul." );
            return;
        }
    }

	if( ch->in_room == victim->in_room )
	{
		sendf( ch, "It's ridiculous." ); return;
	}

    if( IS_SET(world[victim->in_room].flags, NOSUMMON)
        || IS_SET(world[ch->in_room].flags, NOSUMMON)
        || world[victim->in_room].virtual <= 1 )
    {
        sendf( ch, "You nearly succeed, but not nothing happen." );
        return;
    }

    if( IS_NPC(victim) && saves_spell(ch, victim, SAVING_SPELL) )
    {
        sendf( ch, "You failed." );
        return;
	}

	if( find_obj_room( ch, "magicportal" ) || find_obj_room( victim, "magicportal" ) )
	{
		sendf( ch, "Only on portal for one room." ); return;
	}

	to   = create_portal( ch, victim->in_room );
	from = create_portal( ch, ch->in_room );

	obj_to_room( to, ch->in_room );
	obj_to_room( from, victim->in_room );

	record_portal( to ); record_portal( from );

	sendrf( 0, ch->in_room, "You feel some aura from another world is floating in the air." );
	sendf( ch,  "A magic portal appears in the middle of room." );
	act( "A magic portal appears in the middle of room.", FALSE, ch, 0, 0, TO_ROOM );

	sendrf( 0, victim->in_room, "You feel some aura from another world is floating in the air." );
	sendf( victim,  "A magic portal appears in the middle of room." );
	act( "A magic portal appears in the middle of room.", FALSE, victim, 0, 0, TO_ROOM );
}
