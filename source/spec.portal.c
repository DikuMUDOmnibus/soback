#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "world.h"
#include "interpreter.h"
#include "specials.h"
#include "comm.h"
#include "strings.h"
#include "find.h"
#include "transfer.h"

typedef struct __portalunit__
{
	objectType				*	portal;
	struct __portalunit__	*	next;

} portalUnit;

static portalUnit		*	portals;

void record_portal( objectType * portal )
{
	portalUnit		*	prev, * new;

	new = (portalUnit *)errCalloc( sizeof(portalUnit) );
	new->portal = portal;

	if( !portals )
	{
		portals = new;
	}
	else
	{
		for( prev = portals; prev->next; prev = prev->next )
		;

		prev->next = new;
	}	
}

static void del_portalunit( portalUnit * pu )
{
	portalUnit		*	prev;

	if( portals == pu ) portals = pu->next;
	else
	{
		for( prev = portals; prev && prev->next != pu; prev = prev->next )
		;

		if( prev->next != pu )
		{
			FATAL( "del_portalunit> portal unit not found." );
		}

		prev->next = pu->next;
	}

	errFree( pu );
}

static void update_portal( void )
{
	portalUnit		*	curr, * curr_next;
	objectType		*	portal;
	int					room;

	for( curr = portals; curr; curr = curr_next )
	{
		curr_next = curr->next;

		portal = curr->portal;
		room = portal->in_room;

		if( --portal->value[2] == 5 )
		{
			sendrf( 0, room, "%s is starting to lose its aura.", portal->wornd );
		}
		else if( portal->value[2] == 2 )
		{
			sendrf( 0, room, "%s has lost its aura. It should disappear soon.", portal->wornd );
		}
		else if( portal->value[2] == 0 )
		{
			sendrf( 0, room, "%s disapears.", portal->wornd );
			extract_obj( portal, 1 );
			del_portalunit( curr );
		}
	}
}

static int portal_gate( charType * ch, int cmd, char * argu )
{
	objectType	*	portal;

	if( cmd != COM_ENTER ) return 0;
	if( IS_NPC( ch ) ) return 0;
	if( !isoneofp( argu, "portal magicportal" ) ) return 0;

	if( portal = find_obj_room( ch, "magicportal" ), !portal ) return 0;

	act( "$n enters the $p and $n disappears.", TRUE, ch, portal, 0, TO_ROOM );
	act( "You enters the $p. It's too bright here!", TRUE, ch, portal, 0, TO_CHAR );

	char_from_room( ch );
	char_to_room( ch, portal->value[0] );

	act( "In the middle of $p, $n appears suddenly.", TRUE, ch, portal, 0, TO_ROOM );
	sendf( ch, "As you leave the portal, You see you are in another place." );

	do_look( ch, "", COM_LOOK );
	return 1;
}

void init_portal_gate( void )
{
    specialUnitType     portal = { 0, "portal gate", update_portal, 0, 4, 0, 0 };

    add_specUnit( &portal );

	objects[OBJECT_PORTAL].func = portal_gate;
}
