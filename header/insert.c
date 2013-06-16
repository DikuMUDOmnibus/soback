#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "defines.h"
#include "character.h"

struct player_index     *   player_table;
int             top_of_p_file, top_of_p_table = -1;

int find_name( char * name )
{
    int i;

    for( i = 0; i <= top_of_p_table; i++ )
    {
		if( player_table[i].name && player_table[i].name[0] )
        	if( strcmp( player_table[i].name, name) == 0 ) 
			{
				printf( "%s found at %d.\n", name, i );
				return(i);
			}
    }
    return(-1); 
}

void build_player_index( void )   
{
    int                 nr =    -1;
    storecharType       dummy;
    FILE            *   fp;
    
    if( fp = fopen( "players", "rb+"), !fp )
    {
        fprintf( stderr, "build player index> can't find players file" );
    }
                                                  
    for(; !feof(fp);)
    {
        fread( &dummy, sizeof(dummy), 1, fp );    
        if( !feof(fp) )
        {
            if( nr == -1 )
            {
                player_table = calloc( sizeof(struct player_index), 1 );
                nr = 0;
            }
            else
            {
                player_table = (struct player_index *)
                        realloc(player_table, (++nr + 1) * sizeof(struct player_index));
            }

            player_table[nr].nr = nr;
            player_table[nr].name = malloc(strlen(dummy.name)+1) ;

            strcpy( player_table[nr].name, dummy.name );
        }
    }
    fclose(fp);

    top_of_p_table = nr;
    top_of_p_file = top_of_p_table;

	printf( "%d players loaded.\n", nr );
}

void cutout( char * name )
{
    storecharType   dummy, new;
    FILE        * 	load, * save, * read;
    int         	nr, i, saved = 1;

	if( (nr = find_name( name )) == -1 )
	{
		fprintf( stderr, "Can't find %s in players.\n", name );
		exit(1);
	}

    load = fopen( "players", "r" );
    save = fopen( "in.player", "w" );

    if( !save )
    {
        perror("Opening in.player." ); exit( 1 );
    }

	fseek( load, (long)sizeof(dummy) * nr, 0 );
    fread( &dummy, sizeof(dummy), 1, load );

    if( strcmp( dummy.name, name ) == 0 )
    {
		printf( "%s found at %d\n", name, nr );
        fwrite( &dummy, sizeof(dummy), 1, save );
    }
    fclose( load );
    fclose( save );
}

void insert( char * name )
{
    storecharType       dummy, new;
    FILE                * load, * read;
    int                 nr, i, saved = 0;

    load = fopen( "players", "r+" );
    read = fopen( "in.player", "r" );

    if( !load || !read )
    {
        perror("open files." ); exit( 1 );
    }

    fread( &new, sizeof( new ), 1, read );

	if( (nr = find_name( name )) == -1 )
	{
/*
		fprintf( stderr, "%s is not found in players.\n", name );

    	for( i = 0; !feof(load); i++ )
    	{
        	fread( &dummy, sizeof(dummy), 1, load );

        	if( !saved && dummy.name[0] == 0 )
        	{
				printf( "inserted at %d\n", i );
            	dummy = new;
            	saved = 1;
				fseek( load, (long)i * sizeof( storecharType ), 0 );
        		fwrite( &new, sizeof(new), 1, load );
				break;
        	}
    	}
    	if( !saved )
	*/
    	{
			printf( "appended\n" );
			fseek( load, 0, SEEK_END );
        	fwrite( &new, sizeof( new ), 1, load );
    	}
	}
	else
	{
		fprintf( stderr, "%s is inserted in players at %d.\n", name, nr );

		fseek( load, (long)nr * sizeof( storecharType ), 0 );
		fwrite( &dummy, sizeof(dummy), 1, load );
	}

    fclose( load );
    fclose( read );          
}

void main( int argc, char ** argv )
{
	if( argc != 3 )
	{
		printf( "syntex> [x|i] name\nex> i Peda" );
		return;
	}

	build_player_index();

	if( argv[1][0] == 'i' ) insert( argv[2] );
	else if( argv[1][0] == 'x' ) cutout( argv[2] );
	else
	{
		printf( "syntex> [x|i] name\nex> i Peda" );
		return;
	}
}
