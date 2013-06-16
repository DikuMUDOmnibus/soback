/* ************************************************************************
*  file: magic.c , Implementation of spells.              Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "mobile.action.h"
#include "utils.h"
#include "comm.h"
#include "magics.h"
#include "find.h"
#include "limits.h"
#include "fight.h"
#include "interpreter.h"
#include "variables.h"
#include "weather.h"
#include "affect.h"
#include "misc.h"

void spell_magic_missile( int level, charType *ch, char * arg, int type, 
							charType *victim, objectType *obj )
{
  	int dam;

  	dam = number( int_rate( ch, 10, 1 ), int_rate( ch, 10, 5 ) );

  	if( saves_spell( ch, victim, SAVING_SPELL) )
    	dam >>= 1;

  	damage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}

void spell_chill_touch( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj )
{
  	affectType 	af;
  	int 					dam;

  	dam = number( int_rate( ch, 15, 3 ), int_rate( ch, 15, 7 ) );

  	if( !saves_spell( ch, victim, SAVING_SPELL) )
  	{
    	af.type      = SPELL_CHILL_TOUCH;
    	af.duration  = 6;
    	af.modifier  = -2;
    	af.location  = APPLY_STR;
    	af.bitvector = 0;
    	affect_join( victim, &af, TRUE, FALSE );
  	} 
  	else 
  	{
    	dam >>= 1;
  	}
  	damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}

void spell_burning_hands( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj )
{
  	int 	dam;

  	dam = number( int_rate( ch, 22, 5 ), int_rate( ch, 22, 13 ) );

  	if( saves_spell( ch, victim, SAVING_SPELL) )
    	dam >>= 1;

  	damage(ch, victim, dam, SPELL_BURNING_HANDS);
}

void spell_shocking_grasp( int level, charType *ch, char * arg, int type, 
  							charType *victim, objectType *obj )
{
  	int 	dam;

  	dam = number( int_rate( ch, 30, 7 ), int_rate( ch, 30, 15 ) );

  	if( saves_spell( ch, victim, SAVING_SPELL) ) 
    	dam >>= 1;
    
  	if( !saves_spell( ch, victim, SAVING_PARA) )
    	GET_POS(victim) = POSITION_STUNNED;

  	damage( ch, victim, dam, SPELL_SHOCKING_GRASP );
}

void spell_lightning_bolt( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	int 	dam;

  	dam = number( int_rate( ch, 40, 10 ), int_rate( ch, 40, 18 ) );

  	if( saves_spell( ch, victim, SAVING_SPELL) )
    	dam >>= 1;

  	damage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}



void spell_colour_spray( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	int 	dam;

  	dam = number( int_rate( ch, 60, 13 ), int_rate( ch, 60, 22 ) );

  	if( saves_spell( ch, victim, SAVING_SPELL) )
    	dam >>= 1;

  	damage(ch, victim, dam, SPELL_COLOUR_SPRAY);
}

void spell_energy_drain( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	int 	dam, xp, mana;

  	if ( !saves_spell( ch, victim, SAVING_SPELL) ) 
	{
		GET_ALIGN(ch) = MAX( -999, GET_ALIGN(ch) - 333 );

		if (GET_LEVEL(victim) <= 2) 
		{
			damage(ch, victim, 200, SPELL_ENERGY_DRAIN); /* Kill the sucker */
		} 
		else 
		{
			xp = number(level>>1,level)*1000;
			gain_exp(victim, -xp);
			dam = dice(1,GET_LEVEL(ch));
			mana = GET_MANA(victim)>>2;
			GET_MOVE(victim) >>= 1;
		  	GET_MANA(victim) = mana;
		  	GET_MANA(ch) += mana>>1;
		  	GET_HIT(ch) += dam;
		  	send_to_char("Your life energy is drained!\n\r", victim);
      		damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
		}
	} 
	else 
	{
			damage(ch, victim, 0, SPELL_ENERGY_DRAIN); /* Miss */
  	}
}

void spell_fireball( int level, charType *ch, char * arg, int type,
  								charType *victim, objectType *obj )
{
  	int 	dam;

  	dam = number( int_rate( ch, 100, 20 ), int_rate( ch, 100, 30 ) );

  	if( saves_spell( ch, victim, SAVING_SPELL) )
    	dam >>= 1;

  	if( OUTSIDE(ch) && (weather_info.sky == SKY_CLOUDLESS) ) 
  	{
    	sendf( ch, "Your fireball works excellent!!" );
    	act("$n suffers from hot ball.", FALSE, victim, 0,0,TO_ROOM);
    	dam <<= 1;
    }
  	damage( ch, victim, dam, SPELL_FIREBALL );
}

void spell_disintergrate( int level, charType *ch, char * arg, int type,
  										charType *victim, objectType *obj)
{
  	int 	dam;

  	dam = number( int_rate( ch, 200, 33 ), int_rate( ch, 200, 40 ) );

  	if( saves_spell( ch, victim, SAVING_SPELL) ) 	dam >>= 1;
  	if( IS_AFFECTED(victim, AFF_SANCTUARY) ) 		dam <<= 1;

  	damage( ch, victim, dam, SPELL_DISINTERGRATE );
}

void spell_sunfire( int level, charType * ch, char * arg, int type, 
									charType * victim, objectType * obj )
{
	charType * vict;
	int dam;

	do_yell( ch, "SUNFIRE", 0 );

  	act("The outside sun flares brightly, flooding the room with searing rays.", 
  			TRUE, ch, 0, 0, TO_ROOM); 

  	for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room )
  	{
    	if( is_fighting( vict ) && vict->fight == ch )
		{
			dam = (GET_LEVEL(ch)) * (GET_LEVEL(vict) / 3);
			if( saves_spell( ch, vict, SAVING_SPELL) )
					dam = dam * 2 / 3;
			damage(ch, vict, dam, TYPE_UNDEFINED);
		}
  	}
}

void spell_corn_of_ice( int level, charType *ch, char * arg, int type,
  										charType *victim, objectType *obj)
{
 	int 	dam;

  	dam = number( int_rate( ch, 200, 30 ), int_rate( ch, 200, 40 ) );

  	if( saves_spell( ch, victim, SAVING_SPELL) )
    	dam >>= 1;
  	damage(ch, victim, dam, SPELL_CORN_OF_ICE);
}

void spell_sunburst( int level, charType *ch, char * arg, int type,
  										charType *victim, objectType *obj)
{
  	int 	dam, true = 0;

 	dam = number( wis_rate( ch, 120, 25 ), wis_rate( ch, 120, 30 ) );

  	if( OUTSIDE(ch) && (weather_info.sky == SKY_CLOUDLESS) ) 
  	{
    	sendf( ch, "Your sunburst works excellent!!" );
    	act("$n suffers from hot burst.", FALSE, victim, 0,0,TO_ROOM);
    	dam <<= 1;
		true = 1;
    }

  	if( saves_spell( ch, victim, SAVING_SPELL) )
    	dam >>= 1;
  	else if( (true == 1 || number(1,15) == 1) ||  dice(2, 9) > GET_LEVEL(victim)-level+10 )
    	spell_blindness( level-3, ch, "", SPELL_SPELL, victim, 0) ;

  	damage(ch, victim, dam, SPELL_SUNBURST);
}

void spell_energyflow( int level, charType *ch, char * arg, int type, 
										charType *victim, objectType *obj)
{  
	int 	dam = 0;

  	if( GET_EXP(ch) > 100000 )
  	{  
  		dam = number(level,3)+200+level*10;
     	sendf( ch, "Energyflow which is made of your Exp Quaaaaaaaaaaaaaaaaaa!\n\r" );

  		if( saves_spell( ch, victim, SAVING_SPELL) )
    		dam >>= 1;
   		damage(ch, victim, dam, SPELL_ENERGY_FLOW);

  		if( ch->exp > (50000 + (dam * (ch->level)))) ch->exp -= 50000 + (dam * (ch->level));
  		else ch->exp = 0;
  	}
  	else sendf( ch, "You have too little experience" );
}

void spell_full_fire( int level, charType *ch, char * arg, int type, 
										charType *victim, objectType *obj)
{  
    int dam=0;

    sendf( ch, "You concentrated your energy to small bright sphere" );
    act( "$n concentrated energy to small bright sphere.\n\r", FALSE,ch,0,0,TO_ROOM);

    dam = int_rate( ch, GET_MANA(ch) * 3 / 5, GET_LEVEL(ch) );
 
    sendf( ch, "You throw your energy sphere. You see sudden brightness.\r\n" );

    act( "$n throws the energy sphere.\r\n"
         "You cannot see anything by sudden brghtness.", FALSE,ch,0,0,TO_ROOM);
 
    if( saves_spell( ch, victim, SAVING_SPELL) ) 
    {
        dam >>= 1;
        sendf( ch, "But, You couldn't concentrate enough" );
        GET_MANA(ch) -= GET_MANA(ch) / 11;
    }
    else
    {
        GET_MANA(ch) -= GET_MANA(ch) / 7;
    }

    damage(ch, victim, dam, SPELL_FULL_FIRE );    

    sendf( ch, "You are exhausted.");
}

void spell_throw( int level, charType *ch, char * arg, int type,
										charType *victim, objectType *obj)
{  
    int dam;

    dam = GET_HIT(ch) -= GET_HIT(ch)/2 + 1;
                                                 
    dam = int_rate( ch, dam, ch->level );

    damage( ch, victim, dam, SPELL_THROW );

    send_to_char("Light Solar  guooooooooroorooorooorooooaaaaaaaaaaa!\n\r", ch);
}

void spell_trick( int level, charType * ch, char * arg, int type,
  										charType *victim, objectType *obj)
{
  	charType * tmp_victim;

   	tmp_victim = world[victim->in_room].people;

   	if( tmp_victim )
	{
   		while( !is_allow_kill( victim, tmp_victim) && tmp_victim ) 
   			tmp_victim = tmp_victim->next_in_room;	

   		if( ch->skills[SPELL_TRICK]
		  +(GET_INT(ch)-18)-95
          +level-GET_LEVEL(victim) 
		  +number(1,10)-number(1,25) > 0 ) 
		{
       		hit( victim, tmp_victim, TYPE_UNDEFINED );
      	}
    }
}

void spell_dispel_evil( int level, charType *ch, char * arg, int type,
  											charType *victim, objectType *obj)
{
  	int 	dam;

  	if( IS_EVIL(ch) ) 			victim = ch;
  	else if( IS_GOOD(victim) ) 
  	{
      	act( "God protects $N.", FALSE, ch, 0, victim, TO_CHAR );
      	return;
    }

  	if( (GET_LEVEL(victim) < level) || (victim == ch) ) dam = 100;
  	else 
  	{
    	dam = dice( level, 4 );

		dam = wis_rate( ch, dam, 20 );

    	if( saves_spell( ch, victim, SAVING_SPELL) ) dam >>= 1;
  	}

  	if( IS_EVIL( victim ) && victim != ch ) dam >>= 1;

  	damage(ch, victim, dam, SPELL_DISPEL_EVIL);
}

void spell_call_lightning( int level, charType *ch, char * arg, int type,
  											charType *victim, objectType *obj)
{
  	int 	dam;

	if( !OUTSIDE(ch) || !(weather_info.sky >= SKY_RAINING) ) 
	{
	  	sendf( ch, "You fail to call upon the lightning from the sky!" ); 
	}
	else
	{
  		if( weather_info.sky == SKY_RAINING )   dam = wis_rate( ch, 200, 40 );
		else									dam = wis_rate( ch, 200, 20 );

   		damage(ch, victim, dam, SPELL_CALL_LIGHTNING);
	}
}
