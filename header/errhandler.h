#ifndef __UTILS_H
#define __UTILS_H

#ifndef __STDIO_H
#include <stdio.h>
#endif

/* in errhandler.c */

void * 	errMalloc	( size_t );    
void * 	errCalloc	( size_t );    
void * 	errRealloc	( void *, size_t );    
void   	errFree		( void * );      
FILE * 	errOpen		( const char * fname, char * mode );
void   	errClose	( FILE * );     

/* utility.c */

int		dice		( int ndice, int sdice );
int		number		( int from, int to );
int		strn_cmp	( char *, char *, int );
int		str_cmp		( char *, char * );
int		stricmp		( const char *, const char * );
int		strincmp	( const char *, const char *, int lenght );

char *  remove_newline( char * );

void 	sprinttype	(int type, char *names[], char *result);
void 	sprintbit	(long vektor, char *names[], char *result);

char *  itemstatus  ( objectType * obj );

int		isprefix( const char * str, const char * to_compare );

struct time_info_data age	(charType *ch);

int  	file_to_string( char * fname, char * buf );
char *	skip_spaces( char * );

/* weather.c */

void weather_and_time(int mode);

void set_title(charType *ch); 
struct time_info_data age(charType *ch);

#endif/*__UTILS_H*/
