#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "defines.h"
#include "strings.h"

int isnumstr( const char * str )
{ 
  	if( !*str )  return(0);

	if( *str == '-' ) 
	{
		if( !isdigit( *++str ) ) return 0;
	}

  	for( ;*str ; str++ ) if( !isdigit( *str ) ) return 0;

  	return 1;    
} 

int	getnumber( const char * str, int * dig )
{
	if( !isnumstr( str ) ) return 0;

	*dig = atoi( str );

	if( *dig >= 0 ) return  1;
	else		    return -1;
}

int dotnumber( char ** name )
{
    int         i = 1, j;
    char      * cp;

    if( cp = strchr( *name, '.' ), cp ) 
    {
        *cp = '\0';

		j = getnumber( *name, &i );

		*cp++ = '.';

		if( !j || i <= 0 ) return 0;

		*name = cp;
    }
    return i;
}

int splitarg(  char * str, char ** arg )
{
	char	*   dummy;
	char	**	what;
	char 	* 	cp, c;
	int			nr  = 0;

	if( !arg ) what = &dummy;
	else	   what = arg;

	if( !*str ) return  -1;									/* '\0' */

	if( isnumstr( str ) ) return -2;						/* 1 */

	if( strncasecmp( str, "all.", 4 ) == 0  )					/* all.abc */
	{
		if( *what = str + 4, !*what ) nr = -3;				/* 1. */
	}
	else if( strncasecmp( str, "a.", 2 ) == 0  )					/* all.abc */
	{
		if( *what = str + 2, !*what ) nr = -3;				/* 1. */
	}
	else if( strncasecmp( str, "*.", 2 ) == 0 )
	{
		if( *what = str + 2, !*what ) nr = -3;
	}
	else if( stricmp( str, "all" ) == 0 ) *what = str + 3;	/* all */
	else if( strcmp( str, "*" ) == 0 )    *what = str + 1;	/* all */
	else
	{
		if( cp = strchr( str, '.' ), cp && cp != str )
		{
			c = *cp; *cp = 0;

			if( getnumber( str, &nr ) ) 
			{
				if( nr <= 0 ) 	nr = -4;					/* 0.abc */
				else 			*what = cp + 1;				/* 1.abc */
			}
			else			   	nr = -5;					/* a.bc */
	
			*cp = c;
		}
		else
		{
			nr = 1; *what = str;							/* abc */
		}
	}
	if( nr < 0 ) *what = NilString;
	return nr;
}

char * skipsps( const char * str )
{
	char 	* 	cp;

	cp = (char *)str;

	if( !cp  ) return NilString;
	if( !*cp ) return cp;

	for( ; *cp; cp++ )
		if( *cp == ' ' || *cp == '\t' || *cp == '\n' || *cp == '\r' );
		else break;

	return cp;
}

char * strlwr( char * str )
{
	char * cp;

	for( cp = str; *cp; cp++ )
	{
		if( (*cp & 0x80) && *(cp+1) )
		{
			cp++; continue;
		}
		*cp = tolower( *cp );
	}
	return str;
}

char * capitalize( char * cp )
{
	if( !cp  ) return NilString;

	cp = skipsps( cp );

	if( !*cp || (*cp & 0x80) ) return cp;

	*cp = toupper( *cp );

	return cp;
}

char * remove_newline( char * str )
{   
    char * cp;
    
    cp = str + strlen( str );
    
    while( *(cp - 1) == '\n' || *(cp - 1) == '\r' ) cp--;

    *cp = 0;

    return str;
}

char * oneword( const char * str )
{
	static	char 	buf[MAX_STRING_LENGTH];
			char  * cp, * dp;

	cp = skipsps( str );


	for( dp = buf; *cp; cp++, dp++ )
	{
		if( *cp == ' ' || *cp == '\t' || *cp == '\n' || *cp == '\r' ) break;

		*dp = *cp;
	}
	*dp = 0;

	return buf;
}

char * onewordc( const char * str )
{
	return capitalize( oneword( str ) );
}

char * onefword( const char * str , char * buf )
{
	char 		* cp, * dp;

	cp = skipsps( str );

	for( dp = buf; *cp; cp++, dp++ )
	{
		if( *cp == ' ' || *cp == '\t' || *cp == '\n' || *cp == '\r' ) break;
		
		*dp = *cp;
	}

	*dp = 0;

	cp = skipsps( cp );

	return cp;
}

int	stricmp( const char * from, const char * to )
{
	int		diff, tlen, flen;

	flen = strlen( from );
	tlen = strlen( to );

	for( ; flen && tlen; tlen--, flen-- )
	{
		if( (*(from+1) & 0x80) && (*to & 0x80) )
		{
			if( flen >= 2 && tlen >= 2 )
			{
				if( (*from == *to) && (*(from+1) == *(to+1)) )
				{
					from += 2; to += 2;	
					tlen--; flen--;
					continue;
				}
			}
		}

		if( diff = tolower(*from++) - tolower(*to++), diff ) return diff;
	}

	return (*from - *to);
}
#if 0
int strncasecmp( const char * from, const char * to, int len )
{  
    int     diff, tlen, flen;
   
   	flen = strlen( from );
   	tlen = strlen( to );

    for(; len && tlen && flen; flen--, tlen--, len-- )
    {
		if( (*from & 0x80) && (*to & 0x80) )
		{
			if( flen >= 2 && tlen >= 2 )
			{
				if( (*from == *to) && (*(from+1) == *(to+1)) )
				{
					from += 2, to += 2; 
					flen--; tlen--; len--;
					continue;
				}
			}
		}

        if( diff = tolower(*from++) - tolower(*to++), diff ) return diff;
    }

	if( !len ) return 0;

	return (*from - *to);
}
#endif

int isprefix( const char * name, const char * compare )
{
	int len;

	if( !*name || !*compare ) return 0;

	len = strlen( name );

	if( strncasecmp( name, compare, len ) == 0 ) return len;

	return 0;
}

int isexact( const char * name, const char * compare )
{
	if( !*name || !*compare ) return 0;

	if( stricmp( name, compare ) == 0 ) return 1;

	return 0;
}

int	isoneofp( const char * name, const char * compare )
{
	char		buf[256]; 	

	while( compare = onefword( compare, buf ), *buf )	
	{
		if( isprefix( name, buf ) ) return 1;
	}
	return 0;
}

int	isoneof( const char * name, const char * compare )
{
	char 		buf[256];

	while( compare = onefword( compare, buf ), *buf )
	{
		if( isexact( name, buf ) ) return 1;
	}
	return 0;
}

int isinlistp( const char * name, const char ** list )
{
	int			i;
	
	if( !*name ) return -1;

	for( i = 0; list[i] && list[i][0] != '\n'; i++ )
	{
		if( isprefix( name, list[i] ) ) return i;
	}
	return -1;
}	

int isinlist( const char * name, const char ** list )
{
	int			i;
	
	if( !*name ) return -1;

	for( i = 0; list[i] && list[i][0] != '\n'; i++ )
	{
		if( isexact( name, list[i] ) ) return i;
	}
	return -1;
}	

int isdirection( const char * name )
{
	static const char * directions[] =
	{
		"north",
		"east",
		"south",
		"west",
		"up",
		"down",
		"\n"
	};

	return isinlistp( name, directions ) + 1;
}

int isfill( const char * name )
{
	static const char * fills[] =
	{
		"in",
		"on",
		"at",
		"to",
		"into",
		"from",
		"with",
		"an",
		"the",
		"\n"
	};

	return isinlist( name, fills ) + 1;
}

char * oneArgument( const char * args, char * buf )
{
	char 	* 	cp = (char *)args;

	do
	{
		cp = onefword( cp, buf );
	
	} while( isfill( buf ) );

	return cp;
}

char * halfchop( const char * str, char * arg1, char * arg2 )
{
	char	*	cp;

	cp = onefword( str, arg1 );
	strcpy( arg2, cp );

	return cp;
}

char * twoArgument( const char * str, char * arg1, char * arg2 )
{
	char	*	cp;

	cp = oneArgument( str, arg1 );
	cp = oneArgument( cp,  arg2 );

	return cp;
}

int fread_number( FILE *fp )
{
    int     number;
    bool    sign;
    char    c;

    do { c = getc( fp ); } while ( isspace(c) );

    number = 0;

    sign   = FALSE;

    if ( c == '+' )                   c = getc( fp );
    else if ( c == '-' ) sign = TRUE, c = getc( fp );

    if ( !isdigit(c) )
    {
        FATAL( "fread_number. bad format.", 0 );
    }

    while ( isdigit(c) )
    {
        number = number * 10 + c - '0';
        c      = getc( fp );
    }

    if ( sign )     number = 0 - number;

    if ( c == '|' )         number += fread_number( fp );
    else if ( c != ' ' )    ungetc( c, fp );

    return number;
}

char * fread_string( FILE * fl )
{ 
  	char 					buf[MAX_STRING_LENGTH]; 
	char					tmp[500]; 
  	char 				*	rslt; 
  	register char 		*	point;
  	int 					flag;
  
  	memset(buf, 0, MAX_STRING_LENGTH);

  	do
  	{
    	if( !fgets(tmp, MAX_STRING_LENGTH, fl) )
    	{
			FATAL( "fread_string> read error." );
    	}
    	if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
    	{
      		FATAL( "fread_string> string(%s) too large.", buf );
    	} 
    	else
      		strcat(buf, tmp);
      
    	for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point); point--)
		;

    	if((flag = (*point == '~')))
		{
      		if (*(buf + strlen(buf) - 3) == '\n')
      		{  
        		*(buf + strlen(buf) - 2) = '\r'; 
        		*(buf + strlen(buf) - 1) = '\0';
      		}   
      		else
        		*(buf + strlen(buf) -2) = '\0';
		}
    	else  
    	{     
      		*(buf + strlen(buf) + 1) = '\0';
      		*(buf + strlen(buf)) = '\r';
   		}       
  	}
  	while (!flag);

  	/* do the allocate boogie  */

  	if (strlen(buf) > 0)
  	{
    	rslt = errMalloc(strlen(buf) + 1) ;
    	strcpy(rslt, buf);
  	}
  	else
    	rslt = 0;
  	return(rslt);
}

void fwrite_string( FILE * fp, char * str )
{
	char		buf[MAX_STRING_LENGTH];
	char	* 	cp, * dp;

	if( !str )
	{
		fprintf( fp, "~\n" ); return;
	}

	if( strlen( str ) > MAX_STRING_LENGTH - 1 ) 
	{
		DEBUG( "fwrite_string> string too long. %d - %s", strlen( str ), str );
	}

	for( cp = str, dp = buf; *cp; cp++ )
	{
		if( *cp == '\r' ) continue;

		*dp++ = *cp;
	}

	*dp = 0;

	fprintf( fp, "%s~\n", buf );
}
		

int file_to_buf( char *name, char * buf )
{
  	FILE 	*	fl;
  	char 		tmp[200];

  	*buf = '\0';
  
  	if (!(fl = errOpen(name, "r")))
  	{ 
    	*buf = '\0'; return(-1);
  	}   

  	do  
  	{   
    	fgets( tmp, 199, fl ); 

    	if( !feof(fl) )
    	{
      		if( strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH)
      		{
        		buf[25]='\0'; DEBUG( "file_to_buf> string too long %s", buf ); *buf = '\0';
        		fclose(fl);
        		return(-1);
      		}

      		strcat(buf, tmp);
      		*(buf + strlen(buf) + 1) = '\0';
      		*(buf + strlen(buf)) = '\r';
    	}
  	}
  	while (!feof(fl));

  	fclose(fl);

  	return(0);
}

char * numfstr( unsigned int nr )
{
	static	char 	buf[33];
	int				i, j;

    if( nr == 0 ) 
    {
        strcpy( buf, "0" );
        return buf;
    }

	memset( buf, ' ', 31 );

	for( i = 31, buf[32] = 0 ; nr && i > 0; i-- )
	{
		j = nr % 10;

		if( i % 4 == 0 ) buf[i--] = ',';
		buf[i] = '0' + j;
		nr /= 10;
	}

	return buf+i+1;
}

char * snumfstr( int nr )
{
	static	char 	buf[33];
	int				i, j;
	int				sign = 0;

	if( nr < 0 )
	{
		sign = 1; nr = -nr;
	}

    if( nr == 0 ) 
    {
        strcpy( buf, "0" );
        return buf;
    }

	memset( buf, ' ', 31 );

	for( i = 31, buf[32] = 0 ; nr && i > 0; i-- )
	{
		j = nr % 10;

		if( i % 4 == 0 ) buf[i--] = ',';
		buf[i] = '0' + j;
		nr /= 10;
	}

	if( sign ) buf[--i] = '-';

	return buf+i+1;
}
