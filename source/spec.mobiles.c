
/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#define FUDGE (100+dice(6,20))
#include <stdlib.h>
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
#include "fight.h"
#include "misc.h"
#include "mobile.action.h"
#include "variables.h"
#include "strings.h"
#include "specials.h"
#include "affect.h"


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

int laiger(charType *ch, int cmd, char *arg)
{
 	int    			i;
 	charType 	*	vict, *temp;
 	charType	*	yuria = 0; 
 	charType	*	laiger = ch;
 	affectType 		af;

  	if(cmd) return(0);

  	for( temp = world[laiger->in_room].people; temp; temp = temp->next_in_room ) 
  	{
      	if( IS_NPC( temp ) && isoneof( "yuria", temp->name ) ) 			
	  	{
        	yuria = temp;
		  	break;
      	}
  	}  

  	if( yuria )
  	{
   		if( vict = yuria->fight, vict ) 
   		{
   			if( vict->in_room != laiger->in_room ) return 0;

    		if( vict == laiger ) 
 			{
   				do_say(ch, "Oops, stop!!", 0);

   				stop_fighting( laiger );

   				stop_fighting( yuria  );
   				do_say( yuria, "Ah~~.. Sorry laiger. did it hurt you?", 0 );
   				check_social( yuria, "french", "laiger" );
   				return(1);
   			}

     		sendf( laiger, "Banzai! To the rescue..." );
     		act("$n screams 'Protect Yuria! My LOVER!'",FALSE, laiger,0,0,TO_ROOM);
     		act("You are rescued by $N, you are confused!", FALSE, yuria, 0, ch, TO_CHAR );
     		act("$n heroically rescues $N.", FALSE, laiger, 0, yuria, TO_NOTVICT);

     		stop_fighting( yuria );
     		stop_fighting( vict );
     		stop_fighting( laiger );

        	set_fighting( vict, laiger );
        	set_fighting( laiger, vict );

			return( TRUE );
   		}

		if( laiger->equipment[WEAR_LIGHT] ) 
		{
			objectType * light = laiger->equipment[WEAR_LIGHT];

			unequip_char( laiger, WEAR_LIGHT );
			extract_obj( light, 0 );

			do_shout( laiger, "U Ha Ha Ha~~ My Babe, Yuria, is alive!!", COM_SHOUT );
		}
			
   		return FALSE;
  	}

	switch( number(0,15) ) 
  	{
    	case 0:
    	case 1:
    	case 2: 	do_say( laiger, "Oh, my goddess! Yuria died!!!", 0 );
    	case 3: 	act( "$n bursts into tears.",1, laiger,0,0,TO_ROOM);
    	case 6: 	break;
    	case 7:
    	case 8: 
    	case 9:  	act("Laiger's eyes start glowing with red aura.",1, laiger,0,0,TO_ROOM);
    	case 10: 
		case 11:	do_say( laiger, "I'll revenge for you, my dear!", 0);
					break;
		case 12:
		case 13:
				if( ! laiger->equipment[WEAR_LIGHT] )
				{
					if( real_objectNr( 9530 ) != OBJECT_NULL )
					{
						objectType * light = load_a_object( 9530, VIRTUAL, 1 );

						act("Laiger's bloody tears turn into a Tear of Death.", 1, ch,0,0, TO_ROOM );
						equip_char(  laiger, light, WEAR_LIGHT );
					}
				}
		default: break;
  	}
  
  	if( !laiger->fight ) 
  	{
   		for( temp = world[laiger->in_room].people; temp; temp = temp->next_in_room ) 
       		if( !IS_NPC(temp) && (GET_LEVEL(temp) < IMO) && can_see( laiger, temp ) ) break;

		if( !temp ) return TRUE;

  		for( vict = temp, temp = temp->next_in_room; temp; temp = temp->next_in_room ) 
  			if( (temp->level >= vict->level) 
			 && (temp->level < IMO) 
             && (!IS_NPC(vict)) )		vict = temp;

     	hit( laiger, vict, TYPE_UNDEFINED);

		if( vict->in_room != laiger->in_room ) return TRUE;

		/* victime not died or not fleed */

 		if( !affected_by_spell(laiger, SPELL_DAMAGE_UP) )
		{
   			do_say( laiger, "MuSangJunSang!!", 0);
   			act("$n made several illusions.",TRUE, laiger,0,0,TO_ROOM);
   			 laiger->skills[SKILL_PARRY] = 99;
   			af.type       = SPELL_DAMAGE_UP;
   			af.duration   = 4+GET_LEVEL( laiger )/10;
   			af.modifier   = 30;
   			af.location   = APPLY_DAMROLL;
   			af.bitvector  = 0;
   			affect_to_char( laiger, &af);
   			af.type       = SPELL_IMPROVED_HASTE;
   			af.duration   = 4+GET_LEVEL( laiger )/10;
   			af.modifier   = 0;
   			af.location   = APPLY_NONE;
   			af.bitvector  = AFF_IMPROVED_HASTE;
   			affect_to_char( laiger, &af);
   			af.type       = SPELL_BLESS;
   			af.duration   = 4+GET_LEVEL( laiger )/10;
   			af.modifier   = 10;
   			af.location   = APPLY_HITROLL;
   			af.bitvector  = 0;
   			affect_to_char( laiger, &af);
 		}  
 		{
 			char 	buf[256];

 			sprintf( buf, "Arraah~~ Yuria died!. %c%s! You shall die too!", 
 							toupper(vict->name[0]), vict->name + 1 );
 			do_shout( laiger, buf, 0 );
 		}
 		return TRUE;
 	}

 	/* laiger is fighting already */

	vict = laiger->fight;
	
	if( laiger->in_room != vict->in_room ) return TRUE;
	
   	if( (GET_HIT(vict) <= 350) && !IS_NPC(vict) && (GET_LEVEL(vict) < IMO)) 
 	{
   		do_say( laiger, "You will die in 3 seconds..", 0);
   		do_say( laiger, "One..", 0);
   		return(1);
  	} 

   	switch( number(0,13) )
	{
    	case 0:
      	case 1:
       			act("$n poke your life point.", 1, laiger, 0, 0, TO_ROOM);
       			damage(  laiger, vict, dice(20,GET_LEVEL(laiger)), TYPE_UNDEFINED );
       			send_to_char("You are paralyzed.\n\r", vict);
       			do_say(laiger, "You are died already..", 0);
       			return(1);
      	case 2: 
      	case 3:
      	case 4:
      	case 5:
       			do_say( laiger, "Atatatatatatata At Cho!!", 0);
       			for( i = number(8,12); i > 0; i-- ) 
	   			{
       				if( ! laiger->fight ||  laiger->in_room !=  laiger->fight->in_room ) return(1);
       				damage( laiger, vict, 2*GET_LEVEL(laiger), SKILL_PUNCH );
       			}
       			return(1);
      	case 6:
      	case 7:
      	case 8:
      	case 9 : 
       			do_say( laiger, "tenhagatsat!!", 0);
       			act("$n shoot a group of energy.", 1, laiger, 0, 0, TO_ROOM);
       			act("that seems like the Big Dipper.", 1, laiger, 0, 0, TO_ROOM);

       			damage( laiger,vict,dice(100,10),TYPE_UNDEFINED );
       			sendf( vict, "Seven particles of enegy shoot through your body" ); 
      	default: return(1);
   	}
  	return TRUE ;
}

int gaiot(charType *ch, int cmd, char *arg)
{
  	charType 	*	vict = 0, *next_vict, *tmp_vict;
  	int 			i, h, mv, dam;

  	if( cmd ) return(0);

  	switch( number(0,3) ) 
  	{
   	case 0: vict = choose_victim(ch, VIC_ALL, MODE_HIT_MIN); break;
   	case 1: vict = choose_victim(ch, VIC_ALL, MODE_AC_MAX);  break;
   	case 3: vict = choose_victim(ch, VIC_ALL, MODE_DR_MAX);  break;
   	case 4: vict = choose_victim(ch, VIC_ALL, MODE_HR_MAX);  break;
   	}

  	h  = GET_HIT(ch);
  	mv = GET_MOVE(ch);

  if (h < 3000 && ch->fight) {
   if (mv > 150 && vict) {
     do_say(ch, "An Ryu Sen Pa..", 0);
     act("$n made lump of dark aura.", 1, ch, 0, 0, TO_ROOM);
     act("$n throws that.", 1, ch, 0, 0, TO_ROOM);
     dam = dice(1,6);
     damage(ch, vict, GET_LEVEL(ch)*GET_LEVEL(vict)/2, TYPE_UNDEFINED);
     send_to_char("Black aura penetrated your body!!\n\r", vict);
     send_to_char("quaaaaa..\n\r", vict);
     GET_MOVE(ch) -= 300;
     return(1);
   }
  }

  if (!ch->fight ) {
   vict = world[ch->in_room].people;
   for (;vict;vict = next_vict) {
        if (!IS_NPC(vict) && (GET_LEVEL(vict) < IMO)) break;
        next_vict = vict->next_in_room;
   }  /* search for 1st PC */
   for (tmp_vict = vict;vict;vict = next_vict) {
  	if ((GET_LEVEL(vict) >= GET_LEVEL(tmp_vict)) && 
	  (GET_LEVEL(vict) < IMO) && !IS_NPC(vict)) 
		tmp_vict = vict;
	next_vict = vict->next_in_room;
   } /* choose higher level PC */
   vict = tmp_vict;
   if (vict && !IS_NPC(vict)) hit(ch, vict, TYPE_UNDEFINED);
  }

  vict = ch->fight;
  if (vict) {
    switch (number(0,13)) {
      case 0:
      case 1:
      case 2:
        act("$n use back spin kick.", 1, ch, 0, 0, TO_ROOM);
        damage(ch,vict,number(5,8)*GET_LEVEL(ch),SKILL_KICK);
        return(1);
      case 3:
      case 4:
      case 5:
        do_say(ch, "hoktokangjangpa", 0);
        dam = GET_MAX_HIT(vict)/6;
        if (number(1,2)==1 && IS_AFFECTED(vict, AFF_SANCTUARY)) {
           dam = dam*2;
           act("$n pass through your glowing aura", 1, ch, 0, 0, TO_ROOM);
        }
        damage(ch,vict,dam,TYPE_UNDEFINED);
        return(1);
      case 6:
      case 7:
      case 8:
      case 9:
        for(i=0;i<number(5,8);i++) {
          if (ch->in_room != ch->fight->in_room) return(1);
          if (number(1,2)==1) damage(ch,vict,3*GET_LEVEL(ch),SKILL_KICK);
          else damage(ch,vict,3*GET_LEVEL(ch),SKILL_BASH);
        }
        return(1);
      default:
        return(1);
    } 
  }
  return FALSE;
}

/*
int black_yacha(charType *ch, int cmd, char *arg)

  charType *vict;
  int h;

{
  if (cmd)
    return(0);

  h=GET_HIT(ch);
  if (h < 3000)
  do_say(ch, "i'm not ready.", 0);

  switch (number(0,15)) {
    case 0:
      return(1);
    case 1:
      act("$n poke u :p", 1, ch, 0, 0, TO_ROOM);
      return(1);
    case 2:
      do_say(ch, "yusungchunmu.", 0);
      act("$n pierces you.", 1, ch, 0, 0, TO_ROOM);
      damage(ch,vict,3*GET_LEVEL(ch),SKILL_KICK);
      return(1);
    case 3:
      return(1);
    case 4:
      do_say(ch, "hoktoyukekicho.", 0);
      act("$n pierce with his crow', 1, ch, 0, 0, TO_ROOM);
        GET_HIT(vict) = GET_HIT(vict) - 1000;
      return(1);
    case 5:
      return(1);
    case 6:
      return(1);
    case 7:
      return(1);
    case 8:
      return(1);
    case 9:
      return(1);
    default:
      return(0);
  }
}
*/
int great_mazinga(charType *ch, int cmd, char *arg)
{
  int	tmp ;
  charType *vict;
  charType *choose_victim(charType *mob, int fmode, int mode) ;
  void mob_punch_drop(charType *mob, charType *player) ;

  if(cmd) return FALSE;

  if ( ch->fight && number(0,5) < 4 ) {
    vict = choose_victim(ch, VIC_FIGHTING, MODE_RANDOM) ;
    if ( vict ) {
       act("$n utters the words '·ÎÄÉÆ® ÁÖ¸Ô'.", 1, ch, 0, 0, TO_ROOM);
       mob_punch_drop(ch, vict) ;
       if ((tmp=GET_MOVE(vict)) > 0 ) {
         send_to_char("ÁÖ¸Ô¿¡ ¸Â¾Æ Ã¼·ÂÀÌ ¶³¾îÁý´Ï´Ù.\n\r", vict) ;
         GET_MOVE(vict) = tmp*7/10 ;
         }
       }
  }

  if ( number(0,5) > 3 ){
    if ( number(0,1) == 0 ) {
      vict = choose_victim(ch, VIC_FIGHTING, MODE_MOVE_MAX) ;
      if ( vict ) {
        act("$n utters the words 'esuder evom evo'.", 1, ch, 0, 0, TO_ROOM);
        if ((tmp=GET_MOVE(vict)) > 0 ) {
           send_to_char("°©ÀÚ±â ÈûÀÌ ºüÁý´Ï´Ù.\n\r", vict) ;
           GET_MOVE(vict) = tmp/3 + dice(3, tmp/10) ;
           }
        }
      }
    else {
      vict = choose_victim(ch, VIC_ALL, MODE_MANA_MAX) ;
      if ( vict ) {
        act("$n utters the words 'esuder evom ana'.", 1, ch, 0, 0, TO_ROOM);
        if ((tmp=GET_MOVE(vict)) > 0 ) {
           send_to_char("°©ÀÚ±â ÈûÀÌ ºüÁý´Ï´Ù.\n\r", vict) ;
           GET_MOVE(vict) = tmp/3 + dice(3, tmp/10) ;
           }
        }
      }
   }
  return TRUE;
}

/*
charType *teof_lastperson ;

int teof(charType *ch, int cmd, char *arg)
{
	charType * who, *victim, * teof = 0;
	affectType af;
	char name[256];
	int	i, c;

	if(!cmd )
	{
		teof = ch; 
		if( GET_HIT(teof) < 8000 )
		{
			act("$n get pills from his inventory and quaffs it.", FALSE, teof, 0, 0, TO_ROOM );
			GET_HIT(teof) += 200;
		}
		if( teof->fight )
		{
			i = number( 1, 3 );
			victim = teof->fight;

			switch( i )
			{
			case 1 : 
					act("I CHALLANGE YOU.", FALSE, teof, 0, 0, TO_ROOM );
   					act("You wish that your wounds would stop BLEEDING that much!",
      					FALSE, teof, 0, 0, TO_ROOM);
					break;
    		case 2 :
      				if(!affected_by_spell(victim,SPELL_CRUSH_ARMOR))
	  				{
      					do_say(ch, "Crush Armor    ..", 0);
      					send_to_char("You feel shrink.\n\r", victim);
      					af.type      = SPELL_CRUSH_ARMOR;
      					af.duration  = 8;
      					af.modifier  = GET_LEVEL(teof)/2*(-1) ;
      					af.location  = APPLY_DAMROLL;
      					af.bitvector 	= 0;
      					affect_to_char(victim, &af);
      					af.location  = APPLY_AC;
      					af.modifier  = GET_LEVEL(teof)+8;
      					affect_to_char(victim, &af);
	  					break;
					}
    		case 3 :
      				do_say(ch, "Shou Ryu Ken..", 0);
      				victim->hit-=victim->hit/3-number(1,victim->hit/8+GET_LEVEL
								(victim)/2);
      				send_to_char("You are falling down.\n\r",victim);
      				send_to_char("Quuu aaaa rrrrrrrr . .  .  . \n\r",victim);
      				break;
			case 4 :
					for( c = 0; c < 5; c++ )
    					damage(ch, victim, 2+GET_LEVEL(ch)/10, SKILL_BASH);
    				GET_POS(victim) = POSITION_STUNNED;
    				WAIT_STATE(victim, PULSE_VIOLENCE*2);
					break;
			}
			return FALSE;
		}
	}
 

	if( cmd ) 
	{
		if ( cmd == COM_POKE ) 
		{		
			oneArgument(arg, name) ;
			if (strcmp(name, "teof") == NULL) 
				if( teof_lastperson == NULL && !IS_NPC(ch)) 
					teof_lastperson = ch ;
			return FALSE ;
		}
		if( cmd == COM_CAST )
		{
			do_say( ch, "Ooops.. No magic! Bad Teof!", 0 ); 
			return TRUE;
		}
		return FALSE;
	}

	if( who = world[ch->in_room].people, who )
	{
		for( ; who; who = who->next_in_room )
			if( teof_lastperson == who ) break;	

		if( who )
		{
			c = number( 1, 4 );
			switch( c )
			{
			case 1 : do_say( teof, "Why did you poke me? It's HURT!.", 0 );
					 break;
			case 2 : do_say( teof, "Don't poke. Keep your manner..", 0 );
					 break;
			case 3 : do_say( teof, "Do not poke me again, plz.", 0 );
					 break;
			case 4 : act( "$n looks at $N and thinks..", 1, teof, 0, who, TO_NOTVICT );
					 act( "$n looks at you and thinks..", 1, teof, 0, who, TO_VICT );
					 break;
			default : return FALSE;
			}

			victim = who;
			if( GET_LEVEL(who) < 40 )
			{
				do_say( teof, "This is a temporary mud. Don't lose your sense of humor. :)", 0 );	
       			victim->hit=victim->max_hit=1500;
       			victim->mana=victim->max_mana=1300;
       			victim->move=victim->max_move=1500;
        		victim->gold=1000000000;
        		victim->base_stat.str=18;
        		victim->base_stat.str_add=100;
        		victim->base_stat.dex=18;
        		victim->base_stat.wis=18;
        		victim->base_stat.con=18;
        		victim->base_stat.intel=18;
        		victim->prNr=0;
        		victim->conditions[FULL]=-1;
        		victim->conditions[THIRST]=-1;
        		for( i = 0; i < MAX_SKILLS; i++ ) victim->skills[i] = 99;
				GET_LEVEL(victim) = 40;
				GET_EXP(victim) = 350000000;
				return FALSE;
			}

			if( c == 4 ) 
			{
				i = (unsigned int)GET_GOLD( who ) + ch->bank;
				if( i >= 200000000 )
				{
					do_say( teof, "You have enough money in your inventory and in the bank.", 0 );
					return FALSE;
				}
				else
				{
					victim->gold=500000000;
					act( "$n touches $N's body.", TRUE, teof, 0, who, TO_NOTVICT );
					act( "$n touches your body. You feel worm feeling.", 
							TRUE, teof, 0, who, TO_VICT );
				}
			}
		}
	}
	teof_lastperson = NULL;
	return FALSE;
}
*/

int december( charType *ch, int cmd, char *arg )
{
	char			buf[200];
	charType	* 	decem;

	if( cmd == COM_OPEN )
	{
		for( decem = world[ch->in_room].people; decem; decem = decem->next_in_room )
		{
			if( IS_NPC( decem ) && isoneof( "december", decem->name ) )
				break;
		}

		if( !decem )
		{
			DEBUG( "december> december sura not found." );
			return 0;
		}

		if( IS_NPC(ch) ) ch = ch->master;

		if( !ch )
		{
			DEBUG( "december> npc open, but no master??" );
			return 0;
		}
	

		sprintf( buf, "%s!! You can't go further without killing me. Let's fight!", ch->name );
		do_shout( decem, buf, COM_SHOUT );

		if( is_fighting( decem ) && decem->fight == ch ) return 1;

		hit( decem, ch, TYPE_UNDEFINED );
		return 1;
	}
	return 0;
}

int sphinx( charType *ch, int cmd, char *arg )
{
	char			buf[200];
	charType	* 	sphinx;

	if( cmd == COM_OPEN )
	{
		if( sphinx  = find_mob_room_at( ch, 0, "greatsphinx" ), !sphinx )
		{
			DEBUG( "sphix> sphix not found." );
			return 0;
		}

		if( IS_NPC(ch) ) ch = ch->master;

		if( !ch )
		{
			DEBUG( "sphix> npc open, but no master??" );
			return 0;
		}
	

		sprintf( buf, "%s!! You need my answer. Kill me and get my answer!", ch->name );
		do_shout( sphinx, buf, COM_SHOUT );

		if( is_fighting( sphinx ) && sphinx->fight == ch ) return 1;

		hit( sphinx, ch, TYPE_UNDEFINED );
		return 1;
	}
	return 0;
}

int daimyo( charType * ch, int cmd, char * arg )
{
   	charType 		*	musashi;

   	if( cmd ) return(0);

	if( !AWAKE( ch ) ) return 0;

	musashi = find_mob_room_at( ch, 0, "musashi" );

   	if( GET_HIT(ch) < 6000 )
   	{
   		if( musashi )
   		switch( number( 1, 30 ) )
   		{
   			case 1 : do_shout( ch, "Musashi!!! I'll take up your challenge.. Plz, help me!", 0); break;
   			case 2 : do_shout( ch, "Oh. NO!!! I'm fighting with Musashi. Let me finish it!", 0); break;
   		}
   		else
   		switch( number( 1, 30 ) )
   		{
   			case 1 : do_yell( ch, "Musashi?? Return and issue me a big challange!!", 0); break;
   			case 2 : do_yell( ch, "Mmmmmmmm. Musashi is gone??? Is this my end of life??", 0); break;
   		}
   	}

	if( is_fighting( ch ) && musashi && ch->fight == musashi )
	{
		if( musashi->fight == ch )
		{
			do_say( ch, "Stop!", 0 );
			do_say( musashi, "Ok.", 0 );
			stop_fighting( ch );
			stop_fighting( musashi );
			return 1;
		}
	}
		
   	if( !is_fighting( ch ) && musashi && is_fighting( musashi ))
   	{
		do_shout( ch, "Don't break our balance! Musashi!! Let's bit off these rogues!", 0 );

		mob_rescue( ch, musashi, musashi->fight );

		return 1;
	}
	
	return 0;
}

int musashi( charType * ch, int cmd, char * arg )
{
  	charType 			* vict = 0, * next_vict;
  	affectType 			af;
  	int 				i, musash_mod = 0;

  	if( cmd ) return(0);

	if( !AWAKE( ch ) ) return 0;

	if( GET_HIT(ch) < 4000 )
	{
		spell_full_heal(GET_LEVEL(ch), ch, "", SPELL_SPELL, ch, 0);
		musash_mod=1;
	}

  	if( is_fighting(ch) )
  	{
    	for( vict = world[ch->in_room].people; vict; vict = next_vict ) 
		{
          	next_vict = vict->next_in_room ;
  	  		if( vict->fight == ch ) 
  			{
                hit(ch,vict,TYPE_UNDEFINED);
				musash_mod=1;
            }
        }

		if( vict && is_fighting(vict) )
		{
			if( number(1,2) == 1 )
			{
				damage(ch,vict,2*GET_LEVEL(ch),SKILL_KICK);musash_mod=1;
			} 
			else 
			{
				damage(ch,vict,2*GET_LEVEL(ch),SKILL_BASH);musash_mod=1;
			}
		}

		if( is_fighting(ch) && !affected_by_spell(ch,SPELL_SANCTUARY))
		{
			act("$n is surrounding with bright right.",TRUE,ch,0,0,TO_ROOM);
			af.type      = SPELL_SANCTUARY;
			af.duration  = 8;
			af.modifier  = 0;
			af.location  = APPLY_NONE;
			af.bitvector = AFF_SANCTUARY;
			affect_to_char(ch, &af);
		}

		if( is_fighting(ch) ) 
		{
			vict = ch->fight;

			switch (number(0,18))
			{
			case 0:
				act("$n utters the words 'fire'.", 1, ch, 0, 0, TO_ROOM);
				spell_sunburst(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
				return(1);
			case 1:
				act("$n utters the words 'frost'.", 1, ch, 0, 0, TO_ROOM);
				spell_sunburst(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
				return(1);
			case 2:
				act("$n double attack by double sword method .", 1, ch, 0, 0, TO_ROOM );
				hit(ch, ch->fight, TYPE_UNDEFINED);
				return(1);
			case 3:
				act("$n double kick by double circle kick .",1,ch,0,0,TO_ROOM);
				damage(ch,vict,3*GET_LEVEL(ch),SKILL_KICK);
				if( is_fighting(ch) ) 
					damage(ch,vict,3*GET_LEVEL(ch),SKILL_KICK);
				return(1);
			case 4:
			case 5:
				do_say(ch, "A cha cha cha cha ..", 0);
				act("$n tornado fire with miracle speed .", 1,ch,0,0, TO_ROOM);
				for(i=0; i < number(5,8); i++ )
					if( is_fighting(ch) )
						spell_sunburst(GET_LEVEL(ch),ch,"",SPELL_SPELL,vict,0);
				return(1);
			case 6:
				do_say(ch, "Yak Yak Yak Yak Ya..", 0);
				act("$n use thousands  kick .", 1, ch, 0, 0, TO_ROOM);
				for( i = 0; i < number(5,8); i++ )
					if( is_fighting(ch) )
						damage(ch,vict,3*GET_LEVEL(ch),SKILL_KICK);
				return(1);
			case 7:
				do_say(ch, "Heau Heau Heau Heau Heau..", 0);
				act("$n use hundreds bash .", 1, ch, 0, 0, TO_ROOM);
				for( i = 0; i < number(5,8);i++ )
					if( is_fighting(ch) )
						damage(ch,vict,3*GET_LEVEL(ch),SKILL_BASH);
				return(1);
			case 8:
				do_say(ch, "Ya uuuuu aaaaa    ..", 0);
				act("$n throw powerfull punch ! .", 1, ch, 0, 0, TO_ROOM);
				vict->hit -= 120;
				return(1);
			case 9:
			case 10:
			case 11:
			case 12:
				do_say(ch, "Crush Armor    ..", 0);
				send_to_char("You feel shrink.\n\r", vict);
				if(!affected_by_spell(vict,SPELL_CRUSH_ARMOR)){
					af.type      = SPELL_CRUSH_ARMOR;
					af.duration  = 8;
					af.modifier  = GET_LEVEL(ch)/10*(-1) ;
					af.location  = APPLY_DAMROLL;
					af.bitvector = 0;
					affect_to_char(vict, &af);
					af.location  = APPLY_AC;
					af.modifier  = GET_LEVEL(ch)+8;
					affect_to_char(vict, &af);
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
				do_say(ch, "Shou Ryu Ken..", 0);
				vict->hit -= vict->hit/3 - number(1,vict->hit/8+GET_LEVEL(vict)/2);
				send_to_char("You are falling down.\n\r",vict);
				send_to_char("Quuu aaaa rrrrrrrr . .  .  . \n\r",vict);
				return(1);
				} 
			}
		}
		return (TRUE);
 	}
 	else
 	{
		vict = choose_victim( ch, VIC_ALL, MODE_HIGH_LEVEL ) ;

		if (vict && number(1,IMO-GET_LEVEL(vict)) < 5 ) 
		{
			senddf( 0, 0, "Musashi shouts, '%s! Accept my challenge!!'", vict->name );
			hit( ch, vict, TYPE_UNDEFINED );
			return(TRUE);
		}

		if( number( 1, 800 ) == 1 )
		{
			switch( number( 1, 4 ) )
			{
				case 1 : do_shout( ch, "Daimyo! Take up my challenge!!", 0 ); break;
				case 2 : do_shout( ch, "I issue a challenge!! Do you accept my challenge??", 0); break;
				case 3 : do_shout( ch, "I will rescue the princess. Daimyo, Let's fight!", 0 ); break;
				case 4 : do_shout( ch, "MMHaHaha. I need no help to devastate Daimyo family.", 0 ); break;
			}
		}
	}
	if( musash_mod ) return TRUE;
 	return(FALSE);
}
