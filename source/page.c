#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "character.h"
#include "sockets.h"
#include "interpreter.h"
#include "page.h"
#include "strings.h"
#include "comm.h"
#include "page.h"

static pageUnitType		*	pages;

static void add_onepage( pageUnitType * pu, pageType * pt )
{
	pageType	* prev;

	if( !pu->pages )
	{
		pu->pages = pt;
		pt->next = 0;
		pt->prev = 0;

		pu->curr = pu->pages;
	}
	else
	{
		for( prev = pu->pages; prev->next; prev = prev->next );

		prev->next = pt;
		pt->prev = prev;
		pt->next = 0;
	}

	pu->hasPage++;
}

static void add_oneline( pageUnitType * pu, char * str )
{
	pageType		* pt;
	lineType		* ln, * prev;

	for( pt = pu->pages; pt; pt = pt->next )
	{
		if( pt->hasLine < pu->screen ) break;
	}

	if( !pt )
	{
		pt = (pageType *)errCalloc( sizeof(pageType) );
		add_onepage( pu, pt );
	}

	ln = (lineType *)errCalloc( sizeof(lineType) );
	ln->line = strdup( str );

	if( !pt->lines )
	{
		pt->lines = ln;
	}
	else
	{
		for( prev = pt->lines; prev->next; prev = prev->next );

		prev->next = ln;
	}

	pt->hasLine++;
}

pageUnitType * new_pageunit( charType * ch )
{
	pageUnitType	* pu;

	pu = (pageUnitType *)errCalloc( sizeof(pageUnitType) );

	pu->who = strdup( ch->name );
	pu->next = pages;
	pu->screen = ch->screen;

	pages = pu;

	return pu;
}

void del_pageunit( pageUnitType * pu )
{
	pageUnitType	* prev;
	pageType		* pt;
	lineType		* ln;

	if( pages == pu ) pages = pu->next;
	else
	{
		for( prev = pages; prev && prev->next != pu; prev = prev->next )
		;

		if( prev->next != pu )
		{
			FATAL( "del_pageunit> page unit not found." );
		}

		prev->next = pu->next;
	}

	while( pu->hasPage )
	{
		pt = pu->pages;
		pu->pages = pu->pages->next;

		while( pt->hasLine )
		{
			ln = pt->lines;
			pt->lines = pt->lines->next;

			errFree( ln->line );
			errFree( ln );
			pt->hasLine--;	
		}

		errFree( pt );
		pu->hasPage--;
	}

	errFree( pu->who );
	errFree( pu );
}

static void print_onepage( charType * ch, pageType * pt )
{
	lineType		* 	ln;

	if( !ch->desc ) return;

	for( ln = pt->lines; ln; ln = ln->next )
	{
		put_to_output( ch->desc, ln->line );
	}
}

pageUnitType * find_pageunit( charType * ch )
{
	pageUnitType	* pu;

	for( pu = pages; pu; pu = pu->next )
	{
		if( stricmp( pu->who, ch->name ) == 0 ) return pu;
	}

	return 0;
}

void pagef( pageUnitType * pu, char * fmt, ... )
{
    va_list     args;
    char        buf[MAX_STRING_LENGTH + 1];

    va_start( args, fmt );
    vsprintf( buf, fmt, args );

    strcat( buf, "\n\r" );
	add_oneline( pu, buf );
}

void show_page( charType * ch, pageUnitType * pu )
{
	if( pu->hasPage ) print_onepage( ch, pu->curr );

	if( pu->hasPage > 1 )
	{
		ch->page = 1;
	}
	else del_pageunit( pu );
}

static void show_next_page( charType * ch, pageUnitType * pu )
{
	pageType		* pt;

	pt = pu->curr;

	if( !pt->next )
	{
		sendf( ch, "This is the last page." ); return;
	}
	
	pu->curr = pt->next;

	show_page( ch, pu );
}

static void show_prev_page( charType * ch, pageUnitType * pu )
{
	pageType		* pt;

	pt = pu->curr;

	if( !pt->prev )
	{
		sendf( ch, "You are at the first page already." ); return;
	}

	pu->curr = pt->prev;

	show_page( ch, pu );
}

void print_page( charType * ch, char * str )
{
	pageUnitType	*	pu;
	char			*	cp, * bp;
	char				buffer[MAX_STRING_LENGTH];
	int					end = -1;

	if( !str || !*str ) return;

	pu = new_pageunit( ch );

	for( bp = buffer, cp = str; *cp; cp++ )
	{
		if( (((*bp++ = *cp) == '\n') || (*cp == '\r')) && (end = -end) < 0 )
		{
			*bp = 0;
			add_oneline( pu, buffer );
			bp = buffer;
		}
	}
	show_page( ch, pu );
}

void page_prompt( descriptorType * desc )
{
	pageUnitType	*	pu;

	if( pu = find_pageunit( desc->character ), !pu )
	{
		desc->character->page = 0;

		DEBUG( "page_prompt> %s's page unit not found.", desc->character->name );
		return;
	}

	if( pu->curr->next )
		write_to_descriptor( desc->fd, 
			"----- (P) Previouse, (N) Next, (R) Reprint, (Q) Quit.. Enter to continue > " );
	else
		write_to_descriptor( desc->fd, 
			"----- (P) Previouse, (N) Next, (R) Reprint, (Q) Quit.. Enter to Quit > " );
}

void page_interpreter( charType * ch, char * argu )
{
	pageUnitType	*	pu;

	if( pu = find_pageunit( ch ), !pu )
	{
		ch->page = 0;

		DEBUG( "%s's page unit not found.", ch->name );
		return;
	}

	switch( tolower( *argu ) )
	{
		case '\0' :	if( !pu->curr->next )
					{
						del_pageunit( pu ); ch->page = 0; return;
					}
		case 'n'  : show_next_page( ch, pu ); break;
		case 'p'  : show_prev_page( ch, pu ); break;
		case 'r'  : show_page( ch, pu );      break;
		case 'q'  : del_pageunit( pu ); ch->page = 0; break;
	}
}

void do_screen( charType * ch, char * argu, int cmd )
{
    int         lines;
   
    while( isspace( *argu ) ) argu++;
   
    if( !*argu )
    {
        sendf( ch, "[10-127] Your screen size is %d lines.", ch->screen );
        return;
    }
   
    if( !getnumber( argu, &lines ) )
    {
        sendf( ch, "Eh?? I need a number of lines." );                  return;
    }

    if( lines < 10 )
    {
        sendf( ch, "[10-127] %d is too small number.", lines );         return;
    }

    if( lines > 127 )
    {
        sendf( ch, "[10-127] %d is too large number.", lines );         return;
    }

    ch->screen = lines;

    sendf( ch, "Your screen size is now %d lines.", lines );
}
