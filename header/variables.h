#ifndef __VARIABLES_H
#define __VARIABLES_H

#include <sys/time.h>

#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7 
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11  
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14  
#define LIQ_SUNDEW     15  
#define LIQ_NECTAR     16
#define LIQ_GOLDEN_NECTAR   17
#define LIQ_MAKOLI      18
#define LIQ_HOLY_WATER 19
#define LIQ_LAST		19

struct title_type
{
    char *  title_m;
	char *  title_f;
	int     exp;
};



struct dex_skill_type
{
  short p_pocket;
  short p_locks;
  short traps;
  short sneak;
  short hide;
};

struct dex_app_type
{
  short reaction;
  short miss_att;
  short defensive;
};

struct str_app_type
{
  short tohit;    /* To Hit (THAC0) Bonus/Penalty        */
  short todam;    /* Damage Bonus/Penalty                */
  short carry_w;  /* Maximum weight that can be carrried */
  short wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type
{
  byte bonus;       /* how many bonus skills a player can */
};

struct int_app_type
{
  byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type
{
  short hitp;
  short shock;                       
};

/* in variables.c */

extern int			beforeround;

extern int			nokillflag;
extern int			nochatflag;
extern int			nostealflag;
extern int			noshoutflag;
extern int			no_specials;
extern int 			nonewplayers;
extern int 			nologin;   
extern time_t		boottime;
extern int			shutdowngame;
extern int			autoshutdown;
extern int			stopshutdown;
extern int			manualshutdown;

#define BADDOMS	10
extern int 			baddoms;
extern char 		baddomain[BADDOMS][32];

/* in constant.c */

extern int 						rev_dir[];
extern char * 					dirs[];
extern char * 					where[];
extern int						movement_loss[];
extern char *					weekdays[7];
extern char * 					month_name[17];
extern char * 					drinknames[];

extern char *					item_types[]; 
extern char *					wear_bits[];
extern char *					extra_bits[];
extern char *					drinks[];
extern char *					color_liquid[];
extern char *					fullness[];
extern int 						drink_aff[][3];

extern char *					spell_wear_off_msg[];
            
/* for rooms */
extern char *					dirs[];
extern char *					room_bits[];           
extern char *					exit_bits[];
extern char *					sector_types[];
  
extern char *					connected_types[];

/* for chars */
extern char *					equipment_types[];     
extern char *					affected_bits[];       
extern char *					apply_types[];
extern char *					sex_types[];
extern char *					pc_class_types[];      
extern char *					npc_class_types[];     
extern char *					action_bits[];
extern char *					player_bits[];
extern char *					position_types[];      
extern char *					connected_types[];     

/* for zones */
extern char *					zone_aligns[];
extern char *					zone_aggressives[];
extern char *					zone_resets[];

extern struct title_type 		titles[4][IMO+4];

extern struct int_app_type 		int_app[];
extern struct wis_app_type 		wis_app[];
extern struct con_app_type 		con_app[];
extern struct dex_app_type 		dex_app[];
extern struct dex_skill_type 	dex_app_skill[];
extern struct str_app_type 		str_app[];

extern int 						backstab_mult[IMO+4];
extern int 						thaco[4][IMO+4];

extern char credits[MAX_STRING_LENGTH];  /* the Credits List                */
extern char news[MAX_STRING_LENGTH];     /* the news                        */
extern char motd[MAX_STRING_LENGTH];     /* the messages of today           */
extern char todo[MAX_STRING_LENGTH];     /* the messages of today           */
extern char help[MAX_STRING_LENGTH];     /* the main help page              */
extern char info[MAX_STRING_LENGTH];     /* the info text                   */
extern char wizards[MAX_STRING_LENGTH];  /* the wizards text                */

#endif/*__VARIABLES_H*/
