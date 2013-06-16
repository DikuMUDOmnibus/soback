#include <stdio.h>

#include "defines.h"
#include "character.h"
#include "object.h"
#include "world.h"
#include "mobile.h"
#include "limits.h"
#include "utils.h"
#include "magics.h"
#include "comm.h"
#include "fight.h"
#include "sockets.h"
#include "find.h"
#include "interpreter.h"
#include "variables.h"
#include "weather.h"
#include "affect.h"

#define SPELL_LEARN_MAX	120

int calc_used_value( objectType * obj )
{
	int		diff;

	diff = obj->limit;
	diff -= (obj->limit - obj->status) * 2 / 7;

	return (obj->cost * diff / 100);
}

int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
  	if      (age <= 17)		return (p0);                                /* <= 17  */
  	else if (age <= 30) 	return (int) (p0+(((age-17)*(p1-p0))/12));  /* 18..29 */
  	else if (age <= 44) 	return (int) (p1+(((age-30)*(p2-p1))/15));  /* 30..44 */
  	else if (age <= 59) 	return (int) (p2+(((age-45)*(p3-p2))/15));  /* 45..59 */
  	else if (age <= 79) 	return (int) (p3+(((age-60)*(p4-p3))/15));  /* 60..74 */
  	else if (age <= 90) 	return (int) (p4+(((age-60)*(p5-p4))/15));  /* 75..89 */
  	else if (age <= 90) 	return (int) (p5+(((age-60)*(p6-p5))/10));  /* 90..99 */
  	else 					return (p6);                                /* >= 100 */
}

int mana_limit( charType  * ch )
{
  	int 	max;

  	if( !IS_NPC(ch) ) 
  	{
  		if( ch->class == CLASS_MAGIC_USER || ch->class == CLASS_CLERIC ) 
  			max = (ch->max_mana) + graf((int)age(ch).year, 70,200,320,420,320,200,70);
  		else
  			max = (ch->max_mana) + graf((int)age(ch).year, 70,150,200,250,200,150,70);
  	}
  	else			  max = (ch->max_mana);

  	return(max);
}

int hit_limit( charType  * ch )
{
  	int max;

  	if (!IS_NPC(ch)) 
  	{
  		if( ch->class == CLASS_WARRIOR ) 
  			max = ch->max_hit + graf((int)age(ch).year, 20,200,320,420,320,200,20);
  		else
  			max = ch->max_hit + graf((int)age(ch).year, 20,70,90,120,90,50,30);
  		return max;
  	}
  	else
  		return ch->max_hit;
}

int move_limit( charType * ch )
{
  	int max;

  	if( !IS_NPC(ch) )
  	{
  		if( ch->class == CLASS_THIEF ) 
  			max = (ch->max_move) + graf((int)age(ch).year, 50,200,320,420,320,200,50);
  		else
  			max = (ch->max_move) + graf((int)age(ch).year, 50,120,180,220,170,90,50);
  	}
  	else 			  max = (ch->max_move);

  	return (max);
}

int hit_gain( charType * ch )
{
  	int  gain, c;

  	c = GET_CON(ch) + GET_REGEN(ch);

  	if( ch->in_room == room_meditation ) c += 5;

  	if( IS_NPC(ch) ) 
  	{
    	gain = GET_LEVEL(ch) * 4;
  	} 
  	else 
  	{
    	gain = graf( age(ch).year, 3*c, 4*c, 7*c, 9*c, 5*c, 3*c, 2*c);

    	switch (GET_POS(ch)) 
		{
      		case POSITION_SLEEPING: gain += (gain>>1); break;
      		case POSITION_RESTING:  gain += (gain>>2); break;
      		case POSITION_SITTING:  gain += (gain>>3); break;
    	}

    	if( (GET_CLASS(ch) == CLASS_MAGIC_USER) || (GET_CLASS(ch) == CLASS_CLERIC) )
      		gain >>= 1;
  		if( GET_CLASS(ch) == CLASS_WARRIOR ) gain = gain*3/2;
  	}

  	if( IS_AFFECTED(ch,AFF_POISON) ) 
  	{
  		gain >>= 1;
  		damage( ch, ch, 2, SPELL_POISON );
  	}
  	if( !IS_NPC(ch) && ((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))) gain >>= 1;

  	return (gain);
}

int mana_gain( charType * ch )
{
  	int 	gain, i;

  	i = GET_INT(ch) + GET_REGEN(ch);

  	if( ch->in_room == room_library ) i += 5;

  	if( IS_NPC(ch) ) 
  	{
    	gain = GET_LEVEL(ch) * 3;
  	} 
  	else 
  	{
    	gain = graf( age(ch).year, i*3/2, i*2, i*3, i*4, i*3, i*2, i*3/2 );

    	switch (GET_POS(ch)) 
		{
      		case POSITION_SLEEPING:	gain += (gain>>1); break;
      		case POSITION_RESTING:  gain += (gain>>2); break;
      		case POSITION_SITTING:  gain += (gain>>3); break;
    	}
    	if((ch->class == CLASS_MAGIC_USER) || (ch->class ==CLASS_CLERIC)) gain = gain *3/2;
  	}

  	if( IS_AFFECTED(ch,AFF_POISON) ) gain >>= 2;
  	if( !IS_NPC(ch) && ((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))) gain >>= 1;
 
  	return (gain);
}

int move_gain( charType * ch )
{
  	int gain, d;

  	d = GET_DEX(ch) + GET_REGEN(ch);

  	if( ch->in_room  == room_audience ) d += 5;

  	if( IS_NPC(ch) ) 
  	{
    	return(GET_LEVEL(ch)) * 3;  
  	} 
  	else 
  	{
    	gain = graf( (int)age(ch).year, 3*d, 4*d, 5*d, 7*d, 5*d, 4*d, 2*d );

    	switch( GET_POS(ch) ) 
		{
      		case POSITION_SLEEPING: gain += (gain>>1); break;
      		case POSITION_RESTING: 	gain += (gain>>2); break;
      		case POSITION_SITTING: 	gain += (gain>>3); break;
    	}
  		if( GET_CLASS(ch) == CLASS_THIEF ) gain = gain*3/2;
  	}

  	if( IS_AFFECTED(ch,AFF_POISON) ) gain >>= 2;
  	if( !IS_NPC(ch) && ((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))) gain >>= 1;

  	return (gain);
}

void advance_level( charType * ch )
{
  	int add_hp, add_mana, add_move, i;

  	add_hp   = con_app[(int)ch->base_stat.con].hitp;
  	add_mana = wis_app[(int)ch->base_stat.wis].bonus;
  	add_move = dex_app[(int)ch->base_stat.dex].reaction;

  	switch(GET_CLASS(ch)) 
  	{
    	case CLASS_MAGIC_USER : 
			{
				add_hp += number(4, 8);
				add_mana += number(3, 8);
				add_move += number(4, 7);
    		} break;

    	case CLASS_CLERIC : 
			{
				add_hp += number(5, 10);
				add_mana += number(4, 6);
				add_move += number(4, 7);
    		} break;

    	case CLASS_THIEF : 
			{
      			add_hp += number(7,13);
      			add_mana += number(1, 3);
      			add_move += number(6, 9);
    		} break;

    	case CLASS_WARRIOR : 
			{
      			add_hp += number(10,15);
				add_mana += number(1, 3);
				add_move += number(5, 8);
    		} break;
  	}
  	GET_AC(ch)--;
  	ch->max_hit  += MAX(1, add_hp);
  	ch->max_mana += MAX(1, add_mana);
  	ch->max_move += MAX(1, add_move);

  	if( ch->prNr < SPELL_LEARN_MAX ) 
  	{
    	if (GET_CLASS(ch) == CLASS_MAGIC_USER || GET_CLASS(ch) == CLASS_CLERIC)
      		ch->prNr += MAX(2, wis_app[(int)ch->base_stat.wis].bonus);
    	else
      		ch->prNr += MIN(2, MAX(1, wis_app[(int)ch->base_stat.wis].bonus) );
    }
  	if( GET_LEVEL(ch) > IMO )
    	for (i = 0; i < 3; i++)
      		ch->conditions[i] = -1;
}  

void gain_exp( charType * ch, int gain )
{
  	if( IS_NPC(ch) || ((GET_LEVEL(ch)<IMO) && (GET_LEVEL(ch) > 0)) ) 
  	{
    	if( gain > 0 ) 
		{
      		gain = MIN(GET_LEVEL(ch) * 250000, gain);
      		GET_EXP(ch) += gain;
    	}
		else
		{
			gain = MIN( ch->exp, (-1 * gain) );
			ch->exp -= gain;
		}
	}
}

void loose_exp_die( charType * ch )
{
	static	int	 table[] =
	{
		0,
		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
		1,	    2,		4, 	    8,	   10,	   20,	   30,     50,     75,    100,	
	  120,	  140,	  165,    190,	  230,	  270,	  310,    350,    375,    400,	
	  470,	  530,	  600,    690,	  780,	  880,   1010,   1200,   1400,	 1600,	
		0,		0,		0,		0
	};	
		
	int		loose;

	if( loose = table[ch->level] * 10000,  loose == 0 ) return;
			
	if( GET_EXP(ch) < loose )
	{
	 	ch->max_move--;
		ch->max_mana--;
		ch->max_hit--;
      	ch->exp = 0;
	}	
   	else 
	{
		ch->exp -= loose;
  	}
}

void loose_exp_flee( charType *ch, int loose )
{
  	if( IS_NPC(ch) || ((GET_LEVEL(ch)<IMO) && (GET_LEVEL(ch) > 0)) ) 
  	{
    	if( loose <= 0 ) return;

		loose = MIN( ch->exp, loose );

    	if( ch->exp -= loose, ch->exp == 0 ) 
		{
			switch( number( 1, 3 ) )
			{
				case 1 : 	ch->max_move--; break;
				case 2 :	ch->max_mana--; break;
				case 3 :	ch->max_hit--;  break;
			}
    	}
  	}
}

void gain_exp_and_adv( charType *ch, int gain )
{
  	int 	i;
  	bool 	is_altered = FALSE;

  	if( !IS_NPC(ch) ) 
  	{
    	if( gain > 0) 
		{
      		GET_EXP(ch) += gain;
      		for( i = 0; (i <=(IMO+3)) && (titles[GET_CLASS(ch)-1][i].exp <= GET_EXP(ch));i++)
	  		{
        		if(i > GET_LEVEL(ch)) 
				{
          			send_to_char("You raise a level.\n\r",ch);
          			GET_LEVEL(ch) = i;
          			advance_level(ch);
          			is_altered = TRUE;
        		}
      		}
    	}
    	if( gain < 0 ) 			GET_EXP(ch) += gain;
    	if( GET_EXP(ch) < 0 ) 	GET_EXP(ch) = 0;
  	}
  	if( is_altered ) set_title(ch);
}

void gain_condition( charType * ch, int condition, int value )
{
  	bool intoxicated;

  	if( GET_COND(ch, condition)== -1 ) return;

  	intoxicated=(GET_COND(ch, DRUNK) > 0);

  	GET_COND(ch, condition) += value;
  	GET_COND(ch, condition)  = MAX(0,GET_COND(ch,condition));
  	GET_COND(ch, condition)  = MIN(36,GET_COND(ch,condition));

  	if( GET_COND(ch,condition) ) return;

  	switch( condition )
  	{
    	case FULL : 	sendf( ch, "You are hungry." ); 	return; 
		case THIRST : 	sendf( ch, "You are thirsty." ); 	return;
    	case DRUNK : 	
					if(intoxicated)
        				sendf( ch, "You are now sober.r" );	
  	}
}

int saves_spell( charType * ch, charType * vict, int save_type )
{
 	int save, diff, base, all, ran;

	static int saving[ 41 ] = 
	{
		69, 68, 67, 64, 57, 50, 44, 38, 32, 27, 
		23, 23, 21, 20, 19, 18, 17, 17, 17,	16,
		16, 15, 15, 14, 14, 13, 13, 12, 12, 11,
		11, 10, 10,  9,  9,  8,  8,  7,  7,  6,
		 6
	};

	if( IMPL( ch ) ) return 0;

 	save = vict->saving_throw[save_type];
 	diff = GET_LEVEL(ch) - GET_LEVEL(vict) + 20;

 	if( diff < 0  ) diff = 0;
 	if( diff > 40 ) diff = 40;

	base = saving[ 40 - diff ];

 	all = save + base;
 	all = all < 6 ? 6 : all;

 	ran = number( 0, 70 );

 	return ( all < number(0,70) );
}

int calc_hit_rate( charType *ch )
{
	int	div, mod = 25, ret = 0;

	div  = GET_DEX(ch) + GET_LEVEL( ch );

	switch( GET_CLASS(ch ) )
	{
		case CLASS_WARRIOR  : 	mod = 12; break;
		case CLASS_THIEF    : 	mod = 19; break;
		case CLASS_CLERIC   : 	mod = 22; break;
		case CLASS_MAGIC_USER : mod = 25; break;
	}

	if( ch->level > IMO ) mod = 9;

	ret += ((div % mod) >= number( 1, mod ));
	ret += div / mod;

	return (ret ? ret : 1 );
}

int int_rate( charType * ch, int input, int level )
{
	int ret, lev = (GET_LEVEL(ch) - level );
	
	ret = input * GET_INT(ch);
	ret += ret * (lev*2)/100;

	return (ret / 10 );
}

int wis_rate( charType * ch, int input, int level )
{
	int ret, lev = (GET_LEVEL(ch) - level );
	
	ret = input * GET_WIS(ch);
	ret += ret * (lev*2)/100;

	return (ret / 10 );
}

int dex_rate( charType * ch, int input, int level )
{
	int ret, lev = (GET_LEVEL(ch) - level );
	
	ret = input * GET_DEX(ch);
	ret += ret * (lev*2)/100;

	return (ret / 10 );
}

void change_alignment(charType *ch, charType *victim)
{
	/* When ch kills victim , victime's align effect killer's align.
	*/
 	int  al;
 
 	al = (9*GET_ALIGN(ch)-GET_ALIGN(victim))/10;
 	al = (al-GET_ALIGN(ch))/20;

	if( al == 0 )
	{
		if( GET_ALIGN( victim ) > 0 ) al = -1;
		else						   al =  1;
	}

 	GET_ALIGN(ch) +=al;

 	if(GET_ALIGN(ch) < -1000) GET_ALIGN(ch) = -1000;
 	if(GET_ALIGN(ch) >  1000) GET_ALIGN(ch) =  1000;
}

int can_see( charType * ch, charType * sub )
{
    if( OMNI(ch) && (IS_NPC(sub) || (GET_LEVEL(ch) >= GET_LEVEL(sub)))) return TRUE ;
	if( IS_AFFECTED( ch, AFF_BLIND ) ) return FALSE;
    if( !IS_LIGHT(ch->in_room) && !IS_AFFECTED(ch,AFF_INFRAVISION) ) return FALSE;
	if( IS_AFFECTED( sub, AFF_INVISIBLE ) && !IS_AFFECTED( ch, AFF_DETECT_INVISIBLE ) ) return FALSE;
    if( !IS_NPC(sub) )
	{
		if( IS_SET((sub)->act, PLR_WIZINVIS) ) return FALSE;
		if( IS_AFFECTED( sub, AFF_MORPETH )  ) return FALSE;
		if( IS_AFFECTED( sub, AFF_AMBUSH  )  ) return FALSE;
	}

    return TRUE; 
}

int can_see_obj( charType * ch, objectType * obj )
{
	if( OMNI( ch ) ) return TRUE;
	if( IS_AFFECTED( ch, AFF_BLIND ) ) return FALSE;
	if( !IS_LIGHT( obj->in_room ) && !IS_AFFECTED((ch), AFF_INFRAVISION ) ) return FALSE;
	if( IS_SET( obj->extra, ITEM_INVISIBLE) && !IS_AFFECTED( ch, AFF_DETECT_INVISIBLE ) ) return FALSE;

	return TRUE;
}

int can_go( charType * ch, int dir )
{
	directionType	*	door;
	charType		*	tmp;

	if( door = world[ch->in_room].dirs[dir], !door ) return FALSE;
	if( door->to_room == NOWHERE ) return FALSE;
	if( door->exit_info & (EX_CLOSED | EX_LOCKED) ) return FALSE;
	if( IS_NPC( ch ) && (world[door->to_room].flags & NO_MOB) ) return FALSE;

	for( tmp = world[door->to_room].people; tmp; tmp = tmp->next_in_room )
		if( IS_AFFECTED( tmp, AFF_TIME_STOP ) ) return FALSE;
	
	return TRUE;
}

struct str_app_type * str_apply( charType * ch )
{
	static struct str_app_type		app;

	app = str_app[(int)ch->temp_stat.str];

	if( ch->temp_stat.str_add == 100 )	app.tohit += 3, app.todam += 5, app.carry_w += 50;
	else
	if( ch->temp_stat.str_add >=  91 )	app.tohit += 2, app.todam += 3, app.carry_w += 40;
	else
	if( ch->temp_stat.str_add >=  76 )	app.tohit += 2, app.todam += 2, app.carry_w += 30;
	else
	if( ch->temp_stat.str_add >=  51 )	app.tohit += 2, app.todam += 2, app.carry_w += 20;
	else
	if( ch->temp_stat.str_add >=  26 )	app.tohit += 1, app.todam += 2, app.carry_w += 15;
	else
	if( ch->temp_stat.str_add >=  01 )	app.tohit += 1, app.todam += 1, app.carry_w += 10;

	return &app;
}

int can_carry_number( charType * ch )
{
	return 1 + (ch->temp_stat.dex/2) + (ch->level/2);
}

int can_carry_weight( charType * ch )
{
	return str_apply(ch)->carry_w;
}

int can_carry( charType * ch, objectType * obj )
{
	if( ch->carry_weight + obj->weight > str_apply(ch)->carry_w ) 	return FALSE;
	if( ch->carry_items + 1 > can_carry_number( ch ) ) 				return FALSE;

	return TRUE;
}

int can_get( charType * ch, objectType * obj )
{
	if( !can_see_obj( ch, obj )  ) 	return FALSE;
	if( !(obj->wear & ITEM_TAKE) ) 	return FALSE;
	if( OMNI( ch ) ) 				return TRUE;

	return TRUE;
}
