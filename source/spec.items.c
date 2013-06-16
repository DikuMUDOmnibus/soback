#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
      
#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "limits.h"
#include "fight.h"
#include "misc.h"
#include "variables.h"
#include "strings.h"
#include "weather.h"
#include "specials.h"
#include "transfer.h"
#include "allocate.h"
#include "page.h"

char *	cookie[MAX_FORTUNE_COOKIE]; 	/* the fortuen cookie text  */
int  	number_of_cookie;            	/* number of descriptions 	*/

int is_wearing_teleport_ring(charType *ch)
{
	int teleport_ring ;

	if ( !ch ) return 0 ;

	teleport_ring = 0 ;
	if ( ch->equipment[WEAR_FINGER_R] ) {
		if (objects[ ch->equipment[WEAR_FINGER_R]->nr ].virtual
			== 2711 )
			teleport_ring = 1;
		}
	if ( ch->equipment[WEAR_FINGER_L] ) {
		if (objects[ ch->equipment[WEAR_FINGER_L]->nr ].virtual
			== 2711 )
			teleport_ring = 1;
		}
	return teleport_ring ;
}

int choose_teleport_room(charType *ch)
{
	int	destiny, room_from = 0, room_to = 0, tmp ;

	destiny = -1 ;
	if ( ch == NULL ) {
		do {
			destiny = number(0, rooms_in_world);
		} while (IS_SET(world[destiny].flags, DARK));
		return destiny ;
	}

	if (!is_wearing_teleport_ring(ch)) {
		room_from = 0 ;
		room_to = rooms_in_world ;
		}
	else {
		if ( ch->equipment[HOLD] ) {
			switch (objects[ ch->equipment[HOLD]->nr ].virtual) {
				case 5230 : /* stone pale blue */
					destiny = real_roomNr(15193) ;
					break ;
				case 5231 : /* stone scarlet blue */
				case 5232 : /* stone blue incandescent */
				case 5233 : /* stone red deep */
				case 5234 : /* stone pink */
					room_from = real_roomNr(201) ;	/* castle gate */
					room_to = real_roomNr(2900) - 1 ;	/* chok */
					break ;
				case 5235 : /* stone pearly white */
				case 5236 : /* stone lavander pale */
					/* moria, aerie, desert, drowcity */
					room_from = real_roomNr(4000) ;
					room_to = real_roomNr(5200) - 1 ;
					break ;
				case 5237 : /* stone lavander green */
				case 5238 : /* stone rose dusty */
					/* forest, dwarven, sewer, redferne, arachnos, arena */
					room_from = real_roomNr(6000) ;
					room_to = real_roomNr(9500) - 1 ;
					break ;
				case 5239 : /* stone grey dull */
				case 5240 : /* stone purple vibrant */
					/* galaxy, deathstar, shogun, easy, market, olympus */
					room_from = real_roomNr(9701) ;
					room_to = real_roomNr(12029)  - 1;
					break ;
				case 5241 : /* stone pink green */
				case 5242 : /* stone green pale : wee and onara */
					room_from = real_roomNr(16000) ;
					room_to = real_roomNr(17094) ;	/* onara */
					break ;
				case 5243 : /* stone clear : robo city */
					room_from = real_roomNr(14601) ;
					room_to = real_roomNr(16000)  - 1;
					break ;
				case 5244 : /* stone iridescent */
					room_from = rooms_in_world/2 ;
					room_to = rooms_in_world ;
					break ;
				case 1325 :	/* mauve stone */
					room_from = 0 ;
					room_to = rooms_in_world/2 ;
					break ;
				case 1311 :	/* totem : midgaard */
					room_from = real_roomNr(3001) ;
					room_to = real_roomNr(4000) - 1 ;
					break ;
				case 2838 :	/* sharon stone : death kingdom */
					room_from = real_roomNr(9500) ;
					room_to = real_roomNr(9701) -1 ;
					break ;
				default :
					room_from = 0 ;
					room_to = rooms_in_world ;
				}
			}
		else {	/* wear ring, but has no special stone */
			room_from = rooms_in_world / 4 ;
			room_to = rooms_in_world * 3 / 4 ;
			}
		}	/* end of teleport ring */
		if( destiny < 0 || destiny > rooms_in_world ) {	/* not decided yet */
		if ( room_from > room_to ) {
			log("debug: from to swap in select teleport") ;
			tmp = room_from ;
			room_from = room_to ;
			room_to = tmp ;
			}
		do {
			destiny = number(room_from, room_to);
		} while (IS_SET(world[destiny].flags, DARK));
	}

	return destiny ;
}

int teleport_machine(charType *ch, int cmd, char *arg)
{
	charType	*tch, *next_tch ;
	int	 new_room ;
	char	name[256] ;

	/* teleport machine in robot city */
	if ( cmd != COM_PRESS )
		return FALSE ;

	oneArgument(arg, name) ;
	if ( *name == NULL ) {
		send_to_char("Press what ?\n\r", ch) ;
		return TRUE ;
		}

	if ( !isoneof(name, "button 단추")) {
		send_to_char("I can't find that.\n\r", ch) ;
		return TRUE ;
		}
	else {	/* press button */
		if (( time_info.hours <= 12 && ch->in_room == real_roomNr(15076) ) || (
			time_info.hours > 12 && ch->in_room == real_roomNr(15023))) {
			send_to_char("기계가 꺼져 있습니다. 다른 곳에 가보세요.\n\r", ch);
			return TRUE ;
			}
		send_to_char("단추를 누르자 꽝 하는 소리가 납니다.\n\r", ch) ;
		act("꽝 하는 소리와 함께 방안에 연기가 가득 찹니다.",
			FALSE, ch, 0, 0, TO_ROOM);
		if ( is_wearing_teleport_ring(ch)) {
			new_room = choose_teleport_room(ch) ;
			for ( tch = world[ch->in_room].people ; tch ; tch = next_tch ) {
				next_tch = tch->next_in_room ;
				if (IS_NPC(tch)) continue ;
				if (tch->fight)
					stop_fighting(tch);
				char_from_room(tch);
				char_to_room(tch, new_room);
				send_to_char("야릇한 기분이 듭니다.\n\r", tch) ;
				act("$n님이 작은 연기와 함께 방 가운데 나타 납니다.", FALSE,
					tch, 0, 0, TO_ROOM);
				do_look(tch, "", 0);
				}
			}	/* wearing teleport control ring */
		else {
			for ( tch = world[ch->in_room].people ; tch ; tch = next_tch ) {
				next_tch = tch->next_in_room ;
				if (IS_NPC(tch)) continue ;
				if (tch->fight)
					stop_fighting(tch);
				new_room = choose_teleport_room(tch) ;
				char_from_room(tch);
				char_to_room(tch, new_room);
				send_to_char("야릇한 기분이 듭니다.\n\r", tch) ;
				act("$n님이 작은 연기와 함께 방 가운데 나타 납니다.", FALSE,
					tch, 0, 0, TO_ROOM);
				do_look(tch, "", 0);
				}
			}	/* not wearing teleport control ring */
		}	/* press button */

	return TRUE ;
}


int totem(charType *ch, int cmd, char *arg)
{
  int rm;
  objectType *obj;

  if(cmd!= COM_LOOK ) return(FALSE);
  obj=ch->equipment[HOLD];
  if(!obj) return(FALSE);
  if(objects[obj->nr].virtual != 1311) return(FALSE);
  if(!ch->followers) return(FALSE);
  rm=ch->followers->follower->in_room;
  send_to_char(world[rm].name, ch);
  send_to_char("\n\r", ch);
  if(!IS_SET(ch->act,PLR_BRIEF))
    send_to_char(world[rm].description, ch);
  list_obj_to_char( world[rm].contents, ch, 1, FALSE );
  list_room_chars( ch, world[rm].people );
  return(TRUE);
}

int magicseed(charType *ch, int cmd, char *arg)
{
  	int 			skno, maxtry;
  	char 			name[256];
  	objectType 	*	obj;

  	if( cmd != COM_EAT ) return(FALSE);

	if( IS_NPC( ch ) ) return 0;

  	oneArgument( arg, name );

  	obj = find_obj_inven( ch, name );

  	if( !obj ) return(FALSE);

  	if( GET_COND( ch, FULL ) > 20 )  return(FALSE);

  	if( objects[obj->nr].virtual != 2157) return(FALSE);

	maxtry = 0;

	while( ++maxtry < 20 )
	{
		skno = number( 1, MAX_USED_SPELLS ) ;

		if(  spells[skno].name 
          && spells[skno].name[0] 
		  && spells[skno].min_level[GET_CLASS(ch)-1] < ch->level )
		{
			if( ch->skills[skno] != 99 )
			{
				ch->skills[skno] = 99;
				sendf( ch, "You are shocked by some strange power from unknown origin." );
				act( "$n is shocked by some strange power from unknown origin.", 1, ch,0,0, TO_ROOM ); 
			}
		}
	}

	do_eat( ch, "seed", COM_EAT );

	return 1;
}

int file_to_cookie(char *name, char *buf[MAX_FORTUNE_COOKIE])
{
  	FILE 	*	fl;
  	char 		tmp[200], *tmp2;
  	int  		count ;

  	*buf = '\0';

  if (!(fl = errOpen(name, "r")))
  {
    perror("file-to-cookie");
    *buf = '\0';

    return(-1);
  }

  count = 0 ;
  do
  {
    fgets(tmp, 199, fl);

    if (!feof(fl))
    {
      tmp2 = (char *) errMalloc( strlen(tmp) +2 ) ;
      strcpy(tmp2, tmp) ;
      *(tmp2 + strlen(tmp2) +1) = NULL ;
      *(tmp2 + strlen(tmp2)) = '\r' ;
      buf[count] = tmp2 ;
      count ++ ;
    }
  }
  while (!feof(fl) && count < MAX_FORTUNE_COOKIE);

  number_of_cookie = count ;

  fclose(fl);

  return(0);
}

char *select_fortune_cookie( void )
{ 
    if( number_of_cookie == 0 )
    {  
        return "\n\rFortune cookie is not implemented.\n\r";
    }
    return(cookie[number(0,number_of_cookie-1)]);
} 

static int com_objok( objIndexType * obj, int type, int arg )
{
	switch( type )
	{
		case	0 :	return (obj->wear  & (1 << arg)); 
		case	1 : return (obj->extra & (1 << arg));
		case	2 : return (isoneofp( (char *)arg, obj->name ));
		case	3 : return (obj->type == arg );
		case	4 :	return (obj->apply[0].location == arg ? 1 : ( obj->apply[1].location == arg ));
		case	5 : return (obj->level >= arg );
	}
	return 0;
}

static int com_argsok( int type, char * arg )
{
	switch( type )
	{
		case 	0 : return isinlistp( arg, (const char **)wear_bits );
		case	1 : return isinlistp( arg, (const char **)extra_bits );
		case	2 : return (*arg != 0 );
		case    3 : return isinlistp( arg, (const char **)item_types );
		case	4 : return isinlistp( arg, (const char **)apply_types );
		case	5 : return isnumstr( arg );
	}
	return -1;
}

void com_ifind( charType * ch, char * argument )
{
	static const char * what[] =
	{
		"wear", "extra", "name", "type", "apply", "level"
	};

	pageUnitType	* 	pu;
	objIndexType	*	oi;
	int			type1 = -1;
	int			type2 = -1;
	int			opt1  = -1;
	int			opt2  = -1;
	char		buf1[MAX_INPUT_LENGTH];
	char		buf2[MAX_INPUT_LENGTH];
	char		buf3[MAX_INPUT_LENGTH];
	char		buf4[MAX_INPUT_LENGTH];
	char		arg1[MAX_INPUT_LENGTH];
	char		arg2[MAX_INPUT_LENGTH];
	int			i, j;

	if( !*argument )
	{
		sendf( ch, "\n\rpress [[type] [what]][..]\n\r"
                   "  type -> wear, extra, name, type, apply, level\n\r"
                   "  what -> head, hum,   key,  note, wis,   10\n\r\n\r"
				   "\tex> press type trap\n\r"
                   "\tex> press wear head apply wis\n\r"
				   "\tex> press w h a w (same as above)\n\r" );
		return;
	}

	argument = twoArgument( argument, arg1, arg2 );

	if( !*arg2 )
	{
		sendf( ch, "Ok. Got search type. But search what?" ); return;
	}

	type1 = isinlistp( arg1, what );

	if( opt1 = com_argsok( type1, arg2 ), opt1 < 0 )
	{
		sendf( ch, "First pair of options won't match." ); return;
	}

	if( type1 == 2 ) opt1 = (int)strcpy( buf1, arg2);
	if( type1 == 5 ) getnumber( buf1, &opt1 ); 

	if( *argument )
	{
		argument = twoArgument( argument, arg1, arg2 );

		type2 = isinlistp( arg1, what );

		if( opt2 = com_argsok( type2, arg2 ), opt2 < 0 )
		{
			sendf( ch, "Second pair of options wor't match." ); return;
		}
		if( type2 == 2 ) opt2 = (int)strcpy( buf2, arg2);
		if( type1 == 5 ) getnumber( buf2, &opt2 ); 
	}

	pu = new_pageunit( ch );

	for( i = OBJECT_INTERNAL + 1, j = 1; i < obj_index_info.used; i++ )
	{
		oi = &objects[i];

		if( com_objok( oi, type1, opt1 ) )
		{
			if( type2 == -1 || com_objok( oi, type2, opt2 ) )
			{
				pagef( pu, "%3d] %34s-%12s %3d-%12s %3d", 
				j++, oi->name,
				sprinttype( oi->apply[0].location, apply_types, buf3 ), oi->apply[0].modifier,
				sprinttype( oi->apply[1].location, apply_types, buf4 ), oi->apply[1].modifier );
			}
		}
	}
	if( j == 1 ) sendf( ch, "Nothing" );
	show_page( ch, pu );
}

int information( charType * ch, int cmd, char * argu )
{
	if( !cmd ) return 0;

	if( cmd == COM_PRESS )
	{
		com_ifind( ch, argu ); return 1;
	}
	return 0;
}
