#ifndef __SOCKETS_H
#define __SOCKETS_H

#ifndef __DEFINES_H
#include "defines.h"
#endif

#ifndef __CHARACTER_H
#include "character.h"
#endif

#define CON_PLYNG    0      
#define CON_NME      1      
#define CON_NMECNF   2      
#define CON_PWDNRM   3      
#define CON_PWDGET   4      
#define CON_PWDCNF   5                
#define CON_QSEX     6
#define CON_RMOTD    7
#define CON_SLCT     8
#define CON_EXDSCR   9
#define CON_QCLASS   10
#define CON_LDEAD    11
#define CON_PWDNEW   12
#define CON_PWDNCNF  13
#define CON_DEL_PWD  14
#define CON_DEL_CONF 15
#define CON_REROLL   16
#define CON_CONFROLL 17
#define CON_END      18

#define DESC_HOSTNAME	40

struct txt_block 
{ 
	char *text;
	struct txt_block *next;  
};
   
struct txt_q 
{
	struct txt_block *head;
	struct txt_block *tail;
};
  
typedef struct 
{
	struct __chartype__ * who;
	struct __chartype__ * by;

} snoopType;

#define MAX_OUTQUE      128
struct fixedQueue
{ 
    char *  queue[ MAX_OUTQUE ];
    int     head;
    int     tail;
    int     used;
};
  
typedef struct __descriptortype__
{
    int         unit;
	int 		fd;              	/* file descriptor for socket */
	char 		host[DESC_HOSTNAME+1]; /* hostname                */
	char 		ip[DESC_HOSTNAME+1];   /* ip                   	  */
	int 		ncmds;
  	int			contime;
	char 		pwd[12];          	/* password                   */
	int 		pos;               	/* position in player-file    */
	int 		connected;         	/* mode of 'connectedness'    */

	int 		wait;              	/* wait for how many loops    */
	char **		str;            	/* for the modify-str system  */
	int 		max_str;           	/* -                          */
	int 		prompt; 			/* control of prompt-printing */
	int 		qSize;

	char 		buf[MAX_STRING_LENGTH];  	/* buffer for raw input */
	char 		last_input[MAX_INPUT_LENGTH];/* the last input      */

	snoopType 			snoop;      	/* to snoop people.          */
	struct fixedQueue 	output;     	/* q of strings to send      */
	struct txt_q 		input;          /* q of unprocessed input    */
	struct __chartype__ *	character;  	/* linked to char            */
	struct __chartype__ *	original;   	/* original char             */
	struct __descriptortype__ *	next; 	/* link to next descriptor   */

} descriptorType;


void		echo_local			( int fd );
void		no_echo_local		( int fd );

void		echo_telnet			( descriptorType * );
void		no_echo_telnet		( descriptorType * );

int 		init_socket			(int port);

void 		close_sockets		( int );
void 		close_socket		( descriptorType *d );

int 		new_connection		(int s);
int 		new_descriptor		(int s);

int 		write_to_descriptor	(int desc, char *txt);

int 		process_input		( descriptorType *t);
int 		process_output		( descriptorType *t);

int 		get_from_output		( descriptorType * d, char *txt );
int 		get_from_input		( descriptorType * d, char *txt );

void 		put_to_input		( descriptorType * d, char *txt );
void 		put_to_output		( descriptorType * d, char *txt );

void 		flush_inqueue		( descriptorType *d );
void 		flush_ouqueue		( descriptorType *d );

#define		SEND_TO_Q( a, b )   put_to_output( (b), (a) )

extern descriptorType * 	desc_list;
extern descriptorType * 	next_to_process;

extern int 					last_desc; 

#endif/*__SOCKETS_H*/
