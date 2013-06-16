/******************************************************************************
* board.c ( board routine of KIT Mud a.k.a. jinsil mud )                      *
* board.c is coded by Kim Jiho ( Nara of Baram )                              *
******************************************************************************/
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "utils.h"
#include "find.h"
#include "comm.h"
#include "interpreter.h"
#include "misc.h"
#include "variables.h"
#include "strings.h"
#include "transfer.h"
#include "page.h"

#define MAX_MSGS 200 
#define ONE_SHOW 20
#define PAPER_DB_NUM	3035

struct board_data 
{
    char * head[MAX_MSGS];  	/* head of board */
    char * msgs[MAX_MSGS];  	/* msg of board */
    char * writer[MAX_MSGS]; 	/* writer of that mesg */
	char * created[MAX_MSGS];
    int    num;  				/* msg number that board contains */
    int    room_num;  			/* room number that board is in */
    char   bfile[50];  			/* board file name */
    struct board_data *next;  	/* next */
};

struct board_data *boards = 0;

void load_board( struct board_data * );

/* init one board and return its pointer */
struct board_data *init_a_board(charType *ch)
{
    struct board_data *cr_board;

    cr_board = (struct board_data *)errCalloc( sizeof( struct board_data ) );
    if( cr_board )
	{
        cr_board->room_num = world[ch->in_room].virtual;
        sprintf( cr_board->bfile, "%s/%d.board", BOARD_DIR, cr_board->room_num );

        load_board(cr_board);

        cr_board->next = boards;
        boards = cr_board;
    }
    return cr_board;
}

struct board_data *find_board(charType *ch)
{
    struct board_data *tmp_board;
    int ch_rnum;

    ch_rnum = world[ch->in_room].virtual;
    if( boards ){
        for( tmp_board = boards; tmp_board; tmp_board = tmp_board->next ){
            if( tmp_board->room_num == ch_rnum ){
                return tmp_board;
            }
        }
    }
    tmp_board = init_a_board(ch);
    return tmp_board;
}

void load_board(struct board_data *cb )
{
	FILE *  fp, * mfp ;
    int 	i, num, size;
	char	buf[100];

    if( fp = errOpen( cb->bfile, "r+" ), !fp )
	{
		creat( cb->bfile, S_IRUSR | S_IWUSR );
        if( fp = errOpen( cb->bfile, "w+" ), !fp )
		{
			ERROR( "load_board> failed to create new %s.", cb->bfile );
		}
		else
		{
			fprintf( fp, "#0 total\n" );
			log( "load_board> new %s file created.", cb->bfile );
			fclose( fp );
		}
		return;
    }

	fscanf( fp, "#%d %s\n", &num, buf ); 

    if( num < 0 || num > MAX_MSGS ) 
	{
        log("load_board> board msg file(%s) corrupted.", cb->bfile );
        fclose(fp);
        return;
    }

    for( cb->num = num, i = 0; i < cb->num; i++ )
	{
        if( fscanf( fp, "#%d\n", &num ) != 1 || num != i )
        {
            ERROR( "load_board> Message sequence does not match." );
			cb->num = i; break;
            break;
        }
		cb->head[i] = 		remove_newline( strdup( fgets( buf, 80, fp )));
		cb->writer[i] = 	remove_newline( strdup( fgets( buf, 80, fp )));
		cb->created[i] = 	remove_newline( strdup( fgets( buf, 80, fp )));

        sprintf( buf, "%s/%d.%.3d.board", BOARD_DIR, cb->room_num, i );

        if( mfp = errOpen( buf, "r+" ), !mfp ) 
		{
			cb->num = i; break;
		}

        fseek( mfp, 0, SEEK_END );    
        size = (int)ftell( mfp );     
        rewind( mfp );

        cb->msgs[i] = (char *)errMalloc( size+1 );

        if( !cb->msgs[i] )
        {
            ERROR( "load_board> malloc failure of (%d) size.", size+1 );
            break;
        }
        fread( cb->msgs[i], size, 1, mfp );
        cb->msgs[i][size] = 0;      
        errClose( mfp );
    }

	log( "load_board> loaded %d messages on board at %d.", cb->num, cb->room_num );
    fclose( fp );

    return;
}

int show_board( charType *ch, struct board_data *cb, char *arg )
{
    int i, j;
    char buf[MAX_STRING_LENGTH], num[MAX_INPUT_LENGTH], page[MAX_INPUT_LENGTH];

    halfchop(arg, page, num );
    if( !*page || !isoneof(page, "page board bulletin") ) return FALSE;

	if( isdigit(*num ) || *num == '-' ) 
	{
		j = atoi( num );
		if( j <= 0 || j > (( MAX_MSGS / ONE_SHOW ) + 1) )
		{
			send_to_char( "You are trying to see a board does not exist.\n\r", ch );
			return TRUE;
		}
	}
	else j = 1;

	i = (j-1) * ONE_SHOW;

	if( i >= cb->num && i != 0 )
	{
		send_to_char( "That board does not have that much messages.\n\r", ch );
		return 1;
	}

    act("$n looks the board.", TRUE, ch, 0, 0, TO_ROOM);

    sprintf(buf,"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r" 
				"+  Usage> read # / remove # / look [board|page] # / post <note> [header]      +\n\r"
				"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r"
                "+ Total %3d messages are on board.  Page  %d/%d.\n\r",
                cb->num, j, (cb->num / ONE_SHOW) + (cb->num % ONE_SHOW != 0 ) );
	send_to_char( buf, ch );
    if (!cb->num)
	{
        send_to_char( "\n\rThe board is empty.\n\r", ch );
	}
    else 
	{
        for( j = (cb->num - i - 1 ), i = 0; i < 20 && j >= 0; i++, j-- )
		{
            sprintf( buf, "[%-3d] %-12s- %s - %-s\n\r", 
                cb->num - j, cb->writer[j], cb->created[j], cb->head[j] );
    		send_to_char( buf, ch ); 
		}
    }

    return(TRUE);
}

void save_board( struct board_data *cb )
{
	FILE * fp, * mfp;
    int i;
	char	buf[100];

    if( fp = errOpen( cb->bfile, "w" ), !fp )
	{
        ERROR( "save_board> unable to open board file(%s).", cb->bfile );
        return;
    }

	fprintf( fp, "#%d total\n", cb->num );

    for( i = 0; i < cb->num; i++ )
	{
		fprintf( fp, "#%d\n", i );
		fprintf( fp, "%s\n", cb->head[i] );
		fprintf( fp, "%s\n", cb->writer[i] );
		fprintf( fp, "%s\n", cb->created[i] );
   
        sprintf( buf, "%s/%d.%.3d.board", BOARD_DIR, cb->room_num, i );
        if( mfp = errOpen( buf, "w" ), !mfp ) break;
   
        fprintf( mfp, "%s", cb->msgs[i] );
        errClose( mfp );
    }
    errClose(fp);
}

/*  posting message routine.
    this will be called when you post <paper> <header>   */
int post_board(charType *ch, struct board_data *cb, char *arg)
{
    char papername[MAX_INPUT_LENGTH], header[MAX_INPUT_LENGTH],
         buf[MAX_STRING_LENGTH], buf2[100];
    objectType *paper;
	struct tm * tz;
	time_t	t;

    halfchop(arg,papername,header);
    if( !papername || !*papername )
	{
		send_to_char( "Post? Ok. But, What is you r trying to post?\n\r", ch );
		return 1;
	}
	if( !header || !*header )
	{
		send_to_char( "You should specify a header of your message.\n\r", ch );
		return 1;
	}
    paper = find_obj_inven( ch, papername );

    if( !paper )
        sprintf( buf, "You don't see any %s in your inventory.\n\r", papername );
    else if( paper->type != ITEM_NOTE )
        sprintf( buf, "Huh? You are not able to post %s on board.\n\r", papername );
    else if( paper->usedd == 0 )
        sprintf(buf,"%c%s is blank.\n\r", toupper( papername[0]) , papername + 1);
	else if( cb->num >= MAX_MSGS )
		sprintf(buf, "The board is full of messages.\n\r" );
    else
	{
		if( strlen( header ) > 45 ) 
		{
			if( (unsigned char)*( header + 44 ) > 128 ) *(header + 46) = 0; 
			else                                        *(header + 45) = 0;
		}

        cb->head[cb->num] = strdup( header );
        cb->writer[cb->num] = strdup( GET_NAME(ch) );

		t = time(0); tz = localtime( &t );
		sprintf( buf2, "%2d/%.2d/%.2d", tz->tm_year, tz->tm_mon+1, tz->tm_mday );
		cb->created[cb->num] = strdup( buf2 );	

        cb->msgs[cb->num] = paper->usedd;
        paper->usedd = 0;
        extract_obj( paper, 1 );

        cb->num++;
        sprintf(buf, "Done.\n\r" );
    	save_board( cb );
    }
    send_to_char( buf, ch );
    act("$n attaches a messages on board.", TRUE, ch, 0, 0, TO_ROOM);
    return(TRUE);
}

/* routien that remove message from board.
   this is caleed when you type remove #     */
int remove_board(charType *ch, struct board_data *cb, char *arg)
{
    int  i, paper_num;
    char buf[100], number[MAX_INPUT_LENGTH];
	objectType * paper;

    oneArgument(arg, number);
    if(!number || !*number || !isdigit(*number)) return(FALSE);
    if( !(i = atoi(number)) || strchr( number, '.') ) return(FALSE);

	i = cb->num - i;

    if(cb->num==0)
	{
        send_to_char("The board is empty!\n\r", ch);
        return(TRUE);
    }
    if(i < 0 || i >= cb->num){
        send_to_char("That message exists only in your imagination.\n\r",ch);
        return(TRUE);
    }
    if( GET_LEVEL(ch) < IMO && strcmp( GET_NAME(ch), cb->writer[i] ) != 0 )
	{
        send_to_char("It's not a your massage.\n\r",ch);
        return(TRUE);
    }
    if( paper_num = real_objectNr( PAPER_DB_NUM ), paper_num != OBJECT_NULL )
	{
		paper = load_a_object( paper_num, REAL, 1 );
		obj_to_char( paper, ch ); 
		paper->usedd = cb->msgs[i];
		send_to_char( "You detach a paper from the board.\n\r", ch );
	}
	else
	{
		send_to_char( "Arrah! You lost the paper which you just detached.\n\r", ch );
		errFree( cb->msgs[ i ] );
	}

	errFree( cb->head[ i ] );
	errFree( cb->writer[ i ] );
	errFree( cb->created[ i ] );

    for( ; i < cb->num - 1; i++)
	{
        cb->head[i] = cb->head[i+1];
        cb->writer[i] = cb->writer[i+1];
        cb->created[i] = cb->created[i+1];
        cb->msgs[i] = cb->msgs[i+1];
    }

	cb->num--;
    save_board( cb );

    act("$n detaches a message from the board.", TRUE, ch, 0, 0, TO_ROOM);

    sprintf( buf, "%s/%d.%.3d.board", BOARD_DIR, cb->room_num, cb->num );
	log( "remove_board> %s removed.", buf );
    if( unlink( buf ) < 0 )
        ERROR( "remove_board> can't unlink %s.", buf );

    return(TRUE);
}

int read_board(charType *ch, struct board_data *cb, char *arg)
{
    char * buffer, number[MAX_INPUT_LENGTH];
    int i, j;

    oneArgument(arg, number);
    if( !number || !*number || !isdigit(*number)) return(FALSE);
    if(!(i = atoi(number))) return(FALSE);

	j = i--;

    if(cb->num == 0)
	{
        send_to_char("The board is empty!\n\r", ch);
        return(TRUE);
    }
    if( i < 0 || i >= cb->num )
	{
        send_to_char("That message exists only in your imagination.\n\r",ch);
        return(TRUE);
    }

	i = cb->num - 1 - i;

	buffer = errMalloc( strlen( cb->msgs[i] ) + 300 );

	sprintf( buffer, "Message Num : [%.3d]\n\r"
					 "    Subject : %s\n\r"
					 "      Owner : %s\n\r"
                     "     Posted : %s\n\r\n\r"
					 "%s\n\r",
   					 j, cb->head[ i ], cb->writer[ i ], cb->created[ i ], cb->msgs[ i ] );

    act("$n reads a messages on board.", TRUE, ch, 0, 0, TO_ROOM);
    print_page( ch, buffer );

	errFree( buffer );

    return(TRUE);
}

int board( charType *ch, int cmd, char *arg )
{
    struct board_data *cur_board;

    if(!ch->desc) return(FALSE);
    if( cmd != COM_LOOK && cmd != COM_POST &&
		cmd != COM_REMOVE && cmd != COM_READ ) return(FALSE);

    cur_board = find_board(ch);

    if( cur_board == 0 ){
        ERROR( " board> Board allocation failed.\n\r." );
        return( FALSE );
    }

    switch(cmd){
      case COM_LOOK : 	return(show_board(ch,cur_board,arg));
      case COM_POST : 	return(post_board(ch,cur_board,arg));
	  case COM_REMOVE : return(remove_board(ch,cur_board,arg));
      case COM_READ : 	return(read_board(ch,cur_board,arg));
    }
	return FALSE;
}

void init_board( void )
{
    boards = 0;
}
