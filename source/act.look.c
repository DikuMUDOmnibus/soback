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
#include "world.h"
#include "mobile.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "sockets.h"
#include "magics.h"
#include "limits.h"
#include "variables.h"
#include "misc.h"
#include "strings.h"
#include "affect.h"
#include "page.h"

static char * exits[] = 
{
    "North",
    "East ",
    "South",
    "West ",
    "Up   ",
    "Down "
};

void do_exits( charType * ch, char * argument, int cmd )
{
  	int 			door;
  	char 			buf[MAX_STRING_LENGTH];

  	*buf = '\0';

  	for (door = 0; door <= 5; door++)
    	if( EXIT(ch, door) )
      		if(   EXIT(ch, door)->to_room != NOWHERE 
			  && (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) || OMNI(ch)) )
			{
				sprintf( buf + strlen( buf ), "%s", exits[door] );

				if( OMNI(ch) )
				{
					if( IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR) )
						sprintf( buf + strlen( buf ), "-door"  );
					else
						sprintf( buf + strlen( buf ), "-----"  );
				}

        		if( IS_DARK(EXIT(ch, door)->to_room) && !OMNI(ch) )
          			sprintf( buf + strlen(buf), "- Too dark to tell\n\r" );
        		else
          			sprintf( buf + strlen(buf), "- %s\n\r",	world[EXIT(ch, door)->to_room].name);
			}

  	sendf( ch, "Obvious exits:" );

  	if( *buf )
    	sendf( ch, "%s", buf );
  	else
    	sendf( ch, "None." );
}

void auto_exits( charType *ch )
{
  	int 		i, door, found = 0;
  	char 		buf[MAX_STRING_LENGTH];

  	static char * exits[] = 
  	{
	    "North",
	    "East ",
	    "South",
	    "West ",
	    "Up   ",
	    "Down "
	};
	static exitseq[] = { 4, 3, 0, 2, 1, 5 };

	buf[0] = 0;

  	for ( i = 0; i < 6; i++)
  	{
  		door = exitseq[i];
    	if (EXIT(ch, door))
		{
      		if (EXIT(ch, door)->to_room != NOWHERE && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
				{
          			sprintf(buf + strlen(buf), " %s ", exits[door] );
		  			found++;
		  			continue;
		  		}
		}
		sprintf( buf + strlen(buf), "       " );
  	}

  	if( found ) sendf( ch, "[%s]", buf ); 
  	else
    	sendf( ch, "[ None ]" );
}

char * find_extrd( char * word, exdescriptionType * list )
{
  	exdescriptionType *	exd;

  	for( exd = list; exd; exd = exd->next )
  	{
    	if( isoneof( word, exd->keyword) ) 
			return exd->description;
	}
	return 0;
}

static char * append_flags( charType * ch, objectType * object, char * buffer )
{
	if (IS_OBJ(object,ITEM_INVISIBLE)) 
	{
	   strcat(buffer,"(invisible)");
	}
	if (IS_OBJ(object,ITEM_EVIL) && IS_AFFECTED( ch, AFF_DETECT_EVIL )) 
	{
	   strcat(buffer,"..It glows red!");
	}
/*
	if (IS_OBJ(object,ITEM_GOOD) && IS_AFFECTED( ch, AFF_DETECT_GOOD )) 
	{
	   strcat(buffer,"..It glows white!");
	}
*/
	if (IS_OBJ(object,ITEM_GLOW)) 
	{
	  	strcat(buffer,"..It has a soft glowing aura!");
	}
	if (IS_OBJ(object,ITEM_HUM)) 
	{
	  	strcat(buffer,"..It emits a faint humming sound!");
	}
	return buffer;
}

void show_room_objs( objectType * obj, charType * ch, int num )
{
	char		buffer[MAX_STRING_LENGTH];

	buffer[0] = 0;

	if( obj->roomd )
	{
		if( num > 1 ) sprintf( buffer, "[%2d] %s", num, obj->roomd );
  		else		  sprintf( buffer,  "     %s", obj->roomd );

  		append_flags( ch, obj, buffer );
  		sendf( ch, "%s", buffer );
  	}
}

void show_in_objs( objectType * obj, charType * ch, int num )
{
	char		buffer[MAX_STRING_LENGTH];

	buffer[0] = 0;

	if( obj->wornd )
	{
		if( num > 1 ) sprintf( buffer, "[%2d] %s", num, obj->wornd );
  		else		  sprintf( buffer,  "     %s", obj->wornd );

  		append_flags( ch, obj, buffer );
  		sendf( ch, "%s", buffer );
  	}
}

void show_equip_obj( objectType * obj, charType * ch, int at )
{
	char		buffer[MAX_STRING_LENGTH];

	buffer[0] = 0;

	if( obj->wornd )
	{
		sprintf( buffer, "%s%s", where[at], obj->wornd );

  		append_flags( ch, obj, buffer );
  		sendf( ch, "%s", buffer );
  	}
}

void show_read_obj( objectType * obj, charType * ch )
{
	char		buffer[ MAX_STRING_LENGTH ];

	if( obj->type == ITEM_NOTE )
	{
	  	if( obj->usedd )
	  	{
			strcpy( buffer, "There is something written upon it:\n\r\n\r" );
			strcat( buffer, obj->usedd );
			print_page( ch, buffer );
	  	}
	  	else
			sendf( ch, "It's blank." );
	}
	else
		sendf( ch, "It's not a note." );
}

void show_exam_obj( objectType * obj, charType * ch )
{
	char 		 	at[100];

	if( obj->type != ITEM_DRINKCON )
	{
		if     ( obj->carried_by ) strcpy ( at, "Inventory ] " );
		else if( obj->equiped_by ) strcpy ( at, "Equipment ] " );
		else if( obj->in_obj )     sprintf( at, "In %s ] ", onewordc( obj->in_obj->name ) );
		else 					   strcpy ( at, "" );

	  	sendf( ch, "%s %s looks %s.", at, obj->wornd ? obj->wornd : "It", itemstatus( obj ) );
   	}
   	else
   	{
   		sendf( ch, "%s looks like a drink container.", obj->wornd ? obj->wornd : "It" );
   	}
}

void list_obj_to_char( objectType * list, charType *ch, int mode, int show ) 
{
  	objectType 		* 	obj;
  	bool 				found;
  	int					i;

  	found = FALSE;

  	init_search_records();

	for ( obj = list ; obj ; obj = obj->next_content ) 
	{ 
		if( can_see_obj(ch,obj) ) 
		{
			record_found_object( obj, 1 );
			if( !found ) found = TRUE;
		}
	}

	if( found )
	{
		for( i = 0; i < objs_in_found_table; i++ )
		{
			if( mode == 1 )
				show_room_objs( found_objects[i].obj, ch, found_objects[i].num );
			if( mode == 2 )
				show_in_objs( found_objects[i].obj, ch, found_objects[i].num );
		}
	}
  	if( !found & mode == 2) sendf( ch, "Nothing" );
}

void show_room_char( charType * ch, charType * vict )
{
	char 			buffer[MAX_STRING_LENGTH];
  	char		*	aura = 0;
  	char		*	charmie = 0;

   	if( !can_see( ch, vict ) )
   	{
   		if(  IS_AFFECTED(vict, AFF_HIDE) 
		 && !IS_AFFECTED( vict, AFF_AMBUSH ) 
         && !IS_AFFECTED( vict, AFF_MORPETH ) ) 
		{
   			if( IS_AFFECTED( ch, AFF_SENSE_LIFE ) )
       			if( GET_LEVEL( vict ) < (ch->level + 5) )
       				send_to_char("You sense a hidden life form in the room.\n\r", ch);
   		}
   		return;
   	}

   	if( !(vict->roomd) || (GET_POS( vict ) != vict->defaultpos) )
	{
   		if( !IS_NPC(vict) ) 
  		{
       		strcpy( buffer, GET_NAME( vict ) );
       		strcat( buffer, " " );
       		if( GET_TITLE( vict ) ) strcat(buffer,GET_TITLE( vict ));
   		} 
  		else 
  		{
       		strcpy( buffer, vict->moved );
       		capitalize(buffer);
   		}

   		switch( GET_POS( vict ) ) 
  		{
       		case POSITION_STUNNED  : 
       				strcat(buffer," is lying here, stunned."); break;
       		case POSITION_INCAP    : 
       				strcat(buffer," is lying here, incapacitated."); break;
       		case POSITION_MORTALLYW: 
       				strcat(buffer," is lying here, mortally wounded."); break;
      		case POSITION_DEAD     : 
       				strcat(buffer," is lying here, dead."); break;
       		case POSITION_STANDING : 
       				strcat(buffer," is standing here."); break;
       		case POSITION_SITTING  : 
       				strcat(buffer," is sitting here.");  break;
       		case POSITION_RESTING  : 
       				strcat(buffer," is resting here.");  break;
       		case POSITION_SLEEPING : 
       				strcat(buffer," is sleeping here."); break;
       		case POSITION_FIGHTING :
       				if( vict->fight ) 
	  				{
           				strcat(buffer," is here, fighting ");
           				if( vict->fight == ch ) strcat(buffer," YOU!");
           				else 
						{
           					if( vict->in_room == vict->fight->in_room )
               					if( IS_NPC(vict->fight) ) strcat(buffer, vict->fight->moved);
               					else 	
									strcat(buffer, GET_NAME(vict->fight));
           					else
               					strcat(buffer, "someone who has already left.");
           				}
       				} 
	  				else /* NIL fighting pointer */
           				strcat(buffer," is here struggling with thin air.");
       				break;
       		default : strcat(buffer," is floating here."); break;
   		}
   	}
   	else
   	{
		buffer[0] = 0;

   		if( IS_AFFECTED( vict, AFF_INVISIBLE ) ) strcat( buffer, "*" );

   		strcat( buffer, vict->roomd );
   	}
              
	remove_newline( buffer );

   	if( IS_AFFECTED(vict,AFF_INVISIBLE) ) strcat(buffer," (invisible)");

   	if( IS_AFFECTED(ch, AFF_DETECT_EVIL) ) 
  	{
		if     ( IS_DEVIL(vict) )  aura = "(Black Aura)";
   		else if( IS_EVIL(vict)  )  aura = "(Red   Aura)";
   	}
   	if( IS_AFFECTED(ch, AFF_DETECT_GOOD) ) 
  	{
		if     ( IS_SAINT(vict) )  aura = "(White Aura)";
   		else if( IS_GOOD(vict)  )  aura = "(Blue  Aura)";
   	}

   	if( IS_AFFECTED( vict, AFF_CHARM ) ) charmie = "(Charmie)";

	if( charmie && aura )   sendf( ch, "%s %s %s", aura, charmie, buffer );
	else if( charmie )		sendf( ch, "%s %s", charmie, buffer );
	else if( aura ) 		sendf( ch, "%s %s", aura, buffer );
	else					sendf( ch, "%s", buffer );

   	if( IS_AFFECTED( vict, AFF_SANCTUARY ) && can_see( ch, vict ) )
	{
		if( IS_NPC(vict) )
			sendf( ch, "%s glows with a bright light!", vict->moved ); 
		else
			sendf( ch, "%s glows with a bright light!", vict->name ); 
	}
}

void show_equip_char( charType * ch, charType * vict )
{
  	char 			buffer[MAX_STRING_LENGTH];
  	int 			i, found, percent;

   	if( vict->description ) send_to_char( vict->description, ch );
   	else 
	{
   		act( "You see nothing special about $m.", FALSE, vict, 0, ch, TO_VICT );
   	}

   	if( GET_MAX_HIT(vict) > 0 ) percent = (100*GET_HIT(vict))/GET_MAX_HIT(vict);
   	else 					 percent = -1; /* How could MAX_HIT be < 1?? */

   	if( IS_NPC(vict) ) strcpy( buffer, vict->moved );
   	else 			strcpy( buffer, GET_NAME(vict) );

   	if( percent >= 100 )    strcat(buffer, " is in an excellent condition.");
   	else if (percent >= 90) strcat(buffer, " has a few scratches.");
   	else if (percent >= 75) strcat(buffer, " has some small wounds and bruises.");
   	else if (percent >= 50) strcat(buffer, " has quite a few wounds.");
   	else if (percent >= 30) strcat(buffer, " has some big nasty wounds and scratches.");
   	else if (percent >= 15) strcat(buffer, " looks pretty hurt.");
   	else if (percent >= 0) 	strcat(buffer, " is in an awful condition.");
   	else 					strcat(buffer, " is bleeding awfully from big wounds.");

   	sendf( ch, "%s", buffer );

   	found = FALSE;

   	for( i = 0; i < MAX_WEAR; i++ ) 
	{
   		if( vict->equipment[i] ) 
     		if( can_see_obj(ch,vict->equipment[i]) ) 
       			found = TRUE;
   	}

   	if( found ) 
	{
		if( IS_NPC(ch) ) 	sendf( ch, "\n\r%s is using:", vict->moved );
		else 				sendf( ch, "\n\r%s is using:", vict->name );

   		for( i = 0; i < MAX_WEAR; i++ ) 
  		{
       		if( vict->equipment[i] ) 
			{
       			if( can_see_obj( ch, vict->equipment[i] ) ) 
	  			{
           			show_equip_obj( vict->equipment[i], ch, i );
       			}
       		}
   		}
   	}
}

void list_room_chars( charType * ch, charType * list )
{
  	charType * vict;

  	for( vict = list; vict ; vict = vict->next_in_room) 
  	{
    	if( (ch != vict) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) || (can_see(ch,vict))) )
      		show_room_char( ch, vict ); 
  	}
}

void do_look( charType * ch, char *argument, int cmd )
{
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  int keyword_no;
  int j, bits, temp;
  bool found;
  objectType *tmp_object, *found_object;
  charType *tmp_char;
  char *tmp_desc;

  static const char *keywords[]= { 
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "\n" };

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POSITION_SLEEPING)
    send_to_char_han("You can't see anything but stars!\n\r",
      "별들이 아름답게 빛나고 있습니다.\n\r",  ch);
  else if (GET_POS(ch) == POSITION_SLEEPING)
    send_to_char_han("You can't see anything, you're sleeping!\n\r",
      "아마도 지금 꿈을 꾸는게 아닌지요 ??\n\r", ch);
  else if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char_han("You can't see a damn thing, you're blinded!\n\r",
      "앞을 볼 수가 없습니다! 눈이 멀었습니다!\n\r",  ch);
  else if ( IS_DARK(ch->in_room) && !OMNI(ch) && !IS_AFFECTED(ch, AFF_INFRAVISION) )
    send_to_char_han("It is pitch black...\n\r", "너무 깜깜합니다..\n\r",ch);
  else {
    argument = onefword(argument, arg1 );
	argument = onefword(argument, arg2 );
    keyword_no = isinlist( arg1, keywords );

    if( (keyword_no == -1) && *arg1) {
      keyword_no = 7;
      strcpy(arg2, arg1); /* Let arg2 become the target object (arg1) */
    }
	else if( keyword_no == -1 && !*arg1 ) keyword_no = 8;

    found = FALSE;
    tmp_object = 0;
    tmp_char   = 0;
    tmp_desc   = 0;

    switch(keyword_no) {
      /* look <dir> */
      case 0 :
      case 1 :
      case 2 : 
      case 3 : 
      case 4 :
      case 5 : {   

        if (EXIT(ch, keyword_no)) {

          if (EXIT(ch, keyword_no)->description) {
            send_to_char(EXIT(ch, keyword_no)-> description, ch);
          } else {
            send_to_char_han("You see nothing special.\n\r",
               "별다른 것이 없습니다.\n\r", ch);
          }

          if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) && 
            (EXIT(ch, keyword_no)->keyword)) {
              sprintf(buffer, "The %s is closed.\n\r",
                oneword(EXIT(ch, keyword_no)->keyword));
              send_to_char(buffer, ch);
          }  else {
            if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_ISDOOR) &&
                EXIT(ch, keyword_no)->keyword) {
              sprintf(buffer, "The %s is open.\n\r",
                oneword(EXIT(ch, keyword_no)->keyword));
              send_to_char(buffer, ch);
            }
          }
        } else {
            send_to_char_han("Nothing special there...\n\r",
               "거기엔 특별한 것이 없습니다 ... \n\r", ch);
        }
      }
      break;

      /* look 'in'  */
      case 6: {
        if (*arg2) {

          bits = find( arg2, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, 
		  			ch, &tmp_char, &tmp_object);

          if (bits) { /* Found something */
            if( tmp_object->type == ITEM_DRINKCON)
            {
              if (tmp_object->value[1] <= 0) {
                acthan("It is empty.", "비어 있습니다.", FALSE, ch,0,0,TO_CHAR);
              } else {
                temp=((tmp_object->value[1]*3)/tmp_object->value[0]);
                sprintf(buffer,"It's %sfull of a %s liquid.\n\r",
                fullness[temp],color_liquid[tmp_object->value[2]]);
                send_to_char(buffer, ch);
              }
            } else if( tmp_object->type == ITEM_CONTAINER) {
              if (!IS_SET(tmp_object->value[1],CONT_CLOSED)) {
                send_to_char(oneword(tmp_object->name), ch);
                switch (bits) {
                  case FIND_OBJ_INV :
                    send_to_char(" (carried) : \n\r", ch);
                    break;
                  case FIND_OBJ_ROOM :
                    send_to_char(" (here) : \n\r", ch);
                    break;
                  case FIND_OBJ_EQUIP :
                    send_to_char(" (used) : \n\r", ch);
                    break;
                }
                list_obj_to_char( tmp_object->contains, ch, 2, TRUE );
              }
              else
                send_to_char("It is closed.\n\r", ch);
            } else {
              send_to_char("That is not a container.\n\r", ch);
            }
          } else { /* wrong argument */
            send_to_char("You do not see that item here.\n\r", ch);
          }
        } else { /* no argument */
          send_to_char("Look in what?!\n\r", ch);
        }
      }
      break;

      /* look 'at'  */
      case 7 : {

        if( *arg2 ) 
		{
          	bits = find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
                 FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);

          	if( tmp_char ) 
		  	{
            	show_equip_char( ch, tmp_char );
            	if( ch != tmp_char ) 
				{
              		acthan("$n looks at you.", "$n님이 당신을 바라봅니다.",
                 			TRUE, ch, 0, tmp_char, TO_VICT);
              		acthan("$n looks at $N.", "$n님이 $N님을 바라봅니다.",
                 			TRUE, ch, 0, tmp_char, TO_NOTVICT);
            	}
            	return;
          	}


          /* Search for Extra Descriptions in room and items */

          /* Extra description in room?? */
          if( !found ) {
            tmp_desc = find_extrd(arg2, world[ch->in_room].extrd);
            if( tmp_desc ) {
              sendf( ch, "%s", tmp_desc );
              return; /* RETURN SINCE IT WAS A ROOM DESCRIPTION */
              /* Old system was: found = TRUE; */
            }
          }
          /* Search for extra descriptions in items */
          /* Equipment Used */
          if (!found) {
            for (j = 0; j< MAX_WEAR && !found; j++) {
              if (ch->equipment[j]) {
                if (can_see_obj(ch,ch->equipment[j])) {
                  tmp_desc = find_extrd(arg2, ch->equipment[j]->extrd);
                  if (tmp_desc) {
              		sendf( ch, "%s", tmp_desc );
                    found = TRUE;
                  }
                }
              }
            }
          }
          /* In inventory */
          if (!found) {
            for(tmp_object = ch->carrying; 
              tmp_object && !found; 
              tmp_object = tmp_object->next_content) {
              if( can_see_obj(ch, tmp_object) ) {
                tmp_desc = find_extrd(arg2, tmp_object->extrd);
                if (tmp_desc) {
              		sendf( ch, "%s", tmp_desc );
                  found = TRUE;
                }
              }
            }
          }

          /* Object In room */
          if (!found) {
            for(tmp_object = world[ch->in_room].contents; tmp_object && !found; 
              	tmp_object = tmp_object->next_content) 
			{
              if( can_see_obj(ch, tmp_object) ) 
			  {
                tmp_desc = find_extrd(arg2, tmp_object->extrd);
                if( tmp_desc ) 
				{
              		sendf( ch, "%s", tmp_desc );
                  found = TRUE;
                }
              }
            }
          }

          if( bits ) 
		  {
            if( !found )
              show_exam_obj( found_object, ch );
          } else if (!found) {
            send_to_char_han("You do not see that here.\n\r",
               "그런 것은 여기에 없습니다.\n\r", ch);
          }
        } else {
          /* no argument */

          send_to_char_han("Look at what?\n\r",
             "무엇을 바라보라고요 ?\n\r", ch);
        }
      }
      break;

      /* look ''    */ 
      case 8 : 
	  {
        sendf( ch, world[ch->in_room].name );
        if( !IS_SET(ch->act, PLR_BRIEF) )
          send_to_char(world[ch->in_room].description, ch);
		auto_exits( ch );
        list_obj_to_char( world[ch->in_room].contents, ch, 1, FALSE );
        list_room_chars( ch, world[ch->in_room].people );
      }
      break;

      /* wrong arg  */
      case -1 : 
        send_to_char("Sorry, I didn't understand that!\n\r", ch);
        break;
    }
  }
}

void do_read( charType *ch, char *argument, int cmd)
{
  	char 			name[100];
  	int 			bits;
  	charType 	*	tmp_char;
  	objectType 	*	obj;

	oneArgument( argument, name );

  	bits = find( name, 
  			FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &obj);

  	if( obj )
  	{
  		if( obj->type != ITEM_NOTE )
  		{
  			sendf( ch, "It's not a note." );
  		}
  		else
  			show_read_obj( obj, ch );
  	}
  	else
  		sendf( ch, "You don't see anything like that." );
}

void do_examine( charType * ch, char * argument, int cmd )
{
  	char 			name[100], buf[100];
  	int 			bits;
  	charType 	*	tmp_char;
  	objectType 	*	tmp_object;
  	char		*	what;

  	oneArgument(argument, name);

  	if( !*name )
  	{
    	sendf( ch, "Examine what?" );
    	return;
  	}
  	else if( splitarg( argument, &what ) == 0 && !*what )
  	{
  		objectType	*	obj;
  		int				i;

  		for( i = 0; i < MAX_WEAR; i++ )
  		{
  			if( obj = ch->equipment[i], obj )
  			{
  				sendf( ch, "%15s looks %s.", onewordc( obj->name), itemstatus( obj ) );
  			}
  		}
  		return;
  	}

  	bits = find(name, 
  			FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  	if( tmp_object ) 
  	{
  		show_exam_obj( tmp_object, ch );

    	if(( tmp_object->type == ITEM_DRINKCON) || ( tmp_object->type == ITEM_CONTAINER)) 
		{
      		send_to_char("When you look inside, you see:\n\r", ch);
      		sprintf( buf, "in %s", argument );
      		do_look(ch,buf,COM_LOOK);
    	} 
		else 
		{
      		if( tmp_object->type == ITEM_FIREWEAPON )
        		sendf( ch, "There are %d shots left.", tmp_object->value[0]);
    	}
  	}
}

void do_equipment( charType * ch, char * argument, int cmd )
{
	int 		j;

  	send_to_char_han("You are using:\n\r", "쓰고 있는 물건들:\n\r", ch);

  	for( j = 0; j < MAX_WEAR; j++ ) 
  	{
    	if( ch->equipment[j] ) 
		{
      		if( can_see_obj( ch, ch->equipment[j] ) ) 
	  		{
        		show_equip_obj( ch->equipment[j], ch, j );
      		} 
	  		else 
			{
        		sendf(ch, "%sSomething.", where[j] );
      		}
    	}
		else
		{
			sendf( ch, "%s----" , where[j] );
		}
  	}
}

void do_inventory( charType * ch, char * argument, int cmd )
{
  	send_to_char_han("You are carrying:\n\r", "가지고 있는 물건들:\n\r", ch);
	list_obj_to_char( ch->carrying, ch, 2, TRUE );
}

void do_peek( charType * ch, char * argument, int cmd )
{
	int				i, found = 0;
	char	 		name[100];
	charType	*	vict;
  	objectType 	*	tmp_obj;

	oneArgument( argument, name );

	if( !*name )
	{
		sendf( ch, "Peek who?" ); return;
	}

	if( vict = find_char_room( ch, name ), !vict )
	{
		sendf( ch, "You don't see any such thing here." ); return;
	}

    sendf( ch, "You attempt to peek at the inventory:" );

	if( (number( 1, 100 ) + (ch->level - vict->level)) <= 10 )
	{
		act( "$n attempts to peek at $N's inventory.", TRUE, ch, 0, vict, TO_ROOM );
		act( "$n attempts to peek at your inventory.", TRUE, ch, 0, vict, TO_CHAR );
	}

  	init_search_records();

   	for( tmp_obj = vict->carrying; tmp_obj; tmp_obj = tmp_obj->next_content ) 
  	{
    	if( can_see_obj( ch, tmp_obj ) && number( 1, 101 ) < ch->skills[SKILL_PEEK] ) 
		{
			record_found_object( tmp_obj, 1 );
       		found = TRUE;
      	}
   	}

	if( found )
	{
		for( i = 0; i < objs_in_found_table; i++ )
		{
			show_in_objs( found_objects[i].obj, ch, found_objects[i].num );
		}
	}
   	if( !found ) sendf( ch, "You can't see anything." );
}
