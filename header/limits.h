#ifndef __LIMITS_H
#define __LIMITS_H

#ifndef __CHARACTER_H
#include "character.h"
#endif

#ifndef __VARIABLES_H
#include "variables.h"
#endif

#define SAVING_PARA       0
#define SAVING_ROD        1
#define SAVING_PETRI      2
#define SAVING_BREATH     3
#define SAVING_SPELL      4

int		calc_used_value( objectType * obj );

int 	mana_limit	( charType *ch );
int 	hit_limit	( charType *ch );
int 	move_limit	( charType *ch );
int 	mana_gain	( charType *ch );
int 	hit_gain	( charType *ch );
int 	move_gain	( charType *ch );

int 	calc_hit_rate( charType * ch );

void 	gain_condition(charType *ch,int condition,int value);
void 	gain_exp( charType *ch, int gain );
void 	loose_exp_flee( charType *ch, int gain );
void 	loose_exp_die( charType *ch );
void 	gain_exp_and_adv( charType *ch, int gain );

int 	saves_spell(charType * ch, charType *vict, int type);

void 	advance_level(charType *ch);

int 	int_rate			( charType * ch, int input, int level );
int 	wis_rate			( charType * ch, int input, int level );
int 	dex_rate			( charType * ch, int input, int level );

void 	change_alignment	(charType *ch, charType *victim);

void	set_title			( charType * );

int		can_see				( charType * ch, charType * sub );
int		can_see_obj			( charType * ch, objectType * obj );

int		can_carry_number	( charType * ch );
int		can_carry_weight	( charType * ch );

int		can_carry			( charType * ch, objectType * obj );
int		can_get				( charType * ch, objectType * obj );
int		can_go				( charType * ch, int dir );

struct str_app_type * str_apply( charType * ch );

extern unsigned char saving_throws[4][5][IMO+10];

#endif/*__LIMITS_H*/
