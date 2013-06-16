#include <stdio.h>
#include <string.h>
#include <time.h>

#include "defines.h"
#include "character.h"

void main( int argc, char ** args )
{
    storecharType   dummy, new;
    FILE        * load, * save, * read;
    int         i, saved = 1;
	char		name[128];

	if( argc != 2 )
	{
		printf( "syntax> %s name\n", args[0] );
		return;
	}

	strcpy( name, args[1] );

    load = fopen( "players", "r" );
    save = fopen( "in.player", "w" );

    if( !load || !save )
    {
        perror("open files." ); exit( 1 );
    }

    for( i = 0; !feof(load); i++ )
    {
        fread( &dummy, sizeof(dummy), 1, load );

        if( dummy.name[0] )
        if( strcmp( dummy.name, name ) == 0 )
        {
			printf( "%s found at %d\n", name, i );
            fwrite( &dummy, sizeof(dummy), 1, save );
        }
    }
    fclose( load );
    fclose( save );
}
