#ifndef __OBJECT_H
#define __OBJECT_H

#ifndef __DEFINES_H
#include "defines.h"
#endif/*__DEFINES_H*/

#ifndef __CHARACTER_H
#include "character.h"
#endif

#ifndef __extra_description__
#define __extra_description__

#define	MAX_OBJ_LIFE_SPAN	36000
#define OBJ_NOTIMER     	-7000000

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22

#define ITEM_LAST_TYPE	22

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE              	(1 << 0) 
#define ITEM_WEAR_FINGER       	(1 << 1)
#define ITEM_WEAR_NECK         	(1 << 2)
#define ITEM_WEAR_BODY         	(1 << 3)
#define ITEM_WEAR_HEAD        	(1 << 4)
#define ITEM_WEAR_LEGS        	(1 << 5)
#define ITEM_WEAR_FEET        	(1 << 6)
#define ITEM_WEAR_HANDS      	(1 << 7)
#define ITEM_WEAR_ARMS       	(1 << 8)
#define ITEM_WEAR_SHIELD     	(1 << 9)
#define ITEM_WEAR_ABOUT     	(1 << 10)
#define ITEM_WEAR_WAISTE    	(1 << 11)
#define ITEM_WEAR_WRIST     	(1 << 12)
#define ITEM_WIELD          	(1 << 13)
#define ITEM_HOLD          		(1 << 14)
#define ITEM_THROW         		(1 << 15)
#define ITEM_WEAR_KNEE     		(1 << 16)
#define ITEM_WEAR_ABOUTLEGS 	(1 << 17)

#define ITEM_LAST_WEAR			18

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW            	(1 << 0)
#define ITEM_HUM             	(1 << 1)
#define ITEM_DARK            	(1 << 2)
#define ITEM_LOCK            	(1 << 3)
#define ITEM_EVIL           	(1 << 4)
#define ITEM_INVISIBLE      	(1 << 5)
#define ITEM_MAGIC          	(1 << 6)
#define ITEM_NODROP        		(1 << 7)
#define ITEM_BLESS         		(1 << 8)
#define ITEM_ANTI_GOOD     		(1 << 9)
#define ITEM_ANTI_EVIL    		(1 << 10)
#define ITEM_ANTI_NEUTRAL 		(1 << 11)
#define ITEM_NOLOAD       		(1 << 12)
#define ITEM_ANTI_MAGE    		(1 << 13)
#define ITEM_ANTI_CLERIC 		(1 << 14)
#define ITEM_ANTI_THIEF  		(1 << 15)
#define ITEM_ANTI_WARRIOR 		(1 << 16)
#define ITEM_NO_RENT     		(1 << 17)

#define ITEM_LAST_FLAG			18
/* modifiers to char's abilities */   

#define APPLY_NONE              0     
#define APPLY_STR               1     
#define APPLY_DEX               2     
#define APPLY_INT               3     
#define APPLY_WIS               4     
#define APPLY_CON               5     
#define APPLY_SEX               6     
#define APPLY_CLASS             7     
#define APPLY_LEVEL             8     
#define APPLY_AGE               9     
#define APPLY_CHAR_WEIGHT       10    
#define APPLY_CHAR_HEIGHT       11    
#define APPLY_MANA              12    
#define APPLY_HIT               13    
#define APPLY_MOVE              14    
#define APPLY_GOLD              15    
#define APPLY_EXP               16    
#define APPLY_AC                17    
#define APPLY_ARMOR             17    
#define APPLY_HITROLL           18    
#define APPLY_DAMROLL           19    
#define APPLY_SAVING_PARA       20    
#define APPLY_SAVING_ROD        21    
#define APPLY_SAVING_PETRI      22    
#define APPLY_SAVING_BREATH     23    
#define APPLY_SAVING_SPELL      24    
#define APPLY_REGENERATION      25    
#define APPLY_INVISIBLE         26    
#define APPLY_SANCTUARY         27    
#define APPLY_DETECT_EVIL       28    
#define APPLY_SENSE_LIFE        29    
#define APPLY_DETECT_INVISIBLE  30    
#define APPLY_STR_ADD           31    
#define APPLY_MIRRO_IMAGE       32    
#define APPLY_DAMAGE_UP         33    
#define APPLY_HASTE             34    
#define APPLY_IMPROVED_HASTE    35    

#define ITEM_LAST_APPLY			35


#define CONT_CLOSEABLE      	1   
#define CONT_PICKPROOF      	2
#define CONT_CLOSED         	4
#define CONT_LOCKED         	8

#define ITEM_LAST_CLOSE_FLAG	4

#define OBJECT_NULL			0
#define OBJECT_FOOD			1
#define OBJECT_COINS		2
#define OBJECT_PORTAL		3
#define OBJECT_CORPSE		4
#define OBJECT_INTERNAL		(OBJECT_CORPSE)
#define OBJECT_PAPER		3035
#define OBJECT_INFORMATION	3077

typedef struct __extr_description__
{
    char    *   keyword;             /* Keyword in look/examine  */
    char    *   description;         /* What to see              */
    struct __extr_description__ * next;  /* Next in list             */

} exdescriptionType;

#endif/*__extra_description__*/


typedef struct
{
  	unsigned char 	location;  /* Which ability to change (APPLY_XXX) */
  	char 			modifier;  /* How much it changes by              */

} applyType;

typedef struct __obj_type__
{
  int       unit;
  int   	nr;            		 	/* Where in data-base               */
  int		virtual;
  int 		in_room;             	/* In what room -1 when conta/carr  */

  char *	name;                   /* Title of object :get etc.        */
  char *	wornd;      			/* when worn/carry/in cont.         */
  char *	usedd;     				/* What to write when used          */
  char *	roomd;           		/* When in room                     */

  exdescriptionType * extrd; 		/* extra descriptions     */

  byte 		type;     			 	/* Type of item                     */
  word 		extra;               	/* If it hums,glows etc             */
  word 		wear;     			 	/* Where you can wear it            */

  int 		value[4];       	 	/* Values of the item (see list)    */
  int 		weight;         		/* Weigt what else                  */
  word 		cost;           		/* Value when sold (gp.)            */
  int 		magic;
  int 		level;          		/* level limits                     */
  int		what;					/* general purpose data				*/

  int 		timer;          		/* Timer for object                 */
  int 		status;					/* Status of item 100 is max		*/
  int 		limit;					/* Limit of staus of item 100 is max*/
  word 		bitvector;     			/* To set chars bits                */

  applyType 			apply[MAX_APPLY]; 

  struct __chartype__ * carried_by;  /* Carried by :NULL in room/conta   */
  struct __chartype__ * equiped_by;  /* Equiped by :NULL in room/conta   */

  struct __obj_type__ * in_obj;       /* In what object NULL when none   */
  struct __obj_type__ * contains;     /* Contains objects                */

  struct __obj_type__ * next_content; /* For 'contains' lists            */
  struct __obj_type__ * next;         /* For the object list             */

} objectType;

typedef struct object_index
{
	int		  nr;
	int       virtual;              	/* Where in data-base               */

  	char *    name;                   	/* Title of object :get etc.        */ 
  	char *    roomd;           			/* When in room                     */ 
  	char *    wornd;      				/* when worn/carry/in cont.         */ 
  	char *    usedd;     				/* What to write when used          */

  	exdescriptionType * extrd;			/* extra descriptions     */

  	byte      type;                   	/* Type of item                     */
  	word      extra;                  	/* If it hums,glows etc             */
  	word      wear;                   	/* Where you can wear it            */

  	int       value[4];               	/* Values of the item (see list)    */
  	int       weight;                 	/* Weigt what else                  */
  	word      cost;                   	/* Value when sold (gp.)            */
  	int       magic;
  	int       level;                  	/* level limits                     */

  	applyType             apply[MAX_APPLY];

  	int		  in_world;
  	int		  off_world;

  	int		 (*func)( struct __chartype__ * ch, int cmd, char * arg );

} objIndexType;

objIndexType		*	objects;
objectType          *	obj_list;

void 					boot_objects	( FILE * );
void 					write_objects	( FILE *, int nr, int bottom, int top );
void					write_extrd		( FILE *, exdescriptionType * );
objectType 			*	load_a_object 	( int nr, int isVirtual, int in_world );
int						real_objectNr	( int virtual );
void					clear_object	( struct __obj_type__ * );
void					wipe_obj_list   ( struct __obj_type__ * );
void					extract_obj		( struct __obj_type__ *, int in_world );
void					extract_all_obj	( struct __chartype__ * );
void					wipe_all_obj	( struct __chartype__ * );
void					make_a_corpse	( struct __chartype__ *, int );
void					make_c_corpse	( struct __chartype__ *, int );
void					make_t_corpse	( struct __chartype__ *, int );
void					make_d_corpse	( struct __chartype__ *, int );
void					make_h_corpse	( struct __chartype__ *, int );
void					make_l_corpse	( struct __chartype__ *, int );
void					make_ll_corpse	( struct __chartype__ *, int );
void					make_rl_corpse	( struct __chartype__ *, int );
void					make_lh_corpse	( struct __chartype__ *, int );
void					make_rh_corpse	( struct __chartype__ *, int );
void					make_lf_corpse	( struct __chartype__ *, int );
void					make_rf_corpse	( struct __chartype__ *, int );
void					make_nt_corpse	( struct __chartype__ *, int );
void					make_vh_corpse	( struct __chartype__ *, int );

objectType			*	create_money	( int amount );
objectType			*	create_food		( void );
objectType			*	create_portal	( struct __chartype__ *, int );

void					obj_to_in_world	( struct __obj_type__ * );
void					obj_to_off_world( struct __obj_type__ * );

extern	int				objs_in_world;
extern	int				objs_off_world;

#endif/*__OBJECT_H*/
