#ifndef __ALLOCATE_H
#define __ALLOCATE_H

#ifndef __CHARACTER_H
#include "character.h"
#endif

#ifndef __OBJECT_H
#include "object.h"
#endif

#ifndef __SPECIALS_H
#include "specials.h"
#endif

#ifndef __HASH_H
#include "hash.h"
#endif

#ifndef __QUEUE_H
#include "queue.h"
#endif

struct alloc_info
{   
    char    *   name;
    int         nextfree;
    int         used;
    int         max;
    int         unitsize;
    int         delta;
    void   **   table;
	int		*	index;
	int			fixed;
};

charType 		* 	alloc_char		( void );
objectType  	* 	alloc_object	( void );
descriptorType  * 	alloc_descriptor( void );
affectType		*	alloc_affect	( void );
roundAffType	*	alloc_round		( void );
specialUnitType	*	alloc_specUnit	( void );
hashType	    *   alloc_hashType  ( void );
queueNode		*	alloc_queue		( void );

void				free_affect		( affectType * );
void				free_round		( roundAffType * );
void				free_specUnit	( specialUnitType * );
void                free_hashType   ( hashType * );
void                free_desc       ( descriptorType * );
void                free_char       ( charType * );
void				free_object		( objectType * );
void				free_queue		( queueNode * );

void			* 	alloc_unit		( struct alloc_info * );
void		  		free_unit		( struct alloc_info *, void * );

void				expand_table	( struct alloc_info * );
void				init_allocation ( void );

extern struct alloc_info	mob_index_info;
extern struct alloc_info	obj_index_info;
extern struct alloc_info   room_index_info;
extern struct alloc_info   zone_index_info;

#endif
