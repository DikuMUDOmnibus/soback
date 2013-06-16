/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "limits.h"
#include "quest.h"
#include "misc.h"
#include "variables.h"
#include "fight.h"
#include "mobile.action.h"
#include "strings.h"
#include "weather.h"
#include "specials.h"
#include "affect.h"
#include "transfer.h"
#include "spec.rooms.h"

int mayor(charType *ch, int cmd, char *arg)
{
  static char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char *path;
  static int index;
  static bool move = FALSE;

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }

  if (cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) ||
    (GET_POS(ch) == POSITION_FIGHTING))
    return FALSE;

  switch (path[index]) {
    case '0' :
    case '1' :
    case '2' :
    case '3' :
      do_move(ch,"",path[index]-'0'+1);
      break;

    case 'W' :
      GET_POS(ch) = POSITION_STANDING;
      acthan("$n awakens and groans loudly.",
        "$n님이 일어나 기지개를 켜십니다", FALSE,ch,0,0,TO_ROOM);
      break;

    case 'S' :
      GET_POS(ch) = POSITION_SLEEPING;
      acthan("$n lies down and instantly falls asleep.",
        "$n님이 자리에 눕더니 금새 잠이 듭니다.", FALSE,ch,0,0,TO_ROOM);
      break;

    case 'a' :
      acthan("$n says 'Hello Honey!'",
         "$n님이 '안녕 내사랑!' 이라고 말합니다.", FALSE,ch,0,0,TO_ROOM);
      acthan("$n smirks.","$n님이 점잔빼며 웃습니다.", FALSE,ch,0,0,TO_ROOM);
      break;

    case 'b' :
      acthan("$n says 'What a view! I must get something done about dump!'",
        "$n님이 '하! 쓰레기에 대해서 뭔가 조치를 취해야 겠네!' 라고 말합니다",
        FALSE,ch,0,0,TO_ROOM);
      break;

    case 'c' :
      acthan("$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
        "$n님이 '이런 이런! 젊은것들은 요즘 인사를 안한단말야!' 라고 말합니다.",
        FALSE,ch,0,0,TO_ROOM);
      break;

    case 'd' :
      acthan("$n says 'Good day, citizens!'",
        "$n님이 '시민 여러분 안녕들 하십니까!'라고 말합니다",  FALSE, ch, 0,0,TO_ROOM);
      break;

    case 'e' :
      acthan("$n says 'I hereby declare the bazaar open!'",
         "$n님이 '자 이제 시장을 열겠습니다 !' 라고 말합니다", FALSE,ch,0,0,TO_ROOM);
      break;

    case 'E' :
      acthan("$n says 'I hereby declare Midgaard closed!'",
         "$n님이 '자 이제 문을 닫았습니다!' 라고 말합니다", FALSE,ch,0,0,TO_ROOM);
      break;

    case 'O' :
      do_unlock(ch, "gate", 0);
      do_open(ch, "gate", 0);
      break;

    case 'C' :
      do_close(ch, "gate", 0);
      do_lock(ch, "gate", 0);
      break;

    case '.' :
      move = FALSE;
      break;
  }
  index++;
  return FALSE;
}

int super_deathcure(charType *ch, int cmd, char *arg)
{
  	charType *	vict,* mob;
  	int 		h, real_number, ran_num;

  	if( cmd ) return(0);
	if( !AWAKE( ch ) ) return 0;

  	vict = choose_victim(ch, VIC_ALL, MODE_MOVE_MAX) ;

    h = GET_HIT(ch);

    if( h < 7000 ) spell_full_heal(GET_LEVEL(ch), ch, "", SPELL_SPELL, ch, 0);

  	if( vict && !IS_NPC(vict) && (GET_LEVEL(vict) < IMO)) 
  	{
    	if( number(1,10) > 7 )
		{
      		act("$n control your move -100.",1,ch,0,0,TO_ROOM);
			vict->move=0;vict->move-=200;
      		act("$n Wuhhhhhh...... thank you very much ? .",1,ch,0,0,TO_ROOM);	
			return TRUE;	
    	}
    	else if( number(1,10) > 9 )
		{
			act("$n hmmmmm.... miracle !!!",1,ch,0,0,TO_ROOM);	
			vict->mana+=40*(1+GET_LEVEL(vict)/9);
			return TRUE;
    	}	
  	}

  	if( is_fighting( ch ) )
  	{
        h = number(0,12);
        if( h > 8 ) return(0);

	  	switch( h )
		{
        	case 0:ran_num=5107;break;  /* mother */
            case 1:ran_num=5010;break;  /* dracolich */
            case 2:ran_num=5004;break;  /* worm */
            case 3:ran_num=2109;break;  /* medusa third gorgon */
            case 4:ran_num=2118;break;  /* hecate */
            case 5:ran_num=2225;break;  /* green dragon : rosa */
            case 6:ran_num=15107;break; /* demi robot : cyb */
			case 7:ran_num=17015;break;
            case 8:ran_num=16014;break; /* head soldier wee : cyb */
            default:return(1);
		}

		if( real_number = real_mobileNr(ran_num), real_number != MOBILE_NULL )
		{
	    	act("$n utters 'Em pleh! Em pleh!! Em pleh!!!",1,ch,0,0,TO_ROOM);	
	    	mob = load_a_mobile(real_number,REAL);
	    	char_to_room( mob, ch->in_room );
			hit( mob, ch->fight, TYPE_UNDEFINED );
		}
		return(1);
	}
	return(0);
}

int diana( charType * ch, int cmd, char * arg )
{
	return 0;
}

int Quest_bombard( charType * ch, int cmd, char * arg )
{   /* question function and level up monster */

  	charType 		*	vict;
  	objectType 		*	list;
  	affectType 			af;
  	int 				bombard_stat=0,maxnum=0;
  	int 				newnum = 0 ;

	/* bombard like sword 2702 */
	/* bombard room is 3035 and other room is 3094 */
	/* if ch->in_room == real_roomNr(3035)  .... */

  	if( cmd ) return(0);	
	if( !AWAKE( ch ) ) return 0;

	if( !is_fighting( ch ) && !IS_AFFECTED( ch, AFF_BLIND ) )
	{
		if( GET_STR( ch ) < 18 )
		{
			do_say( ch, "Huuueew... I'm too weak to make a bombard sword now.", COM_SAY );
			return 1;
		}

  		if( ch->in_room == real_roomNr(3035)) newnum = 3027 ;
  		else if( ch->in_room == real_roomNr(3094)) newnum = 2702 ;
  		else newnum = 3027 ;

  		for( list = ch->carrying; list; list = list->next_content)
  		{
     		if( objects[list->nr].virtual==3025 ) 
	 		{
	 			maxnum++;
     			bombard_stat = 1;
	 		}
  		}

  		if( maxnum >= 7 ) 
  		{
     		while( (list = ch->carrying) )
	 		{
       			extract_obj( list, 1 );
      		}

     		if( (newnum = real_objectNr(newnum)) != OBJECT_NULL ) 
	 		{
        		list = load_a_object( newnum, REAL, 1 );
            	obj_to_char(list,ch);
				equip_char( ch, list, WIELD );
			}
    		return(1); 
  		}
  		if( maxnum < 7 && maxnum > 0 ) do_say( ch, "I need more particle.", 0 );
  	}

  	if( ch->master && ch->in_room == ch->master->in_room ) 
  	{  
  		hit( ch, ch->master, TYPE_UNDEFINED ); bombard_stat=1; 
  	}

  	if( is_fighting( ch ) )
  	{
  		vict = ch->fight;
   		if( ch->hit %2 == 1 && ch->level < 35 )  
   		{
	    	act("$n	get experience .",1,ch,0,0,TO_ROOM);
	    	act("$n	LEVEL UP !.",1,ch,0,0,TO_ROOM);
        	ch->level+=1;
    	}
  		if( ch->fight && !affected_by_spell(ch,SPELL_SANCTUARY) )
  		{
    		act("$n is surrounded by a white aura.",TRUE,ch,0,0,TO_ROOM);
    		af.type      = SPELL_SANCTUARY;
    		af.duration  = 4;
    		af.modifier  = 0;
    		af.location  = APPLY_NONE;
    		af.bitvector = AFF_SANCTUARY;
    		affect_to_char( ch, &af );
    		return(1);
    	}

    	if( is_fighting(ch) && GET_LEVEL(ch) == 35 )
		{ 
           	do_say(ch, "Shou Ryu Ken..", 0);
            vict->hit -= vict->hit / 3 - number( 1, vict->hit/8+GET_LEVEL(vict)/2 );
           	send_to_char("당신은 꽈당 넘어집니다.\n\r",vict);
           	send_to_char("크으으아아아아 . .  .  . \n\r",vict);
    	}
    	if( is_fighting(ch) && GET_LEVEL(ch) >= 30 )
		{
            act("$n utters the words 'sunburst'.", 1, ch, 0, 0, TO_ROOM);
            spell_sunburst(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
    	}
    	if( is_fighting(ch) && GET_LEVEL(ch) >= 25)
		{
            act("$n utters the words 'fireball'.", 1, ch, 0, 0, TO_ROOM);
            spell_fireball(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
    	}
    	if( is_fighting(ch) && GET_LEVEL(ch) >= 20 )
		{
            hit(ch,vict,TYPE_UNDEFINED);
    	}
    	if( is_fighting(ch) && GET_LEVEL(ch) >= 15 ) hit(ch,vict,TYPE_UNDEFINED);
    	if( is_fighting(ch) && GET_LEVEL(ch)>= 10) damage(ch,vict,GET_LEVEL(ch),SKILL_BASH);
    	if( is_fighting(ch) && GET_LEVEL(ch)>= 5) damage(ch,vict,GET_LEVEL(ch),SKILL_KICK);

        return(1); 
  	}
  	if( bombard_stat == 1 ) return(1);
 	return (FALSE);
}

int mud_message( charType * ch, int cmd, char * arg )
{
  	charType * vict;

  	if( cmd ) return(0);	
	if( !AWAKE( ch ) ) return 0;
  
  	if( is_fighting(ch) )
  	{
  		vict = ch->fight;

		act("$n\tannihilate $N with his full power.\n\r",1, ch, 0, vict, TO_ROOM );

		if( is_fighting(ch) && vict->hit>=51  && vict->hit<101) 	
				damage(ch,vict,dice(14,GET_LEVEL(ch)),TYPE_UNDEFINED);
		if( is_fighting(ch) && vict->hit>=101 && vict->hit<201) 	
				damage(ch,vict,dice(16,GET_LEVEL(ch)),TYPE_UNDEFINED);
		if( is_fighting(ch) && vict->hit>=201 && vict->hit<401) 	
				damage(ch,vict,dice(20,GET_LEVEL(ch)),TYPE_UNDEFINED);
		if( is_fighting(ch) && vict->hit>=401 && vict->hit<801) 	
				damage(ch,vict,dice(25,GET_LEVEL(ch)),TYPE_UNDEFINED);
		if( IS_NPC(vict) ) return TRUE;
		if( is_fighting(ch) && vict->hit>=801 && vict->hit<1601) 	
				damage(ch,vict,dice(28,GET_LEVEL(ch)),TYPE_UNDEFINED);
		if( is_fighting(ch) && vict->hit>=1601 && vict->hit<2001) 	
				damage(ch,vict,dice(33,GET_LEVEL(ch)),TYPE_UNDEFINED);
		if( is_fighting(ch) && vict->hit>=2001) 					
				damage(ch,vict,dice(40,GET_LEVEL(ch)),TYPE_UNDEFINED);
     	return(TRUE);
    }
  	return( FALSE );
}

int mom( charType * ch, int cmd, char * arg )
{
  	charType *	vict;
  	int 		h;

  	if( cmd ) return(0);
	if( !AWAKE( ch ) ) return 0;

  	if( vict = ch->fight, is_fighting( ch ) )
  	{
    	h = GET_HIT(ch);
    	if( (h > 100) && (h < 999)) 
		{
      		act("$n 이 크고 푸른 알약을 삼킵니다. (윙크)",1,ch,0,0,TO_ROOM);
      		spell_heal(GET_LEVEL(ch), ch, "", SPELL_SPELL, ch, 0);
      		return TRUE;
    	} 

		if( vict->in_room == ch->in_room )
		{
    		if( (GET_MOVE(vict) > 50)) 
			{ 
      			sendf( vict, "으라차아  !");
      			GET_MOVE(vict) -= dice(10,10);
      		}
        	shoot(ch,vict,TYPE_SHOOT);
    	}
		return 1;
  	}
  	else
  	{
	  	switch (number(0,50))
		{
		case 0: 		do_say(ch, "오늘 할 일을 내일로 미루지 마라.", 0); 		return(1);
		case 1: 		do_say(ch, "집에서 너무 멀리 가지 말아라.", 0); 		return(1);
		case 2: 		do_say(ch, "일찍 자고 일찍 일어나라.", 0); 		return(1);
		case 3: 		do_say(ch, "얘들아 잘시간이다.", 0); 		return(1);
		case 4: 		do_say(ch, "누가 여기 쓰레기 치우지 않았지 ?", 0); 		return(1);
		case 5: 		do_say(ch, "얘 ! 방좀 가서 치워라.", 0); 		return(1);
		case 6: 		do_say(ch, "밥먹기 전에는 손을 씻어야지.", 0); 		return(1);
		case 7: 		do_say(ch, "밤늦게 까지 오락 하지 말랬지 !",0); 		return(1);
		case 8: 		do_say(ch, "넌 집에서 가정교육을 좀 더 받아야해", 0); 		return(1);
		case 9: 		do_say(ch, "보고서 다 썼니 ?", 0); 		return(1);
		case 10: 		do_say(ch, "숙제 다 했어 ?", 0); 		return(1);
		default: 		return(0);
  		}
  	}
}

static char * song[] =
{
     "$n sings, '...lights flicker from the opposite loft'",
     "$n sings, 'in this room the heat pipes just coughed'",
     "$n sings, 'the country music station plays soft'",
     "$n sings, 'but there's nothing, really nothing, to turn off...'"
};

int singer( charType *ch, int cmd, char *arg )
{
  	static int n = 0;
  	if( cmd ) return FALSE;

  	if( GET_POS(ch) != POSITION_STANDING ) return FALSE;
  	if( n < 4 ) act(song[n],TRUE,ch,0,0,TO_ROOM);
  	if( (++n) == 16) n = 0;
  	return TRUE;
}

int fido(charType *ch, int cmd, char *arg)
{
  	objectType *i, *temp, *next_obj;

  	if( cmd || !AWAKE(ch) ) return(FALSE);

  	for( i = world[ch->in_room].contents; i; i = i->next_content ) 
  	{
    	if( i->type == ITEM_CONTAINER && i->nr == OBJECT_CORPSE )
		{
      		act("$n savagely devour a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      		for( temp = i->contains; temp; temp = next_obj ) 
	  		{
        		next_obj = temp->next_content;
        		obj_from_obj(temp);
        		obj_to_room(temp,ch->in_room);
      		}
      		extract_obj( i, 1 );
      		return(TRUE);
    	}
  	}
  	return(FALSE);
}

int janitor(charType *ch, int cmd, char *arg)
{
  	objectType 	*	i;

  	if( cmd || !AWAKE(ch) ) return(FALSE);

  	for( i = world[ch->in_room].contents; i; i = i->next_content ) 
  	{
    	if( IS_SET( i->wear, ITEM_TAKE) && ((i->type == ITEM_DRINKCON) || (i->cost <= 1000))) 
		{
      		act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
      		obj_from_room(i);
      		obj_to_char(i, ch);
      		return(TRUE);
    	}
  	}
  	return(FALSE);
}

int puff(charType *ch, int cmd, char *arg)
{
  	if( cmd ) return(0);

  	switch( number(0, 60) ) 
  	{
    case 0: 	do_say(ch, "My god! It's full of stars!", 0); return(1);
    case 1: 	do_say(ch, "How'd all those fish get up here?", 0); return(1);
    case 2: 	do_say(ch, "I'm a very female dragon.", 0); return(1);
    case 3: 	do_say(ch, "I've got a peaceful, easy feeling.", 0); return(1);
    default: 	return(0);
  	}
}

static int value_repair( charType * keeper, charType * client, objectType * obj, char * card, int cmd )
{
	char	buf[MAX_STRING_LENGTH];
	int		value = 0, limit, diff;

	limit = obj->limit; 
	limit = limit ? limit : 100;
	diff  = limit - obj->status;

	if( diff < 2 )
	{
		sprintf(buf, "%s %15s could not be better.", GET_NAME(client), onewordc(obj->name) );
		do_tell( keeper, buf, 19 );
		return value;
	}

	limit -= diff * 2 / 9;
	value = obj->cost * diff / 100;

	if( cmd == COM_VALUE )
	{
  		sprintf(buf,"%s It costs %d gold coins for %s!", GET_NAME(client), value, oneword(obj->name) );
  		do_tell( keeper, buf, 19 );
  		return value;
	}

	if( !cost_from_player( client, keeper, card, value, 0 ) ) return value;

  	act("$n repairs $p.", FALSE, client, obj, 0, TO_ROOM);
  
   	GET_GOLD(keeper) += value/3;

	obj->limit  = limit;
	obj->status = limit;

	return value;
}

int mikesmith( charType * ch, int cmd, char * arg )
{
 	char 			argm[100], card[MAX_INPUT_LENGTH+1], buf[MAX_STRING_LENGTH];
  	objectType 	*	temp;
	charType	* 	keeper = 0, * temp_char ;
	char		*	what;
	int				objNr;

	if( !cmd || (cmd != COM_REPAIR && cmd != COM_VALUE && 
                 cmd != COM_LIST   && cmd != COM_GIVE ) ) return 0;

	if( !arg ) return 0;

  	twoArgument(arg, argm, card);

  	for( temp_char = world[ch->in_room].people; (!keeper) && (temp_char) ;
  										temp_char = temp_char->next_in_room)
		if( IS_NPC(temp_char) &&  mobiles[temp_char->nr].func == mikesmith )
				  keeper = temp_char;

	if( !keeper ) return 0;

	if( cmd == COM_LIST )
	{
		sprintf( buf, "%s I don't sell or buy. Look a note.", GET_NAME(ch) );
		do_tell( keeper, buf, 19 );
		return 1;
	}
	
  	if( !*argm )
  	{
    	sprintf(buf,"%s What do you want me to repair??", GET_NAME(ch)); 
    	do_tell( keeper, buf, 19 );
    	return 1;
  	}

  	if( cmd == COM_GIVE && *card )
	{
		temp_char = find_char_room( ch, card );

		if( temp_char == keeper )
		{
			sprintf( buf, "%s Don't give me anything." , GET_NAME(ch) );
			do_tell( keeper, buf, 19 );
			return 1;
		}
		else return 0;
	}

	if( objNr = splitarg( argm, &what ), objNr > 0 )
	{
  		if( !( temp = find_obj_inven( ch, argm )) )
			if( !( temp = find_obj_equip( ch, argm )) )
  			{
    			sprintf( buf, "%s You don't have that item!", GET_NAME(ch));
    			do_tell( keeper, buf, 19 );
    			return 1;
  			}
  		value_repair( keeper, ch, temp, card, cmd );
  	}
  	else if( objNr == 0 )
  	{
  		int			i, ofound, totalValue = 0;

		for( ofound = 0, i = 0; i < MAX_WEAR; i++ )
		{
			if( !ch->equipment[i] ) continue;

			if( !*what || isoneof( what, ch->equipment[i]->name ) )
			{
				if( ++ofound < objNr && objNr ) continue;

				totalValue += value_repair( keeper, ch, ch->equipment[i], card, cmd );

				if( objNr ) break;
			}
		}
		if( cmd == COM_VALUE )
		{
			sendf( ch, "%s tells you '%d coins for all.", onewordc(keeper->name), totalValue );
		}
	}
  	else
  	{
   		sendf( ch, "%s tells you, '%s I don't understand that!'", keeper->moved );
	}

	return 1;
}
