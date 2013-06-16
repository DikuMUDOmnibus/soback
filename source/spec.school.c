#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
      
#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "find.h"
#include "magics.h"
#include "limits.h"
#include "fight.h"
#include "misc.h"
#include "quest.h"
#include "variables.h"
#include "strings.h"
#include "specials.h"
#include "transfer.h"
#include "affect.h"
#include "find.h"

int school_cold(charType *ch, int cmd, char *arg)
{
	charType *vict, *next_victim;
	int lev, alive, healed, blessed, armored ;

	if(cmd) return FALSE ;

	healed = blessed = armored = 0 ;
	for(vict=world[ch->in_room].people; vict; vict = next_victim ) {
		next_victim = vict->next_in_room ;
		if(!IS_NPC(vict)) {		/* helping player */
			lev = GET_LEVEL(vict) ;
			if ( lev > 9 ) continue ;		/* only help low level */
			alive = 100*GET_HIT(vict)/GET_MAX_HIT(vict) ;	/* are they hurt ? */
			if ( alive < 50 && lev < 6 ) {
				if ( !healed ) {
					act("$n���� '����, ���� ��ġ�̱���' ��� ���մϴ�.",
						1,ch,0,0,TO_ROOM);
					act("$n���� ���� ���� ������ ����� ����� ���ϴ�.",
						1,ch,0,0,TO_ROOM);
					}
				spell_cure_light(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
				healed ++ ;
				}
			if ( alive < 90 ) {
				if ( !healed ) {
					act("$n���� ���� ���� ������ ����� ����� ���ϴ�.",
						1,ch,0,0,TO_ROOM);
					}
				spell_cure_light(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
				healed ++ ;
				}
			if ( lev > 6 ) continue ;
			if ( !affected_by_spell(vict, SPELL_ARMOR) ) {
				if ( !armored ) {
					act("$n���� ������ �ٶ��� �ϴ��� ����Ⱑ �Ǿ� ��ȿ� ���� �����ϴ�.",
						1, ch, 0, 0, TO_ROOM);
					spell_armor(20, ch, "", SPELL_SPELL, vict, 0);
					armored ++ ;
					}
				}
			if ( lev > 3 ) continue ;
			if ( !affected_by_spell(vict, SPELL_BLESS) ) {
				if ( !blessed ) {
					act("$n���� ���� �������� ���ν� ������ ����ŵ�ϴ�.",
						1,ch,0,0,TO_ROOM);
					spell_bless(20, ch, "", SPELL_SPELL, vict, 0);
					blessed ++ ;
					}
				}
			}
		}		/* end of for */

	return TRUE;
}

#define G_NONE		0
#define G_SAY		1
#define G_WHISPER	2
#define G_SMILE		3

static int gracia_response ;	/* value is G_NONE to G_SMILE */
static charType *gracia_lastperson ;

int school_gracia(charType *ch, int cmd, char *arg)
{
	static char *say_to[] = {
		"������ �� �ݰ��Ϳ�, $N��.",
		"�� ���ݸ� ������, $N��.", 
		"$N���� ��� ���̳��� ?",
		"$N���� ���⼭ �ٸ� ģ������ ���� ��;����� ?"
		} ;

	static char *say_all[] = {
		"����� ~~ ����� ���� ���� ����� �������� ����.",
		"���� ���� ���µ� ���� ���� ?",
		"����� �� �ɾƼ� ������ ������ :)",
		"�����е��� ���ϱ� ����� �� ���ƿ�.",
		"�����е� ���� ���� �� �� ������ �� �����ٵ�... :)"
		} ;

	static char *whisper_to[] = {
		"������ ����� ��� ?", 
		"�̷� ���� �� ��� �Ȱ����� ?",
		"�ӵ� �б� �����ϸ� ���� ���� �� �ϰ� �������� ?"
		} ;
	charType *vict, *next_victim, *response_to = ch;
	int n_person ;
	char buf[200] ;

	if(cmd) {
		if( cmd == COM_SAY )
			gracia_response = G_SAY ; /* say */
		if( cmd == COM_WHISPER )	/* whisper */
			{
			gracia_response = G_WHISPER ;
			gracia_lastperson  = ch ;
			}

		return FALSE ;	/* anyway return False */
		}

	n_person = 0 ;
	for(vict=world[ch->in_room].people; vict; vict = next_victim ) {
		next_victim = vict->next_in_room ;
		if(!IS_NPC(vict)) {		/* There is a player */
			if (n_person == 0) {
				response_to = vict ;
				}
			n_person ++ ;
			}
		}

	if ( n_person == 0 )
		return TRUE ;

	if ( gracia_response != 0 ) {
		switch (gracia_response) {
			case G_SAY :
			case G_WHISPER :
				if ( number(0,1) == 0)
					act("Gracia���� '��..' �ϰ� ���մϴ�", 1, ch, 0, 0, TO_ROOM);
				else
					act("$n���� ������ ���� �մϴ�.", 1, ch, 0, 0, TO_ROOM);
				break ;
			case G_SMILE :
				if ( gracia_lastperson != NULL &&
					gracia_lastperson->in_room == ch->in_room) {
					act("$n���� $N���� ���� �̼Ҹ� �����ϴ�.", FALSE, ch, 0,
						gracia_lastperson, TO_NOTVICT) ;
					send_to_char("Gracia���� ����� ���� ��� �����ϴ�.\n\r",
						gracia_lastperson) ;
					act("$n���� ������ ������ �����ϴ�.", 1, ch, 0, 0, TO_ROOM);
					}
				break ;
			}
		gracia_response = G_NONE ;
		gracia_lastperson = NULL ;
		}

	if ( n_person + number(0,4) > 4 ) {
		sprintf(buf, "Gracia���� '%s' ��� ���մϴ�.", say_all[number(0,3)]) ;
		act(buf, 1, ch, 0, 0, TO_ROOM);
		}
	else if ( number(0,4) < 3 ) {	/* say to */
		if (response_to != NULL) {
			sprintf(buf, "Gracia���� '%s' ��� ���մϴ�.", say_to[number(0,3)]) ;
			act(buf, 1, ch, 0, response_to, TO_ROOM);
			}
		}
	else {		/* whisper to */
		if ( response_to != NULL &&
			response_to->in_room == ch->in_room) {
			sprintf(buf, "Gracia���� ��ſ��� '%s' ��� �ӻ��Դϴ�.\n\r",
				whisper_to[number(0,2)]) ;
			act("$n���� $N���� �Ϳ� ���� �Ҹ��� �����ΰ� ���մϴ�.",
				FALSE, ch, 0, response_to, TO_NOTVICT) ;
			send_to_char(buf, response_to) ;
			}
		}

	return TRUE ;
}

#undef G_NONE
#undef G_SAY
#undef G_WHISPER
#undef G_SMILE

int school_nara( charType * ch, int cmd, char *arg )
{
	charType 	*	vict, * nara;
	char 			buf[256] ;

	if( cmd ) return 0;

	nara = find_mob_room_at( ch, ch->in_room, "Nara" );

	if( !nara ) return 0;
	
	for( vict = world[nara->in_room].people; vict; vict = vict->next_in_room )
	{
		if( vict->level == 1 ) break;
	}

	if( !vict ) return 0;

	if( !is_solved_quest( vict, QUEST_SCHOOL)) 
	{
		objectType * 	recall;
		int					recall_num;

		sprintf( buf, "%s %s\n\r", "�� �ϼ̽��ϴ�.",
			"����� �� �б����� �ؾ� �� ���� �� ��ġ�̽��ϴ�.") ;
		send_to_char(buf,  vict) ;
		set_solved_quest( vict, QUEST_SCHOOL) ;
		act("$n���� $N�Կ��� �������� ���� �մϴ�.", FALSE, ch, 0,  vict, TO_NOTVICT) ;
		send_to_char("Nara���� ��ſ��� �������� �ݴϴ�.\n\r",  vict) ;
		if( GET_LEVEL( vict) < 2 ) 
		{
			GET_LEVEL( vict) = 2 ;
			advance_level( vict) ;
			set_title( vict) ;
			send_to_char("Nara���� ����� ������ �÷� �־����ϴ�.\n\r",	 vict) ;
		}

        if( (recall_num = real_objectNr( 3052 )) != OBJECT_NULL )
		{
            log("nara> no such recall");
            return(TRUE);
        }
        recall = load_a_object( recall_num, REAL, 1 );
        obj_to_char( recall,  vict );
		act("$n���� ��ſ��� scroll of recall �� �ݴϴ�.",	FALSE, ch, 0,  vict, TO_VICT) ;
	}
	return TRUE ;
}
