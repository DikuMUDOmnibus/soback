#include "character.h"
#include "mobile.h"
#include "object.h"
#include "affect.h"
#include "allocate.h"
#include "comm.h"
#include "transfer.h"

roundAffType	* roundAffs;
roundAffType	* next_roundAffs;

int is_apply_ac(objectType *obj_object)
{
   	return (   IS_WEAR(obj_object,ITEM_WEAR_KNEE)
      		|| IS_WEAR(obj_object,ITEM_WEAR_ABOUTLEGS)
      		|| IS_WEAR(obj_object,ITEM_WEAR_SHIELD)
      		|| IS_WEAR(obj_object,ITEM_WEAR_WRIST)
      		|| IS_WEAR(obj_object,ITEM_WEAR_WAISTE)
      		|| IS_WEAR(obj_object,ITEM_WEAR_ARMS)
      		|| IS_WEAR(obj_object,ITEM_WEAR_HANDS)
      		|| IS_WEAR(obj_object,ITEM_WEAR_FEET)
      		|| IS_WEAR(obj_object,ITEM_WEAR_LEGS)
      		|| IS_WEAR(obj_object,ITEM_WEAR_ABOUT)
      		|| IS_WEAR(obj_object,ITEM_WEAR_HEAD)
      		|| IS_WEAR(obj_object,ITEM_WEAR_BODY) );
}

static int apply_ac(charType *ch, int eq_pos)
{
  	if( !(ch->equipment[eq_pos]) )
  	{
    	log( "apply_ac> %s is not wearing object in %d", ch->name, eq_pos );
    	return(0);
  	}

  	if( ch->equipment[eq_pos]->type != ITEM_ARMOR ) return 0;

  	switch( eq_pos ) 
  	{
    	case WEAR_BODY		:
    	case WEAR_HEAD		:
    	case WEAR_LEGS		:
    	case WEAR_FEET		:
    	case WEAR_HANDS		:
    	case WEAR_ARMS		:
    	case WEAR_SHIELD	:
    	case WEAR_ABOUT 	:
    	case WEAR_WAISTE	:
    	case WEAR_WRIST_R	:
    	case WEAR_WRIST_L	:
    	case WEAR_KNEE_R	:
    	case WEAR_KNEE_L	:
    	case WEAR_ABOUTLEGS : return (ch->equipment[eq_pos]->value[0]);
    	default				: break ;
  	}
  	return 0;
}

void affect_modify( charType * ch, int loc, short mod, long bitv, int add )
{
  	int 		maxabil;

  	if( add ) SET_BIT(ch->affects, bitv);
   	else 
   	{
    	REMOVE_BIT(ch->affects, bitv);
    	mod = -mod;
  	}

  	maxabil = 25;
#define MAXABIL( a, b , c )  ( (a) = MIN( (a)+(b), c ) )
  	switch(loc)
  	{
    	case APPLY_NONE: 												break;
    	case APPLY_STR: 			MAXABIL( GET_STR(ch), mod, 25 );	break; 
		case APPLY_STR_ADD: 		MAXABIL( GET_ADD(ch), mod, 100 ); 	break;
		case APPLY_DEX: 			MAXABIL( GET_DEX(ch), mod, 25 );	break;
    	case APPLY_INT: 			MAXABIL( GET_INT(ch), mod, 25 ); 	break;
    	case APPLY_WIS: 			MAXABIL( GET_WIS(ch), mod, 25 ); 	break;
    	case APPLY_CON: 			MAXABIL( GET_CON(ch), mod, 25 ); 	break;
    	case APPLY_SEX:	  /* 	 	GET_SEX(ch) += mod;   */ 			break;
    	case APPLY_CLASS: /* 		GET_CLASS(ch) += mod; */ 			break;
    	case APPLY_LEVEL: /* 		GET_LEVEL(ch) += mod; */ 			break;
    	case APPLY_AGE:	            ch->age_mod += mod; 				break;
    	case APPLY_CHAR_WEIGHT: 	GET_WEIGHT(ch) += mod; 				break;
    	case APPLY_CHAR_HEIGHT: 	GET_HEIGHT(ch) += mod; 				break;
    	case APPLY_MANA: 			ch->max_mana += mod; 				break;
    	case APPLY_HIT: 			ch->max_hit += mod; 				break;
    	case APPLY_MOVE: 			ch->max_move += mod; 				break;
    	case APPLY_GOLD: 												break; 
		case APPLY_EXP: 												break;
    	case APPLY_AC: 				GET_AC(ch) += mod; 					break;
    	case APPLY_HITROLL: 		GET_HITROLL(ch) += mod; 			break;
    	case APPLY_DAMROLL: 		GET_DAMROLL(ch) += mod; 			break;
    	case APPLY_SAVING_PARA: 	ch->saving_throw[0] += mod; 		break;
    	case APPLY_SAVING_ROD: 		ch->saving_throw[1] += mod; 		break;
    	case APPLY_SAVING_PETRI: 	ch->saving_throw[2] += mod; 		break;
    	case APPLY_SAVING_BREATH: 	ch->saving_throw[3] += mod; 		break;
    	case APPLY_SAVING_SPELL: 	ch->saving_throw[4] += mod; 		break;
    	case APPLY_REGENERATION: 	ch->regen += mod; 					break; 
		case APPLY_INVISIBLE: 
			if( mod >= 0 && !IS_AFFECTED(ch, AFF_INVISIBLE))
			{ 
				SET_BIT(ch->affects, AFF_INVISIBLE);
      		}
      		if( mod < 0 && IS_AFFECTED(ch, AFF_INVISIBLE))
	  		{
        		REMOVE_BIT(ch->affects, AFF_INVISIBLE);
      		}
      		break;
    	case APPLY_SANCTUARY:
      		if( mod >= 0 && !IS_AFFECTED(ch, AFF_SANCTUARY) )
	  		{
        		SET_BIT(ch->affects, AFF_SANCTUARY);
      		}
      		if( mod < 0 && IS_AFFECTED(ch, AFF_SANCTUARY) )
	  		{
        		REMOVE_BIT(ch->affects, AFF_SANCTUARY);
      		}
      		break;
    	default: DEBUG( "apply_modify> unkown loccation [%d] for %s.", loc, ch->name );
      		break;
  	}
}

void affect_total(charType *ch)
{
  	affectType 		*	af;
	roundAffType	*	rf;
  	int 				i, j;

  	for( i = 0; i < MAX_WEAR; i++ )
  	{
    	if( ch->equipment[i] )
      		for( j = 0; j < MAX_APPLY; j++ )
        		affect_modify(ch, ch->equipment[i]->apply[j].location,
                      ch->equipment[i]->apply[j].modifier,
                      ch->equipment[i]->bitvector, FALSE);
  	}

  	for(af = ch->affected; af; af = af->next)
    	affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
  	for(rf = ch->roundAffs; rf; rf = rf->my_next)
    	affect_modify(ch, rf->location, rf->modifier, rf->bitvector, FALSE);

  	ch->temp_stat = ch->base_stat;

  	for(i=0; i<MAX_WEAR; i++) 
  	{
    	if( ch->equipment[i] )
      		for( j = 0; j < MAX_APPLY; j++ )
        		affect_modify(ch, ch->equipment[i]->apply[j].location,
                      ch->equipment[i]->apply[j].modifier,
                      ch->equipment[i]->bitvector, TRUE);
  	}

  	for(af = ch->affected; af; af=af->next)
   		affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
  	for(rf = ch->roundAffs; rf; rf = rf->my_next)
    	affect_modify(ch, rf->location, rf->modifier, rf->bitvector, TRUE);

	/* Make certain values are between 0..25, not < 0 and not > 25! */

  	i = 25;

  	GET_DEX(ch) = MAX(0,MIN(GET_DEX(ch), i));
  	GET_INT(ch) = MAX(0,MIN(GET_INT(ch), i));
  	GET_WIS(ch) = MAX(0,MIN(GET_WIS(ch), i));
  	GET_CON(ch) = MAX(0,MIN(GET_CON(ch), i));

  	if( IS_NPC(ch) ) GET_STR(ch) = MIN(GET_STR(ch), i);
   	else 
   	{
   		GET_STR(ch) = MIN( GET_STR(ch), 18 );
		if( ch->class == CLASS_CLERIC || ch->class == CLASS_WARRIOR ) 
			GET_ADD(ch) = MIN( GET_ADD(ch), 100 );
		else
			GET_ADD(ch) = 0;
  	}
}

void affect_to_char( charType * ch, affectType * af )
{
  	affectType 		* affected_alloc;

  	affected_alloc = alloc_affect();

	af->unit = affected_alloc->unit; *affected_alloc = *af;

  	affected_alloc->next = ch->affected;
  	ch->affected = affected_alloc;

	affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
  	affect_total(ch);
}

void roundAff_to_char( charType * ch, roundAffType * rf )
{
	roundAffType	*	round_alloc;

	round_alloc = alloc_round();

	rf->unit = round_alloc->unit; *round_alloc = *rf;

	round_alloc->my_next = ch->roundAffs;
	ch->roundAffs = round_alloc;

	round_alloc->next = roundAffs;
	roundAffs = round_alloc;

	round_alloc->owner = ch;

	affect_modify( ch, rf->location, rf->modifier, rf->bitvector, TRUE );
  	affect_total( ch );
}

void affect_remove( charType *ch, affectType *af )
{
  	affectType 	* find;

  	affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

	if( ch->affected == af ) ch->affected = af->next;
  	else 
  	{
   		for( find = ch->affected; (find->next) && (find->next != af); find = find->next)
		;

   		if( find->next != af ) 
		{
   			FATAL( "Could not locate affected_type in ch->affected. (handler.c, affect_remove)");
   		}
   		find->next = af->next;
  	}
  	free_affect( af );
  	affect_total(ch);
}

void roundAff_remove( roundAffType * rf )
{
	roundAffType 	*	find;
	charType	 	* 	ch = rf->owner;

	affect_modify(ch, rf->location, rf->modifier, rf->bitvector, FALSE);

	if( next_roundAffs == rf ) next_roundAffs = rf->next;

	if( ch->roundAffs == rf ) ch->roundAffs = rf->my_next;
  	else 
  	{
   		for( find = ch->roundAffs; (find->my_next) && (find->my_next != rf); find = find->my_next)
		;

   		if( find->my_next != rf ) 
		{
   			FATAL( "roundAff_remove> could not locate round affect in personal list.");
   		}
   		find->my_next = rf->my_next;
  	}

	if( roundAffs == rf ) roundAffs = rf->next;
  	else 
  	{
   		for( find = roundAffs; (find->next) && (find->next != rf); find = find->next)
		;

   		if( find->next != rf ) 
		{
   			FATAL( "roundAff_remove> could not locate round affect in gloval list.");
   		}
   		find->next = rf->next;
  	}

  	free_round( rf );
  	affect_total(ch);
}

void affect_from_char( charType * ch, int skill )
{
  	affectType 		*	find;

  	for( find = ch->affected; find; find = find->next )
    	if( find->type == skill )
      		affect_remove( ch, find );
}

int affected_by_spell( charType *ch, int skill )
{
  	affectType * find;

  	for( find = ch->affected; find; find = find->next )
    	if( find->type == skill )
      		return( TRUE );

  	return( FALSE );
}

void affect_join( charType *ch, affectType *af, int avg_dur, int avg_mod )
{
  	affectType 		* 	find;
  	int 				found = FALSE;

  	for( find = ch->affected; !found && find; find = find->next) 
  	{
    	if( find->type == af->type ) 
		{
      		af->duration += find->duration;

      		if( avg_dur ) af->duration /= 2;

      		af->modifier += find->modifier;

      		if( avg_mod ) af->modifier /= 2;

      		affect_remove(ch, find);
      		affect_to_char(ch, af);

      		found = TRUE;
    	}
  	}
  	if( !found ) affect_to_char(ch, af);
}

void equip_char(charType *ch, objectType *obj, int pos)
{
  	int 		j;

  	if( obj->carried_by ) 
  	{
    	DEBUG( "equip_char> Obj is carried_by when equip." );
    	return;
  	}

  	if( obj->in_room!=NOWHERE ) 
  	{
    	DEBUG( "equip_char> Obj is in_room when equip.");
    	return;
  	}
  	if( obj->equiped_by ) 
  	{
    	DEBUG( "equip_char> Obj is equiped_by when equip.");
    	return;
  	}

  	if( (IS_OBJ(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)) 										
  	 ||	(IS_OBJ(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)) 										
	 || (IS_OBJ(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)) 
     ||	(!IS_NPC(ch) && IS_OBJ(obj, ITEM_ANTI_MAGE)    && (GET_CLASS(ch)==CLASS_MAGIC_USER)) 
	 ||	(!IS_NPC(ch) && IS_OBJ(obj, ITEM_ANTI_CLERIC)  && (GET_CLASS(ch)==CLASS_CLERIC)) 
     ||	(!IS_NPC(ch) && IS_OBJ(obj, ITEM_ANTI_WARRIOR) && (GET_CLASS(ch)==CLASS_WARRIOR)) 
	 ||	(!IS_NPC(ch) && IS_OBJ(obj, ITEM_ANTI_THIEF)   && (GET_CLASS(ch)==CLASS_THIEF)) )
  	{
    	if( ch->in_room != NOWHERE 
		 && (!IS_NPC(ch) && ch->desc && ch->desc->connected == CON_PLYNG) ) 
		{

      		act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
      		act("$n is zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_ROOM);
      		obj_to_room(obj, ch->in_room);
      		return;
    	}
  	}

  	ch->equipment[pos] = obj;
  	obj->equiped_by    = ch;

  	if( obj->type == ITEM_ARMOR ) GET_AC(ch) = GET_AC(ch) - apply_ac( ch, pos );

  	for( j = 0; j < MAX_APPLY; j++ )
    	affect_modify( ch, obj->apply[j].location, 
						   obj->apply[j].modifier, 
                           obj->bitvector, TRUE );

  	affect_total(ch);
}

objectType * unequip_char(charType *ch, int pos)
{
  	int 				j;
  	objectType 		*	obj;

  	obj = ch->equipment[pos];

  	if( obj->type == ITEM_ARMOR ) GET_AC(ch) = GET_AC(ch) + apply_ac( ch, pos );

  	ch->equipment[pos] = 0;
  	obj->equiped_by    = 0;

  	for( j = 0; j < MAX_APPLY; j++ )
    	affect_modify( ch, obj->apply[j].location, 
						   obj->apply[j].modifier, 
       		               obj->bitvector, FALSE);
  	affect_total(ch);

  	return(obj);
}
