/* ************************************************************************
*  File: fight.c , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "object.h"
#include "mobile.h"
#include "utils.h"
#include "limits.h"
#include "comm.h"
#include "find.h"
#include "interpreter.h"
#include "weapon.h"
#include "magics.h"
#include "update.h"
#include "variables.h"
#include "fight.h"
#include "misc.h"
#include "strings.h"
#include "stash.h"
#include "affect.h"
#include "transfer.h"

charType *combat_list = 0;     /* head of l-list of fighting chars */
charType *combat_next_dude = 0; /* Next dude global trick           */

static struct message_list fight_messages[MAX_MESSAGES];

#define PART_CORPSE_DAM		140
/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit",   "hits"},             /* TYPE_HIT      */
  {"pound", "pounds"},           /* TYPE_BLUDGEON */
  {"pierce", "pierces"},         /* TYPE_PIERCE   */
  {"slash", "slashes"},          /* TYPE_SLASH    */
  {"whip", "whips"},             /* TYPE_WHIP     */
  {"shoot", "shoots"},           /* TYPE_SHOOT    */
  {"bite", "bites"},             /* TYPE_BITE     */
  {"sting", "stings"},           /* TYPE_STING    */
  {"crush", "crushs"}            /* TYPE_CRUSH    */
};

struct attack_hit_type attack_hit_han[] =
{
  {"Ĩ�ϴ�",   "Ĩ�ϴ�"},		/* TYPE_HIT      */
  {"��Ÿ �մϴ�", "��Ÿ �մϴ�"},	/* TYPE_BLUDGEON */
  {"��ϴ�", "��ϴ�"},		/* TYPE_PIERCE   */
  {"���ϴ�", "���ϴ�"},			/* TYPE_SLASH    */
  {"ä���� �մϴ�", "ä���� �մϴ�"},	/* TYPE_WHIP     */
  {"���ϴ�", "���ϴ�"},			/* TYPE_SHOOT    */
  {"���� ����ϴ�", "���� ����ϴ�"},	/* TYPE_BITE     */
  {"�� ���ϴ�", "�� ���ϴ�"},		/* TYPE_STING    */
  {"�μ��ϴ�", "�μ��ϴ�"}		/* TYPE_CRUSH    */
};

struct attack_hit_type attack_hit_han_brief[] =
{
  {"ħ",   "ħ"},             /* TYPE_HIT      */
  {"��Ÿ", "��Ÿ"},           /* TYPE_BLUDGEON */
  {"�", "�"},         /* TYPE_PIERCE   */
  {"��", "��"},          /* TYPE_SLASH    */
  {"ä������", "ä������"},             /* TYPE_WHIP     */
  {"��", "��"},           /* TYPE_SHOOT    */
  {"���� ����", "���� ����"},             /* TYPE_BITE     */
  {"��", "��"},           /* TYPE_STING    */
  {"�μ�", "�μ�"}           /* TYPE_CRUSH    */
};

struct dam_weapon_type 
{
  	char *to_room;
  	char *to_char;
  	char *to_victim;
};

static struct dam_weapon_type dam_weapons[] = {

  {"$n misses $N with $s #W.",                           /*    0    */
   "You miss $N with your #W.",
   "$n miss you with $s #W." },

   {"$n tickles $N with $s #W.",                          /*  1.. 4  */
    "You tickle $N as you #W $M.",
    "$n tickle you as $e #W you." },

   {"$n barely #W $N.",                                   /*  5.. 8  */
    "You barely #W $N.",
    "$n barely #W you."},

  {"$n #W $N.",                                          /*  9.. 12  */
    "You #W $N.",
    "$n #W you."}, 

  {"$n #W $N hard.",                                     /*  13..20  */
   "You #W $N hard.",
    "$n #W you hard."},

  {"$n #W $N very hard.",                                /* 21..30  */
   "You #W $N very hard.",
   "$n #W you very hard."},

  {"$n #W $N extremely hard.",                          /* 31..40  */
   "You #W $N extremely hard.",
   "$n #W you extremely hard."},

  {"$n massacres $N to small fragments with $s #W.",     /* > 40    */
   "You massacre $N to small fragments with your #W.",
   "$n massacres you to small fragments with $s #W."},

  {"$n annihilates $N to micro molecules with $s #W.",   /* > 65    */
   "You annihilates $N to micro molecules with your #W.",
   "$n annihilates you to micro molecules with $s #W."},

  { "$n mutilates $N with $s #W.",                    /* > 110 */
    "You mutilate $N with your #W.",
    "$n mutilates you with $s #W." },

  { "$n OBLITERATES $N with $s DEADLY #W!!",          /*  > 140   */
    "You OBLITERATE $N with your DEADLY #W!!",
    "$n OBLITERATES you with $s DEADLY #W!!" },

  { "$n DEMOLISHES $N with $s #W. A thrilling POWER!!",         /* > 200 */
    "You DEMOLISH $N with your #W. A thrilling POWER!!", 
	"$n DEMOLISHES you with $s #W. You are THRILLED!!" },
};

static struct dam_weapon_type dam_weapons_brief[] = {

  {"$n misses $N.",                           /*    0    */
   "You miss $N.",
   "$n miss you." },

   {"$n tickles $N.",                          /*  1.. 4  */
    "You tickle $N.",
    "$n tickle you." },

   {"$n barely #W $N.",                        /*  5.. 8  */
    "You barely #W $N.",
    "$n barely #W you."},

  {"$n #W $N.",                                /*  9.. 12  */
    "You #W $N.",
    "$n #W you."}, 

  {"$n #W $N hard.",                           /*  13..20  */
   "You #W $N hard.",
    "$n #W you hard."},

  {"$n #W $N v-hard.",                         /* 21..30  */
   "You #W $N v-hard.",
   "$n #W you v-hard."},

  {"$n #W $N ex-hard.",                        /* 31..40  */
   "You #W $N ex-hard.",
   "$n #W you ex-hard."},

  {"$n massacre $N.",     					   /* > 40    */
   "You massacre $N.",
   "$n massacre you."},

  {"$n annihilates $N.",   						/* > 65    */
   "You annihilate $N.",
   "$n annihilates you."},

  { "$n mutilates $N.",                    /* > 110 */
    "You mutilate $N.",
    "$n mutilates you." },


  { "$n OBLITERATES $N with $s DEADLY #W!!",          /*  > 140   */
    "You OBLITERATE $N with your DEADLY #W!!",
    "$n OBLITERATES you with $s DEADLY #W!!" },

  { "$n DEMOLISHES $N with $s #W. A thrilling POWER!!",         /* > 200 */
    "You DEMOLISH $N with your #W. You thrilled $N!!",
    "$n DEMOLISHES you with $s #W. You are thrilled!!" },
 };

static struct dam_weapon_type han_weapons[] = 
{
  {"$n���� $N���� �������� ���������ϴ�.",        /*    0    */
   "����� $N���� ������ ���߽��ϴ�.",
   "$n���� ����� �������� �����߽��ϴ�." },

   {"$n���� $N���� �������� #W.",                          /*  1.. 4  */
    "����� $N���� �������� #W.",
    "$n���� ����� �������� #W." },

   {"$n���� $N���� ��� #W.",                                   /*  5.. 8  */
    "����� $N���� ��� #W.",
    "$n���� ����� ��� #W."},

  {"$n���� $N���� #W.",                                          /*  9.. 12  */
    "����� $N���� #W.",
    "$n���� ����� #W."}, 

  {"$n���� $N���� ���� #W.",                                     /*  13..20  */
   "����� $N���� ���� #W.",
    "$n���� ����� ���� #W."},

  {"$n���� $N���� �ſ� ���� #W.",                                /* 21..30  */
   "����� $N���� �ſ� ���� #W.",
   "$n���� ����� �ſ� ���� #W."},

  {"$n���� $N���� ���������ϰ� #W.",                          /* 31..40  */
   "����� $N���� ���������ϰ� #W.",
   "$n���� ����� ���������ϰ� #W."},

  {"$n���� $N���� ���� ���������� #W.",     /* > 40    */
   "����� $N���� ���� ���������� #W.",
   "$n���� ����� ���� ���������� #W."},

  {"$n���� $N���� ���� ��������� ���� #W.",     /* > 65    */
   "����� $N���� ���� ��������� ���� #W.",
   "$n���� ����� ���� ��������� ���� #W."},

  {"$n���� $N���� ���� ���簡 �ǰ� #W.",     /* > 110    */
   "����� $N���� ���� ���簡 �ǰ� #W.",
   "$n���� ����� ���� ���簡 �ǰ� #W."},

  {"$n���� $N���� ���� ������ �ǰ� #W.",     /* > 140    */
   "����� $N���� ���� ������ �ǰ� #W.",
   "$n���� ����� ���� ������ �ǰ� #W."},

  {"$n���� $N���� ���� ������ �ǰ� #W.",     /* > 140    */
   "����� $N���� ���� ������ �ǰ� #W.",
   "$n���� ����� ���� ������ �ǰ� #W."},
};

static struct dam_weapon_type han_weapons_brief[] = 
{
   { "$n���� $N���� ������.",             /*    0    */
   	 "����� $N���� ������.",
   	 "$n���� ����� ������." },

   { "$n���� $N���� �������� #W.",        /*  1.. 4  */
   	 "����� $N���� �������� #W.",
   	 "$n���� ����� �������� #W." },

   { "$n���� $N���� ��� #W.",             /*  5.. 8  */
   	 "����� $N���� ��� #W.",
   	 "$n���� ����� ��� #W."},

   { "$n���� $N���� #W.",                   /*  9.. 12  */
     "����� $N���� #W.",
     "$n���� ����� #W."}, 

   { "$n���� $N���� ���� #W.",              /*  13..20  */
   	 "����� $N���� ���� #W.",
   	 "$n���� ����� ���� #W."},

   { "$n���� $N���� �ż��� #W.",            /* 21..30  */
   	 "����� $N���� �ż��� #W.",
   	 "$n���� ����� �ż��� #W."},

   { "$n���� $N���� �����ϰ� #W.",          /* 31..40  */
   	 "����� $N���� �����ϰ� #W.",
   	 "$n���� ����� �����ϰ� #W."},

   { "$n���� $N���� ���������� #W.",        /* > 40    */
   	 "����� $N���� ���������� #W.",
   	 "$n���� ����� ���������� #W."},

   { "$n���� $N���� ����������� #W.",      /* > 65    */
   	 "����� $N���� ����������� #W.",
   	 "$n���� ����� ����������� #W."},

   { "$n���� $N���� ����ǰ� #W.",          /* > 110    */
   	 "����� $N���� ����ǰ� #W.",
   	 "$n���� ����� ����ǰ� #W."},

   { "$n���� $N���� �����ǰ� #W.",          /* > 110    */
   	 "����� $N���� �����ǰ� #W.",
   	 "$n���� ����� �����ǰ� #W."},

   { "$n���� $N���� �����ǰ� #W.",          /* > 110    */
   	 "����� $N���� �����ǰ� #W.",
   	 "$n���� ����� �����ǰ� #W."},
};


void appear(charType *ch)
{
  	act("$n slowly fade into existence.", FALSE, ch,0,0,TO_ROOM);
  	if( affected_by_spell(ch, SPELL_INVISIBLE) )
    	affect_from_char(ch, SPELL_INVISIBLE);
  	REMOVE_BIT(ch->affects, AFF_INVISIBLE);
}

void load_messages(void)
{
 	FILE 				*	f1;
 	int 					i,type;
 	struct message_type *	messages;
 	char 					chk[100];

 	if (!(f1 = errOpen(MESS_FILE, "r")))
 	{
  		FATAL( "load_messages> can't find %s", MESS_FILE );
 	}

 	for (i = 0; i < MAX_MESSAGES; i++)
 	{ 
  		fight_messages[i].a_type = 0;
  		fight_messages[i].number_of_attacks=0;
  		fight_messages[i].msg = 0;
 	}

 	fscanf(f1, " %s \n", chk);

 	while(*chk == 'M')
 	{
  		fscanf(f1," %d\n", &type);

  		for( i = 0; ( i < MAX_MESSAGES ) 
				 && ( fight_messages[i].a_type != type ) 
                 && ( fight_messages[i].a_type);  i++ )
		;
  		if( i >= MAX_MESSAGES )
  		{
   			FATAL("load_fight_message> Too many combat messages.");
  		}

		messages = (struct message_type *)errMalloc( sizeof(struct message_type));
		memset(messages, 0, sizeof(struct message_type)) ;

  		fight_messages[i].number_of_attacks++;
  		fight_messages[i].a_type=type;
  		messages->next=fight_messages[i].msg;
  		fight_messages[i].msg=messages;
  		messages->die_msg.attacker_msg      = fread_string(f1);
  		messages->die_msg.victim_msg        = fread_string(f1);
  		messages->die_msg.room_msg          = fread_string(f1);
  		messages->miss_msg.attacker_msg     = fread_string(f1);
  		messages->miss_msg.victim_msg       = fread_string(f1);
  		messages->miss_msg.room_msg         = fread_string(f1);
  		messages->hit_msg.attacker_msg      = fread_string(f1);
  		messages->hit_msg.victim_msg        = fread_string(f1);
  		messages->hit_msg.room_msg          = fread_string(f1);
  		messages->god_msg.attacker_msg      = fread_string(f1);
  		messages->god_msg.victim_msg        = fread_string(f1);
  		messages->god_msg.room_msg          = fread_string(f1);
  		fscanf(f1, " %s \n", chk);
 	}
 	fclose(f1);
}

int is_fighting( charType * ch )
{
	if( !ch )
	{
		DEBUG( "is_fighting> call with NULL" );
		return FALSE;
	}
	if( !ch->fight ) return FALSE;
	if( GET_POS( ch->fight ) == POSITION_DEAD ) 
	{
		DEBUG( "is_fighting> %s vs %s <-- is DEAD.", ch->name, ch->fight->name );
		return FALSE;
	}
	if( ch->in_room != ch->fight->in_room ) return FALSE;
	if( GET_POS( ch ) == POSITION_DEAD ) 
	{
		DEBUG( "is_fighting> %s is DEAD!!!", ch->name );
		return FALSE;
	}

	return TRUE;
}

void set_fighting(charType *ch, charType *vict)
{
	charType 	*	tmp;

 	for( tmp = combat_list; tmp; tmp = tmp->next_fighting )
 	{
 		if( tmp == ch )
 		{
 			DEBUG( "set_fighting> %s is already in combat_list", ch->name );
 			return;
 		}
 	}

 	ch->next_fighting = combat_list;
 	combat_list = ch;

 	if( IS_AFFECTED(ch,AFF_SLEEP) )	affect_from_char( ch, SPELL_SLEEP );

 	ch->fight = vict;
 	GET_POS(ch) = POSITION_FIGHTING;
}

void stop_fighting(charType *ch)
{
	charType	*	tmp;

	if( !ch->fight ) return;

 	if( ch == combat_next_dude ) combat_next_dude = ch->next_fighting;

 	if( combat_list == ch ) combat_list = ch->next_fighting;
 	else
 	{
  		for( tmp = combat_list; tmp && (tmp->next_fighting != ch ); tmp = tmp->next_fighting)
  		;

  		if( !tmp ) 
  		{
   			FATAL( "stop_fighting> fighting not found Error." );
  		}
  		tmp->next_fighting = ch->next_fighting;
 	}

 	ch->next_fighting 	= 0;
 	ch->fight 			= 0;
 	GET_POS(ch)         = POSITION_STANDING;

 	update_pos(ch);
}

void death_cry( charType * ch , int dam )
{
 	int door, was_in;

 	acthan("Your blood freezes as you hear $ns death cry.",
   				"$n���� ó���� ���Ҹ��� �鸳�ϴ�. �� �Ҹ����� ~~",  FALSE, ch,0,0,TO_ROOM);
 	was_in = ch->in_room;

 	for( door = 0; door <= 5; door++ ) 
 	{
  		if( can_go(ch, door) ) 
  		{
   			ch->in_room = world[was_in].dirs[door]->to_room;
   			acthan("Your blood freezes as you hear someones death cry.",
    			"��𼱰� �Ҹ���ġ�� ���Ҹ��� ����ɴϴ�.", FALSE,ch,0,0,TO_ROOM);
   			ch->in_room = was_in;
  		}
 	}
}

void kill_char( charType * ch, int level, int dam, int attacktype )
{
 	death_cry( ch, dam );

	if( attacktype == SPELL_DISINTERGRATE ) make_t_corpse( ch, level );
	else if( (attacktype == SKILL_DAZZLE || attacktype == TYPE_SLASH) && dam >= PART_CORPSE_DAM )
	{
		if( attacktype == SKILL_DAZZLE ) make_d_corpse( ch, level );
		else if( dam >= PART_CORPSE_DAM )
		{
			if( dam < PART_CORPSE_DAM + 10 )
			{
				switch( number( 1, 2 ) )
				{
					case 1 : make_lf_corpse( ch, level ); break;
					case 2 : make_rf_corpse( ch, level ); break;
				}
			}
			else if( dam < PART_CORPSE_DAM + 20 )
			{
				switch( number( 1, 4 ) )
				{
					case 1 : make_lh_corpse( ch, level ); break;
					case 2 : make_rh_corpse( ch, level ); break;
					case 3 : make_ll_corpse( ch, level ); break;
					case 4 : make_rl_corpse( ch, level ); break;
				}
			}
			else if( dam < PART_CORPSE_DAM + 30 )
			{
				switch( number( 1, 3 ) )
				{
					case 1 : make_h_corpse( ch, level ); break;
					case 2 : make_l_corpse( ch, level ); break;
					case 3 : make_t_corpse( ch, level ); break;
				}
			}
			else
			{
				switch( number( 1, 2 ) )
				{
					case 1 : make_c_corpse( ch, level ); break;
					case 2 : make_d_corpse( ch, level ); break;
				}
			}
		}
	}
	else if( attacktype == TYPE_CRUSH && dam >= PART_CORPSE_DAM )
	{
		make_nt_corpse( ch, level );
	}
	else if( attacktype == TYPE_PIERCE && dam >= PART_CORPSE_DAM )
	{
		make_vh_corpse( ch, level );
	}
	else make_a_corpse( ch, level );

	char_from_world( ch );

 	if( IS_NPC( ch ) ) free_a_char( ch );
 	else
 	{
 		save_char( ch, NOWHERE );

		if( !ch->desc )	free_a_char( ch );
	}
}

char * replace_string(char *str, char *weapon)
{
 	static char buf[5][256];
 	static int count = 0;
 	char *rtn;
 	char *cp;

 	cp = rtn = buf[count % 5];
 	count ++ ;

 	for (; *str; str++) 
 	{
  		if (*str == '#') 
  		{
   			switch(*(++str)) 
   			{
    			case 'W' : 
     				for (; *weapon; *(cp++) = *(weapon++));
     				break;
    		default :
     				*(cp++) = '#';
     				break;
   			}
  		} 
  		else *(cp++) = *str;

  		*cp = 0;
 	} 
 	return(rtn);
}

void dam_message(int dam, charType *ch, charType *victim, int w_type)
{
 	objectType *wield;
 	char *buf, *buf2, *bufeb, *bufhb;
 	int  msg_index ;

  	w_type -= TYPE_HIT;   /* Change to base of table with text */

  	wield = ch->equipment[WIELD];

  	if (dam == 0)		 	msg_index = 0 ;
  	else if ( dam <= 4 )	msg_index = 1 ;
  	else if ( dam <= 8 )	msg_index = 2 ;
  	else if ( dam <= 12 )	msg_index = 3 ;
  	else if ( dam <= 20 )	msg_index = 4 ;
  	else if ( dam <= 30 )	msg_index = 5 ;
  	else if ( dam <= 40 )	msg_index = 6 ;
  	else if ( dam <= 65 )	msg_index = 7 ;
  	else if ( dam <= 100)	msg_index = 8 ;
  	else if ( dam <= 140)	msg_index = 9 ;
  	else if ( dam <= 180)	msg_index = 10 ;
  	else					msg_index = 11 ;

  	if( msg_index == 3 || msg_index == 4 || msg_index == 5 || msg_index == 7 ) 
  	{
    	buf = replace_string ( dam_weapons[msg_index].to_room,
          			attack_hit_text[w_type].plural);
    	bufeb = replace_string ( dam_weapons_brief[msg_index].to_room,
          			attack_hit_text[w_type].plural);
    }
  	else 
  	{
    	buf = replace_string ( dam_weapons[msg_index].to_room,
          		attack_hit_text[w_type].singular);
    	bufeb = replace_string ( dam_weapons_brief[msg_index].to_room,
          		attack_hit_text[w_type].singular);
    }

  	buf2 = replace_string ( han_weapons[msg_index].to_room,
          		attack_hit_han[w_type].singular);
  	bufhb = replace_string ( han_weapons_brief[msg_index].to_room,
          		attack_hit_han_brief[w_type].singular);
  	acthanbrief(buf,buf2,bufeb,bufhb,FALSE, ch, wield, victim, TO_NOTVICT);

  	buf = replace_string ( dam_weapons[msg_index].to_char,
          		attack_hit_text[w_type].singular);
  	bufeb = replace_string ( dam_weapons_brief[msg_index].to_char,
          		attack_hit_text[w_type].singular);
  	buf2 = replace_string ( han_weapons[msg_index].to_char,
          		attack_hit_han[w_type].singular);
  	bufhb = replace_string ( han_weapons_brief[msg_index].to_char,
          		attack_hit_han_brief[w_type].singular);
  	acthanbrief(buf,buf2,bufeb, bufhb, FALSE, ch, wield, victim, TO_CHAR);

  	if ( msg_index == 1 || msg_index == 2 || msg_index == 3 || msg_index == 5 || msg_index == 7 ) 
  	{
    	buf = replace_string ( dam_weapons[msg_index].to_victim,
          		attack_hit_text[w_type].plural);
    	bufeb = replace_string ( dam_weapons_brief[msg_index].to_victim,
          		attack_hit_text[w_type].plural);
    }
  	else 
  	{
    	buf = replace_string ( dam_weapons[msg_index].to_victim,
          		attack_hit_text[w_type].singular);
    	bufeb = replace_string ( dam_weapons_brief[msg_index].to_victim,
          		attack_hit_text[w_type].singular);
    }
  	buf2 = replace_string ( han_weapons[msg_index].to_victim,
          		attack_hit_han[w_type].singular);
  	bufhb = replace_string ( han_weapons_brief[msg_index].to_victim,
          		attack_hit_han_brief[w_type].singular);
  	acthanbrief(buf,buf2,bufeb,bufhb, FALSE, ch, wield, victim, TO_VICT);
}

void damage(charType *ch, charType *victim, int dam, int attacktype) 
{
#define C_MESG( msg )	act((msg), FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR)
#define V_MESG( msg )	act((msg), FALSE, ch, ch->equipment[WIELD], victim, TO_VICT)
#define R_MESG( msg )	act((msg), FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT)

 	struct message_type *	messages;
 	int 					i,j,nr,max_hit,exp, cry = 0;

 	if( !ch || !victim )
 	{
   		DEBUG("damage> called with NULL pointer. (%s vs %s)", 
   						ch ? ch->name : "Null", victim ? victim->name : "Null" );
   		return;
   	}

 	if( ch->in_room != victim->in_room )
 	{
   		DEBUG("damage> %s vs %s <- Not same room??", ch->name, victim->name );
   		return;
   	}

 	if( GET_POS(victim) <= POSITION_DEAD )
 	{
   		DEBUG( "damage> %s vs %s <- Dead??", ch->name, victim->name );
   	}

 	if( !is_allow_kill( ch, victim) ) return;

 	if( (GET_LEVEL(victim) >= IMO ) && !IS_NPC(victim) ) dam = 0;

 	if( dam && (!IS_NPC(victim) && !(victim->desc)) ) 
 	{
  		if (!victim->fight) 
  		{
   			act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
   			victim->was_in_room = victim->in_room;
   			char_from_room(victim);
   			char_to_room(victim, real_roomNr( ROOM_VOID ));
   			dam = 0;
  		}
 	}
  
 	if( victim != ch ) 
 	{
  		if( GET_POS(victim) > POSITION_STUNNED ) 
  		{
   			if( !(victim->fight) ) set_fighting(victim, ch);
   			GET_POS(victim) = POSITION_FIGHTING;
  		}

  		if( GET_POS(ch) > POSITION_STUNNED) 
  		{
   			if( !(ch->fight) ) set_fighting(ch, victim);

   			if(   IS_NPC(ch) 
			   && IS_NPC(victim) && victim->master 
               && !number(0,10) && IS_AFFECTED(victim, AFF_CHARM) 
			   && (victim->master->in_room == ch->in_room) 
			   && can_see( ch, victim->master ) )
			{
    			if( ch->fight ) stop_fighting(ch);
    			hit( ch, victim->master, TYPE_UNDEFINED );
    			return;
   			}
  		}
 	}

 	if( is_same_group( ch, victim) ) die_follower( victim );
	if( victim->master == ch ) stop_follower( victim );

 	if( IS_AFFECTED(ch, AFF_INVISIBLE) ) appear(ch);

 	if( IS_AFFECTED(victim, AFF_SANCTUARY) )
 	{
    	if((attacktype >= TYPE_HIT) && (attacktype <= TYPE_CRUSH))
    	{
        	i = rand() % 100 + 1;
        	dam = dam * i / 100;
    	}
    	else dam >>= 1;
 	}

 	dam = MAX(dam,0);
 	GET_HIT(victim) -= dam;

 	if( ch != victim ) gain_exp( ch, GET_LEVEL(victim) * dam );

 	update_pos( victim );

 	if( (attacktype >= TYPE_HIT) && (attacktype <= TYPE_CRUSH) ) 
 	{
  		if( !ch->equipment[WIELD] ) 
  		{
   			dam_message(dam, ch, victim, TYPE_HIT);
  		} 
  		else 
  		{
   			dam_message(dam, ch, victim, attacktype);
  		}
 	} 
 	else if( attacktype != TYPE_SHOOT ) 
 	{
 		for( i = 0; i < MAX_MESSAGES; i++ ) 
 		{
  			if( fight_messages[i].a_type == attacktype ) 
  			{
   				nr = dice( 1, fight_messages[i].number_of_attacks );
   				for( j = 1, messages = fight_messages[i].msg; ( j < nr )&& messages ; j++ )
    				messages=messages->next;

   				if( !IS_NPC(victim) && (GET_LEVEL(victim) >= IMO) ) 
   				{
   					C_MESG( messages->god_msg.attacker_msg );
   					V_MESG( messages->god_msg.victim_msg );
   					R_MESG( messages->god_msg.room_msg );
   				} 
   				else if (dam != 0) 
   				{
    				if( GET_POS(victim) == POSITION_DEAD ) 
					{
   						C_MESG( messages->die_msg.attacker_msg );
   						V_MESG( messages->die_msg.victim_msg );
   						R_MESG( messages->die_msg.room_msg );
    				} 
					else 
					{
   						C_MESG( messages->hit_msg.attacker_msg );
   						V_MESG( messages->hit_msg.victim_msg );
   						R_MESG( messages->hit_msg.room_msg );
    				}
   				} 
   				else 
   				{
   					C_MESG( messages->miss_msg.attacker_msg );
   					V_MESG( messages->miss_msg.victim_msg );
   					R_MESG( messages->miss_msg.room_msg );
   				}
  			}
 		}
 	}

 	switch (GET_POS(victim)) 
 	{
  	case POSITION_MORTALLYW:
   		act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0,0,TO_ROOM);
   		act("You are mortally wounded, and will die soon, if not aided.", FALSE, victim, 0,0,TO_CHAR);
   		break;
  	case POSITION_INCAP:
   		act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
   		act("You are incapacitated an will slowly die, if not aided.", FALSE, victim, 0, 0, TO_CHAR);
   		break;
  	case POSITION_STUNNED:
   		act("$n is stunned, but could regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
   		act("You're stunned, but could regain consciousness again.", FALSE, victim, 0, 0, TO_CHAR);
   		break;
  	case POSITION_DEAD:

  		if( dam >= PART_CORPSE_DAM && ch->equipment[WIELD] )
  		{
  			if( attacktype == TYPE_SLASH )
  			{
  				act("$n chops $N with $s DREADFUL $p!! Ah~~~ Bloody!!", 
  					1, ch, ch->equipment[WIELD], victim, TO_ROOM );
  				act("You brutally chop $N to pieces with your DREADFUL $p. Ah~~~ Bloody!!", 
  					1, ch, ch->equipment[WIELD], victim, TO_CHAR );
				cry = 1;
  			}
  			if( attacktype == TYPE_CRUSH )
  			{
  				act("$n breaks $N's head down with $s DREADFUL $p!! Ah~~~ Bloody!!", 
					1, ch, ch->equipment[WIELD], victim, TO_ROOM );
  				act("You brutally crush $N's head down with your DREADFUL $p. Ah~~~ Bloody!!", 
  					1, ch, ch->equipment[WIELD], victim, TO_CHAR );
				cry = 1;
  			}
  			if( attacktype == TYPE_PIERCE )
  			{
  				act("$n penetrates $N's body"
					" and makes a hole in $N's body with $s DREADFUL $p!! Ah~~~ Bloody!!", 
					1, ch, ch->equipment[WIELD], victim, TO_ROOM );
  				act("You penetrate $N's body with your DREADFUL $p. Ah~~~ Bloody!!", 
  					1, ch, ch->equipment[WIELD], victim, TO_CHAR );
				cry = 1;
  			}
  			if( cry && victim->level >= 40 )
  			senddf( 0, 0, "%s crys, 'Aaaaaaaaaack, %s!! You shall get what i got!'", 
  						IS_NPC(victim) ? victim->moved : victim->name,
  						IS_NPC(ch) ? ch->moved : ch->name );
  		}
  		
   		act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
   		act("You are dead!  Sorry...", FALSE, victim, 0, 0, TO_CHAR);
   		break;
  	default:  /* >= POSITION SLEEPING */
   		max_hit=hit_limit(victim);
   		if( dam > (max_hit/5) )	
   			act("That Really did HURT!",FALSE, victim, 0, 0, TO_CHAR);
   		if (GET_HIT(victim) < (max_hit/5))
   		{
    		act("You wish that your wounds would stop BLEEDING that much!",	FALSE,victim,0,0,TO_CHAR);
    		if( (IS_NPC(victim) && IS_SET(victim->act, ACT_WIMPY))
     			||(!IS_NPC(victim) && IS_SET(victim->act, PLR_WIMPY)) )
	 		{
   				do_flee(victim, "", 0);
   	 		}	
   		}
   		break;  
 	}

 	if( !IS_NPC(victim) && !(victim->desc) && dam ) do_flee(victim, "", 0);

 	if( GET_POS(victim) < POSITION_MORTALLYW )
  		if( ch->fight == victim )
   			stop_fighting(ch);

 	if( !AWAKE(victim) )
  		if( victim->fight )
   			stop_fighting(victim);

 	if( GET_POS(victim) == POSITION_DEAD ) 
 	{
  		if( IS_NPC(victim) )
  		{
   			if( IS_AFFECTED(ch, AFF_GROUP) ) group_gain(ch, victim);
   			else 
   			{
    			exp = GET_EXP(victim)/2;
    			exp = MAX(exp, 1);
    			gain_exp( ch, exp );
    			change_alignment(ch, victim);
   			}
  		}

		if( GET_LEVEL(victim) >= 40 || !IS_NPC(victim) ) 
		{
			log( "%s killed by %s at %s", 
							GET_NAME(victim), (IS_NPC(ch) ? ch->moved : GET_NAME(ch) ),
							world[victim->in_room].name );
			senddf( 0, 41, "===> %s killed by %s at %s", 
							GET_NAME(victim), (IS_NPC(ch) ? ch->moved : GET_NAME(ch) ),
							world[victim->in_room].name );
		}

    	if( !IS_NPC(victim) )
    	{
        	wipe_stash( GET_NAME(victim), 1 );
        	loose_exp_die( victim );
    	}

		if( IS_AFFECTED( ch, AFF_DAZZLE ) ) attacktype = SKILL_DAZZLE;

    	kill_char( victim, ch->level, dam, attacktype );
 	}
}

void hit( charType *ch, charType *victim, int type )
{
 	objectType	*		wielded = 0;
 	int 				w_type = TYPE_HIT;
 	int 				victim_ac, calc_thaco;
 	int 				dam, prf, gr_num;
 	int 				hitdice;


 	if( !ch || !victim )
 	{
   		DEBUG("hit> called with NULL pointer. (%s vs %s)", 
   						ch ? ch->name : "Null", victim ? victim->name : "Null" );
   		return;
   	}

 	if( ch->in_room != victim->in_room )
 	{
   		DEBUG("hit> %s vs %s <- not same room??", ch->name, victim->name );
   		return;
   	}

 	if( GET_POS(victim) <= POSITION_DEAD )
 	{
   		DEBUG("hit>  %s vs %s <- Dead??", ch->name, victim->name );
   	}

	prf = 0;

	if( GET_LEVEL(victim) > (GET_LEVEL(ch) + 10) )
	{
  		if( number( 1, 33 ) < 3 ) prf = 1;
  	}

	if(  IS_AFFECTED( victim, AFF_MIRROR_IMAGE) 
      && ( 3 * number(1,GET_LEVEL(ch)) < number(1,GET_LEVEL(victim))) )
  		prf=2;

	if( victim->skills[SKILL_PARRY] )
	{
  		if( victim->skills[SKILL_PARRY] >
    		number(1,400) - (GET_LEVEL(victim)-GET_LEVEL(ch)) )
  			prf = 1;
	}

	if( IS_AFFECTED( victim, AFF_DAZZLE ) && number( 1, 3 ) == 1 ) prf = 4;

	if( prf )
	{
 		if( prf == 1 )
 		{
  			send_to_char_han("You parry successfully.\n\r",
    							"������ ���߽��ϴ�.\n\r",victim);
  			acthanbrief("$n parries successfully.", "$n���� ������ ���մϴ�.",
    							"$n parries.", "$n���� ����.", FALSE, victim,0,0,TO_ROOM);
 		}
 		else if( prf == 2 )
 		{
  			send_to_char_han("You hit illusion. You are confused.\n\r",
    							"����� �׸���(?)�� Ĩ�ϴ�.\n\r",ch);
  			acthanbrief("$n hits illusion, looks confused.",
    					"$n���� �׸���(?)�� Ĩ�ϴ�.",
    					"$n hits illusion.",
    					"$n���� �׸��ڸ� ħ.", FALSE, ch,0,0,TO_ROOM );
 		} 
 		else if( prf == 3 ) 
 		{
  			send_to_char_han("You blocks his attack.\n\r",
    							"������ ���ҽ��ϴ�.", victim);
  			acthan("$n blocks.", "$n���� ������ �����ϴ�.", FALSE, victim,0,0,TO_ROOM);
 		}
 		else if( prf == 4 ) 
 		{
  			acthan("$n failed to locate $N.", "$n���� $N���� ã�� ���մϴ�.", 
  					FALSE, ch, 0, victim, TO_NOTVICT);
  			acthan("$n failed to locate you.", "$N���� ����� ã�� ���մϴ�.", 
  					FALSE, ch, 0, victim, TO_VICT);
  			acthan("You failed to locate $N.", "����� $N���� ����ִ��� ��ã���ϴ�.", 
  					FALSE, ch, 0, victim, TO_CHAR);
 		}
	}
	else 
	{
 		if( ch->equipment[WIELD] && (ch->equipment[WIELD]->type == ITEM_WEAPON)) 
 		{
  			wielded = ch->equipment[WIELD];

  			switch( wielded->value[3] ) 
  			{
  			case 0  : case 1 :
   			case 2  : w_type = TYPE_WHIP; break;
   			case 3  : w_type = TYPE_SLASH; break;
   			case 4  : case 5 :
   			case 6  : w_type = TYPE_CRUSH; break;
   			case 7  : w_type = TYPE_BLUDGEON; break;
   			case 8  : case 9 :
   			case 10 : w_type = TYPE_STING; break;
   			case 11 : w_type = TYPE_PIERCE; break;
   			default : w_type = TYPE_HIT; break;
  			}
 		} 
 		else 
 		{
  			if( IS_NPC(ch) && (ch->attack_type >= TYPE_HIT) )
   				w_type = ch->attack_type;
  			else
   				w_type = TYPE_HIT;
 		}

 		/* Calculate the raw armor including magic armor */
 		/* The lower AC, the better                      */

 		if (!IS_NPC(ch))
  			calc_thaco  = thaco[(int)GET_CLASS(ch)-1][(int)GET_LEVEL(ch)];
 		else
  		/* THAC0 for monsters is set in the HitRoll */
  			calc_thaco = 20;

 		calc_thaco -= str_apply(ch)->tohit;
 		calc_thaco -= GET_HITROLL(ch);
 		calc_thaco -= ( gr_num = sum_gr_member( ch ));

 		hitdice = number( 1, 20 );

 		victim_ac  = GET_AC(victim)/10;

 		if( AWAKE(victim) ) victim_ac += dex_app[GET_DEX(victim)].defensive;

 		victim_ac = MAX( -27, victim_ac );  /* -27 is lowest */

 		if( AWAKE(victim) && ((hitdice==1) || ((calc_thaco-hitdice) > victim_ac))) 
 		{
  			if (type == SKILL_BACKSTAB)
   				damage(ch, victim, 0, SKILL_BACKSTAB);
  			else
   				damage(ch, victim, 0, w_type);
 		} 
 		else 
 		{
  			dam  = str_apply(ch)->todam;
  			dam += GET_DAMROLL(ch);
  			dam += gr_num ;
		
  			if( !wielded ) 
  			{
    			dam += dice(ch->ndice, ch->sdice);
  			} 
  			else 
  			{
   				dam += dice(wielded->value[1], wielded->value[2]);
  			}

  			if( GET_POS(victim) < POSITION_FIGHTING )
   				dam *= 1+(POSITION_FIGHTING-GET_POS(victim))/3;
  				/* Position  sitting  x 1.33 */
  				/* Position  resting  x 1.66 */
  				/* Position  sleeping x 2.00 */
  				/* Position  stunned  x 2.33 */
  				/* Position  incap    x 2.66 */
  				/* Position  mortally x 3.00 */

			if( GET_ADD( ch ) )
			{
				dam += (dam * number( 1, GET_ADD( ch ) * 2 )) / 130;
			}

  			dam = MAX(1, dam);  /* Not less than 0 damage */

  			if( IS_AFFECTED( ch, AFF_FEAR ) ) dam -= dam / 5;

  			if (type == SKILL_BACKSTAB) 
  			{
   				dam *= backstab_mult[(int)GET_LEVEL(ch)];
   				dam = dam * (GET_DEX(ch)) / 25;
   				damage(ch, victim, dam, SKILL_BACKSTAB);
  			} 
  			else
   				damage(ch, victim, dam, w_type);
 		}
	}
}

void perform_violence( void )
{
 	charType 	*	ch, * prev;
 	int 			i, j;
 	objectType  *	weapon;

 	for( ch = combat_list; ch; ch = combat_next_dude )
 	{
  		combat_next_dude = ch->next_fighting;

  		if( !ch->fight ) 
  		{
  			if( ch == combat_list ) combat_list = ch->next_fighting;
  			else
  			{
  				for( prev = combat_list; prev && prev->next_fighting != ch; prev= prev->next_fighting )
  				;

  				if( !prev )
  				{
  					FATAL( "perform_violence> combat_list corrupted." );
  				}

  				prev->next_fighting = ch->next_fighting;
  			}

  			ch->next_fighting = 0;	

  			DEBUG( "perform_violence> not fighting dude(%s) found.", ch->name );
  			continue;
  		}

  		if( !AWAKE(ch) || !is_fighting(ch) ) 
  		{
  			stop_fighting( ch );
  			continue;
  		}

		if( IS_AFFECTED( ch, AFF_TIME_STOP ) || IS_AFFECTED( ch->fight, AFF_TIME_STOP ) ) continue;

  		if( !IS_NPC(ch) ) i = calc_hit_rate( ch );
  		else
  		{
  			i = GET_LEVEL( ch ) / 7 + 1;
  			if( IS_SET(ch->act, ACT_THIEF) )   i = GET_LEVEL( ch ) / 8 + 1;
  			if( IS_SET(ch->act, ACT_CLERIC) )  i = GET_LEVEL( ch ) / 10 + 1;
  			if( IS_SET(ch->act, ACT_MAGE) )    i = GET_LEVEL( ch ) / 12 + 1;
  			if( IS_SET(ch->act, ACT_FIGHTER )) i++;
  		}

		if( IS_AFFECTED( ch, AFF_BERSERK ) )  i += number( 1, 3 );
		if( IS_AFFECTED( ch, AFF_FEAR  ) )  i -= number( 1, 3 );

		weapon = ch->equipment[WIELD];

		if( weapon && weapon->value[0] > 0 && IS_SET( weapon->extra, ITEM_MAGIC ) )
			magic_weapon_hit(ch, ch->fight, weapon );

		for( j = 0; j < i; j++ )
		{
	   		if( is_fighting( ch ) ) hit(ch, ch->fight, TYPE_UNDEFINED);
			else
			{
				stop_fighting(ch); break;
			}
   		}
 	}
}
