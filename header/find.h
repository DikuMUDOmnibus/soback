#ifndef __FIND_H
#define __FIND_H

#define FIND_CHAR_ROOM     1
#define FIND_CHAR_WORLD    2
#define FIND_OBJ_INV       4
#define FIND_OBJ_ROOM      8
#define FIND_OBJ_WORLD    16
#define FIND_OBJ_EQUIP    32

struct object_record_type
{
	objectType * 	obj;
	int  			num;
};

objectType * 	find_off_obj			( objectType *, int nr );
void 			renum_off_list			( objectType * );
void			detach_off_obj			( objectType ** , objectType * );

void 			init_search_records		( void );
int  			record_found_object		( objectType * obj, int sameobj );
int 			record_found_char		( charType * );

charType *		find_char_room			( charType * ch, char *name );
charType *		find_mob_room_at		( charType * ch, int, char *name );
charType *		find_player_room_at		( charType * ch, int, char *name );
charType *		find_char_room_at		( charType * ch, int, char *name );
charType *		find_char_zone			( charType * ch, char *name );
charType *		find_char				( charType * ch, char *name );
charType *		find_player				( charType * ch, char *name );

objectType *	find_obj				( charType *, char * );
objectType * 	find_obj_equip			( charType *, char * );
objectType * 	find_obj_room			( charType *, char * );
objectType * 	find_obj_inven			( charType *, char * );
objectType *	find_obj_list			( charType *, char *, objectType *, int );

int find(char *, int bitvector, charType *, charType **, objectType ** );

extern	int								objs_in_found_table;
extern	struct object_record_type	  	found_objects[];
extern	int								chars_in_found_table;
extern	charType					* 	found_chars[];

#endif/*__FIND_H*/
