#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ctype.h>

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "world.h"
#include "comm.h"
#include "limits.h"
#include "interpreter.h"
#include "utils.h"
#include "sockets.h"
#include "find.h"
#include "variables.h"
#include "strings.h"
#include "affect.h"

void do_who( charType * ch, char * argument, int cmd )
{
  	char 				buf[256], buf2[256];
	charType		*	who;
  	descriptorType 	*	d;
  	char			*	cp;
  	char			*   cstr;
  	char			*	criminal = "";
	char			*	pstr;
  	byte				class 	 = 0; 
  	int					korean 	 = 0;
  	int					nr 		 = 0;
  	int					lev  	 = 1; 
  	int					cond 	 = 1;
  	int					zone 	 = 0;
	int					percent  = 0;

  	if( argument ) 
  	{
  		for( cp = argument; *cp; )
  		{
  			if( isdigit( *cp ) )
  			{
  				cp = oneArgument( cp, buf );

  				if( getnumber( buf, &lev ) <= 0 )
  				{
  					sendf( ch, "Invaild level number." ); return;
  				}
  				continue;
  			}

    		switch( tolower( *cp ) )
			{
			case ' ': break;
      		case '+': cond =  1; break;
      		case '-': cond = -1; break;
      		case '=': cond =  0; break;
      		case '*': zone =  world[ch->in_room].zone; break;
	  		case 'm': class = 1; break;
	  		case 'c': class = 2; break;
	  		case 't': class = 3; break;
	  		case 'w': class = 4; break;
    		}
			cp++;
  		} 
  	}
  	else 
  	{
    	cond = 1; lev = 1; class = 0;
  	}

  	send_to_char_han( "Players\n\r-----------\n\r", "사람들\n\r-----------\n\r", ch );

  	for( d = desc_list; d; d = d->next ) 
  	{
    	if( !d->connected && can_see(ch, d->character) ) 
		{
			if( d->original ) who = d->original;
			else			  who = d->character;

			if(  (cond == 1 && who->level  < lev )
			  || (cond == 0 && who->level != lev )
			  || (cond ==-1 && who->level  > lev )
			  || (class && who->class != class   )
              || (zone  && world[who->in_room].zone != zone ) ) continue;

	   		if( IS_SET( who->act, PLR_CRIMINAL ) ) criminal = "(CRIMINAL)"; 
			if( IS_SET( who->act, PLR_BANISHED ) ) criminal = "(BANISHED)";

    		percent = (100*GET_HIT(who))/GET_MAX_HIT(who);
        
			if( IS_SET(who->act, PLR_AFK) ) pstr = "-";
			else if (who->timer > 5) pstr = "-";
			else if ( OMNI( who )  ) pstr = "+";
    		else if (percent >= 100) pstr = " ";
    		else if (percent >= 90 ) pstr = "C";
    		else if (percent >= 75 ) pstr = "S";
    		else if (percent >= 50 ) pstr = "F";
    		else if (percent >= 30 ) pstr = "B";
    		else if (percent >= 15 ) pstr = "P";
    		else if (percent >= 0  ) pstr = "A";
    		else                     pstr = "D";

			switch( who->class )
			{
				case	CLASS_MAGIC_USER :	cstr = "Mg"; break;
          		case 	CLASS_CLERIC     : 	cstr = "Cl"; break;
         		case 	CLASS_THIEF      :  cstr = "Th"; break;
         		case 	CLASS_WARRIOR    :  cstr = "Wr"; break;
         		default                  :  continue;
       		}

			if( IS_SET( d->character->act, PLR_KOREAN ) ) korean = 1;
			else										  korean = 0;

	   		sendf( ch, "%c %s %2d %s %c %s %s %s",
					korean ? '(' : '[',	cstr, who->level, pstr, korean ? ')' : ']',
					who->name, who->title, criminal );
    		nr++;
    	}
  	}

  	sendf( ch, "-----------" );
  	sprintf( buf, "You can see %d players.\n\r", nr ); sprintf(buf2,"%d 명이 있습니다.\n\r", nr );
  	send_to_char_han(buf, buf2, ch);
}

void who_service( descriptorType * d )
{
	time_t t;

	GET_LEVEL( d->character ) = 40;

	SET_BIT( d->character->affects, AFF_DETECT_INVISIBLE );

	do_who( d->character, "", -1 );

	REMOVE_BIT( d->character->affects, AFF_DETECT_INVISIBLE );

  	t = 30 + time(0) - boottime;
  	sendf( d->character, "Running time %d:%02d",t/3600,(t%3600)/60);

	GET_LEVEL( d->character ) = 0;

  	SEND_TO_Q("\n\rEnter \"who\" to read who-list again or enter your mud-name to play.\n\r", d );
    SEND_TO_Q("By what name do you wish to be known? ", d );
}

void do_where(charType *ch, char *argument, int cmd)
{
    char 				name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    charType 		*	i, * tmp;
    int 	 			n = 0, pre, found = 0;
    char	 		*	cp ; 
    objectType 		*	k;
    descriptorType 	*	d;

  	oneArgument(argument, name);

  	if( !*name ) 
  	{
		if( GET_LEVEL(ch) < IMO )
		{
		  	for (d = desc_list; d; d = d->next) 
		  	{
				if (d->character && (d->connected == CON_PLYNG) &&
				 	(d->character->in_room != NOWHERE))
	#ifdef DEATHF
			 	if(can_see(ch,d->character)) 
				{
	#else
			 	if(can_see(ch,d->character) && 
			  		world[d->character->in_room].zone == world[ch->in_room].zone) 
				{
	#endif DEATHF
					sendf( ch , "%-20s - %s [%d]",
				  			d->character->name,
				  			world[d->character->in_room].name,
				  			world[d->character->in_room].virtual);
				}
		  	}
    	} 
		else 
		{
      		for (d = desc_list; d; d = d->next) 
	  		{
        		if (d->character && (d->connected == CON_PLYNG) &&
             		(d->character->in_room != NOWHERE))
         			if(can_see(ch,d->character)) 
		 			{
          				if (d->original)
            				sendf( ch, "%-20s - %s [%d] In body of %s",
								d->original->name,
								world[d->character->in_room].name,
								world[d->character->in_room].virtual,
								oneword(d->character->name));
          				else
            				sendf( ch, "%-20s - %s [%d]",
              					d->character->name,
							  	world[d->character->in_room].name,
							  	world[d->character->in_room].virtual);
        			}
      		}
  			for( tmp = char_list; tmp; tmp = tmp->next ) 
  				if( (!IS_NPC(tmp) && !(tmp->desc )) &&
  					( GET_LEVEL(tmp) < IMO || GET_LEVEL(tmp) <= GET_LEVEL(ch)) )
  					{
           				sendf( ch, "%-15s LOST - %s [%d]",
           					tmp->name,
						  	world[tmp->in_room].name,
						  	world[tmp->in_room].virtual);
					}
    	}
		return;
  	}

	if( strchr( name, '.' ) )
	{
		if( pre = splitarg( name, &cp ), pre < 0 )
		{
			sendf( ch, "Find what??" ); return;
		}
	}
	else
	{
		pre = 0; cp = name;
	}

  	for( i = char_list; pre >= 0 && i; i = i->next )
  	{
  		if( isoneof( cp, i->name) && can_see(ch, i) ) 
		{
      		if((i->in_room != NOWHERE) && ((GET_LEVEL(ch)>=IMO) ||
          		(world[i->in_room].zone == world[ch->in_room].zone))) 
		  	{
				if( ++found < pre && pre ) continue;

        		if (GET_LEVEL(ch) >= IMO) sprintf( buf, "[%d]", world[i->in_room].virtual);

        		if (IS_NPC(i))
          			sendf( ch, "%2d: %-30s- %s %s",++n,i->moved,
            			world[i->in_room].name, (GET_LEVEL(ch) >= IMO) ? buf : "" );
        		else
          			sendf( ch, "%2d: %-30s- %s ",++n,i->name, world[i->in_room].name);

				if( pre && found == pre ) return;

        		if( GET_LEVEL(ch) < IMO ) break;
      		}
    	}
	}

  	if( ch->level >= IMO && pre >= 0 ) 
  	{
   		for( n = 0, k = obj_list; k; k = k->next ) 
   		{
			if( isoneof( cp, k->name) )
			{
				if( ++found < pre && pre ) continue;

				if( k->equiped_by )
       				sendf( ch, "%2d: %s worn by %s", ++n, k->wornd, PERS(k->equiped_by,ch));
   				else if( k->carried_by ) 
       				sendf( ch, "%2d: %s carried by %s.", ++n,k->wornd,PERS(k->carried_by,ch));
  				else if( k->in_obj ) 
				{
					objectType 	 	* 	tmp = k->in_obj;

					while( tmp->in_obj ) tmp = tmp->in_obj;

       				sprintf( buf, "%2d: %s in %s",++n, k->wornd, k->in_obj->wornd );

       				if( tmp->carried_by )
       					sendf(ch, "%s carried by %s", buf, PERS(tmp->carried_by,ch));
       				else
       					sendf(ch, buf );
   				}
  				else if( k->in_room != NOWHERE )
       				sendf( ch, "%2d: %s in %s [%d]",++n, k->wornd,
       						world[k->in_room].name, world[k->in_room].virtual);

				if( pre && found == pre ) return;
   			}
  		}
  	}
  	if( !found || (pre && (found != pre)) )
    	send_to_char("Couldn't find any such thing.\n\r", ch);
}

void do_users( charType * ch, char *argument, int cmd )
{
  	descriptorType 	*	d;
  	charType 		* 	tmp;
  	int 				total = 0, see = 0, flag, player = 0;
  	time_t				t;
  	static int 			most = 0;
  	char				line[MAX_INPUT_LENGTH];

  	oneArgument(argument,line);

  	flag = ((GET_LEVEL(ch) < IMO) || (strcmp("-t",line)==0));

  	for( d = desc_list; d; d = d->next ) 
  	{
		total++;
		if( d->character )
		{
			if( d->original ) tmp = d->original;
			else			  tmp = d->character;
		
			if( !can_see(ch, tmp ) ) continue;

			if( !flag )
				sendf( ch, "[%2d] %2d: %-14s(%2d) -%s- %s(%s)", 
					d->fd, tmp->timer, 
					(d->connected != CON_NME) ?  GET_NAME( tmp ) : "UNKOWN",
					GET_LEVEL( tmp ), connected_types[d->connected], d->host, d->ip );
			see++;
		} 
		else
			if( !flag )
				sendf( ch, "[%2d]   : %-14s     -%s-", 
					d->fd, "UNDEF", connected_types[d->connected] );
	}

	sendf( ch, " " );
  	for( tmp = char_list; tmp; tmp = tmp->next ) 
  		if( !IS_NPC(tmp) && 
			( GET_LEVEL(tmp) < IMO || GET_LEVEL(tmp) <= GET_LEVEL(ch))) 
			player++;

  	if( see > player ) player = see;
  	if( total > most ) most = total;

  	sendf( ch, "You can see %d/%d players out of %d/%d connections.", 
				see, player, total, most );
  	t = 30 + time(0) - boottime;
  	sendf( ch, "Running time %d:%02d",t/3600,(t%3600)/60);
}
