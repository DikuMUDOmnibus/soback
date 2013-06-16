#ifndef __DEFINES_H
#define __DEFINES_H

#ifndef __SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef __STDIO_H
#include <stdio.h>
#endif

typedef int            	bool;  
typedef unsigned char   byte;
typedef unsigned int	word;
typedef unsigned long  dword;

#ifndef 	TRUE
#define 	TRUE		(1)
#endif

#ifndef     FALSE
#define 	FALSE		(0)
#endif

#define SUCCESS			1
#define FAIL			0
  
#define NIL			    0
#define NOKEY		   -1
#define NOWHERE        -1
#define NIHIL			NilString

#define PULSE_ZONE      240
#define PULSE_MOBILE    40
#define PULSE_VIOLENCE  20
#define PULSE_ROUND	    20

#define MAX_STRING_LENGTH   4000
#define MAX_INPUT_LENGTH      80
#define MAX_DESCRIPTION		 239
#define MAX_TITLE			  79
#define MAX_NAME_LENGTH 	  12
#define MAX_PASSWORD		  10

#define MAX_DIRECTIONS  	   6
#define MAX_APPLY   		   4
#define MAX_SKILLS           150  /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define MAX_EQUIP             21
#define MAX_AFFECT            25
#define MAX_SAVINGS			   5
#define MAX_CONDITIONS		   3

#define VIRTUAL                1
#define REAL                   0

#ifdef __allocExternVariables__
#define	EXTERN
#define	INIT(x)		x
#else
#define EXTERN		extern
#define INIT(x)
#endif

#ifdef 		__TRACE__
#define 	TRACE(x)	x
#else
#define 	TRACE(x)
#endif

#define 	DEBUG		debug
#define 	__DEBUG(x)	x

#define		ERROR		error
#define		FATAL		fatal

extern void 	debug			( const char * fmt, ... );
extern void 	log				( const char * fmt, ... );
extern void 	trace			( const char * fmt, ... ); 
extern void 	error			( const char * fmt, ... );
extern void 	fatal			( const char * fmt, ... );
extern void		null_func		( void );

extern void *  	errMalloc  		( size_t );
extern void *  	errCalloc  		( size_t );
extern void *  	errRealloc 		( void *, size_t );
extern void    	errFree    		( void * );
extern FILE *  	errOpen    		( const char * fname, char * mode );
extern void    	errClose   		( FILE * );

extern char * 	BASE_DIR;
extern char * 	WORLD_FILE;
extern char * 	MOB_FILE;
extern char * 	OBJ_FILE;
extern char * 	ZONE_FILE;
extern char * 	SHOP_FILE;
extern char * 	ZONE_DIR;
extern char * 	CREDITS_FILE;
extern char * 	NEWS_FILE;
extern char * 	MOTD_FILE;
extern char * 	TODO_FILE;
extern char * 	PLAYER_FILE;
extern char * 	TIME_FILE;
extern char * 	TYPO_FILE;
extern char * 	BUG_FILE;
extern char * 	IDEA_FILE;
extern char * 	MESS_FILE;
extern char * 	SOCMESS_FILE;
extern char * 	HELP_KWRD_FILE;
extern char * 	HELP_PAGE_FILE;
extern char * 	INFO_FILE;
extern char * 	WIZARDS_FILE;
extern char * 	COOKIE_FILE;
extern char * 	BOARD_DIR;
extern char * 	MBOX_DIR;
extern char * 	STASH_DIR;
extern char * 	ROOM_STASH_DIR;
extern char * 	HELP_FILE;
extern char * 	HELP_KEY_FILE;
extern char * 	PAINT_DIR;

#define MAX_FORTUNE_COOKIE 100  /* size of cookie file (number of line) */

EXTERN int		exitWhenError 	INIT(=0); /* only used in errhandler.c */
EXTERN char *	NilString		INIT(=""); /* used instead of NULL pointer*/

#ifndef __MACROS_H
#include "macros.h"
#endif

#endif/*__DEFINES_H*/
