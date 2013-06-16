#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "defines.h"
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

void spell_cure_critic( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  int healpoints;

  healpoints = dice(3,5)+10;

  healpoints = wis_rate( ch, healpoints, GET_LEVEL(ch)  );

  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;
  send_to_char("You feel better!\n\r", victim);
  update_pos(victim);
}

void spell_cause_critic( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  int dampoints;

  dampoints = dice(level/5,8)+3;
  dampoints = wis_rate( ch, dampoints, GET_LEVEL(ch) );
  send_to_char("You feel pain!\n\r", victim);
  damage(ch,victim,dampoints,SPELL_CAUSE_CRITIC);
}

void spell_mana_boost( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  int healpoints=0;

  assert(victim);
  assert((level > 0) && (level < 55));
  if(GET_MOVE(victim)>50) healpoints = GET_MOVE(victim)/2+2*GET_LEVEL(victim);
    GET_MANA(victim) += healpoints;
    GET_MOVE(victim) =0; 
  send_to_char("You feel better!\n\r", victim);
  update_pos(victim);
}

void spell_vitalize( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  int healpoints=0;

  healpoints = GET_MOVE(victim)/dice(1,3)+GET_MOVE(victim)*level*dice(3,5)/300;
    GET_HIT(victim) += healpoints;
    GET_MOVE(victim) =0;
    GET_MANA(victim)-=10;
    send_to_char("You feel vitalized!\n\r", victim);
  update_pos(victim);
}

void spell_cure_light( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  int healpoints;

  assert(ch && victim);
  assert((level >= 0) && (level <= 55));

  healpoints = dice(1,10)+3;

  healpoints = wis_rate( ch, healpoints, GET_LEVEL(ch)  );

  if ( (healpoints+GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  update_pos( victim );

  send_to_char("You feel better!\n\r", victim);
}

void spell_cause_light( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  int dampoints;

  assert(ch && victim);
  assert((level >= 0) && (level <= 55));

  dampoints = dice(1,10)+3;
  dampoints = wis_rate( ch, dampoints, GET_LEVEL(ch) );

  send_to_char("You feel pain!\n\r", victim);
  damage(ch,victim,dampoints,SPELL_CAUSE_LIGHT);
}


void spell_heal( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
	int heal = 50 + dice(1,12);

	heal = wis_rate( ch, heal, GET_LEVEL(ch) );

  	spell_cure_blind(level, ch, "", 0, victim, obj);

  	GET_HIT(victim) += heal;

  	if (GET_HIT(victim) >= hit_limit(victim))
    	GET_HIT(victim) = hit_limit(victim)-dice(1,4);

    act("$n heals $N.", FALSE, ch, 0, victim, TO_NOTVICT);
	act("You heal $N.", FALSE, ch, 0, victim, TO_CHAR);

  	update_pos( victim );

  	send_to_char("A warm feeling fills your body.\n\r", victim);
}

void spell_harm( int level, charType *ch, char * aru, int type,
  						charType *victim, objectType *obj)
{
  	int 		dam, hit;
 
  	dam = wis_rate( ch, 110, 30 );

  	hit = GET_HIT( victim );

  	if( dam > hit ) dam = hit - 1;
  	else 
	{
    	if ( saves_spell( ch, victim, SAVING_SPELL) )
    	dam >>= 1;
  	}
  	damage(ch, victim, dam, SPELL_HARM);
} 
 
void spell_full_heal( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
	int heal = 90 + dice(1,12);

	heal = wis_rate( ch, heal, GET_LEVEL(ch) );

  	spell_cure_blind(level, ch, "", 0, victim, obj);

  	GET_HIT(victim) += heal;

  	if (GET_HIT(victim) >= hit_limit(victim))
    	GET_HIT(victim) = hit_limit(victim)-dice(1,4);

    act("$n full heals $N.", FALSE, ch, 0, victim, TO_NOTVICT);
	act("You full heal $N.", FALSE, ch, 0, victim, TO_CHAR);

  	update_pos( victim );

  	sendf( victim, "A warm feeling fills your body." );
}
