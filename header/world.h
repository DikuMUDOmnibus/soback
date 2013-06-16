#ifndef __WORLD_H
#define __WORLD_H

#ifndef __CHARACTER_H
#include "character.h"
#endif

#ifndef __OBJECT_H
#include "object.h"
#endif

#define DARK           (1 << 0)      
#define NOSUMMON       (1 << 1)      
#define NO_MOB         (1 << 2)
#define INDOORS        (1 << 3)
#define DEATH_ROOM     (1 << 4)
#define NEUTRAL        (1 << 5)
#define CHAOTIC        (1 << 6)
#define NO_MAGIC       (1 << 7)
#define TUNNEL         (1 << 8)
#define PRIVATE        (1 << 9)
#define OFF_LIMITS     (1 <<10)
#define RENT           (1 <<11)
#define NORELOCATE     (1 <<12)
#define EVERYZONE      (1 <<13)

#define LAST_ROOM_FLAG	14

/* For 'dir_option' */  

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR    	(1 << 0)
#define EX_CLOSED    	(1 << 1)
#define EX_LOCKED    	(1 << 2)
#define EX_RSCLOSED  	(1 << 3)
#define EX_RSLOCKED  	(1 << 4)
#define EX_PICKPROOF 	(1 << 5)
#define EX_NOPHASE   	(1 << 6)

#define EX_LAST			7

/* For 'sector types' */

#define SECT_INSIDE          0  
#define SECT_CITY            1  
#define SECT_FIELD           2
#define SECT_FOREST          3        
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5        
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_SKY             8

#define SECT_LAST			 9

#define ROOM_INTERNAL		 0
#define ROOM_VOID			 1
#define ROOM_LIMBO			 2
#define ROOM_WIZARD			 3
#define ROOM_TEMPLE			 3001
#define ROOM_BANISH			 6999
#define ROOM_RENT			 3008
#define ROOM_SCHOOL			 2900
#define ROOM_POST_OFFICE	 3071
#define ROOM_MEDITATION	 	 3072
#define ROOM_LIBRARY	 	 3075
#define ROOM_AUDIENCE	 	 3073
#define ROOM_PAWN			 3308
#define ROOM_SILVER			 3070
#define ROOM_LOCKER			 3077
#define ROOM_BANK			 3002
#define ROOM_INSURANCE		 3312
#define ROOM_INFORMATION	 3082
#define ROOM_REBOOT			 3083
#define ROOM_DEATH			 99998

#ifndef __extra_description__
#define __extra_description__

typedef struct __extr_description__
{
  	char 	*	keyword;             /* Keyword in look/examine  */
  	char 	*	description;         /* What to see              */
  	struct __extr_description__ * next;  /* Next in list             */

} exdescriptionType;

#endif/*__extra_description__*/

typedef struct __directiontype__
{ 
	char  *		description;       /* When look DIR.                  */
	char  *		keyword;           /* for open/close                  */
	int 		exit_info;         /* Exit info                       */
	int 		key;               /* Key's number (-1 for no key)    */
	int 		to_room;           /* Where direction leeds (NOWHERE) */

} directionType;

typedef struct __roomtype__
{ 
    int     nr;    
    int     virtual;          	/* Rooms number                   */
    int     zone;               /* Room zone (for resetting)      */

    char *  name;               /* Rooms name 'You are ...'       */
    char *  description;        /* Shown when entered             */

    int     sector;        		/* sector type (move/hide)        */
    int     flags;         		/* DEATH,DARK ... etc             */

    exdescriptionType 	* extrd; 	/* for examine/look           */
    directionType 		* dirs[6]; 	/* Directions                 */

    int      light;             /* Number of lightsources in room */
    int     (*func)( struct __chartype__ *, int, char * );          
								/* special procedure              */

  	struct __obj_type__  * contents; /* List of items in room     */
  	struct __chartype__  * people; /* List of NPC / PC in room  */

	struct __roomtype__  * next; 	/* next room ptr in same zone */

} roomType;

void 		boot_rooms 			( FILE * );
void		write_rooms			( FILE *, int );
int  		real_roomNr			( int virtual );
void		reindex_world		( void );

extern int	rooms_in_world;
extern int	room_temple;
extern int	room_meditation;
extern int	room_library;
extern int	room_audience;

extern roomType 	*	world;

#endif/*__WORLD_H*/
