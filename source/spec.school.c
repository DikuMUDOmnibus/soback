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
					act("$n님이 '저런, 많이 다치셨군요' 라고 말합니다.",
						1,ch,0,0,TO_ROOM);
					act("$n님이 손을 저어 따스한 기운을 만들어 냅니다.",
						1,ch,0,0,TO_ROOM);
					}
				spell_cure_light(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
				healed ++ ;
				}
			if ( alive < 90 ) {
				if ( !healed ) {
					act("$n님이 손을 저어 따스한 기운을 만들어 냅니다.",
						1,ch,0,0,TO_ROOM);
					}
				spell_cure_light(GET_LEVEL(ch), ch, "", SPELL_SPELL, vict, 0);
				healed ++ ;
				}
			if ( lev > 6 ) continue ;
			if ( !affected_by_spell(vict, SPELL_ARMOR) ) {
				if ( !armored ) {
					act("$n님의 몸에서 바람이 일더니 꽃향기가 되어 방안에 가득 퍼집니다.",
						1, ch, 0, 0, TO_ROOM);
					spell_armor(20, ch, "", SPELL_SPELL, vict, 0);
					armored ++ ;
					}
				}
			if ( lev > 3 ) continue ;
			if ( !affected_by_spell(vict, SPELL_BLESS) ) {
				if ( !blessed ) {
					act("$n님이 그의 반지에서 눈부신 빛살을 일으킵니다.",
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
		"만나서 참 반가와요, $N님.",
		"자 조금만 기운내세요, $N님.", 
		"$N님은 어디서 오셨나요 ?",
		"$N님은 여기서 다른 친구들은 많이 사귀었나요 ?"
		} ;

	static char *say_all[] = {
		"라라라라 ~~ 사람이 많이 오면 기분이 좋아진단 말야.",
		"여기 까지 오는데 힘드 셨죠 ?",
		"힘들면 좀 앉아서 쉬었다 가세요 :)",
		"여러분들을 보니까 기분이 참 좋아요.",
		"여러분들 얼굴을 직접 볼 수 있으면 더 좋을텐데... :)"
		} ;

	static char *whisper_to[] = {
		"오늘은 기분이 어때요 ?", 
		"이런 좋은 날 어디 안가세요 ?",
		"머드 학교 졸업하면 가장 먼저 뭘 하고 싶으세요 ?"
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
					act("Gracia님이 '흠..' 하고 말합니다", 1, ch, 0, 0, TO_ROOM);
				else
					act("$n님이 곰곰이 생각 합니다.", 1, ch, 0, 0, TO_ROOM);
				break ;
			case G_SMILE :
				if ( gracia_lastperson != NULL &&
					gracia_lastperson->in_room == ch->in_room) {
					act("$n님이 $N님을 보고 미소를 짓습니다.", FALSE, ch, 0,
						gracia_lastperson, TO_NOTVICT) ;
					send_to_char("Gracia님이 당신을 보고 방긋 웃습니다.\n\r",
						gracia_lastperson) ;
					act("$n님의 눈가에 웃음이 번집니다.", 1, ch, 0, 0, TO_ROOM);
					}
				break ;
			}
		gracia_response = G_NONE ;
		gracia_lastperson = NULL ;
		}

	if ( n_person + number(0,4) > 4 ) {
		sprintf(buf, "Gracia님이 '%s' 라고 말합니다.", say_all[number(0,3)]) ;
		act(buf, 1, ch, 0, 0, TO_ROOM);
		}
	else if ( number(0,4) < 3 ) {	/* say to */
		if (response_to != NULL) {
			sprintf(buf, "Gracia님이 '%s' 라고 말합니다.", say_to[number(0,3)]) ;
			act(buf, 1, ch, 0, response_to, TO_ROOM);
			}
		}
	else {		/* whisper to */
		if ( response_to != NULL &&
			response_to->in_room == ch->in_room) {
			sprintf(buf, "Gracia님이 당신에게 '%s' 라고 속삭입니다.\n\r",
				whisper_to[number(0,2)]) ;
			act("$n님이 $N님의 귀에 작은 소리로 무엇인가 말합니다.",
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

		sprintf( buf, "%s %s\n\r", "잘 하셨습니다.",
			"당신은 이 학교에서 해야 할 일을 잘 마치셨습니다.") ;
		send_to_char(buf,  vict) ;
		set_solved_quest( vict, QUEST_SCHOOL) ;
		act("$n님이 $N님에게 졸업장을 수여 합니다.", FALSE, ch, 0,  vict, TO_NOTVICT) ;
		send_to_char("Nara님이 당신에게 졸업장을 줍니다.\n\r",  vict) ;
		if( GET_LEVEL( vict) < 2 ) 
		{
			GET_LEVEL( vict) = 2 ;
			advance_level( vict) ;
			set_title( vict) ;
			send_to_char("Nara님이 당신의 레벨을 올려 주었습니다.\n\r",	 vict) ;
		}

        if( (recall_num = real_objectNr( 3052 )) != OBJECT_NULL )
		{
            log("nara> no such recall");
            return(TRUE);
        }
        recall = load_a_object( recall_num, REAL, 1 );
        obj_to_char( recall,  vict );
		act("$n님이 당신에게 scroll of recall 을 줍니다.",	FALSE, ch, 0,  vict, TO_VICT) ;
	}
	return TRUE ;
}
