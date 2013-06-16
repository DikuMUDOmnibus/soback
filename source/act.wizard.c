/* ************************************************************************
*  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
*  Usage : Wizard Commands.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <sys/time.h>
#include <sys/resource.h>

#ifdef __rusage__
#include <sys/rusage.h>
#endif

#include "defines.h"
#include "character.h"
#include "object.h"
#include "world.h"
#include "mobile.h"
#include "sockets.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "limits.h"
#include "quest.h"
#include "variables.h"
#include "misc.h"
#include "strings.h"
#include "weather.h"
#include "allocate.h"
#include "update.h"
#include "affect.h"
#include "transfer.h"
#include "spec.rooms.h"
#include "stash.h"
#include "page.h"
#include "fight.h"

void do_peace( charType * ch, char * argument, int cmd )
{
	charType	*	vict;

	if( IS_NPC( ch ) ) return;

	for( vict = world[ch->in_room].people; vict; vict = vict->next_in_room )
	{
		if( vict->fight ) stop_fighting( vict );
	}

	sendrf( 0, ch->in_room, "The world seems a little more peaceful." ); 
}

void do_damage( charType * ch, char * argument, int cmd )
{
	charType		*	vict;
	char				name[100], damstr[100];
	int					dam, max;

	if( IS_NPC( ch ) ) return;

	twoArgument( argument, name, damstr );

    if( !(vict = find_char_room( ch, name ) ) )
    {
        if( !(vict = find_char(ch, name)) )
        {
            sendf( ch, "No-one by that name here.." );
            return;
        }
    }

	if( getnumber( damstr, &dam ) <= 0 )
	{
		sendf( ch, "Invalid damage number." ); return;
	}

	max = GET_HIT( vict ) + 1;
	dam = MIN( max, dam );

	GET_HIT( vict ) -= dam;

	act( "You skillfully carve your initials into $N's breast.", TRUE, ch, 0, vict, TO_CHAR );
	act( "$n slashes $s initials onto your breast, OUCH~~~", TRUE, ch, 0, vict, TO_VICT );
	act( "$n makes $s initials known on $N's breast.", TRUE, ch, 0, vict, TO_NOTVICT );

	update_pos( vict );
}

void do_echo(charType *ch, char *argument, int cmd)
{
  	int i;
  
  	if( IS_NPC(ch) ) return;

  	for( i = 0; *(argument + i) == ' '; i++ );

  	if( !*(argument + i) ) sendf( ch, "That must be a mistake..." );
  	else
  	{
    	sendrf( 0, ch->in_room, "%s", argument + i );
    	sendf( ch, "Ok." );
  	}
}

void do_trans(charType *ch, char *argument, int cmd)
{
  	descriptorType 	*	i;
  	charType 		*	vict;
  	char 		    *   buf;		
  	short 				target;

  	if( IS_NPC(ch) ) return;

  	buf = oneword( argument );

  	if( !*buf ) send_to_char("Who do you wish to transfer?\n\r",ch);
  	else if( stricmp( "all", buf ) != 0 ) 
  	{
    	if( !(vict = find_char( ch, buf )) )
      		sendf( ch, "No-one by that name around." );
    	else 
		{
      		if( GET_LEVEL(ch) < IMO+3 && GET_LEVEL(vict) > GET_LEVEL(ch) )
	  		{
        		sendf( ch, "That might not be appreciated." );
        		return;
      		}
      		act("$n disappears in a mushroom cloud.",FALSE,vict,0,0,TO_ROOM);
      		target = ch->in_room;
      		char_from_room(vict);
      		char_to_room(vict,target);
      		act("$n arrives from a puff of smoke.",FALSE,vict,0,0,TO_ROOM);
      		act("$n has transferred you!",FALSE,ch,0,vict,TO_VICT);
      		do_look(vict,"",15);
      		send_to_char("Ok.\n\r",ch);
    	}
  	} 
  	else 
  	{ /* Trans All */
    	if( ch->level < (IMO+3) ) return;

    	for( i = desc_list; i; i = i->next )
      		if( i->character != ch && !i->connected ) 
	  		{
        		target = ch->in_room;
        		vict = i->character;
        		char_from_room(vict);
        		char_to_room(vict,target);
        		act("$n arrives from a puff of smoke.", FALSE, vict, 0, 0, TO_ROOM);
        		act("$n has transferred you!",FALSE,ch,0,vict,TO_VICT);
        		do_look(vict,"",15);
      		}
    	sendf( ch, "Ok." );
  	}
}

void do_at(charType *ch, char *argument, int cmd)
{
  	char 			*	command, loc_str[MAX_INPUT_LENGTH];
  	int 				loc_nr, location, original_loc;
  	charType 		*	target_mob;
  	objectType 		*	target_obj;
  
  	if( IS_NPC(ch) ) return;

	command = onefword( argument, loc_str );

  	if( !*loc_str )
  	{
    	send_to_char("You must supply a room number or a name.\n\r", ch);
    	return;
  	}

  	if( isnumstr(loc_str) )
  	{
    	loc_nr = atoi(loc_str);

		if( location = real_roomNr( loc_nr ), location == NOWHERE )
		{
			sendf( ch, "That room does not exist." ); return;
		}

  	}
  	else if( (target_mob = find_char(ch, loc_str)) )
    	location = target_mob->in_room;
  	else if( (target_obj = find_obj(ch, loc_str))  )
    	if( target_obj->in_room != NOWHERE )
      		location = target_obj->in_room;
    else
    {
      	send_to_char("The object is not available.\n\r", ch); return;
    } 
	else 
	{
    	send_to_char("No such creature or object around.\n\r", ch); return;
  	}

  	if( (GET_LEVEL(ch)<(IMO+3)) && (IS_SET(world[location].flags,OFF_LIMITS)) )
	{
    	send_to_char("That room is off-limits.\n",ch); 	return;
  	}

  	original_loc = ch->in_room;
  	char_from_room( ch );
  	char_to_room( ch, location );

  	interpreter( ch, command, 0 );

  	for( target_mob = world[location].people; target_mob; target_mob = target_mob->next_in_room )
    	if( ch == target_mob )
    	{
      		char_from_room( ch );
      		char_to_room( ch, original_loc );
    	}
}

void do_goto(charType *ch, char *argument, int cmd)
{
  	char 					buf[MAX_INPUT_LENGTH];
  	int 					loc_nr, location,i,flag;
  	charType 			*	target_mob, *pers;
  	objectType 			*	target_obj;

  	if( IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3)) return;

  	oneArgument(argument, buf);

  	if( !*buf ) 
  	{
    	send_to_char("You must supply a room number or a name.\n\r", ch);
    	return;
  	}
			
  	if( isnumstr(buf) && getnumber( buf, &loc_nr) > 0 ) 
  	{
  		if( location = real_roomNr( loc_nr ), location == NOWHERE )
	  	{
        	send_to_char("No room exists with that number.\n\r", ch);
        	return;
      	}
  	}
  	else if(( target_mob = find_char(ch, buf)) ) location = target_mob->in_room;
  	else if(( target_obj = find_obj( ch, buf ))) 
  	{
  		if( target_obj->in_room != NOWHERE ) location = target_obj->in_room;
    	else 
		{
      		send_to_char("The object is not available.\n\r", ch);
      		return;
    	} 
	}
	else 
	{
   		send_to_char("No such creature or object around.\n\r", ch);
   		return;
  	}

  	if( GET_LEVEL(ch) < (IMO+2) )
  	{
    	if( IS_SET(world[location].flags, OFF_LIMITS) )
		{
      		send_to_char("Sorry, off limits.\n",ch);
      		return;
    	}
	}

    if( IS_SET(world[location].flags, PRIVATE) ) 
	{
      	for( i = 0, pers = world[location].people; pers; pers = pers->next_in_room, i++);
      		if( i > 1 ) 
	  		{
        		sendf( ch, "There's a private conversation going on in that room." );
        		return;
      		}
   	}

  	flag = ((GET_LEVEL(ch)>=(IMO+2)) && IS_SET(ch->act,PLR_WIZINVIS));

  	if( !flag ) act("$n disappears in a puff of smoke.", FALSE, ch, 0, 0, TO_ROOM);

  	char_from_room(ch);	char_to_room(ch, location);

  	if( !flag ) act("$n appears with an ear-splitting bang.", FALSE, ch, 0,0,TO_ROOM);

  	do_look(ch, "",15);
}

void do_demote(charType *ch, char *argument, int cmd)
{
  	charType 		*	vict;
  	char 		    *	name;

  	if( IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3) ) return;

  	name = oneword( argument );

  	if( *name ) 
  	{
    	if(!(vict = find_char_room(ch, name))) 
		{
      		send_to_char("That player is not here.\n\r", ch);
      		return;
    	}
  	} 
  	else 
  	{
    	send_to_char("Demote who?\n\r", ch);
    	return;
  	}

  	if (IS_NPC(vict)) 
  	{
    	send_to_char("NO! Not on NPC's.\n\r", ch);
    	return;
  	}

  	if( GET_LEVEL(vict) > IMO && !implementor(GET_NAME(ch)) ) return ;
  	if( implementor(GET_NAME(vict)) && GET_LEVEL(vict) >= IMO+3 ) return ;

	senddf( 0, 0, "%s demotes %s.", ch->name, vict->name );

  	do_start(vict);
}

void do_rstat( charType * ch, char * argu, int cmd )
{
	exdescriptionType *	ep;
	roomType		  *	room;
	char				sector[256], flag[256], keyword[1024];
	int					nr, i;

	if( !*argu )
	{
		nr = ch->in_room;
	}
	else
	{
		if( getnumber( argu, &nr ) <= 0 ) 
		{
			sendf( ch, "Wrong argument.. I need a number of room." ); return;
		}

		if( nr = real_roomNr( nr ), nr == NOWHERE )
		{
			sendf( ch, "That room does not exist." ); return;
		}
	}

	room = &world[nr];

	sprinttype( room->sector, sector_types, sector );
	sprintbit ( room->flags,  room_bits,    flag ); 

	if( room->description )
	{
	sendf( ch, "%s", room->description );
	}
	else
	sendf( ch, "Description : None" );
  	sendf( ch, "       Name : <%s>\n\r"
               "       Zone : %d, R-Nummber: [%d], V-number : [%d]\n\r"
  	           "      Light : [%d], Dark : [%d], Sector type : %s\n\r"
  	           "    Special : %s\n\r"
               "      Flags : %s",
              	room->name, room->zone, room->nr, room->virtual, room->light, IS_DARK(nr), 
				sector,	room->func ? "Exist" : "None", flag );

  	sprintf( keyword, " Keyword(s) : " );

  	if( room->extrd )
  	{
    	for( ep = room->extrd; ep; ep = ep->next)           
    	{
      		sprintf( keyword + strlen( keyword ), "%s ", ep->keyword );            
    	}
  	}
  	else strcat( keyword, "None" );

	sendf( ch, "%s", keyword );
    sendf( ch, "      Exits :" );

    for( i = 0; i <= 5; i++ ) 
	{
        if( room->dirs[i] ) 
		{
         	sendf( ch, "  Direction : [%6s],  Keyword : %s", 
		 		dirs[i], room->dirs[i]->keyword ? room->dirs[i]->keyword : "None" );

			if( room->dirs[i]->description )
          	sendf( ch, "            : %s", room->dirs[i]->description );   

          	sendf( ch, "            : Key [%d], To room [%d], Flag : %s",
                  room->dirs[i]->key, world[room->dirs[i]->to_room].virtual,
                  sprintbit( room->dirs[i]->exit_info, exit_bits, 0 )  );
        }
    }
}

void do_istat( charType * ch, char * argument, int cmd )
{
	objectType			*	obj = 0;
	objIndexType		*	oi = 0;
	char				*	name = 0;
	int						objNr;
	int						ofound = 0;
	int						i;

	if( cmd != COM_OSTAT )
	{
		if( !argument || !*argument ) 
		{
			sendf( ch, "Stat what object?" ); return;
		}	
		
		if( name = argument, objNr = dotnumber( &name ), objNr == 0 || !*name )
		{
			sendf( ch, "Hmm.." ); return;
		}

		if( objNr != 1 || !isnumstr( name ) )
		{
			for( i = 0; i < obj_index_info.used; i++ )
			{
				if( isoneof( name, objects[i].name ) )
				{
					if( ++ofound < objNr ) continue;

					oi = &objects[i];  break;
				}
			}
			if( !oi ) 
			{
				sendf( ch, "No such object in database" ); return;
			}
		}
		else
		{
			if( i = real_objectNr( atoi( name ) ), i != OBJECT_NULL )
				oi = &objects[i];
			else
			{
				sendf( ch, "No object in database by that number." ); return;
			}
		}
	}
	else 
	{
		obj = (objectType *)argument;
		oi  = &objects[obj->nr];
	}

    sendf( ch, "Object name : <%s>, R-number: [%d], V-number : [%d]", oi->name, oi->nr, oi->virtual );
    sendf( ch, "       Type : %s", 	sprinttype( oi->type, item_types, 0 ) );
    sendf( ch, "  When worn : %s", oi->wornd ? oi->wornd : "None");
    sendf( ch, "  When room : %s", oi->roomd ? oi->roomd : "None");
    sendf( ch, "  When used : %s", oi->usedd ? oi->usedd : "None");

    if( oi->extrd )
    {
		char					keyword[1024];
		exdescriptionType	* 	ed;

        strcpy( keyword, " Keyword(s) : ");
        for( ed = oi->extrd; ed; ed = ed->next )
        {
			sprintf( keyword + strlen( keyword ), "%s ", ed->keyword );
        }
		sendf( ch, "%s", keyword );
    }
    else
    {
        sendf( ch, " Keyword(s) : None");
    }
    sendf( ch, "    Worn on : %s", sprintbit( oi->wear, wear_bits, 0 ) );
    sendf( ch, "Extra flags : %s", sprintbit( oi->extra, extra_bits, 0 ) );
	sendf( ch, "     Values : [%d], [%d], [%d], [%d]", 
				oi->value[0], oi->value[1], oi->value[2], oi->value[3] );
    sendf( ch, "     Weight : [%d],  Cost [%d],  Level [%d],  Magic [%d]", 
				oi->weight, oi->cost, oi->level, oi->magic );
	sendf( ch, "    Affects :" );

	if( cmd != COM_OSTAT )
	{
		for ( i = 0; i < MAX_APPLY; ++i ) 
			sendf( ch, "\t      Affects : %s by %d", 
				sprinttype( oi->apply[i].location, apply_types, 0 ), oi->apply[i].modifier);
	}
	sendf( ch, "   In World : [%d], Off World [%d]",  oi->in_world, oi->off_world );
}

void do_mstat( charType * ch, char * argument, int cmd )
{
	mobIndexType		*	mi = 0;
	int						i;
	int						objNr;
	int						ofound;
	char				*	name;

	if( !argument || !*argument ) 
	{
		sendf( ch, "Stat what mobile?" ); return;
	}	
	
	if( name = argument, objNr = dotnumber( &name ), objNr == 0 || !*name )
	{
		sendf( ch, "Hmm.." ); return;
	}

	if( objNr != 1 || !isnumstr( name ) )
	{
		for( ofound = 0, i = 0; i < mob_index_info.used; i++ )
		{
			if( isoneof( name, mobiles[i].name ) )
			{
				if( ++ofound < objNr ) continue;

				mi = &mobiles[i];  break;
			}
		}
	}
	else
	{
		if( i = real_mobileNr( atoi( name ) ), i != MOBILE_NULL )
			mi = &mobiles[i];
	}

	if( !mi )
	{
		sendf( ch, "No such mobile in database." ); return;
	}

	if( mi->description )
	{
	sendf( ch, "%s", mi->description );
	}
	else
	sendf( ch, "Description : None" );
	sendf( ch, "       Name : %s, R-number[%d], V-number[%d]", mi->name, mi->nr, mi->virtual );
	sendf( ch, "  Whem move : %s", mi->moved ? mi->moved : "None" );
	sendf( ch, "  When room : %s", mi->roomd ? mi->roomd : "None" );
	sendf( ch, "      Level : [%d], Class [%s], Sex [%s]", 
				mi->level, sprinttype( mi->class, npc_class_types, 0 ),
				mi->sex ? ( mi->sex == 1 ? "Male" : "Female" ) : "Neutral" );
	sendf( ch, "     Points : Hit[%dd%d+%d]",
				mi->hsdice, mi->hndice, mi->hbase );
	sendf( ch, "         AC : [%d], Damroll : %d[%dD%d], Hitroll: %d", 
				mi->ac, mi->dr, mi->ndice, mi->sdice, mi->hr );
	sendf( ch, "  Alignment : %d, Exp %d, Gold %d", mi->align, mi->exp, mi->gold ); 
	sendf( ch, "   Specials : %s", mi->func ? "Exist" : "None" );
	sendf( ch, "     Action : %s", sprintbit( mi->act, action_bits, 0 ) );
	sendf( ch, "   Affected : %s", sprintbit( mi->affected, affected_bits, 0 ) );
	sendf( ch, "   In World : [%d]", mi->in_world );
}

static int ifind_objok( objIndexType * obj, int type, int arg )
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

static int ifind_argsok( int type, char * arg )
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

void do_ifind( charType * ch, char * argument, int cmd )
{
	static const char * what[] =
	{
		"wear", "extra", "name", "type", "apply", "level"
	};

	pageUnitType	*	pu;
	objIndexType	*	oi;
	int			type1 = -1;
	int			type2 = -1;
	int			opt1  = -1;
	int			opt2  = -1;
	char		buf1[MAX_INPUT_LENGTH];
	char		buf2[MAX_INPUT_LENGTH];
	char		arg1[MAX_INPUT_LENGTH];
	char		arg2[MAX_INPUT_LENGTH];
	int			i, j;

	if( !*argument )
	{
		sendf( ch, "\n\rifind [[type] [what]][..]\n\r"
                   "  type -> wear, extra, name, type, apply, level\n\r"
                   "  what -> head, hum,   key,  note, wis,   10\n\r\n\r"
				   "\tex> ifind type trap\n\r"
                   "\tex> ifind wear head apply wis\n\r"
				   "\tex> ifind w h a w (same as above)\n\r" );
		return;
	}

	argument = twoArgument( argument, arg1, arg2 );

	if( !*arg2 )
	{
		sendf( ch, "Ok. Got search type. But search what?" ); return;
	}

	type1 = isinlistp( arg1, what );

	if( opt1 = ifind_argsok( type1, arg2 ), opt1 < 0 )
	{
		sendf( ch, "First pair of options won't match." ); return;
	}

	if( type1 == 2 ) opt1 = (int)strcpy( buf1, arg2);
	if( type1 == 5 ) getnumber( buf1, &opt1 ); 

	if( *argument )
	{
		argument = twoArgument( argument, arg1, arg2 );

		type2 = isinlistp( arg1, what );

		if( opt2 = ifind_argsok( type2, arg2 ), opt2 < 0 )
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

		if( ifind_objok( oi, type1, opt1 ) )
		{
			if( type2 == -1 || ifind_objok( oi, type2, opt2 ) )
			{
				pagef( pu, "%3d] %35s-%5d-[%2d,%2d,%2d,%2d]-[%2d,%3d] [%2d,%3d]", 
				j++, oi->name, oi->virtual,
				oi->value[0], oi->value[1], oi->value[2], oi->value[3],
				oi->apply[0].location, oi->apply[0].modifier,
				oi->apply[1].location, oi->apply[1].modifier );
			}	
		}
	}
	if( j == 1 ) sendf( ch, "Nothing" );
	show_page( ch, pu );
}

static int mfind_mobok( mobIndexType * mob, int type, int arg )
{
	switch( type )
	{
		case	0 :	return (mob->act  & (1 << arg)); 
		case	1 : return (mob->affected & (1 << arg));
		case	2 : return (isoneofp( (char *)arg, mob->name ));
	}
	return 0;
}

static int mfind_argsok( int type, char * arg )
{
	switch( type )
	{
		case 	0 : return isinlistp( arg, (const char **)action_bits );
		case	1 : return isinlistp( arg, (const char **)affected_bits );
		case	2 : return (*arg != 0 );
	}
	return -1;
}

void do_mfind( charType * ch, char * argument, int cmd )
{
	static const char * what[] =
	{
		"act", "affect", "name"
	};

	pageUnitType	*	pu;
	mobIndexType	*	mi;
	int			type1 = -1;
	int			type2 = -1;
	int			opt1  = -1;
	int			opt2  = -1;
	char		buf1[MAX_INPUT_LENGTH];
	char		buf2[MAX_INPUT_LENGTH];
	char		arg1[MAX_INPUT_LENGTH];
	char		arg2[MAX_INPUT_LENGTH];
	int			i, j;

	if( !*argument )
	{
		sendf( ch, "\n\rmfind [[type] [what]][..]\n\r"
                   "  type -> act,  affect, name\n\r"
                   "  what -> mage, sanc,   fido\n\r\n\r"
				   "\tex> mfind type trap\n\r"
                   "\tex> mfind act mage affect sanc\n\r"
				   "\tex> mfind a m af sa (same as above)\n\r" );
		return;
	}

	argument = twoArgument( argument, arg1, arg2 );

	if( !*arg2 )
	{
		sendf( ch, "Ok. Got search type. But search what?" ); return;
	}

	type1 = isinlistp( arg1, what );

	if( opt1 = mfind_argsok( type1, arg2 ), opt1 < 0 )
	{
		sendf( ch, "First pair of options won't match." ); return;
	}

	if( type1 == 2 ) opt1 = (int)strcpy( buf1, arg2);

	if( *argument )
	{
		argument = twoArgument( argument, arg1, arg2 );

		type2 = isinlistp( arg1, what );

		if( opt2 = mfind_argsok( type2, arg2 ), opt2 < 0 )
		{
			sendf( ch, "Second pair of options wor't match." ); return;
		}
		if( type2 == 2 ) opt2 = (int)strcpy( buf2, arg2);
	}

	pu = new_pageunit( ch );

	for( i = MOBILE_INTERNAL + 1, j = 1; i < mob_index_info.used; i++ )
	{
		mi = &mobiles[i];

		if( mfind_mobok( mi, type1, opt1 ) )
		{
			if( type2 == -1 || mfind_mobok( mi, type2, opt2 ) )
			{
				pagef( pu, "%3d] %28s-%2d-%5d-[%5d+%4d/%-3d]-[%3d+%2d/%-2d,%3d,%3d]", 
							j++, mi->name, mi->level, mi->virtual,
							mi->hbase, mi->hsdice, mi->hndice,
							mi->dr, mi->sdice, mi->ndice, mi->hr, mi->ac );
			}
		}
	}
	if( j == 1 ) sendf( ch, "Nothing" );
	show_page( ch, pu );
}

static int rfind_roomok( roomType * room, int type, int arg )
{
	switch( type )
	{
		case	0 :	return (room->flags  & (1 << arg)); 
		case	1 : return (room->sector == arg);
		case	2 : return (isoneofp( (char *)arg, room->name ));
	}
	return 0;
}

static int rfind_argsok( int type, char * arg )
{
	switch( type )
	{
		case 	0 : return isinlistp( arg, (const char **)room_bits );
		case	1 : return isinlistp( arg, (const char **)sector_types );
		case	2 : return (*arg != 0 );
	}
	return -1;
}

void do_rfind( charType * ch, char * argument, int cmd )
{
	static const char * what[] =
	{
		"room", "sector", "name"
	};

	pageUnitType	* 	pu;
	roomType		*	ri;
	int			type1 = -1;
	int			type2 = -1;
	int			opt1  = -1;
	int			opt2  = -1;
	char		buf1[MAX_INPUT_LENGTH];
	char		buf2[MAX_INPUT_LENGTH];
	char		arg1[MAX_INPUT_LENGTH];
	char		arg2[MAX_INPUT_LENGTH];
	int			i, j;

	if( !*argument )
	{
		sendf( ch, "\n\rrfind [[type] [what]][..]\n\r"
                   "  type -> room,   sector, name\n\r"
                   "  what -> no_mob, city,   temple\n\r\n\r"
				   "\tex> rfind sector outdoor\n\r"
                   "\tex> rfind room norelo sector field\n\r"
				   "\tex> rfind r nor sec f (same as above)\n\r" );
		return;
	}

	argument = twoArgument( argument, arg1, arg2 );

	if( !*arg2 )
	{
		sendf( ch, "Ok. Got search type. But search what?" ); return;
	}

	type1 = isinlistp( arg1, what );

	if( opt1 = rfind_argsok( type1, arg2 ), opt1 < 0 )
	{
		sendf( ch, "First pair of options won't match." ); return;
	}

	if( type1 == 2 ) opt1 = (int)strcpy( buf1, arg2);

	if( *argument )
	{
		argument = twoArgument( argument, arg1, arg2 );

		type2 = isinlistp( arg1, what );

		if( opt2 = rfind_argsok( type2, arg2 ), opt2 < 0 )
		{
			sendf( ch, "Second pair of options wor't match." ); return;
		}
		if( type2 == 2 ) opt2 = (int)strcpy( buf2, arg2);
	}

	pu = new_pageunit( ch );

	for( i = ROOM_INTERNAL + 1, j = 1; i < room_index_info.used; i++ )
	{
		ri = &world[i];

		if( rfind_roomok( ri, type1, opt1 ) )
		{
			if( type2 == -1 || rfind_roomok( ri, type2, opt2 ) )
			{
				pagef( pu, "%3d] %35s-%5d-%9s-%s", 
							j++, ri->name, ri->virtual,
							sprinttype( ri->sector, sector_types, 0 ),
							sprintbit( ri->flags, room_bits, 0 ) );
			}	
		}
	}
	if( j == 1 ) sendf( ch, "Nothing" );
	show_page( ch, pu );
}

void do_stat( charType *ch, char *argument, int cmd )
{
  	affectType 			*	af;
  	charType 			*	sub;
  	objectType  		*	obj;
  	followType 			*	fol;
  	int 					i,	j;
	char				buf[ 4096 ];

  	argument = onefword(argument, buf );

  	if( !*buf ) 
	{
    	sendf( ch, "Stats on who or what??" ); return;
	}

	if( cmd == COM_OSTAT )
	{
		if( obj = find_obj_inven( ch, buf ), !obj || obj->nr < OBJECT_INTERNAL )
			if( obj = find_obj( ch, buf ), !obj ||  obj->nr < OBJECT_INTERNAL )
			{
				sendf( ch, "Can't find such thing." );
				return;
			}
		do_istat( ch, (char *)obj, COM_OSTAT );

		for( i = 0; i < MAX_APPLY; ++i ) 
			sendf( ch, "\t      Affects: %s by %d", 
				sprinttype( obj->apply[i].location, apply_types, 0 ), obj->apply[i].modifier);
    	sendf( ch, "     Limits : [%d],  Status [%d],  Timer [%d]", 
				obj->limit, obj->status, obj->timer );
		return;
    }

    if( cmd == COM_PSTAT ) 
	{
		for( sub = player_list; sub; sub = sub->next_char )
		{
			if( isoneof( buf, sub->name ) && can_see( ch, sub ) ) break;
		}

		if( !sub )
		{
			sendf( ch, "No such player." ); return;
		}
	}
    else if( cmd == COM_NSTAT ) 
	{
		for( sub = mob_list; sub; sub = sub->next_char )
		{
			if( isoneof( buf, sub->name ) && can_see( ch, sub ) ) break;
		}

		if( !sub )
		{
			sendf( ch, "No such mobile." ); return;
		}
	}
    else 
	{
		if( sub = find_char_room( ch, buf ), !sub )
			if( sub = find_char( ch, buf), !sub )
			{
				sendf( ch, "Can't find any such thing" ); return;
			}
	}

	if( IS_NPC( sub ) )
	{
		mobIndexType 	*	mi = &mobiles[sub->nr];

		if( sub->description )
		sendf( ch, "%s", sub->description );
		else
		sendf( ch, "       Name : %s, R-number[%d], V-number[%d]", sub->name, sub->nr, mi->virtual );
		sendf( ch, "  Whem move : %s", sub->moved ? sub->moved : "None" );
		sendf( ch, "  When room : %s", sub->roomd ? sub->roomd : "None" );
		sendf( ch, "      Level : [%d], Class [%s], Sex [%s]", 
					sub->level, sprinttype( sub->class, npc_class_types, 0 ),
					sub->sex ? ( sub->sex == 1 ? "Male" : "Female" ) : "Neutral" );
		sendf( ch, "   Specials : %s", mi->func ? "Exist" : "None" );
		sendf( ch, "     Action : %s", sprintbit( mi->act, action_bits, 0 ) );
		sendf( ch, "   In World : %d", mi->in_world );
	}
	else
	{
		char		birth[30], logon[30];
		time_t		t;
		int			i;

		if( sub->description )
		sendf( ch, "%s", sub->description );
		else
		sendf( ch, "Description : None" );
		sendf( ch, "       Name : %s", sub->name );
		sendf( ch, "      Title : %s", sub->title ? sub->title : "None" );
		sendf( ch, "      Level : [%d], Class [%s], Sex [%s]",
					sub->level, sprinttype( sub->class, pc_class_types, 0 ),
					sub->sex ? ( sub->sex == 1 ? "Male" : "Female" ) : "Neutral" );
		sendf( ch, "     Action : %s", sprintbit( sub->act, player_bits, 0 ) );

		t = sub->birth; strcpy( birth, ctime( &t) ); remove_newline( birth );
		t = sub->logon; strcpy( logon, ctime( &t) ); remove_newline( logon );
		t = sub->played;
		i = t % 86400;	

		sendf( ch, "        Age : %d year %d month, Played : %d days, %d:%02d", 
									age( sub ).year, age(sub).month, t/84600, i/3600, (i+30)%60 );
		sendf( ch, "      Birth : %s", birth ); 
        sendf( ch, "      Logon : %s", logon ); 
	}
		sendf( ch, "  Alignmemt : [%d], Exp [%s], Gold [%s]", 
					sub->align, strcpy( buf, numfstr(sub->exp)), numfstr(sub->gold));
		sendf( ch, "     Height : [%d] cm, Weight [%d] lbs", sub->height, sub->weight );
        sendf( ch, "       Stat : Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]",
        			GET_STR(sub), GET_ADD(sub), GET_INT(sub),
        			GET_WIS(sub), GET_DEX(sub), GET_CON(sub) );
      	sendf( ch, "     Points : Hit: <%d/%d+%d>, Mana: <%d/%d+%d>, Moves: <%d/%d+%d>",
        			GET_HIT(sub),	 hit_limit(sub),  hit_gain(sub),
        			GET_MANA(sub), mana_limit(sub), mana_gain(sub),
        			GET_MOVE(sub), move_limit(sub), move_gain(sub) );
      	sendf( ch, "         AC : %d/10, , Hitroll %d, Damroll %d, Regen %d",
        			GET_AC(sub), sub->hr, sub->dr, sub->regen );
 	if( IS_NPC(sub) ) 
	{
        sendf( ch, "  Bare Hand : %dD%d.", sub->ndice, sub->sdice);
    }
      	sendf( ch, "    Carried : Weight : %d, Items: %d", sub->carry_weight, sub->carry_items );

      	for( i = 0, j = 0 ; i < MAX_WEAR; i++ ) if( sub->equipment[i] ) j++
		;
      	for( i = 0, obj = sub->carrying; obj; obj = obj->next_content, i++ )
		;

      	sendf( ch, "      Items : In inv : %d, In equ : %d", i, j );

   	if( !IS_NPC( sub ))
	{
        sendf( ch, "     Thirst : %d, Hunger: %d, Drunk: %d",
              sub->conditions[THIRST], sub->conditions[FULL], sub->conditions[DRUNK]);

        sendf( ch, "  Practices : %d", sub->prNr );
    }
	if( sub->master )
      	sendf( ch, "     Master : '%s'", (sub->master) ? GET_NAME(sub->master) : "Nobody");
	if( sub->followers )
	{
      	sendf( ch, "  Followers :" );
      	for( fol = sub->followers; fol; fol = fol->next )
			sendf( ch, "\t  %s", IS_NPC(fol->follower) ? fol->follower->moved : fol->follower->name );
	}
      sendf( ch, "Affected by : %s", sprintbit( sub->affects, affected_bits, 0 ) );

    if( sub->affected ) 
	{
        sendf( ch, "     Spells :" );

        for( af = sub->affected; af; af = af->next) 
		{
          sendf( ch, "\t%s: %s by %d, %d hrs, bits: %s ",
				spells[af->type ].name,
            	apply_types[(int)af->location],
				af->modifier,af->duration,
          		sprintbit( af->bitvector, affected_bits, 0) );
        }
  	}

    if( !IS_NPC( sub ) )
	{
        for( buf[0] = 0, i = 0; i < MAX_SKILLS; ++i )
		{
          	sprintf( buf + strlen( buf ), "%4d", sub->skills[i] );
          	if( !((i+1)%19) ) strcat( buf,"\n\r" );
        }

		sendf( ch, "%s", buf );
		sendf( ch, "Quest Solved: %s", what_is_solved( sub ) );
 	}
}

void do_shutdow(charType *ch, char *argument, int cmd)
{
  sendf( ch, "If you want to shut something down - say so!");
}

void do_shutdown(charType *ch, char *argument, int cmd)
{
	int			mins;
  	char 		arg[MAX_INPUT_LENGTH];

  	if( IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3) ) return;

  	oneArgument(argument, arg);

  	if( !*arg ) 
  	{
    	senddf( 0, 0, "Shutting down immediately." );
    	shutdowngame = 2;
  	} 
  	else if( *arg == '-' )
  	{
  		stopshutdown = 1;
  	}
  	else if( *arg == '+' )
  	{
  		stopshutdown = 0;
  	}
  	else
  	{
  		if( getnumber( arg, &mins ) >= 0 )
  		{
  			manualshutdown = mins;
  		}
  		else if( stricmp( arg, "stop" ) ) manualshutdown = 0;
  		else
  		{
  			sendf( ch, "Shutdown?!?  Valid arguments are '-', '+', '#' and 'stop'." );
  		}
  	}
}

void do_snoop(charType *ch, char *argument, int cmd)
{
  	char 				arg[MAX_INPUT_LENGTH];
  	charType 		*	vict;
  	int 				diff;

  	if( IS_NPC(ch) || !ch->desc ) return;

  	oneArgument( argument, arg );

  	if( !*arg ) 
  	{
    	sendf( ch, "Snoop who ?" ); return;
  	}
  	if( !(vict=find_char(ch, arg)) ) 
  	{
    	sendf( ch, "No such person around." ); return;
  	}
  	if( !vict->desc ) 
  	{
    	sendf( ch, "There's no link.. nothing to snoop."); return;
  	}
  	if( vict == ch ) 
  	{
    	sendf( ch, "Ok, you just snoop yourself." );
    	if( ch->desc->snoop.who ) 
		{
        	ch->desc->snoop.who->desc->snoop.by = 0;
        	ch->desc->snoop.who = 0;
    	}
    	return;
  	}
  	if( vict->desc->snoop.by )
  	{
    	sendf( ch, "Busy already." ); return;
  	}
  	if( diff = GET_LEVEL(ch)-GET_LEVEL(vict), diff <= 0 )
  	{
    	send_to_char("You failed.\n\r",ch);
    	return;
  	}
  	if( ch->desc->snoop.who ) ch->desc->snoop.who->desc->snoop.by = 0;
  	ch->desc->snoop.who 	= vict;
  	vict->desc->snoop.by 	= ch;
  	sendf( ch, "Ok." );
}

void do_switch( charType * ch, char * argument, int cmd )
{
  	char 			arg[MAX_INPUT_LENGTH];
  	charType 	*	vict;

	return;
  	if( IS_NPC(ch) ) return;

  	oneArgument( argument, arg );

  	if( !*arg ) 
  	{
    	send_to_char("Switch with who?\n\r", ch);
  	} 
  	else 
  	{
    	if( !(vict = find_char( ch, arg )) )	sendf( ch, "They aren't here." );
    	else 
		{
      		if( ch == vict ) 
	  		{
        		sendf( ch, "He he he... We are jolly funny today, eh?" );
        		return;
      		}
      		if( !ch->desc || ch->desc->snoop.by || ch->desc->snoop.who ) 
	  		{
        		sendf( ch, "You can't do that, the body is already in use." ); return;
      		}
      		if( vict->desc || (!IS_NPC(vict)) ) 
	  		{
        		if( GET_LEVEL(vict) > GET_LEVEL(ch) ) sendf( ch, "They aren't here." );
        		else sendf( ch, "You can't do that, the body is already in use!" );
      		} 
	  		else 
			{
        		sendf( ch, "Ok.");
        		ch->desc->character = vict;
        		ch->desc->original = ch;
        		vict->desc = ch->desc;
        		ch->desc = 0;
      		}
    	}
  	}
}

void do_return(charType *ch, char *argument, int cmd)
{
  	if( !ch->desc) return;

  	if( !ch->desc->original )
   	{ 
    	sendf( ch, "Eh?" ); return;
  	}
  	else
  	{
    	send_to_char("You return to your originaly body.\n\r",ch);
	
    	ch->desc->character = ch->desc->original;
    	ch->desc->original  = 0;

    	ch->desc->character->desc = ch->desc; 
    	ch->desc = 0;
  	}
}

void do_force( charType *ch, char * argument, int cmd )
{
  	descriptorType 	*	i;
  	charType 		*	vict;
  	char 				name[100], to_force[100], buf[100]; 
  	int 				diff;

  	if( IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3) ) return;

  	halfchop(argument, name, to_force);

  	if( !*name || !*to_force ) sendf( ch, "Who do you wish to force to do what?" );
  	else if( stricmp("all", name) != 0 ) 
  	{
		if( !(vict = find_char_room( ch, name ) ) )
		{
    		if( !(vict = find_char(ch, name)) ) 
			{
				sendf( ch, "No-one by that name here.." );
				return;
			}
		}

		{
      		diff = GET_LEVEL(ch) - GET_LEVEL(vict);
      		if( (!IS_NPC(vict)) && (diff <= 0) )
	  		{
        		sendf( ch, "Oh no you don't!!" );
      		} 
	  		else 
			{
        		if( GET_LEVEL(ch) < (IMO+3) )
          			sprintf(buf, "$n has forced you to '%s'.", to_force);
        		else
          			buf[0]=0;
        		act(buf, FALSE, ch, 0, vict, TO_VICT);
        		send_to_char("Ok.\n\r", ch);
        		interpreter( vict, to_force, 1 );
      		}
    	}
  	} 
  	else 
  	{ /* force all */
    	if( GET_LEVEL(ch) < (IMO+3) )
		{
      		sendf( ch, "Force all's are a bad idea these days." ); return;
    	}

    	for( i = desc_list; i; i = i->next )
      		if( i->character != ch && !i->connected ) 
	  		{
        		vict = i->character;
        		interpreter( vict, to_force, 1 );
      		}
    	send_to_char("Ok.\n\r", ch);
  	}
}

void do_load(charType *ch, char *argument, int cmd)
{
  	charType 		*	mob;
  	objectType 		*	obj;
  	char 				type[100], num[100];
  	int 				number, r_num;

  	if(IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3)) return;

  	twoArgument( argument, type, num );

  	if(!*type || !*num || !isdigit(*num)) 
  	{
    	sendf( ch, "Syntax:\n\rload <'char' | 'obj'> <number>." );
    	return;
  	}

  	if( (number = atoi(num)) < 0 ) 
  	{
    	sendf( ch, "A NEGATIVE number??" );
    	return;
  	}

  	if( isprefix(type, "char") ) 
  	{
    	if( (r_num = real_mobileNr(number)) == MOBILE_NULL ) 
		{
      		send_to_char("There is no monster with that number.\n\r", ch);
      		return;
    	}

    	mob = load_a_mobile( r_num, REAL );
    	char_to_room( mob, ch->in_room );
    	act( "$n makes a quaint, magical gesture with one hand.", TRUE, ch, 0, 0, TO_ROOM);
    	act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
    	send_to_char("Done.\n\r", ch);
    	log( "%s loaded char %s(#%d)", ch->name, mob->name, number );
  	}
  	else if( isprefix( type, "obj") ) 
  	{
    	if( (r_num = real_objectNr(number)) <= OBJECT_NULL ) 
		{
      		sendf( ch, "There is no object with that number." ); return;
    	}

    	obj = load_a_object( r_num, REAL, 1 );

    	if( GET_LEVEL(ch) < (IMO+2) && IS_SET(obj->extra,ITEM_NOLOAD) )
		{
      		sendf( ch, "That item is not loadable." );
      		extract_obj( obj, 1 );
      		log( "%s tried to load %d", ch->name, number );
      		return;
    	}
    	act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    	act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    	if( IS_SET( obj->wear, ITEM_TAKE ))
		{
      		obj_to_char(obj,ch);
      		act("$n snares $p!\n\r",FALSE,ch,obj,0,TO_ROOM);
    	} 
		else obj_to_room(obj, ch->in_room);
    	sendf( ch, "Ok." );
    	log( "%s loaded object %s(#%d)", ch->name, obj->name, number );
  	} 
  	else sendf( ch, "That'll have to be either 'char' or 'obj'." );
}

void roll_abilities( charType *ch )
{
  	int i, j, k, temp, sum, count;
  	byte table[5];
	byte table2[5] = { 9, 11, 13, 15, 17 };
	byte rools[6];

  	count = 20;
  	while(--count > 0) 
  	{
		for(i=0; i<5; table[i++]=0)  ;

		for( sum = 0, i = 0; i < 5; i++ ) 
		{
			for( j = 0; j < 6; j++)	rools[j] = number(1,3);
			temp = rools[0]+rools[1]+rools[2]+rools[3]+rools[4]+rools[5] + 2;
			temp = MIN( 18, temp );
			sum += temp ;
			for( k = 0; k < 5; k++)
				if( table[k] < temp) SWITCH(temp, table[k]);
		}
		if( sum >= 70 && sum <= 73 )			/*  It's average 14-15 */
			break ;
	}

	if( count <= 0 ) memcpy( table, table2, 5 );

  	ch->base_stat.str_add = 0;

  	switch (GET_CLASS(ch)) 
  	{
	case CLASS_MAGIC_USER: {
	  	ch->base_stat.intel = table[0];
	  	ch->base_stat.wis   = table[1];
	  	ch->base_stat.dex   = table[2]; 
	  	ch->base_stat.str   = table[3];
	  	ch->base_stat.con   = table[4];
   		}  break;
   	case CLASS_CLERIC: {
      	ch->base_stat.wis   = table[0];
      	ch->base_stat.intel = table[1];
      	ch->base_stat.str   = table[2];
      	ch->base_stat.dex   = table[3];
      	ch->base_stat.con   = table[4];
      	ch->base_stat.str_add = number(10,20);
    	} break;
   	case CLASS_THIEF: {
      	ch->base_stat.dex   = table[0];
      	ch->base_stat.str   = table[1];
      	ch->base_stat.con   = table[2];
      	ch->base_stat.intel = table[3];
      	ch->base_stat.wis   = table[4];
    	} break;
   	case CLASS_WARRIOR: {
      	ch->base_stat.str   = table[0];
      	ch->base_stat.dex   = table[1];
      	ch->base_stat.con   = table[2];
      	ch->base_stat.wis   = table[3];
      	ch->base_stat.intel = table[4];
      	ch->base_stat.str_add = number(30,40);
    	} break;
  	}
  	ch->temp_stat = ch->base_stat;
}

void do_start(charType *ch)
{
  	int 		i;

  	sendf( ch, "Welcome. This is now you character in DikuMud,\n\r"
			 	"You can now earn XP, and lots more...", ch);

  	GET_LEVEL(ch) = 1;
  	GET_EXP(ch) = 1;

  	set_title(ch);

  	ch->max_hit  = 10;  /* These are BASE numbers   */
  	ch->max_move  = 2;  /* These are BASE numbers   */
  	ch->max_mana  = 0;  /* These are BASE numbers   */

  	for( i = 0 ; i < MAX_SKILLS ; i++ ) ch->skills[i] = 0 ;

	clear_all_quest(ch) ;

  	switch( GET_CLASS(ch) ) 
  	{
    	case CLASS_THIEF : {
      		ch->skills[SKILL_SNEAK] = 10;
      		ch->skills[SKILL_HIDE] =  5;
      		ch->skills[SKILL_STEAL] = 15;
      		ch->skills[SKILL_BACKSTAB] = 10;
      		ch->skills[SKILL_PICK_LOCK] = 10;
    		} break;
  	}

  	advance_level(ch);
  	GET_HIT(ch) = hit_limit(ch);
  	GET_MANA(ch) = mana_limit(ch);
  	GET_MOVE(ch) = move_limit(ch);
  	GET_COND(ch,THIRST) = 24;
  	GET_COND(ch,FULL) = 24;
  	GET_COND(ch,DRUNK) = 0;
  	ch->played = 0;
  	ch->logon = time(0);
}

void do_advance(charType *ch, char *argument, int cmd)
{
  	charType 		*	vict;
  	char 				name[100], level[100];
  	int 				i, adv, newlevel;

  	if( IS_NPC(ch) ) return;

  	log( "%s advanced %s", GET_NAME(ch), argument);

  	if( GET_LEVEL(ch) < (IMO+3) )
  	{
    	send_to_char("You can only do that in a guild.\n\r",ch);
    	return;
  	}

  	twoArgument( argument, name, level );

  	if( *name ) 
  	{
    	if( !(vict = find_char_room(ch, name)) ) 
		{
      		sendf( ch, "That player is not here." ); return;
    	}
  	} 
  	else 
  	{
    	sendf( ch, "Advance who?" ); return;
  	}
  	if( IS_NPC(vict) ) 
  	{
    	sendf( ch, "NO! Not on NPC's." ); return;
  	}

  	if( GET_LEVEL(vict) == 0 ) newlevel = adv = 1;
  	else if( !*level ) 
  	{
    	sendf( ch, "You must supply a level number." ); return;
  	} 
  	else 
  	{
    	if (!isdigit(*level)) 
		{
   		   	sendf( ch, "Second argument must be a positive integer." );	return;
    	}
    	if( (newlevel = atoi(level)) < GET_LEVEL(vict) ) 
		{
       		if(GET_LEVEL(ch) <= GET_LEVEL(vict))
	   		{
        		sendf( ch, "Very amusing." ); return;
       		}
       		vict->level=newlevel;
       		if( newlevel < IMO )
	   		{
        		for( i = 0; i < 3; ++i ) 
					if( vict->conditions[i] > 0 ) vict->conditions[i] = 0;
       		}
       		vict->temp_stat = vict->base_stat;
       		sendf( ch, "The poor soul..." );
       		sendf( vict, "You have been punished." );
       		return;
    	}
  	}
  	adv = newlevel - GET_LEVEL(vict);
  	if( ( newlevel  >= 1) && (GET_LEVEL(ch) < (IMO+3) ) && (GET_LEVEL(ch) > (IMO+3))) 
  	{
    	sendf( ch, "Thou art not godly enough." ); return;
  	}
  	if( newlevel  > (IMO+3)) 
  	{
    	sendf( ch, "Too high a level." ); return;
  	}

  	sendf( ch, "You feel generous." );

  	act("$n makes some strange gestures.\n\rA strange feeling comes uppon you,"
      	"\n\rLike a giant hand, light comes down from\n\rabove, grabbing your "
      	"body, that begins\n\rto pulse with coloured lights from inside.\n\rYo"
      	"ur head seems to be filled with deamons\n\rfrom another plane as your"
      	" body dissolves\n\rto the elements of time and space itself.\n\rSudde"
      	"nly a silent explosion of light snaps\n\ryou back to reality. You fee"
      	"l slightly\n\rdifferent.",FALSE,ch,0,vict,TO_VICT);

  	log( "%s advances %s to %d", GET_NAME(ch), GET_NAME(vict), newlevel );

  	if( GET_LEVEL(vict) == 0 ) 
  	{
    	do_start(vict);
  	} 
  	else 
  	{
    	if( GET_LEVEL(ch) < (IMO+3) || GET_LEVEL(vict) < (IMO+3) ) 
		{
      		vict->exp = 1;
      		gain_exp_and_adv( vict, 
	  			(titles[GET_CLASS(vict)-1][GET_LEVEL(vict)+adv].exp)-GET_EXP(vict));
    	} 
		else 
		{
      		sendf( vict, "Some idiot just tried to advance your level." );
      		sendf( ch, "IMPOSSIBLE! IDIOTIC!\n\r" );
    	}
  	}
}

void do_reroll(charType *ch, char *argument, int cmd)
{
  	charType 		*	vict;
  	char 				buf[100];

  	if( IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3) ) return;

  	oneArgument(argument,buf);

  	if( !*buf ) send_to_char("Who do you wish to reroll?\n\r",ch);
  	else
    	if( !(vict = find_char( ch, buf )) ) sendf( ch, "No-one by that name in the world." );
    	else 
		{
      		roll_abilities( vict );
      		sendf( ch, "Rerolled: str %d/%d,  int %d,  wis %d,  dex %d,  con %d",
	  			vict->base_stat.str,
        		vict->base_stat.str_add,
        		vict->base_stat.intel,
        		vict->base_stat.wis,
        		vict->base_stat.dex,
        		vict->base_stat.con);
			sendf( vict, "Rerolled: str %d/%d,  int %d,  wis %d,  dex %d,  con %d",
				vict->base_stat.str,
        		vict->base_stat.str_add,
        		vict->base_stat.intel,
        		vict->base_stat.wis,
        		vict->base_stat.dex,
        		vict->base_stat.con);
    	}
}

void do_restore(charType *ch, char *argument, int cmd)
{
	descriptorType		*	d;
  	charType 			*	vict;
  	char 					buf[100];
  	int 					i;

  	if( IS_NPC(ch) || GET_LEVEL(ch) < (IMO+3)) return;

  	oneArgument(argument,buf);

  	if( !*buf ) sendf( ch, "Who do you wish to restore?" );
  	else if( stricmp( buf, "all" ) != 0 )
  	{
    	if( !(vict = find_char( ch, buf )) )
		{
      		sendf( ch, "No-one by that name in the world." );
			return;
		}
    	else 
		{
      		GET_MANA(vict) = MAX( GET_MAX_MANA(vict), GET_MANA(vict) );
      		GET_HIT(vict)  = MAX( GET_MAX_HIT(vict), GET_HIT(vict) );
      		GET_MOVE(vict) = MAX( GET_MAX_MOVE(vict), GET_MOVE( vict ) );

      		if( GET_LEVEL(vict) >= IMO ) 
	  		{
        		for( i = 0; i < MAX_SKILLS; i++ ) 
				{
          			vict->skills[i] = 100;
        		}

        		if( GET_LEVEL(vict) >= (IMO+3) ) 
				{
          			vict->base_stat.str_add = 100;
          			vict->base_stat.intel = 18;
          			vict->base_stat.wis = 18;
          			vict->base_stat.dex = 18;
          			vict->base_stat.str = 18;
          			vict->base_stat.con = 18;
		  		}
        	}
        	vict->temp_stat = vict->base_stat;
      	}
      	update_pos( vict );
      	sendf( ch, "Done." );
      	act("You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR);
    }
   	else
   	{
    	if( GET_LEVEL(ch) < (IMO+3) )
		{
      		sendf( ch, "Restore all's are a bad idea these days." ); return;
    	}

    	for( d = desc_list; d; d = d->next )
      		if( d->character != ch && !d->connected ) 
	  		{
        		vict = d->character;
      			GET_MANA(vict) = MAX( GET_MAX_MANA(vict), GET_MANA(vict) );
      			GET_HIT(vict)  = MAX( GET_MAX_HIT(vict), GET_HIT(vict) );
      			GET_MOVE(vict) = MAX( GET_MAX_MOVE(vict), GET_MOVE( vict ) );
      			update_pos( vict );
      			act( "You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR);
      		}
    	send_to_char("Ok.\n\r", ch);
	}
}

void do_noshout(charType *ch, char *argument, int cmd)
{
  	charType 	*	vict;
  	objectType 	*	dummy;
  	char 			buf[MAX_INPUT_LENGTH];

  	if( IS_NPC(ch) ) return;

  	oneArgument(argument, buf);

  	if( !*buf ) 
  	{
    	if( IS_SET(ch->act, PLR_EARMUFFS) ) 
		{
      		sendf( ch, "You can now hear shouts again." );
      		REMOVE_BIT(ch->act, PLR_EARMUFFS);
    	} else {
      		sendf( ch, "From now on, you won't hear shouts." );
      		SET_BIT(ch->act, PLR_EARMUFFS);
    	}
    	return;
  	}

  	if(GET_LEVEL(ch) < IMO) return;

  	if( !find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy) )
    	sendf( ch, "Couldn't find any such creature." );
  	else if( IS_NPC(vict) )
    	sendf( ch, "Can't do that to a beast." );
  	else if( GET_LEVEL(vict) > GET_LEVEL(ch) )
    	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  	else if( IS_SET(vict->act, PLR_NOSHOUT) ) 
  	{
    	sendf( vict, "You can shout again." );
    	sendf( ch, "NOSHOUT removed." );
    	REMOVE_BIT(vict->act, PLR_NOSHOUT);
  	} 
  	else 
  	{
    	sendf( vict, "The gods take away your ability to shout!" );
    	sendf( ch, "NOSHOUT set." );
    	SET_BIT(vict->act, PLR_NOSHOUT);
  	}
}

void do_wiznet( charType * ch, char * argument, int cmd )
{
  	descriptorType 	*	i;
  	charType 		*	vict;

  	if( IS_NPC(ch) ) return;

  	for( i = desc_list; i; i = i->next )
    	if( !i->connected ) 
		{
      		if( i->original ) continue;

      		vict = i->character;
      		if( (GET_LEVEL(vict) >= IMO) && (GET_LEVEL(vict) <= (IMO+4)) ) 
	  		{
        		sendf( vict, "[%s - %s]", can_see(vict, ch) ? GET_NAME(ch) : "Someone" , argument );
      		}
    	}
  	sendf( ch, "Ok." );
}

void do_noaffect(charType *ch, char *argument, int cmd)
{
  	charType 		*	vict;
  	objectType 		*	dummy;
  	affectType 		*	hjp;
  	char 				buf[MAX_INPUT_LENGTH];

  	if( IS_NPC(ch) ) return;

  	oneArgument( argument, buf );

  	if( !*buf )
  	{
    	send_to_char("Remove affects from whom?\n\r", ch);
    	return;
  	} 
  	else 
  	{
    	if( !find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy) )
      		sendf( ch, "Couldn't find any such creature." );
    	else if( IS_NPC(vict) )
      		sendf( ch, "Can't do that to a beast." );
    	else if( GET_LEVEL(vict) > GET_LEVEL(ch) )
      		act("$E might object to that.. better not.",0,ch,0,vict,TO_CHAR);
    	else
		{
      		sendf( vict, "You are normal again." );
      		for( hjp = vict->affected; hjp; hjp = hjp->next )
        		affect_remove( vict, hjp );

			vict->affects = vict->affects & AFF_GROUP;
    	}
  	}
  	sendf( ch, "Ok." );
}

void do_wall(charType *ch, char *argument, int cmd)
{
  	if(IS_NPC(ch)||(! *argument) || GET_LEVEL(ch) > (IMO+3)) return;

  	senddf( 0, 0, "%s", skipsps( argument ) );
  	sendf( ch, "Ok." );
}

void do_set(charType *ch, char *argument, int cmd)
{
  	charType 	*	vict;
  	char 			buf[64],buf2[32],buf3[32],buf4[32];
  	int 			k = 0, i;

  	static const char * flags[] =
  	{
  		"nokill", "nosteal", "freeze", "nochat", "nologon", "nospecials", 
  		"noshout", "\n"
  	};

  	if( IS_NPC(ch) ) return;

  	log( "%s set %s", GET_NAME(ch), argument);

  	halfchop( argument, buf, buf2 );

  	if( !*buf )
  	{
    	sendf( ch,  "nokill    = %d\n\r"
					"nosteal   = %d\n\r"
                    "freeze    = %d\n\r"
					"noshout   = %d\n\r"
                    "nochat    = %d\n\r"
                    "nologin   = %d\n\r"
					"nospecial = %d",
      		nokillflag, nostealflag, nonewplayers, noshoutflag,
      		nochatflag, nologin, no_specials );
	  	return;
  	} 

  	if( i = isinlist( buf, flags ), i >= 0 )
  	{
  		onefword( buf2, buf3 );

		switch( i )
		{
			case 0 : if( *buf3 ) k = nokillflag	 = atoi( buf3 );
					 else		 k = nokillflag; 				break;
			case 1 : if( *buf3 ) k = nostealflag = atoi( buf3 );
					 else		 k = nokillflag;				break;
			case 2 : if( *buf3 ) k = nonewplayers= atoi( buf3 );
					 else		 k = nokillflag;				break;
			case 3 : if( *buf3 ) k = nochatflag  = atoi( buf3 );
					 else		 k = nokillflag;				break;
			case 4 : if( *buf3 ) k = nologin 	 = atoi( buf3 );
					 else		 k = nokillflag;				break;
			case 5 : if( *buf3 ) k = no_specials = atoi( buf3 );
					 else		 k = nokillflag;				break;
			case 6 : if( *buf3 ) k = noshoutflag = atoi( buf3 );
					 else		 k = nokillflag;				break;
		}
      	sendf( ch, "%s is %d.", flags[i], k );
      	return;
    }

    if( !(vict = find_char( ch, buf )) )
      sendf( ch, "No-one by that name in the world." );
    else 
	{
      	if( (GET_LEVEL(ch) < (IMO+2)) ) return;

      	halfchop( buf2, buf3, buf4 ); k = atoi( buf4 );

      	if( strcmp( buf3, "skill" ) == 0 )
	  	{
        	for( i = 0; i < MAX_SKILLS; i++ ) vict->skills[i]=0;
      	}
      	else if( strcmp(buf3,"quest") == 0 ) clear_all_quest(vict);
      	else if( strcmp(buf3,"lev") ==0 ) 
	  	{
         	if( implementor(GET_NAME(ch)) )  GET_LEVEL(vict) = k;
        }
      	else if( strcmp( "age", buf3 ) ==0 )
	  	{ 
			vict->birth  = time(0);
       		vict->played = 0;
       		vict->logon  = time(0); 
	   	}
      	else if( stricmp( "exp",    buf3) ==0 ) vict->exp                = k;
      	else if( stricmp( "hit",    buf3) ==0 ) vict->hit                = vict->max_hit  = k;
      	else if( stricmp( "mana", 	buf3) ==0 ) vict->mana			   = vict->max_mana = k;
      	else if( stricmp( "move", 	buf3) ==0 ) vict->move			   = vict->max_move = k;
      	else if( stricmp( "gold", 	buf3) ==0 ) vict->gold			   = k;
      	else if( stricmp( "bank", 	buf3) ==0 ) vict->bank			   = k;
      	else if( stricmp( "align",	buf3) ==0 ) vict->align			   = k;
      	else if( stricmp( "str",	buf3) ==0 ) vict->base_stat.str	   = k;
      	else if( stricmp( "str_add",buf3) ==0 ) vict->base_stat.str_add  = k;
      	else if( stricmp( "dex",	buf3) ==0 ) vict->base_stat.dex	   = k;
      	else if( stricmp( "wis",	buf3) ==0 ) vict->base_stat.wis	   = k;
      	else if( stricmp( "con",	buf3) ==0 ) vict->base_stat.con	   = k;
      	else if( stricmp( "int",	buf3) ==0 ) vict->base_stat.intel	   = k;
      	else if( stricmp( "pra",	buf3) ==0 ) vict->prNr			   = k;
      	else if( stricmp( "hunger", buf3) ==0 ) vict->conditions[FULL]   = k;
      	else if( stricmp( "thirsty",buf3) ==0 ) vict->conditions[THIRST] = k;
      	else if( stricmp( "drunk",	buf3) ==0 )	vict->conditions[0]	   = k;
       	else if( stricmp( "dr",		buf3) ==0 )	vict->dr				   = k;
      	else if( stricmp( "hr",		buf3) ==0 )	vict->hr				   = k;
      	else if( stricmp( "ac",		buf3) ==0 )	vict->armor			   = k; 
	  	else if( stricmp( "sex",    buf3) ==0 )
		{
			if     ( k == 1 )	ch->sex = SEX_MALE;
			else if( k == 2 ) 	ch->sex = SEX_FEMALE;
		}
	  	else if( stricmp( "name",   buf3) ==0 )
	  	{
			if( find_name( buf4 ) != -1 )
			{
				sendf( ch, "That name is already used." ); return;
			}

			if( !vict->desc )
			{
				sendf( ch, "Try when victim has connection." ); return;
			}

			k = find_name( vict->name );
			errFree( player_table[k].name );
			player_table[k].name = strdup( buf4 );
			wipe_stash( vict->name, 0 );
			move_locker( vict->name, buf4 );
	  		vict->name = strdup(buf4);
			do_save( vict, "", COM_SAVE );
		}
      	else sendf( ch, "Huh?" );

      	vict->temp_stat = vict->base_stat;
    }
}

void do_invis(charType *ch, char *argument, int cmd)
{
  	if( IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3) ) return;

  	if( IS_SET(ch->act,PLR_WIZINVIS) )
  	{
    	REMOVE_BIT(ch->act,PLR_WIZINVIS);
    	sendf( ch, "You are visible again." );
  	} 
  	else 
  	{
    	SET_BIT(ch->act,PLR_WIZINVIS);
    	sendf( ch, "You vanish." );
  	}
  	sendf( ch, "Ok." );
}

void do_banish(charType *ch, char *argument, int cmd)
{
  	charType 	*	vict;
  	objectType 	*	dummy;
  	char 			buf[MAX_INPUT_LENGTH];
  	int 			location;

  	if( IS_NPC(ch) ) return;

  	oneArgument(argument, buf);

  	if( !*buf ) sendf( ch, "Banish whom?" );
  	else 
  	if( !find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy) )
    	sendf( ch, "Couldn't find any such creature." );
  	else 
  	if( IS_NPC(vict) ) 	sendf( ch, "Can't do that to a beast." );
  	else 
  	if( GET_LEVEL(vict) >= IMO ) sendf( ch, "It's pointless to banish an immortal." );
  	else if( IS_SET(vict->act, PLR_BANISHED) ) 
  	{
    	REMOVE_BIT( vict->act, PLR_BANISHED );
    	sendf( vict, "You feel forgiven?" );
    	act( "$N is forgiven.", FALSE, ch, 0, vict, TO_CHAR );
  	} 
  	else 
  	{
    	SET_BIT( vict->act, PLR_BANISHED );
		if( location = real_roomNr( ROOM_BANISH ), location == NOWHERE )
		{
			sendf( ch, "Death's Room is gone?" ); return;
		}
    	else 
		{
      		act( "$n disappears in a puff of smoke.", FALSE, vict, 0, 0, TO_ROOM );
      		char_from_room(vict);
      		char_to_room(vict,location);
      		act( "$n appears with an ear-splitting bang.", FALSE, vict, 0, 0, TO_ROOM );
    	}
    	sendf( vict, "You smell fire and brimstone?"  );
    	act( "$N is banished.", FALSE, ch, 0, vict, TO_CHAR );
		senddf( vict, 0, "%s is banished.", vict->name );
  	}
  	sendf( ch, "OK." );
}

void do_flag(charType *ch, char *argument, int cmd)
{
  	charType 	*	vict;
  	objectType 	*	dummy;
  	char 			buf[MAX_INPUT_LENGTH];
  	int 			f;

  	if( IS_NPC(ch) ) return;

  	onefword( argument, buf );

  	if( !*buf )
    	send_to_char("Flag whom?\n\r", ch);
  	else if (!find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    	send_to_char("Couldn't find any such creature.\n\r", ch);
  	else if (IS_NPC(vict))
    	send_to_char("Can't do that to a beast.\n\r", ch);
  	else if ((GET_LEVEL(vict) >= IMO)&&(cmd != COM_LOG))
    	send_to_char("It's pointless to flag an immortal.\n\r",ch);
  	else 
  	{
    	f = (cmd == COM_LOG) ? IS_SET(vict->act, PLR_XYZZY) : IS_SET(vict->act, PLR_CRIMINAL) ;
    	if( f ) 
		{
      		if( cmd == COM_LOG )
	  		{
        		REMOVE_BIT(vict->act, PLR_XYZZY);
        		sendf( ch, "Log removed." );
      		} 
	  		else 
			{
        		REMOVE_BIT(vict->act, PLR_CRIMINAL);
        		sendf( ch, "Flag removed." );
      		}
    	} 
		else 
		{
      		if( cmd == COM_LOG )
	  		{
        		SET_BIT(vict->act, PLR_XYZZY);
        		sendf( ch, "Log set." );
      		} 
	  		else 
			{
        		SET_BIT(vict->act, PLR_CRIMINAL);
        		sendf( ch, "Flag set." );
      		}
    	}
  	}
}

void do_flick(charType *ch, char *argument, int cmd)
{
  charType *vict;
  objectType *obj;
  char victim_name[240];
  char obj_name[240];
  int eq_pos;

  argument = oneArgument(argument, obj_name);
  oneArgument(argument, victim_name);
  if (!(vict = find_char(ch, victim_name))) {
    send_to_char("Who?\n\r", ch);
    return;
  } else if (vict == ch) {
    send_to_char("Odd?\n\r", ch);
    return;
  } else if(GET_LEVEL(ch) <= GET_LEVEL(vict)){
    send_to_char("Bad idea.\n\r",ch);
    return;
  }
  if( !(obj = find_obj_inven( vict, obj_name )) ) {
    for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
      if(vict->equipment[eq_pos] &&
        (isoneof(obj_name, vict->equipment[eq_pos]->name))){
        obj = vict->equipment[eq_pos];
        break;
      }
    if (!obj) {
      send_to_char("Can't find that item.\n\r",ch);
      return;
    } else { /* It is equipment */
      obj_to_char(unequip_char(vict, eq_pos), ch);
      send_to_char("Done.\n\r", ch);
    }
  } else {  /* obj found in inventory */
    obj_from_char(obj);
    obj_to_char(obj, ch);
    send_to_char("Done.\n\r", ch);
  }
}


void do_sys(charType *ch, char *argument, int cmd)
{
/*
  	struct rusage xru;

  	getrusage( 0, &xru );

  	sendf( ch, "sys time : %d secs\n\r"
			   "usr time : %d secs\n\r"
			   "run time : %d secs\n\r",
    		   xru.ru_stime.tv_sec,xru.ru_utime.tv_sec,time(0)-boottime);
*/
}

char *immo_members[] = 
{
	"Nuis", "Nuisance", "Danbi", "Mirinae", "Nisten", "Wony", ""
} ;

int implementor(char *name)
{
	int 		i;
	char 	* 	cp;

	for ( i = 0 ;; i ++ ) 
	{
		if( cp = immo_members[i], !*cp ) break; 
		if( strcmp( cp, name ) == 0 ) return 1 ;
	}

	return 0 ; 
}
