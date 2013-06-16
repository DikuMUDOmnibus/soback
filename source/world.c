#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "allocate.h"
#include "strings.h"
#include "world.h"
#include "zone.h"

#define LAST_WLD_INDEX	99999

roomType 	*	world;
int				rooms_in_world 	= 0;
int				room_temple		= 0;
int				room_meditation = 0;
int				room_audience 	= 0;
int				room_library 	= 0;

int real_roomNr( int vNr )
{
    int     top = room_index_info.used - 1;
    int     mid;
    int     bot = 0;
    int     rval;

    while( 1 )
    {
        mid = bot + (top - bot) / 2;
  
        if( rval = vNr - world[ mid ].virtual, !rval ) return mid;

        if( bot == mid && top == mid )
		{
			DEBUG( "real_roomNr> #%d room not found in world.", vNr );
			return NOWHERE;
		}
 
        if( rval < 0 )
        {
            if( top == mid ) mid--;
            top = mid; continue;
        }
        else
        {
            if( bot == mid ) mid++;
            bot = mid; continue;
        }
    }
}

static void setup_direction( FILE *fp, int room, int dir ) 
{       
  	int scan;
            
	world[room].dirs[dir] = (directionType *)
                                   errCalloc( sizeof( directionType ) );	

  	world[room].dirs[dir]->description = fread_string( fp );
  	world[room].dirs[dir]->keyword 	   = fread_string( fp );
 
  	fscanf(fp, " %d ", &scan);

  	if      (scan == 1) world[room].dirs[dir]->exit_info = EX_ISDOOR; 
  	else if (scan == 2) world[room].dirs[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
  	else if (scan == 3) world[room].dirs[dir]->exit_info = EX_ISDOOR | EX_NOPHASE;
  	else if (scan == 4) world[room].dirs[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF | EX_NOPHASE;
  	else 				world[room].dirs[dir]->exit_info = 0;  

  	fscanf(fp, " %d ", &scan); world[room].dirs[dir]->key     = scan < 0 ? -1 : scan;
  	fscanf(fp, " %d ", &scan); world[room].dirs[dir]->to_room = scan;
}

void reindex_world( void )
{
  	int 		room, door;

  	for( room = 0; room <= room_index_info.used - 1; room++ )   
    	for( door = 0; door <= 5; door++ )
		{
      		if( world[room].dirs[door] && world[room].dirs[door]->to_room != NOWHERE )
       			world[room].dirs[door]->to_room = 
											real_roomNr(world[room].dirs[door]->to_room);
		}
}

static int read_a_room( FILE * fp, roomType * rm, int index, int virtual )
{
    char        			buf[MAX_STRING_LENGTH + 1];
    int         			i;
	exdescriptionType	* 	new_exd;
   
    rm->light 		= 0;
    rm->nr 			= index; 
	rm->virtual 	= virtual;
   
    rm->name        = fread_string( fp );
    rm->description = fread_string( fp );
   
    fscanf( fp, "%d ", &rm->zone );
	rm->flags = fread_number( fp );
    fscanf( fp, "%d \n", &rm->sector );
   
    for( i = 0; i < MAX_DIRECTIONS; i++ ) rm->dirs[i] = 0;
   
    rm->extrd = 0;
   
    for( ;; )
    {
        fscanf( fp, "%s \n", buf );

		switch( buf[0] )
		{
			case 'D' : 	setup_direction( fp, index, atoi( buf + 1 ) );
						break;
			case 'E' :  new_exd = (exdescriptionType *) errMalloc( sizeof( exdescriptionType ) );
          				new_exd->keyword     = fread_string(fp); 
          				new_exd->description = fread_string(fp);  
          				new_exd->next 		 = rm->extrd;
          				rm->extrd     		 = new_exd;  
						break;
			case 'S' : 	return rm->zone;
			default  :	FATAL( "read_a_room> unkown option %c entered from #%d.", buf[0], virtual ); 
		}
	}

	rooms_in_world++;
}

static void write_a_room( FILE * fp, roomType * room )
{
	int						i, ex, tmp;
	directionType		*	door;

	fprintf( fp, "#%d\n", room->virtual );
	fwrite_string( fp, room->name );
	fwrite_string( fp, room->description );
	fprintf( fp, "%d %d %d\n", zones[room->zone].zone, room->flags, room->sector );

	write_extrd( fp, room->extrd );

	for( i = 0; i < 6; i++ )
	{
		if( door = room->dirs[i], door )
		{
			fprintf( fp, "D%d\n", i );
			fwrite_string( fp, door->description );
			fwrite_string( fp, door->keyword );

  			ex = 0;

            tmp = door->exit_info & (EX_ISDOOR | EX_PICKPROOF | EX_NOPHASE);

            if     ( tmp == (EX_ISDOOR | EX_PICKPROOF | EX_NOPHASE) ) ex = 4;
            else if( tmp == (EX_ISDOOR | EX_NOPHASE) )                ex = 3;
            else if( tmp == (EX_ISDOOR | EX_PICKPROOF) )              ex = 2;
            else if( tmp == (EX_ISDOOR) )                             ex = 1;

			fprintf( fp, "%d %d %d\n", 
                   ex, door->key, door->to_room == -1 ? -1 : world[door->to_room].virtual );
		}
	}

	fprintf( fp, "S\n" );
}

void write_rooms( FILE * fp, int zone )
{
	roomType	*	room;

	fprintf( fp, "#ROOMS\n" );

	for( room = zones[zone].rooms; room; room = room->next ) write_a_room( fp, room );

	fprintf( fp, "#%d\n\n", LAST_WLD_INDEX ); 
}

void boot_rooms( FILE * fp )
{
    char        buf[MAX_STRING_LENGTH+1];
    int         nr, scan = -1, oldscan;
   
   	do
   	{
   		fgets( buf, MAX_STRING_LENGTH, fp );

	} while( buf[0] != '#' );

    for( scan = -1, nr = room_index_info.used;; nr++ )
    {
        fgets( buf, MAX_STRING_LENGTH - 1, fp );
   
        oldscan = scan;
   
        sscanf( buf, "#%d", &scan );

        if( oldscan >= scan )
        {
            FATAL( "boot_world> Numbering room index corrupted. #%d -> #%d",
                    oldscan, scan );
        }

        if( scan == LAST_WLD_INDEX || buf[0] == '$' ) break;

        if( buf[0] == '#' )
        {
		 	if( nr >= room_index_info.max ) FATAL( "boot_rooms> room table overflows." );

            read_a_room( fp, &world[nr], nr, scan );
        }
        else
            FATAL( "boot_world> error in room next to (virtualNr) #%ld.", oldscan );

		room_index_info.used++;
		rooms_in_world++;
    }
	fscanf( fp, "\n" );
}
