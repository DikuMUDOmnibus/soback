/* ************************************************************************
*  file: act.movement.c , Implementation of commands      Part of DIKUMUD *
*  Usage : Movement commands, close/open & lock/unlock doors.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "character.h"
#include "object.h"
#include "world.h"
#include "mobile.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "variables.h"
#include "strings.h"
#include "affect.h"
#include "transfer.h"
#include "affect.h"

int special(charType *ch, int cmd, char *arg);

int do_simple_move( charType *ch, int cmd, int following )
{
  	int 			was_in;
  	int 		 	need_movement;
  	objectType * 	obj;
  	bool 			has_boat, has_wing;
  	int 			obj_number ;

  	if( special (ch, cmd+1, "") ) return(FALSE);

  	need_movement = (movement_loss[world[ch->in_room].sector]+
  		movement_loss[world[world[ch->in_room].dirs[cmd]->to_room].sector]) / 2;

	if( IS_AFFECTED( ch, AFF_MORPETH ) ) need_movement *= 2;

  	if(  (world[ch->in_room].sector == SECT_WATER_NOSWIM) 
	  || (world[world[ch->in_room].dirs[cmd]->to_room].sector == SECT_WATER_NOSWIM) ) 
	{
    	has_boat = FALSE;

    	for( obj = ch->carrying; obj; obj = obj->next_content )
      		if( obj->type == ITEM_BOAT )
        		has_boat = TRUE;
    	if( !has_boat ) 
		{
      		send_to_char("You need a boat to go there.\n\r", ch);
      		return(4);
    	}
  	}

  	if(  (world[ch->in_room].sector == SECT_SKY) 
	  || (world[world[ch->in_room].dirs[cmd]->to_room].sector ==SECT_SKY) )
   	{
    	has_wing = FALSE;
    	/* See if char is carrying a wing */
    	for( obj=ch->carrying; obj; obj=obj->next_content )
		{
      		obj_number = objects[obj->nr].virtual ;
      		if( obj_number == 9703 || obj_number == 2700 )
				/* Wings of Pegasus  and feather */
        		has_wing = TRUE;
       	}
    	if( !has_wing && (GET_LEVEL(ch) < IMO+3) ) 
		{
      		send_to_char("You need wings to fly there.\n\r", ch);
      		return(3);
    	}
  	}

  	if( GET_MOVE(ch) < need_movement && !OMNI(ch) )
  	{
    	if( !following ) send_to_char("You are too exhausted.\n\r",ch);
    	else 			 send_to_char("You are too exhausted to follow.\n\r",ch);

    	return(2);
  	}

  	GET_MOVE(ch) -= need_movement;
	  
	if( !IS_AFFECTED(ch, AFF_SNEAK) && !IS_AFFECTED(ch, AFF_MORPETH) ) 
	{
		char 	tmp[200];

		sprintf(tmp, "$n leaves %s.", dirs[cmd]);
		act(tmp, TRUE, ch, 0,0,TO_ROOM);
  	}
					
	was_in = ch->in_room;

  	char_from_room(ch);
  	char_to_room( ch, world[was_in].dirs[cmd]->to_room );

  	if( !IS_AFFECTED(ch, AFF_SNEAK) && !IS_AFFECTED( ch, AFF_MORPETH ) ) 
		act("$n has arrived.", TRUE, ch, 0,0, TO_ROOM);  

  	do_look(ch, "\0", COM_LOOK );
  	return 1;
}

void do_move(charType *ch, char *argument, int cmd)
{
  	char 				tmp[80];
  	int 				was_in;
  	charType		* 	temp;
	directionType	*	door;
  	followType 		*	k, * next_dude;

  	--cmd;

    if( door = world[ch->in_room].dirs[cmd], !door )
	{
    	send_to_char("Alas, you cannot go that way...\n\r", ch);
  	}
	else 
	{          /* Direction is possible */
		if( door->to_room == NOWHERE )
		{
      		send_to_char("Alas, you can't go that way.\n\r", ch);
			return;
		}

    	if( door->exit_info & ( EX_CLOSED | EX_LOCKED ) ) 
		{
      		if( door->keyword ) 
			{
        		sprintf(tmp, "The %s seems to be closed.\n\r", oneword(EXIT(ch, cmd)->keyword)); 
				send_to_char(tmp, ch);
      		} 
			else 
			{
        		send_to_char("It seems to be closed.\n\r", ch);
      		}
			return;
   		} 

        for( temp = world[door->to_room].people; temp; temp = temp->next_in_room )
        {
            if( IS_AFFECTED( temp, AFF_TIME_STOP ) )
            {
                sendf( ch, "Some magical power blocked you." );
                return;
            }
        }

    	if( !ch->followers && !ch->master ) do_simple_move(ch,cmd,FALSE);
    	else 
		{
      		if( (IS_AFFECTED(ch, AFF_CHARM))
	   		 && (ch->master) 
	   		 && (ch->in_room == ch->master->in_room)) 
	  		{
        		send_to_char("The thought of leaving your master makes you weep.\n\r", ch);
        		act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
      		} 
	  		else 
	  		{
        		was_in = ch->in_room;
        		if( do_simple_move(ch, cmd, TRUE) == 1 ) 
				{ 
          			if( ch->followers) 
					{
            			for(k = ch->followers; k; k = next_dude) 
						{
              				next_dude = k->next;
              				if( (was_in == k->follower->in_room) 
						 	 && (GET_POS(k->follower) >= POSITION_STANDING)) 
							{
                				act("You follow $N.\n\r", FALSE, k->follower, 0, ch, TO_CHAR);
                				do_move( k->follower, argument, cmd + 1);
              				}
            			}
          			}
        		}
      		}
    	}
  	}
}

int find_door(charType *ch, char *type, char *dir)
{
  char buf[MAX_STRING_LENGTH];
  int door;

  if (*dir) /* a direction was specified */
  {
    if( door = isdirection( dir ) - 1, door == -1) /* Partial Match */
    {
      send_to_char("That's not a direction.\n\r", ch);
      return(-1);
    }

    if (EXIT(ch, door))
      if (EXIT(ch, door)->keyword)
        if (isoneof(type, EXIT(ch, door)->keyword))
          return(door);
        else
        {
          sprintf(buf, "I see no %s there.\n\r", type);
          send_to_char(buf, ch);
          return(-1);
        }
      else
        return(door);
    else
    {
      send_to_char(
        "I really don't see how you can close anything there.\n\r", ch);
      return(-1);
    }
  }
  else /* try to locate the keyword */
  {
    for (door = 0; door <= 5; door++)
      if (EXIT(ch, door))
        if (EXIT(ch, door)->keyword)
          if (isoneof(type, EXIT(ch, door)->keyword))
            return(door);

    sprintf(buf, "I see no %s here.\n\r", type);
    send_to_char(buf, ch);
    return(-1);
  }
}

int find_container( charType * ch, char * argu, objectType ** obj )
{
	objectType	*	con;

	if( con = find_obj_room( ch, argu ), con )
	{
		*obj = con; return 1;
	}

	if( con = find_obj_inven( ch, argu ), con )
	{
		*obj = con; return 1;
	}

	return 0;
}

void do_open(charType *ch, char *argument, int cmd)
{
  	int door, other_room;
  	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  	directionType *back;
  	objectType *obj;

  	halfchop(argument, type, dir);

  	if( !*type ) send_to_char( "Open what?\n\r", ch );
  	else if( find_container( ch, argument, &obj ) )

    /* this is an object */

    if (obj->type != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (!IS_SET(obj->value[1], CONT_CLOSED))
      send_to_char("But it's already open!\n\r", ch);
    else if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
      send_to_char("You can't do that.\n\r", ch);
    else if (IS_SET(obj->value[1], CONT_LOCKED))
      send_to_char("It seems to be locked.\n\r", ch);
    else
    {
      REMOVE_BIT(obj->value[1], CONT_CLOSED);
      send_to_char("Ok.\n\r", ch);
      act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  	else if ((door = find_door(ch, type, dir)) >= 0)

    /* perhaps it is a door */

    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's impossible, I'm afraid.\n\r", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("It's already open!\n\r", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("It seems to be locked.\n\r", ch);
    else
    {
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
      if (EXIT(ch, door)->keyword)
        act("$n opens the $F.", FALSE, ch, 0, EXIT(ch, door)->keyword,
          TO_ROOM);
      else
        act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Ok.\n\r", ch);
      /* now for opening the OTHER side of the door! */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
        if((back = world[other_room].dirs[rev_dir[door]]))
          if(back->to_room == ch->in_room)
          {
            REMOVE_BIT(back->exit_info, EX_CLOSED);

            if (back->keyword)
            {
              sendrf( 0, EXIT(ch, door)->to_room, "The %s is opened from the other side.",
                			oneword(back->keyword));
            }
            else
              sendrf( 0, EXIT(ch, door)->to_room, "The door is opened from the other side." );
          }             
    }
}

void do_close(charType *ch, char *argument, int cmd)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  directionType *back;
  objectType *obj;

  twoArgument(argument, type, dir);
  if (!*type)
    send_to_char("Close what?\n\r", ch);
  	else if( find_container( ch, argument, &obj ) )

    /* this is an object */

    if (obj->type != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
     else if (IS_SET(obj->value[1], CONT_CLOSED))
      send_to_char("But it's already closed!\n\r", ch);
    else if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
      send_to_char("That's impossible.\n\r", ch);
    else
    {
      SET_BIT(obj->value[1], CONT_CLOSED);
      send_to_char("Ok.\n\r", ch);
      act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  else if ((door = find_door(ch, type, dir)) >= 0)

    /* Or a door */

    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("It's already closed!\n\r", ch);
    else
    {
      SET_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
      if (EXIT(ch, door)->keyword)
        act("$n closes the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
          TO_ROOM);
      else
        act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Ok.\n\r", ch);
      /* now for closing the other side, too */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
        if((back = world[other_room].dirs[rev_dir[door]]))
          if (back->to_room == ch->in_room)
          {
            SET_BIT(back->exit_info, EX_CLOSED);
            if (back->keyword)
            {
              sendrf( 0, EXIT(ch, door)->to_room, "The %s closes quietly.", back->keyword);
            }
            else
              sendrf( 0, EXIT(ch, door)->to_room, "The door closes quietly.\n\r" );
          }             
    }
}


int has_key(charType *ch, int key)
{
  objectType *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (objects[o->nr].virtual == key)
      return(1);

  if (ch->equipment[HOLD])
    if (objects[ch->equipment[HOLD]->nr].virtual == key)
      return(1);

  return(0);
}


void do_lock(charType *ch, char *argument, int cmd)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  directionType *back;
  objectType *obj;


  twoArgument(argument, type, dir);

  if (!*type)
    send_to_char("Lock what?\n\r", ch);
  	else if( find_container( ch, argument, &obj ) )

    /* this is an object */

    if (obj->type != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
     else if (!IS_SET(obj->value[1], CONT_CLOSED))
      send_to_char("Maybe you should close it first...\n\r", ch);
    else if (obj->value[2] < 0)
      send_to_char("That thing can't be locked.\n\r", ch);
    else if (!has_key(ch, obj->value[2]))
      send_to_char("You don't seem to have the proper key.\n\r", ch);  
    else if (IS_SET(obj->value[1], CONT_LOCKED))
      send_to_char("It is locked already.\n\r", ch);
    else
    {
      SET_BIT(obj->value[1], CONT_LOCKED);
      send_to_char("*Cluck*\n\r", ch);
      act("$n locks $p - 'cluck', it says.", FALSE, ch, obj, 0, TO_ROOM);
    }
  else if ((door = find_door(ch, type, dir)) >= 0)

    /* a door, perhaps */

    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("You have to close it first, I'm afraid.\n\r", ch);
    else if (EXIT(ch, door)->key < 0)
      send_to_char("There does not seem to be any keyholes.\n\r", ch);
    else if (!has_key(ch, EXIT(ch, door)->key))
      send_to_char("You don't have the proper key.\n\r", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("It's already locked!\n\r", ch);
    else
    {
      SET_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
      if (EXIT(ch, door)->keyword)
        act("$n locks the $F.", 0, ch, 0,  EXIT(ch, door)->keyword,
          TO_ROOM);
      else
        act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*Click*\n\r", ch);
      /* now for locking the other side, too */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
        if((back = world[other_room].dirs[rev_dir[door]]))
          if (back->to_room == ch->in_room)
            SET_BIT(back->exit_info, EX_LOCKED);
    }
}


void do_unlock(charType *ch, char *argument, int cmd)
{
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  directionType *back;
  objectType *obj;


  twoArgument(argument, type, dir);

  if (!*type)
    send_to_char("Unlock what?\n\r", ch);
  	else if( find_container( ch, argument, &obj ) )

    /* this is an object */

    if (obj->type != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
     else if (!IS_SET(obj->value[1], CONT_CLOSED))
      send_to_char("Silly - it ain't even closed!\n\r", ch);
    else if (obj->value[2] < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
    else if (!has_key(ch, obj->value[2]))
      send_to_char("You don't seem to have the proper key.\n\r", ch);  
    else if (!IS_SET(obj->value[1], CONT_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all.\n\r", ch);
    else
    {
      REMOVE_BIT(obj->value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  else if ((door = find_door(ch, type, dir)) >= 0)

    /* it is a door */

    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("Heck.. it ain't even closed!\n\r", ch);
    else if (EXIT(ch, door)->key < 0)
      send_to_char("You can't seem to spot any keyholes.\n\r", ch);
    else if (!has_key(ch, EXIT(ch, door)->key))
      send_to_char("You do not have the proper key for that.\n\r", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("It's already unlocked, it seems.\n\r", ch);
    else
    {
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
      if (EXIT(ch, door)->keyword)
        act("$n unlocks the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
          TO_ROOM);
      else
        act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*click*\n\r", ch);
      /* now for unlocking the other side, too */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
        if((back = world[other_room].dirs[rev_dir[door]]))
          if (back->to_room == ch->in_room)
            REMOVE_BIT(back->exit_info, EX_LOCKED);
    }
}

void do_pick(charType *ch, char *argument, int cmd)
{
   byte percent;
  int door, other_room;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  directionType *back;
  objectType *obj;

  twoArgument(argument, type, dir);

  percent=number(1,101); /* 101% is a complete failure */

  if (percent > (ch->skills[SKILL_PICK_LOCK])) {
      send_to_char("You failed to pick the lock.\n\r", ch);
      return;
  }

  if (!*type)
    send_to_char("Pick what?\n\r", ch);
  	else if( find_container( ch, argument, &obj ) )

    /* this is an object */

    if (obj->type != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
     else if (!IS_SET(obj->value[1], CONT_CLOSED))
      send_to_char("Silly - it ain't even closed!\n\r", ch);
    else if (obj->value[2] < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
    else if (!IS_SET(obj->value[1], CONT_LOCKED))
      send_to_char("Oho! This thing is NOT locked!\n\r", ch);
    else if (IS_SET(obj->value[1], CONT_PICKPROOF))
      send_to_char("It resists your attempts at picking it.\n\r", ch);
    else
    {
      REMOVE_BIT(obj->value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  else if ((door = find_door(ch, type, dir)) >= 0)
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      send_to_char("You realize that the door is already open.\n\r", ch);
    else if (EXIT(ch, door)->key < 0)
      send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
      send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
      send_to_char("You seem to be unable to pick this lock.\n\r", ch);
    else
    {
      REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
      if (EXIT(ch, door)->keyword)
        act("$n skillfully picks the lock of the $F.", 0, ch, 0,
          EXIT(ch, door)->keyword, TO_ROOM);
      else
        act("$n picks the lock of the $F.", TRUE, ch, (objectType *)type, 0, TO_ROOM);
      send_to_char("The lock quickly yields to your skills.\n\r", ch);
      /* now for unlocking the other side, too */
      if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
        if((back = world[other_room].dirs[rev_dir[door]]))
          if (back->to_room == ch->in_room)
            REMOVE_BIT(back->exit_info, EX_LOCKED);
    }
}


void do_enter(charType *ch, char *argument, int cmd)
{
  	int 		door;
  	char 		buf[MAX_INPUT_LENGTH], tmp[MAX_STRING_LENGTH];

  	oneArgument( argument, buf );

  	if( *buf )  /* an argument was supplied, search for door keyword */
  	{
    	for (door = 0; door <= 5; door++)
      		if( EXIT(ch, door) )
        		if (EXIT(ch, door)->keyword)
          			if (!stricmp(EXIT(ch, door)->keyword, buf))
          			{
            			do_move(ch, "", ++door);
            			return;
          			}
    	sprintf(tmp, "There is no %s here.\n\r", buf);
    	send_to_char(tmp, ch);
   	}
  	else if (IS_SET(world[ch->in_room].flags, INDOORS))
      send_to_char("You are already indoors.\n\r", ch);
    else
    {
      	/* try to locate an entrance */
      	for( door = 0; door <= 5; door++ )
        	if( EXIT(ch, door) )
          		if( EXIT(ch, door)->to_room != NOWHERE )
            		if( !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
              			IS_SET(world[EXIT(ch, door)->to_room].flags, INDOORS))
            		{
              			do_move(ch, "", ++door);
              			return;
            		}
      	send_to_char("You can't seem to find anything to enter.\n\r", ch);
   	}
}


void do_leave(charType *ch, char *argument, int cmd)
{
  int door;

  void do_move(charType *ch, char *argument, int cmd);

  if (!IS_SET(world[ch->in_room].flags, INDOORS))
    send_to_char("You are outside.. where do you want to go?\n\r", ch);
  else
  {
    for (door = 0; door <= 5; door++)
      if (EXIT(ch, door))
        if (EXIT(ch, door)->to_room != NOWHERE)
          if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
            !IS_SET(world[EXIT(ch, door)->to_room].flags, INDOORS))
          {
            do_move(ch, "", ++door);
            return;
          }
    send_to_char("I see no obvious exits to the outside.\n\r", ch);
  }
}


void do_stand(charType *ch, char *argument, int cmd)
{

  switch(GET_POS(ch)) {
    case POSITION_STANDING : { 
      act("You are already standing.",FALSE, ch,0,0,TO_CHAR);
    } break;
    case POSITION_SITTING  : { 
      act("You stand up.", FALSE, ch,0,0,TO_CHAR);
      act("$n clambers on $s feet.",TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_STANDING;
    } break;
    case POSITION_RESTING  : { 
      act("You stop resting, and stand up.", FALSE, ch,0,0,TO_CHAR);
      act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_STANDING;
    } break;
    case POSITION_SLEEPING : { 
      act("You have to wake up first!", FALSE, ch, 0,0,TO_CHAR);
    } break;
    case POSITION_FIGHTING : { 
      act("Do you not consider fighting as standing?",FALSE, ch, 0, 0, TO_CHAR);
    } break;
    default : { 
      act("You stop floating around, and put your feet on the ground.",
        FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and puts $s feet on the ground.",
        TRUE, ch, 0, 0, TO_ROOM);
    } break;
  }
}


void do_sit(charType *ch, char *argument, int cmd)
{

  switch(GET_POS(ch)) {
    case POSITION_STANDING : {
      act("You sit down.", FALSE, ch, 0,0, TO_CHAR);
      act("$n sits down.", FALSE, ch, 0,0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
    case POSITION_SITTING  : {
      send_to_char("You'r sitting already.\n\r", ch);
    } break;
    case POSITION_RESTING  : {
      act("You stop resting, and sit up.", FALSE, ch,0,0,TO_CHAR);
      act("$n stops resting.", TRUE, ch, 0,0,TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
    case POSITION_SLEEPING : {
      act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
    } break;
    case POSITION_FIGHTING : {
      act("Sit down while fighting? are you MAD?", FALSE, ch,0,0,TO_CHAR);
    } break;
    default : {
      act("You stop floating around, and sit down.", FALSE, ch,0,0,TO_CHAR);
      act("$n stops floating around, and sits down.", TRUE, ch,0,0,TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
  }
}
      

void do_rest(charType *ch, char *argument, int cmd) 
{

  switch(GET_POS(ch)) {
    case POSITION_STANDING : {
      act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_RESTING;
    } break;
    case POSITION_SITTING : {
      act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_RESTING;
    } break;
    case POSITION_RESTING : {
      act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
    } break;
    case POSITION_SLEEPING : {
      act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
      } break;
    case POSITION_FIGHTING : {
      act("Rest while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    } break;
    default : {
      act("You stop floating around, and stop to rest your tired bones.",
        FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and rests.", FALSE, ch, 0,0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
  }
}


void do_sleep(charType *ch, char *argument, int cmd) 
{

  switch(GET_POS(ch)) {
    case POSITION_STANDING : 
    case POSITION_SITTING  :
    case POSITION_RESTING  : {
      send_to_char_han("You go to sleep.\n\r","ÀÚ·¯ °©´Ï´Ù. ¾ÆÀÌ Á¹·Á\n\r", ch);
      acthan("$n lies down and falls asleep.",
       "$n´ÔÀÌ ´¯´õ´Ï ÀáÀÌ µì´Ï´Ù.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
    } break;
    case POSITION_SLEEPING : {
      send_to_char_han("You are already sleeping.\n\r","ÄðÄð...\n\r", ch);
    } break;
    case POSITION_FIGHTING : {
      send_to_char_han("Sleep while fighting? are you MAD?\n\r",
        "½Î¿ì¸é¼­ ÀÜ´Ù±¸¿ä ? Áö±Ý Á¤½ÅÀÌ ÀÖ½À´Ï±î ?\n\r", ch);
    } break;
    default : {
      act("You stop floating around, and lie down to sleep.",
        FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and lie down to sleep.",
        TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
    } break;
  }
}


void do_wake(charType *ch, char *argument, int cmd)
{
  charType *tmp_char;
  char arg[MAX_STRING_LENGTH];


  oneArgument(argument,arg);
  if (*arg) {
    if (GET_POS(ch) == POSITION_SLEEPING) {
      act("You can't wake people up if you are asleep yourself!",
        FALSE, ch,0,0,TO_CHAR);
    } else {
      tmp_char = find_char_room(ch, arg);
      if (tmp_char) {
        if (tmp_char == ch) {
          act("If you want to wake yourself up, just type 'wake'",
            FALSE, ch,0,0,TO_CHAR);
        } else {
          if (GET_POS(tmp_char) == POSITION_SLEEPING) {
            if (IS_AFFECTED(tmp_char, AFF_SLEEP)) {
              act("You can not wake $M up!", FALSE, ch, 0, tmp_char, TO_CHAR);
            } else {
              act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR);
              GET_POS(tmp_char) = POSITION_SITTING;
              act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT);
            }
          } else {
            act("$N is already awake.",FALSE,ch,0,tmp_char, TO_CHAR);
          }
        }
      } else {
        send_to_char("You do not see that person here.\n\r", ch);
      }
    }
  } else {
    if (IS_AFFECTED(ch,AFF_SLEEP)) {
      send_to_char("You can't wake up!\n\r", ch);
    } else {
      if (GET_POS(ch) > POSITION_SLEEPING)
        send_to_char("You are already awake...\n\r", ch);
      else {
        send_to_char("You wake, and sit up.\n\r", ch);
        act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SITTING;
      }
    }
  }
}

