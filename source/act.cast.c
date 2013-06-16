/** ************************************************************************
*  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
*  Usage : Interpreter of spells                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

/*#define __TRACE__
*/

#include "defines.h"
#include "world.h"
#include "mobile.h"
#include "utils.h"
#include "comm.h"
#include "limits.h"
#include "interpreter.h" 
#include "magics.h"
#include "find.h"
#include "variables.h"
#include "strings.h"
#include "specials.h"
#include "affect.h"

#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with ei. TAR_CHAR_ROOM */    
#define TAR_SELF_NONO    64 /* Only a check, use with ei. TAR_CHAR_ROOM */    
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024
#define TAR_CHAR_ZONE  2048

#define MANA_MU 1
#define MANA_CL 1

#define SPELL_LEVEL(ch, sn) (spells[(sn)].min_level[GET_CLASS(ch)-1])
#define USE_MANA(ch, sn)                            \
  MAX(spells[(sn)].min_mana, 100/(2+GET_LEVEL((ch))-SPELL_LEVEL((ch),(sn))))

char spell_argument[MAX_INPUT_LENGTH];

struct spell_info_type spells[MAX_SKILLS]=
{
/*  0*/ { 	"",	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 },
/*  1*/ { 	"armor", 			{  5,  1,IMO,IMO }, { 95, 95, 60, 30 },  5,	12,
			spell_armor,		POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},		
/*  2*/ { 	"teleport",			{  8, 18,IMO,IMO }, { 95, 95, 50, 10 }, 35, 12,
			spell_teleport,		POSITION_FIGHTING,	TAR_SELF_ONLY,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/*  3*/ { 	"bless",			{IMO,  5,IMO,IMO }, { 25, 95, 10, 25 },  5,	12,
			spell_bless,		POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/*  4*/ { 	"blindness",		{  8,  6,IMO,IMO }, { 95, 95, 50, 10 }, 15, 12,
			spell_blindness,	POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF_NONO,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/*  5*/ { 	"burning hands",    {  5,IMO,IMO,IMO }, { 95, 60, 30, 30 }, 15,	14,
			spell_burning_hands,POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/*  6*/ { 	"call lightning",	{ 20, 15,IMO,IMO }, { 60, 95, 10, 10 }, 15, 14,
			spell_call_lightning,POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 1
		},
/*  7*/ { 	"charm person",		{ 14, 26,IMO,IMO }, { 95, 60, 25, 20 }, 30,	12,
			spell_charm_person,	POSITION_STANDING,	TAR_CHAR_ROOM,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 0
		},
/*  8*/ { 	"chill touch",		{  3,IMO,IMO,IMO }, { 95, 60, 30, 30 }, 15, 14,
			spell_chill_touch,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/*  9*/ { 	"reanimate",		{  6, 12,IMO,IMO }, { 95, 95, 20, 10 }, 10, 12,
			spell_reanimate,	POSITION_STANDING,	TAR_OBJ_ROOM,
			SPELL_SPELL | SPELL_WAND, 0
		},
/* 10*/ { 	"colour spray",		{ 11,IMO,IMO,IMO }, { 95, 60, 30, 30 }, 15, 14,
			spell_colour_spray,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/* 11*/ { 	"relocate",			{ 20, 25,IMO,IMO }, { 90, 80, 60, 40 }, 50,	12,
			spell_relocate,		POSITION_STANDING,	TAR_CHAR_ZONE,
			SPELL_SPELL, 0
		},
/* 12*/ { 	"create food",		{ 10,  3,IMO,IMO }, { 40, 95, 25, 10 },  5,	12,
			spell_create_food,	POSITION_STANDING,	TAR_IGNORE,
			SPELL_SPELL, 0
		},
/* 13*/ { 	"create water",		{ 10,  2,IMO,IMO }, { 40, 95, 25, 10 },  5,	12,
			spell_create_water,	POSITION_STANDING,	TAR_OBJ_INV | TAR_OBJ_EQUIP,
			SPELL_SPELL, 0
		},
/* 14*/ { 	"cure blind",		{ 20,  4,IMO,IMO }, { 50, 95, 10, 10 },  5, 12,
			spell_cure_blind,	POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 0
		},
/* 15*/ { 	"cure critic",		{ 16,  9,IMO,IMO }, { 60, 95, 50, 60 }, 15,	12,
			spell_cure_critic,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 16*/ { 	"cure light",		{  4,  1,IMO,IMO }, { 50, 95, 45, 45 }, 15,	12,
			spell_cure_light,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 17*/ { 	"curse",			{ 12, 20,IMO,IMO }, { 95, 30, 50, 10 }, 20, 12,
			spell_curse,		POSITION_STANDING,	TAR_CHAR_ROOM,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 18*/ { 	"detect evil",		{ 20,  4,IMO,IMO }, { 50, 95, 75, 25 },  5,	12,
			spell_detect_evil,	POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 19*/ { 	"detect invisibility",{2,  5,IMO,IMO }, { 95, 95, 60, 10 },  5, 12,
			spell_detect_invisibility,	POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 20*/ { 	"recharger",		{  9, 11,IMO,IMO }, { 95, 95, 50, 20 }, 50,	12,
			spell_recharger,	POSITION_STANDING,	TAR_OBJ_INV | TAR_OBJ_EQUIP,
			SPELL_SPELL | SPELL_SCROLL | SPELL_WAND, 0
		},
/* 21*/ { 	"preach",			{IMO, 30,IMO,IMO }, { 60, 95, 40, 30 }, 50,	12,
			spell_preach,		POSITION_STANDING,	TAR_CHAR_ROOM,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 0
		},
/* 22*/ { 	"dispel evil",		{ 25, 10,IMO,IMO }, { 30, 95, 10, 30 }, 15, 14,
			spell_dispel_evil,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 1
		},
/* 23*/ { 	"earthquake",		{ 15,  7,IMO,IMO }, { 80, 95, 25, 50 }, 15,	14,
			spell_earthquake,	POSITION_FIGHTING,	TAR_IGNORE,
			SPELL_SPELL | SPELL_POTION | SPELL_STAFF, 1
		},
/* 24*/ { 	"enchant weapon",	{ 12,IMO,IMO,IMO }, { 90, 50, 30, 20 }, 20,	12,
			spell_enchant_weapon,POSITION_STANDING,	TAR_OBJ_INV | TAR_OBJ_EQUIP,
			SPELL_SPELL | SPELL_SCROLL | SPELL_WAND, 0
		},
/* 25*/ { 	"energy drain",		{ 13, 20,IMO,IMO }, { 95, 40, 65, 40 }, 35,	 4,
			spell_energy_drain,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/* 26*/ { 	"fireball",			{ 15,IMO,IMO,IMO }, { 95, 60, 30, 30 }, 15,	14,
			spell_fireball,		POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/* 27*/ { 	"harm",				{IMO, 14,IMO,IMO }, { 80, 95, 10, 10 }, 15,	14,
			spell_harm,			POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 1
		},
/* 28*/ { 	"heal",				{ 24, 14,IMO,IMO }, { 85, 95, 30, 30 }, 50,	14,
			spell_heal,			POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 29*/ { 	"invisibility",		{  4, 20,IMO,IMO }, { 95, 50, 60, 10 },  5,	12,
			spell_invisibility,	POSITION_STANDING,
								TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 30*/ { 	"lightning bolt",	{  9, 13,IMO,IMO }, { 95, 60, 30, 30 }, 15,	14,
			spell_lightning_bolt,POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/* 31*/ { 	"locate object",	{  6, 5,IMO,IMO }, { 95, 95, 60, 10 }, 20,	12,
			spell_locate_object,POSITION_STANDING,	TAR_OBJ_WORLD,
			SPELL_SPELL, 0
		},
/* 32*/ { 	"magic missile",	{  1,IMO,IMO,IMO }, { 95, 60, 30, 30 }, 15,	14,
			spell_magic_missile,POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/* 33*/ { 	"poison",			{ 20,  8,IMO,IMO }, { 60, 95, 60, 10 }, 20,	12,
			spell_poison,		POSITION_STANDING, 
									TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 34*/ { 	"protection from evil",{20, 6,IMO,IMO}, { 60, 95, 10, 10 },  5, 14,
			spell_protection_from_evil,POSITION_STANDING, 	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 35*/ { 	"remove curse",		{IMO, 12,IMO,IMO }, { 60, 95, 10, 10 },  5, 12,
			spell_remove_curse,	POSITION_STANDING,
							TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 36*/ { 	"sanctuary",		{ 23, 13,IMO,IMO }, { 45, 95, 10, 10 }, 33, 12,
			spell_sanctuary,	POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 37*/ { 	"shocking grasp",	{  7,IMO,IMO,IMO }, { 95, 60, 30, 30 }, 15,	14,
			spell_shocking_grasp,POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/* 38*/ { 	"sleep",			{ 14, 20,IMO,IMO }, { 95, 80, 50, 10 }, 20,	12,
			spell_sleep,		POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF_NONO,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 39*/ { 	"strength",			{  7, 20,IMO,IMO }, { 95, 50, 20, 40 }, 20,	12,
			spell_strength,		POSITION_STANDING,	TAR_SELF_ONLY,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND, 0
		},
/* 40*/ { 	"summon",			{ 20,  8,IMO,IMO }, { 60, 95, 10, 10 }, 50, 12,
			spell_summon,		POSITION_STANDING,	TAR_CHAR_WORLD,
			SPELL_SPELL, 0
		},
/* 41*/ { 	"ventriloquate",	{  1, 20,IMO,IMO }, { 95, 95, 95, 95 },  5,	12,
			spell_ventriloquate,POSITION_STANDING,	TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO,
			SPELL_SPELL, 0
		},
/* 42*/ { 	"word of recall",	{ 20, 11,IMO,IMO }, { 50, 95, 50, 10 },  5,	10,
			spell_word_of_recall,POSITION_STANDING, 	TAR_SELF_ONLY,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 43*/ { 	"remove poison",	{ IMO, 9,IMO,IMO }, { 50, 95, 40, 30 },  5,	12,
			spell_remove_poison,POSITION_STANDING, 	
												TAR_CHAR_ROOM | TAR_SELF | TAR_OBJ_INV | TAR_OBJ_ROOM,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 44*/ { 	"sense life",		{ 15,  7,IMO,IMO }, { 50, 95, 40, 30 },  5,	12,
			spell_sense_life,	POSITION_STANDING, 	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 45*/ { 	"sunburst",			{ 25, 20,IMO,IMO }, { 85, 95, 25, 25 }, 15,	14,
			spell_sunburst,		POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/* 46*/ { 	"clone",			{ 30,IMO,IMO,IMO }, { 95, 75, 75, 10 }, 30, 12,
			spell_clone,		POSITION_STANDING,	TAR_CHAR_ROOM,
			SPELL_SPELL | SPELL_SCROLL | SPELL_WAND, 0
		},
/* 47*/ { 	"morpeth",			{IMO,IMO, 18,IMO }, { 30, 30, 80, 55 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 48*/ { 	"deafen",			{IMO,IMO,IMO, 23 }, { 30, 30, 40, 95 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 49*/ { 	"masquerade",		{IMO,IMO, 30,IMO }, { 30, 30, 95, 60 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 50*/ { 	"ambush",			{IMO,IMO, 15,IMO }, { 30, 30, 95, 30 }, 0,  1, 
			0, 					0, 0, 				0, 0 
		},
/* 51*/ { 	"dazzle blast",		{IMO,IMO,IMO, 40 }, { 30, 30, 95, 80 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 52*/ { 	"serpent stab",		{IMO,IMO, 35,IMO }, { 30, 30, 95, 60 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 53*/ { 	"identify",			{ 17, 15,IMO,IMO }, { 95, 50,  5,  5 }, 25, 12,
			spell_identify,		POSITION_STANDING,	
								TAR_CHAR_ROOM | TAR_SELF | TAR_OBJ_INV | TAR_OBJ_EQUIP,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND, 0
		},
/* 54*/ { 	"fear",				{ 20, 15,IMO,IMO }, { 95, 80,  5,  5 }, 15, 12,
			spell_fear,			POSITION_FIGHTING, TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND, 0
		},
/* 55*/ { 	"sneak",			{IMO,IMO,  1,IMO }, { 30,  1, 95, 20 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 56*/ { 	"hide",				{IMO,IMO,  1,IMO }, { 30,  1, 95, 20 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 57*/ { 	"steal",			{IMO,IMO,  1,IMO }, { 30,  1, 95, 20 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 58*/ { 	"backstab",			{IMO,IMO,  1,IMO }, { 30,  1, 95, 30 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 59*/ { 	"pick",				{IMO,IMO,  1,IMO }, { 30,  1, 95, 20 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 60*/ { 	"kick",				{IMO,IMO,  1,  1 }, { 30, 50, 70, 90 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 61*/ { 	"bash",				{IMO,IMO,  1,  1 }, { 30, 10, 30, 90 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 62*/ { 	"rescue",			{  1,  1,  1,  1 }, { 30, 40, 30, 95 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 63*/ { 	"punch",			{  1,  1,  1,  1 }, { 20, 40, 70, 90 },10,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 64*/ { 	"chain punch",		{IMO,IMO,IMO, 25 }, { 30, 50, 60, 95 },40,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 65*/ { 	"berserk",			{ 30, 25, 20, 10 }, { 20, 35, 40, 85 }, 0,  1, 
			0, 					0, 	0 , 			0, 0
		},
/* 66*/ { 	"",	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 }, /* fire breath */
/* 67*/ { 	"",	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 }, /* gas */
/* 68*/ { 	"",	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 }, /* frost */
/* 69*/ { 	"",	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 }, /* acid */
/* 70*/ { 	"",	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 }, /* lightning */

/* 71*/ { 	"damage up",		{  9, 23,IMO,IMO }, { 95, 25, 20, 15 }, 25,	12,
			spell_damage_up,	POSITION_STANDING,	TAR_SELF_ONLY,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 72*/ { 	"energy flow",		{  4,IMO,IMO,IMO }, { 95, 30, 30, 30 },  3,  6,
			spell_energyflow,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/* 73*/ { 	"mana boost",		{ 15, 22,IMO,IMO }, { 95, 30, 10, 20 }, 10, 12,
			spell_mana_boost,	POSITION_STANDING,	TAR_SELF_ONLY,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 74*/ { 	"vitalize",			{ 17,IMO,IMO,IMO }, { 95, 40, 35, 30 }, 25,	12,
			spell_vitalize,POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 75*/ { 	"full fire",		{ 17,IMO,IMO,IMO }, { 95, 30, 30, 30 },  1,	40,
			spell_full_fire,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND, 1
		},
/* 76*/ { 	"throw",			{ 22,IMO,IMO,IMO }, { 95, 37, 37, 37 }, 15, 14,
			spell_throw,		POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND, 1
		},
/* 77*/ { 	"firestorm",		{ 23, 18,IMO,IMO }, { 90, 95, 50, 50 }, 30, 14,
			spell_fire_storm,	POSITION_FIGHTING,	TAR_IGNORE,
			SPELL_SPELL | SPELL_STAFF, 1
		},
/* 78*/ { 	"crush armor",		{ 19, 28,IMO,IMO }, { 95, 35, 25, 45 },  5,	12,
			spell_crush_armor,	POSITION_STANDING,	TAR_CHAR_ROOM,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 79*/ { 	"full heal",		{IMO, 21,IMO,IMO }, { 40, 95, 30, 30 }, 70,	14,
			spell_full_heal,	POSITION_FIGHTING,	TAR_CHAR_ROOM,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 80*/ { 	"trick",			{ 25,IMO,IMO,IMO }, { 95, 20, 20, 20 }, 50,	12,
			spell_trick,		POSITION_STANDING,	TAR_CHAR_ROOM,
			SPELL_SPELL | SPELL_WAND, 0
		},
/* 81*/ { 	"haste",			{ 17, 23,IMO,IMO }, { 95, 95, 35, 20 }, 30,	12,
			spell_haste,		POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 82*/ { 	"improved haste",	{ 25, 27,IMO,IMO }, { 95, 80, 35, 20 }, 40,	12,
			spell_improved_haste,POSITION_STANDING,	TAR_SELF_ONLY,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 83*/ { 	"far look",			{  7,  5,IMO,IMO }, { 95, 95, 35, 20 }, 20, 12,
			spell_far_look,		POSITION_FIGHTING,	TAR_CHAR_WORLD,
			SPELL_SPELL, 0
		},
/* 84*/ { 	"all heal",			{IMO, 30,IMO,IMO }, { 80, 95, 25, 50 }, 85, 14,
			spell_all_heal,		POSITION_FIGHTING,	TAR_IGNORE,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_STAFF, 0
		},
/* 85*/ { 	"tornado",			{IMO,IMO,  1,IMO }, {  0,  0, 95, 80 },  0,  0, 
			0, 					0, 	0 ,	 0,  0
		},
/* 86*/ { 	"lightning move",	{IMO,IMO,  1,IMO }, {  0,  0, 95,  0 }, 10,  0, 
			0, 					0, 	0 ,	 0,  0
		},
/* 87*/ { 	"",					{IMO,IMO,  0,  0 }, {  0,  0, 95, 50 },  0,  0,  /* parry */
			0, 					0, 	0 ,	 0,  0
		},
/* 88*/ { 	"flash",			{IMO,IMO,  5,IMO }, {  0,  0, 95,  0 },  0,  0, 
			0, 					0,  0 ,	 0,  0
		},
/* 89*/ { 	"multi kick",		{IMO,IMO,IMO, 17 }, {  0,  0,  0, 90 },  0,  0, 
			0, 					0, 	0 ,	 0,  0
		},
/* 90*/ { 	"enchant armor",	{ 30,IMO,IMO,IMO }, { 90, 50, 30, 20 },100, 12,
			spell_enchant_armor,POSITION_STANDING,	TAR_OBJ_INV | TAR_OBJ_EQUIP,
			SPELL_SPELL | SPELL_SCROLL | SPELL_WAND, 0
		},
/* 91*/ { 	"pray for armor",	{IMO, 34,IMO,IMO }, { 90, 90, 30, 20 },100,	12,
			spell_pray_for_armor,POSITION_STANDING,	TAR_OBJ_INV | TAR_OBJ_EQUIP,
			SPELL_SPELL | SPELL_SCROLL | SPELL_WAND, 0
		},
/* 92*/ { 	"infravision",		{  5,  7,IMO,IMO }, { 95, 95, 10, 10 },  5,	12,
			spell_infravision,	POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 93*/ { 	"create nectar",	{IMO, 25,IMO,IMO }, { 40, 95, 25, 10 }, 50,	12,
			spell_create_nectar,POSITION_STANDING,	TAR_OBJ_INV | TAR_OBJ_EQUIP,
			SPELL_SPELL | SPELL_WAND, 0
		},
/* 94*/ { 	"create golden nectar",{IMO,33,IMO,IMO }, {40,95, 25, 10 }, 70,	12,
			spell_create_golden_nectar,POSITION_STANDING,	TAR_OBJ_INV | TAR_OBJ_EQUIP,
			SPELL_SPELL | SPELL_WAND, 0
		},
/* 95*/ { 	"cone of ice",		{ 30,IMO,IMO,IMO }, { 95, 30, 30, 30 }, 20,	14,
			spell_corn_of_ice,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/* 96*/ { 	"disintergrate",	{ 33,IMO,IMO,IMO }, { 95, 30, 30, 30 }, 50, 14,
			spell_disintergrate,POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_WAND | SPELL_STAFF, 1
		},
/* 97*/ { 	"phase",			{ 20, 24,IMO,IMO }, { 95, 95, 30, 30 }, 10,	12,
			spell_phase,		POSITION_STANDING,	TAR_IGNORE,
			SPELL_SPELL, 0
		},
/* 98*/ { 	"mirror image",		{ 35,IMO,IMO,IMO }, { 95, 95, 35, 20 }, 70,	12,
			spell_mirror_image,	POSITION_STANDING,	TAR_SELF_ONLY,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/* 99*/ { 	"hand of god",		{IMO, 35,IMO,IMO }, { 90, 95, 50, 50 }, 33,	14,
			spell_hand_of_god,	POSITION_FIGHTING,	TAR_IGNORE,
			SPELL_SPELL | SPELL_STAFF, 1
		},
/*100*/ { 	"cause light",		{  4,  1,IMO,IMO }, { 50, 95, 45, 45 }, 15,	14,
			spell_cause_light,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 1 		
		},
/*101*/ { 	"cause critic",		{ 16,  9,IMO,IMO }, { 60, 95, 45, 50 }, 15,	14,
			spell_cause_critic,	POSITION_FIGHTING,	TAR_CHAR_ROOM | TAR_FIGHT_VICT,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 1
		},
/*102*/ { 	"time stop",		{IMO, 38,IMO,IMO }, { 70, 85, 45, 50 },150,	20,
			spell_time_stop,	POSITION_FIGHTING,	TAR_IGNORE,
			SPELL_SPELL | SPELL_STAFF, 							  				  1
		},
/*103*/ { 	"sense corpse",		{ 32,IMO,IMO,IMO }, { 95, 70, 45, 50 },100,	10,
			spell_sense_sprit,	POSITION_STANDING,	TAR_CHAR_ROOM,
			SPELL_SPELL | SPELL_WAND, 							  				  0
		},
/*104*/ { 	"sanctuary shield",	{IMO, 31,IMO,IMO }, { 30, 80, 25, 25 },150,	40,
			spell_sanctuary_shield,	POSITION_STANDING,	TAR_IGNORE,
			SPELL_SPELL | SPELL_STAFF, 							  				  0
		},
/*i05*/ { 	"detect good",		{ 20,  4,IMO,IMO }, { 50, 95, 75, 25 },  5,	12,
			spell_detect_good,	POSITION_STANDING,	TAR_CHAR_ROOM | TAR_SELF,
			SPELL_SPELL | SPELL_SCROLL | SPELL_POTION | SPELL_WAND | SPELL_STAFF, 0
		},
/*i06*/ { 	"portal",			{IMO, 32,IMO,IMO }, { 50, 95, 75, 25 }, 50,	12,
			spell_portal,		POSITION_STANDING,	TAR_CHAR_WORLD,
			SPELL_SPELL, 0
		},
/*107*/ { 	"",	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 },
/*108*/ { 	"",	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 },
/*109*/ { 	"",	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 },
/*110*/ { 	"track",			{IMO,IMO, 15,IMO }, { 30, 30, 95, 60 }, 0,  1, 
			0, 					0, 	0, 				0, 0
		},
/*111*/ { 	"peek",				{IMO,IMO,  1,IMO }, { 30,  1, 95, 20 }, 0,  1, 
			0, 					0, 	0, 				0, 0
		},
/*112*/ { 	"swirl blade",		{IMO,IMO,IMO, 10 }, { 30,  1, 40, 95 }, 0,  1, 
			0, 					0, 	0, 				0, 0
		},
/*113*/ { 	"bang the earth",	{IMO,IMO,IMO, 35 }, { 30,  1, 40, 90 }, 0,  1, 
			0, 					0, 	0, 				0, 0
		},
/*114*/ { 	"\n",	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0 },
};

int spell_lookup( char * name )
{
	int		i;

	for( i = 0; spells[i].name[0] != '\n' ; i++ )
	{
		if( spells[i].name[0] && isprefix( name, spells[i].name ) ) return i;
	}

	return -1;
}

static void say_spell( charType *ch, int si )
{
 	char 				buf[MAX_STRING_LENGTH]; 
 	char				to_other[MAX_STRING_LENGTH];
 	char				to_same [MAX_STRING_LENGTH];
 	char			*	cp;	
 	int 				i, len;
 	charType *	char_in_room;

	struct syllable 
	{
  		char org[10];
  		char new[10];
	};

	static struct syllable syls[] = 
	{
		{ " ", " " },
		{ "ar", "abra"   },
		{ "au", "kada"    },
		{ "bless", "fido" },
		{ "blind", "nose" },
		{ "bur", "mosa" },
		{ "cu", "judi" },
		{ "de", "oculo"},
		{ "en", "unso" },
		{ "light", "dies" },
		{ "lo", "hi" },
		{ "mor", "zak" },
		{ "move", "sido" },
		{ "ness", "lacri" },
		{ "ning", "illa" },
		{ "per", "duda" },
		{ "ra", "gru"   },
		{ "re", "candus" },
		{ "son", "sabru" },
		{ "tect", "infra" },
		{ "tri", "cula" },
		{ "ven", "nofo" },
 		{"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
		{"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
		{"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
		{"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
 	};

 	for( buf[0] = '\0', len = 0, cp = spells[si].name; *cp; cp += len ) 
 	{
  		for( i = 0; *(syls[i].org); i++ )
   			if( len = isprefix( syls[i].org, cp ), len ) 
   			{
    			strcat( buf, syls[i].new );
				break;
			}
		len = len ? len : 1;
 	}

 	sprintf(to_other,"$n utters the words, '%s'", buf);
 	sprintf(to_same, "$n utters the words, '%s'", spells[si].name);

 	for( char_in_room = world[ch->in_room].people; char_in_room; 
 									char_in_room = char_in_room->next_in_room)
  		if( char_in_room != ch ) 
  		{
   			if( GET_CLASS(ch) == GET_CLASS(char_in_room) )
    			act(to_same,  FALSE, ch, 0, char_in_room, TO_VICT);
   			else
    			act(to_other, FALSE, ch, 0, char_in_room, TO_VICT);
  		}
}

void do_cast(charType *ch, char *argument, int cmd)
{
 	objectType 		*	tar_obj;
 	charType 		*	tar_char;
 	static char 				name[MAX_INPUT_LENGTH];
 	static char 				spellname[MAX_INPUT_LENGTH];
 	char					*	qe, * qs, * cp;
 	int							spl; 
 	int							fail_rate, learn_rate;
 	int							i;
 	int							wasspace;
 	int							target_ok;

 	if( IS_NPC(ch) ) return;

 	if( GET_LEVEL(ch) >= IMO && GET_LEVEL(ch) < IMO+3) return;

 	qs = skipsps(argument);

 	if( !(*qs) ) 
 	{
  		sendf( ch, "Cast which what where?" );	return;
 	}

 	if( *qs != '\'' ) 
 	{
  		sendf( ch, "Magic must always be enclosed by the magic symbols: cast 'magic'" );
  		return;
 	}

 	for( cp = spellname, qe = ++qs, wasspace = 1; *qe && (*qe != '\'') ; qe++ )
 	{
 		if( *qe == ' ' && !wasspace )
 		{
 			*cp++ = tolower( *qe );
 			wasspace = 1;
 		}
 		else if( !isspace(*qe) ) 
 		{
 			*cp++ = tolower( *qe );
 			wasspace = 0;
 		}
 		else wasspace = 1;
	}
	*cp = '\0';	

 	if( *qe != '\'') 
 	{
  		sendf( ch, "Magic must always be enclosed by the magic symbols: cast 'magic'" );
  		return;
 	}


	TRACE( trace("do_cast> %s => [%s] -> [%s]", GET_NAME(ch), argument, spellname) );

 	spl = spell_lookup( spellname );

 	if( !spl ) 
 	{
  		sendf( ch, "Your lips do not move, no magic appears." ); 		return;
 	}


 	if( spl < 0 || spl > MAX_SKILLS || spells[spl].name[0] == 0 || spells[spl].type == 0 )
 	{
 		sendf( ch, "Eh??"); return;
 	}

  	if( GET_POS(ch) < spells[spl].min_position ) 
  	{
   		switch(GET_POS(ch)) 
   		{
    		case POSITION_SLEEPING :
     			sendf( ch, "You dream about great magical powers." ); 		return;
    		case POSITION_RESTING :
     			sendf( ch, "You can't concentrate enough while resting." ); return;
    		case POSITION_SITTING :
     			sendf( ch, "You can't do this sitting!" );					return;
    		case POSITION_FIGHTING :
     			sendf( ch, "Impossible! You can't concentrate enough!." );  return;
    		default:
     			sendf( ch, "It seems like you're in a pretty bad shape!" ); return;
   		}
   	}

   	if(spells[spl].min_level[GET_CLASS(ch)-1] > GET_LEVEL(ch))
   	{
  		sendf( ch, "Your lips do not move, no magic appears." ); 		return;
   	}

	if( (spl == SPELL_PHASE || spl == SPELL_RELOCATE) && (world[ch->in_room].func == level_gate ))
	{
	    sendf( ch, "You cannot do that here." );
		act("$n attempts to misbehave here.", FALSE, ch, 0, ch, TO_NOTVICT);
        return;
	}		

	if( spells[spl].aggressive )
	{
		if( (world[ch->in_room].func == safe_house) )
		{
			misbehave( ch );
			return;
		}

		for( tar_char = world[ch->in_room].people; tar_char; 
			 tar_char = tar_char->next_in_room )
		{
			if( IS_AFFECTED( tar_char, AFF_TIME_STOP ) )
			{
				misbehave( ch );
				return;
			}
		}
	}

   	target_ok = FALSE;
   	tar_char  = 0;
   	tar_obj   = 0;

   	oneArgument( ++qe, name );

	if( IS_SET(spells[spl].targets, TAR_IGNORE)) 
 	{
    	target_ok = TRUE; /* No target, is a good target */
   	}
 	else 
   	{
    	if( *name ) 
		{
     		if( IS_SET(spells[spl].targets, TAR_CHAR_ROOM))
      			if((tar_char = find_char_room(ch, name)))
       				target_ok = TRUE;

     		if( !target_ok && IS_SET(spells[spl].targets, TAR_CHAR_WORLD))
      			if((tar_char = find_char( ch, name )))
       				target_ok = TRUE;

     		if( !target_ok && IS_SET(spells[spl].targets, TAR_CHAR_ZONE))
	 		{ 
      			if(IS_SET(world[ch->in_room].flags,EVERYZONE))
	  			{
        			if((tar_char = find_char(ch, name)))
         			target_ok = TRUE;
      			}
	  			else
				{
       				if((tar_char = find_char_zone(ch, name)))
        				target_ok = TRUE;
      			}
     		}

     		if( !target_ok && IS_SET(spells[spl].targets, TAR_OBJ_INV) )
      			if( (tar_obj = find_obj_inven(ch, name)) )
       				target_ok = TRUE;

     		if( !target_ok && IS_SET(spells[spl].targets, TAR_OBJ_EQUIP) ) 
	 		{
      			for( i = 0; i < MAX_WEAR && !target_ok; i++ )
       				if( ch->equipment[i] && (stricmp(name, ch->equipment[i]->name) == 0)) 
	   				{
        				tar_obj = ch->equipment[i];
        				target_ok = TRUE;
       				}
     		}

     		if( !target_ok && IS_SET(spells[spl].targets, TAR_OBJ_ROOM) )
      			if( (tar_obj = find_obj_room( ch, name )) )
       				target_ok = TRUE;

     		if( !target_ok && IS_SET(spells[spl].targets, TAR_OBJ_WORLD))
      			if( (tar_obj = find_obj( ch, name )) )
       				target_ok = TRUE;

     		if( !target_ok && IS_SET(spells[spl].targets, (TAR_SELF_ONLY | TAR_SELF)) )
	 		{
      			if (stricmp(GET_NAME(ch), name) == 0) 
	  			{
       				tar_char = ch;
       				target_ok = TRUE;
      			}
    		} 
		}
		else	/* No name */ 
		{
     		if( IS_SET(spells[spl].targets, TAR_FIGHT_SELF)) 
	 			if( ch->fight )
				{
       				tar_char = ch;
       				target_ok = TRUE;
      			}
     		if( !target_ok && IS_SET(spells[spl].targets, TAR_FIGHT_VICT) )
      			if( ch->fight ) 
	  			{
					/* WARNING, MAKE INTO POINTER */
					tar_char = ch->fight;
					target_ok = TRUE;
      			}
			if( !target_ok && IS_SET(spells[spl].targets, (TAR_SELF_ONLY | TAR_SELF)) )
	 		{
       				tar_char = ch;
	   				target_ok = TRUE;
			}
		}
	}

   	if( !target_ok ) 
   	{
    	if( *name )	send_to_char("Say what?\n\r", ch);
		else 
		{ 
     		if( spells[spl].targets < TAR_OBJ_INV )
      			sendf( ch, "Who should the spell be cast upon?" );
     		else
      			sendf( ch, "What should the spell be cast upon?" );
	  	}
		return;
   	} 

   	if(GET_LEVEL(ch) < (IMO+3))
   	{
    	if((tar_char == ch) && IS_SET(spells[spl].targets, TAR_SELF_NONO)) 
		{
     		send_to_char("You can not cast this spell upon yourself.\n\r", ch);
     		return;
    	}
    	else if( (tar_char != ch)&&IS_SET(spells[spl].targets,TAR_SELF_ONLY) ) 
		{
     		send_to_char("You can only cast this spell upon yourself.\n\r", ch);
     		return;
    	} 
		else if( IS_AFFECTED(ch, AFF_CHARM) 
              && (spells[spl].aggressive) 
			  && (ch->master == tar_char) ) 
		{
     		sendf( ch, "You are afraid that it could harm your master.", ch);
     		return;
    	}
	}

	if( (spells[spl].func == 0) && spl > 0 )
		sendf( ch, "Sorry, this magic has not yet been implemented :(" );
	else 
	{
		if( (GET_LEVEL(ch) < IMO) && (GET_MANA(ch) < USE_MANA(ch, spl)) ) 
		{
			sendf( ch, "You can't summon enough energy to cast the spell." );
			return;
		}

		fail_rate = number(1, 101) ;	/* 101% is complete failure */
		learn_rate = ch->skills[spl] ;

		if( spl != SPELL_VENTRILOQUATE ) say_spell(ch, spl);

		if( fail_rate > learn_rate ) 
		{
			sendf(ch, "You lost your concentration!" );
			GET_MANA(ch) -= ( USE_MANA(ch, spl)>>1 );
			WAIT_STATE(ch, spells[spl].beats/2);
		}
		else
		{
			(*spells[spl].func)
				(GET_LEVEL(ch), ch, name, SPELL_SPELL, tar_char, tar_obj);
			GET_MANA(ch) -= (USE_MANA(ch, spl));
			WAIT_STATE(ch, spells[spl].beats);
			sendf( ch, "Ok." );
		}
	}
}
