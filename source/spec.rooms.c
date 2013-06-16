#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

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
#include "fight.h"
#include "variables.h"
#include "strings.h"
#include "weather.h"
#include "specials.h"
#include "affect.h"
#include "transfer.h"
#include "spec.rooms.h"
#include "page.h"

int electric_shock(charType *ch, int cmd, char *arg)
{
  char *msg;
  int weather, shock ;

  shock = 0 ;
  weather = weather_info.sky ;
  if (!IS_NPC(ch) && ( cmd == COM_NORTH || cmd == COM_EAST || cmd == COM_SOUTH
	|| cmd == COM_WEST || cmd == COM_UP || cmd == COM_DOWN)) {
    switch (weather) {
      case SKY_RAINING :
             msg = "ZZirrrr... 비가 오니까 찌릿 합니다.\n\r" ;
             shock = 5 ;
             break ;
      case SKY_LIGHTNING :
             msg = "ZZirk ZZirk .. 이런 날씨에는 감전되어 죽겠습니다.\n\r" ;
             shock = dice(5,7) ;
             break ;
      case SKY_CLOUDLESS :
      case SKY_CLOUDY :
             return FALSE ;
      default :
             return FALSE ;
      }
      send_to_char(msg,ch);
      if ( GET_HIT(ch) < 150 ) shock = shock / 2 ;
      if ( GET_HIT(ch) < 60 ) shock = shock / 2 ;
      if ( GET_HIT(ch) < 30 ) shock = 1 ;
      if ( GET_HIT(ch) < 3 ) shock = 0 ;
      GET_HIT(ch) -= shock ;
    }
   return FALSE ;
}

int level_gate(charType *ch, int cmd, char *arg)
{
  int f,r;

  f = 0;        r=world[ch->in_room].virtual;
  switch ( cmd ) {
        case COM_NORTH : 
				break;
        case COM_EAST  :
				if( r == 3046 ) f = (GET_LEVEL(ch) > 10);
               break;
        case COM_SOUTH :	/* south */
                if(r==1453) f=(GET_LEVEL(ch) < 12);
                else if(r==3041) f=(GET_LEVEL(ch) < 25);
                else if(r==2535) f=(GET_LEVEL(ch) > 15);
                else if(r==6001) f=(GET_LEVEL(ch) > 12);
                else if(r==9400) f=(GET_LEVEL(ch) > 3);
                break;
        case COM_WEST :	/* west */
                if(r==3500) f=(GET_LEVEL(ch) > 20);
                else if(r==5200) f=(GET_LEVEL(ch) > 24 || GET_LEVEL(ch) < 10);
                break;
        case COM_UP :	/* up */
               if(r==3001) f=(GET_LEVEL(ch) > 2);
               break;
        case COM_DOWN :	/* down */
        default :
                break;
  }

  if((f)&&(GET_LEVEL(ch) < IMO))
  {
          act("$n attempts go to where $e is not welcome.",FALSE,ch,0,0,TO_ROOM);
          send_to_char("People of your level may not enter.\n\r",ch);
          return TRUE;
  }
  return FALSE;
}

int neverland(charType *ch, int cmd, char *arg)
{
  int location,loc_nr;

  if(cmd != COM_DOWN )   /* specific to Room 2707. cmd 6 is move down */
    return(FALSE);
  switch(number(1,4)) {
    case 1:  loc_nr = 2720;
             break ;
    case 2:  loc_nr = 2721;
             break ;
    case 3:  loc_nr = 2722;
             break ;
    case 4:  loc_nr = 2723;
             break ;
    default: loc_nr = 2724;
    }
    
  location = real_roomNr(loc_nr) ;
  act("$n씨가 지금 내려 갔나요 ??",FALSE,ch,0,0,TO_NOTVICT);
  send_to_char("악.\n\n악..\n\n 악...\n\n\n떨어지고 있습니다..\n\n\r",ch);
  send_to_char("여기가 어딜까 ??\n\r", ch) ;
  char_from_room(ch);
  char_to_room(ch,location);
  return(TRUE);
}


int pet_shops(charType *ch, int cmd, char *arg)
{
  	char 			buf[MAX_INPUT_LENGTH+1], card[MAX_INPUT_LENGTH+1];
  	int 			pet_room,k;
  	charType 	*	pet;
  	followType 	*	j;

  	if( IS_NPC(ch) ) return(FALSE);

  	pet_room = ch->in_room+1;

  	if( cmd == COM_LIST) 
  	{ /* List */
    	send_to_char("애완동물 이 이런 것이 있습니다:\n\r", ch);
    	for(pet = world[pet_room].people; pet; pet = pet->next_in_room) 
		{
			if( pet->moved )
			{
      			sprintf(buf, "%8d - %s\n\r",10*GET_EXP(pet), pet->moved);
      			send_to_char(buf, ch);
			}
    	}
    	return(TRUE);
  	} 
  	else if (cmd== COM_BUY ) 
  	{ 	
    	twoArgument( arg, buf, card );

    	for( pet = world[pet_room].people; pet; pet = pet->next_in_room )
			if( isoneof( buf, pet->name ) ) break;

    	if( !pet ) 
		{
      		send_to_char("아 그런 동물은 없는데요 ?\n\r", ch);
      		return(TRUE);
    	}

    	for( k = 0, j = ch->followers; (j) && (k<5); )
		{
			if( IS_NPC( j->follower ) ) k++;
      		j = j->next;
    	}
    	if( k >= 4 )
		{
      		send_to_char("당신은 지금 데리고 있는 동물로 충분할 것 같은데요.\n\r",ch);
      		return(TRUE);
    	}

		if( !cost_from_player( ch, 0, card, GET_EXP(pet) * 10, 0 ) ) return 1;

    	pet 		  = load_a_mobile(pet->nr, REAL);
    	GET_EXP(pet)  = 0;
		GET_GOLD(pet) = 0;
    	SET_BIT(pet->affects, AFF_CHARM);
/*
    	if( *pet_name ) 
		{
      		sprintf(buf,"%s %s", pet->name, pet_name);
      		pet->name = strdup( buf );    

      		sprintf(buf,"%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
        			pet->description, pet_name);
      		pet->description = strdup(buf);
    	}
*/
    	char_to_room(pet, ch->in_room);
    	add_follower(pet, ch);

    	/* Be certain that pet's can't get/carry/use/wield/wear items */

		GET_STR( pet ) = 1;

    	send_to_char("애완동물이 마음에 드실껍니다.\n\r", ch);
    	act("$n님이 $N 을 데리고 갑니다.",FALSE,ch,0,pet,TO_ROOM);

    	return(TRUE);
  	}

  	return(FALSE);
}

int is_hold_insurance(charType *ch)
{
  	if( ch->equipment[HOLD] ) 
  	{
    	if( objects[ch->equipment[HOLD]->nr].virtual == 2562 )
			return 1;
  	}  
  	return 0;
}

int hospital( charType * ch, int cmd, char * arg )
{
  	char 			buf[MAX_INPUT_LENGTH+1], card[MAX_INPUT_LENGTH+1];
  	int 			opt,lev,cost[5], i;

  	lev     = GET_LEVEL(ch);
  	cost[0] = lev * lev * 2;

  	if( GET_CLASS(ch) == CLASS_MAGIC_USER ) cost[0] -= (cost[0] /3); /*  33% */
  	if( is_hold_insurance(ch) ) 			cost[0] -= (cost[0] /5); /* discount 20% */

  	cost[1] = 3345 + cost[0] * (GET_MAX_HIT(ch)  - GET_HIT(ch)) ;
  	cost[2] = 2353 + cost[0] * (GET_MAX_MANA(ch) - GET_MANA(ch)) ;
  	cost[3] = 2122 + cost[0] * (GET_MAX_MOVE(ch) - GET_MOVE(ch)) ;
  	cost[4] = 557  + cost[0] * 20 ;

  	for( i = 0; i < 4; i++ ) cost[i] = cost[i] < 0 ? 0 : cost[i];

  	if( cmd == COM_LIST ) 
  	{
    	sendf( ch, "1 - Hit points restoration (%d coins)", cost[1] );
    	sendf( ch, "2 - Mana restoration (%d coins)\n\r", cost[2] );
    	sendf( ch, "3 - Move restoration (%d coins)\n\r", cost[3] );
    	sendf( ch, "4 - Poison cured (%d coins)\n\r", cost[4] );
    	return(TRUE);
  	} 
  	else if( cmd == COM_BUY ) 
  	{ 
    	arg = twoArgument( arg, buf, card );
    	if( getnumber( buf, &opt ) <= 0 || opt > 4 ) 
		{
      		sendf( ch, "Huh?" ); return TRUE;
      	}

    	if( !cost_from_player( ch, 0, card, cost[opt], 0 ) ) return 1;

    	switch(opt)
		{
      	case 1: 
	  			GET_HIT(ch) = GET_MAX_HIT(ch);
        		sendf( ch, "You feel magnificent!" ); 		break;
      	case 2:
        		GET_MANA(ch) = GET_MAX_MANA(ch);
        		sendf( ch, "You feel marvelous!" );			break;
      	case 3:
        		GET_MOVE(ch) = GET_MAX_MOVE(ch);
        		sendf( ch, "You feel The GOD!" );			break;
      	case 4:
        		if( affected_by_spell( ch, SPELL_POISON ) )
				{
           			affect_from_char( ch, SPELL_POISON );
           			sendf( ch, "You feel stupendous!" );
        		} 
				else 
				{
           			sendf( ch, "Nothing wrong with you." );
        		}
				break;
      	default:
        		sendf( ch, "What??" );
    	}

		GET_HIT(ch)  = MIN( GET_MAX_HIT(ch),  GET_HIT(ch) );
		GET_MOVE(ch) = MIN( GET_MAX_MOVE(ch), GET_MOVE(ch) );
		GET_MANA(ch) = MIN( GET_MAX_MANA(ch), GET_MANA(ch) );

		return TRUE;
  	}
  	return(FALSE);
}

#define FUDGE (100+dice(6,20))

int guild(charType *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  char prbuf[80*MAX_SKILLS];
  int number,i,percent,count;
  int lev,cla;

  if ((cmd!=COM_ADVANCE)&&(cmd!=COM_PRACTICE)&&(cmd!=COM_PRACTISE))
		return(FALSE);

  if(cmd==COM_ADVANCE){ /* advance */
    if (!IS_NPC(ch)) {
      for (i = 0; titles[GET_CLASS(ch)-1][i].exp <= GET_EXP(ch); i++) {
        if(i >= IMO){
          send_to_char_han("Immortality cannot be gained here.\n\r",
             "여기서는 신이 될 수 없습니다.\n\r",ch);
          return(TRUE);
        }
        if (i > GET_LEVEL(ch)) {
          send_to_char_han("You raise a level\n\r",
             "레벨을 올렸습니다\n\r", ch);
          GET_LEVEL(ch) = i; advance_level(ch); set_title(ch);
          return(TRUE);
        }
      }
      send_to_char_han("You need more experience.\n\r",
         "경험이 더 필요합니다.\n\r",ch);
      return(TRUE);
    }
  }

  /* practice */
  lev=GET_LEVEL(ch); cla=GET_CLASS(ch)-1;
  for(; *arg==' '; arg++);
  if (!*arg) {
    if (IS_SET(ch->act, PLR_KOREAN))
       strcpy(prbuf, "당신은 다음과 같은 기술을 익힐 수 있습니다:\n\r");
    else
       strcpy(prbuf, "You can practice any of these skills:\n\r") ;

    for(i=0, count=0; *spells[i].name != '\n'; i++){
      if(*spells[i].name && (spells[i].min_level[cla] <= lev)) {
        sprintf(buf, "%s%s", spells[i].name, how_good(ch->skills[i])) ;
        if (count%2 == 0)
           sprintf(buf2, "%-35s", buf) ;
        else
           sprintf(buf2, "%s\n\r", buf) ;
		strcat(prbuf, buf2);
        count++;
      }
    }
    strcat(prbuf, "\n\r") ;
    sprintf(buf,"You have %d practices left.\n\r",
      ch->prNr);
    sprintf(buf2,"지금 %d 번 기술을 연마(practice)할 수 있습니다. \n\r",
      ch->prNr);
/*
    send_to_char_han(buf,buf2,ch);
*/
    if (IS_SET(ch->act, PLR_KOREAN))
       strcat(prbuf, buf2) ;
    else
       strcat(prbuf, buf) ;
    print_page( ch, prbuf );
    return(TRUE);
  }

  	number = spell_lookup( arg );

  	if(number == -1) 
  	{
    	send_to_char_han("You do not know of this spell...\n\r",
      					 "그런 기술은 모르는데요 ...\n\r", ch);
    	return(TRUE);
  	}
  	if( lev < spells[number].min_level[cla] ) 
  	{
    	send_to_char_han("Your level is too low.\n\r",
      					 "아직은 레벨이 낮아 안됩니다...\n\r",ch);
    	return(TRUE);
  	}
  	if( ch->prNr <= 0 ) 
  	{
    	send_to_char_han("You do not seem to be able to practice now.\n\r",
      					"지금은 더이상 배울 수 없습니다.\n\r", ch);
    	return(TRUE);
  	}

  	if( ch->skills[number] >= spells[number].max_skill[cla])
  	{
    	send_to_char_han("You know this area as well as possible.\n\r",
      					 "그 분야는 배울 수 있는만큼 배웠습니다.\n\r",ch);
    	return(TRUE);
  	}
  	send_to_char_han("You Practice for a while...\n\r",
     				 "기술이 늘고 있습니다...\n\r", ch);
  	ch->prNr--;
  	percent = ch->skills[number]+1+
          	((int)int_app[(int)ch->base_stat.intel].learn
          	*(int)spells[number].max_skill[cla])/FUDGE;

  	ch->skills[number]=	MIN(spells[number].max_skill[cla],percent);
  	if(ch->skills[number] >= spells[number].max_skill[cla])
  	{
    	send_to_char_han("You're now as proficient as possible.\n\r",
       					 "이 분야에는 이미 배울만큼 다 배웠습니다.\n\r",ch);
  	}
  	return(TRUE);
}


int dump(charType *ch, int cmd, char *arg) 
{
   	objectType 			*	k;
   	char 					buf[100];
   	charType 			*	tmp_char;
   	int 					value=0, x;
	int						trashquest ;

  	for( k = world[ch->in_room].contents; k && k->nr !=OBJECT_PORTAL  ; 
  		k = world[ch->in_room].contents) 
  	{
    	sprintf(buf, "The %s vanish in a puff of smoke.\n\r" ,oneword(k->name));
    	for( tmp_char = world[ch->in_room].people; tmp_char; tmp_char = tmp_char->next_in_room )
      	if( can_see_obj(tmp_char, k) ) send_to_char(buf,tmp_char);
    	extract_obj( k, 1 );
  	}

  	if( cmd != COM_DROP ) return(FALSE);
  	do_drop(ch, arg, cmd);
  	value = 0;
	trashquest = 0;

  	for( k = world[ch->in_room].contents; k ; k = world[ch->in_room].contents ) 
  	{
		if( k->type == ITEM_TRASH) trashquest ++ ;
    		sprintf(buf, "The %s vanish in a puff of smoke.\n\r",oneword(k->name));
    	for( tmp_char = world[ch->in_room].people; tmp_char; tmp_char = tmp_char->next_in_room)
      	if( can_see_obj(tmp_char, k) )
        	send_to_char(buf,tmp_char);
      	value += (k->cost/2 - 10) ;
    	extract_obj( k, 1 );
  	}
  	if (value > 0) 
  	{
    	acthan("You are awarded for outstanding performance.",
       			"당신은 시장으로부터 선행상을 받았습니다.",  FALSE, ch, 0, 0, TO_CHAR);
    	acthan("$n has been awarded for being a good citizen.",
       			"$n님이 착한 시민상을 받았습니다", TRUE, ch, 0,0, TO_ROOM);

    	if ( value > 1000 ) 
		{
			x = value -1000 ;
			value = 1000 ;
			while ( x > 2 ) 
			{
				value += 100 ;
				x = x / 2 ;
			}
		}
    	if( GET_LEVEL(ch) < 3 && (GET_EXP(ch) < GET_LEVEL(ch)*2000) )
      		gain_exp(ch, value);
    	else
      		GET_GOLD(ch) += value;
  	}

	if( trashquest && !IS_NPC(ch) && !is_solved_quest(ch, QUEST_TRASH)) 
	{
		log( "%s solved TRASH QUEST", GET_NAME(ch)) ;

		send_to_char_han("You knew that ! You solved TRASH Quest\n\r",
						 "알고 있었군요 ! 당신은 쓰레기 문제를 풀었습니다.\n\r", ch) ;
		set_solved_quest(ch, QUEST_TRASH) ;

		if( ch->base_stat.str < 18 ) 
		{
			ch->base_stat.str ++ ;
			affect_total(ch) ;
			send_to_char_han("Your strength increased !\n\r","당신의 힘이 증가 하였습니다.\n\r", ch) ;
			save_char(ch, NOWHERE) ;
		}
		else
		{
			send_to_char_han( "But, You have full of strength.\n\r", 
							  "아..당신은 힘이 꽉차있군요.\n\r", ch );
		}
	}
	return TRUE;
}

#define INC18(X) X=((X < 18) ? X+1 : X)
int metahospital(charType *ch, int cmd, char *arg)
{
  	char 	buf[MAX_STRING_LENGTH];
  	int 	k, opt, cost = 0, tmp = 0;

  	if( IS_NPC(ch) ) return(FALSE);
  	if( cmd != COM_LIST &&  cmd != COM_BUY ) return 0;

  	if( cmd == COM_LIST )
  	{
		send_to_char("1 - Hit points inflator         (  1M exp )\n\r",ch);
		send_to_char("2 - Mana increase               (  1M exp )\n\r",ch);
		send_to_char("3 - Movement Incrementor        (  1M exp )\n\r",ch);
		send_to_char("4 - Freedom from hunger         (  1M exp )\n\r",ch);
		send_to_char("5 - Freedom from thirst         (  1M exp )\n\r",ch);
		send_to_char("6 - Increase coins(200K-2M)     (  1M exp )\n\r",ch);
		send_to_char("7 - Increase practice number    (  1M exp )\n\r",ch);
		return(TRUE);
  	} 

	arg = oneArgument(arg, buf);
	opt = atoi(buf);

	cost = 1000000;

	if( cost > (GET_EXP(ch) - GET_LEVEL(ch) * 150000) )
	{
		send_to_char("Come back when you are more experienced.\n\r",ch);
		return(TRUE);
	}

  	if((opt >= 1) && (opt < 8))
   	{
      	switch(opt)
	  	{
        case 1: 
			k = ch->max_hit;
            tmp =(3*(k<200))+2*(k<300)+(k<400)+(k<500)+(k<600)+(k<800)+(k<1000);

			switch (GET_CLASS(ch)) 
			{
   		 		case CLASS_MAGIC_USER : tmp +=( k<1300 );break;
		 		case CLASS_CLERIC : 	tmp +=( k<1550 );break;
		 		case CLASS_THIEF : 		tmp +=( k<1400 );break;
		 		case CLASS_WARRIOR :    tmp +=( k<1700 );break; 
			}
			if( tmp )
			{
      			ch->exp-=cost;
				ch->max_hit += tmp;
				send_to_char("@>->->--\n\r",ch);
			}
			break;
		case 2: 
			k = ch->max_mana; 
			tmp += (k<150)+(k<200)+(k<300)+(k<500)+(k<700); 

			switch (GET_CLASS(ch)) 
			{
   		 		case CLASS_MAGIC_USER : tmp +=((k<1300));break;
		 		case CLASS_CLERIC : 	tmp +=((k<1200));break;
		 		case CLASS_THIEF : 		tmp +=((k<1000));break;
		 		case CLASS_WARRIOR : 	tmp +=((k<1100));break; 
			}
			if( tmp )
			{
      			ch->exp-=cost;
				ch->max_mana += tmp;
                send_to_char("<,,,,,,,>\n\r",ch);
			}
			break;
        case 3: 
			k = ch->max_move;
            tmp += (k<101)+(k<151) +(k<200)+(k<300);

			switch (GET_CLASS(ch)) 
			{
   		 		case CLASS_MAGIC_USER :  tmp +=((k<750)+(k<1200));break;
		 		case CLASS_CLERIC : 	 tmp +=((k<450)+(k<1100));break;
		 		case CLASS_THIEF : 		 tmp +=((k<700)+(k<1300));break;
		 		case CLASS_WARRIOR : 	 tmp +=((k<600)+(k<1000));break; 
			}
			if( tmp )
			{
      			ch->exp-=cost;
				ch->max_move += tmp;
                send_to_char("<:::::::>\n\r",ch); break;
			}
			break;

        case 4: ch->conditions[1]=(-1);
                send_to_char("You will never again be hungry.\n\r",ch);
      			ch->exp-=cost;
				break;
        case 5: ch->conditions[2]=(-1);
      			ch->exp-=cost;
                send_to_char("You will never again be thirsty.\n\r",ch);
				break;
        case 6: GET_GOLD(ch) += ( 200000 + MAX((10 - (ch->level - 25)), 0) * 60000 );
      			ch->exp-=cost;
                send_to_char("$$$$$$$$\n\r",ch); break;
        case 7: ch->prNr++;
      			ch->exp-=cost;
                send_to_char("Aaaaaaarrrrrrrrggggg\n\r",ch); 
				break;
		default : sendf( ch, "Buy what??" );
      	}

		affect_total( ch );
	}
    return(TRUE);
}

int portal(charType *ch, int cmd, char *arg)
{
  	int 	location,ok;

  	if( cmd != COM_SOUTH ) return(FALSE);

  	location = number(1,rooms_in_world); 	
  	ok=TRUE;

  	if( IS_SET(world[location].flags,OFF_LIMITS) )  ok=FALSE;
  	else if (IS_SET(world[location].flags,PRIVATE)) ok=FALSE;

  	if( !ok )
  	{
    	sendf( ch, "You bump into something, and go nowhere." );
    	act("$n seems to bump into nothing??",FALSE,ch,0,0,TO_NOTVICT);
  	} 
  	else 
  	{
    	act("$n님이 없어진것 같네요 ??",FALSE,ch,0,0,TO_NOTVICT);
    	sendf( ch, "You are momentarily disoriented." );
    	char_from_room(ch);
    	char_to_room(ch,location);
    	do_look(ch,"",15);
  	}
  	return(TRUE);
}

int safe_house(charType *ch, int cmd, char *arg)
{
  	if( GET_LEVEL(ch) >= (IMO+2) ) return FALSE;

  	switch( cmd ) 
  	{
    	case COM_KILL: 	 /* kill */
    	case COM_HIT:	 /* hit */
    	case COM_ORDER:	 /* order */
    	case COM_BACKSTAB:/* backstab */
    	case COM_STEAL:	 /* steal */
    	case COM_BASH:	 /* bash */
    	case COM_RESCUE: /* kick */
    	case COM_USE:	 /* use */
    	case COM_SHOOT:	 /* shoot */
    	case COM_TORNADO:/* tornado */
    	case COM_FLASH:	 /* flash */
		case COM_THROW:	 /* throw */
		case COM_DAZZLE:
		case COM_SERPENT:
		case COM_AMBUSH:
		case COM_PUNCH:
		case COM_KICK:
			misbehave( ch );
      		return TRUE;
    	default:
      		return FALSE;
  	}
}

int death_house(charType *ch, int cmd, char *arg)
{
  	if( GET_LEVEL(ch) >= (IMO+2) ) return FALSE;

  	switch( cmd ) 
  	{
    	case COM_KILL:
    	case COM_HIT:
    	case COM_ORDER:
    	case COM_BACKSTAB:
    	case COM_STEAL:
    	case COM_BASH:
    	case COM_RESCUE:
    	case COM_USE:
		case COM_QUAFF:
    	case COM_SHOOT:	
    	case COM_TORNADO:
    	case COM_FLASH:
		case COM_THROW:
		case COM_RECITE:
		case COM_CAST: 	 
		case COM_DAZZLE:
		case COM_AMBUSH:
		case COM_SERPENT:
		case COM_PUNCH:
		case COM_KICK:
			misbehave( ch );
      		return TRUE;
    	default:
      		return FALSE;
  	}
}
