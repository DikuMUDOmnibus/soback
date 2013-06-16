#ifndef __FIGHT_H
#define __FIGHT_H

#define MAX_MESSAGES	60

#define MESS_ATTACKER 	1
#define MESS_VICTIM   	2
#define MESS_ROOM     	3

struct msg_type
{
  char *attacker_msg;  /* message to attacker */
  char *victim_msg;    /* message to victim   */
  char *room_msg;      /* message to room     */
};

struct message_type
{
  struct msg_type die_msg;      /* messages when death            */
  struct msg_type miss_msg;     /* messages when miss             */
  struct msg_type hit_msg;      /* messages when hit              */
  struct msg_type sanctuary_msg;/* messages when hit on sanctuary */
  struct msg_type god_msg;      /* messages when hit on god       */
  struct message_type *next;/* to next messages of this kind.*/
};

struct message_list
{
  int a_type;               /* Attack type              */
  int number_of_attacks;    /* How many attack messages to chose from. */
  struct message_type *msg; /* List of messages.         */
};

void update_pos( charType *victim );
void kill_char( charType *victim, int level, int dam, int attacktype );
void hit(charType *ch, charType *victim, int type);
void damage(charType *ch, charType *vict, int dam, int type );
void stop_fighting(charType *ch);
void set_fighting(charType *ch, charType *vict);
void perform_violence( void );
void load_messages( void );
int  is_fighting( charType * ch );

extern charType *combat_list;  
extern charType *combat_next_dude;

#endif/*__FIGHT_H*/
