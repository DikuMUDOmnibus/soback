#ifndef __MACROS_H
#define __MACROS_H

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

#define MAX(a, b) max((a), (b))
#define MIN(a, b) min((a), (b))

#define ISNEWL(ch) 	((ch) == '\n' || (ch) == '\r') 

#define IF_STR(st) 	((st) ? (st) : "\0")
#define IS_STR(s)	((s) && (*s))

#define SWITCH(a,b) { (a) ^= (b); (b) ^= (a); (a) ^= (b); }

#define IS_AFFECTED(ch,skill) ( IS_SET((ch)->affects, (skill)) )

#define IS_DARK(room)  (!world[room].light &&  IS_SET(world[room].flags, DARK))
#define IS_LIGHT(room)  (world[room].light || !IS_SET(world[room].flags, DARK))

#define IS_SET(var,bit)  		((var) & (bit))
#define SET_BIT(var,bit)  		((var) = (var) | (bit))
#define REMOVE_BIT(var,bit)  	((var) = (var) & ~(bit) )

#define IS_WEAR(obj, part)		(IS_SET(obj->wear, part))

#define OMNI(sub) (!IS_NPC(sub) && (GET_LEVEL(sub) >= IMO))
#define IMPL(sub) (!IS_NPC(sub) && (GET_LEVEL(sub) == IMO + 3))

#define HSSH(ch) ((ch)->sex ? (((ch)->sex == 1) ? "he" : "she") : "it")
#define HSHR(ch) ((ch)->sex ? (((ch)->sex == 1) ? "his" : "her") : "its")
#define HMHR(ch) ((ch)->sex ? (((ch)->sex == 1) ? "him" : "her") : "it")  

#define  ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define PERS(ch,  vict) (see_whom( vict, ch ))
#define OBJS(obj, vict) (can_see_obj((vict), (obj)) ? (obj)->wornd  : "something")
#define OBJN(obj, vict) (can_see_obj((vict), (obj)) ? oneword((obj)->name) : "something")

#define IS_NPC(ch)  (IS_SET((ch)->act, ACT_ISNPC))
#define IS_MOB(ch)  (IS_SET((ch)->act, ACT_ISNPC) && ((ch)->nr > 1))

#define GET_POS(ch)     ((ch)->position)
#define GET_COND(ch, i) ((ch)->conditions[(i)])
#define GET_NAME(ch)    ((ch)->name)
#define GET_TITLE(ch)   ((ch)->title)
#define GET_LEVEL(ch)   ((ch)->level)
#define GET_CLASS(ch)   ((ch)->class)
#define GET_AGE(ch)     (age(ch).year)
#define GET_STR(ch)     ((ch)->temp_stat.str)
#define GET_ADD(ch)     ((ch)->temp_stat.str_add)
#define GET_DEX(ch)     ((ch)->temp_stat.dex)
#define GET_INT(ch)     ((ch)->temp_stat.intel)
#define GET_WIS(ch)     ((ch)->temp_stat.wis)
#define GET_CON(ch)     ((ch)->temp_stat.con)
#define GET_AC(ch)      ((ch)->armor)
#define GET_HIT(ch)     ((ch)->hit)
#define GET_MAX_HIT(ch) (hit_limit(ch))
#define GET_MOVE(ch)    ((ch)->move)
#define GET_MAX_MOVE(ch)(move_limit(ch))
#define GET_MANA(ch)    ((ch)->mana)
#define GET_MAX_MANA(ch)(mana_limit(ch))
#define GET_GOLD(ch)    ((ch)->gold)
#define GET_EXP(ch)     ((ch)->exp)
#define GET_ALIGN(ch) 	((ch)->align)
#define GET_HEIGHT(ch)  ((ch)->height)
#define GET_WEIGHT(ch)  ((ch)->weight)
#define GET_SEX(ch)     ((ch)->sex)
#define GET_HITROLL(ch) ((ch)->hr)
#define GET_DAMROLL(ch) ((ch)->dr)
#define GET_REGEN(ch) 	((ch)->regen)

#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING)

#define IS_OBJ(obj,stat) (IS_SET((obj)->extra,stat))

#define OUTSIDE(ch) (!IS_SET(world[(ch)->in_room].flags,INDOORS))
#define EXIT(ch, door)  (world[(ch)->in_room].dirs[door])

#define IS_SAINT(ch)    (GET_ALIGN(ch) >= 900)
#define IS_GOOD(ch)     (GET_ALIGN(ch) >= 350)
#define IS_EVIL(ch)     (GET_ALIGN(ch) <= -350)
#define IS_DEVIL(ch)    (GET_ALIGN(ch) <= -900)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define WAIT_STATE(ch, cycle)  (((ch)->desc) ? (ch)->desc->wait = (cycle) : 0)

#endif/*__MACROS_H*/
