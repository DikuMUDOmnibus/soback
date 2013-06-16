#include <string.h>

#include "defines.h"
#include "allocate.h"
#include "character.h"
#include "mobile.h"
#include "world.h"
#include "object.h"
#include "zone.h"
#include "specials.h"
#include "comm.h"
#include "hash.h"
#include "queue.h"

struct alloc_info char_array_info =
{ 	"Char table", 			0, 	0,  4000, sizeof( charType ), 		100, 0, 0, 0 };

struct alloc_info object_array_info =
{ 	"Object table", 		0, 	0,  5000, sizeof( objectType ), 	200, 0, 0, 0 };

struct alloc_info desc_array_info =
{ 	"Desc table", 		    0, 	0,  40,	 sizeof( descriptorType ),    4, 0, 0, 0 };

struct alloc_info affect_array_info =
{ 	"Affect table", 	    0, 	0, 100,	 sizeof( affectType ), 	   	 10, 0, 0, 0 };

struct alloc_info round_array_info =
{ 	"Round Aff table", 	    0, 	0,  30,	 sizeof( roundAffType ),   	 10, 0, 0, 0 };

struct alloc_info spec_array_info =
{ 	"Spec Unit table", 	    0, 	0,  10,	 sizeof( specialUnitType ),	 10, 0, 0, 0 };

struct alloc_info hash_array_info =
{ 	"HashEntry table", 	    0, 	0, 100,	 sizeof( hashType ),	 	 20, 0, 0, 0 };

struct alloc_info queue_array_info =
{ 	"Queue table", 	    	0, 	0, 100,	 sizeof( queueNode ),	 	 20, 0, 0, 0 };

/* fixed size tables currently can't not be expanded on the fly.
*/
struct alloc_info zone_index_info =
{ 	"Zone table", 		0, 	0,  100,  sizeof( zoneType ),  	  	  		0, 0, 0, 1 };

struct alloc_info room_index_info =
{ 	"Room table", 		0, 	0, 4800,  sizeof( roomType ), 	 	  		0, 0, 0, 1 };

struct alloc_info obj_index_info =
{ 	"Object index", 	0, 	0, 2000,  sizeof( struct object_index ),   	0, 0, 0, 1 };

struct alloc_info mob_index_info =
{ 	"Mobile index", 	0, 	0, 1800,  sizeof( struct mobile_index ),    0, 0, 0, 1 };


static void init_alloctable( struct alloc_info * info, void ** table )
{
	void	* 	vp;
	int 		i;

	if( info->fixed )
	{
		info->table = table;

		if( (*info->table) = errCalloc( info->unitsize * info->max ), !(*info->table) )
		{
			FATAL( "init_alloctable> %s initialize failed.", info->name ); 
		}

		for( i = 0; i < info->max; i++ )
		{
			vp = (*info->table) + (i * info->unitsize);
		
			*((int *)vp) = i + 1;
		}
	}
	else
	{
		info->index = (int *)errMalloc( info->max * sizeof(int) );
		info->table = (void **)errMalloc( info->max * sizeof(void *) );

		if( !info->index || !info->table )
		{
			FATAL( "init_alloctable> %s initialize failed.", info->name ); 
		}
		
		for( i = 0; i < info->max; i++ )
		{
			if( info->table[i] = errMalloc( info->unitsize ), !info->table[i] )
			{
				FATAL( "init_alloctable> %s initialize failed when alloc units.", info->name ); 
			}
			info->index[i] = i + 1;
		}
	}		
}

void expand_table( struct alloc_info * info )
{
	int			i;

	info->index = errRealloc( info->index, (info->max + info->delta) * sizeof(int) );
	info->table = errRealloc( info->table, (info->max + info->delta) * sizeof(void *) );

	if( info->index && info->table )
	{
		DEBUG( "expand_table> expanding %s from %d to %d.",	
									info->name, info->max, info->max + info->delta );

		for( i = info->max; i < (info->max + info->delta); i++ )
		{
			if( info->table[i] = errMalloc( info->unitsize ), !info->table[i] )
			{
				FATAL( "init_alloctable> %s initialize failed when alloc units.", info->name ); 
			}
			info->index[i] = i + 1;
		}
		info->max += info->delta;
	}
	else
	{
		FATAL( "expand_table> no more memory?? %s - %d -> %d.",
									info->name, info->max, (info->max + info->delta) );
	}
}

void * alloc_unit( struct alloc_info * info )
{
	int		free;

	if( info->nextfree >= info->max )
	{
		expand_table( info );
	}

	free = info->nextfree;

	info->nextfree = info->index[free];

	memset( info->table[free], 0, info->unitsize );

	*((int *)info->table[free]) = free;

	info->used++;

	return info->table[free];
}

void free_unit( struct alloc_info * info, void * vp )
{
	int		free;

	free = *((int *)vp);

	if( free < 0 || free >= info->max )
	{
		FATAL( "free_unit> %d is out of range of %d.", free, info->max );
	}

	info->used--;
	info->index[free] = info->nextfree;
	info->nextfree = free;
}

charType * alloc_char( void )
{
	return alloc_unit( &char_array_info );
}

void free_char( charType * ch )
{
	free_unit( &char_array_info, ch );
}

objectType * alloc_object( void )
{
	return alloc_unit( &object_array_info );
}

void free_object( objectType * obj )
{
	free_unit( &object_array_info, obj );
}

descriptorType * alloc_descriptor( void )
{
	return alloc_unit( &desc_array_info );
}

void free_desc( descriptorType * desc )
{
	free_unit( &desc_array_info, desc );
}

affectType * alloc_affect( void )
{
	return alloc_unit( &affect_array_info );
}

void free_affect( affectType * af  )
{
	free_unit( &affect_array_info, af );
}

roundAffType * alloc_round( void )
{
	return alloc_unit( &round_array_info );
}

void free_round( roundAffType * rf  )
{
	free_unit( &round_array_info, rf );
}

specialUnitType * alloc_specUnit( void )
{
	return alloc_unit( &spec_array_info );
}

void free_specUnit( specialUnitType * su  )
{
	free_unit( &spec_array_info, su );
}

hashType * alloc_hashType( void )
{
	return alloc_unit( &hash_array_info );
}

void free_hashType( hashType * he  )
{
	free_unit( &hash_array_info, he );
}

queueNode * alloc_queue( void )
{
	return alloc_unit( &queue_array_info );
}

void free_queue( queueNode * qn  )
{
	free_unit( &queue_array_info, qn );
}

void init_allocation( void )
{
	init_alloctable( &room_index_info, 	(void **)&world   );
	init_alloctable( &obj_index_info, 	(void **)&objects );
	init_alloctable( &mob_index_info, 	(void **)&mobiles );
	init_alloctable( &zone_index_info,  (void **)&zones   );
	
	init_alloctable( &char_array_info, 	(void **)0 );
	init_alloctable( &object_array_info,(void **)0 );
	init_alloctable( &desc_array_info, 	(void **)0 );
	init_alloctable( &affect_array_info,(void **)0 );
	init_alloctable( &round_array_info, (void **)0 );
	init_alloctable( &spec_array_info,  (void **)0 );
	init_alloctable( &hash_array_info,  (void **)0 );
	init_alloctable( &queue_array_info,  (void **)0 );
}

static void print_info( charType * ch, struct alloc_info * info )
{
	sendf( ch, "%16s --   [%4d]     [%4d]     [%4d]    [%3d]  %7d bytes.",
				info->name, info->max, info->used, info->unitsize, info->delta, 
				(info->max * info->unitsize) );
}

__DEBUG( extern unsigned long allocCount  );
__DEBUG( extern unsigned long allocAmount );
__DEBUG( extern unsigned long freeCount   );

void do_memory( charType * ch, char * argument, int cmd )
{
	sendf( ch, "Name of Table -- table size, used unit, unit size, delta,   total bytes.\n\r" );

	print_info( ch, &mob_index_info   	);
	print_info( ch, &obj_index_info   	);
	print_info( ch, &room_index_info   	);
	print_info( ch, &zone_index_info   	);
	print_info( ch, &object_array_info  );
	print_info( ch, &char_array_info  	);
	print_info( ch, &desc_array_info   	);
	print_info( ch, &affect_array_info 	);
	print_info( ch, &round_array_info 	);
	print_info( ch, &spec_array_info 	);
	print_info( ch, &hash_array_info 	);
	print_info( ch, &queue_array_info 	);

	sendf( ch, "\n\r objects in  world => %d", objs_in_world );
	sendf( ch,     " objects off world => %d", objs_off_world );
}
