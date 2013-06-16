#include <string.h>
#include <stdio.h>
#include <assert.h>
 
#include "defines.h" 
#include "character.h"
#include "mobile.h"
#include "world.h"
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
#include "weather.h"
#include "affect.h"
#include "transfer.h"
#include "misc.h"


void spell_identify( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  char buf[256], buf2[256], bufh[256];
  int i;
  bool found;

  struct time_info_data age(charType *ch);

  if (obj) 
  {
    send_to_char_han("You feel informed:\n\r",
        "이런 정보를 알 수 있습니다:\n\r", ch);

    sprinttype( obj->type, item_types, buf2 );

    sprintf(buf, "Object '%s', Item type: %s, Item Status: %s\n\r", 
				obj->name, buf2, itemstatus( obj ) );
    sprintf(bufh, "물건 '%s', 종류: %s, 상태: %s\n\r", 
				obj->name, buf2, itemstatus( obj ) );
    send_to_char_han(buf, bufh, ch);

    if (obj->bitvector) {
      send_to_char_han("Item will give you following abilities:  ",
        "이 물건은 다음과 같은 능력을 줍니다:  ",  ch);
      sprintbit(obj->bitvector,affected_bits,buf);
	  sendf( ch, buf );
    }

    send_to_char_han("Item is: ", "성질: ", ch);
    sprintbit(obj->extra,extra_bits,buf);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);

    sprintf(buf,"Weight: %d, Value: %d, Level limit: %d\n\r",
      obj->weight, obj->cost, obj->level ? obj->level : 1 );
    sprintf(bufh,"무게: %d, 값: %d, 사용가능레벨: %d\n\r",
      obj->weight, obj->cost, obj->level ? obj->level : 1 );
    send_to_char_han(buf, bufh, ch);

    switch ( obj->type ) {

      case ITEM_SCROLL : 
      case ITEM_POTION :
        sprintf(buf, "Level %d spells of:\n\r",  obj->value[0]);
        sprintf(bufh, "%d 레벨에 해당하는 마법:\n\r",  obj->value[0]);
        send_to_char_han(buf, bufh, ch);
        if (obj->value[1] >= 1) {
			sendf( ch, "%s", spells[obj->value[1]].name );
        }
        if (obj->value[2] >= 1) {
			sendf( ch, "%s", spells[obj->value[2]].name );
        }
        if (obj->value[3] >= 1) {
			sendf( ch, "%s", spells[obj->value[3]].name );
        }
        break;

      case ITEM_WAND : 
      case ITEM_STAFF : 
        sprintf(buf, "Has %d chages, with %d charges left.\n\r",
          obj->value[1], obj->value[2]);
        send_to_char(buf, ch);

        sprintf(buf, "Level %d spell of:\n\r",  obj->value[0]);
        sprintf(bufh, "%d 레벨에 해당하는 마법:\n\r",  obj->value[0]);
        send_to_char_han(buf, bufh, ch);

        if (obj->value[3] >= 1) {
		  sendf( ch, spells[obj->value[3]].name  );
        }
        break;

      case ITEM_FIREWEAPON :
        sprintf(buf,  "Damage Dice is '1D%d' and has %d bullets out of max %d.\n\r", 
			obj->value[2], obj->value[0], obj->value[1] );
        sprintf(bufh, "Damage Dice 는 '1D%d' 이고 %d 발 탄창에 %d 발 남았습니다.\n\r", 
			obj->value[2], obj->value[1], obj->value[0] );
        send_to_char_han(buf, bufh, ch);
        break;

      case ITEM_WEAPON :
        sprintf(buf,  "Damage Dice is '%dD%d'\n\r", obj->value[1],obj->value[2]);
        sprintf(bufh, "Damage Dice 는 '%dD%d'\n\r", obj->value[1],obj->value[2]);
        send_to_char_han(buf, bufh, ch);
        break;

      case ITEM_ARMOR :
         if( is_apply_ac(obj) ) 
		 {
            sprintf(buf, "AC-apply is %d\n\r", obj->value[0]);
            sprintf(bufh, "무장: %d\n\r", obj->value[0]);
            send_to_char_han(buf, bufh, ch);
         }
    }

    found = FALSE;

    for (i=0;i<MAX_APPLY;i++) {
      if ((obj->apply[i].location != APPLY_NONE) &&
         (obj->apply[i].modifier != 0)) {
        if (!found) {
          send_to_char_han("Can affect you as :\n\r",
             "다음과 같은 작용을 합니다 :\n\r", ch);
          found = TRUE;
        }

        sprinttype(obj->apply[i].location,apply_types,buf2);
        sprintf(buf,"    Affects : %s By %d\n\r", buf2,
          obj->apply[i].modifier);
        sprintf(bufh,"    기능 : %d 만큼의 %s\n\r", obj->apply[i].modifier,
          buf2);
        send_to_char_han(buf, bufh, ch);
      }
    }

  } else {       /* victim */

    if (!IS_NPC(victim)) {
      sprintf(buf,"%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
              age(victim).year, age(victim).month,
              age(victim).day, age(victim).hours);
      sprintf(bufh,"나이 %d 년 %d 달 %d 일 %d 시간 입니다.\n\r",
              age(victim).year, age(victim).month,
              age(victim).day, age(victim).hours);
      send_to_char_han(buf, bufh, ch);

      sprintf(buf,"Height %dcm  Weight %dpounds \n\r",
        GET_HEIGHT(victim), GET_WEIGHT(victim));
      sprintf(bufh,"키 %dcm  몸무게 %d Kg\n\r",
        GET_HEIGHT(victim), GET_WEIGHT(victim));
      send_to_char_han(buf, bufh, ch);
      if(GET_LEVEL(victim) > 5)
      sprintf(buf,"Str %d/%d,  Int %d,  Wis %d,  Dex %d,  Con %d\n\r",
        GET_STR(victim), GET_ADD(victim),
        GET_INT(victim),
        GET_WIS(victim),
        GET_DEX(victim),
        GET_CON(victim) );
      send_to_char(buf,ch);
    } else {
      send_to_char_han("You learn nothing new.\n\r","별다른게 없네요.\n\r",ch);
    }
  }
}

void spell_clone( int level, charType *ch, char * arg, int type,
  									charType *victim, objectType *obj)
{     
 	charType 	*	mob;
        
 	if( obj )
 	{
   		send_to_char("Cloning object is not YET possible.\n\r",ch);
   		return;
 	}
 	if( !victim )
 	{
   		send_to_char("Clone who?\n\r",ch);
   		return;
 	}
 	if( dice( 5, 25 ) > ch->skills[SPELL_CLONE])
 	{
   		send_to_char("You fail, but not by much.\n\r",ch);
   		return;
 	}
 	if( IS_NPC(victim) && mobiles[victim->nr].in_world < (GET_LEVEL(ch)-GET_LEVEL(victim))/3 )
	{
   		mob				= load_a_mobile(victim->nr, REAL);
   		GET_EXP (mob)	= 0;
   		GET_GOLD(mob)	= 0;
   		char_to_room( mob, ch->in_room );
   		sendrf( 0, ch->in_room, "%s has been cloned!", victim->moved);
 	} 
 	else 
 	{
   		send_to_char("You may not clone THAT!\n\r",ch);
 	}
}

void spell_preach( int level, charType *ch, char * arg, int type,
  							charType *victim, objectType *obj)
{
  	if( !IS_NPC(victim) || GET_LEVEL(victim) >= IMO ) return;

  	if( level + number(1,15) < GET_LEVEL(victim) ) return;

  	if( IS_SET(victim->act,ACT_AGGRESSIVE) && number( 1, 20 ) > 5 )
  	{
    	REMOVE_BIT(victim->act,ACT_AGGRESSIVE);
    	act("$n looks less aggressive.",TRUE,victim,0,0,TO_ROOM);
  	} 
  	return;
}     

void spell_charm_person( int level, charType *ch, char * arg, int type, 
										charType *victim, objectType *obj)
{
	followType	*	fol;
  	affectType 		af;
  	int				i;

  	if( victim == ch ) 
  	{
    	sendf( ch, "You like yourself even better!" );
    	return;
  	}

  	if(  !IS_AFFECTED(victim, AFF_CHARM) 
	  && !IS_AFFECTED(ch, AFF_CHARM) 
      && ( level >= GET_LEVEL(victim) || IMPL(ch) ) ) 
  	{
      	for( i = 0, fol = ch->followers; fol; fol = fol->next )
      	{
        	if( IS_NPC( fol->follower ) ) i++;
      	}
   
      	if( i >= 8 )
      	{
          	sendf( ch, "You can't create more followers." );
          	return;
      	}

    	if( circle_follow( victim, ch ) || is_same_group( ch, victim ) ) 
		{
      		send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      		return;
    	}

    	if( (level < IMO) && ( IS_NPC(victim) && GET_LEVEL(victim) > 20) )
		{
      		send_to_char("You fail.\n\r",ch);
      		return;
    	}

   		if( IS_NPC( victim ) )
  		{
  			if( saves_spell( ch, victim, SAVING_PARA ) ) return;
		}
		else if( (number( 1, 5 ) < 5) || saves_spell( ch, victim, SAVING_SPELL ) ) return;

    	if( victim->master ) stop_follower(victim);
    	add_follower(victim, ch);

    	af.type      = SPELL_CHARM_PERSON;
    	af.duration  = 40*GET_INT(ch)/18;
    	af.modifier  = 0;
    	af.location  = 0;
    	af.bitvector = AFF_CHARM;
    	affect_to_char(victim, &af);
    	act( "Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT );
  	}
}

void spell_phase( int level, charType *ch, char *arg, int type,
  										charType *tar_ch, objectType *tar_obj )
{
  	int direction, was_in;

  	if( type != SPELL_SPELL ) 
  	{
    	DEBUG( "spell_phase> Attempt to phase by non-cast.");
    	return;
  	}

	arg = skipsps( arg );

  	switch(*arg)
  	{
    	case 'e' : case 'E': direction=1; break;
    	case 'w' : case 'W': direction=3; break;
    	case 'n' : case 'N': direction=0; break;
    	case 's' : case 'S': direction=2; break;
    	case 'd' : case 'D': direction=5; break;
    	case 'u' : case 'U': direction=4; break;
    	case NULL: send_to_char("Phase to where?\n\r",ch); return;
    	default  : send_to_char("ashgjklasdghiaudsgthl\n\r",ch);return;
  	}

  	if( !world[ch->in_room].dirs[direction] ) 
  	{
    	send_to_char("You cannot phase there...\n\r", ch);
    	return;
  	}
  	if( world[ch->in_room].dirs[direction]->to_room < 0 ) 
  	{
        send_to_char("You bumped into solid wall\n\r", ch) ;
        act("$n bumped into wall.",FALSE,ch,0,0,TO_ROOM);
        return ;
    }

  	if( IS_SET(world[ch->in_room].dirs[direction]->exit_info,EX_NOPHASE) ) 
  	{
    	send_to_char("You feel it's too solid.\n\r", ch);
    	return;
  	}

  	was_in=ch->in_room;

  	act("$n phases out silently.",FALSE,ch,0,0,TO_ROOM);

  	char_from_room(ch);
  	char_to_room( ch, world[was_in].dirs[direction]->to_room );

  	act("$n phases in silently.",FALSE,ch,0,0,TO_ROOM);
  	do_look(ch,"",0);
}

void spell_ventriloquate(  int level, charType *ch, char *arg, int type,
  									charType *tar_ch, objectType *tar_obj )
{
  	charType 	*	tmp_ch;           
  	char 			buf1[MAX_INPUT_LENGTH * 2];      
  	char 			buf2[MAX_INPUT_LENGTH * 2];      
  	char 			buf3[MAX_INPUT_LENGTH * 2];      

  	if( type != SPELL_SPELL ) 
  	{
    	DEBUG("spell_ventriloquate> Attempt to ventriloquate by non-cast.");
    	return;
  	}

  	for(; *arg && (*arg == ' '); arg++);

  	if( tar_obj ) 
  	{
    	sprintf(buf1, "The %s says '%s'\n\r", oneword(tar_obj->name), arg);       
    	sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n\r",
      	oneword(tar_obj->name), arg);  
  	}  
  	else 
  	{
    	sprintf(buf1, "%s says '%s'\n\r", GET_NAME(tar_ch), arg);
    	sprintf(buf2, "Someone makes it sound like %s says '%s'\n\r",
      	GET_NAME(tar_ch), arg);
  	}
                                      
  	sprintf( buf3, "Someone says, '%s'\n\r", arg);

  	for( tmp_ch = world[ch->in_room].people; tmp_ch; tmp_ch = tmp_ch->next_in_room ) 
  	{
    	if( (tmp_ch != ch) && (tmp_ch != tar_ch) ) 
		{
      		if( saves_spell(ch, tmp_ch, SAVING_SPELL) )
        		send_to_char(buf2, tmp_ch);  
      		else
        		send_to_char(buf1, tmp_ch);  
    	} 
		else 
		{
      		if( tmp_ch == tar_ch ) send_to_char(buf3, tar_ch);  
    	}
  	}
}
