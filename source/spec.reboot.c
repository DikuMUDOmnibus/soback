#include "character.h"
#include "mobile.h"
#include "world.h"
#include "object.h"
#include "zone.h"
#include "specials.h"
#include "allocate.h"
#include "find.h"
#include "comm.h"
#include "utils.h"
#include "fight.h"
#include "transfer.h"
#include "variables.h"
#include "interpreter.h"

void auto_reboot( void );

specialUnitType 	*	rebootUnit;

void auto_reboot( void )
{
	charType	*		mob, * mob_next;
	objectType	*		obj, * obj_next;
	roomType	*		room, * room_next;
	charType	*		reboot, * shutdown, * reset;
	int					realNr;
    int     			t, hrs, mins;
	int					i, j, k;
	int					resets[5];

  	t = 30 + time(0) - boottime;

	hrs  = (t/3600); mins = ((t%3600)/60);

	realNr   = real_roomNr( ROOM_REBOOT );

	reset    = find_mob_room_at( 0, realNr, "reset" );
	reboot   = find_mob_room_at( 0, realNr, "reboot" );
	shutdown = find_mob_room_at( 0, realNr, "shutdown" );

	if( !reboot ) 
	{
		if( rebootUnit->nr++ > 10 ) 
		{
			reboot = load_a_mobile( MOBILE_REBOOT, 1 );
			char_to_room( reboot, realNr );
			rebootUnit->nr = 0;
		}
		else if( shutdown )
		{
			senddf( 0, 0, "%s shouts, 'PuHaHaHa. I'll shutdown this mud in %d seconds.'",
					shutdown->moved, 11 - rebootUnit->nr );
		}
		return;
	}
	else if( rebootUnit->nr > 5 ) rebootUnit->nr = 0;

	if( manualshutdown )
	{
		if( stopshutdown ) 
		{
			senddf( 0, 0, "%s shouts, 'Hmmmm. God prevent me from shuting down this mud!!'", 
						reboot->moved );
			return;
		}
		if( manualshutdown > 1 )
			senddf( 0, 0, "%s shouts, 'I'll pull the shutdown lever down after %d minite.'", 
						reboot->moved, --manualshutdown );
		else
		{
			senddf( 0, 0, "%s shouts, 'Good bye all mudders. See you after reboot!'", 
						reboot->moved, --manualshutdown );
			shutdowngame = 4;
		}
		return;
	}

	if( !shutdown || (hrs == 19 && mins >= 55) || (hrs >= 20) )
	{
		if( stopshutdown ) 
		{
			senddf( 0, 0, "%s shouts, 'Hmmmm. God prevent me from shuting down this mud!!'", 
						reboot->moved );
			return;
		}
		if( !shutdown && rebootUnit->nr == 0 )
		{
			senddf( 0, 0, "%s shouts, 'OooH~~ My friend, Shutdown gone!!'", reboot->moved );
		}

		if( rebootUnit->nr != 5 )
		{
			senddf( 0, 0, "%s shouts, 'I'll pull the shutdown lever down after %d minite.'",
						reboot->moved, 5 - rebootUnit->nr );
			rebootUnit->nr++;
		}
		else
		{
			senddf( 0, 0, "%s shouts, 'Good bye all mudders. See you after reboot!'", 
						reboot->moved, --manualshutdown );
			autoshutdown = 1;
			shutdowngame = 4;
		}
		return;
	}

	rebootUnit->nr = 0;

	if( !reset && number( 1, 3 ) == 1 )
	{
		reset = load_a_mobile( MOBILE_RESET, 1 );
		char_to_room( reset, realNr );

		do_shout( reset, "I hate this job. Let me stop this job, please!", 0 );
		do_shout( reset, "OOOF! What zone should I reset now?? I'm confused!", 0 );
		senddf( 0, 0, "You hear some clinking sounds." );

		for( i = 0; i < 5; i++ )
		{
			j = number( 2, zone_index_info.used );

  			if( zones[j].reset_mode == 0 )
			{
				i--; continue;
			}

			for( k = 0; k < i; k++ ) if( resets[k] == j ) break;

			if( k != i )
			{
				i--; continue;
			}
			resets[i] = j;
		}

		for( i = 0; i < 5; i++ )
		{
            senddf( 0, 44, "===> forcing reset of %s [ age %d - lifespan %d ]",
                         zones[resets[i]].name, zones[resets[i]].age, zones[resets[i]].lifespan );

			for( room = zones[resets[i]].rooms; room; room = room_next )
			{
				room_next = room->next;

				for( mob = room->people; mob; mob = mob_next )
				{
					mob_next = mob->next_in_room;

					if( IS_NPC( mob ) )	extract_char( mob );
				}
			}

			for( room = zones[resets[i]].rooms; room; room = room_next )
			{
				room_next = room->next;

				for( obj = room->contents; obj; obj = obj_next )
				{
					obj_next = obj->next_content;

					if( obj->nr >= OBJECT_INTERNAL ) extract_obj( obj, 1 );
				}
			}

			reset_zone( resets[i], 1 ); zones[resets[i]].age = 0;
		}
	}
}

int rebooter( charType * ch, int cmd, char * argu )
{
	charType		* reboot, * shutdown;

	if( cmd ) return 0;

	reboot   = ch;
	shutdown = find_mob_room_at( reboot, 0, "shutdown" );

	if( is_fighting( ch ) )
	{
		if( ch->fight == shutdown )
		{
			do_say( ch, "Oh~ My friend!! Sorry.", 0 );
			stop_fighting( ch );
			if( shutdown->fight && shutdown->fight == ch ) stop_fighting( shutdown );
		}
		else if( number( 1, 4 ) ) 
           	do_shout( ch, "Even if you kill me, I'll resurrect in 10 minutes!", 0 );
		return 0;
	}

	if( shutdown && is_fighting( shutdown ) )
	{
		if( number( 1, 4 ) == 1 ) 
			do_shout( ch, "Uuuuh~ Please, Do not kill my best friend, Shutdown!!", 0 );
		return 0;
	}
		
	switch( number( 1, 3000 ) )
	{
		case 499 : do_shout( ch, "I'll shutdown this mud when running time reachs 20:00.", 0 ); break;
	}
	return 0;
}

int shutdowner( charType * ch, int cmd, char * argu )
{
	charType		* reboot, * shutdown;

	if( cmd ) return 0;

	shutdown = ch;
	reboot   = find_mob_room_at( shutdown, 0, "reboot" );

	if( is_fighting( ch ) )
	{
		do_shout( ch, "Don't kill me! If I died, this mud will run eternally.", 0 );
		return 0;
	}

	if( reboot && is_fighting( reboot ) )
	{
		if( number( 1, 4 ) == 1 ) 
			do_shout( ch, "Fighting!! Kill Reboot!! Let me shutdown this mud", 0 );
		return 0;
	}
		
	switch( number( 1, 3000 ) )
	{
		case 499 : do_shout( ch, "I wanna shutdown this mud! Is thre anyone can help me??", 0 ); break;
		case 1   : do_shout( ch, "Let's kill Reboot. He is preventing me.", 0 ); break;
	}
	return 0;
}

int reseter( charType * ch, int cmd, char * argu )
{
	if( cmd ) return 0;

	if( is_fighting( ch ) )
	{
		if( number( 1, 4 ) == 1 ) do_shout( ch, "Yes!! Kill me!! I'll pray for you, mudderer!", 0 );
		return 0;
	}

	switch( number( 1, 5000 ) )
	{
		case 499 : do_shout( ch, "I hate my job. *sigh* I hate myself too.", 0 ); break;
		case 299 : do_shout( ch, "How can I escape this world???", 0 ); break;
		case 199 : do_shout( ch, "What zone should I reset this time???", 0 ); break;
		case   1 : do_shout( ch, "Heeeew.. This job is too hard to me.", 0 ); break;
	}
	return 0;
}

void init_autoreboot( void )
{
	specialUnitType 	autob = { 0, "auto_reboot", auto_reboot, 0, PULSE_ZONE, 0, 0 };  
	int					rNr, mNr;

	rNr = real_roomNr( ROOM_REBOOT );
	mNr = real_mobileNr( MOBILE_REBOOT );

	if( rNr == NOWHERE || mNr == MOBILE_NULL ) 
	{
		log( "init_autoreboot> init failed." ); return;
	}

	add_specUnit( &autob );

	rebootUnit = specUnit_list;

	mobiles[ real_mobileNr( MOBILE_REBOOT )   ].func = rebooter;
	mobiles[ real_mobileNr( MOBILE_SHUTDOWN ) ].func = shutdowner;
	mobiles[ real_mobileNr( MOBILE_RESET )    ].func = reseter;
}
