/*************************************************************************
*  file: mobact.c , Mobile action module.                 Part of DIKUMUD *
*  Usage: Procedures generating 'intelligent' behavior in the mobiles.    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "utils.h"
#include "comm.h"
#include "magics.h"
#include "limits.h"
#include "find.h"
#include "fight.h"
#include "misc.h"
#include "variables.h"
#include "interpreter.h"
#include "mobile.action.h"
#include "affect.h"
#include "transfer.h"

/* mode and fightmode is defined in mob.magic.h  */
#define NUMBER_OF_MAX_VICTIM	8
#define VICT_IS_SAME_ROOM(mob)	(mob->in_room==mob->fight->in_room)

charType *choose_victim(charType *mob, int fightmode, int mode)
{
    int 					i, min, max, tmp, count, lev ;
    charType 	*	vict, *next_vict ;
    charType 	*	victims[NUMBER_OF_MAX_VICTIM] ;

  	count = 0 ;
  	victims[0] = NULL ;
  	for(vict=world[mob->in_room].people; vict && count < NUMBER_OF_MAX_VICTIM ; vict = next_vict) 
  	{
    	next_vict = vict->next_in_room ;
    	if( !IS_NPC(vict) ) 
		{
       		if( vict->fight ) 
	   		{
          		if( fightmode == VIC_FIGHTING || fightmode == VIC_ALL ) 
		  		{
            		victims[count] = vict ;
            		count ++ ;
            	}
          	}
       		else 
	   		{
          		if( fightmode == VIC_WATCHING || fightmode == VIC_ALL ) 
		  		{
            		victims[count] = vict ;
            		count ++ ;
            	}
          	}
     	}
   	}

  	if( count == 0 ) 
  	{
     	return NULL ;
    }

   	if( mode == MODE_RANDOM ) 
   	{
      	tmp = number( 0, count-1 );
      	TRACE( trace("choose mob> count = %d, rnd # %d, (%s)", count, tmp, victims[tmp]->name));
      	return ( victims[tmp] ) ;
    }

   	if( mode == MODE_HIT_MIN || mode == MODE_MANA_MIN || mode == MODE_MOVE_MIN
     || mode == MODE_AC_MIN  || mode == MODE_HR_MIN   || mode == MODE_DR_MIN )
 	{
     	min  = 32000 ;
     	vict = victims[0] ;

     	for( i = 1 ; i < count ; i ++ ) 
	 	{
       		switch ( mode ) 
	   		{
         	case MODE_HIT_MIN  : tmp = GET_HIT(victims[i]) ; break ;
         	case MODE_MANA_MIN : tmp = GET_MANA(victims[i]) ; break ;
         	case MODE_MOVE_MIN : tmp = GET_MOVE(victims[i]) ; break ;
         	case MODE_AC_MIN   : tmp = GET_AC(victims[i]) ; break ;
         	case MODE_HR_MIN   : tmp = GET_DAMROLL(victims[i]) ; break ;
         	case MODE_DR_MIN   : tmp = GET_HITROLL(victims[i]) ; break ;
         	case MODE_LEVEL_MIN : tmp = GET_LEVEL(victims[i]) ; break ;
         	default : tmp = GET_HIT(victims[i]) ;
         	}
       		if( min > tmp) 
	   		{
        		min = tmp ; vict = victims[i] ;
        	}
     	}
   	}
   	else if( mode == MODE_HIT_MAX  || mode == MODE_MANA_MAX 
          || mode == MODE_MOVE_MAX || mode == MODE_AC_MAX
          || mode == MODE_HR_MAX   || mode == MODE_DR_MAX )
  	{
     	max = -3000 ;
     	vict = victims[0] ;
     	for( i = 1 ; i < count ; i ++ ) 
	 	{
       		switch ( mode ) {
         case MODE_HIT_MAX  : tmp = GET_HIT(victims[i]) ; break ;
         case MODE_MANA_MAX : tmp = GET_MANA(victims[i]) ; break ;
         case MODE_MOVE_MAX : tmp = GET_MOVE(victims[i]) ; break ;
         case MODE_AC_MAX   : tmp = GET_AC(victims[i]) ; break ;
         case MODE_HR_MAX   : tmp = GET_DAMROLL(victims[i]) ; break ;
         case MODE_DR_MAX   : tmp = GET_HITROLL(victims[i]) ; break ;
         case MODE_LEVEL_MAX : tmp = GET_LEVEL(victims[i]) ; break ;
         default : tmp = GET_HIT(victims[i]) ;
         }
       if ( max < tmp) {
        max = tmp ;
        vict = victims[i] ;
        }
     }
   }
   else if ( mode == MODE_HIGH_LEVEL || mode == MODE_MID_LEVEL
     || mode == MODE_LOW_LEVEL ) {
     for ( i = 0 ; i < count ; i ++ ) {
        lev = GET_LEVEL(victims[i]) ;
        if ( mode == MODE_HIGH_LEVEL && lev >= 31 && lev < IMO )
            return ( victims[i] ) ;
        if ( mode == MODE_MID_LEVEL && lev >= 13 && lev <= 30 )
            return ( victims[i] ) ;
        if ( mode == MODE_LOW_LEVEL && lev >= 1 && lev <= 12 )
            return ( victims[i] ) ;
        }
      /* return NULL because thereis no one such that */
      return NULL ;
     }
	else if ( mode == MODE_HAS_OKSE ) {
		for ( i = 0 ; i < count ; i ++) {
			if ((victims[i])->equipment[HOLD] && /* holding okse */
			objects[(victims[i])->equipment[HOLD]->nr].virtual==2706){
				return ( victims[i] ) ;
				}
			}
		return NULL ;
		}

    return (vict) ;
}
#undef NUMBER_OF_MAX_VICTIM

#define NUMBER_OF_MAX_MOB	8
charType * choose_rescue_mob( charType * mob )
{
    int					tmp, count ;
    charType *	vict, * next_vict ;
    charType *	victims[NUMBER_OF_MAX_MOB] ;

  	count = 0 ;
  	victims[count] = NULL ;

  	for( vict=world[mob->in_room].people; vict && count < NUMBER_OF_MAX_MOB ; vict = next_vict) 
  	{
    	next_vict = vict->next_in_room ;
    	if( IS_NPC(vict) ) 
		{
       		if( vict->fight && (mob != vict) ) 
	   		{
             	victims[count] = vict ; count ++ ;
            }
       	}
    }

   	if( count == 0 ) 
   	{
      	if( mob->fight )  return mob ;
      	else return NULL ;
    }

   	tmp = number(0, count-1) ;
   	if( tmp >= 0 && tmp < count ) 	return ( victims[tmp] ) ;
   	else 							return NULL ;
}
#undef NUMBER_OF_MAX_MOB

int spell_blocker(charType *ch, int cmd, char *arg)
{
  	if( cmd == COM_CAST )
  	{
    	do_say(ch,"하하하, 여기서는 마술을 못쓰지!",0);
    	return TRUE;
  	}
  	else return FALSE;
}

int magic_user(charType *ch, int cmd, char *arg)
{
  	charType 	*	vict;
  	charType 	*	vict_next;

  	if( cmd ) return FALSE;
	if( !AWAKE( ch ) ) return 0;

  	if( GET_POS(ch) != POSITION_FIGHTING)
  	{
    	if( GET_LEVEL(ch) != 27 ) return(FALSE);

    	for( vict = world[ch->in_room].people; vict; vict = vict->next_in_room )
      	if( (!IS_NPC(vict)) && (GET_LEVEL(vict) < IMO) )
	  	{
        	act("$n utters the words 'Hypnos Thanatos'.",1,ch,0,0,TO_ROOM);
        	spell_sleep(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
        	return(TRUE);
      	}
    	return FALSE;
  	}

  	if( !is_fighting(ch) ) return FALSE;

  	for( vict = world[ch->in_room].people; vict; vict = vict_next )
  	{
  		vict_next = vict->next_in_room;

    	if( vict->fight == ch && vict != ch->fight && number(0,1) == 0 ) break;
	}

	if( !vict ) vict = ch->fight;

	switch( GET_LEVEL(ch) ) 
	{
		case 1: case 2: case 3: case 4: case 5:
			act("$n utters the words 'hahili duvini'.", 1, ch, 0, 0, TO_ROOM);
			spell_magic_missile(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
			break;
		case 6: case 7: case 8: case 9: case 10:
			act("$n utters the words 'vivani yatima'.",1,ch,0,0,TO_ROOM);
			spell_burning_hands(GET_LEVEL(ch), ch,"",SPELL_SPELL,vict,0);
			break;
		case 11: case 12: case 13: case 14: case 15: case 16: case 17:
			act("$n utters the words 'nasson hof'.", 1, ch, 0, 0, TO_ROOM);
			spell_colour_spray(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
			break;
		case 18: case 19: case 20: case 21: case 22: case 23: case 24:
			act("$n utters the words 'tubu morg'.", 1, ch, 0, 0, TO_ROOM);
			spell_fireball(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
			break;
		case 25: case 26: case 27: case 28: case 29: case 30: case 31: case 32: case 33: case 34:
			act("$n utters the words 'oliel ese'.", 1, ch, 0, 0, TO_ROOM);
			spell_sunburst(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
			break;
		case 35: case 36: case 37: case 38: case 39:
			act("$n utters the words 'ambeh no'.", 1, ch, 0, 0, TO_ROOM);
			spell_corn_of_ice(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
			break;
		default:
			act("$n utters the words 'ambeh no'.", 1, ch, 0, 0, TO_ROOM);
      		spell_corn_of_ice(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
	  		if( is_fighting(ch) && vict != ch->fight )
				spell_sunfire( GET_LEVEL(ch),ch,"", SPELL_SPELL, ch->fight, 0 );
     		break;
	}	

	if( is_fighting( ch ) )
	{
		if( number(1,2) == 1 ) 
		{
			act("$n utters the words 'sulco kow'.", 1, ch, 0, 0, TO_ROOM);
			spell_curse( GET_LEVEL(ch),ch, "", SPELL_SPELL, ch->fight, 0 );
		}
		if( number(1,3) == 1 && is_fighting( ch ) ) 
		{
			act("$n utters the words 'nkunf bnrd'.", 1, ch, 0, 0, TO_ROOM);
      		spell_blindness( GET_LEVEL(ch),ch, "", SPELL_SPELL, ch->fight, 0 );
   		}
   	}
  	return TRUE;
}

void npc_steal(charType *ch,charType *victim)
{
  	int 		dir,gold;

  	if( IS_NPC(victim) || GET_LEVEL(victim) >= IMO ) return;

  	if( AWAKE(victim) && (number(0, GET_LEVEL(ch)) == 0)) 
  	{
    	acthan("You discover that $n has $s hands in your wallet.",
      			"앗! $n님이 당신의 지갑에 손을 넣습니다.",	FALSE,ch,0,victim,TO_VICT);
    	acthan("$n tries to steal gold from $N.",
      			"$n님이 $N님으로부터 돈을 훔치려고 합니다.",TRUE,ch,0,victim,TO_NOTVICT);
  	} 
  	else 
  	{
    	/* Steal some gold coins */
    	acthan("$n suppresses a laugh.","$n님이 키득키득 웃습니다.(왜그렇까?)",
      				TRUE,ch,0,0,TO_NOTVICT);
    	if( gold =(int)((GET_GOLD(victim)*number(1,10))/25), gold > 0 )
		{
      		GET_GOLD(ch)     += gold*4/5;
      		GET_GOLD(victim) -= gold;
      		dir = number(0,5);
      		if( can_go(ch,dir) ) do_simple_move(ch,dir,FALSE);
    	}
  	}
}

int snake(charType *ch, int cmd, char *arg)
{
  	if(cmd) return FALSE;
	if( !AWAKE( ch ) ) return 0;

  	if( is_fighting( ch ) && (number(0,32-GET_LEVEL(ch)) == 0 ) )
    {
      	acthan("$n bites $N!", "$n님이 $N님의 다리를 물었습니다!",
         		1, ch, 0, ch->fight, TO_NOTVICT);
      	acthan("$n bites you!", "$n님에게 다리를 물렸습니다!",
         		1, ch, 0, ch->fight, TO_VICT);
      	spell_poison( GET_LEVEL(ch), ch, "", SPELL_SPELL, ch->fight, 0);
      	return TRUE;
   	}
  	return FALSE;
}

int thief(charType *ch, int cmd, char *arg)
{
  	charType *	cons;
  	charType *	cons_next;

  	if( cmd ) return FALSE;
	if( !AWAKE( ch ) ) return 0;

  	if( GET_POS(ch) == POSITION_FIGHTING ) 
  	{
    	if( !is_fighting( ch ) ) return FALSE;
    	if( number(1,4) > 1) 
		{
      		acthan("$n says 'Ya haa haa hap'.", "$n님이 '야하햐합' 하고 말합니다",
        				1, ch, 0, 0, TO_ROOM);
      		for( cons = world[ch->in_room].people; cons; cons = cons_next ) 
	  		{
				cons_next = cons->next_in_room;
        		if( number(10,IMO+3) < GET_LEVEL(ch) && GET_LEVEL(cons) < IMO)
          			if( !IS_NPC(cons) && ch != cons )
            			hit( ch, cons, TYPE_UNDEFINED );
        	}
      	}
    	else if(  (GET_LEVEL(ch) > 10) 
			   && ((GET_HIT(ch)* 100/GET_MAX_HIT(ch)) < 35)
               && is_fighting( ch ) )
		{
      		acthan("$n says 'Quu ha cha cha'.", "$n님이 '크하차차' 하고 외칩니다",
        			1, ch, 0, 0, TO_ROOM);
      		do_flash( ch, "", 0 );
      	}
    }
  	else if( GET_POS(ch)!= POSITION_STANDING ) return FALSE;
  	else 
  	{
    	for(cons = world[ch->in_room].people; cons; cons = cons->next_in_room ) 
		{
      		if((!IS_NPC(cons)) && (GET_LEVEL(cons)<IMO) &&
        		(GET_LEVEL(cons)>=GET_LEVEL(ch)) && (number(1,3)==1))
        			npc_steal(ch,cons); 
      	}
    }
  	return TRUE;
}

int cleric(charType *ch, int cmd, char *arg)
{
  	charType 	*	vict;
  	charType 	*	v2;
  	int						nfight;

  	if( cmd ) return FALSE;
	if( !AWAKE( ch ) ) return 0;

  	if( GET_POS(ch) != POSITION_FIGHTING)
  	{
    	return FALSE;
  	}

  	if( !is_fighting(ch) ) return FALSE;

  	for( vict=world[ch->in_room].people; vict; vict = vict->next_in_room )
    	if( vict->fight ==ch && number(1,2)==1 )
      		break;

	if( !vict ) vict = ch->fight;

  	switch( GET_LEVEL(ch) ) 
  	{
    case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11:
      	acthan("$n utters the words 'asdghjkasdgi'.", 
	  			"$n님이 '마디해체자' 라고 주문을 욉니다", 1, ch, 0, 0, TO_ROOM);
      	spell_cure_light(GET_LEVEL(ch), ch, "", SPELL_SPELL, ch, 0);
      	break;
    case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19:
      	acthan("$n utters the words 'saghkasdlghui'.",
        		"$n님이 '마디후지이' 라고 주문을 욉니다",1,ch,0,0,TO_ROOM);
      	spell_cure_critic(GET_LEVEL(ch), ch,"",SPELL_SPELL,ch,0);
      	break;
    case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27:
      	acthan("$n utters the words 'heal'.",
        		"$n님이 '힐' 이라고 주문을 욉니다",  1, ch, 0, 0, TO_ROOM);
      	spell_heal(GET_LEVEL(ch), ch, "", SPELL_SPELL, ch, 0);
      	break;
    case 28: case 29: case 30: 
	default:
      	acthan("$n utters the words 'sdagh'.",
        		"$n님이 '푸힐' 이라고 주문을 욉니다", 1, ch, 0, 0, TO_ROOM);
      	spell_full_heal(GET_LEVEL(ch), ch, "", SPELL_SPELL, ch, 0);
      	break;
  	}

  	if( number(1,2) == 1 && GET_LEVEL(ch) > 21 )
  	{
      	acthan("$n utters the words 'oliel ese'.",
        		"$n님이 '오디엘 에세' 라고 주문을 욉니다", 1, ch, 0, 0, TO_ROOM);
      	spell_sunburst(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
  	}

  	if(!IS_EVIL(ch) && !IS_GOOD(vict)  && number(1,2)==1 )
  	{
  		if( is_fighting(ch) )
  		{
			acthan("$n utters the words 'fidprl rbil'.",
				"$n님이 '피이어 불' 이라고 주문을 욉니다", 1, ch, 0, 0, TO_ROOM);
			spell_fireball(GET_LEVEL(ch),ch,"",SPELL_SPELL, ch->fight,0);

			if( GET_LEVEL(ch) > 10 && is_fighting( ch ) )
				spell_dispel_evil(GET_LEVEL(ch),ch, "", SPELL_SPELL, ch->fight, 0 );
		}
  	}

  	if( GET_LEVEL(ch) > 20 || number(1,2) == 1 ) 
  	{
    	for( nfight = 0, v2 = world[ch->in_room].people; v2; v2 = v2->next_in_room )
      		if( v2->fight == ch ) nfight++;

    	if( nfight >=4 )
		{
      		mob_spell_fire_storm( GET_LEVEL(ch), ch );
	  		if( GET_LEVEL(ch) >= 35 ) mob_spell_hand_of_god( ch );
	  	}
    }
  	return TRUE;
}

int paladin(charType *ch, int cmd, char *arg)
{
  	charType 	*	vict;

  	if( cmd ) return FALSE;
	if( !AWAKE( ch ) ) return 0;

  	if( !is_fighting( ch ) ) return FALSE;

  	for( vict = world[ch->in_room].people; vict; vict = vict->next_in_room )
    	if(vict->fight==ch && number(0,2)==0)
      		break;

  	if( number(1,3) == 1 )
  	{
		switch (GET_LEVEL(ch)) 
		{
		case 1: case 2:	case 3:	 case 4:  case 5:  case 6: case 7:
		case 8: case 9:	case 10: case 11: case 12: case 13:
		  act("$n utters the words 'asdghjkasdgi'.", 1, ch, 0, 0, TO_ROOM);
		  spell_cure_light(GET_LEVEL(ch), ch, "", SPELL_SPELL, ch, 0);
		  break;
		case 14: case 15: case 16: case 17: case 18: case 19: case 20:
		  act("$n utters the words 'saghkasdlghui'.",1,ch,0,0,TO_ROOM);
		  spell_cure_critic(GET_LEVEL(ch), ch,"",SPELL_SPELL,ch,0);
		  break;
		case 21: case 22: case 23: case 24: case 25:
		  act("$n utters the words 'heal'.", 1, ch, 0, 0, TO_ROOM);
		  spell_heal(GET_LEVEL(ch), ch, "", SPELL_SPELL, ch, 0);
		  break;
		case 26: case 27: case 28: case 29: case 30: case 31: case 32: case 33: case 34:
    	default:
      		act("$n utters the words 'sdagh'.", 1, ch, 0, 0, TO_ROOM);
      		spell_full_heal(GET_LEVEL(ch), ch, "", SPELL_SPELL, ch, 0);
      	break;
  		}
	}

  	if( number(1,3) == 1 && number(1,GET_LEVEL(ch)) > 30) 
  	{
     	do_multi_kick(ch, "", 0) ;
    }

	if( !is_fighting( ch ) ) return FALSE;
	
	vict = ch->fight;

  	if( number(1,3) == 1 )
  	{
    	if( GET_LEVEL(ch) > 20 && GET_LEVEL(ch) <= 25 && is_fighting( ch ) )
		{
      		act("$n utters the words 'dsagjlse'.", 1, ch, 0, 0, TO_ROOM);
      		spell_colour_spray(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
    	}
    	else if( GET_LEVEL(ch) > 25 && GET_LEVEL(ch) <= 30 && is_fighting( ch ) )
		{
      		act("$n utters the words 'ddsaghjkse'.", 1, ch, 0, 0, TO_ROOM);
      		spell_fireball(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
    	}
    	else if(GET_LEVEL(ch) > 30 && is_fighting( ch ) )
		{
      		act("$n utters the words 'oliel ese'.", 1, ch, 0, 0, TO_ROOM);
      		spell_sunburst(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
    	}
  	}
  	return TRUE;
}

int dragon(charType *ch, int cmd, char *arg)
{
  	charType *vict;

  	if( cmd ) return FALSE;
	if( !AWAKE( ch ) ) return 0;

  	if( !is_fighting(ch) ) return 0;

	vict = ch->fight;
  	switch( number(0,4) )
  	{
   	case 0:
   	case 1:
    	act("$n utters the words 'qassir plaffa'.", 1, ch, 0, 0, TO_ROOM);
    	spell_fire_breath(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
    	if( GET_HIT(vict)  < 300 || !is_fighting( ch ) )
			return TRUE;
   	case 2:
    	act("$n utters the words 'qassir porolo'.", 1, ch, 0, 0, TO_ROOM);
    	spell_gas_breath(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
    	if( GET_HIT(vict)  < 200 || !is_fighting( ch ) )
			return TRUE;
   	case 3:
    	act("$n utters the words 'qassir relata'.", 1, ch, 0, 0, TO_ROOM);
    	spell_lightning_breath(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
    	if( GET_HIT(vict)  < 100 || !is_fighting( ch ) )
			return TRUE;
   	case 4:
    	act("$n utters the words 'qassir moolim'.", 1, ch, 0, 0, TO_ROOM);
    	spell_frost_breath(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
  	}
  	return TRUE;
}

int guard(charType *ch, int cmd, char *arg)
{
  	charType *tch, *evil;
  	int max_evil;

  	if( cmd || !AWAKE(ch) || (GET_POS(ch) == POSITION_FIGHTING )) return (FALSE);

  	max_evil = 1001;
  	evil = NULL;

  	for( tch = world[ch->in_room].people; tch; tch = tch->next_in_room) 
  	{
    	if( is_fighting( tch ) )
		{
      		if( (GET_ALIGN(tch) < max_evil)&&(IS_NPC(tch->fight))) 
	  		{
				max_evil = GET_ALIGN(tch);
				evil = tch;
      		}
    	}
  	}
  	if (evil && (GET_ALIGN(evil->fight) >= 0) && ch != evil ) 
  	{
   		act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!!'",
     		FALSE, ch, 0, 0, TO_ROOM);
    	hit(ch, evil, TYPE_UNDEFINED);
    	return(TRUE);
  	}
  	return(FALSE);
}

void mob_rescue( charType * ch, charType * vict, charType * fight )
{
    sendf( ch, "Banzai! To the rescue..." );
    act( "$n screams 'PROTECT THE INNOCENT! BANZAI!'", FALSE, ch, 0, 0, TO_ROOM );
    act( "You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
    act( "$n heroically rescues $N.", FALSE, ch, 0, vict, TO_NOTVICT);

    stop_fighting( vict );
    stop_fighting( fight );

    stop_fighting(ch);

    set_fighting( ch, fight );
    set_fighting( fight, ch );
}

#define RESCUER_VICTIM	5
int rescuer(charType *ch, int cmd, char *arg)
{
  	charType *tmp_ch, *vict;

  	if(cmd) return FALSE;
	if( !AWAKE( ch ) ) return 0;

  	if( ch->fight && (IS_NPC(ch->fight)) ) 
	{
    	act("$n screams 'PROTECT THE INNOCENT! BANZAI!'",FALSE,ch,0,0,TO_ROOM);
    	mob_light_move(ch, "", 0);
    	return TRUE ;
    }

  	/* find the partner */
  	vict = choose_rescue_mob(ch) ;

  	if ( vict == ch ) 
	{
     	mob_light_move( ch, "", 0 );
     	switch(number(0,3)) 
		{
      	case 0 : vict = choose_victim(ch, VIC_ALL, MODE_HIT_MIN) ; break ;
      	case 1 : vict = choose_victim(ch, VIC_ALL, MODE_AC_MAX) ; break ;
      	case 2 : vict = choose_victim(ch, VIC_ALL, MODE_DR_MAX) ; break ;
      	case 3 : vict = choose_victim(ch, VIC_ALL, MODE_HR_MAX) ; break ;
      	}

     	if( vict && ( IS_NPC(vict) || GET_LEVEL(vict) < IMO) ) 
	 	{
       		act("$n screams 'PROTECT THE INNOCENT! CHARGE!'",FALSE,ch,0,0,TO_ROOM);
       		hit(ch, vict, TYPE_UNDEFINED) ;	/* set fighting */
     	}
     	return TRUE ;
    }

 	if( vict )
    {
        for( tmp_ch = world[ch->in_room].people; 
			tmp_ch && (tmp_ch->fight != vict); tmp_ch=tmp_ch->next_in_room )
		;

        if( !tmp_ch || ( !IS_NPC(tmp_ch) && (GET_LEVEL(tmp_ch) > IMO)) )
                return FALSE;

		mob_rescue( ch, vict, tmp_ch );

        return TRUE ;
    }
  	return FALSE;
}

#undef RESCUER_VICTIM

int superguard(charType *ch, int cmd, char *arg)
{
  void do_shout(charType *ch, char *argument, int cmd);
  charType *tch, *criminal, *king;
  char buf[80];

  if (cmd || !AWAKE(ch) || (GET_POS(ch) == POSITION_FIGHTING))
    return (FALSE);
  criminal = 0;
  for (tch=world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if(IS_NPC(tch)) continue;
    if(IS_SET(tch->act,PLR_CRIMINAL)){
      criminal=tch;
      break;
    }
  }
  if(criminal){
    act("$n 외칩니다. '누가 법을 깨뜨리는 녀석이냐 !!!!'",
     FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, criminal, TYPE_UNDEFINED);
    return(TRUE);
  }

  /* bow to king */
  king = 0 ;
  for (tch=world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if(IS_NPC(tch)) continue;
    if(tch->equipment[HOLD]) {
      if(objects[tch->equipment[HOLD]->nr].virtual==2706){
		/* it is okse */
        king=tch;
        break;
        }
      }
    }
   if (king) {
     act("$n bows before you.", FALSE,ch,0,king,TO_VICT) ;
     act("$n bows before $N.", FALSE,ch,0,king,TO_NOTVICT);
     if (number(1,5) > 4) {
       sprintf(buf, "OUR KING '%s' IS HERE !!", GET_NAME(king)) ;
       do_shout(ch, buf, 0) ;
       }
     return(TRUE);
     }

  return(FALSE);
}


int shooter(charType *ch, int cmd, char *arg)
{
  	charType *tch;
  	charType *tch_next;

  	if( cmd ) return(FALSE);
	if( !AWAKE( ch ) ) return 0;

  	if( GET_POS(ch) < POSITION_RESTING ) return(FALSE);

  	if( IS_SET(ch->act,ACT_AGGRESSIVE)||(GET_POS(ch)==POSITION_FIGHTING) )
  		for (tch=world[ch->in_room].people; tch; tch = tch_next)
  		{
  			tch_next = tch->next_in_room;
    		if( (!IS_NPC(tch))&&(GET_LEVEL(tch) < IMO) )
			{
      			if( GET_POS(tch) <= POSITION_DEAD ) continue;
      			act("$n yells '$N must die!'",FALSE,ch,0,tch,TO_ROOM);
      			shoot(ch, tch, TYPE_SHOOT);
    		}
  		}
  	return(FALSE);
}

int spitter(charType *ch, int cmd, char *arg)
{
  	charType *vict;

	if( !AWAKE( ch ) ) return 0;
  	if( cmd == COM_STEAL )
  	{
    	for( vict=world[ch->in_room].people;vict;vict=vict->next_in_room )
      		if( !IS_NPC(vict) )
	  		{
       			act("$n makes a disgusting noise - then spits at $N.",
        				1,ch,0,vict,TO_NOTVICT);
       			act("$n spit in your eye...",1,ch,0,vict,TO_VICT);
       			spell_blindness(GET_LEVEL(ch), ch,"",SPELL_SPELL,vict,0);
       			hit(ch, vict, TYPE_UNDEFINED);
	   			return (FALSE);
	   		}
    	return(TRUE);
  	}

  	if( !is_fighting(ch) ) return(FALSE);

  	for( vict = world[ch->in_room].people;vict; vict = vict->next_in_room)
  	{
    	if( IS_NPC(vict) || !vict->fight ) continue;

   		switch(number(1,7))
		{
   		case 1:
   		case 2:
   		case 3:
   			damage( ch, vict, 50, SKILL_KICK ); return(FALSE);
   		case 4:
      	case 5:
      	case 6:
       		act("$n makes a disgusting noise - then spits at $N.",
        			1,ch,0,vict,TO_NOTVICT);
       		act("$n spit in your eye...",1,ch,0,vict,TO_VICT);
       		spell_blindness(GET_LEVEL(ch), ch,"",SPELL_SPELL,vict,0);
       		return(FALSE);
      	default:
       		return(FALSE);
      	}
    	if( GET_LEVEL(ch) < 35 || dice(1,8) < 6) break ;
  	}
  	return(FALSE);
}

int kickbasher(charType *ch, int cmd, char *arg)
{
  	charType *vict;

  	if( cmd ) return(FALSE);
	if( !AWAKE( ch ) ) return 0;

  	if( is_fighting( ch ) )
  	{
  		vict = ch->fight;

    	if( number( 1, 2 ) == 1 ) do_kick(ch,"",0);
    	else do_bash(ch,"",0);

    	if( (GET_LEVEL(ch) > 20) && is_fighting(ch) )
		{
      		if( number( 1, 2 ) == 1 ) do_kick(ch,"",0);
      		else do_bash( ch, "", 0 );
    	}
    	if( (GET_LEVEL(ch) > 30 ) && is_fighting(ch) )
		{
      		if( number( 1, 2 ) == 1 ) do_kick(ch,"",0);
      		else do_bash( ch, "", 0 );
    	}
    	if( (GET_LEVEL(ch) > 35 ) && is_fighting(ch) )
		{
      		if( number( 1, 2 ) == 1 ) do_kick(ch,"",0);
      		else do_bash( ch, "", 0 );
    	}
    	if( (GET_LEVEL(ch) > 40 ) && is_fighting(ch) )
		{
      		if( number( 1, 2 ) == 1 ) do_kick(ch,"",0);
      		else do_bash( ch, "", 0 );
    	}
    }
    return(TRUE);
}

int helper(charType *ch, int cmd, char *arg)
{
  	charType 	*	vict, *next_victim;
  	int 					lev ;

  	if(cmd) return FALSE ;
	if( !AWAKE( ch ) ) return 0;

  	for( vict = world[ch->in_room].people; vict; vict = next_victim ) 
  	{
    	next_victim = vict->next_in_room ;
    	if( IS_NPC(vict) ) 
		{
      		if( (100*GET_HIT(vict)/GET_MAX_HIT(vict) ) > 85)
         		continue ;
      		else
         		break;
      	}
    }

  	if( !vict || number( 0, 4 ) > 3 ) return FALSE;

  	lev = GET_LEVEL(ch) ;

  	if( lev <= 11 ) 
  	{
      	act("$n utters the words 'asd iasp'.", 1, ch, 0, 0, TO_ROOM);
      	spell_cure_light(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
    }
  	else if ( lev <= 19 ) 
  	{
      	act("$n utters the words 'sagok sghui'.",1,ch,0,0,TO_ROOM);
      	spell_cure_critic(GET_LEVEL(ch), ch,"",SPELL_SPELL,vict,0);
    }
  	else if ( lev <= 27 ) 
  	{
      	act("$n utters the words 'laeh'.", 1, ch, 0, 0, TO_ROOM);
      	spell_heal(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
    }
  	else 
  	{
      	act("$n utters the words 'sjagh'.", 1, ch, 0, 0, TO_ROOM);
      	spell_full_heal(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
    }
  	return TRUE;
}

void scavenger( charType * ch )
{
 	objectType  *	obj, *best_obj;
 	int					max;

	if( !AWAKE( ch ) ) return;
	if( world[ch->in_room].contents && !number(0,5)) 
	{
		for( max = 1, best_obj = 0, obj = world[ch->in_room].contents;
			 obj; obj = obj->next_content) 
		{
			if( can_get(ch, obj) ) 
			{
				if( obj->cost > max ) 
				{
					 best_obj = obj;
					 max = obj->cost;
				}
			}
		}

		if( best_obj ) 
		{
			obj_from_room(best_obj);
			obj_to_char(best_obj, ch);
			act("$n gets $p.",FALSE,ch,best_obj,0,TO_ROOM);
		}
	}
}

void sentinel( charType * ch )
{
 	int 				door;

	if( !AWAKE( ch ) ) return;
	if(  (GET_POS(ch) == POSITION_STANDING) 
      && ((door = number(0, 40)) <= 5) 
	  && (can_go(ch,door)) ) 
	{
		if( ch->last_move == door ) ch->last_move = -1;
		else 
		{
			if( !IS_SET(ch->act, ACT_STAY_ZONE) ) 
			{
				ch->last_move = door;
				do_move(ch, "", ++door);
			} 
			else 
			{
				if (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone) 
				{
					ch->last_move = door;
					do_move(ch, "", ++door);
				}
			}
		}
	}
}

void aggressive( charType * ch )
{
 	charType *	tmp_ch,*cho_ch = 0;
	int 				found = FALSE;

	if( !AWAKE( ch ) ) return;
	for( tmp_ch = world[ch->in_room].people; tmp_ch; tmp_ch = tmp_ch->next_in_room) 
	{
		if( !IS_NPC(tmp_ch) && can_see(ch, tmp_ch) && (GET_LEVEL(tmp_ch)<IMO)) 
		{
			if(!IS_SET(ch->act, ACT_WIMPY) || !AWAKE(tmp_ch)) 
			{
				if( !found )
				{
					cho_ch = tmp_ch;
					found = TRUE;
					if(IS_EVIL(ch) && IS_GOOD(cho_ch) && IS_AFFECTED(cho_ch,AFF_PROTECT_EVIL))
					{
						if(!saves_spell(ch, tmp_ch, SAVING_PARA) && GET_LEVEL(ch) < GET_LEVEL(cho_ch))
						{
							act("$n tries to attack, but failed miserably.", TRUE,ch,0,0,TO_ROOM);
							found = FALSE;
						}
					}
				} 
				else 
				{
					if( number(1,6) <= 3 ) cho_ch = tmp_ch;
				} 
			} 
		} 
	} 
	if(found) hit( ch, cho_ch, 0 );
}

void mobile_activity( void )
{
 	charType *	ch, * ch_next;

 	for (ch = char_list; ch; ch = ch_next) 
 	{
		ch_next = ch->next;

  		if( !IS_MOB(ch) || IS_AFFECTED( ch, AFF_TIME_STOP ) ) continue;
  		if( GET_POS( ch ) < POSITION_SLEEPING ) continue;

		if( (IS_SET(ch->act, ACT_SPEC) && !no_specials) ) 
		{
   			if( !mobiles[ch->nr].func ) 
			{
   				DEBUG( "mobile_activity> Attempting to call a non-matching %s func of #%d.",	
 						ch->moved, mobiles[ch->nr].virtual );
   				REMOVE_BIT(ch->act, ACT_SPEC);
                REMOVE_BIT(mobiles[ch->nr].act, ACT_SPEC);
   			} 
			else 
			{
				if( (*mobiles[ch->nr].func) (ch, 0, "") )	continue;
   			}
		}

   		if( !is_fighting(ch)) 
   		{
    		if(  IS_SET(ch->act, ACT_SCAVENGER))  scavenger( ch );
    		if( !IS_SET(ch->act, ACT_SENTINEL))   sentinel( ch );
    		if(  IS_SET(ch->act, ACT_AGGRESSIVE)) aggressive( ch );
		}

		if( IS_AFFECTED( ch, AFF_CHARM ) ) continue;

   		if(IS_SET(ch->act, ACT_THIEF)) 		thief		(ch,0,"");
   		if(IS_SET(ch->act, ACT_DRAGON)) 	dragon		(ch,0,"");
   		if(IS_SET(ch->act, ACT_MAGE)) 		magic_user	(ch,0,"");
	   	if(IS_SET(ch->act, ACT_SHOOTER)) 	shooter		(ch,0,"");
	   	if(IS_SET(ch->act, ACT_FIGHTER)) 	kickbasher	(ch,0,"");
	   	if(IS_SET(ch->act, ACT_SPITTER)) 	spitter		(ch,0,"");
	   	if(IS_SET(ch->act, ACT_CLERIC)) 	cleric		(ch,0,"");
	   	if(IS_SET(ch->act, ACT_PALADIN)) 	paladin		(ch,0,"");
	   	if(IS_SET(ch->act, ACT_GUARD)) 		guard		(ch,0,"");
	   	if(IS_SET(ch->act, ACT_SUPERGUARD)) superguard	(ch,0,"");
	   	if(IS_SET(ch->act, ACT_RESCUER))	rescuer		(ch,0,"");
   		if(IS_SET(ch->act, ACT_HELPER)) 	helper		(ch,0,"");
  	}
}
