#include <stdio.h>
#include <string.h>
#include <time.h>

#include "defines.h"
#include "character.h"

typedef short	sbyte;
typedef unsigned char ubyte;
typedef short	sh_int;
  
/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */ 
struct char_ability_data {  
  sbyte str;
  sbyte str_add;
  sbyte intel;
  sbyte wis;
  sbyte dex;
  sbyte con;
};
  
/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data {
  sh_int mana;
  sh_int max_mana;     /* Not useable may be erased upon player file renewal */
  int hit;
  int max_hit;      /* Max hit for NPC                         */  
  sh_int move;
  sh_int max_move;     /* Max move for NPC                        */ 
  sh_int armor;        /* Internal -100..100, external -10..10 AC */ 
  unsigned int gold;            /* Money carried                           */
  int exp;             /* The experience of the player            */
  sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
  sbyte damroll;       /* Any bonus or penalty to the damage roll */
};                                    
 
/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data {
  char learned;           /* % chance for success 0 = not learned   */
  char recognise;         /* If you can recognise the scroll etc.   */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type {
  sbyte type;           /* The type of spell that caused this      */
  sh_int duration;      /* For how long its effects will last      */
  sbyte modifier;       /* This is added to apropriate ability     */
  byte location;        /* Tells which ability to change(APPLY_XXX)*/
  long bitvector;       /* Tells which bits to set (AFF_XXX)       */
  struct affected_type *next;
};
  
struct old_save_type {
  byte sex,class,level;
  time_t birth;
  int played,life;
  ubyte weight,height;
  char title[80];
  char description[240];
  sh_int load_room;
  struct char_ability_data abilities; 
  struct char_point_data points;      
  struct char_skill_data skills[MAX_SKILLS];
  struct affected_type affected[MAX_AFFECT];
  byte spells_to_learn;
  int alignment;
  time_t last_logon;
  unsigned act;
  int bank;
  char name[20];
  char pwd[11];
  int conditions[3];
  int quest[2]; /*  reserving for future expansion */
  int reserved[17]; /*  reserving for future expansion */
  int qSize;
};
 

void save_one_char( FILE * fp, struct old_save_type * old )
{
	storecharType	new;
	int						i;

	memset( &new, 0, sizeof( new ) );

	strcpy( new.name, old->name );
	strcpy( new.pass, old->pwd  );
	strcpy( new.title, old->title );
	strcpy( new.description, old->description );
	printf( "===>%s\n", old->name );
	printf( "===>%s\n", old->title );
	printf( "===>%s\n", old->description );

	for( i = 0; i < MAX_SKILLS; i++ )
	{
		new.skills[i] = old->skills[i].learned;
	}

	for( i = 0; i < 3; i++ )
		new.conditions[i] = old->conditions[i];

	for( i = 0; i < MAX_AFFECT; i++ )
	{
		new.affected[i].location = old->affected[i].location;
		new.affected[i].duration = old->affected[i].duration;
		new.affected[i].type = old->affected[i].type;
		new.affected[i].modifier = old->affected[i].modifier;
		new.affected[i].bitvector = old->affected[i].bitvector;
	}

	new.birth = old->birth;	new.played = old->played; 
	new.life = old->life; new.last_logon = old->last_logon;

	new.sex = old->sex; new.class = old->class; new.level = old->level;

	new.hit = old->points.hit; new.max_hit = old->points.max_hit;
	new.mana = old->points.mana; new.max_mana = old->points.max_mana;
	new.move = old->points.move; new.max_move = old->points.max_move;
	new.gold = old->points.gold; new.exp = old->points.exp;
	new.dr = old->points.damroll; new.hr = old->points.hitroll;
	new.armor = old->points.armor;

	new.load_room = old->load_room;

	new.weight = old->weight;	new.height = old->height;

	new.stat.str = old->abilities.str;
	new.stat.str_add = old->abilities.str_add;
	new.stat.dex = old->abilities.dex;
	new.stat.con = old->abilities.con;
	new.stat.wis = old->abilities.wis;
	new.stat.intel = old->abilities.intel;
	new.align = old->alignment; new.prNr = old->spells_to_learn;

	new.act = old->act; new.bank = old->bank;

	new.quest[0] = old->quest[0];
	new.quest[1] = old->quest[1];

	new.qSize = old->qSize;

	fwrite( &new, sizeof( new ), 1,  fp );
}

void main( void )
{
	static struct old_save_type dummy;
	FILE   * load, * save;
	int		i;

	load = fopen( "players", "r" );
	save = fopen( "players.new", "w" );

	if( !load || !save )
	{
		perror("open files." ); exit( 1 );
	}

    for( i = 0; !feof(load); i++ )
    {
        fread( &dummy, sizeof(dummy), 1, load );    

		save_one_char( save, &dummy );
    }
    fclose( load );
    fclose( save );
}
