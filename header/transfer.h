#ifndef __TRANSFER_H
#define __TRANSFER_H

void obj_to_room(objectType *object, int room);
void obj_from_room(objectType *object);
void obj_to_obj(objectType *obj, objectType *obj_to);
void obj_from_obj(objectType *obj);
void obj_list_new_owner(objectType *list, charType *ch);

void obj_to_char(objectType *object, charType *ch);
void obj_from_char(objectType *object);

void char_from_room( charType * ch );
void char_to_room( charType * ch, int room );
void char_from_world( charType * ch );

#endif/*__TRANSFER_H*/
