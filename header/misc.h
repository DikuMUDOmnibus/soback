#ifndef __MISC_H
#define __MISC_H

/* in act.offensive.c */
void shoot(charType *ch, charType *victim, int type);
int  is_allow_kill( charType *, charType * );

/* in act.movement.c */
int do_simple_move(charType *ch, int cmd, int following);
void page_string(descriptorType *d, char *str, int keep_internal);
int implementor(char *name);
void lastchat_add( char * );
int find_door(charType *ch, char *type, char *dir);

/* in act.look.c */
void list_obj_to_char( objectType * list, charType *, int mode, int finish );
void list_room_chars( charType * ch, charType * room ); 

/* in act.who.c */

void who_service( descriptorType * d );

void weight_change_object(objectType *obj, int weight);
void wear(charType *ch, objectType *obj_object, int keyword);

/* in act.group.c */
void group_gain(charType *ch, charType *victim);
int  sum_gr_member( charType * );
int  circle_follow( charType *, charType * );
void add_follower( charType *, charType * );
void stop_follower( charType * );
void die_follower( charType * );
int  is_same_group( charType *, charType * );

/* in act.wizard.c */

void do_start( charType * ch );
void roll_abilities(charType *ch);

/* in modify.c */
void page_string(descriptorType *d, char *str, int keep_internal);
void show_string(descriptorType *d, char *input);
void string_add( descriptorType *d, char *input);

/* in act.social.c */
void boot_social_messages(void);

/* in act.help.c */
void build_help_index( void );

/* in act.second_skill.c */
void serpent_stab( charType * ch, roundAffType * rf );

#endif/*__MISC_H*/
