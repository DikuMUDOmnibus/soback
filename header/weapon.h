#ifndef __WEAPON_H
#define __WEAPON_H
/*  Define for magic weapon */

#define WEAPON_SMASH 			1
#define WEAPON_FLAME 			2
#define WEAPON_ICE 				3
#define WEAPON_BOMBARD 			4
#define WEAPON_SHOT 			5
#define WEAPON_DRAGON_SLAYER 	6
#define WEAPON_ANTI_EVIL_WEAPON 7
#define WEAPON_ANTI_GOOD_WEAPON 8
#define WEAPON_GIANT_SLAYER 	9
#define WEAPON_MANA_DRAIN		10
#define WEAPON_HIT_DRAIN		11
#define WEAPON_MOVE_DRAIN		12
#define WEAPON_BOLT 			13

#define WEAPON_LAST_SPECIAL		13

void magic_weapon_hit( charType *ch, charType *vict, objectType *weapon );
#endif/*__WEAPON_H*/
