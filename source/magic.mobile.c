/* ************************************************************************
*  file: magic.c , Implementation of spells.              Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <assert.h>

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "world.h"
#include "utils.h"
#include "comm.h"
#include "magics.h"
#include "find.h"
#include "limits.h"
#include "fight.h"
#include "mobile.action.h"
#include "variables.h"
#include "strings.h"
#include "affect.h"
#include "transfer.h"

void mob_spell_fire_storm(int level, charType *ch )
{
  	int dam;
  	charType *tmp_victim, *temp;

  	dam = dice( level, 6 );
  	send_to_char("The fire storm is flowing in the air!\n\r", ch);
	act("$n makes the fire storm flowing in the air.\n\rYou can't see anything!\n\r"
    	,FALSE, ch, 0, 0, TO_ROOM);

  	for(tmp_victim = char_list; tmp_victim; tmp_victim = temp) 
  	{
    	temp = tmp_victim->next;
    	if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)	&& !IS_NPC(tmp_victim) ) 
	  	{
      		damage(ch, tmp_victim, dam, SPELL_FIRE_STORM);
    	} 
		else
      		if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
        		send_to_char("The fire storm is flowing in the atmosphere.\n\r", tmp_victim);
  	}
}

void mob_spell_hand_of_god( charType *ch )
{
  	int dam;
  	charType *tmp_victim, *temp;

  	dam = dice( GET_LEVEL( ch ), 10 );
    send_to_char("Thousand hands are filling all your sight.\n\r", ch);
    act("$n summoned unnumerable hands.\n\rYour face is slapped by hands. BLOOD ALL OVER!\n\r"
    	,FALSE, ch, 0, 0, TO_ROOM);

    for( tmp_victim = char_list; tmp_victim; tmp_victim = temp )
    {
    	temp = tmp_victim->next;
    	if( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim) && !IS_NPC(tmp_victim) ) 
		{
      		damage(ch, tmp_victim, dam, SPELL_HAND_OF_GOD);
    	} 
		else if( world[ch->in_room].zone == world[tmp_victim->in_room].zone )
      		send_to_char("Dirty hands with long nail is going and coming all over sky.\n\r", 
				tmp_victim);
  	}
}

void mob_light_move(charType *ch, char *argument, int cmd)
{
  	charType * victim;

  	if( (victim = ch->fight) )
  	{
    	hit( ch, victim, TYPE_UNDEFINED );
    	if( ch->fight ) stop_fighting(ch); 
    	if( victim->fight ) stop_fighting(victim);
    	WAIT_STATE(ch, PULSE_VIOLENCE*2/3);
    }
}

void mob_punch_drop(charType *ch, charType *victim) 
{
  int i;
  char buffer[MAX_STRING_LENGTH];
  objectType *tmp_object;
  objectType *next_obj;
  bool test = FALSE;
  
  /* punch out equipments  */
  for (i=0; i< MAX_WEAR; i++) {
     if (((victim->equipment)[i]) && (number(10,100) < GET_LEVEL(ch)) ) {
       /* obj is removed form player */
       tmp_object = unequip_char(victim,i) ;
       if (can_see_obj(victim, tmp_object)) {
         sprintf(buffer, "%s is punched out.\n\r", oneword(tmp_object->name));
         send_to_char(buffer, victim);
         }
       else {
         send_to_char("Something is punched out.\n\r", victim);
         }
       act("$n's $p flies in the sky for a while and falls.", 1, victim,
           tmp_object, 0, TO_ROOM);
       obj_to_room(tmp_object,ch->in_room);
       }
     }

      /* punch out carrying items..  */
      for(tmp_object = victim->carrying; tmp_object; tmp_object = next_obj) {
        next_obj = tmp_object->next_content;
        if ( GET_LEVEL(ch) > number(10,160) ) {
          /* item nodrop.. but.. can be punched also.. */
          if (can_see_obj(victim, tmp_object)) {
            sprintf(buffer, "%s is punched out.\n\r", oneword(tmp_object->name));
            send_to_char(buffer, victim);
          } else {
            send_to_char("Something is punched out.\n\r", victim);
          }
          act("$n's $p flies in the sky for a while and falls.", 1, victim,
	    tmp_object, 0, TO_ROOM);
          obj_from_char(tmp_object);
          obj_to_room(tmp_object,ch->in_room);
          test = TRUE;
        }
    }
}

void spell_fire_breath( int level, charType *ch, char *arg, int type,
  charType *tar_ch, objectType *tar_obj )
{
  	int dam = 3 * GET_LEVEL(ch);
  	objectType		* burn;
  	objectType		* burn_next;

  	if ( saves_spell( ch, tar_ch, SAVING_BREATH) ) dam >>= 1;

  	if( !IS_NPC( tar_ch) ) damage(ch, tar_ch, dam, SPELL_FIRE_BREATH);

  	if( number(0,IMO) < GET_LEVEL(ch) ) 
  	{
    	if (!saves_spell( ch, tar_ch, SAVING_BREATH) ) 
		{
			for( burn = tar_ch->carrying ; burn && (number(0,4)<=3 ); burn = burn_next ) 
			{
				burn_next = burn->next_content;

				if ((burn->type==ITEM_SCROLL) || 
					(burn->type==ITEM_WAND) ||
					(burn->type==ITEM_STAFF) ||
					(burn->type==ITEM_NOTE)) 
				{
					act("$o burns.",0, tar_ch, burn,0,TO_CHAR);
					extract_obj( burn, 1 );
				}
       		}
	   	}
  	}
}

void spell_frost_breath( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
  	int 			dam, j;
  	objectType 	*	frozen;
  	objectType 	*	frozen_next;

  	dam = 3*GET_LEVEL(ch);

  	if( saves_spell( ch, victim, SAVING_BREATH) ) dam >>= 1;

  	if (!IS_NPC(victim)) damage(ch, victim, dam, SPELL_FROST_BREATH);


  	if( number(0,30) < GET_LEVEL(ch) ) 
  	{
    	if( !saves_spell( ch, victim, SAVING_BREATH) ) 
		{
      		for(frozen = victim->carrying, j = 2; frozen && (number(0,4)<=j) ; frozen = frozen_next ) 
	  		{
				frozen_next = frozen->next_content;

        		if( (frozen->type==ITEM_DRINKCON) 
				 || (frozen->type==ITEM_FOOD) 
                 || (frozen->type==ITEM_POTION) ) 
			 	{
          			act("$o breaks.",0,victim,frozen,0,TO_CHAR);
          			extract_obj( frozen, 1 );
          		}
		  		if( --j == 0 ) break;
       		}
    	}
  	}
}

void spell_gas_breath( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
  	int 					dam;
  	charType			*	tar_ch, * tar_ch_next;
  	objectType 	*	melt, *temp, *next_obj;

	dam = 3 * GET_LEVEL(ch);

   	for( tar_ch = world[ch->in_room].people ; tar_ch ; tar_ch = tar_ch_next )
	{
		tar_ch_next = tar_ch->next_in_room;

   		if( (tar_ch != ch) && (GET_LEVEL(tar_ch) < IMO) )
   		{
  			if( IS_NPC(tar_ch) ) continue;
  			if( saves_spell( ch, tar_ch, SAVING_BREATH) ) dam >>= 1;

  			damage(ch, tar_ch, dam, SPELL_GAS_BREATH);

  			if( number( 15, IMO+20 ) < GET_LEVEL(ch) ) 
  			{
    			if( !saves_spell( ch, tar_ch, SAVING_BREATH) ) 
				{
      				melt = tar_ch->carrying ;
      				if( melt && (number(0,3) < 2) ) 
	  				{
        				act("$o is melting away.",0,tar_ch,melt,0,TO_CHAR);
        				if( melt->type == ITEM_CONTAINER ) 
						{
          					act("Some trash dumped out as $o is broken!",0,tar_ch,melt,0,TO_ROOM);
          					for( temp = melt->contains; temp; temp=next_obj ) 
		  					{
            					next_obj = temp->next_content;
            					obj_from_obj(temp);
            					obj_to_room(temp,tar_ch->in_room);
            				}
          				}
        				extract_obj( melt, 1 );
        			}
      			}
    		}
		}
	}
}

void spell_lightning_breath( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
  	int 			dam, j;
  	objectType * 	explode;
  	objectType * 	explode_next;

  	dam = 3 * GET_LEVEL(ch);

  	if( saves_spell( ch, victim, SAVING_BREATH) ) dam >>= 1;

  	if (!IS_NPC(victim)) damage(ch, victim, dam, SPELL_LIGHTNING_BREATH);

  	if( number(0,IMO) < GET_LEVEL(ch) ) 
  	{
    	if( !saves_spell( ch, victim, SAVING_BREATH) ) 
		{
      		for( explode = victim->carrying, j = 2; explode && (number(0,4)<j); explode=explode_next ) 
	  		{
				explode_next = explode->next_content;

        		if( (explode->type==ITEM_LIGHT) 
          		 ||	(explode->type==ITEM_WEAPON) 
          	 	 ||	(explode->type==ITEM_FIREWEAPON)
          		 ||	(explode->type==ITEM_ARMOR)
          	     ||	(explode->type==ITEM_WORN)) 
		  		{
          			act("$o explodes.",0,victim,explode,0,TO_CHAR);
          			extract_obj( explode, 1 );
          		}
		  		if( --j == 0 ) break;
        	}
    	}
  	}
}
