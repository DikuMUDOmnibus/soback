#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "defines.h"
#include "character.h"
#include "object.h"
#include "mobile.h"
#include "nanny.h"
#include "utils.h"
#include "interpreter.h"
#include "find.h"
#include "sockets.h"
#include "comm.h"
#include "variables.h"
#include "strings.h"

#define TP_MOB    0
#define TP_OBJ    1
#define TP_ERROR  2

const char *string_fields[] =
{
  "name",
  "short",
  "long",
  "description",
  "title",
  "delete-description",
  "\n"
};

const int length[] =
{
  15,
  60,
  256,
  240,
  60
};

const char *skill_fields[] = 
{
  "learned",
  "affected",
  "duration",
  "recognize",
  "\n"
};

const int max_value[] =
{
  255,
  255,
  10000,
  1
};

/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */

void string_add( descriptorType *d, char * str )
{
	charType	*	ch = d->character;
  	int 			terminator = 0, length = 0;
	char 			tmp_buf[ MAX_STRING_LENGTH ] ;

  	/* determine if this is the terminal string, and truncate if so */

  	if( (terminator = (*str == '@')) ) *str = '\0';
  
  	if( !*d->str )
  	{
    	if( strlen(str) > d->max_str )
    	{
      		sendf( ch, "String too long - Truncated." );
      		*(str + d->max_str) = '\0';
      		terminator = 1;
    	}
    	*d->str = (char *)errMalloc(strlen(str) + 3);
	
    	strcpy( *d->str, str );
  	}
  	else
  	{
    	if( strlen(str) + strlen(*d->str) > d->max_str )
    	{
      		sendf( ch, "String too long. Last line skipped." );
      		terminator = 1;
    	}
    	else 
    	{
			strcpy( tmp_buf, *d->str );
			length = strlen( *d->str );

			errFree( *d->str ) ;

			*d->str =(char *)errMalloc( length + strlen(str) + 3 );

			if( *d->str == NULL) 
      		{
        		FATAL("string_add> allocating %d failed", length + strlen(str) + 3 );
      		}
	   		strcpy( *d->str, tmp_buf );
      		strcat( *d->str, str );
    	}
  	}

  	if( terminator )
  	{
    	d->str = 0;
    	if( d->connected == CON_EXDSCR )
    	{
      		SEND_TO_Q(MENU, d);
      		d->connected = CON_SLCT;
    	}
  	}
  	else
     	strcat( *d->str, "\n\r" );
}

#undef MAX_STR

/* interpret an argument for do_string */
void quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
  	char 	buf[ MAX_STRING_LENGTH ];

  	arg = oneArgument(arg, buf);

  	if( isprefix( buf, "char" ) ) 		*type = TP_MOB;
  	else if( isprefix( buf, "obj" ) ) 	*type = TP_OBJ;
  	else
  	{
    	*type = TP_ERROR; return;
  	}

  	arg = oneArgument(arg, name);

  	arg = oneArgument(arg, buf);

  	if( !(*field = isinlistp( buf, string_fields )) ) return;

  	for (; isspace(*arg); arg++);
  	for (; (*string = *arg); arg++, string++);

  	return;
}
  
void do_string(charType *ch, char *arg, int cmd)
{
  	char 					name[MAX_INPUT_LENGTH + 1],  string[MAX_INPUT_LENGTH + 1];
  	int 					field, type;
  	charType 			* 	mob;
  	objectType 			* 	obj;
  	exdescriptionType 	* 	ed, * tmp;

  	if( IS_NPC(ch) ) return;

  	quad_arg(arg, &type, name, &field, string);

  	if( type == TP_ERROR) 
  	{
    	sendf( ch, "string <obj | char> <name> <field> string." );
    	return;
  	}

  	if( !field ) 
  	{
    	sendf( ch, "No field by that name. Try 'help string'." );
    	return;
  	}

  	if( type == TP_MOB ) 
  	{
    	if( !(mob = find_char(ch, name)) ) 
		{
      		sendf( ch, "I don't know anyone by that name..." );
      		return;
    	}

    	switch( field ) 
		{
      		case 1: if( !IS_NPC(mob) && GET_LEVEL(ch) < (IMO+3) ) 
	  				{
          				sendf( ch, "You can't change that field for players." );
          				return;
        			}
        			if( !IS_NPC(mob) )
					{
          				sendf( mob, "WARNING: You have changed the name of a ");
					}
					else
					{
						mob->name = strdup( mob->name );
					}
					ch->desc->str = &mob->name;
      				break;
      		case 2: if( !IS_NPC(mob) ) 
	  				{
          				sendf( ch, "That field is for monsters only." );
           				return;
         			}
		 			mob->moved = strdup( mob->moved );
         			ch->desc->str = &mob->moved;
      				break;
      		case 3: if( !IS_NPC(mob) ) 
	  				{
        				sendf( ch, "That field is for monsters only." );
           				return;
         			}
		 			mob->roomd = strdup( mob->roomd );
         			ch->desc->str = &mob->roomd;
      				break;
      		case 4: 
	  				if( IS_NPC( mob ) ) mob->description = strdup( mob->description );
	  				ch->desc->str = &mob->description; 
					break;
      		case 5: if( IS_NPC( mob ) ) 
	  				{
           				sendf( ch, "Monsters have no titles." );
           				return;
         			}
         			ch->desc->str = &mob->title;
      				break;
      		default: sendf( ch, "That field is undefined." ); return;
    	}
  	} 
  	else 
  	{ 
    	if( !(obj = find_obj(ch, name)) ) 
		{
      		sendf( ch, "Can't find such a thing here.." );
      		return;
    	}
    	switch( field ) 
		{
      		case 1: ch->desc->str = &obj->name; break;
      		case 2: ch->desc->str = &obj->wornd; break;
      		case 3: ch->desc->str = &obj->roomd; break;
      		case 4:
        		if( !*string ) 	
				{
          			sendf( ch, "You have to supply a keyword." );
          			return;
        		}
        /* try to locate extra description */
        for (ed = obj->extrd; ; ed = ed->next)
          if (!ed) /* the field was not found. create a new one. */
          {
            ed = (exdescriptionType *)errMalloc(
					sizeof( exdescriptionType) ) ;
            ed->next = obj->extrd;
            obj->extrd = ed;
            ed->keyword =(char *)errMalloc(strlen(string) + 1) ;
            strcpy(ed->keyword, string);
            ed->description = 0;
            ch->desc->str = &ed->description;
            send_to_char("New field.\n\r", ch);
            break;
          }
          else if (!stricmp(ed->keyword, string)) /* the field exists */
          {
            errFree(ed->description);
            ed->description = 0;
            ch->desc->str = &ed->description;
            send_to_char(
              "Modifying description.\n\r", ch);
            break;
          }
        ch->desc->max_str = MAX_STRING_LENGTH;
        return; /* the stndrd (see below) procedure does not apply here */
      break;
      case 6: 
        if (!*string) {
          send_to_char("You must supply a field name.\n\r", ch);
          return;
        }
        /* try to locate field */
        for (ed = obj->extrd; ; ed = ed->next)
          if (!ed) {
            send_to_char("No field with that keyword.\n\r", ch);
            return;
          }
          else if (!stricmp(ed->keyword, string))
          {
            errFree(ed->keyword);
            if (ed->description)
              errFree(ed->description);
            
            /* delete the entry in the desr list */            
            if (ed == obj->extrd)
              obj->extrd = ed->next;
            else
            {
              for(tmp = obj->extrd; tmp->next != ed; 
                tmp = tmp->next);
              tmp->next = ed->next;
            }
            errFree(ed) ;

            send_to_char("Field deleted.\n\r", ch);
            return;
          }
      break;        
      default:
         send_to_char(
            "That field is undefined for objects.\n\r", ch);
         return;
      break;
    }
  	}

  	if(*ch->desc->str)
  	{
    	errFree(*ch->desc->str);
  	}

  	if( *string )
  	{
    	if( strlen(string) > length[field - 1])
    	{
      		sendf( ch, "String too long - truncated." );
      		*(string + length[field - 1]) = '\0';
    	}
		*ch->desc->str =(char *)errMalloc(strlen(string) + 1) ;
    	strcpy( *ch->desc->str, string );
    	ch->desc->str = 0;
    	sendf( ch, "Ok." );
  	}
  	else
  	{
    	sendf( ch, "Enter string. terminate with '@'." );
    	ch->desc->max_str = length[field - 1];
  	}
}
