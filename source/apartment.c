#include "defines.h"
#include "character.h"
#include "world.h"

#define MAX_ALLOW	8
#define MAX_FLOOR	100

typedef struct __apartroom__
{
	roomType				*	room;
	char					*	name;
	char					*	allow[MAX_ALLOW];
	char					*	passwd;
	int							using;
	objectType				*	eq1;
	objectType				*	eq2;
	objectType				*	eq3;
	objectType				*	eq4;
	int							floor;
	int							pos;
	int							nr;

	struct __apartroom__	*	next;

} apartType;

typedef struct __hallway__
{

	roomType					hall;
	apartType					rooms[4];
	int							floor;

} hallwayType;

static hallwayType		apart[MAX_FLOOR];

int apart_room( charType * ch, int cmd, char * argu )
{
	return 0;
}

int apart_office( charType * ch, int cmd, char * argu )
{
	return 0;
}

int apart_elivator( charType * ch, int cmd, char * argu )
{
	return 0;
}

int apart_hallway( charType * ch, int cmd, char * argu )
{
	return 0;
}

void init_apartment( void )
{
	int						i;

	for( i = 0; i < MAX_FLOOR; i++ ) 
	{
		apart[i].floor = i;
	}
}
