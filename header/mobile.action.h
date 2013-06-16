#ifndef __MOBILES_H
#define __MOBILES_H

/* fight mode */
#define	VIC_FIGHTING	0	/* victim who is fighting */
#define	VIC_WATCHING	1	/* victim who just watching */
#define	VIC_ALL			2	/* victim all fight or not .. */

/* mode */
#define	MODE_RANDOM		0	/* choose anyone */
#define	MODE_HIT_MIN	1	/* whose hit point is minimum */
#define	MODE_MANA_MIN	2	/* whose mana point is minimum */
#define	MODE_MOVE_MIN	3	/* whose move point is minimum */
#define	MODE_AC_MAX		4	/* poorest armor */
#define	MODE_HR_MIN		5	/* low hitroll */
#define	MODE_DR_MIN		6	/* low damroll */

#define	MODE_HIT_MAX	7	/* whose hit point is minimum */
#define	MODE_MANA_MAX	8	/* whose mana point is minimum */
#define	MODE_MOVE_MAX	9	/* whose move point is minimum */
#define	MODE_AC_MIN		10	/* poorest armor */
#define	MODE_HR_MAX		11	/* low hitroll */
#define	MODE_DR_MAX		12	/* low damroll */

#define	MODE_HIGH_LEVEL	13	/* level 31 - 40 */
#define	MODE_MID_LEVEL	14	/* level 13 - 30 */
#define	MODE_LOW_LEVEL	15	/* level 1 - 12 */

#define	MODE_LEVEL_MAX	16	/* max level.. */
#define	MODE_LEVEL_MIN	16	/* min level.. */

#define MODE_HAS_OKSE	17	/* who has okse - maybe king ? */

void mob_spell_fire_storm( int level, charType *ch );
void mob_spell_hand_of_god( charType *ch );

charType *choose_victim(charType *mob, int fightmode, int mode);
void mob_light_move(charType *ch, char *argument, int cmd);

void mobile_activity( void );
void mob_rescue( charType *, charType *, charType * );

#endif/*__MOBILES_H*/
