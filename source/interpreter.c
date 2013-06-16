/* ************************************************************************
*  file: Interpreter.c , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/*#define __TRACE__
*/

#include "defines.h"
#include "character.h"
#include "sockets.h"
#include "object.h"
#include "mobile.h"
#include "world.h"
#include "comm.h"
#include "strings.h"
#include "interpreter.h"
#include "variables.h"
#include "affect.h"

#define POS_FIGHT	POSITION_FIGHTING
#define POS_DEAD	POSITION_DEAD
#define POS_SIT		POSITION_SITTING
#define POS_REST	POSITION_RESTING
#define POS_STAND	POSITION_STANDING
#define POS_SLEEP	POSITION_SLEEPING

void do_nil( charType * ch, char * argu, int cmd )
{}

static commandType  acmd[] =
{
  {"assist", 	COM_ASSIST, 		POS_FIGHT, 	do_assist,		  	1,	1 },
  {"account",	COM_ACCOUNT,		POS_DEAD,	do_bank,			0, 	0 },
  {"advance",	COM_ADVANCE,		POS_REST,  	do_advance,	  		0,	0 },
  {"afk",		COM_AFK,			POS_DEAD,	do_afk,				0,  0 },
  {"allow",		COM_ALLOW,			POS_DEAD,	do_bank,			0, 	0 },
  {"ambush",	COM_AMBUSH,			POS_STAND,	do_ambush,		   15,  0 },
  {"ask",	  	COM_ASK,			POS_REST,  	do_ask,		  		0,	0 },
  {"at", 		COM_AT, 			POS_DEAD,	do_at,		    IMO+1,	0 },
  {"attribute",	COM_ATTRIBUTE,		POS_DEAD,  	do_attribute, 		5,	0 },
  {"autoexit",  COM_AUTOEXIT,    	POS_DEAD,  	0,    			 	0,  0 },
  {"a",			0,			   		0,		  	do_nil, 	  	  	0,	0 },
};

static commandType  bcmd[] =
{
  {"backstab",	COM_BACKSTAB,		POS_DEAD,	do_backstab,	  	1, 	1 },
  {"balance",	COM_BALANCE,		POS_DEAD,	do_bank,			0, 	0 },
  {"bang",		COM_BANG,			POS_FIGHT,	do_bang,			0,  0 },
  {"bank",		COM_BANK,			POS_DEAD,	do_bank,			0,	0 },
  {"banish",  	COM_BANISH,			POS_DEAD,	do_banish,		IMO+1,	0 },
  {"bash",		COM_BASH,			POS_FIGHT,	do_bash,		  	1,	1 },
  {"berserk",   COM_BERSERK,		POS_FIGHT,  do_berserk,		   10,  1 },
  {"brief",		COM_BRIEF,			POS_DEAD,	do_brief,		    0,	0 },
  {"buy",	    COM_BUY,			POS_REST,	do_not_here,	  	0,	0 },
  {"bug",       COM_BUG,         	POS_DEAD,   do_typo,   			0,  0 },
  {"b",		    0,			    	0,		  	do_nil, 	  		0,	0 },
};

static commandType ccmd[] =
{
  {"cast",		COM_CAST,	  		POS_SIT,	do_cast,			1,	0 },
  {"chat", 		COM_CHAT,	  		POS_DEAD,	do_chat,		   15, 	0 },
  {"chain punch",COM_CHAIN_PUNCH,	POS_FIGHT,	do_chain_punch,	   25,  1 },
  {"checklocker",COM_CHECKLOCKER,	POS_DEAD,	do_checklocker,	IMO+1,	0 },
  {"checkrent",	COM_CHECKRENT,		POS_DEAD,	do_checkrent,	IMO+1,	0 },
  {"chop",		COM_CHOP,			POS_FIGHT,	do_chop,		IMO+2,	0 },
  {"close",		COM_CLOSE,	    	POS_STAND,	do_close,		    0,	0 },
  {"commands",  COM_COMMANDS,		POS_SLEEP,	do_commands,		0,	0 },
  {"compact",  	COM_COMPACT,		POS_DEAD,	do_compact,			0,	0 },
  {"consider",  COM_CONSIDER,		POS_REST,	do_consider,	  	0,	1 },
  {"credits",	COM_CREDITS,		POS_DEAD,	do_credits,			0,	0 },
  {"c",		    0,			    	0,		    do_nil, 	  		0,	0 },
};

static commandType dcmd[] =
{
  {"down",	  	COM_DOWN,	  		POS_STAND,	do_move,			0,	0 },
  {"dazzle",	COM_DAZZLE,			POS_FIGHT,	do_dazzle,		   40,  1 },
  {"damage",	COM_DAMAGE,			POS_REST,	do_damage,		IMO+1,	0 },
  {"date",		COM_DATE,			POS_DEAD,	do_date,			0,  0 },
  {"data",	  	COM_DATA,			POS_DEAD,	do_data,		  IMO, 	0 },
  {"deafen",	COM_DEAFEN,			POS_FIGHT,	do_deafen,		   23,  1 },
  {"demote",    COM_DEMOTE,	    	POS_FIGHT,	do_demote,		IMO+2,  0 },
  {"deposit",	COM_DEPOSIT,		POS_DEAD,	do_bank,			0, 	0 },
  {"disallow",	COM_DISALLOW,		POS_DEAD,	do_bank,			0, 	0 },
  {"drink",     COM_DRINK,	   		POS_REST,	do_drink,		    0,	0 },
  {"drop",	  	COM_DROP,	  		POS_REST,	do_drop,		    0,	0 },
  {"d",		    0,			    	0,		    do_nil,	 	    	0,	0 },
};

static commandType ecmd[] =
{
  {"east",		COM_EAST,			POS_STAND,	do_move,     		0,	0 },
  {"eat",		COM_EAT,			POS_SIT,	do_eat,				0,	0 },
  {"echo",      COM_ECHO,			POS_SLEEP,	do_echo,		  IMO,	0 },
  {"edit",		COM_EDIT,			POS_STAND,	do_edit,		IMO+2,	0 },
  {"emote",		COM_EMOTE,			POS_SLEEP,	do_emote,			0,  0 },
  {"enter",		COM_ENTER,			POS_STAND,	do_enter,			0,	0 },
  {"equipment",	COM_EQUIPMENT,		POS_SLEEP,	do_equipment,		0,	0 },
  {"exits",		COM_EXITS,			POS_REST,	do_exits,			0,	0 },
  {"examine",	COM_EXAMINE,		POS_SIT,	do_examine,			0,  0 },
  {"extractrent",COM_EXTRACTRENT,	POS_STAND,	do_extractrent,	IMO+2,  0 },
  {"e",			0,					0,			do_nil,	 			0,	0 },
};

static commandType fcmd[] =
{
  {"flee",      COM_FLEE,			POS_FIGHT, 	do_flee,			0,	0 },
  {"flash",		COM_FLASH,			POS_FIGHT,	do_flash,			1,  1 },
  {"flag",		COM_FLAG,			POS_DEAD,	do_flag,		IMO+3,  0 },
  {"follow",    COM_FOLLOW,			POS_REST,	do_follow,			0,  0 },
  {"force",		COM_FORCE,			POS_STAND,	do_force,		IMO+2,  0 },
  {"flick",		COM_FLICK,			POS_DEAD,	do_flick,		IMO+3,  0 },
  {"f",			0,					0,			do_nil, 			0,	0 },
};

static commandType gcmd[] =
{
  {"get",    	COM_GET,		  	POS_REST,   do_get,				0,  0 },
  {"give",	  	COM_GIVE,	  		POS_REST,	do_give,			0,  0 },
  {"goto",      COM_GOTO,	  		POS_REST,	do_goto,		  IMO,	0 },
  {"group",	  	COM_GROUP,			POS_SLEEP,	do_group,		  	1,  0 },
  {"grab",	  	COM_GROUP,			POS_REST,	do_grab,			0,	0 },
  {"gtell",		COM_GTELL,	    	POS_DEAD,	do_gtell,		  	0, 	0 },
  {"g",		    0,			    	0,		    do_nil, 			0,  0 },
};

static commandType hcmd[] =
{
  {"hit",	    COM_HIT,			POS_FIGHT,	do_hit,		  		0,	1 },
  {"hangul",	COM_HANGUL,			POS_DEAD,	do_hangul,			0,  0 },
  {"help",	  	COM_HELP,	  		POS_DEAD,   do_help,			0,	0 },
  {"hide",	  	COM_HIDE,	  		POS_REST,	do_hide,			1,	0 },
  {"hold",		COM_HOLD,			POS_REST,	do_grab,			0,	0 },
  {"h",			0,					0,		    do_nil, 			0,	0 },
};

static commandType icmd[] =
{
  {"inventory",	COM_INVENTORY,		POS_DEAD,	do_inventory,	  	0,	0 },
  {"idea",      COM_IDEA,         	POS_DEAD,   do_typo,   			0,  0 },
  {"iedit",		COM_IEDIT,			POS_DEAD,	do_iedit,		 IMO+2, 0 },
  {"info",	  	COM_INFO,			POS_DEAD,	do_info,			0,	0 },
  {"insurance", COM_INSURANCE,		POS_SLEEP,	do_insurance,		0,  0 },
  {"insure",    COM_INSURE,			POS_SIT,	do_insure,			0,  0 },
  {"insult",	COM_INSULT,			POS_SIT,	do_insult,		    0,	0 },
  {"invis",		COM_INVIS,       	POS_DEAD,	do_invis,		  IMO,	0 },
  {"ifind",		COM_IFIND,       	POS_DEAD,	do_ifind,		  IMO,	0 },
  {"istat",		COM_ISTAT,       	POS_DEAD,	do_istat,		  IMO,	0 },
  {"i",		    0,			    	0,		  	do_nil,	 	        0,	0 },
};

static commandType jcmd[] =
{
  {"junk",	  	COM_JUNK,	  		POS_SIT,	do_junk,		  	1,  0 },
  {"j",		    0,			    	0,		  	do_nil, 		  	0,	0 },
};

static commandType kcmd[] =
{
  {"kill",		COM_KILL,			POS_FIGHT,	do_hit,				0,	1 },
  {"kick",	  	COM_KICK,			POS_FIGHT,	do_kick,			1,	1 },
  {"k",		    0,			    	0,			  	do_nil, 	  	0,	0 },
};

static commandType lcmd[] =
{
  {"look",	 	COM_LOOK,	    	POS_REST,  	do_look,			0,	0 },
  {"lastchat",	COM_LASTCHAT,		POS_DEAD,	do_lastchat,		0,  0 },
  {"leave",		COM_LEAVE,	    	POS_STAND,	do_leave,		    0,	0 },
  {"levels",	COM_LEVELS,	    	POS_DEAD,	do_levels,		  	0,  0 },
  {"lightning",	COM_LIGHT,			POS_FIGHT,	do_light_move,		1, 	0 },
  {"list",		COM_LIST,			POS_SIT,	do_not_here,	  	0,	0 },
  {"lock",     	COM_LOCK,	    	POS_SIT,	do_lock,			0,	0 },
  {"log",     	COM_LOG,	    	POS_SLEEP,	do_flag,		IMO+3,	0 },
  {"load",	 	COM_LOAD,	    	POS_REST,	do_load,		IMO+1,  0 },
  {"l",		    0,					0,			do_nil, 		    0,	0 },
};

static commandType mcmd[] =
{
  {"multikick", COM_MULTI,			POS_FIGHT,	do_multi_kick,	   15, 	1 },
  {"mail",		COM_MAIL,			POS_DEAD,	do_mail,			0,  0 },
  {"masquerade",COM_MASQUERADE,		POS_FIGHT,	do_masquerade,	   30,  1 },
  {"medit",		COM_MEDIT,			POS_DEAD,	do_medit,		IMO+2,  0 },
  {"morpeth", 	COM_MORPETH,		POS_STAND,	do_morpeth,		   18,  0 },
  {"motd",		COM_MOTD,			POS_DEAD,	do_motd,			0,  0 },
  {"memory",	COM_MEMORY,			POS_DEAD,	do_memory,		  IMO, 	0 },
  {"mfind",	 	COM_MFIND,	    	POS_REST,	do_mfind,		  IMO,  0 },
  {"mstat",	    COM_MSTAT,			POS_DEAD,	do_mstat,		  IMO, 	0 },
  {"m",		    0,			    	0,		    do_nil, 		    0,	0 },
};

static commandType ncmd[] =
{
  {"north",	    COM_NORTH,			POS_STAND,	do_move,			0,	0 },
  {"news",	    COM_NEWS_LOW,    	POS_DEAD,	do_news,			0,	0 },
  {"nochat",	COM_CHAT,			POS_DEAD,	do_nochat,		   15, 	0 },
  {"noshout",   COM_NOSHOUT,		POS_DEAD,	do_noshout,	        0,  0 },
  {"notell",	COM_NOTELL,			POS_DEAD,	do_notell,		   20,	0 },
  {"noaffect",	COM_NOAFFECT,		POS_DEAD,	do_noaffect,	IMO+1,  0 },
  {"nstat",	    COM_NSTAT,			POS_DEAD,	do_stat,		  IMO, 	0 },
  {"n",		    0,			    	0,		    do_nil, 	    	0,	0 },
};

static commandType ocmd[] =
{
  {"order",	   	COM_ORDER,	    	POS_SIT,	do_order,		    0,	1 },
  {"open",		COM_OPEN,	    	POS_SIT,	do_open,			0,  0 },
  {"ostat",     COM_OSTAT,       	POS_DEAD,   do_stat,          IMO,  0 },
  {"o",		    0,			    	0,			do_nil, 		    0,	0 },
};

static commandType pcmd[] =
{
  {"put",	    COM_PUT,		    POS_REST,	do_put,		    	0,	0 },
  {"peace",		COM_PEACE,			POS_DEAD,	do_peace,		  IMO,  0 },
  {"peek",		COM_PEEK,			POS_REST,	do_peek,            0,  0 },
  {"pick",		COM_PICK,			POS_SIT,	do_pick,			1,	0 },
  {"purge",	    COM_PURGE,	    	POS_DEAD,	do_purge,		IMO+1,	0 },
  {"police",  	COM_POLICE,			POS_DEAD,	do_police,		IMO+3,	0 },
  {"post",		COM_POST,	    	POS_REST,	do_not_here,	    0,	0 },
  {"pour",		COM_POUR,			POS_SIT,	do_pour,        	0,	0 },
  {"practice",	COM_PRACTICE,		POS_REST,	do_practice,	  	0,	0 },
  {"press",		COM_PRESS,			POS_SIT,	do_nothing,			0,  0 },
  {"punch",		COM_PUNCH,			POS_FIGHT,	do_punch,			1,  1 },
  {"pstat",	    COM_STAT,			POS_DEAD,	do_stat,		  IMO, 	0 },
  {"p",		    0,			    	0	,		    do_nil, 	   	0,	0 },
};

static commandType qcmd[] =
{
  {"quaff",	    COM_QUAFF,	    	POS_REST,	do_quaff,		    0,	0 },
  {"quest",		COM_QUEST,			POS_DEAD,	do_quest,			0,  0 },
  {"qui",		COM_QUI,			POS_DEAD,	do_qui,	       		0,	0 },
  {"quit",	  	COM_QUIT,			POS_DEAD,	do_quit,			0,	0 },
  {"q",		    0,			    	0,		    do_nil, 	 	  	0,	0 },
};

static commandType Rcmd[] =
{
  {"rescue",  	COM_RESCUE,	  		POS_FIGHT,	do_rescue,		  	1,	0 },
  {"read",	  	COM_READ,			POS_REST,	do_read,			0,	0 },
  {"recite",	COM_RECITE,			POS_REST,	do_recite,		  	0,	0 },
  {"redit",		COM_REDIT,			POS_DEAD,	do_redit,		IMO+2,  0 },
  {"reload",	COM_RELOAD,			POS_REST,	do_reload,		    0, 	0 },
  {"remove",  	COM_REMOVE,			POS_REST,	do_remove,		  	0,	0 },
  {"rent",		COM_RENT,	    	POS_DEAD,	do_rent,	  		0,	0 },
  {"repair",	COM_REPAIR,			POS_REST,	do_not_here,		0,  0 },
  {"report",	COM_REPORT,			POS_SLEEP,	do_report,		  	0, 	0 },
  {"replacerent",COM_REPLACERENT,	POS_REST,	do_replacerent, IMO+2,	0 },
  {"reroll",	COM_REROLL,			POS_DEAD,	do_reroll,		IMO+3,  0 },
  {"rest",		COM_REST,			POS_REST,	do_rest,			0,	0 },
  {"restore",	COM_RESTORE,		POS_DEAD,	do_restore,	  	  IMO,	0 },
  {"retrieve",  COM_RETRIEVE,		POS_SIT,    do_retrieve,		0,  0 },
  {"return",	COM_RETURN,	    	POS_DEAD,	do_return,	  		0,	0 },
  {"rfind",     COM_RFIND,       	POS_DEAD,   do_rfind,         IMO,  0 },
  {"rstat",     COM_RSTAT,       	POS_DEAD,   do_rstat,         IMO,  0 },
  {"r",			0,			    	0,		    do_nil, 	    	0,	0 },
};

static commandType scmd[] =
{
  {"south",		COM_SOUTH,			POS_STAND,	do_move,			0, 	0 },
  {"say",	    COM_SAY,			POS_REST,	do_say,	   			0,	0 },
  {"save",     	COM_SAVE,			POS_DEAD,	do_save,			0,	0 },
  {"saveworld", COM_SAVEWORLD,		POS_DEAD,	do_saveworld,   IMO+3,	0 },
  {"savezone",  COM_SAVEZONE,		POS_DEAD,	do_savezone,	IMO+3,	0 },
  {"score",		COM_SCORE,			POS_DEAD,	do_score,		    0,	0 },
  {"screen",	COM_SCREEN,			POS_DEAD,	do_screen,			0,	0 },
  {"scroll",	COM_SCROLL,			POS_DEAD,	do_scroll,			0,  0 },
  {"sit",	    COM_SIT,		    POS_REST,	do_sit,		    	0, 	0 },
  {"sell",	    COM_SELL,	  		POS_SIT,	do_not_here,	  	0,	0 },
  {"send",		COM_SEND,			POS_DEAD,	do_send,		   19, 	0 },
  {"serpent",	COM_SERPENT,		POS_FIGHT,	do_serpent,		   35,  0 },
  {"set",		COM_SET,			POS_DEAD,	do_set,			IMO+3,  0 },
  {"shout",		COM_SHOUT,			POS_REST,	do_shout,		    5,	0 },
  {"shoot",		COM_SHOOT,	    	POS_REST,	do_shoot,		    1,	1 },
  {"shutdow",	COM_SHUTDOW,		POS_DEAD,	do_shutdow,	  	IMO+3,	0 },
  {"shutdown",	COM_SHUTDOWN,		POS_DEAD,	do_shutdown,    IMO+3,	0 },
  {"sip",	    COM_SIP,		    POS_REST,   do_sip,		    	0, 	0 },
  {"sleep",		COM_SLEEP,			POS_SLEEP,	do_sleep,		    0,	0 },
  {"sneak",		COM_SNEAK,			POS_REST,	do_sneak,		    0,	0 },
  {"snoop",		COM_SNOOP,			POS_DEAD,	do_snoop,		IMO+2,  0 },
  {"socials",	COM_SOCIALS,		POS_DEAD,	do_socials,			0,	0 },
  {"spells",    COM_SPELLS,			POS_SLEEP,	do_spells,		    0, 	0 },
  {"split",     COM_SPLIT,			POS_REST,	do_split,			0,  0 },
  {"stand",		COM_STAND,			POS_REST,	do_stand,		    0,	0 },
  {"stat",		COM_STAT,	    	POS_DEAD,	do_stat,		  IMO, 	0 },
  {"steal",		COM_STEAL,			POS_STAND,	do_steal,		    0,	1 },
  {"string",    COM_STRING,      	POS_SLEEP,	do_string,		  IMO,	0 },
  {"swirl",		COM_SWIRL,			POS_FIGHT,	do_swirl,			0,  0 },
  {"switch",  	COM_SWITCH,			POS_DEAD,	do_switch,		IMO+1,  0 },
  {"sys",		COM_SYS,		    POS_DEAD,	do_sys,		  	  IMO, 	0 },
  {"s",		    0,			    	0,		    do_nil,	 	    	0,	0 },
};

static commandType tcmd[] =
{
  {"tell",		COM_TELL,			POS_REST,	do_tell,			0,	0 },
  {"take",		COM_TAKE,			POS_REST,	do_get,		    	0,	0 },
  {"taste",	    COM_TASTE,			POS_REST,	do_taste,	      	0,	0 },
  {"throw",		COM_THROW,			POS_FIGHT,  do_throw,			0,  0 },
  {"title",		COM_TITLE,			POS_SLEEP,	do_title,		    0, 	0 },
  {"time",		COM_TIME,			POS_DEAD, 	do_time,			0,	0 },
  {"tornado",	COM_TORNADO,		POS_FIGHT,	do_tornado,	    	1, 	1 },
  {"todo",		COM_TODO,			POS_DEAD,	do_todo,			0,  0 },
  {"track",     COM_TRACK,			POS_STAND,	do_track,			0,  0 },
  {"transfer",	COM_TRANSFER,		POS_REST,	do_trans,		IMO+1,	0 },
  {"typo",     	COM_TYPO,			POS_DEAD,	do_typo,			0,	0 },
  {"t",			0,					0,			do_nil, 		  	0,	0 },
};

static commandType ucmd[] =
{
  {"up",	    COM_UP,		    	POS_STAND,	do_move,			0,	0 },
  {"unlock",	COM_UNLOCK,			POS_SIT,	do_unlock,		  	1,	0 },
  {"ungroup",	COM_UNGROUP,		POS_SIT,	do_ungroup,			1,  0 },
  {"use",	    COM_USE,			POS_SIT,	do_use,		    	1,	0 },
  {"users",		COM_USERS,			POS_DEAD,	do_users,		    1,	0 },
  {"u",			0,			    	0,			do_nil, 		  	0,	0 },
};

static commandType vcmd[] =
{
  {"value",		COM_VALUE,	    	POS_REST,	do_not_here,	  	0,	0 },
  {"v",		    0,			    	0,		  	do_nil, 	  		0,	0 },
};

static commandType wcmd[] =
{
  {"west",		COM_WEST,			POS_STAND,	do_move,		  	0,	0 },
  {"wake",		COM_WAKE,			POS_SLEEP,	do_wake,		  	0,	0 },
  {"wall",		COM_WALL,			POS_DEAD,	do_wall,		  IMO,	0 },
  {"wear",		COM_WEAR,			POS_REST,	do_wear,		  	0,	0 },
  {"who",	    COM_WHO,		    POS_DEAD,	do_who,		   		0,	0 },
  {"wield",		COM_WIELD,			POS_REST,	do_wield,	    	0,	0 },
  {"wizards",   COM_WIZARDS,		POS_DEAD,	do_wizards,			0,  0 },
  {"weather",	COM_WEATHER,		POS_REST,	do_weather,		    0,	0 },
  {"where",		COM_WHERE,			POS_DEAD,	do_where,	    	2,	0 },
  {"whisper",	COM_WHISPER,		POS_REST,	do_whisper,			0,	0 },
  {"wimpy",		COM_WIMPY,			POS_FIGHT,	do_wimpy,	    	0, 	0 },
  {"withdraw",	COM_WITHDRAW,		POS_DEAD,	do_bank,	    	1, 	0 },
  {"wiznet",	COM_WIZNET,			POS_DEAD,	do_wiznet,	  	  IMO,	0 },
  {"wizhelp",   COM_WIZHELP,		POS_DEAD,	do_wizhelp,	  	  IMO,	0 },
  {"wizlock",	COM_WIZLOCK,		POS_DEAD,	do_wizlock,		IMO+2,	0 },
  {"write",		COM_WRITE,			POS_REST,	do_write,	    	0, 	0 },
  {"w",			0,					0,			do_nil,  			0,	0 },
};

static commandType xcmd[] =
{
  {"x",			0,			    	0,		    do_nil, 		    0,	0 },
};

static commandType ycmd[] =
{
  {"yell",		COM_YELL,			POS_SIT,	do_yell,			0,  0 },
  {"y",		    0,					0,			do_nil,  			0,	0 },
};

static commandType zcmd[] =
{
  {"zone",      COM_ZONE,           POS_DEAD,   do_zone,            0,  0 },
  {"zap", 		COM_QUIT, 			POS_DEAD, 	do_quit,	   		0,	0 },
  {"redit",		COM_ZEDIT,			POS_DEAD,	do_zedit,		IMO+2,  0 },
  {"z",		    0,			    	0,		  	do_nil,  			0,	0 },
};

static commandType etccmd[] =
{
  {".",	     	COM_EMOTE,	    	POS_SIT,	do_emote, 	    	1,	0 },
  {",",			COM_GTELL,	    	POS_DEAD,	do_gtell,	      	1,	0 },
  {"'",		    COM_SAY,			POS_REST,	do_say,		   		0,	0 },
  {"\"",		COM_CHAT,			POS_DEAD,	do_chat,		   15,	0 },
  {"",			0,					0,			do_nil,    			0,	0 }, 
};

static char	*	etc_str = ".,':\"";

commandType * commands['z'-'a'+3] =
{
	acmd, bcmd, ccmd, dcmd, ecmd, fcmd, gcmd, hcmd, icmd, jcmd,
	kcmd, lcmd, mcmd, ncmd, ocmd, pcmd, qcmd, Rcmd, scmd, tcmd,
    ucmd, vcmd, wcmd, xcmd, ycmd, zcmd, etccmd, 0
};

/*
static commandType hancmd[] =
{
  {"말",		COM_HAN_SAY,		POS_REST,	do_say,				1,  0 },
  {"ㅣ",		COM_LOOK,			POS_REST,	do_look,			1,  0 },
  {"체팅",		COM_CHAT,			POS_DEAD,	do_chat,			1,  0 },
  {"누구",		COM_WHO,			POS_DEAD,	do_who,				1,  0 },
  {"어디",		COM_WHERE,			POS_SLEEP,	do_where,			1,  0 },
  {"잠",		COM_SLEEP,			POS_REST,	do_sleep,			1,  0 },
  {"",			0,					0,			do_nil,    			0,	0 },
};
*/
/*
  "NEWS",
  "",
  "commands",
  "press",
  "pull",
  "\n"	
*/


void Huh(charType *ch)
{
	int lev ;

	lev = GET_LEVEL(ch) ;
	if ( lev < 3 )
		send_to_char_han("What ? Try 'help' and 'command'.\n\r",
			"무슨말이죠 ? 'help' 와 'command' 를 쳐보세요.\n\r", ch) ;
	else if (lev < 13 ) send_to_char_han("Huh?\n\r", "뭐?\n\r", ch);
	else if ( lev < 31)
		send_to_char_han("What?\n\r", "예 ? 뭐요 ?\n\r", ch);
	else if ( lev < IMO)
		send_to_char_han("What did you say?\n\r", "뭐라고 하셨습니까 ?\n\r", ch);
	else 
		send_to_char_han("What did you say, Sir ?\n\r",
			"네? 다시 말씀하십시오...\n\r", ch);
}

int special( charType * ch, int cmd, char * arg )
{
  	objectType 	*	i;
  	charType 	*	k;
  	int 			j;

	/* special in room? */
	if (world[ch->in_room].func)
	 	if ((*world[ch->in_room].func)(ch, cmd, arg))
			return(1);

	/* special in equipment list? */
	for (j = 0; j <= (MAX_WEAR - 1); j++)
	   if (ch->equipment[j] && ch->equipment[j]->nr>=0)
		  if (objects[ch->equipment[j]->nr].func)
			 if ((*objects[ch->equipment[j]->nr].func)
				(ch, cmd, arg))
					return(1);

	/* special in inventory? */
	for (i = ch->carrying; i; i = i->next_content)
	  if (i->nr>=0)
		if (objects[i->nr].func)
			if ((*objects[i->nr].func)(ch, cmd, arg))
			   return(1);

	/* special in mobile present? */
	for( k = world[ch->in_room].people; k; k = k->next_in_room)
	   if ( IS_MOB(k) && !IS_AFFECTED( k, AFF_TIME_STOP ) )
		  if (mobiles[k->nr].func)
			 if ((*mobiles[k->nr].func)(ch, cmd, arg))
				return(1);

  	/* special in object present? */
  	for( i = world[ch->in_room].contents; i; i = i->next_content)
     	if( i->nr >= 0 )
        	if( objects[i->nr].func )
           		if((*objects[i->nr].func )(ch, cmd, arg))
              		return(1);

  	return(0);
}

commandType * cmd_lookup( char * str )
{
	int				i;
	char		*	cp, * tp;
	commandType	*	cmds;

	cp = str;

	if( *cp & 0x80 )
	{
		return NULL;
	}
	else
	{
		if( isalpha( *cp ) )
		{
			cmds = commands[ tolower(*cp) - 'a' ];

			for( i = 0; cmds[i].name[1]; i++ )
			{
				if( isprefix( cp, cmds[i].name ) ) return &cmds[i];
			}
		}
		
		if( !*(cp+1) && (tp = strchr( etc_str, *cp )) )
		{
			return &etccmd[ tp - etc_str ];
		}
	}
	return NULL;
}

int interpreter( charType * ch, char * argument, int order ) 
{
	commandType	*	info;
	char			buf[128];
	char	* 		args;

  	REMOVE_BIT(ch->affects, AFF_HIDE);
  	REMOVE_BIT(ch->affects, AFF_AMBUSH);

	if( IS_AFFECTED( ch, AFF_TIME_STOP ) )
	{
		DEBUG( "interpreter> time_stoped %s called.(%s)", ch->name, argument );
		return 1;
	}

	if( IS_AFFECTED( ch, AFF_CHARM ) && !order )
	{
		sendf( ch, "It's not a order from your nice fellow, is it?" );
		return 1;
	}

  	argument = skipsps( argument );

  	if( !*argument ) 
  	{
  		if( IS_SET( ch->act, PLR_AFK ) )
  		{
			sendf( ch, "Are you not away from keyboard?" );
			act( "$n has returned to $s keyboard.", FALSE, ch, 0, 0, TO_ROOM );
			REMOVE_BIT( ch->act, PLR_AFK );
		}
  		return 0;
  	}

	args = onefword( argument, buf );
  	info = cmd_lookup( buf );

	TRACE( trace("%s> %s [%d] - %s", GET_NAME(ch), buf, (info ? info->cmdNr : -1), argument ) );

  	if( info && GET_LEVEL(ch) < info->level ) 
  	{
    	sendf( ch, "You are too lowly to use that command." );
    	return(1);
  	}

  	if( info && IS_NPC( ch ) && info->level >= IMO )
  	{
    	sendf( ch, "Mob could not be a wizard." );
    	return(1);
  	}

  	if( info && info->func ) 
  	{
    	if( GET_POS(ch) < info->pos )
		{
      		switch(GET_POS(ch)) 
	  		{
			case POSITION_DEAD:
			  sendf( ch, "Lie still; you are DEAD!!! :-(" ); 							break;
			case POSITION_INCAP:
			case POSITION_MORTALLYW:
			  sendf( ch, "You are in a pretty bad shape, unable to do anything!" ); 	break;
			case POSITION_STUNNED:
			  sendf( ch, "All you can do right now, is think about the stars!" ); 		break;
			case POSITION_SLEEPING:
			  sendf( ch, "In your dreams, or what?" ); 									break;
			case POSITION_RESTING:
			  sendf( ch, "Nah... You feel too relaxed to do that.." ); 					break;
			case POSITION_SITTING:
			  sendf( ch, "Maybe you should get on your feet first?" ); 					break;
			case POSITION_FIGHTING:
			  sendf( ch, "No way! You are fighting for your life!" ); 					break;
      		} 
	  	}
	  	else 
		{
			if( IS_SET( ch->act, PLR_AFK ) && info->cmdNr != COM_AFK )
			{
				sendf( ch, "Are you not away from keyboard?" );
				act( "$n has returned to $s keyboard.", FALSE, ch, 0, 0, TO_ROOM );
				REMOVE_BIT( ch->act, PLR_AFK );
			}
			if( IS_SET( ch->act, PLR_XYZZY ) || (OMNI( ch )) )
				log( "%s> %s", GET_NAME( ch ), argument );

      		if( !no_specials && special(ch, info->cmdNr, args ) ) return(1);  

     		(*info->func)( ch, args, info->cmdNr );

			if( IS_AFFECTED(ch, AFF_MORPETH) && info->aggressive )
				REMOVE_BIT( ch->affects, AFF_MORPETH );
    	}
    	return(1);
  	}

  	if( info && !info->func ) sendf( ch, "Sorry, but that command has yet to be implemented..." );
  	else 					  
  	{
  		if( !check_social( ch, buf, args ) )
  		{
  			sendf( ch, "Huh?" ); return 0;
  		}
  	}
  	return(1);
}
