#ifndef __COMM_H
#define __COMM_H

#ifndef __DEFINES_H
#include "defines.h"
#endif

#ifndef __CHARACTER_H
#include "character.h"
#endif

#ifndef __OBJECT_H
#include "object.h"
#endif

void 	send_to_char	( char *messg, charType * );
void 	send_to_char_han( char *msg, char *han, charType * );
void 	send_to_outdoor	( char *messg );

void 	sendf			( const charType *, const char *, ... );
void 	sendrf			( const charType * ch, int room, const char * fmt, ... );
void 	sendzf			( const charType * ch, int zone, const char * fmt, ... );
void 	senddf			( const charType * ch, int level, const char * fmt, ... );
void 	sendwf			( const charType * ch, const char * fmt, ... );

void 	misbehave		( charType * ch );

#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3

void 	act				( char *, int see, charType *,	objectType *, void * vict, int how );
void 	acthan			( char * eng, char * han, int see, 
							charType *, objectType *, void * vict, int how );
void 	acthanbrief		( char * eng, char * han, char * engb, char * hanb,
       						int see, charType *,objectType *, void * vict, int how );

#endif/*__COMM_H*/
