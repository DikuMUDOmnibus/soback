#include <string.h>
#include <ctype.h>

#include "character.h"
#include "world.h"
#include "mobile.h"
#include "object.h"
#include "zone.h"
#include "interpreter.h"
#include "comm.h"
#include "strings.h"
#include "utils.h"
#include "variables.h"
#include "edit.h"
#include "affect.h"
#include "magics.h"
#include "allocate.h"

#define	MEDIT_NONE			0
#define	MEDIT_NAME			1 
#define	MEDIT_MOVE			2
#define	MEDIT_ROOM			3
#define	MEDIT_DESC			5

#define	MEDIT_SEX			6
#define	MEDIT_CLASS			7
#define	MEDIT_ACT			8
#define	MEDIT_AFFECT		9

#define	MEDIT_EXP			10
#define	MEDIT_ALIGN			11
#define	MEDIT_LEVEL			12
#define MEDIT_GOLD			13

#define	MEDIT_HBASE			15
#define	MEDIT_HSDICE		16
#define	MEDIT_HNDICE		17

#define MEDIT_HR			18
#define MEDIT_DR			19
#define MEDIT_NDICE			20
#define MEDIT_SDICE			21

#define MEDIT_POS			25
#define MEDIT_DPOS			26

#define MEDIT_AC			30

#define MEDIT_QUIT			35

static void me_effect( charType * ch, editUnitType * eu )
{
	charType		*	mob;
	mobIndexType	*	mi = eu->me;
	int					j;

	if( !ed_is_modified( ch, eu ) ) return;

	ed_save_edit( ch, eu );

	for( j = 0, mob = mob_list; mob; mob = mob->next )
	{
		if( mob->nr == mi->nr )
		{
			mob->name		  	= mi->name;
			mob->moved			= mi->moved;
			mob->roomd			= mi->roomd;
			mob->description  	= mi->description;

			mob->sex 			= mi->sex;
			mob->class 			= mi->class;
			mob->level			= mi->level;

			mob->align  		= mi->align;
			mob->exp			= mi->exp;
			mob->gold			= mi->gold;

			mob->armor			= mi->ac;
			mob->hr				= mi->hr;
			mob->dr				= mi->dr;
			mob->ndice			= mi->ndice;
			mob->sdice  		= mi->sdice;

			mob->hit			= mi->hbase + dice( mi->hsdice, mi->hndice );

			mob->act	  		= mi->act;
			mob->affects 		= mi->affected;

			j++;
		}
	}

	sendf( ch, "Total %d mobile.", j );	
}

static void me_print_mob( charType * ch, editUnitType * eu )
{
	char				buf1[100], buf2[100];
	mobIndexType	*	mi;

	mi = eu->me;

	sendf( ch, "---------------- index %4d - virtual %5d.", mi->nr, mi->virtual );
	sendf( ch, "" );
	sendf( ch, " 1]       Name : %s", mi->name  ? mi->name  : "None" );
	sendf( ch, " 2]  When room : %s", mi->roomd ? mi->roomd : "None" );
	sendf( ch, " 3]  When move : %s", mi->moved ? mi->moved : "None" );
	sendf( ch, " 4]Description : %s", mi->description ? mi->description : "None" );	
	sendf( ch, " 5]        Sex : %s", sprinttype( mi->sex, sex_types, 0 ) );
	sendf( ch, " 6]      Class : %s", sprinttype( mi->class,  npc_class_types, 0 ) );
	sendf( ch, " 7]      Level : %d", mi->level );
	sendf( ch, " 8]     Action : %s", sprintbit( mi->act, action_bits, 0 ) );
	sendf( ch, " 9]   Affected : %s", sprintbit( mi->affected, affected_bits,  0 ) );
	sendf( ch, "10]      Align : %d", mi->align );
	sendf( ch, "11]        Exp : %d", mi->exp );
	sendf( ch, "12]       Gold : %d", mi->gold );
	sendf( ch, "13]        Hit : %d+%dD%d", mi->hbase, mi->hsdice, mi->hndice );
	sendf( ch, "14]         AC : %d", mi->ac );
	sendf( ch, "15]    HitRoll : %d", mi->hr );
	sendf( ch, "16]    DamRoll : %d+%dD%d", mi->dr, mi->sdice, mi->ndice );
	sendf( ch, "17]   Position : %s - default - %s", 
							sprinttype( mi->position,   position_types, buf1 ),
							sprinttype( mi->defaultpos, position_types, buf2 ) );
	sendf( ch, "" );
	sendf( ch, "---------------- S), Q), X), N), P), J), K), E), G) #, H) help" );

	ed_new_prompt( eu, "Medit> " );

	ch->sub_1 = EDIT_MOB;
	ch->sub_2 = MEDIT_NONE;
}

static void me_print_name( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[100];
	char				tmp[MAX_STRING_LENGTH+1];

	strcpy( tmp, mi->name );

	sendf( ch, "Medit > %s", remove_newline( tmp ) );

	sprintf( buf, "Medit %s - name > ", mi->moved );	

	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_NAME;
}

static void me_get_name( charType * ch, editUnitType * eu, char * argu )
{
	mobIndexType	* 	mi = eu->me;

	if( argu = skipsps( argu ), *argu )
	{
		mi->name = strdup( argu );		
	}

	me_print_mob( ch, eu );
}

static void me_print_move( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[100];
	char				tmp[MAX_STRING_LENGTH+1];

	strcpy( tmp, mi->moved ? mi->moved : "None" );

	sendf( ch, "Medit > %s", remove_newline( tmp ) );

	sprintf( buf, "Medit %s - move > ", mi->moved );	

	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_MOVE;
}

static void me_get_move( charType * ch, editUnitType * eu, char * argu )
{
	mobIndexType	* 	mi = eu->me;

	if( argu = skipsps( argu ), *argu )
	{
		mi->moved = strdup( argu );
	}

	me_print_mob( ch, eu );
}

static void me_print_room( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[100];
	char				tmp[MAX_STRING_LENGTH+1];

	strcpy( tmp, mi->roomd ? mi->roomd : "None" );

	sendf( ch, "Medit > %s", remove_newline( tmp ) );

	sprintf( buf, "Medit %s - room > ", mi->moved );	

	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_ROOM;
}

static void me_get_room( charType * ch, editUnitType * eu, char * argu )
{
	mobIndexType	* 	mi = eu->me;

	if( argu = skipsps( argu ), *argu )
	{
		mi->roomd = strdup( argu );
	}

	me_print_mob( ch, eu );
}

static void me_print_desc( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[100];
	char				tmp[MAX_STRING_LENGTH+1];

	strcpy( tmp, mi->description ? mi->description : "None" );

	sendf( ch, "Medit > %s", remove_newline( tmp ) );

	sprintf( buf, "Medit %s - description > ", mi->moved );

	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_DESC;
}

static void me_get_desc( charType * ch, editUnitType * eu, char * argu )
{
	mobIndexType	* 	mi = eu->me;

	if( argu = skipsps( argu ), *argu )
	{
		mi->description = strdup( argu );
	}

	me_print_mob( ch, eu );
}

static void me_print_sex( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	int					i, toggle = 0;
	char				buf[200];

	buf[0] = '\0';

	for( i = 0, toggle = 0; sex_types[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i+1, sex_types[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Medit %s - sex %s > ", mi->moved, sprinttype( mi->sex, sex_types, 0));
	
	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_SEX;
}

static void me_get_sex( charType * ch, editUnitType * eu, char * argu )
{
	int			nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || --nr > SEX_LAST )
		{
			sendf( ch, "Invalid item type." ); return;
		}

		eu->me->sex = nr;
	}

	me_print_mob( ch, eu );
}

static void me_print_class( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	int					i, toggle = 0;
	char				buf[200];

	buf[0] = '\0';

	for( i = 0, toggle = 0; sex_types[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i, npc_class_types[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Medit %s - class %s > ", mi->moved, sprinttype( mi->sex, npc_class_types, 0));
	
	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_CLASS;
}

static void me_get_class( charType * ch, editUnitType * eu, char * argu )
{
	int			nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > NPC_CLASS_LAST )
		{
			sendf( ch, "Invalid item type." ); return;
		}

		eu->me->class = nr;
	}

	me_print_mob( ch, eu );
}

static void me_print_action( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	int					i, toggle = 0;
	char				buf[300];

	sprintf( buf, "%2d. %-20s", 0, "Clear" );

	for( i = 0, toggle = 1; action_bits[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i + 1, action_bits[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Medit action > %s\n\rMedit action > ", sprintbit( mi->act, action_bits, 0 ) );
	
	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_ACT;
}

static void me_get_action( charType * ch, editUnitType * eu, char * argu )
{
	mobIndexType	*	mi = eu->me;
	int					nr = 0;
	char				buf[300];

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > ACT_LAST )
		{
			sendf( ch, "Invalid act flag." ); return;
		}
	}

	if( *argu && nr == 0 )
	{
		mi->act = 0;
		sprintf( buf, "Medit action %s\n\rMedit action > ", sprintbit( mi->act, action_bits, 0 ) );
		ed_new_prompt( eu, buf );
	}
	else if( nr )
	{
		nr--;
		if( mi->act & ( 1 << nr ) ) mi->act &= ~(1 << nr);
		else 						mi->act |=   1 << nr ;

		sprintf( buf, "Medit action %s\n\rMedit action > ", sprintbit( mi->act, action_bits, 0 ) );
		ed_new_prompt( eu, buf );
	}
	else
	{
		me_print_mob( ch, eu );
	}
}

static void me_print_affected( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	int					i, toggle = 0;
	char				buf[200];

	sprintf( buf, "%2d. %-20s", 0, "Clear" );

	for( i = 0, toggle = 1; affected_bits[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i + 1, affected_bits[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Medit affected %s\n\rMedit affected > ", 
									sprintbit( mi->affected, affected_bits, 0 ) );
	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_AFFECT;
}

static void me_get_affected( charType * ch, editUnitType * eu, char * argu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[300];
	int					nr = 0;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > AFF_LAST )
		{
			sendf( ch, "Invalid affected flag." ); return;
		}
	}

	if( *argu && nr == 0 )
	{
		mi->affected = 0;
		sprintf( buf, "Medit affected %s\n\rMedit affected > ", 
									sprintbit( mi->affected, affected_bits, 0 ) );
		ed_new_prompt( eu, buf );
	}
	else if( nr )
	{
		nr--;
		if( mi->affected & ( 1 << nr ) ) mi->affected &= ~(1 << nr);
		else 		    				 mi->affected |=   1 << nr ;

		sprintf( buf, "Medit affected %s\n\rMedit affected > ", 
									sprintbit( mi->affected, affected_bits, 0 ) );
		ed_new_prompt( eu, buf );
	}
	else
	{
		me_print_mob( ch, eu );
	}
}

static void me_print_gold( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[300];

	sprintf( buf, "Medit %s - gold %s > ", mi->moved, numfstr( mi->gold ) );

	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_GOLD;
}

static void me_get_gold( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid cost." ); return;
		}
		eu->me->gold = nr;
	}

	me_print_mob( ch, eu );
}

static void me_print_align( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[300];

	sprintf( buf, "Medit %s - align %d ==> ", mi->moved, mi->align );

	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_ALIGN;
}

static void me_get_align( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) == 0 || nr > 1000 || nr < -999 )
		{
			sendf( ch, "Invalid align." ); return;
		}

		eu->me->align = nr;
	}

	me_print_mob( ch, eu );
}

static void me_print_ac( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[300];

	sprintf( buf, "Medit %s - AC %d > ", mi->moved, mi->ac );

	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_AC;
}

static void me_get_ac( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) == 0 )
		{
			sendf( ch, "Invalid AC." ); return;
		}
		eu->me->ac = nr;
	}

	me_print_mob( ch, eu );
}

static void me_print_level( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[300];

	sprintf( buf, "Medit %s - level %d > ", mi->moved, mi->level );

	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_LEVEL;
}

static void me_get_level( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid level." ); return;
		}
		eu->me->level = nr;
	}

	me_print_mob( ch, eu );
}


static void me_print_exp( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[300];

	sprintf( buf, "Medit %s - exp %ld > ", mi->moved, mi->exp );

	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_EXP;
}

static void me_get_exp( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid exp." ); return;
		}
		eu->me->exp = nr;
	}

	me_print_mob( ch, eu );
}


static void me_print_hr( charType * ch, editUnitType * eu )
{
	mobIndexType	*	mi = eu->me;
	char				buf[300];

	sprintf( buf, "Medit %s - HR %d > ", mi->moved, mi->hr );

	ed_new_prompt( eu, buf );

	ch->sub_2 = MEDIT_HR;
}

static void me_get_hr( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid HR." ); return;
		}
		eu->me->hr = nr;
	}

	me_print_mob( ch, eu );
}

static void me_print_position( charType * ch, editUnitType * eu )
{
	mobIndexType		*	mi = eu->me;
	char					buf[300];
	int						i, toggle;

	buf[0] = '\0';

	for( i = 0, toggle = 0; position_types[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i, position_types[i] );

		if( ++toggle >= 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0, buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	if( ch->sub_2 == MEDIT_NONE )
	{
		sprintf( buf, "Medit %s - pos %s > ", 
									mi->moved, sprinttype( mi->position, position_types, 0 ) );

		ch->sub_2 = MEDIT_POS;
	}
	else
	{
		sprintf( buf, "Medit %s - default pos %s > ", 
									mi->moved, sprinttype( mi->defaultpos, position_types, 0 ) );

		ch->sub_2 = MEDIT_DPOS;
	}

	ed_new_prompt( eu, buf );
}

static void me_get_position( charType * ch, editUnitType * eu, char * argu )
{
	mobIndexType		*	mi = eu->me;
	int						nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > POSITION_LAST )
		{
			sendf( ch, "Invalid position type." ); return;
		}

		if( ch->sub_2 == MEDIT_POS ) mi->position   = nr;
		else							   mi->defaultpos = nr;
	}

	if( ch->sub_2 == MEDIT_POS ) me_print_position( ch, eu );
	else 		                       me_print_mob( ch, eu );
}

static void me_print_hit( charType * ch, editUnitType * eu )
{
	mobIndexType		*	mi = eu->me;
	char					buf[300];
	char				* 	prompt = "";

	switch( ch->sub_2 )
	{
		case	MEDIT_NONE	 : 	

				ch->sub_2 = MEDIT_HBASE; 	prompt = "Hit";
				break;

		case	MEDIT_HBASE	 : 	

				ch->sub_2 = MEDIT_HSDICE; prompt = "Hit sdice";
				break;

		case	MEDIT_HSDICE : 	

				ch->sub_2 = MEDIT_HNDICE; prompt = "Hit ndice";
				break;
	}

	sprintf( buf, "Medit %s - %d+%dD%d %s > ", 
						mi->moved, mi->hbase, mi->hsdice, mi->hndice, prompt );

	ed_new_prompt( eu, buf );
}

static void me_get_hit( charType * ch, editUnitType * eu, char * argu )
{
	mobIndexType		*	mi = eu->me;
	int						nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid hit or dice type." ); return;
		}

		switch( ch->sub_2 )
		{
		case	MEDIT_HBASE  :	mi->hbase  = nr; break;
		case	MEDIT_HSDICE :	mi->hsdice = nr; break;
		case	MEDIT_HNDICE :	mi->hndice = nr; break;
		}
	}

	if( ch->sub_2 != MEDIT_HNDICE )   me_print_hit( ch, eu );
	else									me_print_mob( ch, eu );
}

static void me_print_dr( charType * ch, editUnitType * eu )
{
	mobIndexType		*	mi = eu->me;
	char					buf[300];
	char				* 	prompt = "";

	switch( ch->sub_2 )
	{
		case	MEDIT_NONE	 : 	

				ch->sub_2 = MEDIT_DR; 	prompt = "DR";
				break;

		case	MEDIT_DR	 : 	

				ch->sub_2 = MEDIT_SDICE;  prompt = "Hand sdice";
				break;

		case	MEDIT_SDICE : 	

				ch->sub_2 = MEDIT_NDICE;  prompt = "Hand ndice";
				break;
	}

	sprintf( buf, "Medit %s - %d+%dD%d %s > ", 
						mi->moved, mi->dr, mi->sdice, mi->ndice, prompt );

	ed_new_prompt( eu, buf );
}

static void me_get_dr( charType * ch, editUnitType * eu, char * argu )
{
	mobIndexType		*	mi = eu->me;
	int						nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid dr or dice type." ); return;
		}

		switch( ch->sub_2 )
		{
		case	MEDIT_DR    :	mi->dr    = nr; break;
		case	MEDIT_SDICE :	mi->sdice = nr; break;
		case	MEDIT_NDICE :	mi->ndice = nr; break;
		}
	}

	if( ch->sub_2 != MEDIT_NDICE )   me_print_dr ( ch, eu );
	else								   me_print_mob( ch, eu );
}

static void me_quit_edit( charType * ch, editUnitType * eu )
{
	if( ch->sub_3 != EDIT_NONE )
	{
		switch( ch->sub_3 )
		{
			case	EDIT_YESNO  : break;
			case	EDIT_YES	: me_effect( ch, eu ); 
								  del_editing( eu );
								  ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
								  break;
			case    EDIT_NO     : del_editing( eu );
								  ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
								  break;
		}
	}
	else if( ed_is_modified( ch, eu ) )
	{
		sendf( ch, "'%s' modified. Apply this to the table ? ", eu->me->moved );

		ch->sub_2 = MEDIT_QUIT;
		ch->sub_3 = EDIT_YESNO;
	}
	else
	{
	  	del_editing( eu );
	  	ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
	}
}

static void me_next_edit( charType * ch, editUnitType * eu, int mode, int prev )
{
	editUnitType	*	edit;
	int					nr;

	nr = eu->me->nr;

	if( prev )
	{
		if( --nr <= MOBILE_INTERNAL )
		{
			sendf( ch, "No previous mobile in the mobile table." ); return;
		}
	}
	else
	{
		if( ++nr >= mob_index_info.used )
		{
			sendf( ch, "No more mobile in the mobile table." ); return;
		}
	}

	if( mode && ed_is_modified( ch, eu ) )
	{
		me_effect( ch, eu );
	}

	del_editing( eu );
  	ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;

	edit = new_editing( ch, EDIT_MOB, nr );
	me_print_mob( ch, edit );
}

static void me_change_edit( charType * ch, editUnitType * eu, char * argu )
{       
    editUnitType    *   edit;
    int                 nr;
    char                buf1[MAX_INPUT_LENGTH+1];
    char                buf2[MAX_INPUT_LENGTH+1];
 
    twoArgument( argu, buf1, buf2 );
    
    if( !isnumstr( buf2 ) )
    {
        sendf( ch, "Need number of mobile to change editing mobile." );
        return;
    }
        
    if( getnumber( buf2, &nr ) <= 0 )
    {       
        sendf( ch, "Invalid mobile number." ); return;
    }

    if( nr = real_mobileNr( nr ), nr == NOWHERE )
    {   
        sendf( ch, "No mobile by that number." ); return;
    }
    
    if( ed_is_modified( ch, eu ) )
    {   
        ed_save_edit( ch, eu );
    }
    
    del_editing( eu );
    ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
    
    edit = new_editing( ch, EDIT_MOB, nr );
    me_print_mob( ch, edit );
}

static void me_help( charType * ch )
{
	sendf( ch, "Medit help ----------" );
	sendf( ch, "# > to edit specified field." );
	sendf( ch, "S > apply edited mobile to the mobile table." );
	sendf( ch, "Q > quit."			  		  );
	sendf( ch, "X > quit without saving."     );
	sendf( ch, "N > edit next mobile in the mobile table." );
	sendf( ch, "P > edit previous mobile in the mobile table." );
	sendf( ch, "J > edit next mobile without saving." );
	sendf( ch, "K > edit previous mobile without saving." );
	sendf( ch, "G # > change editing mobile to new." );
	sendf( ch, "H > show this page." );
}

void do_medit( charType * ch, char * argu, int cmd )
{
	editUnitType	*	edit;
	int					nr;
	int					ret;

	if( (ch->mode && ch->mode != EDIT_MODE) || (ch->sub_1 && ch->sub_1 != EDIT_MOB) )
	{
		sendf( ch, "You can not do that." ); return;
	}

	ch->mode = EDIT_MODE;

	if( ch->sub_1 == EDIT_NONE )
	{
		if( !*argu )
		{
			sendf( ch, "iedit # ( virtual number )" ); return;
		}

		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid number." ); return;
		}

		if( nr = real_mobileNr( nr ), nr <= MOBILE_INTERNAL )
		{
			sendf( ch, "That mobile is not found." ); return;
		}	

		if( ret = find_editing( ch, EDIT_MOB, nr ), ret > 0 )
		{	
			if( ret == 1 )
			{
				sendf( ch, "That mobile is not available now." ); return;
			}

			if( ret == 2 )
			{
				sendf( ch, "Oops.. You are already in editing list. Retry" ); 
				edit = find_editunit( ch );
				del_editing( edit );
				return;
			}
		}

		edit = new_editing( ch, EDIT_MOB, nr );
		me_print_mob( ch, edit );

		return;
	}

	if( edit = find_editunit( ch ), !edit )
	{
		sendf( ch, "Oops. Your edit record not found!." );
		ch->sub_3 = ch->sub_2 = ch->sub_1 = EDIT_NONE;
		ch->mode  = 0;
		return;
	}

	if( ch->sub_2 != EDIT_NONE )
	{
		argu = skipsps( argu );

		switch( ch->sub_2 )
		{
		case MEDIT_NAME			:	me_get_name( ch, edit, argu ); 			break;
		case MEDIT_MOVE 		:	me_get_move( ch, edit, argu ); 			break;
		case MEDIT_ROOM			: 	me_get_room( ch, edit, argu ); 			break;
		case MEDIT_DESC			:	me_get_desc( ch, edit, argu ); 			break;
		case MEDIT_CLASS		:	me_get_class( ch, edit, argu );			break;
		case MEDIT_ACT			:	me_get_action( ch, edit, argu );		break;
		case MEDIT_AFFECT		:	me_get_affected( ch, edit, argu );		break;
		case MEDIT_EXP			:	me_get_exp( ch, edit, argu );			break;
		case MEDIT_GOLD			:	me_get_gold( ch, edit, argu );			break;
		case MEDIT_ALIGN		:	me_get_align( ch, edit, argu );			break;
		case MEDIT_LEVEL		:	me_get_level( ch, edit, argu );			break;
		case MEDIT_SEX			:	me_get_sex( ch, edit, argu );			break;
		case MEDIT_AC			:	me_get_ac( ch, edit, argu );			break;
		case MEDIT_NDICE		:
		case MEDIT_SDICE		:
		case MEDIT_DR			:	me_get_dr( ch, edit, argu );			break;
		case MEDIT_HR			:	me_get_hr( ch, edit, argu );			break;
		case MEDIT_HBASE		:
		case MEDIT_HSDICE		:
		case MEDIT_HNDICE		:   me_get_hit( ch, edit, argu );			break;
		case MEDIT_POS			:
		case MEDIT_DPOS			:	me_get_position( ch, edit, argu );		break;

		case MEDIT_QUIT			:   me_quit_edit( ch, edit );       	    break;
		}
		return;
	}

	if( getnumber( argu, &nr ) > 0 )
	{
		switch( nr )
		{
		case	 1  :	me_print_name  		( ch, edit ); 			break;
		case	 2  :	me_print_room  		( ch, edit ); 			break;
		case	 3  :	me_print_move  		( ch, edit ); 			break;
		case	 4  :	me_print_desc  		( ch, edit ); 			break;
		case	 5 	:	me_print_sex  		( ch, edit ); 			break;
		case	 6  :	me_print_class  	( ch, edit ); 			break;
		case	 7  :	me_print_level  	( ch, edit ); 			break;
		case	 8  :	me_print_action		( ch, edit ); 			break;
		case	 9  :	me_print_affected	( ch, edit ); 			break;
		case	10  :	me_print_align  	( ch, edit ); 			break;
		case	11  :	me_print_exp   		( ch, edit ); 			break;
		case	12  :	me_print_gold 		( ch, edit ); 			break;
		case	13  :	me_print_hit 		( ch, edit ); 			break;
		case	14  :	me_print_ac 		( ch, edit ); 			break;
		case	15  :	me_print_hr 		( ch, edit ); 			break;
		case	16  :	me_print_dr 		( ch, edit ); 			break;
		case	17  :	me_print_position	( ch, edit ); 			break;
		default		:	sendf( ch, "Huh? type 'h' for help." ); 	break;
		}
	}
	else
	{
		if( !*argu ) 
		{
			me_print_mob( ch, edit ); return;
		}
		else if( strlen( argu ) >= 1 )
		{
			switch( *argu )
			{
			case	'S' : 	me_effect( ch, edit );  					return;	 
			case	'Q' :	me_quit_edit( ch, edit ); 	 				return;
			case 	'X'	:  	del_editing( edit ); 
				ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
																		return;
			case	'N' :   me_next_edit( ch, edit, 1, 0 );  			return;
			case	'P' :   me_next_edit( ch, edit, 1, 1 );  			return;
			case	'J' :   me_next_edit( ch, edit, 0, 0 );  			return;
			case	'K' :   me_next_edit( ch, edit, 0, 1 );  			return;
			case	'E' :	me_effect	( ch, edit );					return;
			case	'H' :	me_help( ch );								return;
			case	'G' :	me_change_edit( ch, edit, argu );			return;
			}
		}
		interpreter( ch, argu, 0 );
	}
}

void ed_mcopy( charType * ch, char * argu, int cmd )
{
}
