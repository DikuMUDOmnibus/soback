#ifndef __STRINGS_H
#define __STRINGS_H

char * 	strlwr			( char * str );
char * 	capitalize		( char * cp  );
char * 	remove_newline	( char * str );

char * 	skipsps			( const char * str );
char * 	skipsps			( const char * str );
char * 	oneword			( const char * str );
char * 	onewordc		( const char * str );
char * 	onefword		( const char * str, char * buf );
char * 	oneArgument		( const char * arg, char * buf );
char * 	halfchop		( const char * str, char * arg1, char * arg2 );
char * 	twoArgument		( const char * str, char * arg1, char * arg2 );

int 	stricmp			( const char * from, const char * to );
// int 	strnicmp		( const char * from, const char * to, int len );

int 	isprefix		( const char * name, const char * compare );
int 	isinfix			( const char * name, const char * compare );
int 	isoneofp		( const char * name, const char * compare );
int 	isoneof			( const char * name, const char * compare );
int 	isinlistp		( const char * name, const char ** list );
int 	isinlist		( const char * name, const char ** list );
int 	isdirection		( const char * name );
int 	isfill			( const char * name );

int 	isnumstr		( const char * str );
char *	numfstr			( unsigned int );
char *	snumfstr		( int );
int 	getnumber		( const char * str, int * dig );
int 	dotnumber		( char ** str );
int 	splitarg		( char * str, char ** what );

int 	fread_number	( FILE *fp );
char * 	fread_string	( FILE * fl );
void	fwrite_string	( FILE * fp, char * );
int 	file_to_buf		( char *name, char *buf );

#endif/*__STRINGS_H*/
