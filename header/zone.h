#ifndef __ZONE_H
#define __ZONE_H

#ifndef __CHARACTER_H
#include "character.h"
#endif

#define ZONE_INTERNAL	0

struct reset_command
{  
    char 	command;   /* current command                      */
    int 	if_flag;   /* if TRUE: exe only if preceding exe'd */
    int 	arg1;      /*                                      */
    int 	arg2;      /* Arguments to the command             */
    int 	arg3;      /*                                      */
   
    /*
    *  Commands:              *
    *  'M': Read a mobile     *
    *  'O': Read an object    *
    *  'G': Give obj to mob   *
    *  'P': Put obj in obj    *
    *  'G': Obj to char       *
    *  'E': Obj to char equip *
    *  'D': Set state of door *
    */
};

typedef struct __zonetype__
{
	int		nr;
	int		zone;
    char *	name;     	/* name of this zone                  */
	char *	info;       /* general infomation of this zone    */
	char *  path;		/* path to this zone from midgaard    */
	int		low, high;  /* player level range for this zone   */
	int		aggressive;
	int		align;
    int 	lifespan;   /* how long between resets (minutes)  */
    int 	age;        /* current age of this zone (minutes) */
	int		max;		/* upper limit for virtualNr in this zone */
    int 	top;        
	int 	bottom;
	int		rooms_in_zone;
   
    struct reset_command * cmds;/* command table for reset          */
    int reset_mode;         	/* conditions for reset (see below) */

	/*
     *  Reset mode:                              *
     *  0: Don't reset, and don't update age.    *
     *  1: Reset if no PC's are located in zone. *
     *  2: Just reset.                           *
    */

	roomType	* 	rooms;

} zoneType;

void 		boot_world 			( void );
void		reset_zone			( int, int  );
void		do_zone				( charType * ch, char * arg, int cmdNr );
void		do_resetzone		( int zoneNr );
void		update_zones		( void );
void		save_a_zone			( int );
void		save_all_zone		( void );

extern	zoneType	*	zones;
extern	int				zones_in_world;

#endif/*__ZONE_H*/
