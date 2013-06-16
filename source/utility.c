/**************************************************************************
*  file: utility.c, Utility module.                       Part of DIKUMUD *
*  Usage: Utility procedures                                              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "limits.h"
#include "utils.h"
#include "variables.h"
#include "strings.h"

void name_from_drinkcon( objectType *obj )
{
	char	dummy[256];

	obj->name = strdup( onefword( obj->name, dummy ) ); 
}

void name_to_drinkcon( objectType * obj, int type )
{
  	char *new_name;

    new_name = errMalloc(strlen(obj->name)+strlen(drinknames[type])+2);
   
    if( new_name )
    {
        sprintf(new_name,"%s %s",drinknames[type],obj->name);
        obj->name=new_name;
    }
    else
    {
        obj->name = 0;
    }
}

#define READ_TITLE(ch) (GET_SEX(ch) == SEX_MALE ?   \
  titles[(int)GET_CLASS(ch)-1][(int)GET_LEVEL(ch)].title_m :  \
  titles[(int)GET_CLASS(ch)-1][(int)GET_LEVEL(ch)].title_f)

void set_title(charType *ch) 
{   
    if( GET_TITLE(ch) ) 
    {   
        errFree(GET_TITLE(ch)) ;
        ch->title = (char *)errMalloc(strlen(READ_TITLE(ch)) + 1);
    }    
    else
    {
        ch->title = (char *)errMalloc(strlen(READ_TITLE(ch)) + 1);
    }
    strcpy(GET_TITLE(ch), READ_TITLE(ch));       
}   

char *how_good(int percent)
{
    static char buf[64];
  
    sprintf(buf,"(%d)",percent);
    return (buf);
} 
                                      

char * itemstatus( objectType * obj )
{
	char * status;

    if( obj->status < 10 ) status = "Awful";
    else
    if( obj->status < 30 ) status = "Bad";
    else
    if( obj->status < 45 ) status = "Poor";
    else  
    if( obj->status < 55 ) status = "Average";
    else
    if( obj->status < 65 ) status = "Fair"; 
    else
    if( obj->status < 80 ) status = "Good"; 
    else
    if( obj->status < 95 ) status = "Very Good";
    else
        status = "Excellent";

	return status;
}	

int number(int from, int to) 
{
  	register int d, tmp;

  	if( from == to ) return from ;

  	if( from > to ) 
  	{
    	tmp  = from; 	from = to; 	to   = tmp;
    }

  	d = to-from + 1;

  	if(d < 2) d = 2;

  	return ((rand() % d) + from);
}

int dice(int number, int size) 
{
  	int r;
  	int sum = 0;

	if(size < 1) return(1);

  	for (r = 0; r < number; r++) sum += ((rand() % size)+1);
  	return(sum);
}

char * sprintbit( long vektor, char *names[], char * buf )
{
	static	char		localbuf[ 1024 ];
	char	*			result;
  	int 				nr;

	if( !buf ) 	result = localbuf;
	else		result = buf;

  	*result = '\0';

  	for( nr = 0; vektor; vektor >>= 1 )
  	{
    	if( IS_SET(1, vektor) )
		{
      		if( *names[nr] != '\n' ) 
	  		{
        		strcat( result, names[nr] );
        		strcat( result," ");
      		} 
	  		else 
			{
        		strcat( result, "UNDEFINED" );
        		strcat( result, " " );
      		}
	  	}
   		if( *names[nr] != '\n' ) nr++;
  	}

  	if( !*result ) strcpy(result, "");

	return result;
}

char * sprinttype(int type, char *names[], char * buf )
{
	static	char	localbuf[ 1024 ];
	char		*	result;

  	int 		nr;

	if( !buf )  result = localbuf;
	else		result = buf;

  	for( nr = 0; (*names[nr]!='\n'); nr++)
  	;
  	if( type < nr ) strcpy(result, names[type] );
  	else 			strcpy(result, "UNDEFINED" );

  	return result;
}

char * see_whom( charType * ch, charType * obj )
{
	if( can_see( ch, obj ) )
	{
		if( IS_NPC( obj ) && obj->moved ) return obj->moved;
		else			    			  return obj->name;
	}

	return "someone";
}
