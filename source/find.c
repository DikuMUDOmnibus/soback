#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "defines.h"
#include "character.h"
#include "object.h"
#include "world.h"
#include "zone.h"
#include "mobile.h"
#include "utils.h"
#include "find.h"
#include "limits.h"
#include "strings.h"

objectType * find_off_obj( objectType * list, int nr )
{  
    objectType  *   obj;
   
    for( obj = list; obj; obj = obj->next_content )
    {
        if( obj->what == nr ) return obj;
    }
    return NULL;
}

void renum_off_list( objectType * list )
{  
    int                 i;
    objectType  *       obj;
   
    for( i = 0, obj = list; obj; obj = obj->next_content )
    {
        obj->what = ++i;
    }
}

void detach_off_obj( objectType ** list, objectType * find )
{
	objectType	*	obj;

	if( find == *list )
	{
		*list = (*list)->next_content;
	}
	else
	{
		for( obj = *list; obj && obj->next_content != find; obj = obj->next_content )
		;

		obj->next_content = obj->next_content->next_content;
	}
	find->what = 0;
	find->next_content = 0;
}

#define RECTBL_SIZE		2000	

struct object_record_type 	found_objects[RECTBL_SIZE];
charType				 *	found_chars[RECTBL_SIZE];

int							objs_in_found_table		= 0;
int							chars_in_found_table 	= 0;

void init_search_records( void )
{
	objs_in_found_table  = 0;
	chars_in_found_table  = 0;
}

int record_found_char( charType * ch )
{
	if( chars_in_found_table == RECTBL_SIZE ) return 0;

	found_chars[chars_in_found_table++] = ch;
	return 1;
}

int record_found_object( objectType * obj, int same )
{
	int		curr;

	if( objs_in_found_table == RECTBL_SIZE ) return 0;

	if( !same || obj->nr <= OBJECT_INTERNAL ) curr = objs_in_found_table;
	else
		for( curr = 0; curr < objs_in_found_table; curr++ )
		{
			if( obj->nr == found_objects[ curr ].obj->nr )
			{
				found_objects[curr].num++;
				break;
			}
		}

	if( curr == objs_in_found_table )
	{
		found_objects[curr].obj = obj;
		found_objects[curr].num = 1;
		objs_in_found_table++;
	}
	return 1;
}

void obj_to_room( objectType * object, int room )
{
	if( object->type == ITEM_MONEY )
	{
		objectType	*	tmp, * gold;

		for( tmp = world[ room ].contents; tmp; tmp = tmp->next_content )
		{
			if( tmp->type == ITEM_MONEY )
			{
				gold = create_money( tmp->value[0] + object->value[0] );
				extract_obj( object, 1 );
				extract_obj( tmp, 1 );
				object = gold;
				break;
			}
		}
	}

  	object->next_content = world[room].contents;
  	world[room].contents = object;
  	object->in_room = room;
  	object->carried_by = 0;
}

void obj_from_room( objectType * obj )
{
  	objectType * i;

  	if(  obj == world[ obj->in_room].contents )  /* head of list */
     	world[ obj->in_room ].contents =  obj->next_content;
  	else     /* locate previous element in list */
  	{
    	for(i = world[ obj->in_room].contents; i &&(i->next_content !=  obj); i = i->next_content);

		if( i )	i->next_content =  obj->next_content;
		else
		{
			FATAL( "obj_from_room> not found in #%d room's contents.", obj->in_room );
		}
   	}

  	 obj->in_room = NOWHERE;
  	 obj->next_content = 0;
}

void obj_to_obj(objectType *obj, objectType *obj_to)
{
	objectType		* tmp, * prev;   

	for( prev = 0, tmp = obj_to->contains; tmp && (tmp->nr != obj->nr); 
				prev = tmp, tmp = tmp->next_content );

	if( tmp && prev )
	{
		obj->next_content = tmp;
		prev->next_content = obj;
	}
	else
	{
		obj->next_content = obj_to->contains;
		obj_to->contains = obj;
	}

  	obj->in_obj = obj_to;

  	for( tmp = obj_to; tmp;	tmp->weight += obj->weight, tmp = tmp->in_obj );
}

void obj_from_obj( objectType *obj )
{
  	objectType 		* tmp, * from;

  	if( obj->in_obj ) 
  	{
    	from = obj->in_obj;
    	if( obj == from->contains )   /* head of list */
       		from->contains = obj->next_content;
    	else 
		{
      		for( tmp = from->contains; tmp && (tmp->next_content != obj); tmp = tmp->next_content);

      		if( !tmp ) 
	  		{
        		FATAL("obj_from_obj> obj not found.");
      		}
      		tmp->next_content = obj->next_content;
    	}

    	for( tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj ) 
			tmp->weight -= obj->weight;

    	tmp->weight -= obj->weight;

    	if( tmp->carried_by ) tmp->carried_by->carry_weight -= obj->weight;

    	obj->in_obj       = 0;
    	obj->next_content = 0;
  	} 
  	else 
  	{
    	FATAL( "obj_from_obj> Trying object when in no object.");
  	}
}

void obj_list_new_owner( objectType *list, charType * ch )
{
  	if( list ) 
  	{
    	obj_list_new_owner(list->contains, ch);
    	obj_list_new_owner(list->next_content, ch);
    	list->carried_by = ch;
  	}
}

charType * find_char_room( charType * ch, char *name )
{
  	charType * 			find;
  	int 				found, number;
  	char 				tmpname[MAX_INPUT_LENGTH];
  	char *				tmp;

  	tmp = strcpy( tmpname, name );

  	if( !(number = dotnumber( &tmp )) ) return 0;

  	TRACE( trace("get_char_room_vis> %d.%s ", number, tmp) );

  	for( found = 0, find = world[ch->in_room].people; find && (found <= number); 
  															find = find->next_in_room)
    	if( isoneof(tmp, GET_NAME(find)) )
      		if (can_see(ch, find))  
	  		{
        		if( ++found == number ) return(find);
      		}

  	for( found = 0, find = world[ch->in_room].people; find && (found <= number); 
  															find = find->next_in_room)
    	if( isoneofp(tmp, GET_NAME(find)) )
      		if (can_see(ch, find))  
	  		{
        		if( ++found == number) return(find);
      		}

  	return 0;
}

charType * find_char_room_at( charType * ch, int room, char * name )
{
  	charType * 			find;
  	char 				tmpname[MAX_INPUT_LENGTH];
  	char *				tmp;
  	int					nr;

	if( room > ROOM_INTERNAL ) nr = room;
	else if( ch )  nr = ch->in_room;
	else 		   return 0;

  	tmp = strcpy( tmpname, name );

  	for( find = world[room].people; find; find = find->next_in_room )
    	if( isoneof( tmp, find->name ) )
			if( can_see(ch, find) ) return find;

  	return 0;
}

charType * find_player_room_at( charType * ch, int room, char *name )
{
  	charType * 			find;
  	int 				nr;
  	char 				tmpname[MAX_INPUT_LENGTH];
  	char *				tmp;

	if( room > ROOM_INTERNAL ) nr = room;
	else if( ch )  nr = ch->in_room;
	else 		   return 0;

  	tmp = strcpy( tmpname, name );

  	for( find = world[room].people; find; find = find->next_in_room )
  	{
  		if( IS_NPC( find ) ) continue;

    	if( isoneof( tmp, find->name ) )
      		if( can_see(ch, find) ) return find;
	}
  	return 0;
}

charType * find_mob_room_at( charType * ch, int room, char * name )
{
	charType		*	find;
	int					nr;
  	char 				tmpname[MAX_INPUT_LENGTH];
  	char *				tmp;

	if( room > ROOM_INTERNAL ) nr = room;
	else if( ch )  nr = ch->in_room;
	else 		   return 0;

  	tmp = strcpy( tmpname, name );

	for( find = world[nr].people; find; find = find->next_in_room )
	{
		if( !IS_NPC( find ) ) continue;

		if( isoneof( tmp, find->name ) )
      		if( !ch || can_see(ch, find) ) return find;
	}
	return 0;
}

charType * find_char( charType * ch, char * name )
{
  	charType 	*	find;
  	int 			found, number;
  	char 			tmpname[MAX_INPUT_LENGTH];
  	char 		*	tmp;

  	tmp = strcpy(tmpname,name);

  	if(!(number = dotnumber( &tmp ))) return 0;

	if( number == 1 )
	for( find = player_list; find ; find = find->next_char )
		if( isoneof(tmp, GET_NAME(find)) && (!ch || can_see(ch, find)) )
				return(find);

  	for( found = 0, find = char_list; find && (found <= number); find = find->next )
    	if( isoneof(tmp, GET_NAME(find)) && (!ch || can_see(ch, find)) )
			{
        		if( ++found == number ) return(find);
      		}
  	return 0;
}

charType * find_player( charType * ch, char * name )
{
  	descriptorType * d;

  	for( d = desc_list; d; d = d->next ) 
    	if( !d->connected && (!ch || can_see(ch, d->character)) && isoneof(name, d->character->name )) 
			return(d->character) ;

  	return 0;
}

charType * find_char_zone( charType * ch, char * name )
{
  	char 			tmpname[MAX_INPUT_LENGTH];
  	int 			number, found;
  	char 		*	who;
  	charType	*   find;
  	roomType	*	room;

  	who = strcpy( tmpname, name );

  	if( number = dotnumber( &who ), !number ) return 0;

	for( found = 0, room = zones[world[ch->in_room].zone].rooms; room; room = room->next )
		for( find = room->people; find; find = find->next_in_room )
    		if( isoneof( who, find->name ) && (!ch || can_see( ch, find )) )
			{
       			if( ++found == number ) return find;
      		}

  	return 0;
}

objectType * find_obj_equip( charType * ch, char *arg )  
{
	objectType 	**	equip;
	char			tmpname[MAX_INPUT_LENGTH];
	char		*	tmp;
	int				i, number, ofound = 0;

	tmp = strcpy( tmpname, arg );

	if( !(number = dotnumber( &tmp ) ) ) return 0;
  
  	equip = ch->equipment;

  	for( i = 0; i < MAX_WEAR ; i++ )
    	if( equip[i] )
      		if( can_see_obj( ch, equip[i]) && isoneof(arg, equip[i]->name) )
			{
				if( ++ofound >= number ) return(equip[i]);
			}

  	for( i = 0; i < MAX_WEAR ; i++ )
    	if( equip[i] )
      		if( can_see_obj( ch, equip[i]) && isoneofp(arg, equip[i]->name) )
			{
				if( ++ofound >= number ) return(equip[i]);
			}

  	return 0;
}

objectType * find_obj_list( charType *ch, char *name, objectType * list, int partial )
{
  	objectType 	*	obj;
  	int 			found, number;
  	char 			tmpname[MAX_INPUT_LENGTH];
  	char 		*	tmp;

  	tmp = strcpy(tmpname,name);

  	if(!(number = dotnumber( &tmp ))) return 0;

  	for( found = 0, obj = list; obj && (found <= number); obj = obj->next_content)
    	if( isoneof(tmp, obj->name) && (!ch || can_see_obj(ch, obj)) ) 
		{
       		if( ++found == number)	return obj;
      	}

	if( partial )
	for( found = 0, obj = list; obj && (found <= number); obj = obj->next_content)
   		if( isoneofp(tmp, obj->name) && (!ch || can_see_obj(ch, obj)) ) 
		{
   			if( ++found == number)	return obj;
   		}
  	return 0;
}

objectType * find_obj_inven( charType * ch, char * name )
{
	return find_obj_list( ch, name, ch->carrying, 1 );
}

objectType * find_obj_room( charType * ch, char * name )
{
	return find_obj_list( ch, name, world[ch->in_room].contents, 1 );
}

objectType * find_obj( charType *ch, char * name )
{
  	objectType 		*	obj;
  	int 				found, number;
  	char 				tmpname[MAX_INPUT_LENGTH];
  	char 			*	tmp;

  	tmp = strcpy( tmpname, name );
  	if( !(number = dotnumber( &tmp )) ) return 0;

  	for( found = 0, obj = obj_list; obj && (found <= number); obj = obj->next)
    	if( !obj->what && isoneof(tmp, obj->name) && (!ch || can_see_obj(ch, obj)) ) 
		{
        	if( ++found == number) return obj;
      	}
  	return 0;
}

/* Find, designed to find any object/character 			                  */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int find( char *arg, int bitvector, charType * ch, charType ** tar_ch, objectType ** tar_obj )
{
  	char 		name[256];

	strcpy( name, skipsps( arg ) );

  	*tar_ch  = 0;	*tar_obj = 0;

  	if( IS_SET(bitvector, FIND_CHAR_ROOM) ) 
    	if((*tar_ch = find_char_room( ch, name )) ) 	return(FIND_CHAR_ROOM);

  	if( IS_SET(bitvector, FIND_CHAR_WORLD) ) 
    	if((*tar_ch = find_char( ch, name )) ) 			return(FIND_CHAR_WORLD);

  	if( IS_SET(bitvector, FIND_OBJ_INV) ) 
    	if((*tar_obj = find_obj_inven( ch, name )) ) 	return(FIND_OBJ_INV);

  	if( IS_SET(bitvector, FIND_OBJ_EQUIP) ) 
    	if((*tar_obj = find_obj_equip( ch, name )) ) 	return(FIND_OBJ_INV);

  	if( IS_SET(bitvector, FIND_OBJ_ROOM) ) 
    	if((*tar_obj = find_obj_room( ch, name )) )		return(FIND_OBJ_ROOM);

  	if( IS_SET(bitvector, FIND_OBJ_WORLD) ) 
    	if((*tar_obj = find_obj( ch, name )) )			return(FIND_OBJ_WORLD);

  	return 0;
}
