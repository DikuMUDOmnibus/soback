#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "comm.h"
#include "sockets.h"
#include "utils.h"
#include "limits.h"
#include "find.h"
#include "variables.h"
#include "strings.h"
#include "zone.h"

void misbehave( charType * ch )
{
    sendf( ch, "You cannot do that here." );
	act("$n attempts to misbehave here.", FALSE, ch, 0, ch, TO_NOTVICT);
}

void send_to_char(char *messg, charType *ch)
{
  if ( ch && ch->desc && messg)
    put_to_output( ch->desc, messg );
}

void send_to_char_han(char *msgeng, char *msghan, charType *ch)
{
    
  if (ch && ch->desc && msgeng && msghan ) {
     if ((IS_SET(ch->act, PLR_KOREAN)))	/* korean */
        put_to_output( ch->desc, msghan );
     else						/* english */
        put_to_output( ch->desc, msgeng );
     }
}

void send_to_outdoor(char * messg )
{
  descriptorType *i;

  if (messg)
    for (i = desc_list; i; i = i->next)
      if (!i->connected)
        if (OUTSIDE(i->character))
          put_to_output( i, messg );
}

void sendf( const charType * ch, const char * fmt, ... )
{  
    va_list     args;
    char        buf[4096];
    
  	if( ch && ch->desc && fmt )
	{
		va_start( args, fmt );
		vsprintf( buf, fmt, args );       

		strcat( buf, "\n\r" );
		put_to_output( ch->desc, buf );
	}  
}

void sendrf( const charType * ch, int room, const char * fmt, ... )
{
    va_list     	args;
    char        	buf[4096];
  	charType  	*	i;

  	if( fmt )
	{
		va_start( args, fmt );
		vsprintf( buf, fmt, args );       

		strcat( buf, "\n\r" );

    	for (i = world[room].people; i; i = i->next_in_room)
      		if(  i->desc && i != ch )
        		put_to_output( i->desc, buf );
	}
}

void sendzf( const charType * ch, int zoneNr, const char * fmt, ... )
{
    va_list     	args;
    char        	buf[4096];
  	charType 	* 	to;
  	zoneType	*   zone;
  	roomType	*	room;

  	zone = &zones[zoneNr];

  	if( fmt )
	{
		va_start( args, fmt );
		vsprintf( buf, fmt, args );       

		strcat( buf, "\n\r" );

    	for( room = zone->rooms; room; room = room->next )
    		for( to = room->people; to; to = to->next_in_room)
      			if( to->desc && to != ch )
        			put_to_output( to->desc, buf );
	}
}

void sendwf( const charType * ch, const char *  fmt, ... )
{
    va_list     	args;
    char        	buf[4096];
  	charType 	* 	to;
  	int				room;

  	if( fmt )
	{
		va_start( args, fmt );
		vsprintf( buf, fmt, args );       

		strcat( buf, "\n\r" );

		for( room = 1; room < rooms_in_world; room++ )
    		for( to = world[room].people; to; to = to->next_in_room )
      			if( to->desc && to != ch )
        			put_to_output( to->desc, buf );
	}
}

void senddf( const charType * ch, int level, const char * fmt, ... )
{
    va_list     		args;
    char        		buf[4096];
  	descriptorType 	*	desc;

  	if( fmt )
	{
		va_start( args, fmt );
		vsprintf( buf, fmt, args );       

		strcat( buf, "\n\r" );

    	for( desc = desc_list; desc; desc = desc->next )
		{
			if( desc->connected == CON_PLYNG 
             && desc->character 
			 && desc->character != ch
			 && desc->character->level >= level )
        		put_to_output( desc, buf );
		}
	}
}


void act(char *str, int hide_invisible, charType *ch, objectType *obj, void *vict_obj, int type)
{
  	register char 	*	strp, *point, *i;
  	charType 		*	to;
  	char 				buf[MAX_STRING_LENGTH];

  	if( !IS_STR( str ) ) return;

  	if( type == TO_VICT ) to = (charType *) vict_obj;
  	else if( type == TO_CHAR ) to = ch;
  	else to = world[ch->in_room].people;

  	for(; to; to = to->next_in_room )
  	{
    	if(   to->desc 
		  && ((to != ch) || (type == TO_CHAR)) 
          && (can_see(to, ch) || !hide_invisible) 
		  && AWAKE(to) 
          && !( (type == TO_NOTVICT) && (to == (charType *)vict_obj) ) )
    	{
      		for( strp = str, point = buf;; )
        		if( *strp == '$' )
        		{
          			switch (*(++strp))
          			{
            			case 'n': i = PERS(ch, to); break;
            			case 'N': i = PERS((charType *) vict_obj, to); break;
            			case 'm': i = HMHR(ch); break;
            			case 'M': i = HMHR((charType *) vict_obj); break;
            			case 's': i = HSHR(ch); break;
            			case 'S': i = HSHR((charType *) vict_obj); break;
            			case 'e': i = HSSH(ch); break;
            			case 'E': i = HSSH((charType *) vict_obj); break;
            			case 'o': i = OBJN(obj, to); break;
            			case 'O': i = OBJN((objectType *) vict_obj, to); break;
            			case 'p': i = OBJS(obj, to); break;
            			case 'P': i = OBJS((objectType *) vict_obj, to); break;
            			case 'a': i = SANA(obj); break;
            			case 'A': i = SANA((objectType *) vict_obj); break;
            			case 'T': i = (char *) obj; break;
            			case 'F': i = oneword((char *) vict_obj); break;
            			case '$': i = "$"; break;
            			default:  i = " ";
              					log( "Illegal $-code to act(%s):", str ); break;
          			}
          			while((*point = *(i++))) ++point;
          			++strp;
        		}
        		else if ( !(*point++ = *strp++) ) break;

      		*(--point) = '\n';
      		*(++point) = '\r';
      		*(++point) = '\0';

      		put_to_output( to->desc, capitalize(buf) );
    	}
    	if ((type == TO_VICT) || (type == TO_CHAR)) return;
  	}
}

void acthan(char *streng,char *strhan,int hide_invisible,charType *ch,
  objectType *obj, void *vict_obj, int type)
{
  register char *strp, *str, *point, *i;
  charType *to;
  char buf[MAX_STRING_LENGTH];

  if (!streng || !strhan)
    return;
  if (!*streng || !*strhan)
    return;

  if (type == TO_VICT)
    to = (charType *) vict_obj;
  else if (type == TO_CHAR)
    to = ch;
  else
    to = world[ch->in_room].people;

  for (; to; to = to->next_in_room)
  {
    if (to->desc && ((to != ch) || (type == TO_CHAR)) &&  
      (can_see(to, ch) || !hide_invisible) && AWAKE(to) &&
      !((type == TO_NOTVICT) && (to == (charType *) vict_obj)))
    {
      if ((IS_SET(to->act, PLR_KOREAN))) {
        str = strhan ;
        }
      else {
        str = streng ;
        }
      for (strp = str, point = buf;;)
        if (*strp == '$')
        {
          switch (*(++strp))
          {
            case 'n': i = PERS(ch, to); break;
            case 'N': i = PERS((charType *) vict_obj, to); break;
            case 'm': i = HMHR(ch); break;
            case 'M': i = HMHR((charType *) vict_obj); break;
            case 's': i = HSHR(ch); break;
            case 'S': i = HSHR((charType *) vict_obj); break;
            case 'e': i = HSSH(ch); break;
            case 'E': i = HSSH((charType *) vict_obj); break;
            case 'o': i = OBJN(obj, to); break;
            case 'O': i = OBJN((objectType *) vict_obj, to); break;
            case 'p': i = OBJS(obj, to); break;
            case 'P': i = OBJS((objectType *) vict_obj, to); break;
            case 'a': i = SANA(obj); break;
            case 'A': i = SANA((objectType *) vict_obj); break;
            case 'T': i = (char *) obj; break;
            case 'F': i = oneword((char *) vict_obj); break;
            case '$': i = "$"; break;
            default : i = " ";
              log("Illegal $-code to act( %s ):", str );
              break;
          }
          while( (*point = *(i++)) )
            ++point;
          ++strp;
        }
        else if (!(*(point++) = *(strp++)))
          break;

      *(--point) = '\n';
      *(++point) = '\r';
      *(++point) = '\0';

      put_to_output( to->desc, capitalize(buf));
    }
    if ((type == TO_VICT) || (type == TO_CHAR))
      return;
  }
}

void acthanbrief(char *streng,char *strhan,char *strengbrief,char *strhanbrief,
  int hide_invisible,charType *ch,
  objectType *obj, void *vict_obj, int type)
{
  register char *strp, *str, *point, *i;
  charType *to;
  char buf[MAX_STRING_LENGTH];

  if (!streng || !strhan || !strengbrief || !strhanbrief)
    return;
  if (!*streng || !*strhan || !*strengbrief || !*strhanbrief)
    return;

  if (type == TO_VICT)
    to = (charType *) vict_obj;
  else if (type == TO_CHAR)
    to = ch;
  else
    to = world[ch->in_room].people;

  for (; to; to = to->next_in_room)
  {
    if (to->desc && ((to != ch) || (type == TO_CHAR)) &&  
      (can_see(to, ch) || !hide_invisible) && AWAKE(to) &&
      !((type == TO_NOTVICT) && (to == (charType *) vict_obj)))
    {
      if ((IS_SET(to->act, PLR_KOREAN))) {
        if ((IS_SET(to->act, PLR_BRIEF))) {
          str = strhanbrief ;
          }
        else {
          str = strhan ;
          }
        }
      else {
        if ((IS_SET(to->act, PLR_BRIEF))) {
          str = strengbrief ;
          }
        else {
          str = streng ;
          }
        }
      for (strp = str, point = buf;;)
        if (*strp == '$')
        {
          switch (*(++strp))
          {
            case 'n': i = PERS(ch, to); break;
            case 'N': i = PERS((charType *) vict_obj, to); break;
            case 'm': i = HMHR(ch); break;
            case 'M': i = HMHR((charType *) vict_obj); break;
            case 's': i = HSHR(ch); break;
            case 'S': i = HSHR((charType *) vict_obj); break;
            case 'e': i = HSSH(ch); break;
            case 'E': i = HSSH((charType *) vict_obj); break;
            case 'o': i = OBJN(obj, to); break;
            case 'O': i = OBJN((objectType *) vict_obj, to); break;
            case 'p': i = OBJS(obj, to); break;
            case 'P': i = OBJS((objectType *) vict_obj, to); break;
            case 'a': i = SANA(obj); break;
            case 'A': i = SANA((objectType *) vict_obj); break;
            case 'T': i = (char *) obj; break;
            case 'F': i = oneword((char *) vict_obj); break;
            case '$': i = "$"; break;
            default : i = " ";
              log("Illegal $-code to act( %s ):", str );
              break;
          }
          while( (*point = *(i++)))
            ++point;
          ++strp;
        }
        else if (!(*(point++) = *(strp++)))
          break;

      *(--point) = '\n';
      *(++point) = '\r';
      *(++point) = '\0';

      put_to_output( to->desc, capitalize(buf) );
    }
    if ((type == TO_VICT) || (type == TO_CHAR))
      return;
  }
}
