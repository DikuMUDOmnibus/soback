#include <string.h>

#include "character.h"
#include "mobile.h"
#include "object.h"
#include "world.h"
#include "zone.h"
#include "interpreter.h"
#include "comm.h"
#include "sockets.h"
#include "strings.h"
#include "utils.h"
#include "variables.h"
#include "edit.h"

typedef struct __edit_command__
{
	char	*		name;
	void   (*		func)( charType * ch, char * argu, int cmd );
	int				cmd;

} edit_commandType;

editUnitType	*	editing;

edit_commandType	edit_cmds[] =
{
	{	"help",		ed_help,	0		},
	{	"iedit",	do_iedit,	0		},
	{	"icopy",	ed_icopy,	0		},
	{	"medit",	do_medit,	0		},
	{	"mcopy",	ed_mcopy,	0		},
	{	"quit",		ed_quit,	0		},
	{	"redit",	do_redit,	0		},
	{	"rcopy",	ed_rcopy,	0		},
	{	"zedit",	do_zedit,	0		},
	{	"\n",		0					}
};

void ed_new_prompt( editUnitType * eu, char * prompt )
{
    if( eu->prompt ) errFree( eu->prompt );

    eu->prompt = strdup( prompt );
}

void edit_prompt( descriptorType * d )
{
	editUnitType	* eu;

	for( eu = editing; eu; eu = eu->next )
	{
		if( stricmp( eu->who, d->character->name ) == 0 ) break;
	}

	if( !eu )
	{
		write_to_descriptor( d->fd, "Edit> " );
	}
	else if( d->character->sub_3 )
	{
		switch( d->character->sub_3 )
		{
			case	EDIT_YESNO : write_to_descriptor( d->fd, "Edit - Yes/No ? " ); break;
			case	EDIT_YES   :
			case	EDIT_NO    : break;
			default			   : DEBUG( "edit_prompt> Unkown sub_mode 3 (%d).", d->character->sub_3 ); 
								 write_to_descriptor( d->fd, "Uknown 3rd sub-mode > " ); break;
		}
	}
	else
	{
		write_to_descriptor( d->fd, eu->prompt );
	}
}

void ed_help( charType * ch, char * argu, int cmd )
{
	sendf( ch, "-------> available editing commands." );
	sendf( ch, " item -> iedit, ifind, istat, icopy" );
	sendf( ch, " char -> medit, mfind, mstat, mcopy" );
	sendf( ch, " room -> redit, rfind, rstat, rcopy" );
	sendf( ch, "" );
}

void ed_quit( charType * ch, char * argu, int cmd )
{
	ch->mode  = 0;
	ch->sub_1 = 0;
	ch->sub_2 = 0;
	ch->sub_3 = 0;
}

editUnitType * find_editunit( charType * ch )
{
	editUnitType	*	curr;

	for( curr = editing; curr; curr = curr->next )
	{
		if( stricmp(curr->who, ch->name ) == 0 ) return curr;
	}
	return 0;
}

int find_editing( charType * ch, int type, int number )
{
	editUnitType	* 	curr;

	for( curr = editing; curr; curr = curr->next )
	{
		if( stricmp( curr->who, ch->name ) == 0 ) return 2;

		switch( type )
		{
			case EDIT_OBJ :	if( curr->obj  == number ) return 1;
							break;
			case EDIT_MOB :	if( curr->mob  == number ) return 1;
							break;
			case EDIT_ROOM:	if( curr->room == number ) return 1;
							break;
			case EDIT_ZONE:	if( curr->zone == number ) return 1;
							break;
		}
	}
	return 0;
}

editUnitType * new_editing( charType * ch, int type, int nr )
{
	editUnitType	*	curr;
	int					ret;

	if( ret = find_editing( ch, type, nr ), ret )
	{
		if( ret == 1 ) 
		{
			sendf( ch, "That resource not available now." ); return 0;
		}

		if( ret == 2 )
		{
			sendf( ch, "Ooops.. You are editing alreay." ); return 0;
		}
	}

	if( curr = errCalloc( sizeof( editUnitType ) ), !curr )
	{
		DEBUG( "new_editing> alloc failure." ); return 0;
	}

	curr->obj  = -1;
	curr->mob  = -1;
	curr->room = -1;

	curr->who  = strdup(ch->name);

	switch( type )
	{
		case EDIT_OBJ  : curr->obj  = nr; 
						 curr->oe   = errMalloc( sizeof(objIndexType) );
						 *curr->oe  = objects[nr];
						 break;
		case EDIT_MOB  : curr->mob  = nr; 
						 curr->me   = errMalloc( sizeof(mobIndexType) );
						 *curr->me  = mobiles[nr];
						 break;
		case EDIT_ROOM : curr->room = nr; 
						 curr->re   = errMalloc( sizeof(roomType) );
						 *curr->re  = world[nr];
						 curr->re->contents = 0;
						 curr->re->people   = 0;
						 curr->re->next     = 0;
						 break;
		case EDIT_ZONE : curr->zone = nr; 
						 curr->ze   = errMalloc( sizeof(zoneType) );
						 *curr->ze  = zones[nr];
						 curr->ze->rooms = 0;
						 break;
	}

	curr->next = editing;
	editing    = curr;

	ch->sub_1 = type;

	return curr;
}

int ed_is_modified( charType * ch, editUnitType * eu )
{
	objIndexType		*	oi, * ei;
	mobIndexType		*	mi, * me;
	roomType			*	ri, * re;
	zoneType			*	zi, * ze;

	switch( ch->sub_1 )
	{
		case	EDIT_OBJ :	

				ei = eu->oe; oi = &objects[ei->nr];

				if(    oi->type     != ei->type
					|| oi->name     != ei->name
					|| oi->wornd    != ei->wornd
					|| oi->roomd    != ei->roomd
					|| oi->extrd    != ei->extrd
					|| oi->usedd    != ei->usedd
					|| oi->wear     != ei->wear
					|| oi->cost     != ei->cost
					|| oi->level    != ei->level
					|| oi->extra    != ei->extra
					|| oi->magic    != ei->magic
					|| oi->weight   != ei->weight
					|| oi->value[0] != ei->value[0]
					|| oi->value[1] != ei->value[1]
					|| oi->value[2] != ei->value[2]
					|| oi->value[3] != ei->value[3]
					|| oi->apply[0].location != ei->apply[0].location
					|| oi->apply[0].modifier != ei->apply[0].modifier
					|| oi->apply[1].location != ei->apply[1].location
					|| oi->apply[1].modifier != ei->apply[1].modifier
				  ) return 1;
				break;

		case	EDIT_MOB  :	

				me = eu->me; mi = &mobiles[me->nr];

				if(    mi->sex			!= me->sex
					|| mi->class		!= me->class
					|| mi->name			!= me->name
					|| mi->roomd		!= me->roomd
					|| mi->moved		!= me->moved
					|| mi->description	!= me->description
					|| mi->gold			!= me->gold
					|| mi->exp			!= me->exp
					|| mi->align		!= me->align
					|| mi->level		!= me->level
					|| mi->ac			!= me->ac
					|| mi->hr			!= me->hr
					|| mi->dr			!= me->dr
					|| mi->ndice		!= me->ndice
					|| mi->sdice		!= me->sdice
					|| mi->hbase		!= me->hbase
					|| mi->hsdice		!= me->hsdice
					|| mi->hndice		!= me->hndice
					|| mi->position		!= me->position
					|| mi->defaultpos	!= me->defaultpos
					|| mi->act			!= me->act
					|| mi->affected		!= me->affected
				  ) return 1;
				break;

		case	EDIT_ROOM :

				re = eu->re; ri = &world[re->nr];

				if(	   ri->name			!= re->name
					|| ri->description	!= re->description
					|| ri->sector		!= re->sector
					|| ri->flags		!= re->flags
					|| ri->extrd		!= re->extrd
					|| ri->dirs[0]	    != re->dirs[0]
					|| ri->dirs[1]	    != re->dirs[1]
					|| ri->dirs[2]	    != re->dirs[2]
					|| ri->dirs[3]	    != re->dirs[3]
					|| ri->dirs[4]	    != re->dirs[4]
					|| ri->dirs[5]	    != re->dirs[5]
				  ) return 1;
				break;

		case	EDIT_ZONE :

				ze = eu->ze; zi = &zones[ze->nr];

				if(	   zi->name			!= ze->name
					|| zi->info			!= ze->info
					|| zi->path			!= ze->path
					|| zi->low			!= ze->low
					|| zi->high			!= ze->high
					|| zi->lifespan		!= ze->lifespan
					|| zi->reset_mode	!= ze->reset_mode
					|| zi->max			!= ze->max
					|| zi->align		!= ze->align
					|| zi->aggressive	!= ze->aggressive
				  ) return 1;
				break;
	}

	return 0;
}

void ed_save_edit( charType * ch, editUnitType * eu )
{
	exdescriptionType	*	ex, * next_ex;
	objIndexType		*	oi, * ei;
	mobIndexType		*	mi, * me;
	roomType			*	ri, * re;
	zoneType			*	zi, * ze;
	int						i;

	if( eu->oe ) 
	{
		ei = eu->oe; oi = &objects[ei->nr];

		if( oi->name  != ei->name  ) errFree( oi->name  );
		if( oi->wornd != ei->wornd ) errFree( oi->wornd );
		if( oi->roomd != ei->roomd ) errFree( oi->roomd );
		if( oi->extrd != ei->extrd ) 
		{
			for( ex = oi->extrd; ex; ex = next_ex )
			{
				next_ex = ex->next;	errFree( ex );
			}
		}
		if( oi->usedd != oi->usedd ) errFree( oi->usedd );

		*oi = *ei;
	}
	else if( eu->me ) 
	{
		me = eu->me; mi = &mobiles[me->nr];

		if( mi->description != me->description ) errFree( mi->description );
		if( mi->name        != me->name )		 errFree( mi->name );
		if( mi->moved       != me->moved )		 errFree( mi->moved );
		if( mi->roomd       != me->roomd )		 errFree( mi->description );

		*mi = *me;
	}
	else if( eu->re ) 
	{
		re = eu->re; ri = &world[re->nr];

		if( ri->description != re->description ) 
		{
			errFree( ri->description );	ri->description = re->description;
		}

		if( ri->name != re->name )
		{		 
			errFree( ri->name ); ri->name = re->name;
		}

		if( ri->extrd != re->extrd )
		{
			for( ex = ri->extrd; ex; ex = next_ex )
			{
				next_ex = ex->next;	errFree( ex );
			}

			ri->extrd = re->extrd;
		}

		for( i = 0; i < MAX_DIRECTIONS; i++ ) 
			if( re->dirs[i] != ri->dirs[i] ) 
			{
				errFree( ri->dirs[i] );
				ri->dirs[i] = re->dirs[i];
			}

		ri->sector		= re->sector;
		ri->flags		= re->flags;
	}
	else if( eu->ze )
	{
		ze = eu->ze; zi = &zones[ze->nr];

		if( ze->name != zi->name ) errFree( zi->name );
		if( ze->info != zi->info ) errFree( zi->info );
		if( ze->path != zi->path ) errFree( zi->path );

		zi->name		= ze->name;
		zi->info		= ze->info;
		zi->path		= ze->path;
		zi->lifespan    = ze->lifespan;
		zi->reset_mode  = ze->reset_mode;
		zi->low			= ze->low;
		zi->high		= ze->high;
		zi->max			= ze->max;
		zi->align		= ze->align;
		zi->aggressive	= ze->aggressive;
	}

	sendf( ch, "Saving----------" );
}

void ed_get_yesno( charType * ch, char * argu )
{
	if( *argu )
	{
		switch( *argu )
		{
			case	'y' : case 'Y'	: ch->sub_3 = EDIT_YES; break;
			case	'n' : case 'N'	: ch->sub_3 = EDIT_NO;  break;
		}
	}
}

void del_editing( editUnitType * del )
{
	editUnitType	*	curr;

	if( editing == del )
	{
		editing = del->next;
	}
	else
	{
		for( curr = editing; curr && (curr->next != del); curr = curr->next )
		;

		if( !curr )
		{
			DEBUG( "del_editing> requested edit_unit not found in list." );
			return;
		}

		curr->next = curr->next->next;
	}

	if( del->oe ) errFree( del->oe );
	if( del->re ) errFree( del->re );
	if( del->me ) errFree( del->me );
	if( del->de ) errFree( del->de );
	if( del->xe ) errFree( del->xe );

	errFree( del->who );
	errFree( del );
}

void do_edit( charType * ch, char * argu, int cmd )
{
	if( IS_NPC(ch) || ch->level < IMO+2 ) return;

	ch->mode  = EDIT_MODE;
}

void edit_interpreter( charType * ch, char * argument )
{
	int			i;
	char		cmd[MAX_INPUT_LENGTH+1];
	char	*	argu;

	if( ch->mode != EDIT_MODE )
	{
		DEBUG( "edit_interpreter> %s's mode is not EDIT_MODE.", ch->name );
		return;
	}

	if( ch->sub_3 )
	{
		switch( ch->sub_3 )
		{
			case	EDIT_YESNO : ed_get_yesno( ch, argument ); break;
		}
	}

	if( ch->sub_1 == 0 )
	{
		argu = onefword( argument, cmd );	

		if( !cmd[0] ) return;
	
		for( i = 0; edit_cmds[i].name[0] != '\n'; i++ )
		{
			if( isoneofp( cmd, edit_cmds[i].name ) )
			{
				(*edit_cmds[i].func)( ch, argu, edit_cmds[i].cmd );	return;
			}
		}
		interpreter( ch, argument, 0 );
	}
	else
	{
		switch( ch->sub_1 )
		{
			case EDIT_OBJ  :	do_iedit( ch, argument, 0 ); break;
			case EDIT_MOB  :	do_medit( ch, argument, 0 ); break;
			case EDIT_ROOM :	do_redit( ch, argument, 0 ); break;
			case EDIT_ZONE :	do_zedit( ch, argument, 0 ); break;
		}
	}
}
