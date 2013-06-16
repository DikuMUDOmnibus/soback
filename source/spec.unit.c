#include "character.h"
#include "mobile.h"
#include "object.h"
#include "world.h"
#include "specials.h"
#include "comm.h"
#include "allocate.h"
#include "variables.h"

specialUnitType		*	specUnit_list;

void add_specUnit( specialUnitType * unit )
{
    specialUnitType   * unit_alloc;
    
    unit_alloc = alloc_specUnit();
    
    unit->unit = unit_alloc->unit; *unit_alloc = *unit;
        
    unit_alloc->next = specUnit_list;
    specUnit_list = unit_alloc;
}

void del_specUnit( specialUnitType * unit )
{
    specialUnitType   * tmp;

	if( unit == specUnit_list ) specUnit_list = unit->next;
	else
	{
		for( tmp = specUnit_list; tmp && tmp->next != unit; tmp = tmp->next )
		;

		if( tmp->next != unit )
		{
			FATAL( "del_specUnit> unit not found." );
		}

		tmp->next = unit->next;
	}

	free_specUnit( unit );
}

void update_specUnit( void )
{
    specialUnitType   * tmp, * tmp_next;

	for( tmp = specUnit_list; tmp; tmp = tmp_next )
	{
		tmp_next = tmp->next;

		if( ++tmp->tics == tmp->life ) 
		{
/*			DEBUG( "update_specUnit> calling %s func (%d tics).", 
					tmp->name? tmp->name : "unkown", tmp->tics );
*/
			(*tmp->func)(); tmp->tics = 0;
		}
	}
}
