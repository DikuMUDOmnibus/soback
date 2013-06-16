#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "world.h"
#include "sockets.h"
#include "find.h"
#include "limits.h"
#include "utils.h"
#include "comm.h"
#include "fight.h"
#include "stash.h"
#include "magics.h"
#include "interpreter.h"
#include "misc.h"
#include "nanny.h"
#include "variables.h"
#include "weather.h"
#include "strings.h"
#include "spec.rooms.h"
#include "affect.h"

struct player_index 	*	player_table;

charType 	*	player_list;
charType	*	char_list;

int   			top_of_p_file, top_of_p_table = -1;
int				chars_in_world;
int				players_in_world;

int create_entry( char * player )
{
	char		name[256];
  	int found = 0, j = 0;

	strcpy( name, player );
  	if (top_of_p_table == -1) 
	{
    	player_table = (struct player_index *)errMalloc( sizeof(struct player_index) );
    	top_of_p_table = 0;
  	} 
	else 
	{
    	for( j = 0; j <= top_of_p_table; j++) 
		{ 
       		if( !player_table[j].name[0] ) 
			{
         		found = 1; break;
       		}
    	}
  	}
  
  	if( found == 0 ) 
	{
   		j = ++top_of_p_table;
   		player_table = (struct player_index *)
						errRealloc(player_table, sizeof(struct player_index) * (j + 1));
  	}
  	player_table[j].name = errMalloc(strlen(name) + 1) ;

    strcpy( player_table[j].name, strlwr( name ) );
  	player_table[j].nr = j;

  	return (j);
}

void build_player_index(void)
{ 
  	int 				nr = 	-1;
  	storecharType	 	dummy;
  	FILE 			*	fp;

  	if( !(fp = errOpen(PLAYER_FILE, "rb+")) )
  	{
    	FATAL( "build player index> can't find %s file", PLAYER_FILE );
  	}

  	for(; !feof(fp);) 
	{
    	fread( &dummy, sizeof(dummy), 1, fp );
    	if( !feof(fp) )
    	{
      		if( nr == -1 ) 
			{
        		player_table = errCalloc( sizeof(struct player_index) );
        		nr = 0;
      		}  
			else 
			{
        		player_table = (struct player_index *)
						errRealloc(player_table, (++nr + 1) * sizeof(struct player_index));
      		}

        	player_table[nr].nr = nr;
        	player_table[nr].name = errMalloc(strlen(dummy.name)+1) ;

        	strcpy( player_table[nr].name, strlwr( dummy.name ) );
    	}
  	}
  	fclose(fp);

  	top_of_p_table = nr;
  	top_of_p_file = top_of_p_table;
}

void delete_char( descriptorType * desc )
{
   	log( "PLAYER \"%s\" DELETED", GET_NAME(desc->character)) ;
	senddf( 0, 41, "---Player '%s' deleted.", desc->character->name );

   	wipe_stash( GET_NAME(desc->character), 0 );
	wipe_locker( GET_NAME(desc->character) );

   	strcpy(GET_NAME(desc->character), "");
   	GET_LEVEL(desc->character) = 0 ;
   	save_char(desc->character, 0);
   	player_table[desc->pos].name = strdup("");
   	close_socket(desc);
}

void char_to_store(charType *ch,  storecharType *st)
{
  	int 			i;
  	affectType 	*	af;
  	objectType 	*	char_eq[MAX_WEAR];

  /* Unaffect everything a character can be affected by */

  	for( i = 0; i < MAX_WEAR; i++ ) 
	{
    	if( ch->equipment[i] ) 	char_eq[i] = unequip_char(ch, i);
    	else 					char_eq[i] = 0;
  	}

  	for( af = ch->affected, i = 0; i < MAX_AFFECT; i++ ) 
	{
    	if( af ) 
		{
      		st->affected[i] = *af;
      		st->affected[i].next = 0;
      		/* subtract effect of the spell or the effect will be doubled */
      		affect_modify( ch, st->affected[i].location,
                         		st->affected[i].modifier,
                         		st->affected[i].bitvector, FALSE);
      		af = af->next;
    	} 
		else 
		{
			st->affected[i].type = 0;  /* Zero signifies not used */
			st->affected[i].duration = 0;
			st->affected[i].modifier = 0;
			st->affected[i].location = 0;
			st->affected[i].bitvector = 0;
			st->affected[i].next = 0;
    	}
  	}
  	if( (i >= MAX_AFFECT) && af && af->next )
    	log("WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

    ch->temp_stat 	= ch->base_stat;

	st->played     	= ch->played;
  	st->played     += (long) (time(0) - ch->logon);
	st->birth		= ch->birth;
  	st->last_logon 	= time(0);
  	ch->played 		= st->played;
  	ch->logon 		= time(0);

  	st->weight   	= GET_WEIGHT(ch);
  	st->height   	= GET_HEIGHT(ch);
  	st->sex      	= GET_SEX(ch);
  	st->class    	= GET_CLASS(ch);
  	st->level    	= GET_LEVEL(ch);
  	st->stat 	 	= ch->base_stat;

    st->hit         = ch->hit;
    st->max_hit     = ch->max_hit;
    st->mana        = ch->mana;
    st->max_mana    = ch->max_mana;
    st->move        = ch->move;
    st->max_move    = ch->max_move;
    st->armor       = ch->armor;
    st->gold        = ch->gold;
    st->exp         = ch->exp;
    st->hr          = ch->hr;
    st->dr          = ch->dr;

  	st->align       = ch->align;
  	st->prNr 		= ch->prNr;
  	st->act         = ch->act;
  	st->bank 		= ch->bank;

  	if( GET_TITLE(ch) ) strcpy(st->title, GET_TITLE(ch));
  	else 				*st->title = '\0';

  	if( ch->description ) 	strcpy(st->description, ch->description);
  	else 					*st->description = '\0';

  	for (i = 0; i <= MAX_SKILLS - 1; i++) st->skills[i] = ch->skills[i];

  	strcpy(st->name, GET_NAME(ch) );

  	for(i = 0; i <= 2; i++) st->conditions[i] = GET_COND(ch, i);

  	for( af = ch->affected, i = 0; i<MAX_AFFECT; i++ ) 
	{
    	if( af ) 
		{
			/* Add effect of the spell or it will be lost */
			/* When saving without quitting               */
			affect_modify( ch, 	st->affected[i].location,
								st->affected[i].modifier,
								st->affected[i].bitvector, TRUE);
			af = af->next;
		}
  	}
  	for( i = 0; i < MAX_WEAR; i++ ) 
	{
    	if( char_eq[i] ) equip_char(ch, char_eq[i], i);
  	}

  	affect_total( ch );

  	st->quest[0] = ch->quest[0] ;		/* save quest bits */
  	st->quest[1] = ch->quest[1] ;

}

void init_char(charType *ch)
{
  	int 	i;

  	if(top_of_p_table < 0)
  	{
    	GET_EXP(ch) 	= 70000000;
    	GET_LEVEL(ch) 	= (IMO+3);
  	}
  	set_title(ch);

  	ch->moved 		= 0;
  	ch->roomd 		= 0;
  	ch->description = 0;

  	ch->birth 		= time(0);
  	ch->played 		= 0;
  	ch->logon 		= time(0);

  	GET_STR(ch) 	= 9;
  	GET_INT(ch) 	= 9;
  	GET_WIS(ch) 	= 9;
  	GET_DEX(ch) 	= 9;
  	GET_CON(ch) 	= 9;

	ch->affects 	= 0;
	ch->prNr 		= 0;
	ch->screen		= 22;

  	if (ch->sex == SEX_MALE) 
	{
		ch->weight = number(120,180);
		ch->height = number(160,200);
  	} else {
   		ch->weight = number(100,160);
    	ch->height = number(150,180);
  	}

	ch->mana = GET_MAX_MANA(ch);
	ch->hit = GET_MAX_HIT(ch);
	ch->move = GET_MAX_MOVE(ch);
	ch->armor = 100 - GET_LEVEL(ch);

  	for (i = 0; i <= MAX_SKILLS - 1; i++)
    	if (GET_LEVEL(ch) < (IMO+3))	ch->skills[i] = 0;
		else 		  					ch->skills[i] = 100;

	for (i = 0; i < 5; i++)	ch->saving_throw[i] = 0;
	for (i = 0; i < 3; i++)	GET_COND(ch, i) = (GET_LEVEL(ch) == (IMO+3) ? -1 : 24);
}

void reset_char( charType * ch )
{
	affectType * af;
  	int i;

  	for (i = 0; i < MAX_WEAR; i++) 	ch->equipment[i] = 0;

  	for(af = ch->affected; af; af=af->next)
    	affect_modify(ch, af->location, af->modifier, af->bitvector, 0 );

	ch->followers 		= 0;
	ch->master 			= 0;
	ch->carrying 		= 0;
	ch->next 			= 0;
	ch->next_fighting 	= 0;
	ch->next_in_room 	= 0;
	ch->fight 			= 0;
	ch->position 		= POSITION_STANDING;
	ch->defaultpos 		= POSITION_STANDING;
	ch->temp_stat 		= ch->base_stat;
	ch->carry_weight 	= 0;
	ch->carry_items  	= 0;
	ch->ndice    		= 1;
	ch->sdice  			= 2;
	ch->armor          	= 101-GET_LEVEL(ch);
  	ch->hr        		= 0;
  	ch->dr        		= 0;
  	ch->regen          	= 0;

  	for( af = ch->affected; af; af = af->next )
    	affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

	if (GET_HIT(ch)  <= 0) 		GET_HIT(ch)  = 1;
	if (GET_MOVE(ch) <= 0) 		GET_MOVE(ch) = 1;
	if (GET_MANA(ch) <= 0) 		GET_MANA(ch) = 1;

	affect_total(ch);
}

void save_char(charType *ch, int load_room)
{
   	storecharType 		st;
  	FILE 			*	fp;
	int					qSize, screen, pos = 0;
	char				pwd[15];

  	if( IS_NPC(ch) ) return;

	if( ch->desc )
	{
		pos = ch->desc->pos;
		strcpy( pwd, ch->desc->pwd );
		qSize = ch->desc->qSize;
		screen = ch->screen;
	}
	else
	{
		if( pos = load_char( ch->name, &st ), pos == -1 ) return;
		strcpy( pwd, st.pass );
		qSize = st.qSize;
		screen = st.screen;
	}

  	if( pos > top_of_p_file ) 
	{
		top_of_p_file++;
		fp = errOpen( PLAYER_FILE, "a" );
  	} 
	else
	{
		fp = errOpen( PLAYER_FILE, "r+" );
  		if( fp ) fseek(fp, pos * sizeof( storecharType), 0);
	}

  	if( !fp ) FATAL( "save_char> Openging player file failed." );

  	char_to_store( ch, &st );
  	st.load_room = load_room;
  	strcpy( st.pass, pwd );
	st.qSize = qSize;
	st.screen = screen;

  	fwrite( &st, sizeof( storecharType), 1, fp );
  	fclose( fp ); 
}   

int find_name( char * name )
{
  	int i;

  	for( i = 0; i <= top_of_p_table; i++ ) 
  	{
     	if( !stricmp( player_table[i].name, name) ) return(i);
  	}
  	return(-1);
}

/* Load a char, TRUE if loaded, FALSE if not */
int load_char( char *name,  storecharType *char_element )
{
  	FILE 	*	fp;
  	int 		player_i;

  	if( (player_i = find_name(name)) >= 0 ) 
  	{
    	if(!(fp = errOpen(PLAYER_FILE, "r"))) 
		{
      		FATAL( "load_char> Opening player file for reading." );
    	}
    	fseek(fp, (long) (player_table[player_i].nr * sizeof( storecharType)), 0);
    	fread(char_element, sizeof( storecharType), 1, fp);
    	fclose(fp);
    	return(player_i);
  	} 
  	else
    	return(-1);
}

void store_to_char( storecharType *st, charType * ch )
{
  	int 		i;

  	ch->sex   = st->sex;
  	ch->class = st->class;

  	if ( st->level >= IMO+3 && !implementor(st->name)) 	GET_LEVEL(ch) = 1 ;
  	else 												GET_LEVEL(ch) = st->level;

  	ch->moved = 0;
  	ch->roomd = 0;

  	if (*st->title) 
  	{
		ch->title = errMalloc(strlen(st->title) + 1) ;
    	strcpy(ch->title, st->title);
  	}
  	else GET_TITLE(ch) = strdup("");

  	if (*st->description) 
  	{
		ch->description = errMalloc(strlen(st->description) + 1);
    	strcpy(ch->description, st->description);
  	}
  	else ch->description = strdup("");

  	ch->birth  = st->birth;
  	ch->played = st->played;
  	ch->logon  = time(0);
  	ch->weight = st->weight;
  	ch->height = st->height;

  	if( st->stat.str_add > 40 ) st->stat.str_add = 40;

  	ch->base_stat = st->stat;
  	ch->temp_stat = st->stat;

  	ch->hit 		= st->hit;
  	ch->max_hit 	= st->max_hit;
  	ch->mana 		= st->mana;
  	ch->max_mana 	= st->max_mana;
  	ch->move 		= st->move;
  	ch->max_move 	= st->max_move;
  	ch->armor 		= st->armor;
  	ch->gold 		= st->gold;
  	ch->exp 		= st->exp;
  	ch->hr 			= st->hr;
  	ch->dr 			= st->dr;

  	ch->prNr 		= st->prNr;
  	ch->align   	= st->align;
  	ch->act     	= st->act & ( ~8 );
  	ch->bank 		= st->bank;

	GET_NAME(ch) 	= errMalloc(strlen(st->name) + 1) ;
  	strcpy(GET_NAME(ch), capitalize(st->name));

	if( ch->level < IMO+3 )
	{
  		for(i = 0; i <= MAX_SKILLS - 1; i++ ) 	
  		{
  			if( spells[i].min_level[ch->class-1] > ch->level ) ch->skills[i] = 0;
  			else 
  				ch->skills[i] = MIN( st->skills[i], spells[i].max_skill[ch->class-1] );
  		}
  	}
  	else
  	{
  		for(i = 0; i <= MAX_SKILLS - 1; i++ ) 	
  		{
  			ch->skills[i] = 100;
  		}
  	}	
  	for(i = 0; i <= 4; i++) 				ch->saving_throw[i] = 0;
  	for(i = 0; i <= 2; i++) 				GET_COND(ch, i) = st->conditions[i];

  	if( GET_CLASS( ch ) == CLASS_WARRIOR )
  		if( ch->skills[ SKILL_PARRY ] < 50 )
  			ch->skills[ SKILL_PARRY ] = 50;

  	if( GET_CLASS( ch ) == CLASS_THIEF )
  		if( ch->skills[ SKILL_PARRY ] < 95 ) 
  			ch->skills[ SKILL_PARRY ] = 95;

  	for(i=0;i<MAX_AFFECT;i++)
  	{
    	if (st->affected[i].type)
      	affect_to_char(ch, &st->affected[i]);
  	}

  	ch->in_room = st->load_room;
  	GET_HIT(ch)	=MIN(GET_HIT(ch)+(time(0)-st->last_logon)/SECS_PER_MUD_HOUR*5,GET_MAX_HIT(ch));
  	GET_MANA(ch)=MIN(GET_MANA(ch)+(time(0)-st->last_logon)/SECS_PER_MUD_HOUR*3,GET_MAX_MANA(ch));
  	GET_MOVE(ch)=MIN(GET_MOVE(ch)+(time(0)-st->last_logon)/SECS_PER_MUD_HOUR*10,GET_MAX_MOVE(ch));
	
  	ch->quest[0] = st->quest[0] ;		/* restore quest bits */
  	ch->quest[1] = st->quest[1] ;

  	ch->desc->qSize = st->qSize;
  	ch->screen = st->screen;

  	if( ch->desc->qSize < 10 || ch->desc->qSize >= MAX_OUTQUE ) ch->desc->qSize = 40;
  	if( ch->screen < 10 || ch->screen >= MAX_OUTQUE ) ch->screen = 22;
}
