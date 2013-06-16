#ifndef __MOBILE_H
#define __MOBILE_H

#ifndef __CHARACTER_H
#include "character.h"
#endif

/* for mobile actions: specials.act */
#define ACT_SPEC            (1 << 0)
#define ACT_SENTINEL        (1 << 1)
#define ACT_SCAVENGER       (1 << 2)
#define ACT_ISNPC           (1 << 3)
#define ACT_NICE_THIEF      (1 << 4)
#define ACT_AGGRESSIVE      (1 << 5)
#define ACT_STAY_ZONE       (1 << 6)
#define ACT_WIMPY           (1 << 7)
#define ACT_FIGHTER         (1 << 8)
#define ACT_MAGE            (1 << 9)
#define ACT_CLERIC          (1 << 10)
#define ACT_THIEF           (1 << 11)
#define ACT_PALADIN         (1 << 12)
#define ACT_DRAGON          (1 << 13)
#define ACT_SPITTER         (1 << 14)
#define ACT_SHOOTER         (1 << 15)
#define ACT_GUARD           (1 << 16)
#define ACT_SUPERGUARD      (1 << 17)
#define ACT_GIANT           (1 << 18)
#define ACT_HELPER          (1 << 19)
#define ACT_RESCUER         (1 << 20)
#define ACT_SPELL_BLOCKER   (1 << 21)

#define ACT_LAST			22

#define MOBILE_NULL			0
#define MOBILE_INTERNAL		0
#define MOBILE_PAWN			3300
#define MOBILE_REPAIR		3068
#define MOBILE_INSURANCE	3301
#define MOBILE_INS_KID		3302
#define MOBILE_REBOOT		3080
#define MOBILE_SHUTDOWN		3081
#define MOBILE_RESET		3082

typedef struct mobile_index
{
    int        nr;
    int        virtual;

    char  *    name;
    char  *    moved;
    char  *    roomd;
    char  *    description;

    long       act;
    long       affected;
    int        level;
	int		   class;

	int		   hbase;
    int        hsdice;
	int        hndice;
    long       gold; 
    long       exp;  
    int        align;
    int        ac;
    int        hr;
    int        dr;
    int        ndice;
    int        sdice;
    int        sex;  
    int        position;
    int        defaultpos;

	int		  (*func)( struct __chartype__ *, int, char * );

	int		   in_world;

} mobIndexType; 

void 			boot_mobiles	( FILE * );
charType 	* 	load_a_mobile	( int nr, int isVirtual );
void			write_mobiles	( FILE *, int nr, int bottom, int top );
void			free_a_char		( charType * ch );
int				real_mobileNr	( int virtual );

extern charType 			*	mob_list;
extern struct mobile_index 	*	mobiles;
extern int						mobs_in_world;

#endif/*__MOBILE_H*/
