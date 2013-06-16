#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "find.h"
#include "limits.h"
#include "fight.h"
#include "variables.h"
#include "strings.h"
#include "affect.h"
#include "transfer.h"
#include "specials.h"

void weight_change_object(objectType *obj, int weight)
{
  	objectType 	*	tmp_obj;
  	charType 	*	tmp_ch;

  	if( obj->in_room != NOWHERE) 
  	{
    	obj->weight += weight;
  	} 
  	else if((tmp_ch = obj->carried_by))
  	{
    	obj_from_char(obj);
    	obj->weight += weight;
    	obj_to_char(obj, tmp_ch);
  	} 
  	else if((tmp_obj = obj->in_obj))
  	{
    	obj_from_obj(obj);
    	obj->weight += weight;
    	obj_to_obj(obj, tmp_obj);
  	} 
  	else 
  	{
    	log("Unknown attempt to subtract weight from an object.");
  	}

  	obj->weight = MAX( objects[obj->nr].weight, obj->weight );
}

void do_drink(charType *ch, char *argument, int cmd)
{
  	char 				buf[100];
  	objectType 	*		temp;
  	affectType 			af;
  	int 				amount;

  	oneArgument(argument,buf);

	if( world[ch->in_room].virtual == ROOM_TEMPLE )
	{
		if( temp = find_obj_room( ch, buf ), !temp ) temp = find_obj_inven( ch, buf );
	}
	else
	{
		if( temp = find_obj_inven( ch, buf ), !temp ) temp = find_obj_room( ch, buf );
	}

 	if(!temp)
	{
		act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
		return;
  	}
	if (temp->type != ITEM_DRINKCON)
	{
		act("You can't drink from that!",FALSE,ch,0,0,TO_CHAR);
		return;
	}

	if(GET_COND(ch,DRUNK)>20) /* The pig is drunk */
	{
		act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
		act("$n tried to drink but missed $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
		return;
	}

	if((GET_COND(ch,FULL)>32)&&(GET_COND(ch,THIRST)>0)) /* Stomach full */
	{
		act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
		return;
	}

	if( temp->value[1] <= 0 )  /* empty */
	{
  		act("It's empty already.",FALSE,ch,0,0,TO_CHAR);
  	}
  	else
	{  
	  sprintf(buf,"$n drinks %s from $p",drinks[temp->value[2]]);
	  act(buf, TRUE, ch, temp, 0, TO_ROOM);
	  sprintf(buf,"You drink the %s.\n\r",drinks[temp->value[2]]);
	  send_to_char(buf,ch);

		if( temp->value[2] == LIQ_HOLY_WATER )
		{
			if( IS_AFFECTED( ch, AFF_CURSE ) )
			{
				affect_from_char( ch, SPELL_CURSE );	
				sendf( ch, "\n\rHoly water blesses you.\n\rYou feel better." );
			}
		}
		else if( temp->value[2]==LIQ_NECTAR)
			spell_cure_critic(GET_LEVEL(ch),ch,"",SPELL_POTION,ch,0);
		else if( temp->value[2]==LIQ_GOLDEN_NECTAR)
			spell_heal(GET_LEVEL(ch),ch,"",SPELL_POTION,ch,0);

/*    	if(drink_aff[temp->value[2]][DRUNK] > 0 )
			amount = (25-GET_COND(ch,THIRST))/drink_aff[temp->value[2]][DRUNK];
	  	else
			amount = number(3,10);   
*/
		amount = min( 5, temp->value[1] );

		weight_change_object(temp, -amount);  /* Subtract amount */

		gain_condition(ch,DRUNK,(int)((int)drink_aff[temp->value[2]][DRUNK]*amount)/4);
		gain_condition(ch,FULL,(int)((int)drink_aff[temp->value[2]][FULL]*amount)/4);
		gain_condition(ch,THIRST,(int)((int)drink_aff[temp->value[2]][THIRST]*amount)/4);

		if( GET_COND(ch,DRUNK)  > 20 ) 	act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);
		if( GET_COND(ch,THIRST) > 32 ) 	act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);
		if( GET_COND(ch,FULL) > 32 ) 	act("You are full.",FALSE,ch,0,0,TO_CHAR);

		switch( temp->value[3] ) 
		{
		case 1 :    /* The shit was poisoned ! */
		  act("Ooups, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
		  act("$n chokes and utters some strange sounds.",
			 TRUE,ch,0,0,TO_ROOM);
		  af.type = SPELL_POISON;
		  af.duration = amount*3;
		  af.modifier = 0;
		  af.location = APPLY_NONE;
		  af.bitvector = AFF_POISON;
		  affect_join(ch,&af, FALSE, FALSE);
		  break;
		case 2 :    /* hit point restore */
		  act("You feel momentarily warm.",FALSE,ch,0,0,TO_CHAR);
		  acthan("$n blushes.", "$n´ÔÀÇ ¾ó±¼ÀÌ ºÓ¾îÁý´Ï´Ù.",
			TRUE,ch,0,0,TO_ROOM);
		  GET_HIT(ch) = MIN(GET_MAX_HIT(ch),GET_HIT(ch) + 
			dice(drink_aff[temp->value[2]][DRUNK], 8));
		  break;
		case 3 :    /* mana restore */
		  act("You feel momentarily warm.",FALSE,ch,0,0,TO_CHAR);
		  act("$n blushes.", TRUE,ch,0,0,TO_ROOM);
		  GET_MANA(ch) = MIN(GET_MAX_MANA(ch),GET_MANA(ch) + 
			dice(drink_aff[temp->value[2]][DRUNK], 7));
		  break ;
		case 4 :    /* move restore */
		  break;
		case 0  :    /* nothing */
		default : break ;
		}

		/* empty the container, and no longer poison. */
		temp->value[1] -= amount;
		if( !temp->value[1]) 
		{  /* The last bit */
			temp->value[2]=0;
			temp->value[3]=0;
			name_from_drinkcon(temp);
		}
  	}
}

void do_eat(charType *ch, char *argument, int cmd)
{
  	char 				buf[256];
  	objectType 		*	temp;
  	affectType 			af;

  	oneArgument(argument,buf);

  	if(!(temp = find_obj_inven( ch, buf )) ) 
  	{
    	sendf( ch, "You can't find it!" ); return;
  	}
  	if( (temp->type != ITEM_FOOD) && (GET_LEVEL(ch) < (IMO+1)) ) 
  	{
    	sendf( ch, "Your stomach refuses to eat that!?!" ); return;
  	}

  	if( GET_COND(ch,FULL) > 32 ) /* Stomach full */ 
  	{
    	sendf( ch, "You are too full to eat more!" ); return;
  	}

  	act("$n eats $p",TRUE,ch,temp,0,TO_ROOM);
  	act("You eat the $o.",FALSE,ch,temp,0,TO_CHAR);
  	gain_condition(ch,FULL,temp->value[0]);

  	if( GET_COND( ch, FULL ) > 32 )
    	act("You are full.",FALSE,ch,0,0,TO_CHAR);

  	if( (temp->value[3] == 1) && (GET_LEVEL(ch) < IMO) ) 
  	{
    	act("Ooups, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
    	act("$n coughs and utters some strange sounds.",FALSE,ch,0,0,TO_ROOM);
    	af.type = SPELL_POISON;
    	af.duration  = temp->value[0]*2;
    	af.modifier  = 0;
    	af.location  = APPLY_NONE;
    	af.bitvector = AFF_POISON;
    	affect_join(ch,&af, FALSE, FALSE);
  	}

  	if( temp->value[3] >= 2 ) 
  	{
    	act("Wow, you feel great !!",FALSE,ch,0,0,TO_CHAR);
    	act("$n looks better now.",FALSE,ch,0,0,TO_ROOM);
    	if( GET_HIT(ch) < 500 )
         	GET_HIT(ch) = MIN(GET_MAX_HIT(ch),GET_HIT(ch) + dice(2,4) * temp->value[3]);
    	else 
			GET_HIT(ch) = GET_HIT(ch)-dice(2,4) * temp->value[3];
    	if( GET_MANA(ch) < 500 )
         	GET_MANA(ch) = MIN(GET_MAX_MANA(ch),GET_MANA(ch) + dice(2,2) * temp->value[3]);
    	else 
			GET_MANA(ch) = GET_MANA(ch)-dice(2,2)*temp->value[3];
    	if( GET_MOVE(ch) < 500 )
         	GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch),GET_MOVE(ch) + dice(2,3) * temp->value[3]);
    	else 
			GET_MOVE(ch) = GET_MOVE(ch)-dice(2,3)*temp->value[3];
    	update_pos(ch) ;
  	}

  	if( objects[temp->nr].virtual == 2701 ) 
  	{
    	sendf( ch, "There is some scripts inside the cookie. It reads.." );
    	sendf( ch, "%s", select_fortune_cookie() );
    }
  	if( objects[temp->nr].virtual == 16012 ) 
  	{ /* Do-ra-ji */
    	act("Çá~ ±âºÐÀÌ ÁÁÀºµ¥ ?",FALSE,ch,0,0,TO_CHAR);
    	act("$n´Ô²²¼­ ±â¿îÀ» Â÷¸³´Ï´Ù.",FALSE,ch,0,0,TO_ROOM);
    	GET_MANA(ch) = MIN(GET_MAX_MANA(ch),GET_MANA(ch) + dice(10,5) );
    	update_pos(ch) ;
    }
  	if( objects[temp->nr].virtual == 16011 ) 
  	{ /* San-sam */
    	act("²ôÀ¸À¸¾ï !!",FALSE,ch,0,0,TO_CHAR);
    	act("$n´ÔÀÌ °©ÀÚ±â ÈûÀÌ ¼Ú³ª º¾´Ï´Ù.",FALSE,ch,0,0,TO_ROOM);
    	GET_HIT(ch)  = MIN(GET_MAX_HIT(ch), GET_HIT(ch)  + dice(20,10) );
    	GET_MANA(ch) = MIN(GET_MAX_MANA(ch),GET_MANA(ch) + dice(20,10) );
    	GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch),GET_MOVE(ch) + dice(20,10) );
    	update_pos(ch) ;
    }
  	extract_obj(temp, 1);
}

void do_junk(charType *ch, char *argument, int cmd)
{
  	char 			buf[100];
  	objectType 	*	temp;

  	oneArgument(argument,buf);

  	if( !(temp = find_obj_inven( ch, buf)) ) 
  	{
    	sendf( ch, "You can't find it!" ); return;
  	}
  	if( IS_SET(temp->extra, ITEM_NODROP) ) 
  	{
    	sendf( ch, "You can't junk it! Cursed?" ); return;
  	}
  	act("$n junks $p",TRUE,ch,temp,0,TO_ROOM);
  	act("You junk the $o.",FALSE,ch,temp,0,TO_CHAR);

  	extract_obj( temp, 1 );
}

void do_pour(charType *ch, char *argument, int cmd)
{
  	char arg1[MAX_STRING_LENGTH];
  	char arg2[MAX_STRING_LENGTH];
  	char buf[MAX_STRING_LENGTH];
  	objectType *from_obj;
  	objectType *to_obj;
  	int amount;

  	twoArgument(argument, arg1, arg2);

  	if(!*arg1) /* No arguments */
  	{
    	act("What do you want to pour from?",FALSE,ch,0,0,TO_CHAR);
    	return;
  	}

  	if(!(from_obj = find_obj_inven( ch, arg1)) )
  	{
    	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    	return;
 	} 

  	if( from_obj->type != ITEM_DRINKCON )
  	{
    	act("You can't pour from that!",FALSE,ch,0,0,TO_CHAR);
    	return;
  	}

  	if( from_obj->value[1] == 0 )
  	{
    	act("The $p is empty.",FALSE, ch,from_obj, 0,TO_CHAR);
    	return;
  	}

  	if( !*arg2 )
  	{
    	act("Where do you want it? Out or in what?",FALSE,ch,0,0,TO_CHAR);
    	return;
  	}

  	if( !stricmp(arg2,"out") )
  	{
    	act("$n empties $p", TRUE, ch,from_obj,0,TO_ROOM);
    	act("You empty the $p.", FALSE, ch,from_obj,0,TO_CHAR);

    	weight_change_object(from_obj, -from_obj->value[1]); /* Empty */

    	from_obj->value[1]=0;
    	from_obj->value[2]=0;
    	from_obj->value[3]=0;
    	name_from_drinkcon(from_obj);
    
    	return;
  	}

  	if( !(to_obj = find_obj_inven( ch, arg2)) )
  	{
    	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    	return;
  	}

  if(to_obj->type!=ITEM_DRINKCON)
  {
    act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  if((to_obj->value[1]!=0)&&
    (to_obj->value[2]!=from_obj->value[2]))
  {
    act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  if(!(to_obj->value[1]<to_obj->value[0]))
  {
    act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  if(from_obj == to_obj)
  {
    act("That would be silly.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  sprintf(buf,"You pour the %s into the %s.",
    drinks[from_obj->value[2]],arg2);
  send_to_char(buf,ch);

  /* New alias */
  if (to_obj->value[1]==0) 
    name_to_drinkcon(to_obj,from_obj->value[2]);

  /* First same type liq. */
  to_obj->value[2]=from_obj->value[2];

  /* Then how much to pour */
  from_obj->value[1]-= (amount=
    (to_obj->value[0]-to_obj->value[1]));

  to_obj->value[1]=to_obj->value[0];

  if(from_obj->value[1]<0)    /* There was to little */
  {
    to_obj->value[1]+=from_obj->value[1];
    amount += from_obj->value[1];
    from_obj->value[1]=0;
    from_obj->value[2]=0;
    from_obj->value[3]=0;
    name_from_drinkcon(from_obj);
  }

  /* Then the poison boogie */
  to_obj->value[3]=
    (to_obj->value[3]||from_obj->value[3]);

  /* And the weight boogie */

  weight_change_object(from_obj, -amount);
  weight_change_object(to_obj, amount);   /* Add weight */

  return;
}

void do_sip(charType *ch, char *argument, int cmd)
{
  affectType af;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  objectType *temp;

  oneArgument(argument,arg);

  if( !(temp = find_obj_inven( ch, arg )) )
  {
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(temp->type!=ITEM_DRINKCON)
  {
    act("You can't sip from that!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(GET_COND(ch,DRUNK)>10) /* The pig is drunk ! */
  {
    act("You simply fail to reach your mouth!",FALSE,ch,0,0,TO_CHAR);
    act("$n tries to sip, but fails!",TRUE,ch,0,0,TO_ROOM);
    return;
  }

  if(!temp->value[1])  /* Empty */
  {
    act("But there is nothing in it?",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  act("$n sips from the $o",TRUE,ch,temp,0,TO_ROOM);
  sprintf(buf,"It tastes like %s.\n\r",drinks[temp->value[2]]);
  send_to_char(buf,ch);

  gain_condition(ch,DRUNK,(int)(drink_aff[temp->value[2]][DRUNK]/4));

  gain_condition(ch,FULL,(int)(drink_aff[temp->value[2]][FULL]/4));

  gain_condition(ch,THIRST,(int)(drink_aff[temp->value[2]][THIRST]/4));

  weight_change_object(temp, -1);  /* Subtract one unit */

  if(GET_COND(ch,DRUNK)>10)
    act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);

  if(GET_COND(ch,THIRST)>20)
    act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

  if(GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);

  if(temp->value[3]&&!IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */
  {
    act("But it also had a strange taste!",FALSE,ch,0,0,TO_CHAR);

    af.type = SPELL_POISON;
    af.duration = 3;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_to_char(ch,&af);
  }

  temp->value[1]--;

  if(!temp->value[1])  /* The last bit */
  {
    temp->value[2]=0;
    temp->value[3]=0;
    name_from_drinkcon(temp);
  }
  return;
}


void do_taste(charType *ch, char *argument, int cmd)
{
  	affectType af;
  	char arg[MAX_STRING_LENGTH];
  	objectType *temp;

  	oneArgument(argument,arg);

  	if(!(temp = find_obj_inven( ch, arg )) )
  	{
    	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    	return;
  	}

  if(temp->type==ITEM_DRINKCON)
  {
    do_sip(ch,argument,0);
    return;
  }

  if(!(temp->type==ITEM_FOOD))
  {
    act("Taste that?!? Your stomach refuses!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
  act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);

  gain_condition(ch,FULL,1);

  if(GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);

  if(temp->value[3]&&!IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */
  {
    act("Ooups, it did not taste good at all!",FALSE,ch,0,0,TO_CHAR);

    af.type = SPELL_POISON;
    af.duration = 2;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_to_char(ch,&af);
  }

  temp->value[0]--;

  if(!temp->value[0])  /* Nothing left */
  {
    act("There is nothing left now.",FALSE,ch,0,0,TO_CHAR);
    extract_obj( temp, 1 );
  }
  return;
}
