#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "character.h"
#include "object.h"
#include "mobile.h"
#include "world.h"
#include "strings.h"
#include "interpreter.h"
#include "comm.h"
#include "find.h"
#include "transfer.h"
#include "variables.h"
#include "limits.h"
#include "spec.rooms.h"

#define		MAX_INSURE 	40

static roomType *	insure_room;
static charType *	insure_keeper;
static charType *	insure_kid;
static int			insurance_count;
static int			objs [MAX_INSURE];
static char		*	owner[MAX_INSURE];

static void load_insurance( void )
{
	char			fname[200];
	char			buf[200];
	FILE		*	fp;
	int				i, nr;
	
    sprintf( fname, "%s/insurance", ROOM_STASH_DIR );

    if( fp = errOpen( fname, "r" ), !fp )
    {
        creat( fname, S_IRUSR | S_IWUSR );
        if( fp = errOpen( fname, "w+" ), !fp )
        {
            FATAL( "load_bank> failed to create new %s.", fname );
        }
        else 
        {
            fprintf( fp, "#0 insurance\n" );
            log( "load_insurance> new %s file created.", fname );
            fclose( fp );
        }
        return;
    }

	nr = fscanf( fp, "#%d insurance\n", &insurance_count );
        
    if( nr != 1 )
    {       
        ERROR( "load_insurance> insurance file corrupted." );
		insurance_count = 0;
        errClose( fp );
        return;
    }
    
	for( i = 0; i < insurance_count; i++ )
	{
		fscanf( fp, "#%d %d %s\n", &nr, &objs[i], buf ); 

		if( nr != i+1 )
		{
        	ERROR( "load_insurance> insurance file corrupted." );
        	errClose( fp );
			insurance_count = i;
        	return;
    	}   
		owner[i] = strdup( buf );
	}
	errClose( fp );
}

static void save_insurance( void )
{
	int				i;
	FILE		*	fp;
	char			fname[200];

	sprintf( fname, "%s/insurance", ROOM_STASH_DIR );

	if( fp = errOpen( fname, "w" ), !fp )
	{
		ERROR( "save_insurance> file open failed." );
		return;
	}

	fprintf( fp, "#%d insurance\n", insurance_count );

	for( i = 0; i < insurance_count; i++ )
	{
		fprintf( fp, "#%d %d %s\n", i+1, objs[i], owner[i] );
	}
	errClose( fp );
}

static void add_insurance( charType * ch, objectType * obj )
{
	objs [insurance_count] = obj->virtual;
	owner[insurance_count] = strdup( ch->name );

	insurance_count++;
}

static void del_insurance( int nr )
{
	int		i;

	errFree( owner[nr] );

	for( i = nr; i < MAX_INSURE - 2; i++ )
	{
		objs [i] = objs [i + 1];
		owner[i] = owner[i + 1];		
	}

	objs [MAX_INSURE -1] = 0;
	owner[MAX_INSURE -1] = 0;

	insurance_count--;
}

static int init_keeper( void )
{
	charType	*	tmp;

	insure_keeper = 0;
	insure_kid    = 0;

	for( tmp = insure_room->people; tmp; tmp = tmp->next_in_room )
	{
		if( IS_NPC( tmp ) )
		{
			if( mobiles[tmp->nr].virtual == MOBILE_INSURANCE ) insure_keeper = tmp;
			if( mobiles[tmp->nr].virtual == MOBILE_INS_KID   ) insure_kid    = tmp;

			if( insure_keeper && insure_kid ) return 1;
		}
	}

	if( insure_keeper )
		do_shout( insure_keeper, "Arrrah~~ Where is my boy??? Come back!", COM_SHOUT );

	return 0;
}

static void send_kid_to_player( charType * ch )
{
	char		tmp[100];
	int			i;

	for( i = 0; i < 6; i++ )
	{
		if( can_go( insure_kid, i ) ) break;
	}

	if( i < 6 )
	{
    	sprintf( tmp, "$n leaves %s.", dirs[ i ] ); 
		act(tmp, TRUE, ch, 0,0,TO_ROOM);
	}
	else
	{
		act( "$n disapears a puff of smoke.", TRUE, insure_kid, 0, 0, TO_ROOM );
	}
	char_from_room( insure_kid );
	char_to_room( insure_kid, ch->in_room );

   	act( "$n has arrived.", TRUE, insure_kid, 0, 0, TO_ROOM);
}

static int return_kid( charType * ch, int cmd, char * argu )
{
	char		tmp[100];
	int			i;

	if( cmd ) return 0;

	if( ch->in_room == insure_room->nr ) return 0;

	for( i = 0; i < 6; i++ )
	{
		if( can_go( ch, i ) ) break;
	}

	if( i < 6 )
	{
    	sprintf( tmp, "$n leaves %s.", dirs[ i ] ); 
		act( tmp, TRUE, ch, 0,0,TO_ROOM);
	}
	else
	{
		act( "$n disapears a puff of smoke.", TRUE, ch, 0, 0, TO_ROOM );
	}
	char_from_room( ch );
	char_to_room( ch, insure_room->nr );

   	act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
   	return 1;
}

static int look_insurance( charType * ch, int cmd, char * argu, int tele )
{
	char		buf[MAX_INPUT_LENGTH+1];
    int     	i, nr, found = 0;

	if( IS_NPC( ch ) ) return 0;

	if( !*argu ) return 0;

	oneArgument( argu, buf );

	if( !isoneof( buf, "board" ) ) return 0;

	sendf( ch, "---------- Board of Insurance status ---------" );
    for( i = 0; i < insurance_count; i++ )
    {
        if( objs[i] && owner[i] )
        {
			if( nr = real_objectNr( objs[i] ), nr == OBJECT_NULL )
			{
				DEBUG( "list_insurance> #%d object not found.", objs[i] );
				del_insurance( i );
				i--;
				continue;
			}
            sendf( ch, "%2d] %25s - %s", i+1, objects[nr].wornd, owner[i] );
            found++;
        }
    }

	if( !found ) sendf( ch, "None." );
	if( !tele )
		act( "$n looks the insurance board.", TRUE, ch, 0, 0, TO_ROOM );

	return 1;
}

static void insure_insurance( charType * ch, int cmd, char * argu, int tele )
{
	objectType		*	obj;
	char				item[MAX_INPUT_LENGTH+1];
	char				card[MAX_INPUT_LENGTH+1];

	if( IS_NPC( ch ) ) return;
	if( !init_keeper() ) return;
	
	if( !*argu )
	{
		sendf( ch, "%s tells you, 'Insure what?'", insure_keeper->moved );
		return;
	}

	if( insurance_count >= MAX_INSURE )
	{
		sendf( ch, "%s tells you, 'Sorry, I can't accept more insurance.'", insure_keeper->moved );
		return;
	}

	twoArgument( argu, item, card );

	if( obj = find_obj_inven( ch, item ), !obj )
	{
		sendf( ch, "%s tells you, 'You don't have any such item.'", insure_keeper->moved );
		return;
	}

	if( obj->type != ITEM_KEY || obj->contains )
	{
		sendf( ch, "%s tells you, 'I do not take that.'", insure_keeper->moved );
		return;
	}

	if( !cost_from_player( ch, insure_keeper, card, 10000, 0 ) ) return;

	obj_from_char( obj );	
	add_insurance( ch, obj );

	if( tele )
	{
		send_kid_to_player( ch );
		act( "$n tries to steal $p from $N! Bingo~.", FALSE, insure_kid, obj, ch, TO_NOTVICT);
		act( "$n robbed you!! Ouch~.", FALSE, insure_kid, 0, ch, TO_VICT);
	}
	extract_obj( obj, 1 );
	save_insurance();
}

static void retrieve_insurance( charType * ch, int cmd, char * argu, int tele )
{
	objectType	*	obj;
	int				nr;
	char			num [MAX_INPUT_LENGTH+1];
	char			card[MAX_INPUT_LENGTH+1];

	if( IS_NPC( ch ) ) return;

	if( !init_keeper() ) return;

	if( !*argu )
	{
		sendf( ch, "%s tells you, 'retrieve what?'", insure_keeper->moved );
		return;
	}

	twoArgument( argu, num, card );

	if( getnumber( num, &nr ) <= 0 || --nr >= (MAX_INSURE) || objs[nr] == 0 )
	{
		sendf( ch, "%s tells you, 'Check your insurance first!'", insure_keeper->moved );
		return;
	}

	if( stricmp( owner[nr], ch->name ) != 0 )
	{
		sendf( ch, "%s tells you, 'It's not yours!'", insure_keeper->moved );
		return;
	}

	if( !cost_from_player( ch, insure_keeper, card, 20000, 0 ) ) return;

	if( real_objectNr( objs[nr] ) == OBJECT_NULL )
	{
		DEBUG( "retrieve_insurance> can't not locate #%d object.", objs[nr] );
		return;
	}

	obj = load_a_object( objs[nr], 1, 1 );
	obj_to_char( obj, ch );

	if( tele )
	{
		send_kid_to_player( ch );
		act( "$n politely gives $N $p." , FALSE, insure_kid, obj, ch, TO_NOTVICT);
		act( "$n politely gives you $p.", FALSE, insure_kid, obj, ch, TO_VICT);
	}

	del_insurance( nr );
	save_insurance();
}

void do_insurance( charType * ch, char * argu, int cmd )
{
    int     i, nr, found = 0;
    
	if( IS_NPC( ch ) ) return;

	sendf( ch, "---------- Insurance status ------------" );
    for( i = 0; i < insurance_count; i++ )
    {       
        if( objs[i] && owner[i] && ( OMNI(ch) || stricmp( owner[i], ch->name ) == 0 ) )
        {
            if( nr = real_objectNr( objs[i] ), nr == OBJECT_NULL )
            {
                DEBUG( "list_insurance> #%d object not found.", objs[i] );
                del_insurance( i );
                i--;
                continue;
            }
            
            sendf( ch, "%2d] %s", i + 1, objects[nr].wornd );
            found++;
        }   
    }
	if( !found ) sendf( ch, "No Insurance." );
}

void do_insure( charType * ch, char * argu, int cmd )
{
	insure_insurance( ch, cmd, argu, 1 );
}

void do_retrieve( charType * ch, char * argu, int cmd )
{
	retrieve_insurance( ch, cmd, argu, 1 );
}

static int insurance( charType * ch, int cmd, char * argu )
{
	if( IS_NPC( ch ) ) return 0;
	switch( cmd )
	{
		case	COM_LOOK     : return look_insurance( ch, cmd, argu, 0 );
		case	COM_INSURE   : insure_insurance  ( ch, cmd, argu, 0 ); return 1;
		case	COM_RETRIEVE : retrieve_insurance( ch, cmd, argu, 0 ); return 1;
	}
	return 0;
}

void init_insurance( void )
{
	int			keeper, kid, room;

	keeper = real_mobileNr( MOBILE_INSURANCE );
	kid    = real_mobileNr( MOBILE_INS_KID );
	room   = real_roomNr( ROOM_INSURANCE );

	if( keeper == MOBILE_NULL || kid == MOBILE_NULL || room == NOWHERE ) return;

	insure_room = &world[room];

	load_insurance();

	mobiles[keeper].func = insurance;
	mobiles[kid].func    = return_kid;
}

void clear_insurance( void )
{
	insurance_count = 0;

	save_insurance();
}
