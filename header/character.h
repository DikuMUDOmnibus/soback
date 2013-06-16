#ifndef __CHARACTER_H
#define __CHARACTER_H

#ifndef __DEFINES_H
#include "defines.h"
#endif

#ifndef __OBJECT_H
#include "object.h"
#endif

#ifndef __SOCKETS_H
#include "sockets.h"
#endif

#define DRUNK        0
#define FULL         1
#define THIRST       2

#define MAX_WEAR    21
#define MAX_AFFECT  25    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

#define WEAR_LIGHT      0     
#define WEAR_FINGER_R   1     
#define WEAR_FINGER_L   2     
#define WEAR_NECK_1     3     
#define WEAR_NECK_2     4     
#define WEAR_BODY       5     
#define WEAR_HEAD       6     
#define WEAR_LEGS       7     
#define WEAR_FEET       8     
#define WEAR_HANDS      9     
#define WEAR_ARMS      10     
#define WEAR_SHIELD    11     
#define WEAR_ABOUT     12     
#define WEAR_WAISTE    13     
#define WEAR_WRIST_R   14     
#define WEAR_WRIST_L   15     
#define WIELD          16     
#define HOLD           17     
#define WEAR_KNEE_R    18
#define WEAR_KNEE_L    19
#define WEAR_ABOUTLEGS 20   

#define CLASS_MAGIC_USER  1
#define CLASS_CLERIC      2
#define CLASS_THIEF       3
#define CLASS_WARRIOR     4
#define PC_CLASS_LAST	  4

#define CLASS_OTHER       0 /* These are not yet used!   */
#define CLASS_UNDEAD      1 /* But may soon be so        */
#define CLASS_HUMANOID    2 /* Idea is to use general    */
#define CLASS_ANIMAL      3 /* monster classes           */
#define CLASS_DRAGON      4 /* Then for example a weapon */
#define CLASS_GIANT       5 /* of dragon slaying is pos. */
#define NPC_CLASS_LAST	  5

#define SEX_NEUTRAL       0
#define SEX_MALE          1
#define SEX_FEMALE        2
#define SEX_LAST		  2

#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8
#define POSITION_LAST		8

#define PLR_BRIEF           (1 << 0)
#define PLR_NOSHOUT         (1 << 1)
#define PLR_COMPACT         (1 << 2)
#define PLR_DONTSET         (1 << 3)
#define PLR_NOTELL          (1 << 4)
#define PLR_BANISHED        (1 << 5)  
#define PLR_CRIMINAL        (1 << 6)  
#define PLR_WIZINVIS        (1 << 7)  
#define PLR_EARMUFFS        (1 << 8)  
#define PLR_XYZZY           (1 << 9)  
#define PLR_AGGR            (1 << 10) 
#define PLR_KOREAN          (1 << 11) 
#define PLR_NOCHAT          (1 << 12) 
#define PLR_WIMPY           (1 << 13) 
#define PLR_AUTO_EXIT       (1 << 14) 
#define PLR_AFK             (1 << 15)
#define PLR_LAST			16
                                      
#define MAX_CONDITION		3
#define MAX_SAVING			5

typedef struct 
{
    byte   			hours,
                    day,
                    month;
    short           year;

} timeType;


typedef struct
{
	char 	str;
	char 	str_add;
	char 	intel;
	char 	wis;
	char 	dex;
  	char 	con;

} abilityType;
 
typedef struct __affecttype__
{
    int         unit;
    short       type;      
    short       duration;  
    short       modifier;  
    byte        location;  
    long        bitvector;

    struct      __affecttype__ * next;

} affectType;

typedef struct __roundaff__ 
{
	int		unit;

	int		type;
	int		duration;
	int		modifier;
	int		location;
	int		bitvector;
	int		d1;
	int		d2;
	int		d3;
	int		d4;

	struct __roundaff__	*	next;
	struct __roundaff__	*	my_next;
	struct __chartype__	*	owner;

} roundAffType; 

typedef struct __followtype__
{
  	struct __chartype__ 	*	follower;
  	struct __followtype__ 	*	next;

} followType;

typedef struct __chartype__
{
	int         unit;
  	int   		nr;
  	int   		in_room;
	int			was_in_room;

  	char  *   	name;
  	char  *  	moved;
  	char  *   	roomd;
  	char  *   	title;
  	char  *   	description;

  	struct __descriptortype__ * desc;

    int     	mana;
    int     	max_mana;       /* Not useable may be erased upon player file */
    int     	hit;
    int     	max_hit;        /* Max hit for NPC                         */
    int     	move;
    int     	max_move;       /* Max move for NPC                        */
    int     	armor;          /* Internal -100..100, external -10..10 AC */
    word   		gold;           /* Money carried                           */
    word     	exp;            /* The experience of the player            */

    short    	hr;        		/* Any bonus or penalty to the hit roll    */
    short    	dr;        		/* Any bonus or penalty to the damage roll */


  	byte        sex;
  	byte        class;
  	byte        level;

  	byte        weight;
  	byte        height;

  	time_t      birth;
  	time_t      logon;
	int			life;
  	int     	played;
  	int			age_mod;

	int 		align;
 	word 		bank;
	int			regen;

	byte 		ndice;   
	byte 		sdice; 

	byte		attack_type;
	byte 		last_move;

  	timeType 	time; 
  
  	abilityType base_stat;
  	abilityType temp_stat;

	long		affects;
	long		affround;
	long		act;
	short		channel;

	byte		position;
	byte		defaultpos;

	byte		prNr;

	byte		carry_items;
	short		carry_weight;

	int			timer;

  	int 		quest[2];
  	byte 		skills[MAX_SKILLS];
	char		saving_throw[5];
	char		conditions[3];

	int			mode;
	int			sub_1;
	int			sub_2;
	int			sub_3;
	int			page;

	int			screen;
	int			qSize;

  	affectType 	 *	affected;
	roundAffType *	roundAffs;

  	struct __obj_type__ 	*	equipment[MAX_WEAR];
  	struct __obj_type__ 	*	carrying;
  	struct __obj_type__ 	*	off_world;
  	struct __chartype__ 	*	next_in_room;

  	struct __chartype__ 	*	next;
  	struct __chartype__ 	*	next_char;

  	struct __chartype__ 	*	next_fighting;

	struct __chartype__		*	hunt;
	struct __chartype__		*	fight;

  	struct __chartype__ 	*	master;
  	followType  			*	followers;

} charType;
  
typedef struct  __storechar__
{
  	char 		name[20];
  	char 		pass[11];

  	char 		title[80];
  	char 		description[240];

  	byte 		sex,class,level;

  	time_t 		birth;
  	int 		played;
	int			life;

  	byte 		weight;
	byte		height;

  	int 		load_room;

  	abilityType	stat;

    int         mana;
    int         max_mana; 
    int         hit;
    int         max_hit; 
    int         move;
    int         max_move;
    int         armor;  
    word        gold;  
    word        exp;  

    byte        hr;  
    byte        dr; 

  	byte	 	skills[MAX_SKILLS];
  	affectType 	affected[MAX_AFFECT]; 
  	char 		conditions[3];

  	byte 		prNr;
  	int 		align;
  	time_t 		last_logon;  
  	word 		act; 
  	word 		bank;     
	int			channel;
  	int 		qSize;
  	int			screen;
  	int 		reserved[9]; 	/*  reserving for future expansion */
  	int 		quest[2]; 		

} storecharType;

struct player_index
{
    int         nr;
	char *      name;
};

void 	init_char						( struct __chartype__ *ch);
void 	reset_char						( struct __chartype__ *ch);
void 	free_char						( struct __chartype__ *ch);
void 	store_to_char_for_transform		( struct __storechar__ *st,  struct __chartype__ *ch);
void 	unstash_char					( struct __chartype__ *ch, char *filename );
int 	stash_char						( struct __chartype__ *ch, char *filename );
void 	store_to_char					( struct __storechar__ *st, struct __chartype__ *ch);
int  	load_char						( char *, struct __storechar__ * );
void 	clear_char						( struct __chartype__ *ch);
void 	free_char						( struct __chartype__ *ch);
void 	save_char						( struct __chartype__ *, int load_room);
void	build_player_index				( void );

int		find_name						( char * name );
int 	create_entry					( char * name );
void 	delete_char						( struct __descriptortype__ * d );

void    extract_char					( struct __chartype__ * ch );

void				free_a_player		( struct __chartype__ * );
struct __chartype__	alloc_a_player		( void );

extern charType 	*			char_list;
extern charType 	*			player_list;
extern int						chars_in_world;
extern int						players_in_world;
extern int						top_of_p_table;
extern struct player_index * 	player_table;

#endif/*__CHARACTER_H*/
