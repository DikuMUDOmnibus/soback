#include <string.h>

#include "character.h"
#include "mobile.h"
#include "world.h"
#include "affect.h"
#include "interpreter.h"
#include "magics.h"
#include "transfer.h"
#include "comm.h"
#include "fight.h"
#include "find.h"
#include "limits.h"
#include "utils.h"

void do_berserk( charType * ch, char * argu, int cmd )
{
	roundAffType		rf;

	if( !is_fighting( ch ) )
	{
		sendf( ch, "This skill can be used only in fighting." );
		return;
	}

	if( ch->hit < hit_limit( ch ) / 3 )
	{
		sendf( ch, "You are exhousted." );
		return;
	}

/*  DEBUG( "===> %s berserk.", ch->name );
*/

	if( number( 1, 101 ) < ch->skills[SKILL_BERSERK] )
	{
/*		DEBUG( "===> ok." );
*/
		ch->hit  -= ch->hit  / 3;
		ch->mana -= ch->mana / 3;
		ch->move -= ch->move / 3;

		memset( &rf, 0, sizeof( rf ) );

		rf.type      = SKILL_BERSERK + ROUND_SKILL_TYPE;
		rf.duration  = 3;
		rf.modifier  = ch->level;
		rf.bitvector = AFF_BERSERK;

		roundAff_to_char( ch, &rf );

		act( "$n looks like a Berserker. Oh, really $e is!!.", FALSE, ch, 0, 0, TO_ROOM );
		sendf( ch, "You feel like being a Berserker!" );

		WAIT_STATE( ch, PULSE_VIOLENCE * 3 );
	}
	else
	{
		sendf( ch, "You failed." );

		WAIT_STATE( ch, PULSE_VIOLENCE );
	}
}

void do_deafen( charType * ch, char * argu, int cmd )
{
	roundAffType		rf;
	charType		*	tmp;
	int					level;

	if( !is_fighting( ch ) )
	{
		sendf( ch, "This skill can be used only in fighting." );
		return;
	}

	if( ch->move <= 0 )
	{
		sendf( ch, "You are exhousted." );
		return;
	}

/*	DEBUG( "===> %s deafen.", ch->name );
*/
	if( number( 1, 101 ) < ch->skills[SKILL_DEAFEN] )
	{
		memset( &rf, 0, sizeof( rf ) );

		rf.type      = SKILL_DEAFEN + ROUND_SKILL_TYPE;
		rf.duration  = 2;
		rf.modifier  = ch->level;
		rf.bitvector = AFF_FEAR;

		act( "Suddenly $n moves faster and $n yells out oath all around.", FALSE, ch, 0, 0, TO_ROOM );

		for( tmp = world[ch->in_room].people; tmp; tmp = tmp->next_in_room )
		{
			if( tmp == ch ) continue;

			act( "$n yells, \"$N, you shall die!\"", FALSE, ch, 0, tmp, TO_ROOM );
			act( "you yell, \"$N, you shall die!\"", FALSE, ch, 0, tmp, TO_CHAR );

			if( !IS_NPC(ch) ) level = IS_NPC(tmp) ? tmp->level : tmp->level + 10;
			else			  level = IS_NPC(tmp) ? tmp->level + 10 : tmp->level;

			if( (GET_ADD(ch) + ch->level) > number( level * 2, level * 4) ) 
			{
/*				DEBUG( "===> %s [%d]", tmp->name, tmp->level );
*/
				sendf( tmp, "Ack~~ You feel a pain in your ear." );
				roundAff_to_char( tmp, &rf );
			}

			if( ch->move -= number(20,30), ch->move <= 0 )
			{
				sendf( ch, "You can't move more." );
				return;
			}
		}
		WAIT_STATE( ch, PULSE_VIOLENCE );
	}
	else
	{
		sendf( ch, "You failed." );
		WAIT_STATE( ch, PULSE_VIOLENCE / 2 );
	}
}

void do_masquerade( charType * ch, char * argu, int cmd )
{
	roundAffType		rf;
	charType		*	tmp, * vict;

	if( !is_fighting( ch ) )
	{
		sendf( ch, "This skill can be used only in fighting." );
		return;
	}

	if( ch->mana <= 0 )
	{
		sendf( ch, "You are exhousted." );
		return;
	}

	ch->mana -= number( 30, 50 );

  	DEBUG( "===> %s masquerade.", ch->name );

	if( number( 1, 101 ) < ch->skills[SKILL_MASQUERADE] )
	{
		vict = ch->fight;

		for( tmp = world[ch->in_room].people; tmp; tmp = tmp->next_in_room )
		{
			if( IS_NPC( tmp ) && (vict != tmp) ) break;
		}

		if( !tmp )
		{
			sendf( ch, "You masquerade as yourself." );
		}
		else
		{
			memset( &rf, 0, sizeof( rf ) );

			rf.type      = SKILL_MASQUERADE + ROUND_SKILL_TYPE;
			rf.duration  = 2;
			rf.modifier  = ch->level;
			rf.bitvector = 0;

			act( "As $n turns $s body around, $n masquerade as $N.", FALSE, ch, 0, tmp, TO_ROOM );
			act( "You turn your body around and masquerade as $N ", FALSE, ch, 0, tmp, TO_CHAR );

			if( (GET_DEX(ch) + 13 + ch->level) > number(vict->level, vict->level * 3) )
			{
				DEBUG( "===> %s [%d]", vict->name, vict->level );
				act( "$n is confused.", FALSE, vict, 0, 0, TO_ROOM );

				roundAff_to_char( vict, &rf );
				vict->hunt = ch;

				if( vict->fight ) stop_fighting( vict );
				hit( vict, tmp, TYPE_UNDEFINED );
			}
		}
	}
	else
	{
		sendf( ch, "You failed." );
	}
	WAIT_STATE( ch, PULSE_VIOLENCE / 2 );
}

void do_morpeth( charType * ch, char * argument, int cmd )
{
  	roundAffType 	rf;
  	int 			percent;

	if( IS_NPC(ch) ) return;

	if( ch->move < 0 )
	{
		sendf( ch, "You are too exhousted." );
		return;
	}
  	if( IS_AFFECTED(ch, AFF_MORPETH) ) affect_from_char(ch, SKILL_MORPETH);

  	percent = number(1,101); /* 101% is a complete failure */

	DEBUG( "===> %s morpeth.", ch->name );

  	if( percent < ch->skills[SKILL_MORPETH] 
	 && percent > number( 1, (200 - ((ch->skills[SKILL_MORPETH]) + dex_rate( ch, 20, 20 )))) )
	{
		DEBUG( "===> ok." );
		memset( &rf, 0, sizeof( rf ) );

  		rf.type 	 = SKILL_MORPETH + ROUND_SKILL_TYPE;
  		rf.duration  = MAX( GET_DEX(ch) - 20, 1 );
  		rf.modifier  = 0;
  		rf.location  = APPLY_NONE;
  		rf.bitvector = AFF_MORPETH;
  		roundAff_to_char( ch, &rf );

   		act( "$n fade out of existence. $n disapears.", TRUE, ch, 0, 0, TO_ROOM );
		sendf( ch, "Ok. You vanish." );
	}
	ch->move -= dex_rate( ch, 20, 20 );
}

void do_ambush( charType * ch, char * argu, int cmd )
{
  	roundAffType 	rf;
  	int				percent;

	if( IS_MOB( ch ) || ch->level < 15 ) return;

	if( is_fighting(ch) )
	{
		sendf( ch, "No way! You can't not ambush when you are fighting." );
		return;
	}

	if( ch->mana < 0 )
	{
		sendf( ch, "You feel lack of magical power." ); return;
	}

  	percent = number(1,101);

	DEBUG( "===> %s ambush.", ch->name );
  	if( percent < ch->skills[SKILL_AMBUSH] 
	 && percent > number( 1, (200 - ((ch->skills[SKILL_AMBUSH]) + dex_rate( ch, 15, 30 )))) )
	{
		DEBUG( "===> ok." );
		memset( &rf, 0, sizeof( rf ) );

		rf.type      = SKILL_AMBUSH + ROUND_SKILL_TYPE;
		rf.duration  = 1;
		rf.modifier  = ch->level;
		rf.bitvector = AFF_AMBUSH;

  		roundAff_to_char( ch, &rf );

   		act( "$n hides $s body under the ground.", TRUE, ch, 0, 0, TO_ROOM );
		sendf( ch, "Ok. You ambush." );
		WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
	}
	else
	{
		sendf( ch, "You failed." );
		WAIT_STATE( ch, PULSE_VIOLENCE / 3 );
	}
	ch->mana -= dex_rate( ch, 15, 30 );
}

void do_dazzle( charType * ch, char * argu, int cmd )
{
	objectType		*	weapon;
	charType		*	vict;
  	roundAffType 		rf;
  	int					percent;

	if( IS_MOB( ch )  || ch->level < 40 ) return;

	if( ch->mana < 50 || ch->move < 50 )
	{
		sendf( ch, "You feel lack of your power." ); return;
	}

	weapon = ch->equipment[WIELD];

	if( !weapon || (weapon->value[3] != 3 && weapon->value[3] != 6) )
	{
		sendf( ch, "You need a crush or slash weapon." ); return;
	}

	if( !ch->fight )
	{
		if( vict = find_char_room( ch, argu ), !vict )
		{
			sendf( ch, "You don't see any such target." ); return;
		}
		set_fighting( ch, vict );
		if( AWAKE( vict ) && !vict->fight ) set_fighting( vict, ch );
	}

  	percent = number(1,101);

	DEBUG( "===> %s dazzle blast.", ch->name );

  	if( percent < ch->skills[SKILL_DAZZLE] )
	{
		DEBUG( "===> ok." );
		memset( &rf, 0, sizeof( rf ) );

		rf.type      = SKILL_DAZZLE + ROUND_SKILL_TYPE;

		if( beforeround ) rf.duration  = 1;
		else              rf.duration  = 0;

		rf.modifier  = ch->level;
		rf.bitvector = AFF_DAZZLE;

  		roundAff_to_char( ch, &rf );

		do_yell( ch, "Chuhuhuhu Haaaaaaaaaaaaap!", COM_YELL );

		act( "$n points the SKY with $s $p and BLASTS OFF to the SKY!", 
				FALSE, ch, weapon, 0, TO_ROOM );
		act( "You point the SKY with your $p and BLAST OFF to the SKY! ", 
				FALSE, ch, weapon, 0, TO_CHAR );
	}
	else
	{
		sendf( ch, "You failed." );
	}

	WAIT_STATE( ch, PULSE_VIOLENCE / 1 );

	ch->mana -= dex_rate( ch, 15, 40 );
	ch->move -= dex_rate( ch, 15, 40 );
}
