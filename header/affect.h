#ifndef __AFFECT_H
#define __AFFECT_H

#ifndef __CHARACTER_H
#include "character.h"
#endif

#define AFF_BLIND             (1 << 0)
#define AFF_INVISIBLE         (1 << 1)
#define AFF_DETECT_EVIL       (1 << 2)
#define AFF_DETECT_INVISIBLE  (1 << 3)
#define AFF_INFRAVISION       (1 << 4)
#define AFF_SENSE_LIFE        (1 << 5)
#define AFF_DAZZLE            (1 << 6)
#define AFF_SANCTUARY         (1 << 7)
#define AFF_GROUP             (1 << 8)
#define AFF_BERSERK           (1 << 9)
#define AFF_CURSE             (1 << 10)
#define AFF_MIRROR_IMAGE      (1 << 11)
#define AFF_POISON            (1 << 12)
#define AFF_PROTECT_EVIL      (1 << 13)
#define AFF_PARALYSIS         (1 << 14)
#define AFF_DETECT_GOOD    	  (1 << 15)
#define AFF_AMBUSH            (1 << 16)
#define AFF_SLEEP             (1 << 17)
#define AFF_MORPETH           (1 << 18)
#define AFF_SNEAK             (1 << 19)
#define AFF_HIDE              (1 << 20)
#define AFF_FEAR              (1 << 21)
#define AFF_CHARM             (1 << 22)
#define AFF_FOLLOW            (1 << 23)
#define AFF_HASTE             (1 << 24)
#define AFF_IMPROVED_HASTE    (1 << 25)
#define AFF_TIME_STOP		  (1 << 26)
#define AFF_LAST			  27

int				is_apply_ac			( objectType * obj );
void 			affect_total		( charType *ch );
void 			affect_modify		( charType *ch, int loc, short mod, long bitv, int add );
void 			affect_to_char		( charType *ch, affectType * af );
void 			affect_remove		( charType *ch, affectType * af );
void 			affect_from_char	( charType *ch, int skill);
int	 			affected_by_spell	( charType *ch, int skill );
void 			affect_join			( charType *ch, affectType * af, int avg_dur, int avg_mod );

void			roundAff_to_char	( charType *ch, roundAffType * rf );
void			roundAff_remove		( roundAffType * rf );

void 			equip_char			( charType *ch, objectType * obj, int pos );
objectType	* 	unequip_char		( charType *ch, int pos );

extern	roundAffType	*	roundAffs;
extern	roundAffType	*	next_roundAffs;

#endif/*__AFFECT_H*/
