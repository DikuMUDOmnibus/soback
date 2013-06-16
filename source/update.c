#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "comm.h"
#include "limits.h"
#include "magics.h"
#include "fight.h"
#include "find.h"
#include "sockets.h"
#include "interpreter.h"
#include "variables.h"
#include "update.h"
#include "transfer.h"
#include "utils.h"
#include "affect.h"
#include "misc.h"
#include "weather.h"

void update_pos( charType *victim )
{   
  	if( (GET_HIT(victim) > 0 ) && (GET_POS(victim) > POSITION_STUNNED)) return;
   	else if (GET_HIT(victim) > 0   ) GET_POS(victim) = POSITION_STANDING;
	else if (GET_HIT(victim) <= -11) GET_POS(victim) = POSITION_DEAD;
	else if (GET_HIT(victim) <= -6 ) GET_POS(victim) = POSITION_MORTALLYW;
	else if (GET_HIT(victim) <= -3 ) GET_POS(victim) = POSITION_INCAP;
	else GET_POS(victim) = POSITION_STUNNED;
}

static void update_object( objectType *obj, int use)
{
	int	max;

  	if (obj->timer > 0)  obj->timer -= use;

  	max = (obj->timer+(MAX_OBJ_LIFE_SPAN/100)) / ((MAX_OBJ_LIFE_SPAN) / 100);
  	if( obj->status > max ) obj->status = max;
  	if( obj->limit > max ) obj->limit = max;

  	if (obj->contains) update_object(obj->contains, use);
  	if (obj->next_content) update_object(obj->next_content, use);
}

static void update_char_objects( charType *ch )
{
  	int 		i;

  	if( ch->equipment[WEAR_LIGHT] )
    	if( ch->equipment[WEAR_LIGHT]->type == ITEM_LIGHT)
      		if( ch->equipment[WEAR_LIGHT]->value[2] > 0 )
        		(ch->equipment[WEAR_LIGHT]->value[2])--;

  	for( i = 0; i < MAX_WEAR; i++ ) 
    	if( ch->equipment[i] )
      		update_object( ch->equipment[i], 2 );

  	if( ch->carrying ) update_object( ch->carrying, 1 );
}

static void check_idling(charType *ch)
{
  	if( ++(ch->timer) > 20 && !OMNI( ch ) )
  	{
    	if( ch->was_in_room == NOWHERE && ch->in_room != NOWHERE) 
		{
      		ch->was_in_room = ch->in_room;

      		act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      		send_to_char("You have been idle, and are pulled into a void.\n\r", ch);
      		char_from_room(ch);
      		char_to_room(ch, real_roomNr( ROOM_LIMBO ));
    	} 
		else if (ch->timer >= 40) 
		{
			senddf( 0, 41, "==> Extracting %s.", ch->name );

    		if( stash_char( ch, 0 ) < 0 )
    		{
        		log( "check_idling> saving %s failed.", ch->name );
    		}

    		wipe_all_obj( ch );
 
    		save_char( ch, NOWHERE );

      		if( ch->desc ) 	close_socket( ch->desc );

    		char_from_world( ch );
    
			free_a_char( ch );
    	}
	}
}

void update_tick( void )
{  
  	charType 		* ch , * next_dude;
  	objectType 		* obj, * next_obj;
  	objectType		* content, *next_content;
 	affectType 		* af,  * next_af;

  	for( ch = char_list; ch; ch = next_dude ) 
  	{
    	next_dude = ch->next;

  		for( af = ch->affected; af; af = next_af ) 
  		{
   			next_af = af->next;
			if( af->type < ROUND_SKILL_TYPE )
			{
				if( af->duration >= 1 )	af->duration--;
				else 
				{
					if( !af->next || (af->next->type != af->type) || (af->next->duration > 0))
						if( *spell_wear_off_msg[af->type] ) 
						{
							sendf( ch, spell_wear_off_msg[af->type] );
						}
					affect_remove(ch, af);
				}
			}
		}

		update_pos( ch );

    	if( GET_POS(ch) > POSITION_STUNNED ) 
		{
      		GET_HIT(ch)  = MIN(GET_HIT(ch)  + hit_gain(ch),  hit_limit(ch));
      		GET_MANA(ch) = MIN(GET_MANA(ch) + mana_gain(ch), mana_limit(ch));
      		GET_MOVE(ch) = MIN(GET_MOVE(ch) + move_gain(ch), move_limit(ch));
    	} 
		else if (GET_POS(ch) == POSITION_STUNNED) 
		{
      		GET_HIT(ch)  = MIN(GET_HIT(ch)  + hit_gain(ch),  hit_limit(ch));
      		GET_MANA(ch) = MIN(GET_MANA(ch) + mana_gain(ch), mana_limit(ch));
      		GET_MOVE(ch) = MIN(GET_MOVE(ch) + move_gain(ch), move_limit(ch));
      		update_pos( ch );
    	} 
		else if( GET_POS(ch) == POSITION_INCAP )	damage(ch, ch, 1, TYPE_SUFFERING);
    	else if( !IS_NPC(ch) && (GET_POS(ch) <= POSITION_MORTALLYW)) damage(ch, ch, 2, TYPE_SUFFERING);

		gain_condition( ch, FULL,   -1);
		gain_condition( ch, DRUNK,  -1);
		gain_condition( ch, THIRST, -1);

    	if( !IS_NPC(ch) )
    	{
      		update_char_objects(ch);
      		check_idling(ch);
    	}
  	}

  	for( obj = obj_list; obj ; obj = next_obj )
  	{
    	next_obj = obj->next;

    	if( (obj->type == ITEM_CONTAINER) && (obj->nr == OBJECT_CORPSE) ) 
		{
      		if( --obj->timer <= 0 ) 
	  		{
				if( obj->carried_by )
					act("$p decay in your hands.", FALSE, obj->carried_by, obj, 0, TO_CHAR);
				else if ((obj->in_room != NOWHERE) && (world[obj->in_room].people))
				{
					act("A quivering hoard of maggots consume $p.",	
						TRUE, world[obj->in_room].people, obj, 0, TO_ROOM);
					act("A quivering hoard of maggots consume $p.",
						TRUE, world[obj->in_room].people, obj, 0, TO_CHAR);
				}

				for( content = obj->contains; content; content = next_content) 
				{
					next_content = content->next_content; /* Next in inventory */

					obj_from_obj(content);

					if( obj->in_obj ) obj_to_obj( content, obj->in_obj );
					else if( obj->carried_by )
						obj_to_room( content, obj->carried_by->in_room);
					else if( obj->in_room != NOWHERE )
						obj_to_room( content, obj->in_room );
					else
						DEBUG( "point_update> Item in corpse .. where is it??" );
					
				}
				extract_obj( obj, 1 );
			}
		}
	}
}

void update_berserk( roundAffType * rf )
{
	charType	*	ch = rf->owner;

	if( rf->duration >= 1 ) 
	{
		rf->duration--;
		act( "$n's eyes glows red.", TRUE, ch, 0, 0, TO_ROOM );
		sendf( ch, "Your eyes glows red." );
	}
	else
	{
		ch->hit  += ch->hit  / 3;
		ch->move += ch->move / 3;
		ch->mana += ch->mana / 3;
  		roundAff_remove( rf );
		sendf( ch, "Your eyes turns normal." );
   	}
}

void update_time_stop( roundAffType * rf )
{
	charType	*	ch = rf->owner;

	if( rf->duration >= 1 ) rf->duration--;
	else
	{
		roundAff_remove( rf );
		sendf( ch, "You feel the world retun to normal." );
	}
}

void update_morpeth( roundAffType * rf )
{
	charType	*	ch = rf->owner;

	if( rf->duration >= 1 ) rf->duration--;
	else
	{
		roundAff_remove( rf );
		act( "$n slowly fade into exsitance. $n appears.", FALSE, ch, 0, 0, TO_ROOM );
		sendf( ch, "You are exposed." );
	}
}

void update_deafen( roundAffType * rf )
{
	charType	*	ch = rf->owner;

	if( rf->duration >= 1 ) 
	{
		rf->duration--;
		
        act( "$n shivers and $s's teeth chattered.", FALSE, ch, 0, 0, TO_ROOM );
        act( "You are fearful to hit.", FALSE, ch, 0, 0, TO_CHAR );
	}
	else
	{
		roundAff_remove( rf );
	}
}

void update_masquerade( roundAffType * rf )
{
	charType	*	ch = rf->owner;

	if( rf->duration >= 1 ) rf->duration--;
	else
	{
		roundAff_remove( rf );

		if( ch->hunt )
		{
			if( ch->fight )
				if( is_fighting(ch->fight) && ch->fight->fight == ch ) stop_fighting( ch->fight );

			stop_fighting( ch );
			if( ch->hunt->in_room == ch->in_room )
			{
				act( "$n screms, \"Araah~ $N!!\"", FALSE, ch, 0, ch->hunt, TO_ROOM );

				set_fighting( ch, ch->hunt );
			}
			else
			{
				act( "$n screams, \"Araah~ Where is $N???\"", FALSE, ch, 0, ch->hunt, TO_ROOM );
			}
			ch->hunt = 0;
		}
	}
}

void update_ambush( roundAffType * rf )
{
	charType		*	ch = rf->owner;
	charType		*	vict;
	int					percent, dam;

	if( !IS_AFFECTED( ch, AFF_AMBUSH ) )	
	{
		sendf( ch, "You break ambush." );
		roundAff_remove( rf );
		return;
	}

	if( rf->duration >= 1 ) 
	{
		if( rf->duration > 1 )	
			sendf( ch, "shiiiiiiik... You breath silently." );
		else
			sendf( ch, "shiiiiiiik... You breath deeply and slienty." );

		rf->duration--;
	}
	else
	{
		for( vict = world[ch->in_room].people; vict; vict = vict->next_in_room )
		{
			if( vict != ch && IS_MOB( vict ) ) break;
		}

		if( !vict )
		{
			sendf( ch, "You feel less powerful." );

			if( ch->move > 100 ) ch->move -= ch->move / 10;
			else
			{
				sendf( ch, "You feel lack of magical power." );
				roundAff_remove( rf );
			}
		}
		else
		{
			roundAff_remove( rf );

			if( number( 1, 5 ) == 1 )
			{
				DEBUG( "===> ambush detected by %s", vict->moved );
				if( vict->level > 30 )
					act( "$n yells, \"$N, I know you are there.\"", TRUE, vict, 0, ch, TO_ROOM );
				else
				{
					if( vict->act & ACT_WIMPY )
					{
						act( "$n detects $N.", TRUE, vict, 0, ch, TO_ROOM );
						do_flee( vict, "", COM_FLEE );
						return;
					}
				}	
				hit( vict, ch, TYPE_UNDEFINED );
			}
			else
			{
				act( "$n suddenly break out, \"Yaaaaaaaaaaaaaaa Hap.\"", TRUE, ch, 0, 0, TO_ROOM );	
				act( "You break out, \"Yaaaaaaaaaaaaaaa Hap.\"", FALSE, ch, 0, 0, TO_CHAR );

				percent = number(1,101) + ( vict->level - ch->level ); 

				if( percent < 70 )
				{
					DEBUG( "===> ambush fully success to %s", vict->moved );
					act( "$n assaults $N with $s full power!!",   TRUE, ch, 0, vict, TO_ROOM );
					act( "you assault $N with your full power!!", TRUE, ch, 0, vict, TO_CHAR );

					dam = dex_rate( ch, ch->mana/2, 10 );

					damage( ch, vict, dam, TYPE_UNDEFINED );
				}
				else
				{
					DEBUG( "===> ambush success to %s", vict->moved );
					act( "$n assaults $N.", TRUE, ch, 0, vict, TO_ROOM );
					act( "you assault $N.", TRUE, ch, 0, vict, TO_CHAR );

					dam = dex_rate( ch, ch->mana/2, 40 );

					damage( ch, vict, dam, TYPE_UNDEFINED );
				}

				ch->mana -= ch->mana / 5;
			}
		}
	}
}

void update_summon( roundAffType * rf )
{
	charType		*	ch = rf->owner;
	charType		*	vict;
	int					was_room;

	if( rf->duration >= 1 ) rf->duration--;
	else
	{
		if( ch->hunt )
		{
			for( vict = char_list; vict && vict != ch->hunt; vict = vict->next );

			if( vict != ch->hunt )
			{
				sendf( ch, "You failed." );
			}
			else
			{
  				act( "$n disappears suddenly.", TRUE, vict, 0, 0, TO_ROOM );
	
  				was_room = ch->in_room;

  				char_from_room(vict);
  				char_to_room(vict, was_room );

  				if( vict->was_in_room != NOWHERE ) vict->was_in_room = was_room ;      

  				act( "$n arrives suddenly.", TRUE,  vict, 0, 0,  TO_ROOM );
  				act( "$n has summoned you!", FALSE, ch, 0, vict, TO_VICT );
  				do_look( vict, "", 15 );
  			}
 			ch->hunt = 0;
		}
        else
        {
            sendf( ch, "You failed." );
        }
		roundAff_remove( rf );
	}
}

void update_dazzle( roundAffType * rf )
{
	charType		*	ch = rf->owner;
	objectType		*	weapon;
	int					sunny = 0;
	int					dam;

	if( rf->duration >= 1 ) rf->duration--;
	else
	{
		weapon = ch->equipment[WIELD];

		if(  is_fighting(ch) 
          && weapon 
		  && (weapon->value[3] == 3 || weapon->value[3] == 6) )
		{
			if( weather_info.sunlight && weather_info.sky == SKY_CLOUDLESS )
			{
				sunny = 1;
				act( "$n's $p radiates dazzling bright light and reflects SUN light on $N's face!",
					 TRUE, ch, weapon, ch->fight, TO_ROOM );
				act( "your $p radiates dazzling bright light and reflects SUN light on $N's face!",
					 TRUE, ch, weapon, ch->fight, TO_CHAR );
			}
			else
			{
				sunny = 0;
				act( "$n's $p radiates dazzling bright light and beams $N!",
					 TRUE, ch, weapon, ch->fight, TO_ROOM );
				act( "your $p radiates dazzling bright light and beams $N!",
					 TRUE, ch, weapon, ch->fight, TO_CHAR );
			}

			dam = GET_MOVE( ch ) + GET_MANA( ch );

			if( !sunny ) dam = dam * 2 / 3;

			GET_MANA( ch ) = GET_MANA( ch ) * 2 / 3;
			GET_MOVE( ch ) = GET_MOVE( ch ) * 2 / 3;

			if( weapon->value[3] == 3 )
			{
				act( "The beam from $n's $p draws a longitudinal line on $N's body.",
					TRUE, ch, weapon, ch->fight, TO_ROOM );
				act( "The beam from your $p draws a longitudinal line on $N's body.",
					TRUE, ch, weapon, ch->fight, TO_CHAR );
				sendrf( 0, ch->in_room, "Shuoooooooooooooooooooooooo Fah~~~~~~" );
				damage( ch, ch->fight, dam, TYPE_SLASH );
			}
			else
			{
				act( "The beam from $n's $p explodes on $N's head.",
					TRUE, ch, weapon, ch->fight, TO_ROOM );
				act( "The beam from your $p explodes on $N's head.",
					TRUE, ch, weapon, ch->fight, TO_CHAR );
				sendrf( 0, ch->in_room, "Kwuuuuuuuuuuuuaaaaaaaaaaaaaaaah~~~~~~" );
				damage( ch, ch->fight, dam, TYPE_CRUSH );
			}
		}
		else
		{
			act( "$n losts $s balance in the air.", FALSE, ch, 0, 0, TO_ROOM );
			act( "you lost your balance in the air.", FALSE, ch, 0, 0, TO_CHAR );
		}

		act( "$n lands.", TRUE, ch, 0, 0, TO_ROOM );
		sendf( ch, "You land." );
		roundAff_remove( rf );
	}
}

void update_round( void )
{
	roundAffType 	* rf;

	for( rf = roundAffs; rf; rf = next_roundAffs )
	{
		next_roundAffs = rf->next;

		if( rf->type >= SECOND_SKILL_TYPE ) continue;

		if( IS_AFFECTED( rf->owner, AFF_TIME_STOP ) 
         && rf->type != SPELL_TIME_STOP + ROUND_SKILL_TYPE ) continue;

		switch( rf->type - ROUND_SKILL_TYPE ) 
		{
			case	SKILL_BERSERK	  :	update_berserk( rf ); 	break;
			case	SPELL_TIME_STOP   :	update_time_stop( rf ); break;
			case	SKILL_MORPETH     :	update_morpeth( rf );   break;
			case	SKILL_AMBUSH      :	update_ambush( rf );    break;
			case	SKILL_DEAFEN   	  :	update_deafen( rf );    break;
			case	SKILL_MASQUERADE  :	update_masquerade( rf );break;
			case	SPELL_SUMMON  	  :	update_summon( rf );    break;
			case	SKILL_DAZZLE	  : update_dazzle( rf );    break;
			default					  :
				if( rf->duration >= 1 )	rf->duration--;
				else					roundAff_remove( rf );
		}
	}
}

void update_serpent( roundAffType * rf )
{
	charType		*	ch = rf->owner;

	serpent_stab( ch, rf );

	if( rf->duration >= 1 ) 
	{
		rf->duration--;
	}
	else
	{
		roundAff_remove( rf );
	}
}

void update_bang( roundAffType * rf )
{
	charType		*	ch = rf->owner;
	charType		*	vict, * vict_next;
	int					dam;

	act( "$n clenches the FIST and turns $s GAZE off the sky to the earth.",
			FALSE, ch, 0, 0, TO_ROOM );
	act( "You clench the FIST and turn your GAZE off the sky to the earth.",
			FALSE, ch, 0, 0, TO_CHAR );

	do_yell( ch, "UUoooooo OOoooooo Ho", 0 );

	act( "$n knees and bangs the earth with $s fist. --- KKKWANG --", TRUE, ch, 0, 0, TO_ROOM );
	act( "You knee and bang the earth with your fist. --- KKKWANG --", TRUE, ch, 0, 0, TO_CHAR );

	dam = GET_ADD( ch ) * 2;

	for( vict = world[ch->in_room].people; vict; vict = vict_next )
	{
		vict_next = vict->next_in_room;

		if( !IS_NPC( vict ) ) continue;

		act( "The earth beneth $N's feet rises and Explodes.", TRUE, ch, 0, vict, TO_ROOM );
		act( "The earth beneth $N's feet rises and Explodes.", TRUE, ch, 0, vict, TO_CHAR );
		act( "The earth beneth your feet rises and Explodes.", TRUE, ch, 0, vict, TO_VICT );

		damage( ch, vict, dam, TYPE_UNDEFINED );
	}

	GET_MANA( ch ) -= max( 10, dam / 3 );

	roundAff_remove( rf );
}

void update_second( void )
{
	roundAffType	* rf;

	for( rf = roundAffs; rf; rf = next_roundAffs )
	{
		next_roundAffs = rf->next;

		if( rf->type < SECOND_SKILL_TYPE ) continue;

		if( IS_AFFECTED( rf->owner, AFF_TIME_STOP ) ) continue;

		switch( rf->type - SECOND_SKILL_TYPE )
		{
			case	SKILL_SERPENT	  : update_serpent( rf ); 	break;
			case	SKILL_BANG		  : update_bang( rf );		break;
			default					  :
				if( rf->duration >= 1 )	rf->duration--;
				else					roundAff_remove( rf );
		}
	}
}
