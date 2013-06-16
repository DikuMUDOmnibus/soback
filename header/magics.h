#ifndef __MAGICS_H
#define __MAGICS_H

#ifndef __CHARACTER_H
#include "character.h"
#endif

#ifndef __OBJECT_H
#include "object.h"
#endif

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO */
#define SPELL_ARMOR                   1
#define SPELL_TELEPORT                2
#define SPELL_BLESS                   3
#define SPELL_BLINDNESS               4
#define SPELL_BURNING_HANDS           5
#define SPELL_CALL_LIGHTNING          6
#define SPELL_CHARM_PERSON            7
#define SPELL_CHILL_TOUCH             8
#define SPELL_REANIMATE               9
#define SPELL_COLOUR_SPRAY           10 
#define SPELL_RELOCATE               11 
#define SPELL_CREATE_FOOD            12 
#define SPELL_CREATE_WATER           13 
#define SPELL_CURE_BLIND             14 
#define SPELL_CURE_CRITIC            15 
#define SPELL_CURE_LIGHT             16 
#define SPELL_CURSE                  17 
#define SPELL_DETECT_EVIL            18 
#define SPELL_DETECT_INVISIBLE       19 
#define SPELL_RECHARGER              20 
#define SPELL_PREACH                 21 
#define SPELL_DISPEL_EVIL            22 
#define SPELL_EARTHQUAKE             23 
#define SPELL_ENCHANT_WEAPON         24 
#define SPELL_ENERGY_DRAIN           25 
#define SPELL_FIREBALL               26 
#define SPELL_HARM                   27 
#define SPELL_HEAL                   28 
#define SPELL_INVISIBLE              29 
#define SPELL_LIGHTNING_BOLT         30 
#define SPELL_LOCATE_OBJECT          31 
#define SPELL_MAGIC_MISSILE          32 
#define SPELL_POISON                 33 
#define SPELL_PROTECT_FROM_EVIL      34 
#define SPELL_REMOVE_CURSE           35 
#define SPELL_SANCTUARY              36 
#define SPELL_SHOCKING_GRASP         37 
#define SPELL_SLEEP                  38 
#define SPELL_STRENGTH               39 
#define SPELL_SUMMON                 40 
#define SPELL_VENTRILOQUATE          41 
#define SPELL_WORD_OF_RECALL         42
#define SPELL_REMOVE_POISON          43
#define SPELL_SENSE_LIFE             44
#define SPELL_SUNBURST               45
#define SPELL_CLONE                  46
#define SKILL_MORPETH				 47
#define SKILL_DEAFEN				 48
#define SKILL_MASQUERADE			 49
#define SKILL_AMBUSH				 50
#define SKILL_DAZZLE				 51
#define SKILL_SERPENT				 52

#define SPELL_IDENTIFY               53
#define SPELL_FEAR					 54
#define SKILL_SNEAK                  55
#define SKILL_HIDE                   56
#define SKILL_STEAL                  57
#define SKILL_BACKSTAB               58
#define SKILL_PICK_LOCK              59

#define SKILL_KICK                   60
#define SKILL_BASH                   61
#define SKILL_RESCUE                 62

#define SKILL_PUNCH                  63
#define SKILL_CHAIN_PUNCH            64
#define SKILL_BERSERK		         65

#define SPELL_FIRE_BREATH          	 66
#define SPELL_GAS_BREATH             67
#define SPELL_FROST_BREATH           68
#define SPELL_ACID_BREATH            69
#define SPELL_LIGHTNING_BREATH       70

#define SPELL_DAMAGE_UP              71
#define SPELL_ENERGY_FLOW            72
#define SPELL_MANA_BOOST             73
#define SPELL_VITALIZE               74
#define SPELL_FULL_FIRE              75
#define SPELL_THROW                  76
#define SPELL_FIRE_STORM             77
#define SPELL_CRUSH_ARMOR            78
#define SPELL_FULL_HEAL		     	 79
#define SPELL_TRICK                  80
#define SPELL_HASTE                  81
#define SPELL_IMPROVED_HASTE         82
#define SPELL_FAR_LOOK               83
#define SPELL_ALL_HEAL               84

#define SKILL_TORNADO                85
#define SKILL_LIGHT_MOVE             86
#define SKILL_PARRY                  87
#define SKILL_FLASH	             	 88
#define SKILL_MULTI_KICK             89

#define SPELL_ENCHANT_ARMOR          90
#define SPELL_PRAY_FOR_ARMOR         91
#define SPELL_INFRAVISION            92
#define SPELL_CREATE_NECTAR          93
#define SPELL_CREATE_GOLDEN_NECTAR   94
#define SPELL_CORN_OF_ICE            95
#define SPELL_DISINTERGRATE          96
#define SPELL_PHASE                  97
#define SPELL_MIRROR_IMAGE           98
#define SPELL_HAND_OF_GOD            99
#define SPELL_CAUSE_LIGHT           100
#define SPELL_CAUSE_CRITIC          101
#define SPELL_TIME_STOP             102
#define SPELL_SENSE_SPRIT           103
#define SPELL_SANCTUARY_SHIELD      104
#define SPELL_DETECT_GOOD			105
#define SPELL_PORTAL				106

#define SKILL_TRACK					110
#define SKILL_PEEK					111
#define SKILL_SWIRL					112
#define SKILL_BANG					113

#define MAX_USED_SPELLS				114
#define MAX_SKILLS					150

#define ROUND_SKILL_TYPE			512
#define ROUND_SPELL_TYPE			512

#define SECOND_SPELL_TYPE		   1024	
#define SECOND_SKILL_TYPE		   1024	
/*
 * Attack type by weapon 
 */

#define TYPE_HIT                    150
#define TYPE_BLUDGEON               151
#define TYPE_PIERCE                 152
#define TYPE_SLASH                  153
#define TYPE_WHIP                   154 /* EXAMPLE */
#define TYPE_SHOOT                  155  /* NO MESSAGES WRITTEN YET! */
#define TYPE_BITE                   156  /* NO MESSAGES WRITTEN YET! */
#define TYPE_STING                  157  /* NO MESSAGES WRITTEN YET! */
#define TYPE_CRUSH                  158  /* NO MESSAGES WRITTEN YET! */

/*
 * Stunned and waiting death.
 */

#define TYPE_SUFFERING              200


/* 
 * More anything but spells and weapontypes can be insterted here! 
 */

#define SPELL_SPELL   1
#define SPELL_POTION  2
#define SPELL_WAND    4
#define SPELL_STAFF   8
#define SPELL_SCROLL 16
#define SPELL_WEAPON 32
#define SPELL_MOBILE 64 

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
#define TAR_SELF	   4096

typedef void (spellfuncType) 
	( int, charType *, char *, int,	charType *, objectType  * );

struct spell_info_type
{
	char *		name;
   	int 		min_level[4];
   	int 		max_skill[4];
   	int 	    min_mana;     /* Amount of mana used by a spell   */
   	int 		beats;        /* Heartbeats until ready for next */
  	spellfuncType (*func);
   	int 		min_position; /* Position for caster              */
   	int 	    targets;
   	int			type;
   	int			aggressive;

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.
*/
};

extern struct spell_info_type spells[MAX_SKILLS];

int spell_lookup( char * spname );

/* Attacktypes with grammar 
*/

struct attack_hit_type 
{
  	char *singular;
  	char *plural;
};

spellfuncType spell_dispel_evil;
spellfuncType spell_curse;
spellfuncType spell_blindness;
spellfuncType spell_burning_hands;
spellfuncType spell_call_lightning;
spellfuncType spell_chill_touch;
spellfuncType spell_shocking_grasp;
spellfuncType spell_colour_spray;
spellfuncType spell_earthquake;
spellfuncType spell_all_heal;
spellfuncType spell_hand_of_god;
spellfuncType spell_fire_storm;
spellfuncType spell_energy_drain;
spellfuncType spell_disintergrate;
spellfuncType spell_sunfire;
spellfuncType spell_sunburst;
spellfuncType spell_energyflow;
spellfuncType spell_fullfire;
spellfuncType spell_throw;
spellfuncType spell_fireball;
spellfuncType spell_corn_of_ice;
spellfuncType spell_harm;
spellfuncType spell_lightning_bolt;
spellfuncType spell_magic_missile;
spellfuncType spell_clone;
spellfuncType spell_armor;
spellfuncType spell_teleport;
spellfuncType spell_damage_up;
spellfuncType spell_crush_armor;
spellfuncType spell_bless;
spellfuncType spell_create_food;
spellfuncType spell_create_water;
spellfuncType spell_create_nectar;
spellfuncType spell_create_golden_nectar;
spellfuncType spell_cure_blind;
spellfuncType spell_mana_boost;
spellfuncType spell_vitalize;
spellfuncType spell_trick;
spellfuncType spell_cause_critic;
spellfuncType spell_cure_critic;
spellfuncType spell_cure_light;
spellfuncType spell_cause_light;
spellfuncType spell_detect_evil;
spellfuncType spell_detect_invisibility;
spellfuncType spell_preach;
spellfuncType spell_recharger;
spellfuncType spell_enchant_weapon;
spellfuncType spell_enchant_armor;
spellfuncType spell_pray_for_armor;
spellfuncType spell_heal;
spellfuncType spell_full_heal;
spellfuncType spell_invisibility;
spellfuncType spell_locate_object;
spellfuncType spell_poison;
spellfuncType spell_protection_from_evil;
spellfuncType spell_haste;
spellfuncType spell_improved_haste;
spellfuncType spell_remove_curse;
spellfuncType spell_remove_poison;
spellfuncType spell_infravision;
spellfuncType spell_mirror_image;
spellfuncType spell_sanctuary;
spellfuncType spell_sleep;
spellfuncType spell_strength;
spellfuncType spell_word_of_recall;
spellfuncType spell_summon;
spellfuncType spell_far_look;
spellfuncType spell_relocate;
spellfuncType spell_charm_person;
spellfuncType spell_sense_life;
spellfuncType spell_identify;
spellfuncType spell_fire_breath;
spellfuncType spell_frost_breath;
spellfuncType spell_lightning_breath;
spellfuncType spell_gas_breath;
spellfuncType spell_reanimate;
spellfuncType spell_full_fire;
spellfuncType spell_ventriloquate;
spellfuncType spell_phase;
spellfuncType spell_time_stop;
spellfuncType spell_sense_sprit;
spellfuncType spell_sanctuary_shield;
spellfuncType spell_fear;
spellfuncType spell_detect_good;
spellfuncType spell_portal;

#endif/*__MAGICS_H*/
