/******************************************************************************
 * mail.c ( mail routine of KIT Mud a.k.a. jinsil mud )                       *
 * mail.c is coded by Kim Jiho ( Nara of Baram )                              *
 ******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <sys/stat.h> 
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "utils.h"
#include "comm.h"
#include "find.h"
#include "interpreter.h"
#include "misc.h"
#include "variables.h"
#include "strings.h"
#include "stash.h"
#include "transfer.h"

#define MAX_MAILS 200
#define PNAME_LEN 50

#define MAIL_PAPER 1
#define MAIL_ITEMS 2

struct mbox_data
{
	int				type;
	char 		*	head[MAX_MAILS];  			/* head of mbox */
	char 		*	msgs[MAX_MAILS];  			/* msg of mbox */
	char 		* 	sender[MAX_MAILS]; 			/* sender of that mesg */
	char 		*	receiver[MAX_MAILS]; 		/* receiver of that mesg */
	char 		*	create_time[MAX_MAILS] ;	/* posting time */
	objectType 	*	objs;
	int		 		num;  						/* msg number that mbox contains */
	int 			room_num;  					/* room number that mbox is in */
	char 			bfile[PNAME_LEN];  			/* mbox file name */
	struct 			mbox_data *next;  			/* next */
};

struct mbox_data * mbox_list = NULL;

static int calc_mail_cost( objectType * obj )
{
	return obj->cost / 100;
}

int load_mbox(struct mbox_data * mbox )
{
	FILE 	* fp, * mfp;
    int 	i, num = 0, size;
	char   	buf[100], dummy[100];

    if( fp = errOpen( mbox->bfile, "r+" ), !fp )
	{
        creat( mbox->bfile, S_IRUSR | S_IWUSR );
        if( fp = errOpen( mbox->bfile, "w+" ), !fp )
        {
            FATAL( "load_mbox> failed to create new %s.", mbox->bfile );
        }
        else
        {
            fprintf( fp, "#0 total" );
            log( "load_mbox> new %s file created.", mbox->bfile );
            fclose( fp );
        }
        return 0;
    }

	fscanf( fp, "#%d %s\n", &num, dummy );

    if( num < 0 || num > MAX_MAILS ) 
	{
        ERROR( "load_mbox> mbox corrupted." );
        mbox->num = 0;
        fclose(fp);
        return 0;	
    }

	if( mbox->type == MAIL_ITEMS )
	{
		unstash_off_world( "silver", &mbox->objs );
	}

    for( mbox->num = num, i = 0; i < mbox->num; i++ )
	{
		if( fscanf( fp, "#%d\n", &num ) != 1 || num != i ) 
		{
			ERROR( "load_mail> Mail sequence does not match." );
			mbox->num = i;
			break;
		}
        mbox->head[i]        = remove_newline( strdup( fgets( buf, 80, fp )));
		mbox->sender[i]      = remove_newline( strdup( fgets( buf, 80, fp )));
		mbox->receiver[i]    = remove_newline( strdup( fgets( buf, 80, fp )));
		mbox->create_time[i] = remove_newline( strdup( fgets( buf, 80, fp )));

		if( mbox->type == MAIL_PAPER )
		{
			sprintf( buf, "%s/%d.%.3d.mail", MBOX_DIR, mbox->room_num, i );

			if( mfp = errOpen( buf, "r" ), !mfp ) break;

			fseek( mfp, 0, SEEK_END );
			size = (int)ftell( mfp );
			rewind( mfp );

			mbox->msgs[i] = (char *)errMalloc( size+1 );

			if( !mbox->msgs[i] )
			{
				ERROR( "load_mbox> malloc failure of (%d) size.", size+1 );
				break;
			}
			fread( mbox->msgs[i], size, 1, mfp );
			mbox->msgs[i][size] = 0;
			errClose( mfp );
		}
	}
	fclose(fp);

	mbox->num = i;
	log( "load_mbox> loading %d mails at %d finished.", i, mbox->room_num );
    return 1;
}

struct mbox_data *init_a_mbox( int roomNr, int type )
{
	struct mbox_data * tmp;

    tmp = (struct mbox_data *)errCalloc( sizeof( struct mbox_data ) );

    if( tmp )
	{
        tmp->room_num = world[roomNr].virtual;
		tmp->type     = type;

        sprintf( tmp->bfile, "%s/%d.mbox", MBOX_DIR, tmp->room_num );
        load_mbox( tmp );

      	tmp->next = mbox_list;
       	mbox_list = tmp;
    }
    return tmp;
}

void init_mbox( void )
{
	(void)init_a_mbox(real_roomNr( ROOM_POST_OFFICE ), MAIL_PAPER );
	(void)init_a_mbox(real_roomNr( ROOM_SILVER      ), MAIL_ITEMS );
}

struct mbox_data * find_mbox( charType *ch )
{
    struct mbox_data * tmp_mbox = 0;
    int ch_rnum;

    ch_rnum = world[ch->in_room].virtual;

    if( mbox_list )
	{
        for( tmp_mbox = mbox_list; tmp_mbox; tmp_mbox = tmp_mbox->next )
		{
            if( tmp_mbox->room_num == ch_rnum )
			{
                return tmp_mbox;
            }
        }
    }
    return tmp_mbox;
}

int save_mbox( struct mbox_data *mbox )
{
	FILE * fp, * mfp;
    int i  ;
	char   buf[100];

    if( fp = errOpen( mbox->bfile, "w" ), !fp )
	{
		ERROR( "save_mbox> Saving mbox(%d) failed.", mbox->room_num );
        return 0 ;
    }

    fprintf( fp, "#%d total\n", mbox->num );

	if( mbox->type == MAIL_ITEMS )
	{
		stash_off_world( "silver", mbox->objs );
	}

    for( i = 0; i < mbox->num; i++ )
	{
		fprintf( fp, "#%d\n", i );
		fprintf( fp, "%s\n", mbox->head[i] );
		fprintf( fp, "%s\n", mbox->sender[i] );
		fprintf( fp, "%s\n", mbox->receiver[i] );
		fprintf( fp, "%s\n", mbox->create_time[i] );

		if( mbox->type == MAIL_PAPER )
		{
			sprintf( buf, "%s/%d.%.3d.mail", MBOX_DIR, mbox->room_num, i );
			if( mfp = errOpen( buf, "w" ), !mfp ) break;
		
			fprintf( mfp, "%s\n", mbox->msgs[i] );
			errClose( mfp );
		}
    }
    fclose(fp);

	return 1 ;	/* save successful */
}

int check_mail( charType *ch, int room )
{
	struct mbox_data *	mbox, * find ;
	int					i, rNr;

	if ( !ch ) return (FALSE) ;

	mbox = NULL;

	if( rNr = real_roomNr( room ), rNr == NOWHERE )
	{
		DEBUG( "check_mail> %d room does not exist.", room );
		return FALSE;
	}

	if( mbox_list )
	{
		for(  find = mbox_list;  find;  find =  find->next )
		{
			if(  find->room_num == room )
			{
				mbox =  find;
				break ;
			}
		}
	}
	else 
	{
		log("check_mail> There is no mbox list.") ;
		return FALSE;
	}

	if( mbox == NULL )
	{
		DEBUG("check_mail> There is no mbox at #%d.", room ) ;
		return( FALSE );
	}

	for( i = 0 ; i < mbox->num; i++ )
	{
		if( isoneof( GET_NAME(ch), mbox->receiver[i]) ) return rNr;
	}
	return FALSE ;	/* have no mail for you */
}

int show_mail( charType *ch, struct mbox_data *mbox, char *arg )
{
	objectType *	temp;
    int 			i, j, k, pm_num = 0;
    char 			tmp[MAX_INPUT_LENGTH];

	/* 'look mail' or 'look mbox' or 'look board' is valid */
    oneArgument(arg, tmp);

	if( !*tmp || !isoneof(tmp, "mail mbox board")) return(FALSE);

	act("$n looks in the mail box.", TRUE, ch, 0, 0, TO_ROOM);

	if( mbox->type == MAIL_PAPER )
	{
		sendf( ch, "\n\r" 
				   "  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n\r"   
				   "  + usage> get letter mbox / post <paper> <receiver> <header>   + \n\r"
				   "  +    ex> post paper nuis  how you doing?                      + \n\r"
				   "  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n\r" );

		for (i = 0, j = 0 ; i < mbox->num; i++)
		{
			if( isoneof(GET_NAME(ch), mbox->receiver[i]) || GET_LEVEL(ch) > IMO+2 )
			{
				sendf( ch, "[%03d] Subject ---[%s]\n\r"
						   "       Sender   :  %s\n\r"
						   "       Reciever :  %s\n\r"
						   "       Sent     :  %s",
							++j, mbox->head[i], mbox->sender[i], mbox->receiver[i], 
							mbox->create_time[i] );
				pm_num++;
			}
		}
	}
	else if( mbox->type == MAIL_ITEMS )
	{
		sendf( ch, "\n\r" 
				   "  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n\r"   
				   "  + usage> get  <object> mbox / post <object> <receiver> <header>  + \n\r"
				   "  +    ex> post boddari nuis  Hi ya! I found your boddari.         + \n\r"
				   "  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n\r" );

		for (i = 0, j = 0 ; i < mbox->num; i++)
		{
			if( isoneof(GET_NAME(ch), mbox->receiver[i]) || GET_LEVEL(ch) > IMO+2 )
			{
				for( k = 0, temp = mbox->objs; temp && k != i; k++, temp = temp->next_content )
				;
				if( !temp )
				{
					DEBUG( "show_mbox> finding object failed." );
					return 1;
				}
				sendf( ch, "[%03d] Subject ---[%s]\n\r"
						   "       Sender   :  %s\n\r"
						   "       Reciever :  %s\n\r"
						   "       Sent     :  %s\n\r"
						   "       Object   :  %s\n\r"
						   "       Cost     :  %s coins",
							++j, 
							mbox->head[i],	mbox->sender[i], mbox->receiver[i], 
							mbox->create_time[i], temp->wornd, numfstr( calc_mail_cost( temp )) );
				pm_num++;
			}
		}
	}
	else
	{
		DEBUG( "show_mbox> unknown mbox type." );
	}

	if( !pm_num ) sendf( ch, "Nothing." );

    return(TRUE);
}

objectType * remove_mail( struct mbox_data * mbox, int i )
{
	objectType 	  *	obj = 0, * find;
	int				j;
	char  			buf[100];

    if(mbox->head[i]) 			errFree(mbox->head[i]);
	if(mbox->sender[i]) 		errFree( mbox->sender[i] );
	if(mbox->receiver[i] ) 		errFree( mbox->receiver[i] );
	if(mbox->create_time[i] ) 	errFree( mbox->create_time[i] );
	if(mbox->msgs[i]) 			errFree(mbox->msgs[i]);

	if( mbox->type == MAIL_ITEMS )
	{
		if( !mbox->objs )
		{
			DEBUG( "remove_mail> mbox has no item." );
			return NULL;
		}

		if( i == 0 )
		{
			obj = mbox->objs;
			mbox->objs = obj->next_content;
		}
		else
		{
			for( find = mbox->objs, j = 0; find && j < i - 1; j++, find = find->next_content )
			;

			if( !find )
			{
				DEBUG( "remove_mail> object not found." );
				return NULL;
			}

			obj = find->next_content;
			find->next_content = obj->next_content;
		}

		obj->next_content = 0;
		obj->what = 0;
	}	

    for(; i < mbox->num - 1; i++)
	{
        mbox->head[i]        = mbox->head[i+1];
        mbox->msgs[i]        = mbox->msgs[i+1];
        mbox->sender[i]      = mbox->sender[i+1];
        mbox->receiver[i]    = mbox->receiver[i+1];
        mbox->create_time[i] = mbox->create_time[i+1];
    }

	mbox->num--;

    if( !save_mbox(mbox) ) ERROR( "remove_mail> save failed ") ;

	if( mbox->type == MAIL_PAPER )
	{ 
		sprintf( buf, "%s/%d.%.3d.mail", MBOX_DIR, mbox->room_num, mbox->num );
		if( unlink( buf ) < 0 )	ERROR( "remove_mail> can't unlink %s.", buf );
	}

	return obj;
}

int get_mail(charType *ch, struct mbox_data * mbox,  char * arg )
{
	char 					lt[MAX_INPUT_LENGTH], mb[MAX_INPUT_LENGTH];
	int 					pm_num=0, i ;
	int 					paper_num, found = 0;
	objectType 		*		mail;
	long					ct ;
	char			*		buf, * current_time;

	twoArgument( arg, lt, mb );

	if( !*arg || !isoneof(mb, "mail mbox board") ) return 0;

	if( mbox->type == MAIL_PAPER && !isoneof(lt, "letter mail") ) return(FALSE);

	for( i = 0; i < mbox->num; i++ )
	{
        if( isoneof(GET_NAME(ch), mbox->receiver[i]) )
		{
			int				j;
			objectType 	*	sub = 0;

			found = 1;
			if( mbox->type == MAIL_ITEMS ) 
			{
				for( sub = mbox->objs, j = 0; sub && j != i; j++, sub = sub->next_content )
				;

				if( !sub )
				{
					DEBUG( "get_mail> object not found." );
					return FALSE;
				}
				if( !isoneof( lt, sub->name ) ) continue;
			}

			ct = time(0) ;	
			current_time = strdup(ctime( &ct )) ;
			current_time[strlen(current_time) -1] = NULL ;

			buf = errMalloc( mbox->msgs[i] ? strlen( mbox->msgs[i] ) + 300 : 300 );

			sprintf( buf,"Subject  : %s\n\r"
					     "Sender   : %s\n\r"
					     "Sent     : %s\n\r"
					     "Recieved : %s\n\r\n\r"
					     "%s\n\r",
						 mbox->head[i], mbox->sender[i],	mbox->create_time[i], current_time,
						 mbox->type == MAIL_PAPER ? mbox->msgs[i] : sub->wornd );
			errFree( current_time );
            pm_num++;

            if( paper_num = real_objectNr( OBJECT_PAPER ), paper_num == OBJECT_NULL )
			{
                DEBUG( "get_mail> no such paper item #d", OBJECT_PAPER );
                return(FALSE);
            }

            remove_mail(mbox,i);

            mail = load_a_object( paper_num, REAL, 1 );

			if( sub ) obj_to_char( sub, ch );
            obj_to_char(mail,ch);

            mail->usedd = buf;

			if( sub ) 
			{
				act( "$n pick up $p from the mail box.", TRUE, ch, sub, 0, TO_ROOM );
            	sendf( ch, "You got %s from mail box.", sub->wornd );
			}
			else      
			{
				act("$n pick up a letter from the mail box.", TRUE, ch, 0, 0, TO_ROOM);
            	sendf( ch, "You got a letter from mail box." );
			}
            return(TRUE);
        }
    }

	if( !found ) sendf( ch, "Your mail box is empty." );
	else		 sendf( ch, "No such object in your mail box." );
    return(TRUE);
}

int is_postable( objectType * obj )
{
    if( obj->contains )     if( !is_postable( obj->contains ) ) return 0;
    if( obj->next_content ) if( !is_postable( obj->next_content ) ) return 0;

    if( !is_stashable( obj ) ) return 0;

    return 1;
}

int post_mail( charType * ch, struct mbox_data * mbox, char * arg )
{
	long			ct;
	char 			papername[MAX_INPUT_LENGTH], header[MAX_INPUT_LENGTH], receiver[MAX_INPUT_LENGTH];
	char		 	buf[MAX_INPUT_LENGTH];
	objectType 	*	paper;

	halfchop(arg, papername, buf);
	halfchop(buf, receiver, header);

	if( *papername == 0 || *header == 0 || *receiver == 0 ) 
	{
		sendf( ch, "Post? Yeah, We should post. But, look mbox first for usage!." ) ;
		return(FALSE);
	}

	paper = find_obj_inven( ch, papername );

	if( !paper )
	{
	    sendf( ch, "You can't find %s in your inventory.", papername );
		return FALSE;
	}

	if( mbox->type == MAIL_PAPER )
	{
		if( paper->type != ITEM_NOTE )
		{
			sendf( ch, "You can't post %s on this mbox.", papername );
			return FALSE;
		}
		else if( paper->usedd == 0 )
		{
        	sprintf(buf,"%s is empty.\n\r",papername);
			return FALSE;
    	}
	}
	else
	{
		if( !is_postable( paper ) )
		{
			sendf( ch, "You can not post that." );
			return FALSE;
		}
	}

	if( find_name(receiver) < 0 ) 
	{
		sendf( ch,  "There is no such player in this MUD." );
		return FALSE;
	}
	else
	{
		if( mbox->num >= MAX_MAILS ) 
		{
			sendf( ch, "Too much mails are posted already, try later" );
			return TRUE;
		}

        sendf( ch, "Ok. You posted %s on mbox.", papername );

		ct = time(0) ;

        mbox->head[mbox->num]        = strdup(header);
        mbox->sender[mbox->num]      = strdup(GET_NAME(ch));
        mbox->receiver[mbox->num]    = strdup(receiver);
		mbox->create_time[mbox->num] = strdup(ctime(&ct));

		mbox->create_time[mbox->num][strlen(mbox->create_time[mbox->num])-1]= 0 ;

		if( mbox->type == MAIL_PAPER )
		{
        	mbox->msgs[mbox->num]    = paper->usedd;
        	paper->usedd             = 0;
        	extract_obj( paper, 1 );
		}
		else
		{
			obj_from_char( paper );

			if( !mbox->objs )
			{
				mbox->objs = paper;
			}
			else
			{
				objectType	*	temp;

				obj_to_off_world( paper );

				for( temp = mbox->objs; temp->next_content; temp = temp->next_content )
				;
				temp->next_content = paper;
			}
		}
        mbox->num++;
    }

    if( !save_mbox(mbox) ) 
	{
		sendf( ch, "Opps! There is a deliverly problem. Ask to wizard." );
	}
	else
	{
		act( "$n puts $p in the mail box.", TRUE, ch, paper, 0, TO_ROOM);
	}
    return(TRUE);
}

void do_mail( charType *ch, char *argument, int cmd )
{
	int		nr, found = 0;

	if( ch == NULL ) return ;

	if( nr = check_mail( ch, ROOM_POST_OFFICE ), nr )
	{
		sendf( ch, "\r\nYou have MAIL in %s.", world[nr].name ) ;
		found = 1;
	}
	if( nr = check_mail( ch, ROOM_SILVER ), nr )
	{
		sendf( ch, "\r\nYou have MAIL in %s.", world[nr].name ) ;
		found = 1;
	}

	if( cmd && !found ) sendf( ch, "No mail for you.\n\r");

	return ;
}

int mbox( charType *ch, int cmd, char *arg )
{
    struct mbox_data *cur_mbox;

	if ( !ch ) return (FALSE) ;
    if(!ch->desc) return(FALSE);
    if( cmd != COM_LOOK && cmd != COM_POST && cmd != COM_GET ) return(FALSE);

    cur_mbox = find_mbox(ch);

    if( cur_mbox = find_mbox( ch ), cur_mbox == NULL )
		DEBUG( "mbox> locating #%d mbox failed", world[ch->in_room].virtual ) ;
	else
		switch(cmd)
		{
		  case COM_LOOK :	 return( show_mail(ch,cur_mbox,arg) );
		  case COM_POST :	 return( post_mail(ch,cur_mbox,arg) );
		  case COM_GET  :	 return( get_mail(ch,cur_mbox,arg)  );
		}
	return 0;
}
