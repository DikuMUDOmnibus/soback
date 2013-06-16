#ifndef __HANDLER_H
#define __HANDLER_H

struct object_record_type
{
	objectType * 	obj;
	int  			num;
};

objectType * 	find_off_obj( objectType *, int nr );
void 			renum_off_list( objectType * );
void			detach_off_obj( objectType ** , objectType * );

void 			init_search_records	( void );
int  			record_found_object	( objectType * obj, int sameobj );
int 			record_found_char		( charType * );

void obj_to_char(objectType *object, charType *ch);
void obj_from_char(objectType *object);

objectType *get_obj_in_list(char *name, objectType *list);
objectType *get_obj_in_list_num(int num, objectType *list);
objectType *get_obj(char *name);
objectType *get_obj_num(int nr);

void obj_to_room(objectType *object, int room);
void obj_from_room(objectType *object);
void obj_to_obj(objectType *obj, objectType *obj_to);
void obj_from_obj(objectType *obj);
void obj_list_new_owner(objectType *list, charType *ch);

charType *get_char_room(char *name, int room);
charType *get_char_num(int nr);
charType *get_char(char *name);

void char_from_room( charType * ch );
void char_to_room( charType * ch, int room );
void char_from_world( charType * ch );

/* find if character can see */
charType *get_char_room_vis(charType *ch, char *name);
charType *get_char_vis(charType *ch, char *name);
charType *get_char_vis_player(charType *ch, char *name);/*cyb*/
charType *get_char_vis_zone(charType *ch, char *name);
objectType *get_obj_in_list_vis(charType *ch, char *name, 
				objectType *list);
objectType *get_obj_vis(charType *ch, char *name);
charType *get_specific_vis(charType *ch,char *name,int type);
objectType * get_obj_in_equip_vis( charType *, char *, objectType ** );

/* Generic Find */

int generic_find(char *arg, int bitvector, charType *ch,
                   charType **tar_ch, objectType **tar_obj);

#define FIND_CHAR_ROOM     1
#define FIND_CHAR_WORLD    2
#define FIND_OBJ_INV       4
#define FIND_OBJ_ROOM      8
#define FIND_OBJ_WORLD    16
#define FIND_OBJ_EQUIP    32

extern	int								objs_in_found_table;
extern	struct object_record_type	  	found_objects[];
extern	int								chars_in_found_table;
extern	charType					* 	found_chars[];

#endif/*__HANDLER_H*/
