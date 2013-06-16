#ifndef __SPECIALS_H
#define __SPECIALS_H

typedef int speicalfuncType( charType * ch, int cmd, char * arg );

typedef struct __special_unit__
{
	int							unit;
	char					*	name;
	void 				   (*   func)( void ); 
	int							tics;
	int							life;
	int							last;
	int							nr;

	struct __special_unit__	*	next;

} specialUnitType;

extern 	specialUnitType	    *	specUnit_list;

extern	void	add_specUnit( specialUnitType * );
extern	void	del_specUnit( specialUnitType * );

extern  void	update_specUnit( void );
extern  void	init_autoreboot( void );

extern  void    init_portal_gate( void );
extern  void	record_portal( objectType * );
/*
 * Speicals funcs for mobiles.
*/

speicalfuncType		cityguard;
speicalfuncType		guild;
speicalfuncType		puff;
speicalfuncType		fido;
speicalfuncType		janitor;
speicalfuncType		mayor;
speicalfuncType		mud_message;
speicalfuncType		Quest_bombard;
speicalfuncType		super_deathcure;
speicalfuncType		mom;
speicalfuncType		musashi;
speicalfuncType		snake;
speicalfuncType		singer;
speicalfuncType		thief;
speicalfuncType		magic_user;
speicalfuncType		superguard;
speicalfuncType		dragon;
speicalfuncType		kickbasher;
speicalfuncType		spitter;
speicalfuncType		shooter;
speicalfuncType		spell_blocker;
speicalfuncType		archmage;
speicalfuncType		helper;
speicalfuncType		great_mazinga;
speicalfuncType		teleport_machine;
speicalfuncType		school_cold;
speicalfuncType		school_gracia;
speicalfuncType		school_nara;
speicalfuncType		okse_sunin;
speicalfuncType		laiger;
speicalfuncType		gaiot;
speicalfuncType		december;
speicalfuncType		black_yacha;
speicalfuncType		donjonkeeper;
speicalfuncType		teof;
speicalfuncType		mikesmith;
speicalfuncType		sund_earl;
speicalfuncType		stu;
speicalfuncType		athos;
speicalfuncType		idiot;
speicalfuncType		butcher;
speicalfuncType		silktrader;
speicalfuncType		blinder;
speicalfuncType		hangman;
speicalfuncType		sphinx;
speicalfuncType		daimyo;

/* 
 * Special funcs for Objects 
*/

speicalfuncType		totem;
speicalfuncType		board;
speicalfuncType		mbox;
speicalfuncType		magicseed;
speicalfuncType		chamber;
speicalfuncType		marbles;
speicalfuncType		information;

/*
 * Special funcs for Rooms
*/
speicalfuncType		dump;
speicalfuncType		pet_shops;
speicalfuncType		hospital;
speicalfuncType		metahospital;
speicalfuncType		safe_house;
speicalfuncType		death_house;
speicalfuncType		level_gate;
speicalfuncType		bank;
speicalfuncType		portal;
speicalfuncType		neverland;
speicalfuncType		electric_shock;
speicalfuncType		control_center;

/* in spec.assign.c */

void 				assign_mobiles		( void );
void 				assign_objects		( void );
void 				assign_rooms		( void );

/* in spec.items.c */
int file_to_cookie(char *name, char *buf[MAX_FORTUNE_COOKIE]);
char *select_fortune_cookie( void );

extern char *  cookie[MAX_FORTUNE_COOKIE];     /* the fortuen cookie text  */

#endif/*__SPECIALS_H*/
