#ifndef __INTERPRETER_H
#define __INTERPRETER_H

#ifndef __DEFINES_H
#include "defines.h"
#endif

/* ************************************************************************
*  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
************************************************************************* */

int 	interpreter			( charType * ch, char * argument, int order );
void 	nanny				( descriptorType * d, char * arg );
void 	Huh					( charType * ch );

/* in act.social.c */
int		check_social		( charType * ch, char * cmd, char * args );

typedef struct __commandtype__
{
	char		* 	name;
	short		  	cmdNr;
	byte			pos;
	void 			(*func) ( charType *ch, char *argument, int cmd );
	byte			level;
	byte			aggressive;

} commandType;

extern struct __commandtype__ * commands[];

/*
	definition of command number
*/

#define COM_NONE				0
#define	COM_NORTH				1
#define	COM_EAST				2
#define	COM_SOUTH				3
#define	COM_WEST				4
#define	COM_UP					5
#define	COM_DOWN				6
#define	COM_ENTER				7
#define	COM_EXITS				8
#define	COM_GET					10
#define	COM_DRINK				11
#define	COM_EAT					12
#define	COM_WEAR				13
#define	COM_WIELD				14
#define	COM_LOOK				15
#define	COM_SCORE				16
#define	COM_SAY					17
#define	COM_SHOUT				18
#define	COM_TELL				19
#define	COM_INVENTORY			20
#define	COM_QUI					21
#define	COM_KILL				25
#define	COM_INSULT				33
#define	COM_HELP				38
#define	COM_WHO					39
#define	COM_EMOTE				40
#define	COM_ECHO				41
#define	COM_STAND				42
#define	COM_SIT					43
#define	COM_REST				44
#define	COM_SLEEP				45
#define	COM_WAKE				46
#define	COM_FORCE				47
#define	COM_TRANSFER			48
#define	COM_NEWS_LOW			54
#define	COM_EQUIPMENT			55
#define	COM_BUY					56
#define	COM_SELL				57
#define	COM_VALUE				58
#define	COM_LIST				59
#define	COM_DROP				60
#define	COM_GOTO				61
#define	COM_WEATHER				62
#define	COM_READ				63
#define	COM_POUR				64
#define	COM_GRAB				65
#define	COM_REMOVE				66
#define	COM_PUT					67
#define	COM_SHUTDOW				68
#define	COM_SAVE				69
#define	COM_HIT					70
#define	COM_STRING				71
#define	COM_GIVE				72
#define	COM_QUIT				73
#define	COM_STAT				74
#define	COM_TIME				76
#define	COM_LOAD				77
#define	COM_PURGE				78
#define	COM_SHUTDOWN			79
#define	COM_TYPO				81
#define	COM_REPLACERENT			82
#define	COM_WHISPER				83
#define	COM_CAST				84
#define	COM_AT					85
#define	COM_ASK					86
#define	COM_ORDER				87
#define	COM_SIP					88
#define	COM_TASTE				89
#define	COM_SNOOP				90
#define	COM_FOLLOW				91
#define	COM_RENT				92
#define	COM_JUNK				93
#define	COM_ADVANCE				95
#define	COM_OPEN				99
#define	COM_CLOSE				100
#define	COM_LOCK				101
#define	COM_UNLOCK				102
#define	COM_LEAVE				103
#define	COM_WAVE				143
#define	COM_EXTRACTRENT			149
#define	COM_HOLD				150
#define	COM_FLEE				151
#define	COM_SNEAK				152
#define	COM_HIDE				153
#define	COM_BACKSTAB			154
#define	COM_PICK				155
#define	COM_STEAL				156
#define	COM_BASH				157
#define	COM_RESCUE				158
#define	COM_KICK				159
#define	COM_PRACTICE			164
#define	COM_EXAMINE				166
#define	COM_TAKE				167
#define	COM_INFO				168
#define	COM_PRACTISE			170
#define	COM_CURSE				171
#define	COM_USE					172
#define	COM_WHERE				173
#define	COM_LEVELS				174
#define	COM_REROLL				175
#define	COM_SHOOT				192
#define	COM_BRIEF				199
#define	COM_WIZNET				200
#define	COM_CONSIDER			201
#define	COM_GROUP				202
#define	COM_RESTORE				203
#define	COM_RETURN				204
#define	COM_SWITCH				205
#define	COM_QUAFF				206
#define	COM_RECITE				207
#define	COM_USERS				208
#define	COM_FLAG				209
#define	COM_NOSHOUT				210
#define	COM_WIZHELP				211
#define	COM_CREDITS				212
#define	COM_COMPACT				213
#define	COM_FLICK				214
#define	COM_WALL				215
#define	COM_SET					216
#define	COM_POLICE				217
#define	COM_WIZLOCK				218
#define	COM_NOAFFECT			219
#define	COM_INVIS				220
#define	COM_NOTELL				221
#define	COM_BANISH				222
#define	COM_RELOAD				223
#define	COM_DATA				224
#define	COM_CHECKRENT			225
#define	COM_CHAT				226
#define	COM_BALANCE				227
#define	COM_DEPOSIT				228
#define	COM_WITHDRAW			229
#define	COM_SYS					230
#define	COM_LOG					231
#define	COM_MSTAT				232
#define	COM_PSTAT				233
#define	COM_TORNADO				234
#define	COM_LIGHT				235
#define	COM_TITLE				236
#define	COM_REPORT				237
#define	COM_SPELLS				238
#define	COM_FLASH				239
#define	COM_MULTI				240
#define	COM_DEMOTE				241
#define	COM_NOCHAT				242
#define	COM_WIMPY				243
#define	COM_GTELL				244
#define	COM_SEND				245
#define	COM_WRITE				246
#define	COM_POST				247

#define COM_HAN_SAY				248

#define	COM_ASSIST				262
#define	COM_UNGROUP				263
#define	COM_WIZARDS				264
#define	COM_HANGUL				265
#define	COM_NEWS_UPPER			266
#define	COM_VERSION				267
#define	COM_LASTCHAT			268
#define	COM_DATE				269
#define	COM_MAIL				270
#define	COM_THROW				271
#define	COM_COMMANDS			272
#define	COM_PRESS				273
#define	COM_PULL				274
#define	COM_QUEST				275

#define	COM_SCROLL				276
#define	COM_REPAIR				277
#define	COM_PUNCH				278
#define	COM_MOTD				279
#define	COM_CHAIN_PUNCH			280
#define COM_AUTOEXIT			281
#define COM_RSTAT				282
#define COM_ISTAT				283
#define COM_ATTRIBUTE 			284
#define COM_BUG		  			285
#define COM_MEMORY	  			286
#define COM_ZONE	  			287
#define COM_IFIND	  			288
#define COM_MFIND	  			289
#define COM_RFIND	  			290
#define COM_OSTAT	  			291
#define COM_NSTAT	  			292
#define COM_BERSERK	  			293
#define COM_MORPETH	  			294
#define COM_DEAFEN	  			295
#define COM_MASQUERADE  		296
#define COM_ACCOUNT   			297
#define COM_ALLOW     			298
#define COM_DISALLOW  			299
#define COM_IDEA	  			300
#define COM_BANK	  			301
#define COM_INSURANCE 			302
#define COM_INSURE    			303
#define COM_RETRIEVE  			304
#define COM_SAVEWORLD 			305
#define COM_SAVEZONE  			306
#define COM_SPLIT	  			307
#define COM_AMBUSH	  			308
#define COM_EDIT	  			309
#define COM_IEDIT	  			310
#define COM_REDIT	  			311
#define COM_ZEDIT	  			312
#define COM_MEDIT	  			313
#define COM_CHECKLOCKER	  		314
#define COM_DAZZLE				315
#define COM_SERPENT				316
#define COM_YELL				317
#define COM_TODO				318
#define COM_SOCIALS				319
#define COM_TRACK				320
#define COM_PEEK				321
#define COM_AFK					322
#define COM_BANG				323
#define COM_SWIRL				324
#define COM_SCREEN				325
#define COM_CHOP				326
#define COM_DAMAGE				327
#define COM_PEACE				328

void do_shoot				(charType *ch, char *arg, int cmd);
void do_wimpy				(charType *ch, char *arg, int cmd);
void do_move				(charType *ch, char *arg, int cmd);
void do_look				(charType *ch, char *arg, int cmd);
void do_read				(charType *ch, char *arg, int cmd);
void do_write				(charType *ch, char *arg, int cmd);
void do_say					(charType *ch, char *arg, int cmd);
void do_sayh				(charType *ch, char *arg, int cmd);
void do_exit				(charType *ch, char *arg, int cmd);
void do_snoop				(charType *ch, char *arg, int cmd);
void do_insult				(charType *ch, char *arg, int cmd);
void do_quit				(charType *ch, char *arg, int cmd);
void do_help				(charType *ch, char *arg, int cmd);
void do_who					(charType *ch, char *arg, int cmd);
void do_emote				(charType *ch, char *arg, int cmd);
void do_echo				(charType *ch, char *arg, int cmd);
void do_trans				(charType *ch, char *arg, int cmd);
void do_kill				(charType *ch, char *arg, int cmd);
void do_stand				(charType *ch, char *arg, int cmd);
void do_sit					(charType *ch, char *arg, int cmd);
void do_rest				(charType *ch, char *arg, int cmd);
void do_sleep				(charType *ch, char *arg, int cmd);
void do_wake				(charType *ch, char *arg, int cmd);
void do_force				(charType *ch, char *arg, int cmd);
void do_get					(charType *ch, char *arg, int cmd);
void do_drop				(charType *ch, char *arg, int cmd);
void do_news				(charType *ch, char *arg, int cmd);
void do_score				(charType *ch, char *arg, int cmd);
void do_title				(charType *ch, char *arg, int cmd);
void do_spells				(charType *ch, char *arg, int cmd);
void do_report				(charType *ch, char *arg, int cmd);
void do_inventory			(charType *ch, char *arg, int cmd);
void do_qui					(charType *ch, char *arg, int cmd);
void do_equipment			(charType *ch, char *arg, int cmd);
void do_shout				(charType *ch, char *arg, int cmd);
void do_not_here			(charType *ch, char *arg, int cmd);
void do_tell				(charType *ch, char *arg, int cmd);
void do_send				(charType *ch, char *arg, int cmd);
void do_gtell				(charType *ch, char *arg, int cmd);
void do_wear				(charType *ch, char *arg, int cmd);
void do_wield				(charType *ch, char *arg, int cmd);
void do_grab				(charType *ch, char *arg, int cmd);
void do_remove				(charType *ch, char *arg, int cmd);
void do_put					(charType *ch, char *arg, int cmd);
void do_shutdown			(charType *ch, char *arg, int cmd);
void do_save				(charType *ch, char *arg, int cmd);
void do_hit					(charType *ch, char *arg, int cmd);
void do_string				(charType *ch, char *arg, int cmd);
void do_give				(charType *ch, char *arg, int cmd);
void do_stat				(charType *ch, char *arg, int cmd);
void do_time				(charType *ch, char *arg, int cmd);
void do_weather				(charType *ch, char *arg, int cmd);
void do_load				(charType *ch, char *arg, int cmd);
void do_purge				(charType *ch, char *arg, int cmd);
void do_shutdow				(charType *ch, char *arg, int cmd);
void do_typo				(charType *ch, char *arg, int cmd);
void do_whisper				(charType *ch, char *arg, int cmd);
void do_cast				(charType *ch, char *arg, int cmd);
void do_at					(charType *ch, char *arg, int cmd);
void do_goto				(charType *ch, char *arg, int cmd);
void do_ask					(charType *ch, char *arg, int cmd);
void do_drink				(charType *ch, char *arg, int cmd);
void do_eat					(charType *ch, char *arg, int cmd);
void do_pour				(charType *ch, char *arg, int cmd);
void do_sip					(charType *ch, char *arg, int cmd);
void do_taste				(charType *ch, char *arg, int cmd);
void do_order				(charType *ch, char *arg, int cmd);
void do_follow				(charType *ch, char *arg, int cmd);
void do_rent				(charType *ch, char *arg, int cmd);
void do_junk				(charType *ch, char *arg, int cmd);
void do_advance				(charType *ch, char *arg, int cmd);
void do_close				(charType *ch, char *arg, int cmd);
void do_open				(charType *ch, char *arg, int cmd);
void do_lock				(charType *ch, char *arg, int cmd);
void do_unlock				(charType *ch, char *arg, int cmd);
void do_exits				(charType *ch, char *arg, int cmd);
void do_enter				(charType *ch, char *arg, int cmd);
void do_leave				(charType *ch, char *arg, int cmd);
void do_flee				(charType *ch, char *arg, int cmd);
void do_sneak				(charType *ch, char *arg, int cmd);
void do_hide				(charType *ch, char *arg, int cmd);
void do_backstab			(charType *ch, char *arg, int cmd);
void do_pick				(charType *ch, char *arg, int cmd);
void do_steal				(charType *ch, char *arg, int cmd);
void do_bash				(charType *ch, char *arg, int cmd);
void do_rescue				(charType *ch, char *arg, int cmd);
void do_kick				(charType *ch, char *arg, int cmd);
void do_examine				(charType *ch, char *arg, int cmd);
void do_info				(charType *ch, char *arg, int cmd);
void do_users				(charType *ch, char *arg, int cmd);
void do_where				(charType *ch, char *arg, int cmd);
void do_levels				(charType *ch, char *arg, int cmd);
void do_reroll				(charType *ch, char *arg, int cmd);
void do_brief				(charType *ch, char *arg, int cmd);
void do_wiznet				(charType *ch, char *arg, int cmd);
void do_consider			(charType *ch, char *arg, int cmd);
void do_group				(charType *ch, char *arg, int cmd);
void do_restore				(charType *ch, char *arg, int cmd);
void do_return				(charType *ch, char *arg, int cmd);
void do_switch				(charType *ch, char *arg, int cmd);
void do_quaff				(charType *ch, char *arg, int cmd);
void do_recite				(charType *ch, char *arg, int cmd);
void do_use					(charType *ch, char *arg, int cmd);
void do_flag				(charType *ch, char *arg, int cmd);
void do_noshout				(charType *ch, char *arg, int cmd);
void do_wizhelp				(charType *ch, char *arg, int cmd);
void do_credits				(charType *ch, char *arg, int cmd);
void do_compact				(charType *ch, char *arg, int cmd);
void do_action				(charType *ch, char *arg, int cmd);
void do_practice			(charType *ch, char *arg, int cmd);
void do_flick				(charType *ch, char *arg, int cmd);
void do_wall				(charType *ch, char *arg, int cmd);
void do_set					(charType *ch, char *arg, int cmd);
void do_police				(charType *ch, char *arg, int cmd);
void do_wizlock				(charType *ch, char *arg, int cmd);
void do_noaffect			(charType *ch, char *arg, int cmd);
void do_invis				(charType *ch, char *arg, int cmd);
void do_notell				(charType *ch, char *arg, int cmd);
void do_banish				(charType *ch, char *arg, int cmd);
void do_reload				(charType *ch, char *arg, int cmd);
void do_data				(charType *ch, char *arg, int cmd);
void do_checkrent			(charType *ch, char *arg, int cmd);
void do_chat				(charType *ch, char *arg, int cmd);
void do_bank				(charType *ch, char *arg, int cmd);
void do_sys					(charType *ch, char *arg, int cmd);
void do_extractrent			(charType *ch, char *arg, int cmd);
void do_replacerent			(charType *ch, char *arg, int cmd);
void do_tornado				(charType *ch, char *arg, int cmd);
void do_light_move			(charType *ch, char *arg, int cmd);
void do_flash				(charType *ch, char *arg, int cmd);
void do_multi_kick			(charType *ch, char *arg, int cmd);
void do_demote				(charType *ch, char *arg, int cmd);
void do_nochat				(charType *ch, char *arg, int cmd);
void do_post				(charType *ch, char *arg, int cmd);
void do_sayh				(charType *ch, char *arg, int cmd);
void do_assist				(charType *ch, char *arg, int cmd);
void do_ungroup				(charType *ch, char *arg, int cmd);
void do_wizards				(charType *ch, char *arg, int cmd);
void do_hangul				(charType *ch, char *arg, int cmd);
void do_version				(charType *ch, char *arg, int cmd);
void do_lastchat			(charType *ch, char *arg, int cmd);
void do_date				(charType *ch, char *arg, int cmd);
void do_mail				(charType *ch, char *arg, int cmd);
void do_throw				(charType *ch, char *arg, int cmd);
void do_commands			(charType *ch, char *arg, int cmd);
void do_nothing				(charType *ch, char *arg, int cmd);
void do_quest				(charType *ch, char *arg, int cmd);
void do_scroll				(charType *ch, char *arg, int cmd);
void do_punch				(charType *ch, char *arg, int cmd);
void do_motd				(charType *ch, char *arg, int cmd);
void do_chain_punch			(charType *ch, char *arg, int cmd);
void do_memory				(charType *ch, char *arg, int cmd);
void do_zone				(charType *ch, char *arg, int cmd);
void do_attribute			(charType *ch, char *arg, int cmd);
void do_berserk				(charType *ch, char *arg, int cmd);
void do_morpeth				(charType *ch, char *arg, int cmd);
void do_deafen				(charType *ch, char *arg, int cmd);
void do_masquerade			(charType *ch, char *arg, int cmd);
void do_insurance			(charType *ch, char *arg, int cmd);
void do_insure				(charType *ch, char *arg, int cmd);
void do_retrieve			(charType *ch, char *arg, int cmd);
void do_saveworld			(charType *ch, char *arg, int cmd);
void do_savezone			(charType *ch, char *arg, int cmd);
void do_split				(charType *ch, char *arg, int cmd);
void do_rstat				(charType *ch, char *arg, int cmd);
void do_mstat				(charType *ch, char *arg, int cmd);
void do_istat				(charType *ch, char *arg, int cmd);
void do_pstat				(charType *ch, char *arg, int cmd);
void do_ifind				(charType *ch, char *arg, int cmd);
void do_mfind				(charType *ch, char *arg, int cmd);
void do_rfind				(charType *ch, char *arg, int cmd);
void do_ambush				(charType *ch, char *arg, int cmd);
void do_edit				(charType *ch, char *arg, int cmd);
void do_effect				(charType *ch, char *arg, int cmd);
void do_iedit				(charType *ch, char *arg, int cmd);
void do_redit				(charType *ch, char *arg, int cmd);
void do_medit				(charType *ch, char *arg, int cmd);
void do_zedit				(charType *ch, char *arg, int cmd);
void do_checklocker			(charType *ch, char *arg, int cmd);
void do_dazzle				(charType *ch, char *arg, int cmd);
void do_serpent   			(charType *ch, char *arg, int cmd);
void do_yell   				(charType *ch, char *arg, int cmd);
void do_todo   				(charType *ch, char *arg, int cmd);
void do_socials 			(charType *ch, char *arg, int cmd);
void do_track 				(charType *ch, char *arg, int cmd);
void do_peek 				(charType *ch, char *arg, int cmd);
void do_afk 				(charType *ch, char *arg, int cmd);
void do_bang 				(charType *ch, char *arg, int cmd);
void do_swirl 				(charType *ch, char *arg, int cmd);
void do_screen 				(charType *ch, char *arg, int cmd);
void do_chop				(charType *ch, char *arg, int cmd);
void do_damage				(charType *ch, char *arg, int cmd);
void do_peace				(charType *ch, char *arg, int cmd);

#endif/*__INTERPRETER_H*/
