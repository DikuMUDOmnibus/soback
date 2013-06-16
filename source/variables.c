#include "defines.h"
#include "variables.h"
#include "weather.h"

#define __allockExternVariables__

char * WORLD_FILE        = "lib/tinyworld.wld"; /* room definitions           */
char * MOB_FILE          = "lib/tinyworld.mob"; /* monster prototypes         */
char * OBJ_FILE          = "lib/tinyworld.obj"; /* object prototypes          */
char * ZONE_FILE         = "lib/tinyworld.zon"; /* object prototypes          */
char * ZONE_DIR          = "lib/zones"; 		/* zone defs & command tables */
char * SHOP_FILE         = "lib/tinyworld.shp"; 
char * CREDITS_FILE      = "lib/credits";       /* for the 'credits' command  */
char * NEWS_FILE         = "lib/news";          /* for the 'news' command     */
char * MOTD_FILE         = "lib/motd";          /* messages of today          */
char * TODO_FILE         = "lib/todo";          /* messages of today          */
char * PLAYER_FILE       = "lib/players";       /* the player database        */
char * TIME_FILE         = "lib/time";          /* game calendar information  */
char * TYPO_FILE         = "lib/typos";         /*         'typo'             */
char * BUG_FILE          = "lib/bugs";          /*         'bug'              */
char * IDEA_FILE         = "lib/ideas";         /*         'idea'             */
char * MESS_FILE         = "lib/messages";      /* damage message             */
char * SOCMESS_FILE      = "lib/actions";       /* messgs for social acts     */
char * HELP_KWRD_FILE    = "lib/help_table";    /* for HELP <keywrd>          */
char * HELP_PAGE_FILE    = "lib/help";          /* for HELP <CR>              */
char * INFO_FILE         = "lib/info";          /* for INFO                   */
char * WIZARDS_FILE      = "lib/wizards";       /* for wizards command : cyb  */
char * COOKIE_FILE 		 = "lib/cookie";        /* for fortune cookie  : cyb  */
char * BOARD_DIR		 = "lib/boards";
char * MBOX_DIR			 = "lib/mboxes";
char * STASH_DIR		 = "lib/stash";
char * ROOM_STASH_DIR	 = "lib/rooms";
char * HELP_KEY_FILE	 = "lib/help_table";
char * PAINT_DIR		 = "lib/paints";

/*
 *	for errhandler.c
*/
int			exitWhenError 	= 0;
char *		NilString		= "";

/*
 *	for main.c
*/
int 		baddoms;
char 		baddomain[BADDOMS][32];

int         nokillflag = 0;
int         nochatflag = 0;
int         nostealflag = 0;
int         noshoutflag = 0;
int         no_specials = 0;
int         nonewplayers = 0;
int         nologin = 0;
time_t		boottime = 0;

char 		credits[MAX_STRING_LENGTH];  /* the Credits List                */ 
char 		news[MAX_STRING_LENGTH];     /* the news                        */ 
char 		motd[MAX_STRING_LENGTH];     /* the messages of today           */ 
char 		todo[MAX_STRING_LENGTH];     /* the messages of today           */ 
char 		help[MAX_STRING_LENGTH];     /* the main help page              */ 
char 		info[MAX_STRING_LENGTH];     /* the info text                   */ 
char 		wizards[MAX_STRING_LENGTH];  /* the wizards text                */ 
