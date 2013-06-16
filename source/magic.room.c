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
#include "affect.h"

void spell_all_heal( int level, charType *ch, char * arg, int type, 
					charType *victim, objectType *obj)
{
    charType *tmp_victim, *temp;
   
    sendf( ch, "You feel some saintness!" );
    act("$n is glowing with saintly aura!", FALSE, ch, 0, 0, TO_ROOM);
    
	/*
  	if (GET_ALIGN(ch) >= 350) GET_ALIGN(ch) = GET_ALIGN(ch)+125;
  	else if (GET_ALIGN(ch) > -350) GET_ALIGN(ch) = GET_ALIGN(ch)+100; 
  	else GET_ALIGN(ch) = GET_ALIGN(ch)+75;
  	*/
    
    for( tmp_victim = char_list; tmp_victim; tmp_victim = temp )
    {
        temp = tmp_victim->next;
        if( (ch->in_room == tmp_victim->in_room) && (!IS_NPC(ch)) )
        {
            spell_heal(level, ch, "", SPELL_SPELL, tmp_victim, obj); 
        }
        else if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
            sendf( victim, "You see some aura which make you comfort.\n\r" );
    }
}   

void spell_hand_of_god( int level, charType *ch, char * arg, int type,
                        charType *victim, objectType *obj)
{ 
    int                 dam;
    charType *  temp;
    charType *  tmp_victim;
                

    dam = number( wis_rate( ch, 160, 30 ), wis_rate( ch, 160, 40 ) );
 
    send_to_char("Thousand hands are filling all your sight.\n\r", ch);
    act("$n summoned unnumerable hands.\n\rYour face is slapped by hands. BLOOD ALL OVER!\n\r"
    ,FALSE, ch, 0, 0, TO_ROOM);
 
    for( tmp_victim = char_list; tmp_victim; tmp_victim = temp )
    {
    	temp = tmp_victim->next;
    	if( (ch->in_room == tmp_victim->in_room) && is_allow_kill( ch, tmp_victim) ) 
		{
      		damage( ch, tmp_victim, dam, SPELL_HAND_OF_GOD );
    	} 
		else if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
      	send_to_char("Dirty hands with long nail is going and coming all over sky.\n\r", tmp_victim);
  	}
}      
  
void spell_fire_storm( int level, charType *ch, char * arg, int type,
                        charType *victim, objectType *obj)
{   
    int     dam;
    charType    * tmp_victim, *temp;
    
    dam = number( int_rate( ch, 100, 22 ), int_rate( ch, 100, 36 ) );
    
    send_to_char("The fire storm is flowing in the air!\n\r", ch);

    act("$n makes the fire storm flowing in the air.\n\rYou can't see anything!\n\r"
            ,FALSE, ch, 0, 0, TO_ROOM);
    for( tmp_victim = char_list; tmp_victim; tmp_victim = temp )
    {
        temp = tmp_victim->next;
        if( (ch->in_room == tmp_victim->in_room) && is_allow_kill( ch, tmp_victim) )
        {
            damage(ch, tmp_victim, dam, SPELL_FIRE_STORM);
        }
        else if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
            send_to_char("The fire storm is flowing in the atmosphere.\n\r", tmp_victim);
    }
}   
 
void spell_earthquake( int level, charType *ch, char * arg, int type,
                        charType *victim, objectType *obj)
{ 
    int                 dam;
    charType *  tmp_victim, *temp;
    
    dam = dice(1,5)+level;
        
    dam = wis_rate( ch, dam, 10 );
        
    sendf( ch, "The earth trembles beneath your feet!" );
    act("$n makes the earth tremble and shiver\n\rYou fall, and hit yourself!",
            FALSE, ch, 0, 0, TO_ROOM);
            
    for(tmp_victim = char_list; tmp_victim; tmp_victim = temp)
    {
        temp = tmp_victim->next;
        if( (ch->in_room == tmp_victim->in_room) && is_allow_kill( ch, tmp_victim ) ) 
        {               
            damage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
        }               
        else
            if(world[ch->in_room].zone == world[tmp_victim->in_room].zone)
                send_to_char("The earth trembles and shivers.\n\r", tmp_victim);
    }
}

void spell_time_stop( int level, charType *ch, char * arg, int type,
                        charType *victim, objectType *obj)
{
	charType 		* 	tmp, * tmp_next;
	roundAffType		rf;

	memset( &rf, 0, sizeof( rf ) );

	rf.type      = SPELL_TIME_STOP + ROUND_SPELL_TYPE;
	rf.duration  = 3;
	rf.modifier  = level;
	rf.bitvector = AFF_TIME_STOP;
	rf.location  = 0;

    for( tmp = char_list; tmp; tmp= tmp_next )
    {
        tmp_next = tmp->next;

        if( (ch->in_room == tmp->in_room) && (ch != tmp) )
        {
			roundAff_to_char( tmp, &rf );
			sendf( tmp,  "You feel the world is stopped." );
			WAIT_STATE( tmp, PULSE_VIOLENCE * 4 );
        }
		else if( world[ch->in_room].zone == world[tmp->in_room].zone )
			sendf( tmp, "Your world is suffering a distortion of time." );
		else
			sendf( tmp, "You sense a distortion of time." );
    }
}
