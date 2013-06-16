#include <string.h>
#include <stdio.h>

#define __TRACE__

#include "defines.h"
#include "character.h"
#include "world.h"
#include "object.h"
#include "mobile.h"
#include "allocate.h"
#include "zone.h"
#include "find.h"
#include "strings.h"
#include "utils.h"
#include "comm.h"
#include "transfer.h"
#include "affect.h"
#include "variables.h"
#include "spec.rooms.h"
#include "page.h"

zoneType *	zones;
int			zones_in_world = 0;

#define OFF_LIMIT			100000	
#define LAST_ZONE_INDEX		999

static void index_rooms( void )
{
	int		i, curr, zone;

	for( i = 0; i < zones_in_world; i++ )
	{
		zones[i].top	= -1;
		zones[i].bottom = OFF_LIMIT;
	}

	for( curr = rooms_in_world - 1; curr > 0 ; curr-- )
	{
		zone = world[curr].zone;

		for( i = 0; i < zones_in_world; i++ )
		{
			if( zones[i].zone == zone || world[curr].virtual <= zones[i].max )
			{
				if( zones[i].bottom > world[curr].virtual ) zones[i].bottom = world[curr].virtual;
				if( zones[i].top    < world[curr].virtual ) zones[i].top    = world[curr].virtual;

				world[curr].next = zones[i].rooms;
				zones[i].rooms   = &world[curr];
				world[curr].zone = i;
				zones[i].rooms_in_zone++;
				break;
			}
		}
		if( i >= zone_index_info.used )
		{
			FATAL( "index_rooms> #%d room is not inside of any zone(%d).", 
						world[curr].virtual, zone );
		}
	}

	for( i = 0; i < zone_index_info.used; i++ )
	{
		zones[i].bottom = zones[i].bottom / 100 * 100;
		zones[i].max    = (zones[i].top / 100 + 1) * 100 - 1;
	}
}
#define Zcmd	(zones[zone].cmds[nr])

static void rebuild_reset_commands( void )
{
  	int 	zone, nr;
    
  	for( zone = ZONE_INTERNAL+1; zone < zones_in_world; zone++ )
  	{
    	for( nr = 0; Zcmd.command != 'S'; nr++)
		{
      		switch( Zcmd.command ) 
			{
				case 'M': 	Zcmd.arg1 = real_mobileNr( Zcmd.arg1 );
				  		  	Zcmd.arg3 = real_roomNr( Zcmd.arg3 );
						  	break;
				case 'O': 	Zcmd.arg1 = real_objectNr( Zcmd.arg1 );
				  			if( Zcmd.arg3 != NOWHERE)
								Zcmd.arg3  = real_roomNr( Zcmd.arg3 );
							break;
				case 'G':	Zcmd.arg1 = real_objectNr( Zcmd.arg1 );
							break;
				case 'E':   Zcmd.arg1 = real_objectNr( Zcmd.arg1 );
							break;
				case 'P':   Zcmd.arg1 = real_objectNr( Zcmd.arg1 );
				  			Zcmd.arg3 = real_objectNr( Zcmd.arg3 );
							break;
				case 'X':	Zcmd.arg1 = real_objectNr( Zcmd.arg1 );
				  			Zcmd.arg2 = real_roomNr( Zcmd.arg2 );
							break;
				case 'D':   Zcmd.arg1 = real_roomNr( Zcmd.arg1 );
							break;
				case 'R':   Zcmd.arg1 = real_roomNr( Zcmd.arg1 );
        					break;
      		}
  		}
	}
}

void reset_zone( int zone, int boot )
{
	int 				nr, last_cmd = 1;
	charType 		*	mob = 0;
	objectType 		*   obj, * obj_to;

	if( zones[zone].reset_mode == -1 ) return;

	sendzf( 0, zone, "You feel some quake of your world with new energy." );

  	for( nr = 0;; nr++ ) 
	{
    	if( Zcmd.command == 'S' ) break;

    	if( (!Zcmd.if_flag) || (Zcmd.if_flag == 1 && last_cmd) || (Zcmd.if_flag == 2 && !last_cmd) )
		{
      		switch( Zcmd.command ) 
			{
      		case 'M':	mob = 0;	
	  					if( Zcmd.arg1 != NIL 
                          && (mobiles[Zcmd.arg1].in_world < Zcmd.arg2 || boot || Zcmd.arg2 == -1) )
						{
          					mob = load_a_mobile( Zcmd.arg1, REAL );
          					char_to_room( mob, Zcmd.arg3 );
          					last_cmd = 1;
        				} 
						else last_cmd = 0;
      					break;

      		case 'O': 	if( Zcmd.arg1 != NIL 
						  && (objects[Zcmd.arg1].in_world < Zcmd.arg2 || boot || Zcmd.arg2 == -1) )
						{
							if( Zcmd.arg3 != NOWHERE ) 
							{
								for( obj = world[Zcmd.arg3].contents; obj; obj = obj->next_content )
									if( obj->nr == Zcmd.arg1 ) break;

								if( !obj )
								{
									obj = load_a_object(Zcmd.arg1, REAL, 1 );
									obj_to_room(obj, Zcmd.arg3);
									last_cmd = 1;
								} 
								else last_cmd = 0;
							}
							else last_cmd = 0;
						}
						else last_cmd = 0;
      					break;

      		case 'X': 	if( Zcmd.arg1 != NIL )
	  					{
							if( Zcmd.arg2 != NOWHERE )
							{
								for( obj = world[Zcmd.arg2].contents; obj; obj = obj->next_content )
									if( obj->nr == Zcmd.arg1 ) break;

								if( obj )
								{
									extract_obj( obj, 1 ); last_cmd = 1;
								}
								else last_cmd = 0;
							}
							else last_cmd = 0;
	  					}
	  					else last_cmd = 0;
						break;

      		case 'P': 	if( Zcmd.arg1 != NIL 
						 && (objects[Zcmd.arg1].in_world < Zcmd.arg2 || boot || Zcmd.arg2 == -1) ) 
						{
							if( Zcmd.arg3 != NIL && objects[Zcmd.arg3].in_world )
							{
          						obj = load_a_object( Zcmd.arg1, REAL, 1 );
  								for( obj_to = obj_list; obj_to; obj_to = obj_to->next )
    								if( obj_to->nr == Zcmd.arg3 ) break;

								if( !obj_to ) 
								{
									DEBUG( "reset_zone> put %s to what?.", obj->name );
									break;
								}
							}
							else
							{
								DEBUG( "reset_zone> [P] %s container(#%d) not loaded yet.",
											objects[Zcmd.arg3].name, objects[Zcmd.arg3].virtual );
								break;
							}

          					obj_to_obj(obj, obj_to);
          					last_cmd = 1;
        				}
        				else last_cmd = 0;
      					break;

      		case 'G':	if(  Zcmd.arg1 != NIL  
						  && mob 
                          && (objects[Zcmd.arg1].in_world < Zcmd.arg2 || boot || Zcmd.arg2 == -1) )
        				{    
          					obj = load_a_object( Zcmd.arg1, REAL, 1 );
          					obj_to_char(obj, mob);
          					last_cmd = 1;
        				}
        				else last_cmd = 0;
      					break;

      		case 'E':	if(  Zcmd.arg1 != NIL 
						  && mob 
                          && (objects[Zcmd.arg1].in_world < Zcmd.arg2 || boot || Zcmd.arg2 == -1) )
        				{    
          					obj = load_a_object( Zcmd.arg1, REAL, 1 );
          					equip_char(mob, obj, Zcmd.arg3);
          					last_cmd = 1;
        				} 
						else last_cmd = 0;
      					break;

      		case 'D':	if( Zcmd.arg1 != NOWHERE )
						{
        					switch (Zcmd.arg3)
        					{
          					case 0:
            					REMOVE_BIT(world[Zcmd.arg1].dirs[Zcmd.arg2]->exit_info,	EX_LOCKED);
            					REMOVE_BIT(world[Zcmd.arg1].dirs[Zcmd.arg2]->exit_info,	EX_CLOSED);
          						break;
          					case 1:
            					SET_BIT(world[Zcmd.arg1].dirs[Zcmd.arg2]->exit_info, EX_CLOSED);
            					REMOVE_BIT(world[Zcmd.arg1].dirs[Zcmd.arg2]->exit_info,	EX_LOCKED);
          						break;
          					case 2:
            					SET_BIT(world[Zcmd.arg1].dirs[Zcmd.arg2]->exit_info, EX_LOCKED);
            					SET_BIT(world[Zcmd.arg1].dirs[Zcmd.arg2]->exit_info, EX_CLOSED);
          						break;
        					}
						}
        				last_cmd = 1;
      					break;

			case 'R': 	if( Zcmd.arg1 != NOWHERE )
					  	{
							directionType	*	dir;
							int					i, j;
				
							for( i = 0; i < Zcmd.arg2 - 1; i++ )
							{
								j = number( 0, Zcmd.arg2 - 1 );
								dir = world[Zcmd.arg1].dirs[i];
								world[Zcmd.arg1].dirs[i] = world[Zcmd.arg1].dirs[j];
								world[Zcmd.arg1].dirs[j] = dir;
							}
						}
						last_cmd = 1;
						break;
			case '*': break;
      		default:
        			FATAL( "reset_zone> Undefined cmd in reset table - zone %d cmd %d.", zone, nr);
      				break;
    		}
		}
    	else last_cmd = 0;
  	}
}

#undef Zcmd

int is_empty( int zone )
{
	charType    *		temp;
	roomType 	*		room;

	for( room = zones[zone].rooms; room; room = room->next )
		for( temp = room->people; temp; temp = temp->next_in_room )
			if( !IS_NPC(temp) ) return FALSE;

	return TRUE;
}

void update_zones( void )
{
  	int 		i, j;

  	for( i = 0, j = 0; i < zones_in_world; i++)
  	{
  		if( zones[i].reset_mode >= 1 )
  		{
    		if( ++zones[i].age >= (zones[i].lifespan-2) )
    		{
				if( zones[i].reset_mode == 1 && !is_empty( i ) ) continue;

				if( (zones[i].age - zones[i].lifespan > 2 ) || number( 1, 3 ) == 1 ) 
				{
					reset_zone( i, 0 );

  					if( zones[i].age -= zones[i].lifespan, zones[i].age > 5 ) zones[i].age = 5;

  					if( ++j >= 10 ) return;
  				}
    		} 
		}
	}
}

void write_resets( FILE * fp, int nr )
{
	int				i;
	zoneType	*	zone = &zones[nr];

	fprintf( fp, "#RESET\n" );

	for( i = 0;; i++ )
	{
		switch( zone->cmds[i].command )
		{
			case  'M' :
						fprintf( fp, "M %d %5d %3d %5d   %s (%s)\n", 
							zone->cmds[i].if_flag,
							mobiles[zone->cmds[i].arg1].virtual,
							zone->cmds[i].arg2,
							world[zone->cmds[i].arg3].virtual,
							mobiles[zone->cmds[i].arg1].moved,
							world[zone->cmds[i].arg3].name ); break;
			case  'G' :
						fprintf( fp, "G %d %5d %3d             %s\n",
							zone->cmds[i].if_flag,
							objects[zone->cmds[i].arg1].virtual,
							zone->cmds[i].arg2,
							objects[zone->cmds[i].arg1].wornd ); break;
			case  'E' :
						fprintf( fp, "E %d %5d %3d %5d     %s\n", 
							zone->cmds[i].if_flag,
							objects[zone->cmds[i].arg1].virtual,
							zone->cmds[i].arg2,
							zone->cmds[i].arg3,
							objects[zone->cmds[i].arg1].wornd ); break;
			case  'O' : 
						fprintf( fp, "O %d %5d %3d %5d    %s (%s)\n", 
							zone->cmds[i].if_flag,
							objects[zone->cmds[i].arg1].virtual,
							zone->cmds[i].arg2,
							world[zone->cmds[i].arg3].virtual,
							objects[zone->cmds[i].arg1].wornd,
							world[zone->cmds[i].arg3].name ); break;
			case  'P' :
						fprintf( fp, "P %d %5d %3d %5d       %s (%s)\n", 
							zone->cmds[i].if_flag,
							objects[zone->cmds[i].arg1].virtual,
							zone->cmds[i].arg2,
							objects[zone->cmds[i].arg3].virtual,
							objects[zone->cmds[i].arg1].wornd,
							objects[zone->cmds[i].arg3].name ); break;
			case  'D' :
						fprintf( fp, "D %d %5d %3d %5d    %s\n", 
							zone->cmds[i].if_flag,
							world[zone->cmds[i].arg1].virtual,
							zone->cmds[i].arg2,
							zone->cmds[i].arg3,
							world[zone->cmds[i].arg1].name ); break;
			case  'X' :
						fprintf( fp, "X %d %5d %5d       %s (%s)\n", 
							zone->cmds[i].if_flag,
							objects[zone->cmds[i].arg1].virtual,
							world[zone->cmds[i].arg2].virtual,
							objects[zone->cmds[i].arg1].wornd,
							world[zone->cmds[i].arg2].name ); break;
			case  'L' :
						fprintf( fp, "L %d %5d %3d %5d    %s (%s)\n", 
							zone->cmds[i].if_flag,
							objects[zone->cmds[i].arg1].virtual,
							zone->cmds[i].arg2,
							world[zone->cmds[i].arg3].virtual,
							objects[zone->cmds[i].arg1].wornd,
							world[zone->cmds[i].arg3].name ); break;
			case  'Q' :
						fprintf( fp, "Q %d %5d %3d %5d    %s\n", 
							zone->cmds[i].if_flag,
							objects[zone->cmds[i].arg1].virtual,
							zone->cmds[i].arg2,
							mobiles[zone->cmds[i].arg3].virtual,
							objects[zone->cmds[i].arg1].wornd ); break;
			case  'R' :
						fprintf( fp, "R %d %5d %3d          %s\n", 
							zone->cmds[i].if_flag,
							world[zone->cmds[i].arg1].virtual,
							zone->cmds[i].arg2,
							world[zone->cmds[i].arg1].name ); break;
			case  '*' : fprintf( fp, "*%s", zone->cmds[i].arg1 ? (char *)zone->cmds[i].arg1 : "\n" );
						break;
			case  'S' : 
						fprintf( fp, "S\n\n" ); return;
			default   : ERROR( "write_reset> unknown reset command(%c)", zone->cmds[i].command );
		}
	}
}

static void boot_resets( FILE * fp, zoneType * zp )
{
	int			expand = 1;
	int			cmdNr;
	int			scan0, scan1, scan2;
	char		cmd;
	char		buf[MAX_STRING_LENGTH+1];

    do
	{
		fgets( buf, MAX_STRING_LENGTH, fp );

	} while( buf[0] != '#' );

	for( expand = 1, cmdNr = 0; cmd != 'S'; )
	{
		if( expand )
		{
			if( !zp->cmds ) zp->cmds = (struct reset_command *)errMalloc(sizeof(struct reset_command));
			else
				zp->cmds = (struct reset_command *)
							errRealloc( zp->cmds, (cmdNr+1) * (sizeof(struct reset_command)) );
		}

		expand = 0;

	  	fscanf( fp, " %c", &cmd );

        zp->cmds[cmdNr].command = cmd;

      	if( cmd == 'S' ) break;
      	if( cmd == '*')
      	{
        	fgets( buf, 255, fp ); 
			(char *)zp->cmds[ cmdNr ].arg1 = strdup( buf );
      	}
	  	else
		{
      		fscanf( fp, " %d %d %d", &scan0, &scan1, &scan2 ); 

			switch( cmd )
			{
				case 'M' :
				case 'O' :
				case 'E' :
				case 'P' :
				case 'D' :	fscanf( fp, " %d", &zp->cmds[ cmdNr ].arg3 ); break;
				case 'X' :
				case 'R' :
				case 'G' :  break;
				default  :
					FATAL( "read_a_zone> unkown command(%c) encountered in %d zone.", cmd, zp->zone );
			}

			zp->cmds[ cmdNr ].if_flag = scan0;
			zp->cmds[ cmdNr ].arg1    = scan1;
			zp->cmds[ cmdNr ].arg2 	  = scan2;

       		fgets( buf, 255, fp ); 
		}

		expand = 1; cmdNr++;
    }
	fscanf( fp, "\n" );
}

void save_a_zone( int	nr )
{
	FILE	*	fp;
	char		fname[200];

	sprintf( fname, "%s/%03d.zone", ZONE_DIR, zones[nr].zone );

	if( fp = errOpen( fname, "w" ), !fp )
	{
		ERROR( "save_zone> %s open error.", fname );
		return;
	}

	fprintf( fp, "#%d Zone.\n", zones[nr].zone );
	fprintf( fp, "NAME       : " ); 	fwrite_string( fp, zones[nr].name );
	fprintf( fp, "INFO       : " ); 	fwrite_string( fp, zones[nr].info ); 	
	fprintf( fp, "PATH       : " ); 	fwrite_string( fp, zones[nr].path );
	fprintf( fp, "ALIGN      : %d\n", zones[nr].align );
	fprintf( fp, "AGGRESSIVE : %d\n", zones[nr].aggressive );
	fprintf( fp, "LEVEL      : %d ~ %d\n", zones[nr].low, zones[nr].high );
	fprintf( fp, "MAX        : %d\n", zones[nr].max );
	fprintf( fp, "LIFESPAN   : %d\n", zones[nr].lifespan );
	fprintf( fp, "RESET      : %d\n", zones[nr].reset_mode );
	fprintf( fp, "\n" );

	write_mobiles( fp, nr, zones[nr].bottom, zones[nr].max );
	write_objects( fp, nr, zones[nr].bottom, zones[nr].max );
	write_rooms  ( fp, nr );
	write_resets ( fp, nr );
	write_shops	 ( fp, nr );

	fprintf( fp, "#END of %s.\n", zones[nr].name );

	errClose( fp );
}

void boot_a_zone( int nr, int scan )
{
	char				buf[256];
	int					zone;
	FILE			*	fp;

	sprintf( buf, "%s/%03d.zone", ZONE_DIR, scan );

	if( fp = errOpen( buf, "r" ), !fp ) FATAL( "boot_zones> %s open error.", buf );

	TRACE( trace("boot_zones> %d.zone", scan) );

	zones[nr].nr   = nr;
	zones[nr].zone = scan;

	fscanf( fp, "#%d Zone.\n", &zone );

	if( scan != zone ) FATAL( "boot_a_zone> scan(%d) not match with zone(%d).", scan, zone );

	fscanf( fp, "NAME       : " ); 	zones[nr].name = fread_string( fp );
	fscanf( fp, "INFO       : " ); 	zones[nr].info = fread_string( fp );	
	fscanf( fp, "PATH       : " ); 	zones[nr].path = fread_string( fp );

	fscanf( fp, "ALIGN      : %d\n", &zones[nr].align );
	fscanf( fp, "AGGRESSIVE : %d\n", &zones[nr].aggressive );
	fscanf( fp, "LEVEL      : %d ~ %d\n", &zones[nr].low, &zones[nr].high );
	fscanf( fp, "MAX        : %d\n", &zones[nr].max );
	fscanf( fp, "LIFESPAN   : %d\n", &zones[nr].lifespan );
	fscanf( fp, "RESET      : %d\n", &zones[nr].reset_mode );
	fscanf( fp, "\n" );

	boot_mobiles( fp );
	boot_objects( fp );
	boot_rooms  ( fp );
	boot_resets ( fp, &zones[nr] );
	boot_shops	( fp );

	errClose( fp );

}

void save_world( void )
{
	FILE	*		fp;
	int				i;
	char			fname[200];

	sprintf( fname, "%s/zone.index", ZONE_DIR );

	if( fp = errOpen( fname, "w" ), !fp )
	{
		DEBUG( "save_all_zone> can't open %s.", fname ); return;
	}

	for( i = ZONE_INTERNAL+1; i < zones_in_world; i++ )
	{
		fprintf( fp, "%3d.zone %s\n", zones[i].zone, zones[i].name );
		save_a_zone( i );
	}

	fprintf( fp, "%3d.zone End of index file.\n", LAST_ZONE_INDEX );
	errClose( fp );
}

void boot_world( void )
{
	int					nr, scan, oldscan;
	char 				buf[300];
	FILE			*	index;

	sprintf( buf, "%s/zone.index", ZONE_DIR );

	if( index = errOpen( buf, "r" ), !index )
	{
		FATAL( "boot_zones> %s open error.", buf );
	}

	zones_in_world = zone_index_info.used = ZONE_INTERNAL + 1;
	 				  mob_index_info.used = MOBILE_INTERNAL + 1;
	 				  obj_index_info.used = OBJECT_INTERNAL + 1;
	rooms_in_world = room_index_info.used = ROOM_INTERNAL + 1;

	for( scan = -1, nr = zone_index_info.used ; ; nr++ )
	{
		fgets( buf, 299, index );

		oldscan = scan;

		if( sscanf( buf, " %d.zone ", &scan ) != 1 ) FATAL( "boot_zones> index file corrupted." );

		if( scan <= oldscan ) FATAL( "boot_zones> index file corrupted. #%d -> #%d", oldscan, scan );

		if( scan == LAST_ZONE_INDEX ) break;

		boot_a_zone( nr, scan );
		
		if( nr >= zone_index_info.max ) 
							FATAL( "boot_world> zone table overflows." );

		zone_index_info.used++;
		zones_in_world++;
	}
	fclose( index );

	log( "\t   obj = %d - %d",  obj_index_info.used,  obj_index_info.max );
	log( "\t   mob = %d - %d",  mob_index_info.used,  mob_index_info.max );
	log( "\t world = %d - %d", room_index_info.used, room_index_info.max );
	log( "\t  zone = %d - %d", zone_index_info.used, zone_index_info.max );
	log( "" );
    log( "\tReindexing world." );			reindex_world();

    room_temple     = real_roomNr( ROOM_TEMPLE );
    room_meditation = real_roomNr( ROOM_MEDITATION );
    room_library    = real_roomNr( ROOM_LIBRARY );
    room_audience   = real_roomNr( ROOM_AUDIENCE );

	log( "\tReindexing zones." ); 			index_rooms();
	log( "\tRebuild reset commands" ); 		rebuild_reset_commands();
	log( "\tAssign shopkeepers." );			assign_the_shopkeepers();
}

void do_saveworld( charType * ch, char * argu, int cmd )
{
	if( !IMPL( ch ) ) return;

	save_world();

	sendf( ch, "Done." );
}

void do_savezone( charType * ch, char * argu, int cmd )
{
	int		nr;

	if( !IMPL( ch ) ) return;

	if( !isnumstr( argu ) )
	{
		sendf( ch, "Need a number of zone." ); return;
	}
	
	if( getnumber( argu, &nr ) < 0 || nr >= zone_index_info.used )
	{
		sendf( ch, "[1-%d]. %d is out of range.", zone_index_info.used - 1, nr );
		return;
	}

	save_a_zone( nr );

	sendf( ch, "Saving %s. Done", zones[nr].name );
}

static void list_zone( charType * ch, zoneType * zone, int nr, pageUnitType * pu, int brief )
{
	char		* ap, * gp;

	if( nr <= 0 || nr >= zones_in_world ) return;

	ap = zone_aligns[zone->align];
	gp = zone_aggressives[ zone->aggressive ];

	if( !brief )
	{
		if( IMPL(ch) )
		{
		pagef( pu, "%2d] %30s ==> %s & %s\n"
			   "     Reset %3d(%3d) mins with %3d rooms, zone for %2d ~ %2d levelers.\n"
			   "     Path to zone = %s\n"
			   "     Infomations  = %s\n"
			   "     Index        = [%d - %d - %d]",
				nr, zone->name, ap, gp, 
                zone->lifespan, zone->age, zone->rooms_in_zone, zone->low, zone->high,
				zone->path ? zone->path : "", zone->info ? zone->info : "None.",
				zone->bottom, zone->top, zone->max );
		}
		else
		{
		pagef( pu, "%2d] %30s ==> %s & %s\n"
			   "     Reset %3d mins with %3d rooms, zone for %2d ~ %2d levelers.\n"
			   "     Path to zone = %s\n"
			   "     Infomations  = %s" ,
				nr, zone->name, ap, gp, zone->lifespan, zone->rooms_in_zone, zone->low, zone->high,
				zone->path ? zone->path : "", zone->info ? zone->info : "None." );
		}
	}
	else
	pagef( pu, "%2d] %30s - %s - %s - [%2d~%2d]", nr, zone->name, ap, gp, zone->low, zone->high );
}	

void do_zone( charType * ch, char * argument, int cmd )
{
	int					brief = 0, i, lw = 0, hi, found = 0;
	pageUnitType	*	pu;
	char				buf[256];

	if( IS_NPC(ch) ) return;
	argument = onefword( argument, buf );

	pu = new_pageunit( ch );

	if( !buf[0] )
	{
		lw = 1; hi = zones_in_world - 1; brief = 1;
	}
	else
	{
		if( getnumber( buf, &lw ) > 0 )
		{
			argument = onefword( argument, buf );

			lw = lw == 0 ? 1 : lw;

			if( getnumber( buf, &hi ) <= 0 ) 
			{
				hi = lw; brief = 0;
			}
			else
			{
				brief = 1;
				if( hi < lw ) 
				{
					 i = lw; lw = hi; hi = i;
				}
			}
		}
		else
		{
			for( found = 0, i = 1; i < zones_in_world; i++ )
			{
				if( isoneofp( buf, zones[i].name ) )
				{
					list_zone( ch, &zones[i], i, pu, 0 );
					found++;
				}
			}

			if( !found++ )
			{
				sendf( ch, "I can not found %s zone.", buf );
			}
		}
	}

	if( lw )
	{
		for( found = 0, i = lw; i <= hi; i++ )
		{
			list_zone( ch, &zones[i], i, pu, brief );
			found++;
		}
	}
	if( !found ) sendf( ch, "What?" );
	show_page( ch, pu );
}
