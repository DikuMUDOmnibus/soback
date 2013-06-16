#include <string.h>

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "interpreter.h"
#include "comm.h"
#include "strings.h"
#include "utils.h"
#include "misc.h"
#include "variables.h"
#include "strings.h"
#include "page.h"

struct help_index_element
{
    char *keyword;
    long pos;
};

static struct help_index_element *	help_index;
static int 							top_of_helpt;  

FILE * help_fl;

void build_help_index( void )
{
	int nr = -1, issorted, i;
	struct help_index_element *list = 0, mem;
	char buf[81], tmp[81], *scan;
	long pos;

  	if( help_fl = errOpen( HELP_KEY_FILE, "r" ), !help_fl )
  		FATAL( "build_help_index> open help file error." );

  	for (;;)
  	{
    	pos = ftell( help_fl );
    	fgets( buf, 81,  help_fl );
    	*(buf + strlen(buf) - 1) = '\0';
    	scan = buf;

    	for (;;)
    	{
      		/* extract the keywords */
			if( *scan == '\"' ) 
			{
				for( scan++, i = 0; *scan && *scan != '\"'; scan++ )
				{
					tmp[i++] = *scan;
				}
				if( *scan == '\"' ) scan++;
				tmp[i] = 0;
			}
			else scan = onefword( scan, tmp );
  
      		if( !*tmp ) break;

      		if( !list )
      		{
        		list = (struct help_index_element *)errMalloc( sizeof(struct help_index_element));
        		nr   = 0;
      		}
      		else
        		list=(struct help_index_element *)errRealloc( 
										 list, sizeof(struct help_index_element) * ( ++nr + 1) );

      		list[nr].pos     = pos;
        	list[nr].keyword = (char *)errMalloc(strlen(tmp) + 1) ;
      		strcpy(list[nr].keyword, tmp);  
   		}

    		/* skip the text */
    	do 
		{
			fgets(buf, 81,  help_fl ); 
		} while (*buf != '#');

    	if( *(buf + 1) == '~' ) break;
  }
  /* we might as well sort the stuff */
  do
  {
    issorted = 1;
    for (i = 0; i < nr; i++)
      if (stricmp(list[i].keyword, list[i + 1].keyword) > 0)
      {
        mem = list[i];
        list[i] = list[i + 1];
        list[i + 1] = mem;
        issorted = 0;
      }
  }
  while (!issorted);

	top_of_helpt = nr;
  	help_index = list;
}

void do_credits(charType *ch, char *argument, int cmd)
{
	if( !ch->desc ) return;
    print_page( ch, credits );
}

void do_news(charType *ch, char *argument, int cmd)
{
	if( !ch->desc ) return;
    print_page( ch, news );   
}

void do_info(charType *ch, char *argument, int cmd)
{
	if( !ch->desc ) return;
    print_page( ch, info );   
}

void do_wizards(charType *ch, char *argument, int cmd)
{
	if( !ch->desc ) return;
    print_page( ch, wizards );
}

void do_motd(charType *ch, char *argument, int cmd)
{
	if( !ch->desc ) return;
    print_page( ch, motd );   
}

void do_todo(charType *ch, char *argument, int cmd)
{
	if( !ch->desc ) return;
    print_page( ch, todo );   
}

void do_wizhelp(charType *ch, char *argument, int cmd)
{   
	commandType		*	cmds;
    char    			buf[MAX_STRING_LENGTH];
    int     			no, i, j;
        
    if(IS_NPC(ch)) return;
            
    send_to_char("The following privileged commands are available:\n\r", ch);
            
    *buf = '\0'; 

    for( no = 1, i = 0; commands[i]; i++ )
	{
		cmds = commands[i];
		
		for( j = 0; cmds && cmds[j].name[1]; j++ )
		{
        	if( (GET_LEVEL(ch) >= cmds[j].level) && (cmds[j].level >= IMO))
        	{   
            	sprintf( buf + strlen(buf), "%-15s", cmds[j].name );
            	if( !(no % 5) ) strcat(buf, "\n\r");
            	no++;
        	}
		}
	}
     
    strcat(buf, "\n\r");
    print_page( ch, buf );    
}

void do_help(charType *ch, char *argument, int cmd)
{
  int chk, bot, top, mid, minlen;
  char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];

  	if( !ch->desc ) return;

	argument = skipsps( argument );

  if (*argument != NULL)
  {
    if (!help_index)
    {
      send_to_char("No help available.\n\r", ch);
      return;
    }
    bot = 0;
    top = top_of_helpt;

    for (;;)
    {
      mid = (bot + top) / 2;
      minlen = strlen(argument);

      if( !(chk = strnicmp(argument, help_index[mid].keyword, minlen)) )
      {
        fseek(help_fl, help_index[mid].pos, 0);
        *buffer = '\0';
        for (;;)
        {
          fgets(buf, 80, help_fl);
          if (*buf == '#')
            break;
          strcat(buffer, buf);
          strcat(buffer, "\r");
        }
        print_page( ch, buffer );
        return;
      }
      else if (bot >= top)
      {
        send_to_char("There is no help on that word.\n\r", ch);
		sprintf(buf, "HELP: tried to seek '%s'", argument) ;
		log(buf) ;
        return;
      }
      else if (chk > 0)
        bot = ++mid;
      else
        top = --mid;
    }
    return;
  }
  send_to_char(help, ch);
}

