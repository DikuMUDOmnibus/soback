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
#include "magics.h"
#include "weapon.h"
#include "allocate.h"

#define	IEDIT_NONE			0
#define	IEDIT_NAME			1 
#define	IEDIT_WORN			2
#define	IEDIT_ROOM			3
#define	IEDIT_USED			4
#define	IEDIT_EXTR			5
#define	IEDIT_TYPE			6
#define	IEDIT_FLAG			7
#define	IEDIT_WEAR			8
#define	IEDIT_WEIGHT		9
#define	IEDIT_COST			10
#define	IEDIT_GPD			11
#define	IEDIT_LEVEL			12
#define	IEDIT_VALUE			13
#define	IEDIT_APPLY			17
#define	IEDIT_SAVE			21

#define IEDIT_WEAP_M		25
#define IEDIT_WEAP_S  		26
#define IEDIT_WEAP_N  		27
#define IEDIT_WEAP_T  		28

#define IEDIT_WAND_L		30
#define IEDIT_WAND_M		31
#define IEDIT_WAND_C		32

#define IEDIT_SPEL_L		35
#define IEDIT_SPEL_1		36
#define IEDIT_SPEL_2		37
#define IEDIT_SPEL_3		38

#define IEDIT_AC			40

#define IEDIT_LIGHT			45
#define IEDIT_TRAP			46

#define IEDIT_FIRE_M		50
#define IEDIT_FIRE_C		51
#define IEDIT_FIRE_D		52

#define IEDIT_FOOD_P		55

#define IEDIT_CONT_S		60
#define IEDIT_CONT_L		61

#define IEDIT_DCON_C		65
#define IEDIT_DCON_T		66
#define IEDIT_DCON_P		67

#define IEDIT_APPL_1		70
#define IEDIT_APPM_1		71
#define IEDIT_APPL_2		72
#define IEDIT_APPM_2		73

#define IEDIT_QUIT			75

static void ie_effect( charType * ch, editUnitType * eu )
{
	objectType		*	obj;
	objIndexType	*	oi = eu->oe;
	char			* 	usedd = 0;
	int					i, j;

	if( !ed_is_modified( ch, eu ) ) return;

	ed_save_edit( ch, eu );

	for( j = 0, obj = obj_list; obj; obj = obj->next )
	{
		if( obj->nr == oi->nr )
		{
			if( obj->type == ITEM_NOTE ) usedd = obj->usedd;
			else						 usedd = 0;

			obj->name	= oi->name;
			obj->wornd	= oi->wornd;
			obj->roomd	= oi->roomd;
			obj->usedd  = oi->usedd;
			obj->extrd  = oi->extrd;

			obj->type 	= oi->type;
			obj->wear	= oi->wear;
			obj->extra  = oi->extra;
			obj->cost	= oi->cost;
			obj->magic	= oi->magic;
			obj->weight = oi->weight;
			obj->level	= oi->level;

			for( i = 0; i < 4; i++ ) obj->value[i] = oi->value[i];
			for( i = 0; i < 2; i++ ) 
				if( oi->apply[i].location != APPLY_NONE )
					obj->apply[i] = oi->apply[i];

			if( usedd ) obj->usedd = usedd;	

			j++;
		}
	}

	sendf( ch, "Total %d objects.", j );	
}

static void ie_print_obj( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi;
	char				app1[100], app2[100], app3[100], app4[100];

	oi = eu->oe;

	sendf( ch, "---------------- index %4d - virtual %5d.", oi->nr, oi->virtual );
	sendf( ch, "" );
	sendf( ch, " 1]       Name : %s", oi->name  ? oi->name  : "None" );
	sendf( ch, " 2]  When room : %s", oi->roomd ? oi->roomd : "None" );
	sendf( ch, " 3]  When worn : %s", oi->wornd ? oi->wornd : "None" );
	if( oi->type == ITEM_WEAPON )
	sendf( ch, " 4]  When used : %s", oi->usedd ? oi->usedd : "None" );
	else
	sendf( ch, " 4]  When used : %s", oi->usedd ? "Exist" : "None" );	
	sendf( ch, " 5] Extra desc : %s", oi->extrd ? oi->extrd->keyword : "None" );
	sendf( ch, " 6]  Item type : %s", sprinttype( oi->type,  item_types, 0 ) );
	sendf( ch, " 7] Extra flag : %s", sprintbit( oi->extra, extra_bits, 0 ) );
	sendf( ch, " 8]  Wear flag : %s", sprintbit( oi->wear,  wear_bits,  0 ) );
	sendf( ch, " 9]     Weight : %d", oi->weight );
	sendf( ch, "10]       Cost : %d", oi->cost );
	sendf( ch, "11] General pd : %d", oi->magic );
	sendf( ch, "12]      Level : %d", oi->level );
	sendf( ch, "13]     Values : [%d] [%d] [%d] [%d]",
					oi->value[0], oi->value[1], oi->value[2], oi->value[3] );

	sprinttype( oi->apply[0].location, apply_types, app1 );
	sprinttype( oi->apply[1].location, apply_types, app2 );
	sprinttype( oi->apply[2].location, apply_types, app3 );
	sprinttype( oi->apply[3].location, apply_types, app4 );

	sendf( ch, "14]      Apply : [%s - %d] [%s - %d] [%s - %d] [%s - %d]",
					app1, oi->apply[0].modifier, app2, oi->apply[1].modifier,
					app3, oi->apply[2].modifier, app4, oi->apply[3].modifier );
	sendf( ch, "" );
	sendf( ch, "---------------- S), Q), X), N), P), J), K), G) #, H) help" );

	ed_new_prompt( eu, "Iedit> " );

	ch->sub_2 = IEDIT_NONE;
	ch->sub_1 = EDIT_OBJ;
}

static void ie_spell_prompt( editUnitType * eu, int nr, int spNr )
{
	char				buf[200];
	int					valid = 0;
	char		*		spellName;

	if( spNr < -1 || spNr > MAX_USED_SPELLS )
	{
		spellName = "Invalid spell number";
	}
	else
	{
		if( spNr > 0 )
		{
			switch( eu->oe->type )
			{
				case	ITEM_POTION :	if( !(spells[spNr].type & SPELL_POTION) );
										else	valid = 1;
										break;
				case	ITEM_WAND 	:	if( !(spells[spNr].type & SPELL_WAND) );
										else	valid = 1;
										break;
				case	ITEM_STAFF  :	if( !(spells[spNr].type & SPELL_STAFF) );
										else	valid = 1;
										break;
				case	ITEM_SCROLL :	if( !(spells[spNr].type & SPELL_SCROLL) );
										else	valid = 1;
										break;
			}

			if( !valid ) spellName = "Invalid spell type";
			else		 spellName = spells[spNr].name;
		}
		else			 spellName = "None";
	}

	sprintf( buf, "Iedit %s - spell %d %s > ", eu->oe->wornd, nr, spellName );

	ed_new_prompt( eu, buf );
}

static void ie_print_name( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[100];
	char				tmp[MAX_STRING_LENGTH+1];

	strcpy( tmp, oi->name );

	sendf( ch, "Iedit > %s", remove_newline( tmp ) );

	sprintf( buf, "Iedit %s - name > ", oi->wornd );	

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_NAME;
}

static void ie_get_name( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	* 	oi = eu->oe;

	if( argu = skipsps( argu ), *argu )
	{
		oi->name = strdup( argu );		
	}

	ie_print_obj( ch, eu );
}

static void ie_print_worn( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[100];
	char				tmp[MAX_STRING_LENGTH+1];

	strcpy( tmp, oi->wornd ? oi->wornd : "None" );

	sendf( ch, "Iedit > %s", remove_newline( tmp ) );

	sprintf( buf, "Iedit %s - worn > ", oi->wornd );	

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_WORN;
}

static void ie_get_worn( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	* 	oi = eu->oe;

	if( argu = skipsps( argu ), *argu )
	{
		oi->wornd = strdup( argu );		
	}

	ie_print_obj( ch, eu );
}

static void ie_print_room( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[100];
	char				tmp[MAX_STRING_LENGTH+1];

	strcpy( tmp, oi->roomd ? oi->roomd : "None" );

	sendf( ch, "Iedit > %s", remove_newline( tmp ) );

	sprintf( buf, "Iedit %s - room > ", oi->wornd );	

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_ROOM;
}

static void ie_get_room( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	* 	oi = eu->oe;

	if( argu = skipsps( argu ), *argu )
	{
		oi->roomd = strdup( argu );
	}

	ie_print_obj( ch, eu );
}

static void ie_print_used( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[100];
	char				tmp[MAX_STRING_LENGTH+1];

	strcpy( tmp, oi->usedd ? oi->usedd : "None" );

	sendf( ch, "Iedit > %s", remove_newline( tmp ) );

	sprintf( buf, "Iedit %s - used > ", oi->wornd );

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_USED;
}

static void ie_get_used( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	* 	oi = eu->oe;

	if( argu = skipsps( argu ), *argu )
	{
		oi->usedd = strdup( argu );
	}

	ie_print_obj( ch, eu );
}

static void ie_print_extr( charType * ch, editUnitType * eu )
{
	exdescriptionType	*	ex;
	objIndexType		*	oi = eu->oe;
	int						i;

	ex = oi->extrd;

	if( !ex )
	{
		sendf( ch, "Iedit > None" );
	}
	else
	{
		for( i = 1; ex; ex = ex->next )
			sendf( ch, "Iedit %d > %s\n\r%s", i++, ex->keyword, ex->description );
	}

	sendf( ch, "Editing extra description is not yet implemented." );
	ch->sub_2 = IEDIT_NONE;
}

static void ie_print_type( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	int					i, toggle = 0;
	char				buf[200];

	for( i = 1, toggle = 0; item_types[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i, item_types[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Iedit %s - type %s > ", oi->wornd, sprinttype( oi->type, item_types, 0));
	
	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_TYPE;
}

static void ie_get_type( charType * ch, editUnitType * eu, char * argu )
{
	int			nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > ITEM_BOAT )
		{
			sendf( ch, "Invalid item type." ); return;
		}

		if( nr && eu->oe->type != nr ) 
		{
			eu->oe->type = nr;
		}
	}

	ie_print_obj( ch, eu );
}

static void ie_print_wear( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	int					i, toggle = 0;
	char				buf[300];

	sprintf( buf, "%2d. %-20s", 0, "Clear" );

	for( i = 0, toggle = 1; wear_bits[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i + 1, wear_bits[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Iedit wear   %s \n\rIedit wear > ", sprintbit( oi->wear, wear_bits, 0 ) );
	
	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_WEAR;
}

static void ie_get_wear( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	*	oi = eu->oe;
	int					nr = 0;
	char				buf[300];

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > ITEM_LAST_WEAR )
		{
			sendf( ch, "Invalid wear flag." ); return;
		}
	}

	if( *argu && nr == 0 )
	{
		oi->wear = 0;
		sprintf( buf, "Iedit wear   %s \n\rIedit wear > ", sprintbit( oi->wear, wear_bits, 0 ) );
		ed_new_prompt( eu, buf );
	}
	else if( nr )
	{
		nr--;
		if( oi->wear & ( 1 << nr ) ) oi->wear &= ~(1 << nr);
		else 						 oi->wear |=   1 << nr ;

		sprintf( buf, "Iedit wear   %s \n\rIedit wear > ", sprintbit( oi->wear, wear_bits, 0 ) );
		ed_new_prompt( eu, buf );
	}
	else
	{
		ie_print_obj( ch, eu );
	}
}

static void ie_print_flag( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	int					i, toggle = 0;
	char				buf[200];

	sprintf( buf, "%2d. %-20s", 0, "Clear" );

	for( i = 0, toggle = 1; extra_bits[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i + 1, extra_bits[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Iedit extra   %s \n\rIedit extra > ", sprintbit( oi->extra, extra_bits, 0 ) );
	
	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_FLAG;
}

static void ie_get_flag( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[300];
	int					nr = 0;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > ITEM_LAST_FLAG )
		{
			sendf( ch, "Invalid extra flag." ); return;
		}
	}

	if( *argu && nr == 0 )
	{
		oi->extra = 0;
		sprintf( buf, "Iedit extra   %s \n\rIedit extra > ", sprintbit( oi->extra, extra_bits, 0 ) );
		ed_new_prompt( eu, buf );
	}
	else if( nr )
	{
		nr--;
		if( oi->extra & ( 1 << nr ) ) oi->extra &= ~(1 << nr);
		else 						  oi->extra |=   1 << nr ;

		sprintf( buf, "Iedit extra   %s \n\rIedit extra > ", sprintbit( oi->extra, extra_bits, 0 ) );
		ed_new_prompt( eu, buf );
	}
	else
	{
		ie_print_obj( ch, eu );
	}
}

static void ie_print_cost( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[300];

	sprintf( buf, "Iedit %s - cost %s > ", oi->wornd, numfstr( oi->cost ) );

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_COST;
}

static void ie_get_cost( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid cost." ); return;
		}

		eu->oe->cost = nr;
	}

	ie_print_obj( ch, eu );
}

static void ie_print_weight( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[300];

	sprintf( buf, "Iedit %s - weight %d ==> ", oi->wornd, oi->weight );

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_WEIGHT;
}

static void ie_get_weight( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid weight." ); return;
		}

		eu->oe->weight = nr;
	}

	ie_print_obj( ch, eu );
}

static void ie_print_gpd( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[300];

	sprintf( buf, "Iedit %s - gpd %d > ", oi->wornd, oi->magic );

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_GPD;
}

static void ie_get_gpd( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) == 0 )
		{
			sendf( ch, "Invalid gpd." ); return;
		}
		eu->oe->magic = nr;
	}

	ie_print_obj( ch, eu );
}

static void ie_print_level( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[300];

	sprintf( buf, "Iedit %s - level %d > ", oi->wornd, oi->level );

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_LEVEL;
}

static void ie_get_level( charType * ch, editUnitType * eu, char * argu )
{
	int		nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 )
		{
			sendf( ch, "Invalid level." ); return;
		}

		eu->oe->level = nr;
	}

	ie_print_obj( ch, eu );
}

static void ie_get_trap( charType * ch, editUnitType * eu, char * argu )
{
	int			nr;

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr == 0 )
		{
			sendf( ch, "Invalid timer value." ); return;
		}

		eu->oe->value[0] = nr;
	}

	ie_print_obj( ch, eu );
}

static void ie_list_spells( charType * ch, objIndexType * oi )
{
	int			i, j, toggle, level, type;
	char		buf[300];

	level = oi->value[0];
	type  = oi->type;

	sprintf( buf, "%3d. %-30s", 0, "None" );

	for( i = 1, toggle = 1; spells[i].name[0] != '\n'; i++ )
	{

		if( spells[i].type & type )
		{
			for( j = 0; j < 4; j++ ) if( spells[i].min_level[j] <= level ) break;

			if( j != 4 )
			{
				sprintf( buf + strlen( buf ), "%3d. %-30s", i, spells[i].name );
				
				if( ++toggle >= 2 )
				{
					sendf( ch, "%s", buf ); buf[0] = 0; toggle = 0;
				}
			}
		}
	}
}

static void ie_get_max_charge( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[300];
    int     			nr;
   
   	if( *argu )
   	{
    	if( getnumber( argu, &nr ) <= 0 )
    	{
        	sendf( ch, "Invalid max charge." ); return;
    	}
   
    	eu->oe->value[1] = nr;
	}

	sprintf( buf, "Iedit %s - charge %d > ", oi->wornd, oi->value[1] );

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_WAND_C;
}

static void ie_get_charge( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	*	oi = eu->oe;
    int             	nr;
    
	if( *argu )
	{
    	if( getnumber( argu, &nr ) <= 0 )
    	{
        	sendf( ch, "Invalid charge." ); return; 
    	}       
        
    	eu->oe->value[2] = nr;
	}

	ie_list_spells( ch, oi );

    ch->sub_2 = IEDIT_SPEL_3;    

	ie_spell_prompt( eu, 1, eu->oe->value[3] );

}

static void ie_get_ac( charType * ch, editUnitType * eu, char * argu )
{
    int             nr; 
    
	if( *argu )
	{
    	if( getnumber( argu, &nr ) == 0 || nr > 128 || nr < -127 )
    	{
        	sendf( ch, "Invalid ac." ); return; 
    	}
        
    	eu->oe->value[0] = nr;
	}

	ie_print_obj( ch, eu );
}

static void ie_get_light( charType * ch, editUnitType * eu, char * argu )
{
    int             nr; 
    
	if( *argu )
	{
    	if( getnumber( argu, &nr ) == 0 )
    	{
        	sendf( ch, "Invalid light." ); return; 
    	}

    	eu->oe->value[2] = nr;
	}
            
	ie_print_obj( ch, eu );
}

static void ie_get_spell_l( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[300];
    int     			nr;
   
   	if( *argu )
   	{
    	if( getnumber( argu, &nr ) <= 0 )
    	{
        	sendf( ch, "Invalid level of spell." ); return;
    	}
   
    	eu->oe->value[0] = nr;
	}


	switch( oi->type )
	{
		case	ITEM_SCROLL :
		case	ITEM_POTION :

			ch->sub_2 = IEDIT_SPEL_1;
    		ie_list_spells( ch, oi );
			ie_spell_prompt( eu, 1, oi->value[1] );
			break;

		case	ITEM_WAND	:
		case	ITEM_STAFF  :

			ch->sub_2 = IEDIT_WAND_M;
			sprintf( buf, "Iedit %s - max charge %d > ", oi->wornd, oi->value[1] );
			ed_new_prompt( eu, buf );
			break;

		default				:

			DEBUG( "ie_get_spell_l> Wrong type of item entered." );
			ie_print_obj( ch, eu );
			return;
	}
}


static void ie_get_spell( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	*	oi = eu->oe;
	int					nr;

	if( *argu )
	{
    	if( getnumber( argu, &nr ) <= 0 || nr > MAX_USED_SPELLS )
    	{
        	sendf( ch, "Invalid spell number." ); return;   
    	}
	}


	if( ch->sub_2 != IEDIT_SPEL_3 )
	{
		ie_list_spells( ch, oi );

		ie_spell_prompt( eu, 
						 ch->sub_2 == IEDIT_SPEL_1 ? 2 : 3, 
						 ch->sub_2 == IEDIT_SPEL_1 ? oi->value[2] : oi->value[3] );
	}
	else ie_print_obj( ch, eu );

	switch( ch->sub_2 )
	{
		case IEDIT_SPEL_1 : if( *argu ) oi->value[1] = nr; ch->sub_2 = IEDIT_SPEL_2; break;
		case IEDIT_SPEL_2 : if( *argu ) oi->value[2] = nr; ch->sub_2 = IEDIT_SPEL_3; break;
		case IEDIT_NONE   : if( *argu ) oi->value[3] = nr; break;
	}
}

static void ie_list_apply( charType * ch )
{
	int					i, toggle;
	char				buf[300];

	for( i = 1, toggle = 0; apply_types[i][0] != '\n'  ; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i, apply_types[i] );

		if( ++toggle >= 3 )
		{
			toggle = 0; sendf( ch, "%s", buf ); buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );
}

static void ie_print_apply( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	char				buf[300];

	ie_list_apply( ch );

	sprintf( buf, "Iedit %s - apply %s > ",	
			oi->wornd, sprinttype( oi->apply[0].location, apply_types, 0 ) );

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_APPL_1;
}

static void ie_get_apply_l( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	*	oi = eu->oe;
	int					j, nr;
	char				buf[300];

	if( *argu )
	{
		if( getnumber( argu, &nr ) <= 0 || nr > ITEM_LAST_APPLY )
		{
			sendf( ch, "Apply number is out of range." ); return;
		}
	}

	if( ch->sub_2 == IEDIT_APPL_1 )
	{
		j = 0; ch->sub_2 = IEDIT_APPM_1;
	}
	else
	{
		j = 1; ch->sub_2 = IEDIT_APPM_2;
	}

	if( *argu ) 	oi->apply[j].location = nr;
	else			nr = oi->apply[j].location;

	sprintf( buf, "Iedit %s - apply %s - %d > ", 
					oi->wornd, apply_types[nr], oi->apply[j].modifier );

	ed_new_prompt( eu, buf );
}

static void ie_get_apply_m( charType * ch, editUnitType * eu, char * argu )
{
	objIndexType	*	oi = eu->oe;
	int					nr;
	char				buf[300];

	if( *argu )
	{
		if( getnumber( argu, &nr ) == 0 )
		{
			sendf( ch, "Wrong number." ); return;
		}
	}

	if( ch->sub_2 == IEDIT_APPM_1 )
	{
		ch->sub_2 = IEDIT_APPL_2;

		ie_list_apply( ch );

		sprintf( buf, "Iedit %s - apply %s  > ", 
					oi->wornd, apply_types[oi->apply[1].location] );

		if( *argu ) oi->apply[0].modifier = nr;
		ed_new_prompt( eu, buf );
	}
	else
	{
		if( *argu ) oi->apply[1].modifier = nr;
		ie_print_obj( ch, eu );
	}

}

static char * w_specials[] =
{
	"Smash",
	"Flame",
	"Ice",
	"Bombard",
	"Shot",
	"Bolt",
	"Dragon Slayer",
	"Anti Good",
	"Anti Evil",
	"Mana Drain",
	"Hit Drain",
	"Move Drain",
	"\n"
};

static char * ie_weap_sp_name( objIndexType * oi )
{
	int				nr = oi->value[0];

	if( nr == 0 ) return "None";
	if( nr > 12 ) return "Invalid";

	return w_specials[nr-1];
}

static void ie_list_weapon_m( charType * ch )
{
	char				buf[300];
	int					i, toggle;

	sprintf( buf, "%2d. %-20s", 0, "None" );

	for( i = 0, toggle = 1; w_specials[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i + 1, w_specials[i] );

		if( ++toggle >= 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );
}

static void ie_list_weapon_t( charType * ch, editUnitType * eu )
{
	static char * weapon_t[] =
	{
		"Whip",
		"Slash",
		"Crush",
		"Bludgeon",
		"Sting",
		"Pierce",
		"Invalid",
		"\n"
	};

	objIndexType	*	oi = eu->oe;
	char				buf[300];
	int					i, toggle;

	for( i = 0, toggle = 0; i < 6; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i + 1, weapon_t[i] );

		if( ++toggle >= 3 )
		{
			sendf( ch, "%s", buf ); 
			toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	switch( oi->value[3] )
	{
        case 0  : 
		case 1  :
        case 2  : i = 0; break;
        case 3  : i = 1; break;
        case 4  : 
		case 5  :
        case 6  : i = 2; break;
        case 7  : i = 3; break;
        case 8  : 
		case 9  :
        case 10 : i = 4; break;
        case 11 : i = 5; break;
		default	: i = 6; break;
	}

	sprintf( buf, "Iedit %s - type %s  > ", oi->wornd, weapon_t[i] );

	ed_new_prompt( eu, buf );
}

static void ie_print_drink( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	int					i, toggle = 0;
	char				buf[200];

	buf[0] = 0;

	for( i = 0, toggle = 0; drinknames[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i, drinknames[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Iedit %s - liquid %s > ", oi->wornd, sprinttype( oi->value[2], drinknames, 0));
	
	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_DCON_T;
}

static char * l_poisons[] =
{
	"No",
	"Yes",
	"Hit restore",
	"Mana restore",
	"Move restore",
	"\n"
};

static void ie_print_poison( charType * ch, editUnitType * eu )
{
	objIndexType	*	oi = eu->oe;
	int					i, toggle = 0;
	char				buf[200];

	buf[0] = 0;

	for( i = 0, toggle = 0; l_poisons[i][0] != '\n'; i++ )
	{
		sprintf( buf + strlen( buf ), "%2d. %-20s", i, l_poisons[i] );

		if( ++toggle == 3 )
		{
			sendf( ch, "%s", buf ); toggle = 0; buf[0] = 0;
		}
	}

	if( toggle ) sendf( ch, "%s", buf );

	sprintf( buf, "Iedit %s - poison %s > ", oi->wornd, sprinttype( oi->value[2], l_poisons, 0));
	
	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_DCON_P;
}

static void ie_get_cont_s( charType * ch, editUnitType * eu, char * argu )
{
    objIndexType    *   oi = eu->oe;
    int                 nr;
    char                buf[300];

    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0  )
        {
            sendf( ch, "Wrong container size." ); return;
        }

		oi->value[0] = nr;
	}

	if( oi->type == ITEM_CONTAINER )
	{
		ie_print_obj( ch, eu );
	}
	else
	{
		sprintf( buf, "Iedit %s - contains %d  > ", oi->wornd, oi->value[1] );

		ed_new_prompt( eu, buf );

		ch->sub_2 = IEDIT_DCON_C;
	}
}

static void ie_get_dcon_c( charType * ch, editUnitType * eu, char * argu )
{
    objIndexType    *   oi = eu->oe;
    int                 nr;

    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0  )
        {
            sendf( ch, "Wrong container size." ); return;
        }

		oi->value[1] = nr;
	}

	ie_print_drink( ch, eu );
}

static void ie_get_dcon_t( charType * ch, editUnitType * eu, char * argu )
{
    objIndexType    *   oi = eu->oe;
    int                 nr;

    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0 || nr > LIQ_LAST )
        {
            sendf( ch, "Wrong liquid type." ); return;
        }

		oi->value[2] = nr;
	}

	ie_print_poison( ch, eu );
}

static void ie_get_dcon_p( charType * ch, editUnitType * eu, char * argu )
{
    objIndexType    *   oi = eu->oe;
    int                 nr;

    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0 || nr > 4 )
        {
            sendf( ch, "Wrong poison type." ); return;
        }

		oi->value[3] = nr;
	}

	ie_print_obj( ch, eu );
}

static void ie_get_weapon_m( charType * ch, editUnitType * eu, char * argu )
{
    objIndexType    *   oi = eu->oe;
    int                 nr;
    char                buf[300];
   
    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0 || nr > WEAPON_LAST_SPECIAL )
        {
            sendf( ch, "Wrong weapon speical number." ); return;
        }

		oi->value[0] = nr;
    }

	sprintf( buf, "Iedit %s - sDice %d > ", oi->wornd, oi->value[1] );

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_WEAP_S;
}

static void ie_get_weapon_s( charType * ch, editUnitType * eu, char * argu )
{
    objIndexType    *   oi = eu->oe;
    int                 nr;
    char                buf[300];
   
    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0 )
        {
            sendf( ch, "Wrong size dice." ); return;
        }

		oi->value[1] = nr;
    }

	sprintf( buf, "Iedit %s - nDice %d > ", oi->wornd, oi->value[2] );

	ed_new_prompt( eu, buf );

	ch->sub_2 = IEDIT_WEAP_N;
}

static void ie_get_weapon_n( charType * ch, editUnitType * eu, char * argu )
{
    objIndexType    *   oi = eu->oe;
    int                 nr;
   
    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0 )
        {
            sendf( ch, "Wrong number of dice." ); return;
        }

		oi->value[2] = nr;
    }

	ie_list_weapon_t( ch, eu );

	ch->sub_2 = IEDIT_WEAP_T;
}

static void ie_get_weapon_t( charType * ch, editUnitType * eu, char * argu )
{
    objIndexType    *   oi = eu->oe;
    int                 nr;
   
    if( *argu )
    {
        if( getnumber( argu, &nr ) <= 0 || nr > 6 )
        {
            sendf( ch, "Wrong number." ); return;
        }

		switch( nr )
		{
			case 0 : oi->value[3] = 0; break;
			case 1 : oi->value[3] = 2; break;
			case 2 : oi->value[3] = 3; break;
			case 3 : oi->value[3] = 6; break;
			case 4 : oi->value[3] = 7; break;
			case 5 : oi->value[3] = 10; break;
			case 6 : oi->value[3] = 11; break;
		}
    }

	ie_print_obj( ch, eu );
}

static void ie_print_value( charType * ch, editUnitType * eu )
{
	char				buf[300];
	objIndexType	*	oi = eu->oe;

	switch( oi->type )
	{
		case	ITEM_WAND 		:
		case	ITEM_SCROLL		:
		case	ITEM_POTION		:
		case	ITEM_STAFF		:

			sprintf( buf, "Iedit %s - level %d > ", oi->wornd, oi->value[0] );
			break;

		case	ITEM_ARMOR		:
		case	ITEM_WORN		:

			sprintf( buf, "Iedit %s - AC %d > ", oi->wornd, oi->value[0] );
			break;

		case	ITEM_WEAPON		:

			ie_list_weapon_m( ch );

			sprintf( buf, "Iedit %s - Speicals %s > ", oi->wornd, ie_weap_sp_name( oi ));
			break;

		case	ITEM_FIREWEAPON	:

			sprintf( buf, "Iedit %s - Bullets %d > ", oi->wornd, oi->value[0] );
			break;

		case	ITEM_LIGHT		:

			sprintf( buf, "Iedit %s - light %d > ", oi->wornd, oi->value[2] );
			break;

		case	ITEM_CONTAINER	:
		case	ITEM_DRINKCON	:

			sprintf( buf, "Iedit %s - Size %d > ", oi->wornd, oi->value[0] );
			break;

		case	ITEM_TRAP		:
			
			sprintf( buf, "Iedit %s - Timer %d > ", oi->wornd, oi->value[0] );
			break;

		default					:

			sendf( ch, "Not yet implemented or meaningless values." );
			return;
	}

	ed_new_prompt( eu, buf );

	switch( oi->type )
	{
        case    ITEM_WAND       :
        case    ITEM_STAFF      : 
        case    ITEM_SCROLL     : 
        case    ITEM_POTION     : ch->sub_2 = IEDIT_SPEL_L;			break;
        case    ITEM_WORN       :
        case    ITEM_ARMOR      : ch->sub_2 = IEDIT_AC;				break;
        case    ITEM_WEAPON     : ch->sub_2 = IEDIT_WEAP_M;			break;
        case    ITEM_FIREWEAPON : ch->sub_2 = IEDIT_FIRE_M;			break;
        case    ITEM_DRINKCON   :
        case    ITEM_CONTAINER  : ch->sub_2 = IEDIT_CONT_S;			break;
		case	ITEM_LIGHT		: ch->sub_2 = IEDIT_LIGHT;			break;
		case	ITEM_TRAP		: ch->sub_2 = IEDIT_TRAP;			break;
	}
}

static void ie_quit_edit( charType * ch, editUnitType * eu )
{
	if( ch->sub_3 != EDIT_NONE )
	{
		switch( ch->sub_3 )
		{
			case	EDIT_YESNO  : break;
			case	EDIT_YES	: ie_effect( ch, eu ); 
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
		sendf( ch, "'%s' modified. Apply this to the table ? ", eu->oe->wornd );

		ch->sub_2 = IEDIT_QUIT;
		ch->sub_3 = EDIT_YESNO;
	}
	else
	{	
  		del_editing( eu );
	  	ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
	}
}

static void ie_next_edit( charType * ch, editUnitType * eu, int mode, int prev )
{
	editUnitType	*	edit;
	int					nr;

	nr = eu->oe->nr;

	if( prev )
	{
		if( --nr <= OBJECT_INTERNAL )
		{
			sendf( ch, "No previous object in the object table." ); return;
		}
	}
	else
	{
		if( ++nr >= obj_index_info.used )
		{
			sendf( ch, "No more object in the object table." ); return;
		}
	}

	if( mode && ed_is_modified( ch, eu ) )
	{
		ie_effect( ch, eu );
	}

	del_editing( eu );
  	ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;

	edit = new_editing( ch, EDIT_OBJ, nr );
	ie_print_obj( ch, edit );
}

static void ie_change_edit( charType * ch, editUnitType * eu, char * argu )
{       
    editUnitType    *   edit;
    int                 nr;
    char                buf1[MAX_INPUT_LENGTH+1];
    char                buf2[MAX_INPUT_LENGTH+1];
 
    twoArgument( argu, buf1, buf2 );
    
    if( !isnumstr( buf2 ) )
    {
        sendf( ch, "Need number of object to change editing object." );
        return;
    }
        
    if( getnumber( buf2, &nr ) <= 0 )
    {       
        sendf( ch, "Invalid object number." ); return;
    }

    if( nr = real_objectNr( nr ), nr == NOWHERE )
    {   
        sendf( ch, "No object by that number." ); return;
    }
    
    if( ed_is_modified( ch, eu ) )
    {   
        ed_save_edit( ch, eu );
    }
    
    del_editing( eu );
    ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
    
    edit = new_editing( ch, EDIT_OBJ, nr );
    ie_print_obj( ch, edit );       
}

static void ie_help( charType * ch )
{
	sendf( ch, "Iedit help ----------" );
	sendf( ch, "# > to edit specified field." );
	sendf( ch, "S > apply edited object to the object table." );
	sendf( ch, "Q > quit."			  		  );
	sendf( ch, "X > quit without saving."     );
	sendf( ch, "N > edit next object in the object table." );
	sendf( ch, "P > edit previous object in the object table." );
	sendf( ch, "J > edit next object without saving." );
	sendf( ch, "K > edit previous object without saving." );
	sendf( ch, "G # > change editing object to new." );
	sendf( ch, "H > show this page." );
}

void do_iedit( charType * ch, char * argu, int cmd )
{
	editUnitType	*	edit;
	int					nr;
	int					ret;

	if( (ch->mode && ch->mode != EDIT_MODE) || (ch->sub_1 && ch->sub_1 != EDIT_OBJ) )
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

		if( nr = real_objectNr( nr ), nr <= OBJECT_INTERNAL )
		{
			sendf( ch, "That object is not found." ); return;
		}	

		if( ret = find_editing( ch, EDIT_OBJ, nr ), ret > 0 )
		{	
			if( ret == 1 )
			{
				sendf( ch, "That object is not available now." ); return;
			}

			if( ret == 2 )
			{
				sendf( ch, "Oops.. You are already in editing list. Retry" ); 
				edit = find_editunit( ch );
				del_editing( edit );
				return;
			}
		}

		edit = new_editing( ch, EDIT_OBJ, nr );
		ie_print_obj( ch, edit );

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
		case IEDIT_NAME			:	ie_get_name( ch, edit, argu ); 			break;
		case IEDIT_WORN 		:	ie_get_worn( ch, edit, argu ); 			break;
		case IEDIT_ROOM			: 	ie_get_room( ch, edit, argu ); 			break;
		case IEDIT_USED			:	ie_get_used( ch, edit, argu ); 			break;
		case IEDIT_TYPE			:	ie_get_type( ch, edit, argu );			break;
		case IEDIT_WEAR			:	ie_get_wear( ch, edit, argu );			break;
		case IEDIT_FLAG			:	ie_get_flag( ch, edit, argu );			break;
		case IEDIT_COST			:	ie_get_cost( ch, edit, argu );			break;
		case IEDIT_WEIGHT		:	ie_get_weight( ch, edit, argu );		break;
		case IEDIT_LEVEL		:	ie_get_level( ch, edit, argu );			break;
		case IEDIT_GPD			:	ie_get_gpd( ch, edit, argu );			break;

		case IEDIT_WAND_M		:	ie_get_max_charge( ch, edit, argu ); 	break;
		case IEDIT_WAND_C		:	ie_get_charge( ch, edit, argu );		break;

		case IEDIT_SPEL_L		:	ie_get_spell_l( ch, edit, argu );		break;

		case IEDIT_SPEL_1		:
		case IEDIT_SPEL_2		:
		case IEDIT_SPEL_3		:	ie_get_spell( ch, edit, argu );			break;

		case IEDIT_AC			:	ie_get_ac( ch, edit, argu );			break;
		case IEDIT_LIGHT		:	ie_get_light( ch, edit, argu );			break;

		case IEDIT_WEAP_M		:	ie_get_weapon_m( ch, edit, argu );		break;
		case IEDIT_WEAP_S		:	ie_get_weapon_s( ch, edit, argu );		break;
		case IEDIT_WEAP_N		:	ie_get_weapon_n( ch, edit, argu );		break;
		case IEDIT_WEAP_T		:	ie_get_weapon_t( ch, edit, argu );		break;

		case IEDIT_APPL_1		:
		case IEDIT_APPL_2		: 	ie_get_apply_l( ch, edit, argu );		break;
		case IEDIT_APPM_1		:
		case IEDIT_APPM_2		: 	ie_get_apply_m( ch, edit, argu );		break;

		case IEDIT_CONT_S		:	ie_get_cont_s( ch, edit, argu );		break;
		case IEDIT_DCON_C		:	ie_get_dcon_c( ch, edit, argu );		break;
		case IEDIT_DCON_T		:	ie_get_dcon_t( ch, edit, argu );		break;
		case IEDIT_DCON_P		:	ie_get_dcon_p( ch, edit, argu );		break;
		case IEDIT_TRAP			:	ie_get_trap( ch, edit, argu );			break;

		case IEDIT_QUIT			:   ie_quit_edit( ch, edit );       	    break;
		}
		return;
	}

	if( getnumber( argu, &nr ) > 0 )
	{
		switch( nr )
		{
		case	 1  :	ie_print_name  ( ch, edit ); 				break;
		case	 2  :	ie_print_room  ( ch, edit ); 				break;
		case	 3  :	ie_print_worn  ( ch, edit ); 				break;
		case	 4  :	ie_print_used  ( ch, edit ); 				break;
		case	 5 	:	ie_print_extr  ( ch, edit ); 				break;
		case	 6  :	ie_print_type  ( ch, edit ); 				break;
		case	 7  :	ie_print_flag  ( ch, edit ); 				break;
		case	 8  :	ie_print_wear  ( ch, edit ); 				break;
		case	 9  :	ie_print_weight( ch, edit ); 				break;
		case	10  :	ie_print_cost  ( ch, edit ); 				break;
		case	11  :	ie_print_gpd   ( ch, edit ); 				break;
		case	12  :	ie_print_level ( ch, edit ); 				break;
		case	13  :	ie_print_value ( ch, edit ); 				break;
		case	14  :	ie_print_apply ( ch, edit ); 				break;
		default		:	sendf( ch, "Huh? type 'h' for help." ); 	break;
		}
	}
	else
	{
		if( !*argu ) 
		{
			ie_print_obj( ch, edit ); return;
		}
		else if( strlen( argu ) >= 1 )
		{
			switch( *argu )
			{
			case	'S' : 	ie_effect( ch, edit );  					return;	 
			case	'Q' :	ie_quit_edit( ch, edit ); 	 				return;
			case 	'X'	:  	del_editing( edit ); 
							ch->sub_1 = ch->sub_2 = ch->sub_3 = EDIT_NONE;
																		return;
			case	'N' :   ie_next_edit( ch, edit, 1, 0 );  			return;
			case	'P' :   ie_next_edit( ch, edit, 1, 1 );  			return;
			case	'J' :   ie_next_edit( ch, edit, 0, 0 );  			return;
			case	'K' :   ie_next_edit( ch, edit, 0, 1 );  			return;
			case	'F' :	ie_effect	( ch, edit );					return;
			case	'H' :	ie_help( ch );								return;
			case	'G' :	ie_change_edit( ch, edit, argu );			return;
			}
		}
		interpreter( ch, argu, 0 );
	}
}

void ed_icopy( charType * ch, char * argu, int cmd )
{
}
