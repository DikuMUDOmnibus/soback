#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "world.h"
#include "utils.h"
#include "stash.h"
#include "find.h"
#include "strings.h"
#include "transfer.h"
#include "affect.h"

/*
void move_stash( char * victim )
{
  	char 	sf1[100], sf2[100], name[16];    
  	int 	i;

  	strcpy( name, victim );
  	for( i = 0; name[i]; ++i )
    	if( isupper( name[i] ) ) name[i] = tolower( name[i] );      

  	sprintf( sf1, "%s/%c/%s.x",   STASH_DIR, name[0], name );
  	sprintf( sf2, "%s/%c/%s.x.y", STASH_DIR, name[0], name );

  	rename( sf1, sf2 );
}
*/

void wipe_stash( char * filename, int remake )
{
  	char stashfile[100], name[MAX_NAME_LENGTH+1];       
  	FILE * fp;
  	int i;

    for( i = 0; filename[i]; i++ ) name[i] = tolower( filename[i] );

  	name[i] = 0;

  	sprintf( stashfile, "%s/%c/%s.x.y", STASH_DIR, name[0], name );
  	unlink( stashfile );

  	sprintf( stashfile, "%s/%c/%s.x", STASH_DIR, name[0], name );
  	unlink( stashfile );

  	if( remake && (fp = errOpen( stashfile, "w" ), fp) )       
	{
    	fprintf( fp, "#Start %s\n#End %s\n", filename, filename );   
		fclose( fp );
	}
}

int is_stashable( objectType * obj )
{
	if( obj->status <= 0 ) return 0;
	if( obj->type == ITEM_KEY ) return 0;
	if( obj->nr   == OBJECT_CORPSE ) return 0;
	if( IS_SET( obj->extra , ITEM_NO_RENT ) ) return 0;
	return 1;
}

void stash_item( FILE * fp, objectType * obj )
{
	int j;

	fprintf( fp, "%d ", objects[obj->nr].virtual );

    for( j = 0; j < 4; ++j )
		fprintf( fp, "%d ", obj->value[j] );

	for( j = 0; j < MAX_APPLY; j++ )
		fprintf( fp," %d %d", (int)obj->apply[j].location, (int)obj->apply[j].modifier );

	fprintf( fp, " %d %d %d\n", obj->status, obj->limit, obj->timer );
}

struct objpointer
{
	objectType   * ptr;
	struct objpointer * next;
};

void stash_contents( FILE * fp, objectType * objs, int mode )
{
	struct objpointer head, * next;
	objectType * curr;

	head.next = 0;

	for( curr = objs; curr; curr = curr->next_content )
	{
		next = errMalloc( sizeof( struct objpointer ) );

		next->ptr = curr;
		next->next = head.next;
		head.next = next;
	}

	for( next = head.next; next; next = next->next )
	{
		curr = next->ptr;

		if( !is_stashable( curr ) )
		{
			if( curr->contains ) stash_contents( fp, curr->contains, 2 ); 
		}
		else
		{
			if( curr->contains ) fprintf( fp, "C " );
			else				 fprintf( fp, "I " );

			stash_item( fp, curr );

			if( curr->contains ) stash_contents( fp, curr->contains, 0 );
		}
	}
	if( mode == 1 ) fprintf( fp, "End\n" );
	else if ( mode == 0 ) fprintf( fp, "E\n" );

	for( ; head.next; head.next = next )
	{
		next = head.next->next;

		errFree( head.next );
	}
}

void stash_equip( FILE * fp, charType * ch )
{
	int		i;
	objectType * curr;

  	for( i = 0; i < MAX_WEAR; ++i )
    	if( (curr = ch->equipment[i]) ) 
		{
			if( !is_stashable( curr ) )
			{
				if( curr->contains ) stash_contents( fp, curr->contains, 2 ); 
			}
			else
			{
				fprintf( fp, "Q %d ", i );
				if( curr->contains ) fprintf( fp, "C " );
				else	 			 fprintf( fp, "I " );

				stash_item( fp, curr );

				if( curr->contains ) stash_contents( fp, curr->contains, 0 );
			}
		}
}

int stash_char( charType * ch, char * filename )
{
  	char stashfile[100], name[MAX_NAME_LENGTH + 1];
  	FILE *fp;
  	int i;
    
	if( IS_NPC( ch ) )
	{
		DEBUG( "stash_char> Trying to stash NPC." ); 
		return -2;
	}

  	strcpy(name, filename ? filename : GET_NAME(ch));

 	for( i = 0; name[i]; ++i )
    	if( isupper( name[i] ) )
       		name[i] = tolower( name[i] );

  	sprintf(stashfile,"%s/%c/%s.x",STASH_DIR,name[0],name);

  	if( !(fp = errOpen( stashfile, "w" )) ) return -1;

  	fprintf( fp, "#Start %s\n", GET_NAME( ch ) );

   	stash_contents( fp, ch->carrying, 1 );  

	stash_equip( fp, ch );

  	fprintf( fp, "#End %s\n", GET_NAME( ch ) );
  	fclose( fp );

    return 0; 
}

static int unstash_error = 0;

objectType * unstash_item( const char * str, int in_world )
{
	int					nr, vir = -1; 
	int					ov1, ov2, ov3, ov0; 
	int					af0, df0, af1, df1, af2, df2, af3, df3;
	int					status, limit, timer;
	objectType 		* 	obj;

	nr = sscanf( str, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
					 &vir, &ov0, &ov1, &ov2, &ov3,
					 &af0, &df0, &af1, &df1, &af2, &df2, &af3, &df3,
					 &status, &limit, &timer );

	if( nr == 0 || (nr != 9 && nr != 12 && nr != 16 ) )
	{
		DEBUG( "unstash_item> Stash field(%d:%d) doesn't 9 , 12 or 16.", 
						vir, nr );
		unstash_error = 1;
		return NULL;
	}

	if( nr != 16 )
	{
		status = af2; limit = df2; timer = af3;
	}

	if( real_objectNr( vir ) == OBJECT_NULL ) return 0;

	if( obj = load_a_object( vir, VIRTUAL, in_world ), obj == NULL ) 
	{ 
		DEBUG( "unstash_item> No such item %d", vir );
		unstash_error = 1;
		return NULL;
	}

	if(   obj->type == ITEM_DRINKCON 
       || obj->type == ITEM_WAND
	   || obj->type == ITEM_STAFF )
	{
		obj->value[0] = ov0; 	 obj->value[1] = ov1;
		obj->value[2] = ov2; 	 obj->value[3] = ov3;

		if( obj->type == ITEM_DRINKCON )
		{
			obj->weight = objects[obj->nr].weight + ov1;
		}
	}
	else if( obj->type == ITEM_LIGHT )
		obj->value[2] = ov2;
	else if( obj->type == ITEM_FIREWEAPON )
		obj->value[0] = ov0;

	if(	!IS_SET(obj->extra, ITEM_MAGIC)
        && obj->apply[0].location == APPLY_NONE )
	{
		obj->apply[0].location = af0; obj->apply[0].modifier = df0;
		obj->apply[1].location = af1; obj->apply[1].modifier = df1;

		if( nr == 16 )
		{
			obj->apply[2].location = af2; obj->apply[2].modifier = df2;
			obj->apply[3].location = af3; obj->apply[3].modifier = df3;
		}
	}

	if( limit == 0 )  limit  = 100;
	if( status == 0 ) status = 100;

	obj->limit  = limit;
	obj->status = status;

	if( nr == 12 ) 	obj->timer = timer;
	else			obj->timer = MAX_OBJ_LIFE_SPAN;

	return obj;
}

void unstash_contents( FILE * fp, objectType * obj, int in_world )
{
	objectType * curr;
	char	* cp, buf[1024];

	for( curr = 0;; curr = 0 )
	{
		if( cp = fgets( buf, 1024, fp ), cp <= 0 ) return;

		switch( *cp++ )
		{
			case '\0' : DEBUG( "unstash_contents> file corrupted." );
						return;
			case 'I'  : curr = unstash_item( cp, in_world ); break; 
			case 'C'  : curr = unstash_item( cp, in_world ); 
						if( curr ) unstash_contents( fp, curr, in_world ); break;
			case 'E'  : return;
			case '#'  :
			case 'Q'  : 
			default	  : 
						DEBUG( "unstash_contents> Unkown stash line.(%s)."
								, --cp );
						return;
		}
		if( curr ) obj_to_obj( curr, obj );
	}
}

void unstash_equip( FILE * fp, charType * ch, const char * str )
{
	objectType * obj;
	int		pos, nr;
	char	type;
	const char * cp;

	nr = sscanf( str, "%d %c ", &pos, &type );

	if( nr != 2 )
	{
		DEBUG( "unstash_equip> Equip-pos unknown. (%s - %s).",
					   GET_NAME( ch ), str );
		unstash_error = 1;
		return;
	}

	if( type != 'I' && type != 'C' )
	{
		DEBUG( "unstash_equip> Equip-type unknown. (%s - %s).",
					   GET_NAME( ch ), str );
		unstash_error = 1;
		return;
	}

	for( cp = str; *cp != 'I' && *cp != 'C'; cp++ );
	cp++;

	if( obj = unstash_item( cp, 1 ), !obj ) return;

	if( type == 'C' ) unstash_contents( fp, obj, 1 );

	equip_char( ch, obj, pos );
}

void unstash_char( charType * ch, char * filename )
{
	objectType 	* 	obj = 0;
	FILE 		*	fp;
	char			stashfile[100], name[100];
	char		*	cp, *np , buf[1024], line[1024];
	int				i, isX = 1;

	if( ch == NULL )
	{
		DEBUG( "unstash_char> NULL pointer entered" );
		return;
	}

	strcpy( name, filename ? filename : GET_NAME( ch ) );
	
    for( i = 0; name[i]; ++i ) name[i] = tolower( name[i] );

	sprintf( stashfile, "%s/%c/%s.x", STASH_DIR, name[0], name );

	if( fp = fopen( stashfile, "r" ), !fp )
	{
unstash_y :

		isX = 0;
		strcat( stashfile, ".y" );

		if( fp = errOpen( stashfile, "r" ), !fp )
		{
			ERROR( "unstash_char> can't open %s's stash file.(%s).", GET_NAME(ch), stashfile );
			return;
		}
	}

	cp = fgets( line, 1024 - 1, fp );

	if( cp > 0 && strncmp( cp, "#Start", 6 ) == 0 ) 
	{
		np  = cp + 7;		

		for( *cp = 0; *cp != '#' ; obj = 0 )
		{
			if( cp = fgets( buf, 1024-1, fp ), cp <= 0 ) break;

			switch( *cp )
			{
				case 'I' : obj = unstash_item( ++cp, 1 ); 			break;
				case 'Q' : unstash_equip( fp, ch, ++cp ); break;
				case 'C' : obj = unstash_item( ++cp, 1 ); 	
						   unstash_contents( fp, obj, 1 ); break;
				case 'E' : 
				case '#' : break;
				default  :
					DEBUG( "unstash_char> Unkown item type.(%s)", cp );
					*cp = '#';
			}

			if( obj ) obj_to_char( obj, ch );
		}
		if( unstash_error )	DEBUG( "unstash_char> player = %s, file = %s has error.", name, np );
	}
	else
	{
		if( isX ) 
		{
			errClose( fp ); goto unstash_y;
		}

		DEBUG( "unstash_char> %s's stash files are all corrupted.",	GET_NAME( ch ) );
	}

	unstash_error = 0;
	errClose(fp);
}

void unstash_off_world( char * rname, objectType ** list )
{
	objectType 	* 	obj = 0;
	FILE 		*	fp;
	char			stashfile[256];
	char		*	cp, *np , buf[1024], line[1024];

  	if( !rname )
  	{
  		ERROR( "unstash_off_world> Null pointer entered for room name." );
  		return;
  	}

	sprintf( stashfile, "%s/%s", ROOM_STASH_DIR, rname );

	if( fp = fopen( stashfile, "r" ), !fp )
	{
		log( "unstash_off_world> can't open %s's stash file.(%s).", rname, stashfile );
		return;
	}

	cp = fgets( line, 1024 - 1, fp );

	if( cp > 0 && strncmp( cp, "#Start", 6 ) == 0 ) 
	{
		np  = cp + 7;

		for( *cp = 0; *cp != '#' ; obj = 0 )
		{
			if( cp = fgets( buf, 1024-1, fp ), cp <= 0 ) break;

			switch( *cp )
			{
				case 'I' : obj = unstash_item( ++cp, 0 ); 			break;
				case 'C' : obj = unstash_item( ++cp, 0 ); 	
						   unstash_contents( fp, obj, 0  ); break;
				case 'E' : 
				case '#' : break;
				default  :
					DEBUG( "unstash_char> Unkown item type.(%s)", cp );
					*cp = '#';
			}

			if( obj ) 
			{
				obj->next_content = *list;
				*list = obj;
			}
		}
		if( unstash_error )	
			DEBUG( "unstash_off_world> room = %s., file = %s has error.", rname );
	}
	else
		DEBUG( "unstash_off_world> %s's stash files are corrupted.", rname );

	unstash_error = 0;
	errClose(fp);
	return;
}

void stash_off_world( char * rname, objectType * list )
{
  	char 			stashfile[256];
  	FILE 		*	fp;

  	if( !rname )
  	{
  		ERROR( "stash_off_world> Null pointer entered for room name." );
  		return;
  	}
    
  	sprintf( stashfile,"%s/%s", ROOM_STASH_DIR, rname );

  	if( !(fp = errOpen( stashfile, "w" )) ) return;

  	fprintf( fp, "#Start %s.\n", rname );

   	stash_contents( fp, list, 1 );  

  	fprintf( fp, "#End %s\n", rname );

  	fclose( fp );
}
