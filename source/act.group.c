#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "comm.h"
#include "find.h"
#include "limits.h"
#include "magics.h"
#include "utils.h"
#include "strings.h"
#include "affect.h"
#include "spec.rooms.h"

/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */

int circle_follow(charType *ch, charType * toFol )
{
 	charType * master;

 	for( master = toFol; master; master = master->master ) 
  		if( master == ch ) return(TRUE);

 	return(FALSE);
}


/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */

void stop_follower( charType * ch )
{
 	followType *j, *k;

	if( !ch->master )
	{
		DEBUG( "stop_follower> function called for who has no master." );
		return;
	}

 	if( IS_AFFECTED( ch, AFF_CHARM ) ) 
 	{
	  	act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
	  	act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
	  	act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);

  		if( affected_by_spell(ch, SPELL_CHARM_PERSON) )
   			affect_from_char(ch, SPELL_CHARM_PERSON);
 	} 
 	else 
 	{
  		act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
  		act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
  		act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
 	}

 	if( ch->master->followers->follower == ch) 
 	{ /* Head of follower-list? */
  		k = ch->master->followers;
  		ch->master->followers = k->next;
  		errFree( k );
 	} 
 	else 
 	{ /* locate follower who is not head of list */
  		for(k = ch->master->followers; k->next->follower!=ch; k=k->next)  ;

  		j = k->next;
  		k->next = j->next;
  		errFree( j ) ;
 	}

 	ch->master = 0;
 	REMOVE_BIT( ch->affects, AFF_CHARM | AFF_GROUP );
}

/* Called when a character that follows/is followed dies */

void die_follower( charType * ch )
{
 	followType 	*j, *k;

 	if( ch->master ) stop_follower(ch);

 	for( k = ch->followers; k; k = j ) 
 	{
  		j = k->next;
  		stop_follower( k->follower );
 	}
 	REMOVE_BIT( ch->affects, AFF_CHARM | AFF_GROUP );
}

/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */

void add_follower(charType *ch, charType *leader)
{
 	followType 	*	fol;

 	ch->master = leader;

 	REMOVE_BIT(ch->affects, AFF_GROUP);

 	fol = (followType *)errCalloc( sizeof(followType ));

 	fol->follower = ch;
 	fol->next = leader->followers;
 	leader->followers = fol;

 	act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
 	act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
 	act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

int sum_gr_member( charType * ch )
{
	int					 sum = 0;
	followType * fol;
	charType   * master;

	if( !IS_AFFECTED( ch, AFF_GROUP ) ) return 0;

	if( master = ch->master, !master ) master = ch;

 	if( IS_AFFECTED( master ,AFF_GROUP ) && ( master->in_room==ch->in_room) )
  		sum = 1;
 	else
  		sum = 0;

 	for( fol = master->followers; fol; fol = fol->next )
  		if( !IS_NPC(fol->follower) 
  		  && IS_AFFECTED(fol->follower,AFF_GROUP)
  		  && (fol->follower->in_room == ch->in_room) ) sum++;

  	return sum;
}

int is_same_group( charType * ch, charType * vict )
{
	followType 		* fol;
	charType   		* master;

	if( !IS_AFFECTED( ch, AFF_GROUP ) ) return 0;

	if( master = ch->master, !master ) master = ch;

	if( master == vict ) return 1;

 	for( fol = master->followers; fol; fol = fol->next )
  		if( (vict == fol->follower) && (IS_AFFECTED( fol->follower, AFF_GROUP )) ) return 1;

  	return 0;
}

void group_gain(charType *ch, charType *victim)
{
 	char 					buf[256], buf2[BUFSIZ];
 	int 					no_members, share, levsum;
 	charType 			*	master;
 	followType 			*	follow;

	if( !(master = ch->master) ) master = ch;

 	if( IS_AFFECTED( master, AFF_GROUP ) && ( master->in_room == ch->in_room) )
 	{
  		no_members = 1;	levsum = GET_LEVEL( master );
 	}
 	else
 	{
  		no_members = 0;	levsum = 0;
  	}


 	for( follow = master->followers; follow; follow = follow->next )
  		if(IS_AFFECTED(follow->follower,AFF_GROUP)&&(follow->follower->in_room==ch->in_room))
  		{
   			no_members++; levsum+=GET_LEVEL(follow->follower);
  		}

 	if( IS_AFFECTED(master, AFF_GROUP) && (master->in_room == ch->in_room)) 
 	{
  		if((no_members >= 1)&&(levsum >= 1))
  		{
   			share = (GET_EXP(victim)*GET_LEVEL(master))/levsum;
  		} 
  		else share = 0;
		
		if( IS_NPC( master ) || master->desc )
		{
  			sprintf(buf,"You receive %s experience.", numfstr( share ) );
  			sprintf(buf2,"당신은 %s 점의 경헙치를 얻었습니다.", numfstr( share ) );
  			acthan(buf, buf2, FALSE, master, 0, 0, TO_CHAR);
  			gain_exp(master,share);
  			change_alignment(master,victim);
  		}
 	}

 	for( follow = master->followers; follow ; follow= follow->next) 
 	{
  		if( IS_AFFECTED( follow->follower,AFF_GROUP) && ( follow->follower->in_room == ch->in_room)) 
  		{
   			if( (no_members >= 1) && (levsum >= 1) )
   			{
    			share = (GET_EXP(victim)*GET_LEVEL( follow->follower))/levsum;
   			} 
   			else share = 0;

			if( IS_NPC( follow->follower) || follow->follower->desc )
			{
   				sprintf( buf,"You receive %s experience.", numfstr(share) );
   				sprintf( buf2,"당신은 %s 점의 경헙치를 얻었습니다.", numfstr(share) );
   				acthan( buf, buf2, FALSE,  follow->follower, 0, 0, TO_CHAR );
   				gain_exp( follow->follower, share );
   				change_alignment( follow->follower, victim );
   			}
  		}
 	}
}

void do_ungroup(charType *ch, char *argument, int cmd)
{
 	followType *curr, *next;

	if( IS_NPC( ch ) || IS_AFFECTED(ch, AFF_CHARM) )
	{
		send_to_char("The thought of leaving your master makes you weep.\n\r", ch); 
		act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
		return;
	}

 	if( ch->master && !IS_AFFECTED(ch, AFF_CHARM) ) stop_follower( ch );

 	for( curr = ch->followers; curr; curr = next ) 
 	{
  		next = curr->next;
		if( IS_NPC( curr->follower ) || IS_AFFECTED(curr->follower, AFF_CHARM) )
		{
			sendf( curr->follower, "The thought of leaving your master makes you weep." ); 
			act("$n bursts into tears.", FALSE, curr->follower, 0, 0, TO_ROOM);
			continue;
		}
  		stop_follower( curr->follower );
 	}
 	REMOVE_BIT(ch->affects, AFF_CHARM | AFF_GROUP );
 	sendf( ch, "Your group is dismissed." );
}

void do_group(charType *ch, char *argument, int cmd)
{
  	char 				name[256];
  	charType 		*	victim, * k;
  	followType 		*	f;
  	bool 				found = 0;
	char				cls;

	if( IS_NPC(ch) ) return;

  	oneArgument(argument, name);

  	if( !*name ) 
  	{
    	if (!IS_AFFECTED(ch, AFF_GROUP)) 
		{
      		send_to_char("But you are a member of no group?!\n\r", ch);
    	} 
		else 
		{
      		send_to_char("Your group consists of:\n\r", ch);
      		if( ch->master ) k = ch->master;
      		else 			 k = ch;

      		if( IS_AFFECTED(k, AFF_GROUP) )
	  		{
				if( !IS_NPC(k) )  cls =  pc_class_types[k->class][0];
				else  			  cls = npc_class_types[k->class][0];

        		sendf( ch, "    [ %4d/%4d %4d/%4d %4d/%4d .. %c %2d ] %s (Head of group)",
         					GET_HIT(k),GET_MAX_HIT(k),GET_MANA(k),
         					GET_MAX_MANA(k),GET_MOVE(k),GET_MAX_MOVE(k), 
							cls, k->level, GET_NAME(k) );

      			for( f = k->followers; f; f = f->next )
        			if( IS_AFFECTED(f->follower, AFF_GROUP) )
					{
						if( !IS_NPC(f->follower) ) cls = pc_class_types[f->follower->class][0];
						else                       cls = npc_class_types[f->follower->class][0];

        				sendf( ch, "    [ %4d/%4d %4d/%4d %4d/%4d .. %c %2d ] %s",
         					GET_HIT(f->follower),GET_MAX_HIT(f->follower),GET_MANA(f->follower),
         					GET_MAX_MANA(f->follower),GET_MOVE(f->follower),GET_MAX_MOVE(f->follower), 
							cls, f->follower->level, GET_NAME(f->follower) );
        			}
    		}
		}
    	return;
  	}

  	if( GET_POS(ch) <= POSITION_SLEEPING )
  	{
  		sendf( ch, "Ah~~ You dream about great powerful group." );
  		return;
  	}

	if( stricmp( name, "all" ) == 0 )
	{
   		for( f = ch->followers; f; f = f->next ) 
  		{
      		if( !IS_AFFECTED( f->follower, AFF_GROUP ) && f->follower->in_room == ch->in_room ) 
			{
        		act("$n is now a group member.",   FALSE, f->follower, 0, 0, TO_ROOM);
        		act("You are now a group member.", FALSE, f->follower, 0, 0, TO_CHAR);
        		SET_BIT(f->follower->affects, AFF_GROUP);
      		}
	   	}

		if( !IS_AFFECTED( ch, AFF_GROUP ) )
		{
       		act("$n is now a group member.",   FALSE, ch, 0, 0, TO_ROOM);
       		act("You are now a group member.", FALSE, ch, 0, 0, TO_CHAR);
       		SET_BIT(ch->affects, AFF_GROUP);
   		}
		return;
	}
			
  	if( victim = find_char_room(ch, name), !victim ) 
  	{
    	send_to_char("No one here by that name.\n\r", ch);
  	}
  	else 
  	{

    	if( ch->master ) 
		{
      		act("You can not enroll group members without being head of a group.", 
	  				FALSE, ch, 0, 0, TO_CHAR);
      		return;
    	}

    	if( victim == ch ) found = TRUE;
    	else 
		{
      		for( f = ch->followers; f; f = f->next ) 
	  		{
        		if( f->follower == victim ) 
				{
          			found = TRUE;
          			break;
        		}
      		}
    	}
    
		if( found ) 
		{
      		if( IS_AFFECTED( victim, AFF_GROUP ) ) 
	  		{
        		act( "$n has been kicked out of the group!",     FALSE, victim, 0, ch, TO_ROOM );
        		act( "You are no longer a member of the group!", FALSE, victim, 0, 0,  TO_CHAR );
        		REMOVE_BIT(victim->affects, AFF_GROUP);
      		} 
	  		else 
			{
        		act( "$n is now a group member.",   FALSE, victim, 0, 0, TO_ROOM );
        		act( "You are now a group member.", FALSE, victim, 0, 0, TO_CHAR );
        		SET_BIT(victim->affects, AFF_GROUP);
      		}
    	} 
		else 
		{
      		act("$N must follow you, to enter the group", FALSE, ch, 0, victim, TO_CHAR);
    	}
  	}
}

void do_follow( charType * ch, char * argument, int cmd )
{
  	char 				name[160];
  	charType 		*	leader ;

  	oneArgument(argument, name);

  	if( *name ) 
  	{
    	if(!(leader = find_char_room(ch, name))) 
		{
      		sendf( ch, "I see no person by that name here!" ); return;
    	}
  	} 
  	else 
  	{
    	sendf( ch, "Who do you wish to follow?" ); return;
  	}

  	if( IS_AFFECTED(ch, AFF_CHARM) && (ch->master) ) 
  	{
    	act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
  	} 
  	else 
  	{
    	if( leader == ch ) 
		{
      		if( !ch->master ) 
	  		{
        		sendf( ch, "You are already following yourself." ); return;
      		}
      		stop_follower( ch );
    	} 
		else 
		{
      		if( circle_follow(ch, leader) ) 
	  		{
        		act("Sorry, but following in 'loops' is not allowed", FALSE, ch, 0, 0, TO_CHAR);
        		return;
      		}
      		if( ch->master ) stop_follower(ch);

 			REMOVE_BIT(ch->affects, AFF_GROUP );

      		add_follower(ch, leader);
    	}
  	}
}

void do_split( charType * ch, char * argu, int cmd )
{
	followType		*		f;
	int						amount;
	int						gave;
	int						divide;

	if( IS_NPC(ch) ) return;

   	if( !IS_AFFECTED(ch, AFF_GROUP) )
	{
   		sendf( ch, "But you are a member of no group?!" ); return;
   	} 

   	if( ch->master )
   	{
   		sendf( ch, "Only master of group can split." ); return;
   	}

   	if( f = ch->followers, !f )
   	{
   		sendf( ch, "Yeah! We should split money. But, You are too loney to do that." );
   		return;
   	}

   	if( getnumber( argu, &amount ) <= 0 )
   	{
   		sendf( ch, "Hmmm... You can split only coins." ); return;
   	}

	if( amount <= 0 )
	{
		sendf( ch, "How about steal coins from your group member??.." ); return;
	}

	if( amount > ch->gold )
	{
		sendf( ch, "You can't afford it." ); return;
	}

	coins_from_char( ch, amount );

	divide = amount / sum_gr_member( ch );

	for( gave = 0; f; f = f->next )
	{
        if(  !IS_NPC( f->follower )   
          && f->follower->in_room == ch->in_room
          && coins_to_char( f->follower, divide ) )
        {
            sendf( f->follower, "You recieved %d coins.", divide );
            sendf( ch, "You give %d coins to %s.", divide, f->follower );
            gave += divide;
        }
		else
		{
			sendf( ch, "%s is full of money.", f->follower->name );
		}
	}

	if( amount - gave > 0 )
	{
		coins_to_char( ch, amount - gave );
		sendf( ch, "%s coins remains.", numfstr( amount - gave ) );
	}
}
