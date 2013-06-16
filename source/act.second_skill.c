#include <string.h>

#include "character.h"
#include "object.h"
#include "mobile.h"
#include "affect.h"
#include "fight.h"
#include "magics.h"
#include "comm.h"
#include "find.h"
#include "utils.h"
#include "limits.h"
#include "interpreter.h"

void serpent_stab( charType * ch, roundAffType * rf )
{
	static char *	target[] =
	{
		"EYE",
		"NECK",
		"RIB",
		"BELLY"
	};

	charType		*	vict    = ch->fight;
	int					last 	= rf->d1;
	int					critic  = rf->d2;
	int					pers	= rf->d3;
	int					moved   = rf->d4;
	int					hit;
	int					dam;
	int					tar;

	if( pers <= 0 ) return;

    if( !is_fighting( ch ) ) 
	{
		rf->d3 = 0; return;
	}

   	hit = ((GET_AC(vict)/10)<<1) + number(1,101);

	dam = ch->dr;

	if( number( 1, 3 ) == 1 ) tar = last == -1 ? 0 : last;
	else					  tar = (number( 1, 64 ) - 1) / 16;

	if( tar != last )
	{
		if( moved )
		{
			act( "$n slides to left.", TRUE, ch, 0, 0, TO_ROOM );
			act( "you slide to left.", TRUE, ch, 0, 0, TO_CHAR );
			moved = 0;
		}
		else
		{
			act( "$n slides to right.", TRUE, ch, 0, 0, TO_ROOM );
			act( "you slide to right.", TRUE, ch, 0, 0, TO_CHAR );
			moved = 1;
		}
		critic = 0;
	}
	else critic++;

   	if( hit > ch->skills[SKILL_SERPENT] )
   	{
       	damage( ch, vict, 0, SKILL_SERPENT );

		act( "$n aims $N's $T, but misses.", 
			FALSE, ch,	(objectType *)target[tar], vict, TO_NOTVICT );
		act( "you aim $N's $T, but miss.", 
			FALSE, ch,	(objectType *)target[tar], vict, TO_CHAR );
		act( "$n aims your $T, but misses.", 
			FALSE, ch,	(objectType *)target[tar], vict, TO_VICT );
	}
   	else
   	{
		if( !critic )
		{
			act( "$n stabs $N in the $T. It makes a small wound.",
				FALSE, ch, (objectType *)target[tar], vict, TO_NOTVICT );
			act( "you stab $N in the $T. It makes a small wound.",
				FALSE, ch, (objectType *)target[tar], vict, TO_CHAR );
			act( "$n stabs your in the $T. It makes a small wound.",
				FALSE, ch, (objectType *)target[tar], vict, TO_VICT );
		}
		else if( critic == 1 )
		{
			act( "$n stabs $N in the $T AGAIN. It makes a wound. Ouch~~", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_NOTVICT );
			act( "you stab $N in the $T AGAIN. It makes a wound. Ouch~~", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_CHAR );
			act( "$n stabs you in the $T AGAIN. It makes a wound. Ouch~~", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_VICT );
		}
		else if( critic == 2 )
		{
			act( "$n stabs $N in the $T AGAIN. $N screams in PAIN. Bloody~~", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_NOTVICT );
			act( "you stab $N in the $T AGAIN. $N screams in PAIN. Bloody~~", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_CHAR );
			act( "$n stabs you in the $T AGAIN. You screams in PAIN. Bloody~~", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_VICT );
		}
		else if( critic == 3 )
		{
			act( "$n stabs $N in the $T AGAIN. a small HOLE. BLOODY!!", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_NOTVICT );
			act( "you stab $N in the $T AGAIN. a small HOLE. BLOODY!!", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_CHAR );
			act( "$n stabs you in the $T AGAIN. a small HOLE. BLOODY!!", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_VICT );

			if( tar == 0 )
			{
				affectType		af;

    			af.type      = SPELL_BLINDNESS;
    			af.location  = APPLY_HITROLL;
    			af.modifier  = -10;  /* Make hitroll worse */
    			af.duration  = 1;
    			af.bitvector = AFF_BLIND;
    			affect_to_char(vict, &af);

    			af.location = APPLY_AC;
    			af.modifier = 50;
    			af.duration  = 1;
    			affect_to_char( vict, &af );
      
    			act("As $n stabs $N in the eye, $N seems to be blinded!", 
						TRUE, ch, 0, vict, TO_NOTVICT);
    			act("As you stab $N in the eye, $N seems to be blinded!", 
						TRUE, ch, 0, vict, TO_CHAR);
    			sendf( vict, "You have been blinded!" );

				DEBUG( "===> serpent blind %s", vict->moved ? vict->moved : vict->name );
			}

			if( tar == 1 )
			{
				roundAffType		rf;

        		memset( &rf, 0, sizeof( rf ) );
        
        		rf.type      = SKILL_DEAFEN + ROUND_SKILL_TYPE;
        		rf.duration  = 2; 
        		rf.modifier  = ch->level;
        		rf.bitvector = AFF_FEAR;
        
				roundAff_to_char( vict, &rf );

    			act("As $n stabs $N in the neck, $N seems to be scared!", 
						TRUE, ch, 0, vict, TO_NOTVICT);
    			act("As you stab $N in the neck, $N seems to be scared!", 
						TRUE, ch, 0, vict, TO_CHAR);
    			sendf( vict, "You are scared!" );
				DEBUG( "===> serpent fear %s", vict->moved ? vict->moved : vict->name );
			}
	   	}
		else if( critic == 4 ) 
		{
			act( "$n stabs $N in the $T AGAIN. a BIG HOLE in $N's body! AWFUL!!", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_NOTVICT );
			act( "you stab $N in the $T AGAIN. a BIG HOLE in $N's body! AWFUL!!", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_CHAR );
			act( "$n stabs YOU in the $T AGAIN. a BIG HOLE in $N's body! AWFUL!!", 
					FALSE, ch,	(objectType *)target[tar], vict, TO_VICT );
		}
		else 
		{
			act( "$n EVISCERATES $N as $n's stab to $N. DREADFUL!!", 
					FALSE, ch,	0, vict, TO_NOTVICT );
			act( "you EVISCERATE $N as your stab to $N. DREADFUL!!", 
					FALSE, ch,	0, vict, TO_CHAR );
			act( "$n EVISCERATES you as $s stab to YOU. DREADFUL!!", 
					FALSE, ch,	0, vict, TO_VICT );
		}

		if( critic > 2 )
		{
			DEBUG( "===> %s's serpent seccess %d times.", ch->name, critic + 1 );
		}
		damage( ch, vict, dam * (critic+1), SKILL_SERPENT );
	}

	GET_MOVE( ch ) -= MAX( 20, 75 - dex_rate( ch, 20, 20 ));

	if( GET_MOVE( ch ) < 0 ) pers = 0;

	pers -= number( 10, 15 );

	rf->d1 = tar;
	rf->d2 = critic;
	rf->d3 = pers;
	rf->d4 = moved;
}

void do_serpent( charType * ch, char * argu, int cmd )
{
	charType		*	vict;
	roundAffType	 	rf;

	if( IS_NPC( ch ) ) return;

    if( !is_fighting( ch ) )
    {
        sendf( ch, "This skill can be used only in fighting." );
        return;
    }           
        
	vict = ch->fight;

    if( !ch->equipment[WIELD] )
    {
        sendf( ch, "You need to wield a weapon, to make it a succes." );
        return;
    }
    if( ch->equipment[WIELD]->value[3] != 11 )   
    {
        sendf( ch, "Only piercing weapons can be used for serpent stab." );
        return;
    }

	if( GET_MOVE( ch ) < 0 )
	{
		sendf( ch, "You are too exhausted." ); return;
	}

	memset( &rf, 0, sizeof( rf ) );

    rf.type      = SKILL_SERPENT + SECOND_SKILL_TYPE;
    rf.duration  = 10;
	rf.d1		 = -1;
	rf.d3 		 = number( dex_rate( ch, 20, 35 ), dex_rate( ch, 30, 35 ) );
	rf.d4		 = number( 0, 1 );

    roundAff_to_char( ch, &rf );

	serpent_stab( ch, &rf );

	DEBUG( "===> %s serpent stab.", ch->name );

   	WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_bang( charType * ch, char * argu, int cmd )
{
	roundAffType	 	rf;

	if( IS_NPC( ch ) ) return;

	if( GET_MANA( ch ) < 100 )
	{
		sendf( ch, "You need more mana to bang the earth." ); return;
	}

	if( number( 1, 101 ) > ch->skills[SKILL_BANG] )
	{
		sendf( ch, "You failed." ); return;
	}

	memset( &rf, 0, sizeof( rf ) );

    rf.type      = SKILL_BANG + SECOND_SKILL_TYPE;
    rf.duration  = 1;
	rf.modifier  = 0;
	rf.bitvector = 0;

	roundAff_to_char( ch, &rf );

	DEBUG( "===> %s bang the earth.", ch->name );

	act( "$n stands big rock and gaze up the sky. --Silent--", FALSE, ch, 0, 0, TO_ROOM );
	act( "You stand big rock and gaze up the sky. --Silent--", FALSE, ch, 0, 0, TO_CHAR );

   	WAIT_STATE( ch, PULSE_VIOLENCE / 2 );
}
