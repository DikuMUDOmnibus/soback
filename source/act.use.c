#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "comm.h"
#include "magics.h"
#include "interpreter.h"
#include "find.h"
#include "strings.h"
#include "variables.h"
#include "affect.h"

void do_quaff(charType *ch, char *argument, int cmd)
{
  	char 					buf[100];
  	objectType 	*	temp;
  	int 					i;
  	bool 					equipped;

  	equipped = FALSE;

  	oneArgument( argument, buf );

  	if( !(temp = find_obj_inven( ch, buf )) ) 
  	{
    	temp = ch->equipment[HOLD];
    	equipped = TRUE;
    	if( (temp==0) || !isoneof(buf, temp->name) ) 
		{
      		act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      		return;
    	}
  	}

  	if( temp->type != ITEM_POTION )
  	{
    	act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
    	return;
  	}

  	act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
  	act("You quaff $p which dissolves.",FALSE,ch,temp,0,TO_CHAR);

  	for( i = 1; i <= 3; i++ )
    	if( temp->value[i] >= 1 )
		{
  			if( !(spells[temp->value[i]].type & SPELL_POTION) )
  			{
  				DEBUG( "do_quaff> %s potion has invalid magic number( %d )", 
  						temp->name, temp->value[i] );
  				continue;
  			}
  			else
  			{
      			(*spells[temp->value[i]].func)
        			((int) temp->value[0], ch, "", SPELL_POTION, ch, 0);
			}
		}

  	if( equipped ) unequip_char(ch, HOLD);

  	extract_obj( temp, 1 );
}

void do_recite(charType *ch, char *argument, int cmd)
{
  	char 					buf[100];
  	objectType 	*	scroll; 
  	objectType				*	obj;
  	charType 	*	victim;
  	int 					i, bits = 0;
  	int 					spNr;
  	bool 					equipped;

  	equipped = FALSE;
  	obj = 0;
  	victim = 0;

  	argument = oneArgument(argument,buf);

  	if( !(scroll = find_obj_inven( ch, buf )) ) 
  	{
    	scroll = ch->equipment[HOLD];
    	equipped = TRUE;

    	if( (scroll==0) || !isoneof(buf, scroll->name) ) 
		{
      		act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      		return;
    	}
  	}
  	if( scroll->type != ITEM_SCROLL )
  	{
    	act("Recite is normally used for scroll's.",FALSE,ch,0,0,TO_CHAR);
    	return;
  	} 	

  	if( *argument ) 	
  	{
    	bits = find( argument, FIND_OBJ_INV | FIND_OBJ_EQUIP , ch, &victim, &obj );
    	if( bits == 0 ) 
		{
      		sendf( ch, "No such object around to recite the scroll on." );
      		return;
    	}
  	} 
  	else 
  	{
		for( i = 1; i < 4; i++ )
			if( scroll->value[i] == SPELL_WORD_OF_RECALL )
			{
				if( world[ch->in_room].virtual == ROOM_TEMPLE )
				{
					misbehave( ch ); return;
				}
			}
    	victim = ch;
  	}

  	act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
  	act("You recite $p which dissolves.",FALSE,ch,scroll,0,TO_CHAR);

  	for( i = 1; i < 4; i++ )
  	{
    	if( spNr = scroll->value[i], spNr >= 1 )
		{
  			if( (( bits == FIND_OBJ_INV )  && !( spells[spNr].targets & TAR_OBJ_INV ))
  	 		 || (( bits == FIND_OBJ_EQUIP) && !( spells[spNr].targets & TAR_OBJ_EQUIP )) )
  			{
				DEBUG( "do_recite> %d spell to object (SCROLL - %s)", spNr, scroll->name );
				continue;
			}

			if( (!bits && !( spells[spNr].targets & (TAR_CHAR_ROOM | TAR_SELF | TAR_SELF_ONLY)))
			  || !( spells[spNr].type & SPELL_SCROLL ) )
			{
				DEBUG( "do_recite> %d spell to self (SCROLL - %s)", spNr, scroll->name );
				continue;
			}

      		(*spells[scroll->value[i]].func)
      			((int)scroll->value[0], ch, "", SPELL_SCROLL, victim, obj );
	  	}
	}

  	if( equipped ) unequip_char(ch, HOLD);

  	extract_obj( scroll, 1 );
}

void do_use(charType *ch, char *argument, int cmd)
{
  	char 				buf[100];
	charType		*	vict;
	charType		*	next_vict;
	objectType			*	obj;
	objectType			*	stick;
  	int 				bits;
  	int 				level;
  	int 				spNr;

  	argument = oneArgument(argument,buf);

  	if( ch->equipment[HOLD] == 0 || !isoneof(buf, ch->equipment[HOLD]->name)) 
	{
    	act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);
    	return;
  	}

  	stick = ch->equipment[HOLD];

	spNr  = stick->value[3];
	level = stick->value[0];

	if( spNr <= 0 || !spells[spNr].func )
	{
		DEBUG( "do_use> [%s] has invalid spell info, nr == %d", buf, spNr );
		return;
	}

  	if( stick->type == ITEM_STAFF )
  	{
		act("$n taps $p three times on the ground.",TRUE, ch, stick, 0,TO_ROOM);
    	act("You tap $p three times on the ground.",FALSE,ch, stick, 0,TO_CHAR);

    	if( stick->value[2] <= 0) 
		{
      		sendf( ch, "The staff seems powerless.\n\r", ch);
			return;
		}
			
		if( !( spells[spNr].targets & TAR_IGNORE) )
		{
			if( spells[spNr].targets & (TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_ONLY) )
			{
				for( vict = world[ch->in_room].people; vict; vict = next_vict )
				{
					next_vict = vict->next_in_room;

					if( GET_POS( vict ) != POSITION_DEAD )
					{
						(*spells[spNr].func)( level, ch, "", SPELL_STAFF, vict, 0 );
					}
				}
			}
			else
			{
				DEBUG( "do_use> %s spell (SPELL_STAFF - %s)", 
													spells[spNr].name, stick->name );
			}
		}
		else
		{
			if( spells[stick->value[3]].type & SPELL_STAFF )
				(*spells[stick->value[3]].func)( level, ch, "", SPELL_WAND, vict, obj);
			else
			{
				TRACE( trace("do_use> %s spell to %s (SPELL_STAFF - %s)", 
										spells[spNr].name, argument, stick->name) );
				return;
			}
		}

		WAIT_STATE( ch, PULSE_ROUND * 3 / 4 );
      	stick->value[2]--;
    } 
	else if( stick->type == ITEM_WAND ) 
	{
      	if( stick->value[2] <= 0) 
		{
       		sendf( ch, "The wand seems powerless." );
	   		return;
   		}

    	bits = find( argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP
							, ch, &vict, &obj );
    	if( bits ) 
		{
      		if( bits == FIND_CHAR_ROOM && spells[spNr].targets & (TAR_CHAR_ROOM | TAR_SELF_ONLY))
			{
        		act("$n point $p at $N.", TRUE, ch, stick, vict, TO_ROOM);
        		act("You point $p at $N.",FALSE,ch, stick, vict, TO_CHAR);
      		} 
			else if( ((bits == FIND_OBJ_INV ) && (spells[spNr].targets & TAR_OBJ_INV ))
			      || ((bits == FIND_OBJ_ROOM) && (spells[spNr].targets & TAR_OBJ_ROOM)) ) 
			{
        		act("$n point $p at $P.", TRUE, ch, stick, obj, TO_ROOM);
        		act("You point $p at $P.",FALSE,ch, stick, obj, TO_CHAR);
      		}
	  		else
			{
				TRACE( trace("do_use> %s spell to %s (SPELL_WAND - %s)", 
										spells[spNr].name, argument, stick->name) );
				return;
			}


			if( spells[stick->value[3]].type & SPELL_WAND ) 
				(*spells[stick->value[3]].func)( level, ch, "", SPELL_WAND, vict, obj);
			else
			{
				TRACE( trace("do_use> %s spell to %s (SPELL_WAND - %s)", 
										spells[spNr].name, argument, stick->name) );
			}
			stick->value[2]--;
			WAIT_STATE( ch, PULSE_ROUND * 3 / 4 );
    	} 
		else 
		{
      		sendf( ch, "What should the wand be pointed at?" );
    	}
  	} 
	else 
	{
    	sendf( ch, "Use is normally only for wand's and staff's." );
  	}
}
