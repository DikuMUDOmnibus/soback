/* ************************************************************************
*  file: act.informative.c , Implementation of commands.  Part of DIKUMUD *
*  Usage : Informative commands.                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "defines.h"
#include "character.h"
#include "object.h"
#include "world.h"
#include "mobile.h"
#include "sockets.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "limits.h"
#include "variables.h"
#include "misc.h"
#include "strings.h"
#include "weather.h"
#include "page.h"

void do_report(charType *ch, char *argument, int cmd)
{
  	char 	buf[80];

  	sprintf(buf, "%s %d/%d hp, %d/%d mn, %d/%d mv", 
  			GET_NAME(ch),
   			GET_HIT(ch),GET_MAX_HIT(ch),
   			GET_MANA(ch),GET_MAX_MANA(ch),
   			GET_MOVE(ch),GET_MAX_MOVE(ch) );
  	act(buf,FALSE,ch,0,0,TO_ROOM);
  	send_to_char("ok.\n\r",ch);
}

void do_title(charType *ch, char *argument, int cmd)
{
  	char buf[100];

	if( IS_NPC(ch) ) return;

    argument = skipsps( argument );

    if( *argument == 0 )
    {
        sendf( ch, "You are %s %s", GET_NAME(ch), GET_TITLE(ch));
        return;
    }

    strcpy( buf, argument );

    if( buf[0] == '.' && buf[1] == 0 ) buf[0] = ' ';

    if( GET_TITLE(ch) )
    {
        errFree(GET_TITLE(ch)) ;
        GET_TITLE(ch) = errMalloc(strlen(buf) + 1) ;
    }
    else
    {
        GET_TITLE(ch) = errMalloc(strlen(buf) + 1) ;
    }

    if( GET_TITLE(ch) ) strcpy(GET_TITLE(ch), buf );
}

static char align_msg[13][44] = {
  "You are a saint.\n\r",
  "You feel like being a saint.\n\r",
  "You are good.\n\r",
  "You are slightly good.\n\r",
  "You are almost good.\n\r",
  "You are going to be good.\n\r",
  "You are neutral.\n\r",
  "You are going to be evil.\n\r",
  "You are almost evil.\n\r",
  "You are slightly evil.\n\r",
  "You are evil.\n\r",
  "You feel like being a devil.\n\r",
  "You are a devil.\n\r"
};

static char align_msg_han[13][44] = {
  "당신은 성인 군자십니다.\n\r",
  "당신은 성이 군자가 되어가고 있습니다.\n\r",
  "당신은 선하십니다.\n\r",
  "당신은 약간 선합니다.\n\r",
  "당신은 거의 선한 경지에 이르렀습니다.\n\r",
  "당신은 선한쪽으로 변하고 있습니다.\n\r",
  "당신은 아주 평범한 성향입니다.\n\r",
  "당신은 악한 쪽으로 변해가고 있습니다.\n\r",
  "당신은 거의 악해졌습니다.\n\r",
  "당신은 약간 악한 성향을 띄고 있습니다.\n\r",
  "당신은 악하십니다.\n\r",
  "당신은 거의 악마가 되어갑니다.\n\r",
  "당신은 악마가 되셨습니다.\n\r"
};

void do_score(charType *ch, char *argument, int cmd)
{
  affectType *aff;
  struct time_info_data playing_time;
  static char buf[200], buf2[200], buf3[200], buf4[200];
  struct time_info_data real_time_passed(time_t t2, time_t t1);
  int tmp;

	if( IS_NPC(ch) ) return;
  sprintf(buf, "You are %d years old.\n\r", GET_AGE(ch) );
  sprintf(buf2, "당신은 %d 살 입니다.\n\r", GET_AGE(ch) );
  send_to_char_han(buf,buf2,ch);

  /* alignment message */

  tmp = ch->align/50;

  if     ( tmp > 17 )	send_to_char_han( align_msg[0], align_msg_han[0], ch );
  else if( tmp > 12 ) 	send_to_char_han( align_msg[1], align_msg_han[1], ch );
  else if( tmp > 7  ) 	send_to_char_han( align_msg[2], align_msg_han[2], ch );
  else if( tmp > 6  ) 	send_to_char_han( align_msg[3], align_msg_han[3], ch );
  else if( tmp > 5  ) 	send_to_char_han( align_msg[4], align_msg_han[4], ch );
  else if( tmp > 2  ) 	send_to_char_han( align_msg[5], align_msg_han[5], ch );
  else if( tmp > -3  ) 	send_to_char_han( align_msg[6], align_msg_han[6], ch );
  else if( tmp > -6  ) 	send_to_char_han( align_msg[7], align_msg_han[7], ch );
  else if( tmp > -7  ) 	send_to_char_han( align_msg[8], align_msg_han[8], ch );
  else if( tmp > -8  ) 	send_to_char_han( align_msg[9], align_msg_han[9], ch );
  else if( tmp > -13 ) 	send_to_char_han( align_msg[10], align_msg_han[10], ch );
  else if( tmp > -18 )  send_to_char_han( align_msg[11], align_msg_han[11], ch );
  else 					send_to_char_han( align_msg[12], align_msg_han[12], ch );

  sprintf(buf,"Your ac is %d.\n\r",ch->armor);
  sprintf(buf2,"당신의 무장정도는 %d 입니다.\n\r",ch->armor);
  send_to_char_han(buf,buf2,ch);

  if (GET_COND(ch,DRUNK)>10)
   send_to_char_han("You are intoxicated.\n\r","당신은 취해 있습니다.\n\r",ch);
  sprintf(buf, 
    "You have %d(%d) hit, %d(%d) mana and %d(%d) movement \n\r",
    GET_HIT(ch),GET_MAX_HIT(ch),
    GET_MANA(ch),GET_MAX_MANA(ch),
    GET_MOVE(ch),GET_MAX_MOVE(ch));
  sprintf(buf2, 
    "당신은 %d(%d) hit, %d(%d) mana 와 %d(%d) movement 를 가지고 있습니다.\n\r",
    GET_HIT(ch),GET_MAX_HIT(ch),
    GET_MANA(ch),GET_MAX_MANA(ch),
    GET_MOVE(ch),GET_MAX_MOVE(ch));
  send_to_char_han(buf,buf2,ch);

  if (GET_LEVEL(ch) > 12) {
    sprintf(buf,"Your stat: str %d/%d wis %d int %d dex %d con %d.\n\r",
      GET_STR(ch),GET_ADD(ch),GET_WIS(ch),GET_INT(ch),GET_DEX(ch),GET_CON(ch));
    sprintf(buf2,"당신의 체질: str %d/%d wis %d int %d dex %d con %d.\n\r",
      GET_STR(ch),GET_ADD(ch),GET_WIS(ch),GET_INT(ch),GET_DEX(ch),GET_CON(ch));
    send_to_char_han(buf, buf2, ch);

    sprintf(buf,"Your hitroll is %d , and damroll is %d.\n\r",
      GET_HITROLL(ch),GET_DAMROLL(ch));
    sprintf(buf2,"당신의 hitroll 은 %d , damroll 은 %d.\n\r",
      GET_HITROLL(ch),GET_DAMROLL(ch));
    send_to_char_han(buf, buf2, ch); /* changed by shin won dong */
    }

	strcpy( buf3, numfstr( ch->gold ) );
	strcpy( buf4, numfstr( ch->exp ) );
  sprintf(buf,"You have scored %s exp, and have %s gold coins.\n\r", buf4, buf3 );
  sprintf(buf2,"당신은 %s 의 경험치와 %s 원의 돈을 가지고 있습니다.\n\r", buf4, buf3 );
  send_to_char_han(buf, buf2, ch);

  playing_time = real_time_passed((time(0)-ch->logon) + ch->played, 0);

  sprintf(buf,"You have been playing for %d days and %d hours.\n\r",
    playing_time.day, playing_time.hours);    
  sprintf(buf2,"당신은 이 안에서 %d 일 %d 시간동안 있었습니다.\n\r",
    playing_time.day, playing_time.hours);    
  send_to_char_han(buf, buf2, ch);    
  sprintf(buf,"This ranks you as %s (level %d).\n\r",
    GET_NAME(ch), GET_LEVEL(ch) );
  sprintf(buf2, "당신은 %s (레벨 %d) 입니다.\n\r",
    GET_NAME(ch), GET_LEVEL(ch) );
  send_to_char_han(buf, buf2, ch);
  sendf( ch, "Your title is %s", GET_TITLE(ch) );
  if(GET_LEVEL(ch)<IMO){
    if(titles[GET_CLASS(ch)-1][GET_LEVEL(ch)+1].exp<GET_EXP(ch)) {
      strcpy(buf,"You have enough experience to advance.\n\r");
      strcpy(buf2,"레벨을 올릴만큼 충분한 경험치가 쌓였습니다.\n\r");
      }
    else {
      sprintf(buf,"You need %s experience to advance\n\r",
       numfstr( titles[GET_CLASS(ch)-1][GET_LEVEL(ch)+1].exp-GET_EXP(ch)) );
      sprintf(buf2,"다음 레벨까지 %s 만큼의 경험치가 필요합니다.\n\r",
       numfstr( titles[GET_CLASS(ch)-1][GET_LEVEL(ch)+1].exp-GET_EXP(ch)) );
       }
    send_to_char_han(buf, buf2, ch);
  }

  switch(GET_POS(ch)) {
    case POSITION_DEAD : 
      send_to_char_han("You are DEAD!\n\r", "당신은 죽으셨습니다\n\r", ch);
      break;
    case POSITION_MORTALLYW :
      send_to_char("You are mortally wounded!, you should seek help!\n\r", ch); break;
    case POSITION_INCAP : 
      send_to_char("You are incapacitated, slowly fading away\n\r", ch); break;
    case POSITION_STUNNED : 
      send_to_char("You are stunned! You can't move\n\r", ch); break;
    case POSITION_SLEEPING : 
      send_to_char_han("You are sleeping.\n\r", "당신은 자고 있습니다.\n\r",
       ch);
      break;
    case POSITION_RESTING  : 
      send_to_char_han("You are resting.\n\r", "당신은 쉬고 있습니다.\n\r",ch);
      break;
    case POSITION_SITTING  : 
      send_to_char_han("You are sitting.\n\r","당신은 앉아 있습니다.\n\r",ch);
      break;
    case POSITION_FIGHTING :
      if( ch->fight )
        acthan("You are fighting $N.\n\r", "당신은 $N님과 싸우고 있습니다.\n\r",
          FALSE, ch, 0, ch->fight, TO_CHAR);
      else
        send_to_char("You are fighting thin air.\n\r", ch);
      break;
    case POSITION_STANDING : 
      send_to_char_han("You are standing.\n\r","당신은 서있습니다.\n\r", ch);
      break;
    default :
      send_to_char("You are floating.\n\r",ch); break;
  }

  if (ch->affected) {
    send_to_char_han("Affecting Spells:\n\r", "걸려있는 마법들:\n\r", ch);
    for(aff = ch->affected; aff; aff = aff->next) {
      sprintf(buf,"%s: %d hrs\n\r",spells[aff->type].name,aff->duration);
      sprintf(buf2,"%s: %d 시간\n\r",spells[aff->type].name,aff->duration);
      send_to_char_han( buf, buf2, ch);
    }
  }
}

void do_attribute(charType *ch, char *argument, int cmd)
{
  	affectType 	*	aff;
  	struct time_info_data	played;
  
	if( IS_NPC(ch) ) return;

  	played = real_time_passed((time(0)-ch->logon) + ch->played, 0);

  	sendf( ch, "You are %d years and %d months. You have played %dd:%dhrs.", 
  				GET_AGE(ch), age(ch).month, played.day, played.hours );

  	if( (GET_LEVEL(ch) > 15) ) 
  	{
  		if((GET_LEVEL(ch) > 15) ) 
  		{
      		sendf( ch, "You have %d/%d str, %d int, %d wis, %d dex, %d con, %d regen.",
	   				GET_STR(ch), GET_ADD(ch), GET_INT(ch), GET_WIS(ch), 
					GET_DEX(ch), GET_CON(ch), GET_REGEN(ch) );
  		}  
  	}
	sendf( ch, "Your armor is %d. And your alignment is %d.", ch->armor, ch->align );
  	sendf( ch, "Your hitroll and damroll are %d and %d.",GET_HITROLL(ch), GET_DAMROLL(ch));
  	sendf( ch, "Your saving_spell is [%d], and saving_breath is [%d].",
  				ch->saving_throw[4],
  				ch->saving_throw[3] );

  	if( ch->affected ) 
  	{
    	sendf( ch, "\n\rAffected spells:\n\r--------------");
    	for( aff = ch->affected; aff; aff = aff->next ) 
		{
      		switch(aff->type) 
	  		{
   			case SKILL_SNEAK:
   			case SPELL_POISON:
			case SPELL_BLINDNESS:
  			case SPELL_CRUSH_ARMOR:
   			case SPELL_CURSE:	break;
   			default:			sendf( ch, "%s : %d hrs", spells[aff->type].name, aff->duration );
      		}
    	}
  	}
}

/* end of Attribute Module... */

void do_time(charType *ch, char *argument, int cmd)
{
  	char 		buf[100], * suf;
  	int 		weekday, 	day;

  	sprintf(buf, "It is %d o'clock %s, on ",
    			((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
    			((time_info.hours >= 12) ? "pm" : "am") );

  	weekday = ((35*time_info.month)+time_info.day+1) % 7;/* 35 days in a month */

  	strcat( buf, weekdays[weekday] );
  	sendf( ch, "%s", buf );

  	day = time_info.day + 1;   /* day in [1..35] */

  	if      (day == 1) 			suf = "st";
  	else if (day == 2) 			suf = "nd";
  	else if (day == 3) 			suf = "rd";
  	else if (day < 20) 			suf = "th";
  	else if ((day % 10) == 1) 	suf = "st";
  	else if ((day % 10) == 2) 	suf = "nd";
  	else if ((day % 10) == 3) 	suf = "rd";
  	else 						suf = "th";

  	sendf( ch, "The %d%s Day of the %s, Year %d.",
    				day,
    				suf,
    				month_name[(int)time_info.month], time_info.year);
}

void do_date(charType *ch, char *argument, int cmd)
{  
    long    		ct;
    char    	*	tmstr ;
    char    		buf[BUFSIZ] ;

    ct = time(0);
    tmstr = asctime(localtime(&ct));

    sprintf(buf, "Real life time : %s\r", tmstr) ;
    send_to_char(buf, ch) ;
}

void do_weather(charType *ch, char *argument, int cmd)
{
  	static char 		buf[100], buf2[100];
  	static char 	*	sky_look[4]= 
  	{
     	"cloudless",
     	"cloudy",
     	"rainy",
     	"lit by flashes of lightning"
    };

  	static char 	*	sky_look_han[4]= 
  	{
     	"하늘이 구름 한점 없이 맑습니다.",
     	"하늘에 구름이 조금 있습니다.",
     	"비가 오고 있습니다.",
     	"천둥 번개가 칩니다."
    };

  	if( OUTSIDE(ch) ) 
  	{
    	sprintf( buf, "The sky is %s and %s.\n\r", sky_look[weather_info.sky],
      					(weather_info.change >=0 ? "you feel a warm wind from south" :
       					"your foot tells you bad weather is due"));
    	sprintf( buf2, "%s %s.\n\r", sky_look_han[weather_info.sky],
      					(weather_info.change >=0 ? "서편하늘부터 구름이 개이고 있습니다" :
       					"더 궂은 날이 되것 같습니다"));
    	send_to_char_han(buf, buf2, ch);
  	} 
  	else
   		 send_to_char_han("You have no feeling about the weather at all.\n\r",
      						"여기선 날씨가 어떤지 알 수 없습니다.\n\r", ch);
}

void do_spells(charType *ch, char *argument, int cmd)
{
  	char 	buf[80*MAX_SKILLS], tmp[MAX_STRING_LENGTH];
  	char 	name[100];
  	int 	class;
  	int 	i, count;

  	if( IS_NPC(ch) ) return;

  	oneArgument(argument, name);

  	if( !*name ) 
  	{
    	class = GET_CLASS(ch);
    }
  	else 
  	{
    	switch( *name ) 
		{
     		case 'm':
     		case 'M': class = CLASS_MAGIC_USER ; break ;      /* 1 */
     		case 'c':
      		case 'C': class = CLASS_CLERIC ; break ;          /* 2 */
      		case 't':
     		case 'T': class = CLASS_THIEF ; break ;           /* 3 */
      		case 'w':
      		case 'W': class = CLASS_WARRIOR ; break ;           /* 4 */
      		default:
       			send_to_char("Usage: spells {CMTW}\n\r", ch) ;
       			return ;
      	}
   	}

  	strcpy( buf,"    SPELL NAME       LEVEL/PRACTICE/MANA-MINIMUM\n\r" ) ;

  	for( i = 1, count = 0; *spells[i].name != '\n'; i++ )
  	{
    	if( *spells[i].name == '\0' ) continue;

    	if( spells[i].min_level[class-1] > 40 ) continue;

    	sprintf(tmp,"%-20s%4d%8d%6d%s",
      		spells[i].name, spells[i].min_level[class-1],
      		spells[i].max_skill[class-1],
      		spells[i].min_mana, (count%2==0)? " | " : "\n\r" );
    	strcat( buf, tmp );
    	count++ ;
    }
  	strcat(buf, "\n\r");
  	print_page( ch, buf );
}

void do_levels(charType *ch, char *argument, int cmd)
{
	pageUnitType	* 	pu;
  	int 	i, class;
  	char	name[MAX_INPUT_LENGTH];
	char	buf[128], expf[128], expt[128];

  	if( IS_NPC(ch) ) return;

  	oneArgument( argument, name );	

  	if( !*name ) 
  	{
    	class = GET_CLASS(ch);
    }
  	else 
  	{
    	switch( *name ) 
		{
     		case 'm':
     		case 'M': class = CLASS_MAGIC_USER ; break ;      /* 1 */
     		case 'c':
      		case 'C': class = CLASS_CLERIC ; break ;          /* 2 */
      		case 't':
     		case 'T': class = CLASS_THIEF ; break ;           /* 3 */
      		case 'w':
      		case 'W': class = CLASS_WARRIOR ; break ;           /* 4 */
      		default:
       			sendf( ch, "Usage: levels {CMTW}\n\r" ) ;
       			return ;
      	}
   	}

	pu = new_pageunit( ch );

  	for( i = 1; i < IMO; i++ ) 
  	{
       	strcpy( expf, numfstr( titles[ class -1 ][i].exp ));
       	strcpy( expt, numfstr( titles[ class -1 ][i+1].exp ));
 
       	sprintf(buf, "%2d: %11s to %11s: ", i, expf, expt );
 
    	switch(GET_SEX(ch)) 
		{
      		case SEX_MALE:
        		strcat(buf, titles[class - 1][i].title_m); break;
      		case SEX_FEMALE:
        		strcat(buf, titles[class - 1][i].title_f); break;
      		default:
	  			DEBUG( "do_level> Unkown sex!" );
    	}
		pagef( pu, "%s", buf );
  	}
  	show_page( ch, pu );
}

void do_consider(charType *ch, char *argument, int cmd)
{
  	charType 		*	victim;
  	char 				name[256];
  	int 				diff;

  	oneArgument( argument, name );

  	if( !(victim = find_char_room(ch, name)) ) 
  	{
    	sendf( ch, "Consider killing who?" );
    	return;
  	}

  	if( victim == ch ) 
  	{
    	sendf( ch, "Easy! Very easy indeed!" );
    	return;
  	}

  	if( !IS_NPC(victim) ) 
  	{
    	sendf( ch, "Would you like to borrow a cross and a shovel?" );
    	return;
  	}

  	diff = (GET_LEVEL(victim)-GET_LEVEL(ch));

  	if      (diff <= -10) 	sendf( ch, "Now where did that chicken go?" );
  	else if (diff <= -5) 	sendf( ch, "You could do it with a needle!" );
  	else if (diff <= -2) 	sendf( ch, "Easy." );
  	else if (diff <= -1) 	sendf( ch, "Fairly easy." );
  	else if (diff == 0) 	sendf( ch, "The perfect match!" );
  	else if (diff <= 1) 	sendf( ch, "You would need some luck!" );
  	else if (diff <= 2) 	sendf( ch, "You would need a lot of luck!" );
  	else if (diff <= 3) 	sendf( ch, "You would need a lot of luck and great equipment!" );
  	else if (diff <= 5) 	sendf( ch, "Do you feel lucky, punk?" );
  	else if (diff <= 10) 	sendf( ch, "Are you mad!?" );
  	else if (diff <= 100) 	sendf( ch, "You ARE mad!" );
}

void do_wizlock(charType *ch, char *argument, int cmd)
{
  	char 			buf[200];
  	int 			i;

  	buf[0] = 0;

  	oneArgument( argument, buf );

  	if( *argument )
  	{
    	for( i = 0; i < baddoms; i++ )
      		if( strcmp(baddomain[i],buf) == 0 ) break;

    	if( i < baddoms )
		{
      		strcpy( baddomain[i], baddomain[--baddoms] );
    	} 
		else 
		{
      		if( baddoms < BADDOMS )
        	strcpy( baddomain[baddoms++], buf );
    	}
  	} 
  	else 
  	{
     	for( i = 0; i < baddoms; ++i )
	 	{
        	sendf( ch, "%2d] %s", i + 1, baddomain[i] );
     	}
  	}
}

void do_nochat(charType *ch, char *argument, int cmd)
{
  	if( IS_NPC(ch) ) return;

  	if( IS_SET(ch->act, PLR_NOCHAT) ) 
  	{
    	sendf( ch, "You can now hear chats again." );
    	REMOVE_BIT(ch->act, PLR_NOCHAT);
  	} 
  	else 
  	{
    	sendf( ch, "From now on, you won't hear chats." );
    	SET_BIT(ch->act, PLR_NOCHAT);
  	}
}

void do_notell(charType *ch, char *argument, int cmd)
{
  	if( IS_NPC(ch) ) return;

  	if( IS_SET(ch->act, PLR_NOTELL) ) 
  	{
    	sendf( ch, "You can now hear tells again." );
    	REMOVE_BIT(ch->act, PLR_NOTELL);
  	} 
  	else 
  	{
    	sendf( ch, "From now on, you won't hear tells." );
    	SET_BIT(ch->act, PLR_NOTELL);
  	}
}

void do_data(charType *ch, char *argument, int cmd)
{
  	descriptorType 		*	d;
  	charType 			*	victim;
  	char 					buf[256],name[256],fmt[16];
  	int 					i = 0, k, n = 0, t = 0, nc;

  	oneArgument(argument, name);

  	if     (strcmp("exp",   name)==0) k=1;
  	else if(strcmp("hit",   name)==0) k=2;
  	else if(strcmp("gold",  name)==0) k=3;
  	else if(strcmp("armor", name)==0) k=4;
  	else if(strcmp("age",   name)==0){ k=5; t=time(0); }
  	else if(strcmp("com",   name)==0){ k=6; t=time(0); }
  	else if(strcmp("flags", name)==0) k=7;
  	else if(strcmp("bank",  name)==0) k=8;
  	else if(strcmp("des",   name)==0) k=9;
  	else if(strcmp("level", name)==0) k=10;
  	else 
  	{
		sendf( ch, "Usage: data mode" ) ;
		sendf( ch, " mode: exp hit gold armor age com flags bank des level" );
		return ;
	}

  	if( (k==1) || (k==3) || (k==8) )
  	{
    	nc = 3;	strcpy( fmt, "%-15s%10d%s" );
  	} 
  	else 
  	{
    	nc = 4;	strcpy( fmt, "%-15s%4d%s" );
  	}

  	for( buf[0] = 0, d = desc_list; d; d = d->next ) 
  	{
    	if( !d->connected && can_see(ch, d->character) ) 
		{
      		++i;
      		if( d->original ) 	victim = d->original;
      		else 				victim = d->character;
      		switch( k )
	  		{
        		case  1: n = victim->exp; 											break;
        		case  2: n = victim->max_hit; 										break;
        		case  3: n = victim->gold; 											break;
        		case  4: n = victim->armor; 										break;
        		case  5: n = (t-victim->birth)/86400; 								break;
        		case  6: n = (60*victim->desc->ncmds)/(1+t-victim->desc->contime); 	break;
        		case  7: n = victim->act; 											break;
        		case  8: n = victim->bank; 											break;
        		case  9: n = d->fd; 												break;
        		case 10: n = victim->level;											break;
      		}
      		sprintf( buf + strlen(buf), fmt, GET_NAME(victim), n, (i%nc) ? "|" : "" );
	  		if( !(i % nc) ) 
			{
				sendf( ch, "%s", buf ); buf[0] = 0;
			}
    	}
  	}
  	if( i % nc ) sendf( ch, "" );
}

void do_hangul(charType *ch, char *argument, int cmd)
{
  	if( IS_NPC(ch) ) return;

  	if( IS_SET(ch->act, PLR_KOREAN) ) 
  	{
    	sendf( ch, "English command message mode." );
    	REMOVE_BIT(ch->act, PLR_KOREAN);
  	} 
  	else 
  	{
    	send_to_char("You can now see hangul command messages.\n\r", ch);
    	SET_BIT(ch->act, PLR_KOREAN);
  	}
}

void do_commands( charType * ch, char * argument, int cmd )
{   
	int				i, j, level, count = 0;
	char			arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	char			buf[MAX_STRING_LENGTH];
	char			start, end;

	if( IS_NPC(ch) ) return;

    level = GET_LEVEL(ch);

	if( !*argument )
	{
		sendf( ch, "commands usage> [from [to]]  ex> commands a d" );
		sendf( ch, "commands usage> [prefix]     ex> commands a, commands as" );
		return;
	}

	halfchop( argument, arg1, arg2 );

	start = *arg1, end = *arg2;
	start = tolower( start ) - 'a';

	if( end )
	{
		end   = tolower( end   ) - 'a';
		
		if(  (start < 0 || start > ('z' - 'a')) || (end < 0 || end > ('z' - 'a')) )
		{
			sendf( ch, "It is out of range." );
			return;
		}

		for( i = start; i <= end; i++ )
		{
			sendf( ch, "%c commands", (char)(i + 'A') );
			for( j = 0, buf[0] = 0, count = 1; commands[i][j].name[1]; j++ )
			{
				if( commands[i][j].level <= level )
				{
					sprintf( buf + strlen(buf), "%c %-17s%c", 
						( count % 4 == 1 ?  ' ' : '|' ), 
						commands[i][j].name,
						( count % 4 == 0 ? '\n' : ' ' ) );
					count++;
				}
			}
			if( count > 1 ) sendf( ch, "%s", buf );
		}
	}
	else
	{
		if(  (start < 0 || start > ('z' - 'a')) )
		{
			sendf( ch, "It is out of range." );
			return;
		}

		for( j = 0, buf[0] = 0, count = 1, i = start; commands[i][j].name[1]; j++ )
		{
			if( commands[i][j].level <= level && isprefix( arg1, commands[i][j].name ) )
			{
				sprintf( buf + strlen(buf), "%c %-17s%c", 
					( count % 4 == 1 ?  ' ' : '|' ), 
					commands[i][j].name,
					( count % 4 == 0 ? '\n' : ' ' ) );
				count++;
			}
		}
		if( count > 1 ) sendf( ch, "%s", buf );
	}
	if( count > 1 ) sendf( ch, "\t * Upper-left command has the highest priority. *" );
	else			sendf( ch, "Nothing." );
}
