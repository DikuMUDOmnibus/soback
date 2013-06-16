#include <sys/types.h>

#ifdef __USE_TERMIOS__
#include <termios.h>
#else
#include <termio.h>
#endif

#include <sys/socket.h>
#include <sys/ioctl.h>
/*
#include <sys/signal.h>
#include <sys/wait.h> 
*/
#include <sys/file.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define TELOPTS
#include <arpa/telnet.h> 

#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

#include "defines.h"
#include "character.h"
#include "sockets.h"
#include "mobile.h"
#include "utils.h"
#include "comm.h"
#include "variables.h"
#include "nanny.h"
#include "allocate.h"

#define MAX_HOSTNAME 256
#define MAXFDALLOWED 120

int avail_descs	= MAXFDALLOWED;
int last_desc		= 0;

descriptorType *     desc_list;
descriptorType *     next_to_process;

#ifndef __USE_TERMIOS__

void echo_local(int fd)
{
   struct termio io;

   ioctl(fd, TCGETA, &io);
   io.c_line = 0;
   io.c_lflag |= ECHO;
   ioctl(fd, TCSETA, &io);
}

void no_echo_local(int fd)	
{
   struct termio io;

   ioctl(fd, TCGETA, &io); 
   io.c_cc[VMIN] = 1;
   io.c_cc[VTIME] = 0;
   io.c_line = 0;
   io.c_lflag &= ~ECHO;
   ioctl(fd, TCSETA, &io);
}  

#else

void echo_local(int fd)
{
   struct termios io;

   ioctl(fd, TIOCGETA, &io);
   io.c_lflag |= ECHO;
   ioctl(fd, TIOCSETA, &io); 
}

void no_echo_local(int fd)	
{
   struct termios io;

   ioctl(fd, TIOCGETA, &io); 
   io.c_cc[VMIN] = 1;
   io.c_cc[VTIME] = 0;
   io.c_lflag &= ~ECHO;
   ioctl(fd, TIOCSETA, &io);
}  

#endif

void no_echo_telnet(descriptorType *d)
{
	char buf[5];

	sprintf(buf,"%c%c%c",IAC,WILL,TELOPT_ECHO);
	write(d->fd,buf,3);
}

void echo_telnet(descriptorType *d)
{
	char buf[5];

	sprintf(buf,"%c%c%c",IAC,WONT,TELOPT_ECHO);
	write(d->fd,buf,3);
}

/*--------------------------------------------------*/

/*******************************************************************
*  general utility stuff (for local use)                   *
****************************************************************** */

int get_from_input(descriptorType * d, char *dest)
{
  struct txt_block *tmp;
  struct txt_q     *queue;

  queue = &d->input;

   /* Q empty? */
  if(!queue->head) return(0);

  tmp = queue->head;

  strcpy(dest, queue->head->text);
  queue->head = queue->head->next;

  errFree(tmp->text);
  errFree(tmp);

  return(1);
}

void put_to_input(descriptorType * d, char * txt )
{
	struct	txt_block *new;
	struct	txt_q     *queue;
	int					len;

	if( !txt ) return;

  	new = errMalloc( sizeof(struct txt_block) );
	if ( new )
	{
		len = strlen( txt );
		new->text = errMalloc( len + 1 ) ;
	}
	else
		return ;

	if( new->text )	strcpy(new->text, txt);
	else
	{
		errFree( new );
		return;
	}

	queue = &d->input;

  	/* Q empty? */
  	if (!queue->head) 
  	{
    	new->next = NULL;
    	queue->head = queue->tail = new;
  	} 
  	else 
  	{
    	queue->tail->next = new;
    	queue->tail = new;
    	new->next = NULL;
  	}
}

void put_to_output( descriptorType * d, char * txt )
{
	struct fixedQueue * queue = &(d->output);
	char				dummy[MAX_STRING_LENGTH+1];

	if( !txt ) return;

	queue->queue[queue->head] = errMalloc( strlen( txt ) + 1 );
	strcpy( queue->queue[queue->head++], txt );

	if( queue->head > d->qSize ) queue->head = 0;

	queue->used++;

	if( queue->head == queue->tail )
	{
		get_from_output( d, dummy );
	}
}

int get_from_output( descriptorType * d, char * txt )
{
	struct fixedQueue * queue = &(d->output);

	if( queue->used <= 0 ) return 0;

	strcpy( txt, queue->queue[queue->tail] );
	errFree( queue->queue[queue->tail++] );

	if( queue->tail > d->qSize ) queue->tail = 0;
	
	queue->used--;

	return 1;
}

void flush_inqueue(descriptorType *d)
{
  	char dummy[MAX_STRING_LENGTH + 1];

  	while( get_from_input(d, dummy) );
}

void flush_ouqueue(descriptorType *d)
{
  	char dummy[MAX_STRING_LENGTH + 1];

  	while( get_from_output(d, dummy) );

  	d->output.head = d->output.tail = d->output.used = 0;
}

void flush_queues(descriptorType *d)
{
    flush_inqueue( d );
    flush_ouqueue( d );
}

/* ******************************************************************
*  socket handling               *
****************************************************************** */

int init_socket( int port )
{
  	int 				s;
  	int 				opt;
  	struct sockaddr_in 	sa;
  	struct linger 		ld;
	struct hostent *	hp; 
	char				hostname[256];


  	if( s = socket(AF_INET, SOCK_STREAM, 0), s < 0 ) 	FATAL( "init_socket> socket" );

  	if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) 
  	{
    	FATAL("init_socket> setsockopt REUSEADDR");
  	}

  	ld.l_onoff  = 1;
  	ld.l_linger = 10;

  	if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof(ld)) < 0) 
  	{
  		close( s );
    	FATAL("init_socket> setsockopt LINGER");
  	}

	memset( &sa, 0, sizeof( sa ) );

  	gethostname( hostname, MAX_HOSTNAME );
  	if( hp = gethostbyname(hostname), !hp )
  	{
  		close( s );
  		FATAL( "init_socket> gethostbyname." );
  	}

  	sa.sin_family = hp->h_addrtype;
/*  	sa.sin_family = AF_INET;		*/
  	sa.sin_port   = htons(port);

  	if( bind(s,(struct sockaddr *) &sa, sizeof(sa)) < 0 ) 
  	{
  		close( s );
    	ERROR( "init_socket> Bind" );
		exit( 4 );
  	}

  	if( listen( s, 3 ) < 0 )
  	{
  		close( s );
  		FATAL( "init_socket> listen" );
  	}
  	return(s);
}

int new_connection(int s)
{
  	struct sockaddr_in 	isa;
  	int 				i;
  	int 				t;

  	i = sizeof(isa);
  	getsockname(s, (struct sockaddr *) &isa, &i);
  	if(( t = accept(s, (struct sockaddr *) &isa, &i)) < 0) 
  	{
    	ERROR("new_connection> Accept");
		return -1;
  	}

  	if( fcntl(s, F_SETFL, FNDELAY) == -1)
  	{
    	ERROR("noblock> %d fd failed.", s );
		return -1;
  	}
  	return(t);
}

int unfriendly_domain( char * h )
{
   	int i;

   	for( i = 0; i < baddoms; ++i )
   	{
      	if( strncmp( h, baddomain[i], strlen(baddomain[i])) == 0 )
         	return(1);
   	}
   	return(0);
}

int new_descriptor(int s)
{
	int 				desc;
	descriptorType *	newd;
	struct sockaddr_in 	sock;
  	struct hostent * 	hentry;
	char   * 			cp;
	int 				size = sizeof(sock);

  	if ((desc = new_connection(s)) < 0) return (-1);

	newd = alloc_descriptor();

  	if( getpeername(desc, (struct sockaddr *) &sock, &size) < 0) 
  	{
    	ERROR( "new_descriptor> getpeername name failed." );
  	} 
  	else 
  	{
  	    if( cp = inet_ntoa( sock.sin_addr ), cp )
  		{	
  			strncpy( newd->ip, cp, DESC_HOSTNAME  );
/*  			if( (strncmp( newd->ip, "165.194", 7 ) != 0)
  			 && (strncmp( newd->ip, "202.31",  6 ) != 0)
  			 && (strncmp( newd->ip, "163.152", 7 ) != 0)
			 && ( hentry = 
				  gethostbyaddr((char *) &sock.sin_addr, sizeof(sock.sin_addr), AF_INET), hentry) )
				strncpy( newd->host, hentry->h_name, DESC_HOSTNAME );
			else
*/				strcpy( newd->host, newd->ip );
		}
		else
			ERROR( "new_descriptor> inet_ntoa failed." );
  	}

	if( !newd->ip[0] ) 
	{
		strcpy( newd->ip,   "(unkown)" );
		strcpy( newd->host, "(unkown)" );
	}

  	if((last_desc+1) >= avail_descs)
  	{
    	write_to_descriptor(desc, "Sorry.. The game is full...\n\r");
    	write_to_descriptor(desc, "Wait at least one minute before retrying.\n\r");
    	close(desc);
    	flush_queues(newd);
		free_desc( newd );
    	return(0);
  	} 
  	else if (desc > last_desc) last_desc = desc;

  	if(unfriendly_domain(newd->ip)) 
  	{
    	write_to_descriptor(desc, "Sorry, the game is unavailable from your site.\n\r");
    	close(desc);
    	flush_queues(newd);
		free_desc( newd );
    	return(-1);
  	}

  	newd->contime 		= time(0);
  	newd->fd 			= desc;
  	newd->connected 	= 1;
  	newd->wait 			= 1;
  	newd->qSize 		= 40;

  	*newd->buf 			= '\0';
  	*newd->last_input	= '\0';

  	newd->next = desc_list;
  	desc_list  = newd;

  	SEND_TO_Q(GREETINGS, newd);

  	if( nonewplayers )
	{
    	SEND_TO_Q("WARNING:\n\r",newd);
    	SEND_TO_Q("No NEW characters are being accepted right now.\n\r\n\r",newd);
  	}

  	if( nologin )
	{
    	SEND_TO_Q("Notice:\n\r",newd);
    	SEND_TO_Q("System is wating for rebooting, try later. \n\r\n\r",newd);
  	}
  	else 
  	{
  		SEND_TO_Q(
		"Enter \"who\", if you just want to peep through a keyhole or enter name to play.\n\r", newd );
    	SEND_TO_Q("By what name do you wish to be known? ", newd);
  	}

  	log( "New connection from %s", newd->host[0] ? newd->host : newd->ip );
  	return(0);
}

int process_output(descriptorType *t)
{
  	char 			i[MAX_STRING_LENGTH + 1];
  	char 			buf[MAX_STRING_LENGTH + 1];

  	if( !t->prompt && !t->connected )
    	if( write_to_descriptor(t->fd, "\n\r") < 0)
      		return(-1);

  	if( t->output.used >= t->qSize - 1 )
  		write_to_descriptor( t->fd, "=======> skipping <======\n\r" );
  	while( get_from_output(t, i) ) 
  	{
    	if( t->snoop.by ) 
		{
			sprintf( buf, "%% %s", i );
      		put_to_output( t->snoop.by->desc, buf );
    	}
    	if( write_to_descriptor( t->fd, i ) ) return( -1 );
  	}
  	if( !t->connected 
	 && !(t->character && !IS_NPC(t->character) && IS_SET(t->character->act, PLR_COMPACT)) )
    	if( write_to_descriptor( t->fd, "\n\r") < 0 )
      		return(-1);
  	return(1);
}

#include <sys/ioctl.h>

int write_to_descriptor(int desc, char *txt)
{
  	int sofar, thisround, total;

  	total = strlen(txt);
  	sofar = 0;
  	do 
  	{
  		thisround = write(desc, txt + sofar, total - sofar);
    	if( thisround < 0 ) 
		{
      		perror( "Write to socket" );
      		return(-1);
    	} 
		else if( thisround == 0 ) 
		{
      		log("Zero in write_to_descriptor");
    	} 
		else 
		{
      		sofar += thisround;
    	}
  	} while (sofar < total);

  	return(0);
}

static descriptorType * t;

int process_input(descriptorType *d)
{
	int sofar, thisround, begin, squelch, i, k, flag;
	char tmp[MAX_STRING_LENGTH+1], buffer[MAX_INPUT_LENGTH + 60];
	char snoop[MAX_INPUT_LENGTH+10];

	t = d;
	if( t->input.head ) return 0;
	sofar = 0;
	flag = 0;
	begin = strlen(t->buf);

	for( i = 0 ; i < MAX_INPUT_LENGTH + 2 ; i ++ ) tmp[i] = NULL ;

	do {
		if ((thisround = read(t->fd, 
						      t->buf + begin + sofar, 
                              MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0)
			sofar += thisround;    
		else if (thisround < 0)
		{
			if(errno != EWOULDBLOCK) 
			{
				return -1;
			}
			else
				break;
		}
		else 
		{
			log( "process_input> red EOF from %d.", t->fd );
			return(-1);
			
		}
	} while (!ISNEWL(*(t->buf + begin + sofar - 1)));  

	*(t->buf + begin + sofar) = 0;

	for( i = begin; !ISNEWL(*(t->buf + i)); i++ )
		if( !*(t->buf + i) )
			return(0);

	for (i = 0, k = 0; *(t->buf + i);) 
	{
		if( !ISNEWL(*(t->buf + i)) && !(flag = (k >= (MAX_INPUT_LENGTH - 2))) ) 
		{
			if(*(t->buf + i) == '\b') 
			{
				if (k) 
				{ 
					if (*(tmp + --k) == '$') k--;        
					i++;
				}
				else
					i++;  /* no or just one char.. Skip backsp */
			}
			else if( *(t->buf+i) == -1 ) 
			{
				log( "process_input> -1 entered.[-1][%d][%d][%c]",
						(int)*(t->buf+i+1), (int)*(t->buf+i+2), *(t->buf+i+3) );

				if( *(t->buf+i) && !ISNEWL(*(t->buf+i)) ) i++;
				if( *(t->buf + i) == NULL ) break ;
				if( *(t->buf+i) && !ISNEWL(*(t->buf+i)) ) i++;
				if( *(t->buf + i) == NULL ) break ;
				if( *(t->buf+i) && !ISNEWL(*(t->buf+i)) ) i++;
				if( *(t->buf + i) == NULL ) break ;
			}
			else if( (*(t->buf+i) & 0x80) && !ISNEWL( *(t->buf+i+1) ) ) 
			{
				*(tmp + k++) = *(t->buf + i++);
				if( *(t->buf + i ) == NULL ) break ;
				*(tmp + k++) = *(t->buf + i++);
				if( *(t->buf + i ) == NULL ) break ;
			}
			else if( isascii( *(t->buf + i) ) && isprint( *(t->buf + i) ) ) 
			{
				if( (*( tmp + k ) = *( t->buf + i ) ) == '$' ) *(tmp + ++k) = '$';
				k++;
				i++;
			}
			else
				i++;
		}
		else 
		{
			*(tmp + k) = 0;

			if( *tmp == '!' )	strcpy( tmp, t->last_input );
			else 				strcpy( t->last_input, tmp );

			put_to_input( t, tmp );

			if( t->snoop.by ) 
			{
				sprintf( snoop, "%% %s\n\r", tmp );
				put_to_output( t->snoop.by->desc, snoop );
			}

			if( flag ) 
			{
				sprintf(buffer, "Line too long. Truncated to:\n\r%s\n\r", tmp);

				if( write_to_descriptor(t->fd, buffer) < 0)
					return(-1);

				for (; !ISNEWL(*(t->buf + i)); i++);
			}

			for(; ISNEWL(*(t->buf + i)); i++);

			for(squelch = 0;; squelch++)
				if ((*(t->buf + squelch) = *(t->buf + i + squelch)) == '\0')
					break;
			k = 0;
			i = 0;
		}
	}

	return(1);
}

void close_sockets( int s )
{
  	log("Closing all sockets.");
  	while(desc_list) close_socket(desc_list);
  	close(s);
}

void close_socket(descriptorType *d)
{
  	descriptorType 	*	tmp;

  	close(d->fd);

  	flush_queues(d);

	if (d->fd == last_desc) --last_desc;
  	if (d->snoop.who)
    	d->snoop.who->desc->snoop.by = 0;
  	if (d->snoop.by) 
  	{
      	send_to_char("Your victim is no longer among us.\n\r",d->snoop.by);
      	d->snoop.by->desc->snoop.who = 0;
    }
  	if( d->character )
  	{
    	if( d->connected == CON_PLYNG ) 
		{
		  	save_char(d->character, NOWHERE);
		  	act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
		  	log( "Closing link to: %s.", GET_NAME(d->character));
		  	senddf( 0, 41, "===> lost link - %s.", GET_NAME(d->character) );
      		d->character->desc = 0;
    	} 
		else 
		{
			if( d->character->nr > -3 )
		   		log( "Losing player: %s.", d->character->name );
			else
				log( "Losing player without name." );
			free_a_char(d->character);
    	}
	}
  	else
    	log( "close_socket> Losing descriptor without a char." );

  	if( next_to_process == d ) next_to_process = next_to_process->next;   

  	if( d == desc_list ) desc_list = desc_list->next;
  	else 
  	{
    	for( tmp = desc_list; (tmp->next != d) && tmp; tmp = tmp->next);
    
    	if( tmp ) tmp->next = d->next;
		else
		{
			FATAL( "close_socket> locating descriptor failed." );
		}
  	}
	free_desc( d );
}
