#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "defines.h"
#include "character.h"
#include "object.h"
#include "world.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "limits.h"
#include "fight.h"
#include "variables.h"
#include "strings.h"
#include "affect.h"
#include "transfer.h"


void perform_wear(charType *ch, objectType *obj_object, int keyword)
{

  	switch(keyword) 
	{
    case WEAR_LIGHT :
      act("$n light $p and holds it.", FALSE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_FINGER_R : 
    case WEAR_FINGER_L : 
      act("$n wears $p on $s finger.", TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_NECK_1 : 
    case WEAR_NECK_2 : 
      act("$n wears $p around $s neck.", TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_BODY : 
      act("$n wears $p on $s body.", TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_HEAD : 
      act("$n wears $p on $s head.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_LEGS : 
      act("$n wears $p on $s legs.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_FEET : 
		act("$n wears $p on $s feet.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_HANDS : 
      act("$n wears $p on $s hands.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_ARMS : 
      act("$n wears $p on $s arms.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_ABOUT : 
      act("$n wears $p about $s body.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_WAISTE : 
      act("$n wears $p about $s waist.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_WRIST_R : 
    case WEAR_WRIST_L : 
      act("$n wears $p around $s wrist.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WIELD : 
      act("$n wields $p.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case HOLD : 
      act("$n grabs $p.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_SHIELD : 
      act("$n starts using $p as a shield.", TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_KNEE_R :
    case WEAR_KNEE_L :
      act("$n wears $p around $s knee.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
    case WEAR_ABOUTLEGS :
      act("$n wears $p about $s legs.",TRUE, ch, obj_object,0,TO_ROOM);
      break;
  	}
}

void wear(charType *ch, objectType *obj_object, int keyword)
{
  	char buffer[MAX_STRING_LENGTH];
  	int  level ;

  	level = obj_object->level ;
  	if( level < IMO && GET_LEVEL(ch) < level )
	{
    	sendf( ch, "You can use %s from %d level.", obj_object->wornd, level);
    	return;
  	}

  	switch(keyword) 
	{
    case WEAR_LIGHT:
		if( obj_object->type != ITEM_LIGHT ) 
			sendf( ch, "You can not use that as your light source." );
		else if( ch->equipment[WEAR_LIGHT] )
        	sendf( ch, "You are already holding a light source." );
      	else 
	  	{
        	sendf( ch, "Ok." );
        	perform_wear(ch,obj_object,keyword);
        	obj_from_char(obj_object);
        	equip_char(ch,obj_object, WEAR_LIGHT);
        	if( obj_object->value[2] ) world[ch->in_room].light++;
      	}
    	break;

    case WEAR_FINGER_R: 
    case WEAR_FINGER_L: 
	{
      	if( IS_WEAR(obj_object,ITEM_WEAR_FINGER) ) 
		{
        	if((ch->equipment[WEAR_FINGER_L]) && (ch->equipment[WEAR_FINGER_R])) 
          		send_to_char( "You are already wearing something on your fingers.\n\r", ch);
			else 
			{
          		perform_wear(ch,obj_object,keyword);
          		if( ch->equipment[WEAR_FINGER_R] ) 
				{
            		sendf( ch, "You put the %s on your left finger.",oneword(obj_object->name));
            		obj_from_char(obj_object);
            		equip_char(ch, obj_object, WEAR_FINGER_L);
          		} 
				else 
				{
            		sendf( ch, "You put the %s on your right finger.", oneword(obj_object->name));
            		obj_from_char(obj_object);
            		equip_char(ch, obj_object, WEAR_FINGER_R);
          		}
        	}
      	} 
		else 
        	send_to_char("You can't wear that on your finger.\n\r", ch);
    } break;
    case WEAR_NECK_1: 
    case WEAR_NECK_2: 
	{
      if ( IS_WEAR(obj_object,ITEM_WEAR_NECK)) {
        if ((ch->equipment[WEAR_NECK_1]) && (ch->equipment[WEAR_NECK_2])) {
          send_to_char("You can't wear any more around your neck.\n\r", ch);
        } else {
          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          if( ch->equipment[WEAR_NECK_1] ) {
            obj_from_char(obj_object);
            equip_char(ch, obj_object, WEAR_NECK_2);
          } else {
            obj_from_char(obj_object);
            equip_char(ch, obj_object, WEAR_NECK_1);
          }
        }
      } else {
              send_to_char("You can't wear that around your neck.\n\r", ch);
      }
    } break;
    case WEAR_BODY: {
      if ( IS_WEAR(obj_object,ITEM_WEAR_BODY)) {
        if (ch->equipment[WEAR_BODY]) {
          send_to_char("You already wear something on your body.\n\r", ch);
        } else {
          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch,  obj_object, WEAR_BODY);
        }
      } else {
        send_to_char("You can't wear that on your body.\n\r", ch);
      }
    } break;
    case WEAR_HEAD: {
      if ( IS_WEAR(obj_object,ITEM_WEAR_HEAD)) {
        if (ch->equipment[WEAR_HEAD]) {
          send_to_char("You already wear something on your head.\n\r", ch);
        } else {
          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_HEAD);
        }
      } else {
        send_to_char("You can't wear that on your head.\n\r", ch);
      }
    } break;
    case WEAR_LEGS: {
      if ( IS_WEAR(obj_object,ITEM_WEAR_LEGS)) {
        if (ch->equipment[WEAR_LEGS]) {
          send_to_char("You already wear something on your legs.\n\r", ch);
        } else {
          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_LEGS);
        }
      } else {
        send_to_char("You can't wear that on your legs.\n\r", ch);
      }
    } break;
    case WEAR_FEET: {
      if ( IS_WEAR(obj_object,ITEM_WEAR_FEET)) {
        if (ch->equipment[WEAR_FEET]) {
          send_to_char("You already wear something on your feet.\n\r", ch);
        } else {
          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_FEET);
        }
      } else {
        send_to_char("You can't wear that on your feet.\n\r", ch);
      }
    } break;
    case WEAR_HANDS: {
      if ( IS_WEAR(obj_object,ITEM_WEAR_HANDS)) {
        if (ch->equipment[WEAR_HANDS]) {
          send_to_char("You already wear something on your hands.\n\r", ch);
        } else {
          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_HANDS);
        }
      } else {
        send_to_char("You can't wear that on your hands.\n\r", ch);
      }
    } break;
    case WEAR_ARMS: {
      if ( IS_WEAR(obj_object,ITEM_WEAR_ARMS)) {
        if (ch->equipment[WEAR_ARMS]) {
          send_to_char("You already wear something on your arms.\n\r", ch);
        } else {
          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_ARMS);
        }
      } else {
        send_to_char("You can't wear that on your arms.\n\r", ch);
      }
    } break;
    case WEAR_ABOUT: {
      if ( IS_WEAR(obj_object,ITEM_WEAR_ABOUT)) {
        if (ch->equipment[WEAR_ABOUT]) {
          send_to_char("You already wear something about your body.\n\r", ch);
        } else {
          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_ABOUT);
        }
      } else {
        send_to_char("You can't wear that about your body.\n\r", ch);
      }
    } break;
    case WEAR_WAISTE: {
      if ( IS_WEAR(obj_object,ITEM_WEAR_WAISTE)) {
        if (ch->equipment[WEAR_WAISTE]) {
          send_to_char("You already wear something about your waiste.\n\r",
            ch);
        } else {
          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch,  obj_object, WEAR_WAISTE);
        }
      } else {
        send_to_char("You can't wear that about your waist.\n\r", ch);
      }
    } break;
    case WEAR_WRIST_R: 
    case WEAR_WRIST_L: 
	{
      if ( IS_WEAR(obj_object,ITEM_WEAR_WRIST)) {
        if ((ch->equipment[WEAR_WRIST_L]) && (ch->equipment[WEAR_WRIST_R])) {
          send_to_char(
            "You already wear something around both your wrists.\n\r", ch);
        } else {
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          if (ch->equipment[WEAR_WRIST_R]) {
            sprintf(buffer, "You wear the %s around your left wrist.\n\r", 
              oneword(obj_object->name));
            send_to_char(buffer, ch);
            equip_char(ch,  obj_object, WEAR_WRIST_L);
          } else {
            sprintf(buffer, "You wear the %s around your right wrist.\n\r", 
              oneword(obj_object->name));
            send_to_char(buffer, ch);
            equip_char(ch, obj_object, WEAR_WRIST_R);
          }
        }
      } else {
        send_to_char("You can't wear that around your wrist.\n\r", ch);
      }
    } break;

    case WEAR_KNEE_R:
    case WEAR_KNEE_L:
	{
        if ( IS_WEAR(obj_object,ITEM_WEAR_KNEE)) {
         if ((ch->equipment[WEAR_KNEE_R]) && (ch->equipment[WEAR_KNEE_L])) {
    send_to_char("You already wear something around your both knees.\n\r", ch);
        } else {
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
        if (ch->equipment[WEAR_KNEE_R]) {
              sprintf(buffer, "You put the %s on your left knee.\n\r",
              oneword(obj_object->name));
              send_to_char(buffer, ch);
              equip_char(ch, obj_object, WEAR_KNEE_L);
          } else {
              sprintf(buffer, "You put the %s on your right knee.\n\r",
              oneword(obj_object->name));
              send_to_char(buffer, ch);
              equip_char(ch, obj_object, WEAR_KNEE_R);
            }
         }
       } else {
          send_to_char("You cant'wear that on around your both knees.\n\r", ch);       }
     } break;

    case WEAR_ABOUTLEGS: {
      if ( IS_WEAR(obj_object,ITEM_WEAR_ABOUTLEGS)) {
        if (ch->equipment[WEAR_ABOUTLEGS]) {
          send_to_char("You already wear something about your legs.\n\r", ch);
        } else {
          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_ABOUTLEGS);
        }
      } else {
        send_to_char("You can't wear that about your legs.\n\r", ch);
      }
    } break;
    case WIELD:
      if ( IS_WEAR(obj_object,ITEM_WIELD)) {
        if (ch->equipment[WIELD]) {
          send_to_char("You are already wielding something.\n\r", ch);
        } else {
          /* Cleric execption has been removed, and is temporarily placed */
          /* at the end of this file                                      */

          if( obj_object->weight > str_apply(ch)->wield_w ) {
            send_to_char("It is too heavy for you to use.\n\r",ch);
          } else {
            send_to_char("OK.\n\r", ch);
            perform_wear(ch,obj_object,keyword);
            obj_from_char(obj_object);
            equip_char(ch, obj_object, WIELD);
          }
        }
      } else {
        send_to_char("You can't wield that.\n\r", ch);
      }
      break;

    case HOLD:
      if( IS_WEAR(obj_object,ITEM_HOLD)) {
        if (ch->equipment[HOLD]) {
          send_to_char("You are already holding something.\n\r", ch);
        } else {
          /* Cleric execption has been removed, and is temporarily placed */
          /* at the end of this file                                      */

          send_to_char("OK.\n\r", ch);
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, HOLD);
        }
      } else {
        send_to_char("You can't hold this.\n\r", ch);
      }
      break;
    case WEAR_SHIELD: {
      if ( IS_WEAR(obj_object,ITEM_WEAR_SHIELD)) {
        if ((ch->equipment[WEAR_SHIELD])) {
          send_to_char(
            "You are already using a shield\n\r", ch);
        } else {
          perform_wear(ch,obj_object,keyword);
          sprintf(buffer, "You start using the %s.\n\r", 
            oneword(obj_object->name));
          send_to_char(buffer, ch);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_SHIELD);
        }
      } else {
        send_to_char("You can't use that as a shield.\n\r", ch);
      }
    } break;
    case -1: {
      sprintf(buffer,"Wear %s where?.\n\r", oneword(obj_object->name));
      send_to_char(buffer, ch);
    } break;
    case -2: {
      sprintf(buffer,"You can't wear the %s.\n\r", oneword(obj_object->name));
      send_to_char(buffer, ch);
    } break;
    default: {
      log("Unknown type called in wear.");
    } break;
  }
}

static int unwear( charType * ch, int where )
{
	objectType 		* obj;

	if( obj = ch->equipment[where], !obj ) return -3;

	if( can_carry_number(ch) > ch->carry_items ) 
	{
		if( obj->type == ITEM_TRAP )
		{
			sendf( ch, "Oops.. You are trapped!" );
			act( "$n tried to unequipment $p. But $n failed.", 0, ch, obj, 0, TO_ROOM );
			return -2;
		}
		else
		{
       		obj_to_char( unequip_char(ch, where), ch );

   			if( obj->type == ITEM_LIGHT)
            	if( obj->value[2] )
            		world[ch->in_room].light--;

   			act( "You stop using $p.", FALSE, ch, obj, 0, TO_CHAR );
   			act( "$n stops using $p.", TRUE,  ch, obj, 0, TO_ROOM );
   			return 1;
		}
   	} 
	else 
	{
   		sendf( ch, "You can't carry that many items." );
	}
	return -1;
}

static int where_wear( charType * ch, objectType *obj )
{
/*
int found = 0;
#define CANEQ(a, b) (  IS_WEAR(obj, (a)) && ++found && !ch->equipment[(b)])
*/

#define CANEQ(a, b) (  IS_WEAR(obj, (a)) )

  	if( CANEQ(ITEM_WEAR_FINGER, WEAR_FINGER_R))		return( WEAR_FINGER_R );
  	if( CANEQ(ITEM_WEAR_FINGER, WEAR_FINGER_L))		return( WEAR_FINGER_L );
  	if( CANEQ(ITEM_WEAR_NECK, WEAR_NECK_1)) 		return( WEAR_NECK_1 );
  	if( CANEQ(ITEM_WEAR_NECK, WEAR_NECK_2)) 		return( WEAR_NECK_2 );
  	if( CANEQ(ITEM_WEAR_KNEE, WEAR_KNEE_R)) 		return( WEAR_KNEE_R );
  	if( CANEQ(ITEM_WEAR_KNEE, WEAR_KNEE_L)) 		return( WEAR_KNEE_L );
  	if( CANEQ(ITEM_WEAR_ABOUTLEGS, WEAR_ABOUTLEGS)) return( WEAR_ABOUTLEGS );
  	if( CANEQ(ITEM_WEAR_SHIELD, WEAR_SHIELD))		return( WEAR_SHIELD );
  	if( CANEQ(ITEM_WEAR_WRIST, WEAR_WRIST_R))		return( WEAR_WRIST_R );
  	if( CANEQ(ITEM_WEAR_WRIST, WEAR_WRIST_L))		return( WEAR_WRIST_L );
  	if( CANEQ(ITEM_WEAR_WAISTE, WEAR_WAISTE))		return( WEAR_WAISTE );
  	if( CANEQ(ITEM_WEAR_ARMS, WEAR_ARMS))			return( WEAR_ARMS );
  	if( CANEQ(ITEM_WEAR_HANDS, WEAR_HANDS))			return( WEAR_HANDS );
  	if( CANEQ(ITEM_WEAR_FEET, WEAR_FEET))			return( WEAR_FEET );
  	if( CANEQ(ITEM_WEAR_LEGS, WEAR_LEGS))			return( WEAR_LEGS );
  	if( CANEQ(ITEM_WEAR_ABOUT, WEAR_ABOUT))			return( WEAR_ABOUT );
  	if( CANEQ(ITEM_WEAR_BODY, WEAR_BODY))			return( WEAR_BODY );
  	if( CANEQ(ITEM_WEAR_HEAD, WEAR_HEAD))			return( WEAR_HEAD );
  	if( CANEQ(ITEM_WIELD, WIELD))					return( WIELD );
  	if( obj->type == ITEM_LIGHT )			return( WEAR_LIGHT ); 
  	return (-2);
}

void do_wear( charType * ch, char * argument, int cmd ) 
{
	char 				arg1[MAX_STRING_LENGTH];
	char 				arg2[MAX_STRING_LENGTH];
	objectType  	*   obj; 
	objectType  	*   next_obj;
	int 				keyword = - 1;
	int 				done    = 0;
	int					found   = 0;
	int					objnr   = 0;
	char			 *  what    = 0;
	int					equip[MAX_WEAR];

	static const char * keywords[ MAX_WEAR + 1 ] = 
	{
		"light", 
		"finger right", 
		"finger left", 	  	
		"neck right", 	  	
		"neck left",
	  	"body", 	  	
		"head", 	  	
		"legs", 	  	
		"feet", 	  	
		"hands",
	  	"arms", 
		"shield",
		"about", 		
		"waiste", 		
		"wrist right", 
		"wrist left",
  		"wield", 
  		" ", 		
  		"knee right", 
  		"knee left", 
  		"aboutlegs",
  		"\n"
	};

  	twoArgument(argument, arg1, arg2);

  	if( !*arg1 ) 
  	{
  		sendf( ch, "Wear what?" ); return;
  	}

	if( objnr = splitarg( arg1, &what ), objnr < 0 )
	{
		sendf( ch, "Wear what???" ); return;
	}

  	memset( equip, 0, sizeof(int) * MAX_WEAR );

    for( obj = ch->carrying ; obj ; obj = next_obj )
	{
        next_obj = obj->next_content;

        if( !*what || (isoneofp( what, obj->name ) && can_see_obj( ch, obj )) )
		{
			if( ++found < objnr && objnr ) continue;

			if( *arg2 )
			{
				if( keyword = isinlistp( arg2, keywords ), keyword == -1 )
					sendf( ch, "%s is an unkown body location.", arg2 );
				else
					wear( ch, obj, keyword );
			}
			else
			{
				if( keyword = where_wear( ch, obj ), keyword >= 0 )
		  		{
					if( !equip[keyword] )
					{
						equip[keyword] = 1;

						if( !*what )
						{
							if( !ch->equipment[keyword] ) wear( ch, obj, keyword );
						}
						else 
						{ 
							if( ch->equipment[keyword] )
							{
								if( (  keyword == WEAR_NECK_1 
							  		|| keyword == WEAR_KNEE_R 
							  		|| keyword == WEAR_FINGER_R 
							  		|| keyword == WEAR_WRIST_R   )
								  && !ch->equipment[keyword+1] ) equip[++keyword] = 1;
							}
								
							if( ch->equipment[keyword] && unwear( ch, keyword ) < 0 ) return;
							wear(ch , obj, keyword );
						}
					}
					else
					{
						if( (    keyword == WEAR_NECK_1 
							  || keyword == WEAR_KNEE_R 
							  || keyword == WEAR_FINGER_R 
							  || keyword == WEAR_WRIST_R   )
							&& !equip[++keyword] )
						{
							equip[keyword] = 1;

							if( !*what )
							{
								if( !ch->equipment[keyword] ) wear( ch, obj, keyword );
							}
							else 
							{ 
								if( ch->equipment[keyword] && unwear( ch, keyword ) < 0 ) return;
								wear(ch , obj, keyword );
							}
						}
					}
				}
				else if( keyword == -2 && *what )
				{
					sendf( ch, "You can't not wear %s.", oneword( obj->name ) ); return;
				}
			}
			done++;
		  	if( found == objnr ) break;
        }
    }

	if( !done )  
	{
		if( *what ) sendf( ch, "You don't have it." );
		else		sendf( ch, "You don't have anything to wear." );
	}
}

void do_wield( charType * ch, char * argument, int cmd ) 
{
	char 				arg1[MAX_STRING_LENGTH];
	char 				arg2[MAX_STRING_LENGTH]; 	
	objectType   	*   obj; 	

	twoArgument(argument, arg1, arg2);

  	if( *arg1 ) 
  	{
    	obj = find_obj_inven( ch, arg1 );

    	if( obj && IS_WEAR( obj, ITEM_WIELD ) ) 
		{
			if( ch->equipment[WIELD] && unwear( ch, WIELD ) < 0 ) return;
			wear(ch, obj, WIELD);
		}
    	else 	  sendf( ch, "You do not seem to have the '%s'.", arg1 );
  	} 
  	else sendf( ch, "Wield what?" );
}

void do_grab(charType *ch, char *argument, int cmd)
{
  	char 				arg1[MAX_STRING_LENGTH];
  	char 				arg2[MAX_STRING_LENGTH];
  	objectType	  	*	obj;

  	twoArgument(argument, arg1, arg2);

  	if( *arg1 ) 
  	{
    	obj = find_obj_inven( ch, arg1 );
    	if( obj ) 
		{
      		if( obj->type == ITEM_LIGHT )
	  		{
				if( ch->equipment[WEAR_LIGHT] && unwear( ch, WEAR_LIGHT ) < 0 ) return;
        		wear(ch, obj, WEAR_LIGHT );
			}
      		else
	  		{
				if( ch->equipment[HOLD] && unwear( ch, HOLD ) < 0 ) return;
        		wear(ch, obj, HOLD );
			}
    	} 
		else sendf( ch, "You do not seem to have the '%s'.",arg1 );
  	}
  	else sendf( ch, "Hold what?" );
}

void do_remove( charType *ch, char *argument, int cmd )
{
  	char 				arg1[MAX_STRING_LENGTH];
  	objectType *	obj;
  	int  				i;
	int                 done   = 0;
  	char			  * what   = 0;
	int                 objnr  = 0;
    int                 ofound = 0;;

  	oneArgument(argument, arg1);

  	if( !*arg1 )
  	{
  		sendf( ch, "Remove what?" ); return;
  	}

	if( objnr = splitarg( arg1, &what ), objnr < 0 )
	{
		sendf( ch, "Remove what???" ); return;
	}

	for( i = 0; i < MAX_WEAR; i++ )
	{
		obj = ch->equipment[i];

		if( obj && (!*what || ( isoneofp( what, obj->name ) && can_see_obj( ch, obj))) )
		{

			if( ++ofound < objnr && objnr ) continue;

			if( unwear( ch, i ) == -1 ) return;

			done++;
			if( ofound == objnr ) break;
      	}
	}

	if( !done )
	{
		if( *what ) sendf( ch, "You are not using it." );
		else	    sendf( ch, "You are not using anything." );
	}
}
