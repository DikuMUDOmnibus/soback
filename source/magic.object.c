#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "utils.h"
#include "mobile.h"
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
#include "transfer.h"
#include "affect.h"
  
void spell_create_food( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  	objectType *tmp_obj;

    tmp_obj = create_food();

  	obj_to_room(tmp_obj,ch->in_room);

  	act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  	act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);
}

void spell_create_water( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  	int water;

  	if( obj->type == ITEM_DRINKCON ) 
  	{     
    	if ((obj->value[2] != LIQ_WATER)   
         && (obj->value[1] != 0)) 
		{
      		name_from_drinkcon(obj);
      		obj->value[2] = LIQ_SLIME;       
      		name_to_drinkcon(obj, LIQ_SLIME);
    	} 
		else 
		{

      		water = 2*level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

      		/* Calculate water it can contain, or water created */
      		water = MIN(obj->value[0]-obj->value[1], water);

      		if( water > 0) 
	  		{
        		obj->value[2] = LIQ_WATER;     
        		obj->value[1] += water;

        		weight_change_object(obj, water);

        		name_from_drinkcon(obj);
        		name_to_drinkcon(obj, LIQ_WATER);
        		act("$p is filled.", FALSE, ch,obj,0,TO_CHAR);
      		}
    	}
  	}
  	else
  	{
        sendf( ch, "It is unable to hold water." );
	}
}

void spell_create_nectar( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  int nectar;

  void name_to_drinkcon(objectType *obj,int type);
  void name_from_drinkcon(objectType *obj); 

  	if( obj->type == ITEM_DRINKCON ) 
  	{     
    	if ((obj->value[2] != LIQ_NECTAR)  
         && (obj->value[1] != 0)) 
	 	{    
      		name_from_drinkcon(obj);
      		obj->value[2] = LIQ_SLIME;
      		name_to_drinkcon(obj, LIQ_SLIME);
    	} 
		else 
		{
      		nectar = 2*level;

      		/* Calculate water it can contain, or water created */
      		nectar = MIN(obj->value[0]-obj->value[1], nectar);

      		if( nectar > 0 ) 
	  		{
        		obj->value[2] = LIQ_NECTAR;
        		obj->value[1] += nectar;       

        		weight_change_object(obj, nectar);       

        		name_from_drinkcon(obj);
        		name_to_drinkcon(obj, LIQ_NECTAR);
        		act("$p is filled.", FALSE, ch,obj,0,TO_CHAR);
      		}
    	}
  	}
  	else
  	{
        sendf( ch, "It is unable to hold water." );
	}
}

void spell_create_golden_nectar( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  	int nectar;

  	void name_to_drinkcon(objectType *obj,int type);
  	void name_from_drinkcon(objectType *obj); 

  	if( obj->type == ITEM_DRINKCON ) 
  	{     
    	if ((obj->value[2] != LIQ_GOLDEN_NECTAR)
         && (obj->value[1] != 0)) 
		{
      		name_from_drinkcon(obj);
      		obj->value[2] = LIQ_SLIME;
      		name_to_drinkcon(obj, LIQ_SLIME);
    	}
		else 
		{
      		nectar = 3*level/2;

		    /* Calculate water it can contain, or water created */
      		nectar = MIN(obj->value[0]-obj->value[1], nectar);

      		if( nectar > 0 ) 
	  		{
        		obj->value[2] = LIQ_GOLDEN_NECTAR;
        		obj->value[1] += nectar;       

        		weight_change_object(obj, nectar);       

        		name_from_drinkcon(obj);
        		name_to_drinkcon(obj, LIQ_GOLDEN_NECTAR);
        		act("$p is filled.", FALSE, ch,obj,0,TO_CHAR);
      		}
    	}
  	}
  	else
  	{
        sendf( ch, "It is unable to hold water." );
	}
}

void spell_recharger( int level, charType *ch, char * arg, int type,
                    charType *victim, objectType *obj)
{  
    int i;
   
    if( ( obj->type == ITEM_WAND ) ||    
        ( obj->type == ITEM_STAFF))      
    {
        if( number( 1, 1+level/5 ) == 1 )
        {
            act( "BANG!!!! $p explodes. It hurts!", FALSE, ch, obj, 0, TO_CHAR );

            for( i = 0; i < MAX_WEAR; i++ ) if( ch->equipment[i] == obj ) break;
   
            if( i < MAX_WEAR ) obj_to_char( unequip_char( ch, i ), ch );
   
            extract_obj( obj, 1 );

            GET_HIT(ch)-=GET_HIT(ch)/10;
            return;
        }

        obj->value[2]=
            MIN(obj->value[1],obj->value[2]+5);
        act("$p looks recharged.",FALSE,ch,obj,0,TO_CHAR);
    }
}

void spell_enchant_weapon( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  	int 	i;

  	if(( obj->type == ITEM_WEAPON) &&     
       !IS_SET(obj->extra, ITEM_MAGIC)) 
	{
    	for (i=0; i < MAX_APPLY; i++)
      		if(obj->apply[i].location != APPLY_NONE) return;

    	SET_BIT(obj->extra, ITEM_MAGIC);

    	obj->apply[0].location = APPLY_HITROLL;   
    	obj->apply[0].modifier = 1 + (level >= 18) + (level >= 30);

    	obj->apply[1].location = APPLY_DAMROLL;   
    	obj->apply[1].modifier = 1 + (level >= 20) + (level >= 35);

    	if( IS_GOOD(ch) ) 
		{
      		SET_BIT(obj->extra, ITEM_ANTI_EVIL);
      		act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
    	} 
		else if( IS_EVIL(ch) ) 
		{
      		SET_BIT(obj->extra, ITEM_ANTI_GOOD);
      		act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
    	} 
		else 
		{
      		act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
    	}
  	}
}

void spell_enchant_armor(  int level, charType *ch,  char * arg, int type,
                            charType *victim, objectType *obj)
{
    int i;

    if(( obj->type == ITEM_ARMOR) && !IS_SET(obj->extra, ITEM_MAGIC))
    {

        for (i=0; i < MAX_APPLY; i++)
            if (obj->apply[i].location != APPLY_NONE)
        return;

        SET_BIT(obj->extra, ITEM_MAGIC);

        obj->apply[0].location = APPLY_AC;    
        obj->apply[0].modifier = -1 - (level >= 35);

        if(IS_GOOD(ch))
        {
            SET_BIT(obj->extra, ITEM_ANTI_EVIL);
            act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
        }
        else if( IS_EVIL(ch) )
        {
            SET_BIT(obj->extra, ITEM_ANTI_GOOD);
            act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
        }
        else
        {
            act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
        }
    }
}


void spell_pray_for_armor( int level, charType *ch, char * arg, int type,
  charType *victim, objectType *obj)
{
  	int 	i;

  	if( ( obj->type == ITEM_ARMOR) &&      
      !IS_SET(obj->extra, ITEM_MAGIC)) 
  	{
    	for( i = 0; i < MAX_APPLY; i++ )
      		if (obj->apply[i].location != APPLY_NONE)
        		return;

    	SET_BIT(obj->extra, ITEM_MAGIC);

    	obj->apply[0].location = APPLY_HITROLL;   
    	obj->apply[0].modifier = 1;
    	if( level >= 38 )
		{
      		obj->apply[1].location=APPLY_HITROLL;   
      		obj->apply[1].modifier = 1;
    	}

    	if( IS_GOOD(ch) ) 
		{
      		SET_BIT(obj->extra, ITEM_ANTI_EVIL);
      		act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
    	} 
		else if( IS_EVIL(ch) ) 
		{
      		SET_BIT(obj->extra, ITEM_ANTI_GOOD);
      		act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
    	} 
		else 
		{
      		act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
    	}
  	}
}

void spell_locate_object( int level, charType *ch, char * arg, int type,
									charType *victim, objectType *obj)
{
  	char 				* 	name;
  	int 					number, i, j;

  	name   = arg;
  	number = dotnumber( &name );

  	j = GET_INT(ch);

  	for( i = 0, obj = obj_list; obj && ( j > 0); obj = obj->next ) 
  	{
    	if( !obj->what && isoneof(name, obj->name) ) 
		{
			if( ++i < number ) continue;	

      		if(obj->carried_by) 
	  		{
        		sendf( ch, "%s carried by %s.",	obj->wornd,PERS(obj->carried_by,ch));
      		} 
	  		else if( obj->in_obj ) 
			{
        		sendf( ch, "%s in %s.",obj->wornd,	obj->in_obj->wornd);
      		} 
	  		else 
			{
        		sendf( ch, "%s in %s.",obj->wornd,
          		((obj->in_room==NOWHERE) ? "uncertain." :world[obj->in_room].name));
        		if(GET_LEVEL(ch) < IMO) j--;
      		}
    	}
	}
  	if( j == 0 ) sendf( ch, "You are very confused." );
  	if( j == GET_INT(ch) ) sendf( ch, "No such object." );      
}

void spell_reanimate( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj)
{
	followType	* 	fol;
 	charType 	* 	mob;
 	int				i;

 	if( !(obj &&  obj->type == ITEM_CONTAINER && obj->magic != OBJECT_CORPSE ) )
 	{
   		sendf( ch, "There do not appear to be any corpses hereabouts?" );
   		return;
 	}
    for( i = 0, fol = ch->followers; fol; fol = fol->next )
    {
        if( IS_NPC(fol->follower) ) i++;
    }
 
    if( i >= (level / 2) )
    {
        sendf( ch, "You can't create more followers." );
        return;
    }

 	if( dice(9,9) > ch->skills[SPELL_REANIMATE] ) 
 	{ 
   		sendf( ch, "The spell fails miserably." ); 
   		return;
 	}

 	extract_obj( obj, 1 );
 	mob = load_a_mobile( 2, VIRTUAL );

 	char_to_room(mob,ch->in_room);
 	act("$n has created a zombie!",TRUE,ch,0,0,TO_ROOM); 
 	sendf( ch, "You have created a zombie." );
 	add_follower(mob,ch);
 	mob->max_hit += dice( 8, GET_INT(ch) );     
 	mob->hit = mob->max_hit;
 	mob->max_move = dice( 6, GET_WIS(ch) );
}

void spell_sense_sprit( int level, charType *ch, char * arg, int type,
  						charType *victim, objectType *obj )
{
	if( !*arg ) return;

	for( obj = obj_list; obj; obj = obj->next )
	{
		if( obj->type == ITEM_CONTAINER && obj->magic != OBJECT_CORPSE )
			if( isoneof( arg, obj->name ) 
             && obj->level == victim->level 
			 && obj->magic == victim->class
			 && obj->in_room != NOWHERE )
			{
				act( "$n이 땅을 그의 손가락으로 가리키자, 꿈틀거리는 진동과 함께 어두운 그림자가"
                     "가 솟아나와 $n 에게 절하며 그를 어디론가 인도했다.", FALSE, ch, 0, 0, TO_ROOM );
				sendf( ch, "당신이 가리키는 손끝에 대지의 영이 진동과 함께 솟아나와 당신을 한 방"
						   "으로 인도한다.\n\r" );
        		sendf( ch, world[obj->in_room].name );
			    send_to_char( world[obj->in_room].description, ch );
			  	list_obj_to_char( world[obj->in_room].contents, ch, 0, FALSE );
				list_room_chars( ch, world[obj->in_room].people );
				break;
			}
	}
	if( !obj )
	{
		act( "$n 이 땅을 가리키고, 또 땅은 진동했으나 아무런 일도 일어나지 않았다.", 
			FALSE, ch, 0, 0, TO_ROOM ); 
		sendf( ch, "당신의 마법의 힘으로 땅의 혼을 소환했으나 아무런 일도 생기지 않았다." );
	}
}
