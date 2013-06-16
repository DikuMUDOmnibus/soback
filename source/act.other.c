/* ************************************************************************
*  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
*  Usage : Other commands.                                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "character.h"
#include "object.h"
#include "mobile.h"
#include "world.h"
#include "sockets.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "limits.h"
#include "variables.h"
#include "fight.h"
#include "misc.h"
#include "strings.h"
#include "quest.h"
#include "affect.h"
#include "transfer.h"
#include "page.h"

void do_wimpy(charType *ch, char *argument, int cmd)
{
  	if( IS_NPC(ch) ) return;

  	if( IS_SET( ch->act, PLR_WIMPY ) ) 
	{
    	sendf( ch, "You are not wimpy now." );
    	REMOVE_BIT(ch->act, PLR_WIMPY);
  	} 
	else 
	{
    	sendf( ch, "You are wimpy now." );
    	SET_BIT(ch->act, PLR_WIMPY);
  	}
}

void do_afk( charType * ch, char * argument, int cmd )
{
	if( IS_NPC(ch) ) return;

  	if( IS_SET( ch->act, PLR_AFK ) ) 
	{
    	sendf( ch, "You tell everyone in the room that you are back." );
		act( "$n announces, 'I'm back'.", FALSE, ch, 0, 0, TO_ROOM );
    	REMOVE_BIT(ch->act, PLR_AFK);
  	} 
	else 
	{
    	sendf( ch, "You tell everyone in the room that you'll be AFK." );
		act( "$n announces, 'I'm going AFK'.", FALSE, ch, 0, 0, TO_ROOM );
    	SET_BIT(ch->act, PLR_AFK );
  	}
}

void do_save(charType *ch, char *argument, int cmd)
{
  	if( IS_NPC(ch) || !ch->desc ) return;

  	sendf( ch, "Saving %s.", GET_NAME(ch));

  	save_char( ch, NOWHERE );

  	if( stash_char( ch, 0 ) < 0 ) 
  	{
		sendf( ch, "Fail to save rent file - report this to wizard" );
	}
}

void do_not_here(charType *ch, char *argument, int cmd)
{
  	send_to_char("Sorry, but you cannot do that here!\n\r",ch);
}

void do_practice(charType *ch, char *arg, int cmd)
{
  	int 		i, count;
  	char 		prbuf[80*MAX_SKILLS], buf[80], buf2[80];

	if( IS_NPC(ch) ) return;

  	strcpy(prbuf, "\n\r");

  	for(i=0, count=0; *spells[i].name != '\n';i++) 
  	{
    	if(! *spells[i].name) continue;
    	if(! ch->skills[i]) continue;
    	if(spells[i].min_level[GET_CLASS(ch)-1] > GET_LEVEL(ch)) continue;
    	sprintf(buf, "%s %s", spells[i].name, how_good(ch->skills[i]));
    	if(count %2 == 0)
      		sprintf(buf2, "%-35s", buf) ;
    	else
      		sprintf(buf2, "%s\n\r", buf) ;
    	strcat(prbuf, buf2) ;
    	count++;
  	}
  	strcat(prbuf, "\n\r");
  	print_page( ch, prbuf );
}

void do_typo( charType * ch, char * argument, int cmd )
{
  	FILE *fl;
  	char str[MAX_STRING_LENGTH];

  	if( IS_NPC(ch) )
  	{
    	sendf( ch, "Monsters can't spell - leave me alone." );
    	return;
  	}

  	if( argument = skipsps( argument), !*argument && !OMNI(ch) ) 
  	{
    	sendf( ch, "I beg your pardon?" );
    	return;
  	}

	if( cmd == COM_TYPO )
	{
  		if( !(fl = errOpen( TYPO_FILE, "a+")) ) 
  		{
    		sendf( ch, "Could not open the typo-file. Report to wizard, plz" );
    		return;
  		}
  	}
  	else if( cmd == COM_BUG )
  	{
  		if( !(fl = errOpen( BUG_FILE, "a+")) ) 
  		{
    		sendf( ch, "Could not open the bug-file. Report to wizard, plz" );
    		return;
  		}
  	}
  	else
  	{
  		if( !(fl = errOpen( IDEA_FILE, "a+")) ) 
  		{
    		sendf( ch, "Could not open the idea-file. Report to wizard, plz" );
    		return;
  		}
  	}

	if( !*argument )
	{
		char 	buf[1024]; 	

		rewind( fl );

		while( !feof( fl ) )
    	{
        	if( fgets( buf, 1023, fl ) ) sendf( ch, "%s", buf );
        }
    }
	else
	{
  		sprintf(str, "**%s[%d]: %s\n", GET_NAME(ch), world[ch->in_room].virtual, argument);
  		fputs(str, fl);
  		sendf( ch, "Ok. thanks." );
	}

	errClose(fl);
}

void do_brief(charType *ch, char *argument, int cmd)
{
  	if( IS_NPC(ch) ) return;

  	if( IS_SET(ch->act, PLR_BRIEF) )
  	{
    	sendf( ch, "Brief mode off." );
    	REMOVE_BIT(ch->act, PLR_BRIEF);
  	} 
	else 
	{
    	sendf( ch, "Brief mode on." );
    	SET_BIT(ch->act, PLR_BRIEF);
  	}
}

void do_compact(charType *ch, char *argument, int cmd)
{
  	if( IS_NPC(ch) ) return;

  	if( IS_SET(ch->act, PLR_COMPACT) ) 
	{
    	sendf( ch, "You are now in the uncompacted mode." );
    	REMOVE_BIT(ch->act, PLR_COMPACT);
  	} 
	else 
	{
    	sendf( ch, "You are now in compact mode." );
    	SET_BIT(ch->act, PLR_COMPACT);
  	}
}

void do_post(charType *ch, char *argument, int cmd)
{
    send_to_char( "You can only post on board.\n\r", ch );
}

void do_nothing(charType *ch, char *argument, int cmd)
{
    Huh(ch) ;
}   
  
void do_quest(charType *ch, char *argument, int cmd)
{    
	if( IS_NPC(ch) ) return;

    send_to_char_han("You solved Quest :\n\r",
        "다음과 같은 문제를 해결 하였습니다.\n\r", ch) ;
    send_to_char(what_is_solved(ch), ch) ;
}    
