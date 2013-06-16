#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define __TRACE__

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "world.h"
#include "limits.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "variables.h"
#include "sockets.h"
#include "strings.h"
#include "affect.h"


#define MAX_LASTCHAT	128
static char lastchatbuf[MAX_LASTCHAT][BUFSIZ] ;
int lastchat_pointer = 0 ;

void lastchat_add(char *str)
{
   	strcpy(lastchatbuf[lastchat_pointer], str);
/*  strcat(lastchatbuf[lastchat_pointer], "\n\r" );   */
   	lastchat_pointer = (lastchat_pointer +1) % MAX_LASTCHAT ;
}

void do_chat(charType *ch, char *argument, int cmd)
{
  	descriptorType 		*	iter, *next_desc;
  	charType 			*	victim;
  	char 					buf[MAX_STRING_LENGTH];

  	if( IS_NPC(ch) ) return;

  	if( nochatflag )
  	{
    	sendf( ch, "chat is forbidened now." );
    	return;
  	}

  	if( !IS_NPC(ch) && (IS_SET(ch->act, PLR_NOSHOUT)) )
  	{
    	sendf( ch, "You can't chat!!" );
    	return;
  	}

  	sprintf( buf,"%s> %s", ch->name, argument );
	TRACE( trace( "%s", buf ) );
  	lastchat_add(buf);

  	for( iter = desc_list; iter; iter = next_desc)
  	{
  		next_desc = iter->next;
    	if( !iter->connected ) 
		{
      		if( iter->original ) continue;
      		victim = iter->character;
      		if( !IS_SET( victim->act, PLR_NOCHAT) )
        		sendf( victim, "%s", buf );
    	}
  	}
  	sendf( ch, "Ok.");
}

void do_emote(charType *ch, char *argument, int cmd)
{
  int i;
  static char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;
  for (i = 0; *(argument + i) == ' '; i++);
  if (!*(argument + i))
    send_to_char("Yes.. But what?\n\r", ch);
  else {
    sprintf(buf,"$n %s", argument + i);
	TRACE( trace( "%s emotes '%s'", ch->name, argument + i) );
    act(buf,FALSE,ch,0,0,TO_ROOM);
    send_to_char("Ok.\n\r", ch);
  }
}


void do_say(charType *ch, char *argument, int cmd)
{
  	char 	buf[MAX_STRING_LENGTH];

	argument = skipsps( argument );

  	if(!*argument) send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
  	else 
  	{
    	sendf( ch, "You say '%s'", argument );
    	sprintf( buf, "$n says '%s'", argument );
		TRACE( if( !IS_NPC( ch ) ) trace( "%s says '%s'", ch->name, argument ) );
    	act(buf,FALSE,ch,0,0,TO_ROOM);
  	}
}

void do_sayh(charType *ch, char *argument, int cmd)
{		/* hangul say */
  int i;
  static char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

  for (i = 0; *(argument + i) == ' '; i++);

  if (!*(argument + i))
    send_to_char_han("Yes, but WHAT do you want to say?\n\r",
      "예? 뭐라고 말해요 ?\n\r", ch);
  else {
    sprintf(buf,"You say '%s'\n\r", argument + i);
    sprintf(buf2,"'%s' 하고 말합니다\n\r", argument + i);
    /* English - Korean display act() , english text first.. */
    send_to_char_han(buf, buf2, ch);

	TRACE( if( !IS_NPC( ch ) ) trace( "%s says '%s'", ch->name, argument+i ) );
    sprintf(buf,"$n says '%s'", argument + i);
    sprintf(buf2,"$n 님이 '%s' 하고 말합니다", argument + i);
    /* English - Korean display act() , english text first.. */
    acthan(buf,buf2,FALSE,ch,0,0,TO_ROOM);
  }
}

void do_shout(charType *ch, char *argument, int cmd)
{
  	static 	char 				buf1[MAX_STRING_LENGTH];
  			descriptorType 	*	iter, * next_desc;

  	if( noshoutflag && !IS_NPC(ch) && (GET_LEVEL(ch) < IMO))
  	{
    	sendf( ch, "I guess you can't shout now?" );
    	return;
  	}

  	if( !IS_NPC(ch) && (IS_SET(ch->act, PLR_NOSHOUT)) )
  	{
    	sendf( ch, "You can't shout!!" );
    	return;
  	}

	argument = skipsps( argument );

  	if( !(*argument) )
    	sendf( ch, "Shout? Yes! Fine! Shout we must, but WHAT??" );
  	else 
  	{
  		sendf( ch, "You shout, '%s'", argument );
    	sendf( ch, "Ok." );

		if( IS_NPC(ch) )
    		sprintf(buf1, "%s shouts '%s'", ch->moved, argument);
		else
		{
    		sprintf(buf1, "%s shouts '%s'", ch->name, argument);
			TRACE( trace( "%s", buf1 ) );
		}

      	for( iter = desc_list; iter; iter = next_desc )
	  	{
	  		next_desc = iter->next;
        	if( iter->character != ch 
			 && iter->connected == CON_PLYNG 
             && !IS_SET(iter->character->act, PLR_EARMUFFS) )
        		act( buf1, 0, ch, 0, iter->character, TO_VICT);
		}
		if( !IS_NPC(ch ) ) lastchat_add( buf1 );
  	}
}

void do_yell(charType *ch, char *argument, int cmd)
{
  	static 	char 				buf1[MAX_STRING_LENGTH];
  			descriptorType 	*	iter, * next_desc;

  	if( noshoutflag && !IS_NPC(ch) && (GET_LEVEL(ch) < IMO))
  	{
    	sendf( ch, "I guess you can't yell now?" );
    	return;
  	}

  	if( !IS_NPC(ch) && (IS_SET(ch->act, PLR_NOSHOUT)) )
  	{
    	sendf( ch, "You can't yell!!" );
    	return;
  	}

	argument = skipsps( argument );

  	if( !(*argument) )
    	sendf( ch, "Yell? Yes! Fine! Yell we must, but WHAT??" );
  	else 
  	{
  		sendf( ch, "You yell, '%s'", argument );
    	sendf( ch, "Ok." );

		if( IS_NPC(ch) )
    		sprintf(buf1, "%s yells '%s'", ch->moved, argument);
		else
		{
    		sprintf(buf1, "%s yells '%s'", ch->name, argument);
			TRACE( trace( "%s", buf1 ) );
		}

      	for( iter = desc_list; iter; iter = next_desc )
	  	{
	  		next_desc = iter->next;
        	if( iter->character != ch 
			 && iter->connected == CON_PLYNG 
			 && world[iter->character->in_room].zone == world[ch->in_room].zone
             && !IS_SET(iter->character->act, PLR_EARMUFFS) )
        		act( buf1, 0, ch, 0, iter->character, TO_VICT);
		}
  	}
}

void do_tell(charType *ch, char *argument, int cmd)
{
  	charType 	*		vict;
  	char 		*		s, name[100], message[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];

  halfchop(argument,name,message);
  if(!*name || !*message)
    send_to_char("Who do you wish to tell what??\n\r", ch);
  else if (!(vict = find_player(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (ch == vict)
    send_to_char("You try to tell yourself something.\n\r", ch);
  else if (GET_POS(vict) == POSITION_SLEEPING) {
    act("$E can't hear you.",FALSE,ch,0,vict,TO_CHAR);
  } else if ((!IS_SET(vict->act,PLR_NOTELL)) ||
             ((GET_LEVEL(ch) >= IMO) && (GET_LEVEL(ch) > GET_LEVEL(vict)))){
    if(IS_NPC(ch))
      s=ch->moved;
    else
      s = can_see(vict,ch) ? GET_NAME(ch) : "Someone";
    sprintf(buf,"%s tells you '%s'\n\r",s,message);
	TRACE( if( !IS_NPC( ch ) ) trace( "%s tells %s '%s'", ch->name, vict->name, message ) );
    send_to_char(buf, vict);
    sprintf(buf,"You tell %s '%s'\n\r",GET_NAME(vict),message);
    send_to_char(buf, ch);
  } else {
    act("$E isn't listening now.",FALSE,ch,0,vict,TO_CHAR);
  }
}

void do_send(charType *ch, char *argument, int cmd)
{
  charType *vict;
  char *s, name[100], paint_name[100], message[200],
  buf[MAX_STRING_LENGTH], paint[MAX_STRING_LENGTH];

  halfchop(argument,name,message);
  if( strchr( message, '.' ) || strchr( message, '/' ) ) 
  {
    send_to_char("No such paint prepared.\n\r",ch);
	return;
  }
  sprintf(paint_name,"%s/%s", PAINT_DIR, message);
  if(!*name || !*message)
    send_to_char("Who do you wish to tell what??\n\r", ch);
  else if (!(vict = find_player(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (file_to_buf(paint_name,paint) == -1)
    send_to_char("No such paint prepared.\n\r",ch);
  else if ((!IS_SET(vict->act,PLR_NOTELL)) ||
             ((GET_LEVEL(ch) >= IMO) && (GET_LEVEL(ch) > GET_LEVEL(vict))))
	{
    	send_to_char(paint, vict);
    	sprintf(buf,"You send %s %s\n\r",GET_NAME(vict),message);
    	send_to_char(buf, ch);
    	if(IS_NPC(ch))	s=ch->moved;
    	else
      		s=can_see(vict,ch) ? GET_NAME(ch) : "Someone";  
    	sprintf(buf,"%s sends you '%s'\n\r",s,message);
    	send_to_char(buf, vict);
  } 
  else 
  {
    act("$E isn't listening now.",FALSE,ch,0,vict,TO_CHAR);
  }
}

void do_gtell(charType *ch, char *argument, int cmd)
{
 charType *k;
 followType *f;
 char *s, buf[200];

 if (!(k=ch->master))
  k = ch;
 if(IS_NPC(ch)) s=ch->moved;
 else
   s=GET_NAME(ch);
  if (IS_AFFECTED(k,AFF_GROUP)){
    sprintf(buf,"** %s ** '%s'\n\r",s,argument);
    send_to_char(buf, k);
	TRACE( if( !IS_NPC( ch ) ) trace( "** %s ** '%s'", s, argument ) );
  }
 for (f=k->followers;f;f=f->next) {
  if( IS_AFFECTED(f->follower, AFF_GROUP) ){
    sprintf(buf,"** %s ** '%s'\n\r",s,argument);
    send_to_char(buf, f->follower);
  }
 }
 send_to_char("Ok.\n\r", ch);
}

void do_whisper(charType *ch, char *argument, int cmd)
{
  charType *vict;
  char name[100], message[MAX_STRING_LENGTH],
    buf[MAX_STRING_LENGTH];

  halfchop(argument,name,message);

  if(!*name || !*message)
    send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
  else if (!(vict = find_player(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (vict == ch)
  {
    act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
    send_to_char(
      "You can't seem to get your mouth close enough to your ear...\n\r",
       ch);
  }
  else
  {
    sprintf(buf,"$n whispers to you, '%s'",message);
	TRACE( trace( "%s whispers %s '%s'", ch->name, vict->name, message ) );
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    send_to_char("Ok.\n\r", ch);
    act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
  }
}


void do_ask(charType *ch, char *argument, int cmd)
{
  charType *vict;
  char name[100], message[MAX_STRING_LENGTH],
    buf[MAX_STRING_LENGTH];

  halfchop(argument,name,message);

  if(!*name || !*message)
    send_to_char("Who do you want to ask something.. and what??\n\r", ch);
  else if (!(vict = find_player(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (vict == ch)
  {
    act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
    send_to_char("You think about it for a while...\n\r", ch);
  }
  else
  {
    sprintf(buf,"$n asks you '%s'",message);
	TRACE( trace( "%s asks %s '%s'", ch->name, vict->name, message ) );
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    send_to_char("Ok.\n\r", ch);
    act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);
  }
}

#define MAX_NOTE_LENGTH MAX_STRING_LENGTH      /* arbitrary */

void do_write(charType *ch, char *argument, int cmd)
{
    objectType 	* 	paper = 0, * pen = 0;
    char 			papername[MAX_INPUT_LENGTH]; 
	char			penname[MAX_INPUT_LENGTH];	

    twoArgument(argument, papername, penname);

    if( !ch->desc ) return;

    if (!*papername)  /* nothing was delivered */
    {   
		sendf( ch, "Write? with what? ON what? what are you trying to do??" ); return;
    }

    if( *penname ) /* there were two arguments */
    {
		if (!(paper = find_obj_inven( ch, papername )) )
		{
            sendf( ch, "You have no %s.", papername); return;
		}
		if( !(pen = find_obj_inven( ch, penname)) && !(pen = find_obj_equip( ch, penname)) )
		{
    		sendf( ch, "You have no %s.", penname );  return;
		}
   	}
    else  /* there was one arg.let's see what we can find */
    {			
		if( !(paper = find_obj_inven( ch, papername)) )
		{
            sendf( ch, "There is no %s in your inventory.", papername ); return;
		}

		if( paper->type == ITEM_PEN )  /* oops, a pen.. */
		{
	    	pen   = paper;
	    	paper = 0;
		}
		else if( paper->type != ITEM_NOTE )
		{
	    	sendf( ch, "That thing has nothing to do with writing." );
	    	return;
		}

		if( !ch->equipment[HOLD] )
		{
	    	sendf( ch, "You can't write with a %s alone.", papername ); return;
		}

		if( !can_see_obj(ch, ch->equipment[HOLD]) )
		{
	    	sendf( ch, "The stuff in your hand is invisible! Yeech!!" ); return;
		}
		
		if( pen ) paper = ch->equipment[HOLD];
		else 	  pen   = ch->equipment[HOLD];
    }
			
    /* ok.. now let's see what kind of stuff we've found */
    if( pen->type != ITEM_PEN)
    {
		act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR );
    }
    else if( paper->type != ITEM_NOTE )
    {
		act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
    }
    else if( paper->usedd )
		sendf( ch, "There's something written on it already." );
    else
    {
		/* we can write - hooray! */
				
		sendf( ch, "Ok.. go ahead and write.. end the note with a @." );
		act( "$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM );
	    ch->desc->str     = &paper->usedd;
		ch->desc->max_str = MAX_NOTE_LENGTH;
    }
}

void do_scroll(charType *ch, char *argument, int cmd)
{
	int			lines;

	while( isspace( *argument ) ) argument++;

	if( !*argument )
	{
		sendf( ch, "[10-127] Your scroll limit is %d lines.", ch->desc->qSize - 1);
		return;
	}

	if( !getnumber( argument, &lines ) )
	{
		sendf( ch, "Eh?? I need a number of lines." ); 					return;
	}

	if( lines < 10 )
	{
		sendf( ch, "[10-127] %d is too small number.", lines ); 		return;
	}

	if( lines >= MAX_OUTQUE )
	{
		sendf( ch, "[10-127] %d is too large number.", lines ); 		return;
	}

	flush_ouqueue( ch->desc );

	ch->desc->qSize = lines;

	sendf( ch, "Your scroll limit is now %d lines.", lines );
}


void do_lastchat( charType * ch, char *argument, int cmd )
{
  	int  		i, point;

  	if( !ch->desc ) return;

  	sendf( ch, "There were some chat(shout) messages.." );

  	point = lastchat_pointer - (ch->desc->qSize - 3);

	if( point < 0 ) point = MAX_LASTCHAT + point;

  	for( i = 0; i < (ch->desc->qSize - 3); i++) 
  	{
     	if( lastchatbuf[point][0] ) 
	 	{
     		sendf( ch, "%s", lastchatbuf[point] );
        }
     	point = (point +1) % MAX_LASTCHAT ;
	}
}
