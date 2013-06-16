#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "macros.h"
#include "comm.h"
#include "weapon.h"
#include "utils.h"
#include "magics.h"
#include "fight.h"
#include "limits.h"

void sweapon_smash( charType * ch, charType * victim, objectType * weapon )
{
  	int dam = 55;

  	act("$p smashes $n with bone crushing sound.",TRUE,victim,weapon,0,TO_ROOM);
  	act("You are smashed by $p. You are hard to stand.",TRUE,victim,weapon,0,TO_CHAR);
  
  	damage(ch,victim,dam,TYPE_UNDEFINED);

  	if(GET_POS(victim)>POSITION_SITTING) GET_POS(victim)=POSITION_SITTING;
}
 
void sweapon_flame( charType * ch, charType * victim, objectType * weapon )
{
  	int dam;

  	dam = number(30,45);

  	act("Large flame from $o strikes $n. $n is burned." ,TRUE,victim,weapon,0,TO_ROOM);
  	act("Large flame from $o strikes you. You are burned." ,TRUE,victim,weapon,0,TO_CHAR);

  	WAIT_STATE(victim,PULSE_VIOLENCE);
  	damage(ch,victim,dam,TYPE_UNDEFINED);
}
 
void sweapon_ice( charType * ch, charType * victim, objectType * weapon )
{
  	int dam;

  	dam = number(20,35);

  	act("White beam from $o strikes $n. $n is frozen." ,TRUE,victim,weapon,0,TO_ROOM);
  	act("White beam from $o strikes you. You are frozen." ,TRUE,victim,weapon,0,TO_CHAR);

  	WAIT_STATE(victim,PULSE_VIOLENCE);
  	damage(ch,victim,dam,TYPE_UNDEFINED);
}

void sweapon_bombard( charType * ch, charType * victim, objectType * weapon )
{
  	int dam;

  	dam = number(1, GET_LEVEL(ch)+GET_LEVEL(victim));

  	act("Bombard sword divides in paticles and strikes $n. $n is confused."	,TRUE,victim,0,0,TO_ROOM);
  	act("Bombard sword divides in paticles and strikes you. You are confused.",TRUE,ch,0,0,TO_VICT);

  	WAIT_STATE(victim,PULSE_VIOLENCE+4);
  	damage(ch,victim,dam,TYPE_UNDEFINED);

  	if( GET_POS( victim) > POSITION_STUNNED ) GET_POS(victim)=POSITION_STUNNED;
}

void sweapon_shot( charType * ch, charType * victim, objectType * weapon )
{
  	int dam = 70;

  	dam = dam - GET_STR(victim) - GET_CON(victim);
  	act("$p glows with striking aura.",TRUE,victim,weapon,0,TO_ROOM);
  	act("You see $p glows. It HURTS you.",TRUE,victim,weapon,0,TO_CHAR);
  	damage(ch,victim,dam,TYPE_UNDEFINED);
}

void sweapon_bolt( charType * ch, charType * victim, objectType * weapon )
{
  	int dam;

  	dam = number( 20, ch->level * 2 );

  	act("A lightning bolt from $p hits $n. ZZZZZ~~~",TRUE,victim,weapon,0,TO_ROOM);
  	act("You are hit by lightning from $p. Ouch~.", TRUE,victim,weapon,0,TO_CHAR);
  	damage(ch,victim,dam,TYPE_UNDEFINED);

  	if( GET_POS( victim) > POSITION_STUNNED ) GET_POS(victim)=POSITION_STUNNED;
}

void sweapon_dragon_slayer( charType * ch, charType * victim, objectType * weapon )
{
  	int dam;

  	if(IS_NPC(victim) && !IS_SET(victim->act,ACT_DRAGON)) return;

  	dam = number(GET_LEVEL(ch),2*GET_LEVEL(ch));

  	act("$p PENETRATES the $n's tough skin.",TRUE,victim,weapon,0,TO_ROOM);
  	damage(ch,victim,dam,TYPE_UNDEFINED);
}

void sweapon_anti_good( charType * ch, charType * victim, objectType * weapon )
{
  	int dam;

  	dam = number(GET_LEVEL(ch),GET_LEVEL(ch)+GET_LEVEL(victim));

  	if( (IS_EVIL(ch) && number( 1, 6 ) == 1)
  		|| (IS_NEUTRAL( ch ) && IS_EVIL( victim ) && number( 1, 3 ) == 1) )
  	{
    	act("$n screams in pain. $n looks like being shocked by something.", TRUE,ch,weapon,0,TO_ROOM);
    	act("You feel sudden pain from your hands. You are shocked.", TRUE,ch,weapon,0,TO_CHAR);
    	dam=MIN(GET_HIT(ch)-1,2*dam);
    	damage(victim,ch,dam,TYPE_UNDEFINED);
    	return;
  	}

  	if( IS_GOOD(victim) )
  	{
    	act("$p glows with dark aura.",TRUE,victim,weapon,0,TO_ROOM);
    	act("You feel sudden darkness. It really did hurt." ,TRUE,victim,weapon,0,TO_CHAR);
    	damage(ch,victim,dam,TYPE_UNDEFINED);
    	return;
  	}
}

void sweapon_anti_evil( charType * ch, charType * victim, objectType * weapon )
{
  	int dam;

  	dam = number(GET_LEVEL(ch),GET_LEVEL(ch)+GET_LEVEL(victim));

  	if( (IS_GOOD(ch) && number( 1, 6 ) == 1)
  		|| (IS_NEUTRAL( ch ) && IS_GOOD( victim ) && number( 1, 3 ) == 1) )
  	{
    	act("$n screams in pain. $n looks like being shocked by something.", TRUE,ch,weapon,0,TO_ROOM);
    	act("You feel sudden pain from your hands. You are shocked.", TRUE,ch,weapon,0,TO_CHAR);
    	dam=MIN(GET_HIT(ch)-1,2*dam);
    	damage(victim,ch,dam,TYPE_UNDEFINED);
    	return;
  	}
  	if( IS_EVIL(victim) )
  	{
    	act("$p glows with bright aura.",TRUE,victim,weapon,0,TO_ROOM);
    	act("You feel sudden brightness. It really did hurt." ,TRUE,victim,weapon,0,TO_CHAR);
    	damage(ch,victim,dam,TYPE_UNDEFINED);
    	return;
  	}
}

void sweapon_mana_drain( charType * ch, charType * victim, objectType * weapon )
{
	int		diff;

	if( number( 1, 5 ) == 1 )
	{
  		act("Dark aura from $o surrounds $n. $n turns pale.",  TRUE,victim,weapon,0,TO_ROOM);
  		act("Dark aura from $o surrounds you. You are tired.", TRUE,victim,weapon,0,TO_CHAR);

		diff = GET_MANA( victim ) = GET_MANA( victim ) / 2;
		diff /= 2;

		GET_MANA( ch ) += diff;
        GET_ALIGN(ch) = MAX( -999, GET_ALIGN(ch) - 50 );
	}
}

void sweapon_hit_drain( charType * ch, charType * victim, objectType * weapon )
{
	int		diff;

	if( number( 1, 5 ) == 1 )
	{
  		act("Red aura from $o surrounds $n. $n turns pale.",  TRUE,victim,weapon,0,TO_ROOM);
  		act("Red aura from $o surrounds you. You are tired.", TRUE,victim,weapon,0,TO_CHAR);

		diff = GET_MANA( victim ) = GET_MANA( victim ) / 2;
		diff /= 2;

		GET_HIT( ch ) += diff;
	}
}

void sweapon_move_drain( charType * ch, charType * victim, objectType * weapon )
{
	int		diff;

	if( number( 1, 4 ) == 2 )
	{
		act("Black aura from $o surrounds $n. $n turns pale.",  TRUE,victim,weapon,0,TO_ROOM);
  		act("Black aura from $o surrounds you. You are tired.", TRUE,victim,weapon,0,TO_CHAR);

		diff = GET_MANA( victim ) = GET_MANA( victim ) / 2;
		diff /= 2;

		GET_MOVE( ch ) += diff;
	}
}

void magic_weapon_hit( charType *ch, charType *victim, objectType * weapon )
{
  	int 	d;

  	d = (weapon->magic ==0) ? 1: number(1, weapon->magic );

  	if( d == 1)
  	{
    	switch(weapon->value[0]) 
		{
      	case WEAPON_SMASH: 				sweapon_smash			(ch,victim,weapon); break;
      	case WEAPON_FLAME: 				sweapon_flame			(ch,victim,weapon); break;
      	case WEAPON_ICE:   				sweapon_ice  			(ch,victim,weapon); break;
      	case WEAPON_BOMBARD:  			sweapon_bombard			(ch,victim,weapon); break;
      	case WEAPON_SHOT:  				sweapon_shot 			(ch,victim,weapon); break;
      	case WEAPON_BOLT:  				sweapon_bolt 			(ch,victim,weapon); break;
      	case WEAPON_DRAGON_SLAYER: 		sweapon_dragon_slayer	(ch,victim,weapon); break;
      	case WEAPON_ANTI_GOOD_WEAPON: 	sweapon_anti_good		(ch,victim,weapon); break;
      	case WEAPON_ANTI_EVIL_WEAPON: 	sweapon_anti_evil		(ch,victim,weapon); break;
      	case WEAPON_MANA_DRAIN: 		sweapon_mana_drain		(ch,victim,weapon); break;
      	case WEAPON_HIT_DRAIN: 			sweapon_hit_drain		(ch,victim,weapon); break;
      	case WEAPON_MOVE_DRAIN: 		sweapon_move_drain		(ch,victim,weapon); break;
      	default: return;  
    	}
  	}
  	else return;
}
