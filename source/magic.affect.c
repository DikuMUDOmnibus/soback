#include <string.h>

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "world.h"
#include "utils.h"
#include "comm.h"
#include "magics.h"
#include "find.h"
#include "limits.h"
#include "misc.h"
#include "variables.h"
#include "strings.h"
#include "affect.h"
  
void spell_armor( int level, charType *ch, char * arg, int type,
 								charType *victim, objectType *obj)
{ 
  	affectType 	af;

  	if( affected_by_spell(victim, SPELL_ARMOR) )
	{
        sendf( ch, "Nothing seems to happen." ); return;
    }

    if( ch != victim ) act("$N is protected by your deity.", FALSE, ch, 0, victim, TO_CHAR);

    af.type      = SPELL_ARMOR;
    af.duration  = (GET_INT(ch)*2);
    af.modifier  = -GET_WIS(ch);
    af.location  = APPLY_AC;
    if( level > 30 ) af.modifier = af.modifier-level+30;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    sendf( victim, "You feel someone protecting you." );
}

void spell_damage_up( int level, charType *ch, char * arg, int type,
  								charType * victim,objectType *obj) 
{
  	affectType af;

  	if( affected_by_spell(victim, SPELL_DAMAGE_UP) )
	{
        sendf( ch, "Nothing seems to happen." ); return;
    }

    af.type      = SPELL_DAMAGE_UP;
    af.duration  = 4+level/10;
    af.modifier  = GET_INT(ch)/8+1;
    af.location  = APPLY_DAMROLL;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    sendf( victim, "You feel brave." );
}

void spell_crush_armor( int level, charType *ch, char * arg, int type,
  								charType *victim,objectType *obj) 
{
  	affectType af;

  	if( affected_by_spell(victim, SPELL_CRUSH_ARMOR) || (GET_POS(victim) == POSITION_FIGHTING))
	{
        sendf( ch, "Nothing seems to happen." ); return;
	}

    af.type      = SPELL_CRUSH_ARMOR;
    af.duration  = 3;
    af.modifier  = GET_INT(ch)/5*(-1) ;
    af.location  = APPLY_DAMROLL;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    af.location  = APPLY_AC;
    af.modifier  = 4*GET_INT(ch)+8;
    affect_to_char(victim, &af);

    sendf( ch, "Your victim's armor has crumbled!" );
    sendf( victim, "You feel shrink." );
}

void spell_bless( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
  	affectType af;

  	if( victim && (affected_by_spell(victim, SPELL_BLESS) || (GET_POS(victim) == POSITION_FIGHTING)))
	{
        sendf( ch, "Nothing seems to happen." ); return;
	}

  	if( obj ) 
	{
    	if( (5*GET_LEVEL(ch) > obj->weight ) 
         && (GET_POS(ch) != POSITION_FIGHTING) 
         && !IS_OBJ(obj, ITEM_EVIL)) 
		{
      		SET_BIT(obj->extra, ITEM_BLESS);
      		act("$p briefly glows.",FALSE,ch,obj,0,TO_CHAR);
    	}
  	} 
	else 
	{
    	if( (GET_POS(victim) != POSITION_FIGHTING) 
		 && (!affected_by_spell(victim, SPELL_BLESS))) 
		{
      		send_to_char("You feel righteous.\n\r", victim);
      		af.type      = SPELL_BLESS;
      		af.duration  = level / 7 + 1;
      		af.modifier  = GET_WIS(ch) / 8;
      		af.location  = APPLY_HITROLL;
      		af.bitvector = 0;
      		affect_to_char(victim, &af);

      		af.location = APPLY_SAVING_SPELL;
      		af.modifier = (GET_LEVEL(ch) > 30) ? -2 : -1;
      		affect_to_char(victim, &af);
    	}
  	}
}

void spell_blindness( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
  	affectType af;

  	if( saves_spell( ch, victim, SAVING_SPELL ) 
	 || affected_by_spell(victim, SPELL_BLINDNESS)) return;

  	if( level+number( 1,15 ) < GET_LEVEL(victim) ) return;

  	af.type      = SPELL_BLINDNESS;
  	af.location  = APPLY_HITROLL;
  	af.modifier  = -10;  /* Make hitroll worse */   
  	af.duration  = 1;
  	af.bitvector = AFF_BLIND;
  	affect_to_char(victim, &af);

  	af.location = APPLY_AC;
  	af.modifier = 50; 
  	af.duration  = 1;
  	affect_to_char( victim, &af );

  	act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
  	sendf( victim, "You have been blinded!" );
}

void spell_cure_blind( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	if( affected_by_spell(victim, SPELL_BLINDNESS) ) 
	{
    	affect_from_char(victim, SPELL_BLINDNESS);   
    	sendf( victim, "Your vision returns!" ); 
  	}
}

void spell_curse( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  affectType af;
  
  	if( obj ) 
	{
    	SET_BIT(obj->extra, ITEM_EVIL);
    	SET_BIT(obj->extra, ITEM_NODROP); 

    	if( obj->type == ITEM_WEAPON )  
      		obj->value[2]--;
    	act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR); 
  	} 
	else 
	{
    	if( saves_spell( ch, victim, SAVING_SPELL) 
         || affected_by_spell(victim, SPELL_CURSE) )   return;

    	af.type      = SPELL_CURSE;
    	af.duration  = 24*7;
    	af.modifier  = -1;
    	af.location  = APPLY_HITROLL;
    	af.bitvector = AFF_CURSE;
    	affect_to_char(victim, &af);

    	af.location = APPLY_SAVING_PARA;
    	af.modifier = 5;
    	affect_to_char(victim, &af);

    	act("$n briefly reveal a red aura!", FALSE, victim, 0, 0, TO_ROOM);
    	act("You feel very uncomfortable.",FALSE,victim,0,0,TO_CHAR);
  	}
}

void spell_detect_evil( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	affectType af;

  	if( affected_by_spell(victim, SPELL_DETECT_EVIL) )
	{
        sendf( ch, "Nothing seems to happen." ); return;
    }

  	af.type      = SPELL_DETECT_EVIL;
  	af.duration  = GET_INT(ch)*5;
  	af.modifier  = 0;
  	af.location  = APPLY_NONE;
  	af.bitvector = AFF_DETECT_EVIL;

  	affect_to_char(victim, &af);

  	sendf( victim, "Your eyes tingle." ); 
}

void spell_detect_good( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	affectType af;

  	if( affected_by_spell(victim, SPELL_DETECT_GOOD) )
	{
        sendf( ch, "Nothing seems to happen." ); return;
    }

  	af.type      = SPELL_DETECT_GOOD;
  	af.duration  = GET_INT(ch)*5;
  	af.modifier  = 0;
  	af.location  = APPLY_NONE;
  	af.bitvector = AFF_DETECT_GOOD;

  	affect_to_char(victim, &af);

  	sendf( victim, "Your eyes tingle." ); 
}

void spell_detect_invisibility( int level, charType *ch, char * arg, int type,
  									charType *victim, objectType *obj)
{
  	affectType af;

  	if( affected_by_spell(victim, SPELL_DETECT_INVISIBLE) )
	{
        sendf( ch, "Nothing seems to happen." ); return;
    }

  	af.type      = SPELL_DETECT_INVISIBLE;
  	af.duration  = level*5;
  	af.modifier  = 0;
  	af.location  = APPLY_NONE;
  	af.bitvector = AFF_DETECT_INVISIBLE;

  	affect_to_char(victim, &af);
                                                 
  	sendf( victim, "Your eyes tingle." ); 
}

void spell_invisibility( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{   
  	affectType af;                       

  	if( obj ) 
	{
    	if ( !IS_SET(obj->extra, ITEM_INVISIBLE) ) 
		{
      		act("$p turns invisible.",FALSE,ch,obj,0,TO_CHAR);
      		act("$p turns invisible.",TRUE,ch,obj,0,TO_ROOM);
      		SET_BIT(obj->extra, ITEM_INVISIBLE);
    	}
  	} 
	else 
	{ 
    	if( !affected_by_spell(victim, SPELL_INVISIBLE) ) 
		{
      		act("$n slowly fade out of existence.", TRUE, victim,0,0,TO_ROOM);
      		sendf( victim, "You vanish." );

      		af.type      = SPELL_INVISIBLE;
      		af.duration  = 24;
      		af.modifier  = -40;
      		af.location  = APPLY_AC;
      		af.bitvector = AFF_INVISIBLE;
      		affect_to_char(victim, &af);
    	}
		else
		{
			sendf( ch, "Nothing seems to happen." );
		}
  	}
} 

void spell_poison( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
  	affectType 	af;

  	if( victim ) 
	{
    	if( !saves_spell( ch, victim, SAVING_PARA) )   
    	{
      		af.type = SPELL_POISON;
      		af.duration = level*2;
      		af.modifier = -1;
      		af.location = APPLY_STR;
      		af.bitvector = AFF_POISON;
      		affect_join(victim, &af, FALSE, FALSE);    
      		sendf( victim, "You feel very sick." );
	        act( "$n turns pale.", FALSE, victim, 0, 0, TO_ROOM);
    	}
  	} 
	else 
	{
    	if( (obj->type == ITEM_DRINKCON)
         || (obj->type == ITEM_FOOD)) 
		{
      		obj->value[3] = 1;
    	}
  	}
}

void spell_protection_from_evil( int level, charType *ch, char * arg, int type,
  									charType *victim, objectType *obj)
{
  	affectType af;

  	if( !affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) ) 
	{
    	af.type      = SPELL_PROTECT_FROM_EVIL;      
    	af.duration  = 24;
    	af.modifier  = 0;
    	af.location  = APPLY_NONE;
    	af.bitvector = AFF_PROTECT_EVIL;
    	affect_to_char(victim, &af);

    	sendf( victim, "You have a righteous feeling!" );
  	}
	else
	{
		sendf( ch, "Nothing seems to happen." );
	}
}

void spell_haste( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
  	affectType af;

  	if( !affected_by_spell(victim, SPELL_HASTE) &&
        !affected_by_spell(victim, SPELL_IMPROVED_HASTE) ) 
	{
		af.type      = SPELL_HASTE;
		af.duration  = 5;
		af.modifier  = 3;
		af.location  = APPLY_DEX;
		af.bitvector = AFF_HASTE;
		affect_to_char(victim, &af);

		sendf( victim, "You feel world is going slowly!" );
  	}
	else
	{
		sendf( ch, "Nothing seems to happen." );
	}
}

void spell_improved_haste( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	affectType af;

  	if( !affected_by_spell(victim, SPELL_HASTE) && 
        !affected_by_spell(victim, SPELL_IMPROVED_HASTE) ) 
	{
		af.type      = SPELL_IMPROVED_HASTE;
		af.duration  = 5;
		af.modifier  = 6;
		af.location  = APPLY_DEX;
		af.bitvector = AFF_IMPROVED_HASTE;
		if (GET_CLASS(ch) == CLASS_MAGIC_USER) af.duration++;
		affect_to_char(victim, &af);

    	sendf( victim, "You feel world is going slowly!" );
  	}
	else
	{
		sendf( ch, "Nothing seems to happen." );
	}
}

void spell_remove_curse( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	if( obj ) 
	{
    	if( IS_SET(obj->extra, ITEM_EVIL)
         || IS_SET(obj->extra, ITEM_NODROP)) 
		{
      		act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);

      		REMOVE_BIT(obj->extra, ITEM_EVIL);
      		REMOVE_BIT(obj->extra, ITEM_NODROP);
    	}
  	} 
	else 
	{
    	if( affected_by_spell(victim, SPELL_CURSE) ) 
		{
      		act("$n briefly glows red, then blue.",FALSE,victim,0,0,TO_ROOM);
      		act("You feel better.",FALSE,victim,0,0,TO_CHAR);
      		affect_from_char(victim, SPELL_CURSE);     
    	}
  	}
}

void spell_remove_poison( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	if( victim ) 
	{
    	if( affected_by_spell(victim,SPELL_POISON) ) 
		{
      		affect_from_char(victim,SPELL_POISON);
      		act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
      		act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
    	}
  	} 
	else 
	{
    	if( (obj->type == ITEM_DRINKCON)
         || (obj->type == ITEM_FOOD)) 
		{
      		obj->value[3] = 0;
      		act("The $p steams briefly.",FALSE,ch,obj,0,TO_CHAR);
    	}
  	}
}

void spell_infravision( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	affectType af;

  	if(IS_NPC(victim)) return;

  	if( !affected_by_spell(victim, SPELL_INFRAVISION) ) 
	{
		act("$n's eyes glow red.",TRUE,victim,0,0,TO_ROOM);
		act("You feel your eyes become more sensitive.",TRUE,victim,0,0,TO_CHAR);
		af.type      = SPELL_INFRAVISION;
		af.duration  = 2*(GET_WIS(ch)*2);
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_INFRAVISION;
		affect_to_char(victim, &af);
  	}
	else
	{
		sendf( ch, "Nothing seems to happen." );
	}
}

void spell_mirror_image( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	affectType 	af;

  	if(IS_NPC(victim)) return;

  	if( !affected_by_spell(victim, SPELL_MIRROR_IMAGE) ) 
	{
		af.type      = SPELL_MIRROR_IMAGE;
		af.duration  = (level<IMO) ? 3 : level;
		af.duration += (level>30) + (level>35);      
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_MIRROR_IMAGE;
    	affect_to_char(victim, &af);

		act("$n creats own mirror image.",TRUE,victim,0,0,TO_ROOM);
		act("You made your illusion.",TRUE,victim,0,0,TO_CHAR);
  	}
	else
	{
		sendf( ch, "Nothing seems to happen." );
	}
}

void spell_sanctuary( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	affectType af;

  	if(IS_NPC(victim)) return;

  	if( !affected_by_spell(victim, SPELL_SANCTUARY) ) 
	{
		act("$n is surrounded by a white aura.",TRUE,victim,0,0,TO_ROOM);
		act("You start glowing.",TRUE,victim,0,0,TO_CHAR);
		af.type      = SPELL_SANCTUARY;
		af.duration  = (level<IMO) ? 3 : level*2;    
		af.duration += (GET_WIS(ch) >= 24 ) + (level > 25) + (level > 35 );
		af.modifier  = GET_LEVEL(ch)-15+GET_WIS(ch);
		af.location  = APPLY_NONE;
		af.bitvector = AFF_SANCTUARY;
    	affect_to_char(victim, &af);
  	}
	else
	{
		sendf( ch, "Nothing seems to happen." );
	}
}

void spell_sanctuary_shield( int level, charType * ch, char * arg, int type,
							charType * victim, objectType * obj )
{
	charType	*	master;
	followType	*	f;

	if( !IS_AFFECTED( ch, AFF_GROUP ) )
	{
		sendf( ch, "Nobody to cast on." );
		return;
	}

	if( ch->master ) master = ch->master;
	else			 master = ch;

	if(   IS_AFFECTED( master, AFF_GROUP )
	  && !IS_AFFECTED( master, AFF_SANCTUARY ) ) spell_sanctuary( level, ch, 0, 0, master, 0 );

	for( f = master->followers; f; f = f->next )
	{
		if( IS_AFFECTED( f->follower, AFF_GROUP ) && !IS_AFFECTED( f->follower, AFF_SANCTUARY ) )
			spell_sanctuary( level, ch, 0, 0, f->follower, 0 );
	}
}

void spell_sleep( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	affectType 	af;

  	if( IS_NPC(victim) ) 
	{
    	if( number(1,25) < GET_LEVEL(victim)) return;
    }
  	else if( GET_LEVEL(victim) >= (IMO+3) || GET_POS(victim) == POSITION_SLEEPING ) return;

  	if( IMPL( ch ) || !saves_spell( ch, victim, SAVING_SPELL) ) 
	{
		af.type      = SPELL_SLEEP;
		af.duration  = 4+level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_SLEEP;
		affect_join(victim, &af, FALSE, FALSE);

		if( GET_POS(victim) > POSITION_SLEEPING) 
		{
		  	act("You feel very sleepy ..... zzzzzz",FALSE,victim,0,0,TO_CHAR);
		  	act("$n go to sleep.",TRUE,victim,0,0,TO_ROOM);
		  	GET_POS(victim)=POSITION_SLEEPING;
    	}
  	}
}

void spell_strength( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
  	affectType 	af;

  	act("You feel stronger.",FALSE,victim,0,0,TO_CHAR);
  	af.type      = SPELL_STRENGTH;
  	af.duration  = level;
  	af.modifier  = 1+(level>18);
  	af.location  = APPLY_STR;
  	af.bitvector = 0;
  	affect_join(victim, &af, TRUE, FALSE);
}

void spell_sense_life( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
  	affectType af;

  	if( !affected_by_spell(victim, SPELL_SENSE_LIFE) ) 
	{
		af.type      = SPELL_SENSE_LIFE;
		af.duration  = 10*GET_INT(ch);
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_SENSE_LIFE;
		affect_to_char(victim, &af);

    	sendf( victim, "Your feel your awareness improve." );
  	}
	else
	{
		sendf( ch, "Nothing seems to happen." );
	}
}

void spell_fear( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
	roundAffType	rf;

  	if( saves_spell( ch, victim, SAVING_SPELL ) 
	 || affected_by_spell(victim, SPELL_FEAR) ) return;

    memset( &rf, 0, sizeof( rf ) );

    rf.type      = SKILL_DEAFEN + ROUND_SKILL_TYPE;
    rf.duration  = 7;
    rf.modifier  = ch->level;
    rf.bitvector = AFF_FEAR;
}
