/* ************************************************************************
*  file: comm.c , Communication module.                   Part of DIKUMUD *
*  Usage: Communication, central game loop.                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
*  Using *any* part of DikuMud without having read license.doc is         *
*  violating our copyright.
************************************************************************* */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

#ifdef __svr4__
#include <rpc/types.h>
#endif

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "world.h"
#include "object.h"
#include "zone.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "sockets.h"
#include "variables.h"
#include "misc.h"
#include "fight.h"
#include "update.h"
#include "mobile.action.h"
#include "weather.h"
#include "strings.h"
#include "specials.h"
#include "nanny.h"
#include "quest.h"
#include "allocate.h"
#include "transfer.h"
#include "spec.rooms.h"
#include "edit.h"
#include "page.h"
#include "specials.h"

#define DFLT_DIR "."

//#ifndef __BSD__
//#define DFLT_DIR "/var/tmp/MUD/diku"        /* default dir  */
//#else
//#define DFLT_DIR "."
//#endif
#define DFLT_PORT 12700      		/* default port */

#define OPT_USEC 250000       /* time delay corresponding to 4 passes/sec */
#define MAXOCLOCK (120*4)

//jmp_buf env;

int 	shutdowngame 	= 0;    	/* clean shutdown */
int 	autoshutdown 	= 0;    	/* clean shutdown */
int 	stopshutdown 	= 0;    	/* clean shutdown */
int		manualshutdown  = 0;
int 	ticks 			= 1;   		/* checkpointing  */
int		beforeround		= 0;

descriptorType		* watch;

/**********************************************************************
*  main game loop and related stuff               *
********************************************************************* */

void freaky( descriptorType *d )
{
  	DEBUG( "freaky> connected[%d], fd[%d] - %s - %s",
    	d->connected, 
		d->fd,
    	d->character ? ( d->character->name ? d->character->name : "NoName") : "NoChar", 
		d->host );
}

void checkpointing(int sig)
{
  	if( !ticks ) 
  	{
    	DEBUG( "CHECKPOINT> ticks not updated" );
		abort();
  	}
  	else
	{
    	log( "checkpoint> %d ticks", ticks );
		ticks = 0;
	}
}

void shutdown_request( int sig )
{   
    senddf( 0, 0, "Received USR2 - Host shutdown requested!");
    log( "shutdown_request> Received USR2 - Host shutdown request");
    shutdowngame = 2;
}

void hupsig( int sig )
{ 
  	static int 	signalcounter = 1;
  
	if( watch && watch->character && watch->character->name )
	{
		if( watch->connected == CON_PLYNG )
			DEBUG( "hupsig> [%d,%d] <-- %s(%d) %s from %s in %s(%d)", sig, signalcounter,
					watch->character->name,
					watch->character->level, 
					watch->last_input,
					watch->host, 
					world[watch->character->in_room].name,
					watch->character->in_room) ;
		else
			DEBUG( "hupsig> [%d,%d] <-- %s(%d) %s from %s.", sig,signalcounter,
					watch->character->name,
					watch->character->level,
					watch->last_input,
					watch->host );
	}
	else if( watch )
	{
		DEBUG( "hupsig> [%d,%d] <-- unkown player %s - connection( %d ) from %s.", sig, signalcounter,
				watch->last_input, 
				watch->connected, watch->host );
	}
	else
		DEBUG( "hupsig> [%d.%d] . received at outside of interpreter.", sig, signalcounter );

  	if( ++signalcounter > 5 ) 
	{
		senddf( 0, 0, "Recieved SIGNAL, Shutting Down!");
		shutdowngame = 2 ;
	}
	if( signalcounter >= 100 ) FATAL( "hupsig> too many signals" );
} 

void signal_setup(void)
{
	struct sigaction 	act;

	memset( &act, 0, sizeof(act) );

	act.sa_handler = hupsig;
	act.sa_flags   = SA_RESTART;
	sigemptyset( &act.sa_mask );

  	signal(SIGPIPE, SIG_IGN);

  	sigaction(SIGHUP , &act, 0 );
  	sigaction(SIGINT , &act, 0 );
  	sigaction(SIGTERM, &act, 0 );

	act.sa_handler = shutdown_request;
	sigaction(SIGUSR2, &act, 0 );

/*
#ifndef __BSD_DEBUG__ 
	{
		struct itimerval 	itime;
		struct timeval 		interval;

		interval.tv_sec   = 120;
		interval.tv_usec  = 0;
		itime.it_interval = interval;
		itime.it_value 	  = interval;

		act.sa_handler = checkpointing;

		setitimer( ITIMER_REAL, &itime, 0 );
		sigaction(SIGVTALRM, &act, 0 );
		sigaction(SIGALRM,   &act, 0 );
	}
#endif
*/
}

struct timeval timediff(struct timeval *a, struct timeval *b)
{
  	struct timeval rslt, tmp;

  	tmp = *a;

  	if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0)
  	{
    	rslt.tv_usec += 1000000;
    	--(tmp.tv_sec);
  	}
  	if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0)
  	{
    	rslt.tv_usec = 0;
    	rslt.tv_sec =0;
  	}
  	return(rslt);
}

void record_player_number( void )
{
  	descriptorType 	*		d;
  	int 					t,  tod;
  	int 					m = 0;
  	static int most = 0;

  	t = time(0) + 32400;        /* 86400 is a day */

  	if( tod = (t%3600), tod > 3540 )
  	{
  		for( d = desc_list; d; d = d->next ) 
  		{
			++m;
			if( d->original )
			{
				log( "%3d-%2d-%-14s-%2d-%s", d->fd, d->original->timer, d->original->name, 
						d->original->level, d->host );
			} 
			else if( d->character )
			{
				log( "%3d-%2d-%-14s-%2d-%s", d->fd, d->character->timer,	
					(d->connected == CON_PLYNG) ? d->character->name : "Not in game", 
					d->character->level, d->host );
			} 
			else
                log( "%3d%9s%10s ", d->fd,"  UNDEF  ", connected_types[d->connected] );
  		}

  		if( m > most ) most = m;

  		log( "%d/%d active connections", m, most );

  		t = 30 + time(0) - boottime;
  		log( "Running time %d:%02d", t/3600, (t%3600)/60 );
  	}
}

void saveallplayers( void )
{
    descriptorType *pt, *npt;

    for( pt = desc_list; pt; pt = npt ) 
	{
        npt = pt->next;

        if(pt->connected == CON_PLYNG && pt->character)
		{
            save_char( pt->character, NOWHERE );
            stash_char( pt->character,0 );
        }
    }
}

void game_loop(int s)
{
	static int 					pulse, xoclock = 0;
  	static struct timeval 		opt_time;

  	fd_set 						input_set; 
	fd_set						output_set; 
	fd_set						exc_set;

  	struct timeval 				last_time; 
	struct timeval				now; 
	struct timeval				timespent; 
	struct timeval				timeout; 
	struct timeval				null_time;

  	char 						comm[MAX_INPUT_LENGTH];
  	descriptorType 			* 	point;
  	sigset_t 					mask; 
	sigset_t					oldmask;

  	null_time.tv_sec  = 0;
  	null_time.tv_usec = 0;
  	opt_time.tv_usec  = OPT_USEC;  /* Init time values */
  	opt_time.tv_sec   = 0;

  	gettimeofday(&last_time, (struct timezone *) 0);

  	last_desc = s;

	sigfillset( &mask );

  	for( pulse = 1, xoclock = 0; shutdowngame != 1 ; pulse++, ticks++, xoclock++ ) 
  	{
  		if( shutdowngame > 1 ) 
  		{
  			if( --shutdowngame == 1 ) senddf( 0, 0, "Saving all players." );
  		}

    	FD_ZERO( &input_set );
    	FD_ZERO( &output_set );
    	FD_ZERO( &exc_set );
    	FD_SET ( s, &input_set );

    	for (point = desc_list; point; point = point->next) 
		{
      		FD_SET(point->fd, &input_set);
      		FD_SET(point->fd, &exc_set);
      		FD_SET(point->fd, &output_set);
    	}

    	gettimeofday( &now, (struct timezone *) 0 );

    	timespent 	= timediff(&now, &last_time);
    	timeout 	= timediff(&opt_time, &timespent);

    	last_time.tv_sec  = now.tv_sec  + timeout.tv_sec;
    	last_time.tv_usec = now.tv_usec + timeout.tv_usec;

    	if (last_time.tv_usec >= 1000000) 
		{
      		last_time.tv_usec -= 1000000;
      		last_time.tv_sec++;
    	}

    	sigprocmask( SIG_BLOCK, &mask, &oldmask );

    	if(select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) 
        	FATAL("Select sleep");

    	if(select(last_desc+1, &input_set, &output_set, &exc_set, &null_time) < 0) 
      		FATAL("Select poll");

 		sigprocmask( SIG_UNBLOCK, &oldmask, 0 );

    	if( FD_ISSET(s, &input_set) &&  new_descriptor(s) < 0 ) ERROR( "GameLoop> New connection" );

    	for (point = desc_list; point; point = next_to_process) 
		{
      		next_to_process = point->next;   
      		if( FD_ISSET(point->fd, &exc_set)) 
	  		{
        		freaky(point);
        		FD_CLR(point->fd, &input_set);
        		FD_CLR(point->fd, &output_set);
        		close_socket(point);
      		}
    	}

    	for (point = desc_list; point; point = next_to_process) 
		{
      		next_to_process = point->next;

			if (FD_ISSET(point->fd, &input_set) && (process_input(point) < 0))
			{
           		FD_CLR(point->fd, &input_set);
           		FD_CLR(point->fd, &output_set);
           		if (point->connected == CON_PLYNG) 
		   		{
					save_char( point->character, NOWHERE );
             		stash_char(point->character,0);
           		}
           		close_socket(point);
        	}
    	}

    	for( point = desc_list; point; point = next_to_process) 
		{
      		next_to_process = point->next;	watch = point;

      		if( (--(point->wait) <= 0) && get_from_input( point, comm)) 
	  		{
        		if ( no_echo ) SEND_TO_Q( "\n", point );

        		echo_telnet( point ); no_echo = 0;

        		if(   point->character 
				   && point->connected == CON_PLYNG 
                   && point->character->was_in_room !=  NOWHERE ) 
			 	{
          			if( point->character->in_room != NOWHERE ) char_from_room(point->character);
          			char_to_room(point->character, point->character->was_in_room);
          			point->character->was_in_room = NOWHERE;
          			act("$n has returned.",  TRUE, point->character, 0, 0, TO_ROOM);
        		}

        		point->prompt = 1;	
				point->wait   = 1;
				
        		if( point->character ) point->character->timer = 0;

        		if( point->str ) string_add( point, comm );
				else if( point->character && point->character->page )
					page_interpreter( point->character, comm );
				else if( point->character && point->character->mode )
				{
					switch( point->character->mode )
					{
						case EDIT_MODE : edit_interpreter( point->character, comm ); break;
						default		   : point->character->mode = 0;
					}
				}
        		else if( point->connected == CON_PLYNG )  
				{
					int wait_value = interpreter( point->character, comm, 0 );
           			point->wait += wait_value;
           			++point->ncmds;
        		}
        		else
				{
          			nanny(point, comm); 
          			if( no_echo ) no_echo_telnet( point );
        		}
      		}
    	}

		watch = 0;

    	for( point = desc_list; point; point = next_to_process ) 
		{
      		next_to_process = point->next;
      		if( FD_ISSET(point->fd, &output_set) && point->output.used )
			{
        		if( process_output(point) < 0 )
				{
          			FD_CLR(point->fd, &input_set);
          			FD_CLR(point->fd, &output_set);
          			if( point->connected == CON_PLYNG ) 
		  			{
						save_char( point->character, NOWHERE );
             			stash_char(point->character,0);
          			}
          			close_socket(point);
        		} 
				else point->prompt = 1;
			}
     	}

    	for( point = desc_list; point; point = next_to_process )
		{
			next_to_process = point->next;

      		if( point->prompt ) 
	  		{
        		if( point->str ) write_to_descriptor(point->fd, "] ");
				else if( point->character && point->character->page )
					page_prompt( point );
				else if( point->character && point->character->mode )
				{
					switch( point->character->mode )
					{
						case EDIT_MODE : edit_prompt( point ); break;
						default		   : point->character->mode = 0;
					}
				}
        		else if( point->connected == CON_PLYNG )
				{
					char		prompt[32];

           			sprintf( prompt, "<%d,%d,%d> ",	GET_HIT(point->character),
													GET_MANA(point->character),
													GET_MOVE(point->character));
           			write_to_descriptor( point->fd, prompt );
      			}
       			point->prompt = 0;
    		}
		}

    	// setjmp(env);

    	if( !(pulse % PULSE_ZONE) ) 
		{
      		update_zones();
       		record_player_number();
    	}	

		update_specUnit();

    	if( !((pulse   ) % PULSE_ROUND ) )
		{
			perform_violence();
			beforeround = 1;
		}

		if( !((pulse    ) % 3           ) ) update_second();
    	if( !((pulse+17 ) % PULSE_ROUND ) )   
		{
			update_round();
			beforeround = 0;
		}
    	if( !((pulse+9) % PULSE_MOBILE) )  mobile_activity();

    	if( !((pulse+5 ) % (SECS_PER_MUD_HOUR * 4)) )
		{
      		weather_and_time(1);
      		update_tick();
    	}

        if( pulse   >= 2400 )       pulse   = 0;
        if( xoclock >= MAXOCLOCK )  xoclock = 0; 
                                          
        if( !(pulse + 6 % PULSE_ZONE) && (xoclock % 4 == 0) )
        {
            int             i = 0;
            charType    *   tmp;

            for( tmp = player_list; tmp; tmp = tmp->next_char, i++ )
            {
                if( (tmp->level < IMO) && (i == xoclock/4) )      
                {
                    save_char( tmp, NOWHERE );
                    stash_char( tmp, 0 );
                }
            }
        }
 	}
  	saveallplayers(); 

  	if( autoshutdown ) 
  	{
  		log( "Clearing insurance." );
  		clear_insurance();
  	}
}

void boot_db(void)
{
  	int i;

  	log( "====> Boot db -- BEGIN." );

	log( "Init memeory system...." ); 	init_allocation();
  	log( "Resetting the game time:"); 	reset_time();
  	log(">> News, Credits, Help-page, Info, Wizards, Cookie and MOTD.<<");

  	file_to_buf(NEWS_FILE, news);
  	file_to_buf(CREDITS_FILE, credits);
  	file_to_buf(MOTD_FILE, motd);
  	file_to_buf(TODO_FILE, todo);
  	file_to_buf(HELP_PAGE_FILE, help);
  	file_to_buf(INFO_FILE, info);
  	file_to_buf(WIZARDS_FILE, wizards);

  	file_to_cookie(COOKIE_FILE, cookie);

	log( ">> World     <<" );   boot_world();
  	log( ">> Player    <<" );   build_player_index();
	log( ">> Help      <<" );	build_help_index();
	log( ">> Messages  <<" );   load_messages();
 	log( ">> Rebooting <<" );

  	for( i = 1; i < zones_in_world; i++ ) 
	{
    	log( "%2d> Reset of %28s < %3d > [ #%-5d - #%-5d ]",
      			i, zones[i].name, zones[i].lifespan, 	
	  			zones[i].bottom, zones[i].top );
    	reset_zone( i, 1 );
  	}
	log( ">> Specials  <<" );
  	log( "\t Mobiles. "); 		assign_mobiles();
   	log( "\t Objects. "); 		assign_objects();
   	log( "\t Room.    "); 		assign_rooms();
	log( ">> Miscell   <<" );
  	log( "\t Board ") ; 		init_board(); 
  	log( "\t Mail  ") ; 		init_mbox() ;	
  	log( "\t Pawn  ") ; 		init_pawn() ;	
  	log( "\t Bank  ") ; 		init_bank() ;	
  	log( "\t Locker") ; 		init_locker() ;	
  	log( "\t Insurance"); 		init_insurance() ;	
  	log( "\t autoreboot"); 		init_autoreboot() ;	
  	log( "\t portal gate" );	init_portal_gate();
 	log( "\t Quest ") ; 		init_quest();
  	log("====> Boot db -- DONE.");
}

int main(int argc, char **argv)
{
  	int 		port;
    int 		sock;
  	char 	*	dir;
  	FILE	*   pidfp;

  	close(0); close(1);

  	port = DFLT_PORT; 	dir  = DFLT_DIR;

  	strcpy(baddomain[0],"131.215");

  	if( argc == 2 ) 
  	{
    	if( !isnumstr(argv[1]) ) 
		{
      		FATAL( "Usage: %s [ port # ]", argv[0] );
    	} 
		else if ((port = atoi(argv[1])) <= 3999 ) 
		{
      		FATAL("main> Illegal port #" );
    	}
  	}

  	log("====> Running game on port %d", port );
  	log("====> Signal.   " );	signal_setup(); 
  	log("====> Bind.     " );	sock = init_socket(port);
	log("====> PID file  " );

  	if( pidfp = errOpen( "dms.pid", "w+" ), !pidfp )
	{
		FATAL( "main> can't open pid file." );
	}
	else
	{
  		fprintf( pidfp, "%ld\n", (long)getpid() );
  		errClose( pidfp );
	}

	log("====> Chdir to %s", dir );

  	if( chdir(dir) < 0 ) 
  	{
    	FATAL( "chdir> fail to change %s", dir );
  	}
	

	log("====> Booting.  " ); 	boot_db(); 

	if( argc == 3 ) exit( 0 );

  	srand( boottime = time(0) );

  	log("====> Game loop." ); 	no_echo_local( sock );

  	game_loop(sock);

  	close_sockets(sock);

  	log( "====> Normal termination of game." );

	exit( 0 );
}
