#include "character.h"
#include "world.h"
#include "object.h"
#include "mobile.h"
#include "zone.h"
#include "transfer.h"
#include "spec.rooms.h"
#include "affect.h"
#include "comm.h"
#include "fight.h"
#include "interpreter.h"
#include "misc.h"
#include "nanny.h"
#include "edit.h"
#include "page.h"

void char_from_room(charType *ch)
{
  	charType *i;

  	if( ch->in_room == NOWHERE ) 
  	{
    	FATAL( "char_from_room> NOWHERE extracting char from room");
  	}

	if( world[ch->in_room].virtual == ROOM_LOCKER ) char_from_locker( ch );

  	if( ch->equipment[WEAR_LIGHT] )
    	if( ch->equipment[WEAR_LIGHT]->type == ITEM_LIGHT )
      		if( ch->equipment[WEAR_LIGHT]->value[2] ) 
	  			world[ch->in_room].light--;

  	if( ch == world[ch->in_room].people )  /* head of list */
     	world[ch->in_room].people = ch->next_in_room;
  	else    /* locate the previous element */
  	{
    	for (i = world[ch->in_room].people; 
       		i->next_in_room != ch; i = i->next_in_room);

     	i->next_in_room = ch->next_in_room;
  	}

  	ch->in_room = NOWHERE;
  	ch->next_in_room = 0;
}

void char_to_room(charType *ch, int room)
{
  	ch->next_in_room   = world[room].people;
  	world[room].people = ch;
  	ch->in_room = room;
  	if( ch->equipment[WEAR_LIGHT])
    	if( ch->equipment[WEAR_LIGHT]->type == ITEM_LIGHT)
      		if( ch->equipment[WEAR_LIGHT]->value[2]) /* Light is ON */
        		world[room].light++;

	if( world[room].virtual == ROOM_LOCKER ) char_to_locker( ch );
}

/* give an object to a char   */
void obj_to_char(objectType *object, charType *ch)
{
	objectType		* tmp, * prev;   

	for( prev = 0, tmp = ch->carrying; tmp && (tmp->nr != object->nr); 
			prev = tmp, tmp = tmp->next_content );

	if( tmp && prev )
	{
		object->next_content = tmp;
		prev->next_content = object;
	}
	else
	{
		object->next_content = ch->carrying;
		ch->carrying = object;
	}
  	object->carried_by = ch;
  	object->in_room = NOWHERE;
  	ch->carry_weight += object->weight;
  	ch->carry_items++;
}

/* take an object from a char */
void obj_from_char(objectType *object)
{
  	objectType *tmp;

  	if( object->carried_by->carrying == object)   /* head of list */
     	object->carried_by->carrying = object->next_content;

  	else
  	{
    	for (tmp = object->carried_by->carrying; 
       		tmp && (tmp->next_content != object); 
          		tmp = tmp->next_content); /* locate previous */

    	tmp->next_content = object->next_content;
  	}

  	object->carried_by->carry_weight -= object->weight;
  	object->carried_by->carry_items--;
  	object->carried_by = 0;
  	object->next_content = 0;
}

void char_from_world( charType * ch )
{
  	charType 		* 	tmp;
  	descriptorType 	*	t_desc;
	roundAffType	*	rf, * rf_next;
	editUnitType	*	edit;
  	pageUnitType	*	page;

  	if( ch->in_room == NOWHERE ) 
  	{
    	FATAL("char_from_world> char in NOWHERE (%s).", ch->name );
  	}

  	if( edit = find_editunit( ch ), edit ) del_editing( edit );
  	if( page = find_pageunit( ch ), page ) del_pageunit( page );

	for( rf = ch->roundAffs; rf; rf = rf_next )
	{
		rf_next = rf->my_next;
		roundAff_remove( rf );
	}

  	if( ch->fight ) stop_fighting( ch );

	for( tmp = char_list; tmp; tmp = tmp->next )
	{
		if( tmp->hunt  == ch ) tmp->hunt = 0;
    	if( tmp->fight == ch ) stop_fighting( tmp );
	}

  	if( !IS_NPC(ch) && !ch->desc )
  	{
    	for( t_desc = desc_list; t_desc; t_desc = t_desc->next )
      		if( t_desc->original == ch )
        		do_return( t_desc->character, "", 0);
  	}

 	if( ch->desc && ch->desc->original ) do_return( ch, "", 0 );
  	if( ch->followers || ch->master )    die_follower(ch);

  	if( ch->desc ) 
  	{
    	if( ch->desc->snoop.who )
      		ch->desc->snoop.who->desc->snoop.by = 0;
    	if( ch->desc->snoop.by ) 
		{
        	send_to_char("Your victim is no longer among us.\n\r", ch->desc->snoop.by);
        	ch->desc->snoop.by->desc->snoop.who = 0;
    	}
    	ch->desc->snoop.who = ch->desc->snoop.by = 0;
  	}

  	if( ch == char_list ) char_list = ch->next;
  	else 
  	{
    	for( tmp = char_list; tmp && (tmp->next != ch); tmp = tmp->next )
		;
    	if( tmp ) tmp->next = ch->next;
    	else 
		{
      		FATAL( "char_from_world> Removing char %s is not in the list.", ch->name );
    	}
		chars_in_world--;
  	}
	
	if( !IS_NPC(ch ) )
	{
  		if( ch == player_list ) player_list = ch->next_char;
  		else 
  		{
    		for( tmp = player_list; (tmp) && (tmp->next_char != ch); tmp = tmp->next_char )
			;
			if( tmp ) tmp->next_char = ch->next_char;
			else 
			{
				FATAL( "char_from_world> Removing player %s is not in the list.", ch->name );
			}
			players_in_world--;
		}
	}
	else
	{
		if( ch == mob_list ) mob_list = ch->next_char;
		else 
		{
			for( tmp = mob_list; (tmp) && (tmp->next_char != ch); tmp = tmp->next_char )
			;
			if( tmp ) tmp->next_char = ch->next_char;
			else 
			{
				FATAL( "char_from_world> Removing mob %s is not in the list.", ch->name );
			}
			mobs_in_world--;
		}
	}

	char_from_room( ch );

    if( ch->desc )
    {
        flush_inqueue( ch->desc );
    
        ch->desc->connected = CON_SLCT;
        ch->nr = -2;
        SEND_TO_Q( MENU, ch->desc );
    }
}

void wipe_all_obj( charType * ch )
{
	int 	i;

    for( i = 0; i < MAX_WEAR; i++ )
        if( ch->equipment[i] )
        {
            extract_obj( unequip_char(ch,i), 1 );   /* extract and free object */
            ch->equipment[i]=0;
        }

    wipe_obj_list( ch->carrying );  ch->carrying = 0; 
}	

void extract_all_obj( charType * ch )
{
	objectType 	*	obj;
	int				i;

    if( ch->carrying )
    {
        if (world[ch->in_room].contents)  /* room nonempty */
        {
            for( obj = world[ch->in_room].contents; obj->next_content; obj = obj->next_content)
			;
           	obj->next_content = ch->carrying;  
        }
        else
            world[ch->in_room].contents = ch->carrying;
   
        for( obj = ch->carrying; obj; obj = obj->next_content)
        {
            obj->carried_by = 0;
            obj->in_room = ch->in_room;
        }
        ch->carrying = 0;
    }
   
    for( i = 0; i < MAX_WEAR; i++ )
        if( ch->equipment[i] )
            obj_to_room( unequip_char(ch,i), ch->in_room );
}

void obj_to_in_world( objectType * obj )
{
	if( obj->contains ) 		obj_to_in_world( obj->contains );
	if( obj->next_content ) 	obj_to_in_world( obj->next_content );

	objects[obj->nr].off_world--;
	objects[obj->nr].in_world++;

	objs_in_world++;
	objs_off_world--;
}

void obj_to_off_world( objectType * obj )
{
	if( obj->contains ) 		obj_to_off_world( obj->contains );
	if( obj->next_content ) 	obj_to_off_world( obj->next_content );

	objects[obj->nr].off_world++;
	objects[obj->nr].in_world--;

	objs_in_world--;
	objs_off_world++;
}
