#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "defines.h"
#include "character.h"
#include "object.h"
#include "sockets.h"
#include "world.h"
#include "utils.h"
#include "mobile.h"
#include "limits.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "variables.h"
#include "strings.h"
#include "transfer.h"

static int get( charType *ch, objectType *obj, objectType *sub ) 
{
  	if( sub ) 
  	{
        if( sub->type == ITEM_CONTAINER) 
		{
          	if( IS_SET( sub->value[1], CONT_CLOSED) )
		 	{
            	sendf( ch, "%s is closed.", sub->name );
            	return 0;	
          	} 
		  	else if( IS_SET(sub->value[1],CONT_LOCKED) )
			{
            	sendf( ch, "%s is locked.", sub->name );
            	return 0;
          	} 
		}
		else
		{
			sendf( ch, "%s is not a container", sub->name );
			return 0;
		}
  	} 

    if( ch->carry_items + 1 > can_carry_number(ch) ) 
	{
		sendf( ch, "You can't get %s, too many items." , oneword( obj->name ) );
		return 0;
	}
		
   	if( (ch->carry_weight + obj->weight) > can_carry_weight(ch) ) 
	{
		sendf( ch, "You can't get %s, too much weight.", oneword( obj->name ) );
		return 0;
	}

    if( !can_get( ch, obj ) ) 
	{
		sendf( ch, "You can't take %s.", oneword( obj->name ) );
		return 0;
	}

	if( sub )
	{
		obj_from_obj( obj );
		obj_to_char( obj, ch );

		if( sub->carried_by == ch ) 
		{
			act("You get $p from $P.", 0, ch, obj, sub, TO_CHAR);
			act("$n gets $p from $s $P.", 1, ch, obj, sub, TO_ROOM);
		} 
		else 
		{
			act("You get $p from $P.", 0, ch, obj, sub, TO_CHAR);
			act("$n gets $p from $P.", 1, ch, obj, sub, TO_ROOM);
		}
	}
  	else 
  	{
    	obj_from_room( obj );
    	obj_to_char( obj, ch );

    	act("You get $p.", 0, ch, obj, 0, TO_CHAR);
    	act("$n gets $p.", 1, ch, obj, 0, TO_ROOM);
  	}

  	if( (obj->type == ITEM_MONEY) && (obj->value[0]>=1) )
  	{
    	obj_from_char( obj );
    	sendf( ch, "There was %d coins.", obj->value[0]);

    	GET_GOLD(ch) += obj->value[0];
    	extract_obj( obj, 1 );
  	}
  	return 1;
}

void do_get(charType *ch, char *argument, int cmd)
{
  	char 				arg1[MAX_STRING_LENGTH];
  	char 				arg2[MAX_STRING_LENGTH];
  	objectType 		*	rnext;
  	objectType 		*	rnext_next;
  	objectType 		*	inext;
  	objectType 		*	inext_next;
  	int 				sfound = 0, ofound = 0;
  	int					done  = 0;
  	int 				items = 0;
  	int					objnr = 1;
  	int					subnr = 1;
  	char			*	from  = 0;
  	char			*	what  = 0;

  	twoArgument( argument, arg1, arg2 );

  	if( !*arg1 )
  	{
  		sendf( ch, "Get what?" ); return;
  	}

	objnr = splitarg( arg1, &what );
	if( *arg2 ) subnr = splitarg( arg2, &from );

	if( objnr < 0 || subnr < -1 )
	{
   		sendf( ch, "Get what from what?" ); return;
   	}

   	for( ofound = sfound = 0, rnext = world[ch->in_room].contents; rnext; rnext = rnext_next )
   	{
   		rnext_next = rnext->next_content;

		if( from )	
		{
			if( isoneofp( from, rnext->name ) && can_see_obj( ch, rnext ) )
			{
				if( ++sfound < subnr && subnr ) continue;

				for( ofound = 0, inext = rnext->contains; inext; inext = inext_next )
				{
					inext_next = inext->next_content;

					if( (!*what || isoneofp( what, inext->name )) && can_see_obj( ch, inext ) )
					{
						if( ++ofound < objnr && objnr ) continue;

						items += get( ch, inext, rnext );
						done = 1;

						if( ofound == objnr ) break;
					}
				}

				if( sfound == subnr ) break;
			}
		}
		else
		{
			if( (!*what || isoneofp( what, rnext->name )) && can_see_obj( ch, rnext ) )
			{
				if( ++ofound < objnr && objnr ) continue;

				items += get( ch, rnext, 0 );
				done = 1;

				if( ofound == objnr ) break;
			}
		}
	}
	
	if( from )
		for( rnext = ch->carrying; rnext; rnext = rnext_next )
		{
			rnext_next = rnext->next_content;

			if( isoneofp( from, rnext->name ) && can_see_obj( ch, rnext ) )
			{
				if( ++sfound < subnr && subnr ) continue;

				for( ofound = 0, inext = rnext->contains; inext; inext = inext_next )
				{
					inext_next = inext->next_content;

					if( (!*what || isoneofp( what, inext->name )) && can_see_obj( ch, inext ) )
					{
						if( ++ofound < objnr && objnr ) continue;

						items += get( ch, inext, rnext ); done = 1;

						if( ofound == objnr ) break;
					}
				}
				if( sfound == subnr ) break;
			}
		}

	if( items ) sendf( ch, "Ok." );
	else if( !done )
		if( !*what )
		{
			if( !from ) sendf( ch, "You don't see anything here." );
			else		sendf( ch, "You don't see anything in %s.", from );
		}
		else
		{
			if( !from ) sendf( ch, "You don't see %s here.", what );
			else		sendf( ch, "You don't see %s in %s.", what, from );
		}
}				
					
void do_drop( charType * ch, char * argument, int cmd )
{
  	char 			arg[MAX_INPUT_LENGTH];
  	int 			amount;
  	objectType 	*	obj;
  	objectType 	*	next_obj;
  	char		*	what;
  	int				objNr;
  	int				found;
  	bool 			done;
  
  	argument = oneArgument(argument, arg);

  	if( isnumstr(arg) ) 
  	{
    	amount   = atoi(arg);
    	argument = oneArgument( argument, arg );

    	if( stricmp( "coins", arg ) != 0 && stricmp( "coin", arg ) != 0 ) 
		{
      		sendf( ch, "Drop %s coins? Then spell it out.", numfstr(amount) );
      		return;
    	}

    	if( amount < 0 ) 
		{
      		sendf( ch, "You can not do that!" );
      		return;
    	}

    	if( GET_GOLD(ch) < amount ) 
		{
      		sendf( ch, "You haven't got that many coins!" );
      		return;
    	}
    	if( amount >= 10 )
		{
      		act( "$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM );
      		obj = create_money(amount);
      		obj_to_room( obj, ch->in_room );
    	} 
		else 
		{
      		act("$n scatters some coins to winds...",FALSE,ch,0,0,TO_ROOM);
      		act("You scatter some coins to winds...",FALSE,ch,0,0,TO_CHAR);
    	}
    	GET_GOLD(ch) -= amount;

    	sendf( ch, "OK." );
    	return;
  	}

  	if( !*arg || (objNr = splitarg( arg, &what ), objNr < 0) ) 
  	{
    	sendf( ch, "Drop what?" );
		return;
  	}

    for( found = 0, done = 0, obj = ch->carrying; obj; obj = next_obj ) 
	{
        next_obj = obj->next_content;

		if( !*what || (isoneofp( what, obj->name ) && can_see_obj( ch, obj )) ) 
		{
			if( ++found < objNr && objNr ) continue;

        	if( !IS_SET( obj->extra, ITEM_NODROP) ) 
			{
          		if( can_see_obj(ch, obj) ) 
		  		{
            		sendf( ch, "You drop the %s.", oneword( obj->name ) );
          		} 
		  		else 
				{
            		sendf( ch, "You drop something." );
          		}
          		act( "$n drops $p.", 1, ch, obj, 0, TO_ROOM);
          		obj_from_char( obj);
          		obj_to_room( obj, ch->in_room );
          		done = TRUE;
        	} 
			else if( can_see_obj(ch, obj) ) 
			{
           		sendf( ch, "You can't drop the %s, it must be CURSED!", oneword(obj->name) );
           		done = TRUE;
		 	}

			if( objNr >= found ) break;
       	}
   	}
    if( !done ) sendf( ch, "You do not seem to have anything." );
}

void do_put( charType *ch, char *argument, int cmd )
{
  	char 				arg1[MAX_STRING_LENGTH];
  	char 				arg2[MAX_STRING_LENGTH];
  	objectType	  	* 	obj;
  	objectType   	* 	sub = 0;
  	objectType   	* 	obj_next;
  	int 				ofound;
  	int					sfound;
  	char		 	* 	to    = 0;
  	char		 	* 	what  = 0;
  	char		 	* 	mesg  = "Put what in what?";
  	int					items = 0;
	int				    objnr = 0; 
	int					subnr = 0;
	int					inventory = 0;

  	twoArgument(argument, arg1, arg2);

  	if( !*arg1 || !*arg2 ) 
  	{
  		sendf( ch, mesg ); return;
  	}
      		
	objnr = splitarg( arg1, &what );
	subnr = splitarg( arg2, &to );

	if( objnr < 0 || subnr <= 0 )
	{
		sendf( ch, mesg ); return;
	}
	
	for( ofound = sfound = 0, obj = ch->carrying; obj; obj = obj_next )
	{
		obj_next = obj->next_content;

		if( isoneofp( to, obj->name ) && can_see_obj( ch, obj ) )
			if( ++sfound < subnr && subnr ) continue;
			else 							
			{
				sub = obj; inventory = 1;
				break;
			}
	}

	if( !sub )
   		for( obj = world[ch->in_room].contents; obj; obj = obj_next )
   		{
			obj_next = obj->next_content;

			if( isoneofp( to, obj->name ) && can_see_obj( ch, obj ) )
				if( ++sfound < subnr && subnr ) continue;
				else
				{
					sub = obj;
					break;
				}
		}

	if( !sub )
	{
		sendf( ch, "You don't see %s here.", to );
		return;
	}
	
    if( sub->type == ITEM_CONTAINER) 
	{
		if( IS_SET( sub->value[1], CONT_CLOSED) )
		{
			sendf( ch, "%s is closed.", sub->name );
			return;	
		} 
		else if( IS_SET(sub->value[1],CONT_LOCKED) )
		{
			sendf( ch, "%s is locked.", sub->name );
			return;
		} 
	}
	else
	{
		sendf( ch, "%s is not a container", sub->name );
		return;
	}

	for( obj = ch->carrying; obj; obj = obj_next )
	{
		obj_next = obj->next_content;

		if( (!*what || isoneofp( what, obj->name )) && can_see_obj( ch, obj ) )
		{
			if( ++ofound < objnr && objnr ) continue;

			if( obj == sub ) 
			{
				sendf( ch, "You attempt to fold it into itself, but fail." );
			}
			else
			{
				if( sub->weight + obj->weight > sub->value[0] )
				{
					sendf( ch, "%s won't fit.", sub->name );
					return;
				}
				if( obj->type == ITEM_CONTAINER )
					sendf( ch, "%s(container)", oneword( obj->name ) );
				else
					sendf( ch, "Ok." );

				act( "$n puts $p in $P", TRUE, ch, obj, sub, TO_ROOM );		
	
				obj_from_char( obj );
				obj_to_obj( obj, sub );

                if( inventory ) ch->carry_weight += obj->weight;
				items++;
			}
			if( ofound == objnr ) break;
		}
	}
	if( !items ) sendf( ch, "You don't have it." );
}

void do_give(charType *ch, char *argument, int cmd)
{
  	char 					obj_name [80]; 	
  	char					vict_name[80]; 	
  	int 					amount;
  	int 					done   = 0;
  	int 					ofound = 0;
  	objectType 			* 	obj;
  	objectType 			* 	obj_next;
  	charType 			* 	vict;
  	char				*	what  = 0;
  	int						objnr = 0;

  	argument = oneArgument( argument, obj_name );

  	if( isnumstr(obj_name) )
  	{
    	amount = atoi(obj_name);
    	argument = oneArgument( argument, obj_name );

    	if( strcmp( "coins", obj_name ) && strcmp( "coin", obj_name))
    	{
      		sendf( ch, "If you wan't to give some coins to someone, spell it out.." ); return;
    	}
    	if( amount < 0 )
    	{
      		sendf( ch, "You can't do that!" ); return;
    	}
    	if( (GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < (IMO+1))))
    	{
      		sendf( ch, "You haven't got that many coins!" );
      		return;
    	}

    	argument = oneArgument(argument, vict_name);

    	if( !*vict_name)
    	{
      		sendf( ch, "To who?" );	return;
    	}
    	if( !(vict = find_char_room(ch, vict_name)) )
    	{
      		sendf( ch, "To who?" ); return;
    	}
		if( (vict->gold + amount) < vict->gold )
		{
			sendf( ch, "%s is full of gold.", IS_NPC(vict) ? vict->moved : vict->name );
			return;
		}
    	sendf( ch, "Ok.\n\r" );

    	sendf( vict, "%s gives you %s gold coins.", PERS(ch,vict), numfstr(amount) );
    	act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT);
    	if( IS_NPC(ch) || (GET_LEVEL(ch) < (IMO+1)) )
      		GET_GOLD(ch)-=amount;
    	else
     		log( "Give>%s gives %s coins to %s.",GET_NAME(ch), numfstr(amount), GET_NAME(vict));

    	GET_GOLD(vict)+=amount;
    	return;
  	}

  	argument = oneArgument( argument, vict_name );

  	if( !*obj_name || !*vict_name )
  	{
    	sendf( ch, "Give what to who?" );
    	return;
  	}
  	if( !(vict = find_char_room(ch, vict_name)) )
  	{
    	sendf( ch, "No one by that name around here.", ch);
    	return;
	}

	if( objnr = splitarg( obj_name, &what ), objnr < 0 )
	{
		sendf( ch, "Pardon me?" ); return;
	}
	
	for( obj = ch->carrying; obj; obj = obj_next )
	{
		obj_next = obj->next_content;

		if( (!*what || isoneofp( what, obj->name )) && can_see_obj( ch, obj ) )
		{
			if( ++ofound < objnr && objnr ) continue;

  			if( IS_SET(obj->extra, ITEM_NODROP) )
  			{
    			sendf( ch, "You can't let go of it! Yeech!!", ch);
  			}
  			else if( (1+vict->carry_items) > can_carry_number(vict) )
  			{
    			act("$N seems to have $S hands full.", 0, ch, 0, vict, TO_CHAR);
  			}
  			else if( obj->weight + vict->carry_weight > can_carry_weight(vict) )
  			{
    			act("$E can't carry that much weight.", 0, ch, 0, vict, TO_CHAR);
  			}
  			else
  			{
				obj_from_char(obj);
				obj_to_char(obj, vict);
				act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT);
				act("$n gives you $p.", 0, ch, obj, vict, TO_VICT);
				sendf( ch, "Ok." );

  				if( GET_LEVEL(ch) >= IMO)
     				log( "%s gives %d to %s.", 
	 					GET_NAME(ch),objects[obj->nr].virtual,GET_NAME(vict));
			}
			done++;
			if( ofound == objnr ) break;
		}
	}
  	if( !done ) sendf( ch, "You do not seem to have anything like that." );
}

void do_reload(charType *ch, char *argument, int cmd)
{
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  objectType *gun;
  objectType *ammo;

  if(GET_POS(ch) < POSITION_STANDING){
    send_to_char("You need to be standing still for that.\n\r",ch);
    return;
  }
  twoArgument(argument, arg1, arg2);
  if (*arg1) {
    if (*arg2) {
      gun=find_obj_inven( ch, arg1 );
      if (gun) {
        if( gun->type != ITEM_FIREWEAPON){
          send_to_char("You can't reload that!\n\r",ch);
          return;
        }
        ammo=find_obj_inven( ch, arg2 );
        if(ammo){
          if((1+objects[gun->nr].virtual)!=
            (objects[ammo->nr].virtual)){
            sprintf(buffer,"%s cannot be used as ammo for %s\n\r",arg2,arg1);
            send_to_char(buffer,ch);
            return;
          }
          gun->value[0]=gun->value[1];
          extract_obj( ammo, 1 );
          act("$n reloads $p",TRUE,ch,gun,0,TO_ROOM);
          send_to_char("You reload.\n\r",ch);
        } else {
          sprintf(buffer, "You dont have the %s.\n\r", arg2);
          send_to_char(buffer, ch);
        }
      } else {
        sprintf(buffer, "You dont have the %s.\n\r", arg1);
        send_to_char(buffer, ch);
      }
    } else {
      sprintf(buffer, "Reload %s with what?\n\r", arg1);
      send_to_char(buffer, ch);
    }
  } else {
    send_to_char("Reload what with what?\n\r",ch);
  }
}

