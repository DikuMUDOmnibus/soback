#include <stdio.h>
#include <time.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "magics.h"
#include "allocate.h"
#include "strings.h"
#include "utils.h"
#include "find.h"
#include "affect.h"

#define LAST_MOB_INDEX	99999

struct  mobile_index *	mobiles;
int						mobs_in_world;
charType 			 *	mob_list;

static void read_a_mobile( FILE * fp, mobIndexType * mi, int real, int virtual )
{
	char		ch;
	
	mi->virtual = virtual;	mi->nr = real;

    mi->name  = fread_string(fp);
    mi->moved = fread_string(fp);
    mi->roomd = fread_string(fp);

    mi->description = fread_string(fp);

	mi->act      = fread_number( fp ); mi->act |= 8;
	mi->affected = fread_number( fp );
	REMOVE_BIT( mi->affected, AFF_BERSERK );
	REMOVE_BIT( mi->affected, AFF_TIME_STOP );
	REMOVE_BIT( mi->affected, AFF_MORPETH );
	REMOVE_BIT( mi->affected, AFF_AMBUSH );
	REMOVE_BIT( mi->affected, AFF_FEAR );
    fscanf(fp, " %d ",  &mi->align );
    fscanf(fp, " %c \n", &ch);

    if( ch == 'S')
    {
        fscanf(fp, " %d ", &mi->level );
        fscanf(fp, " %d ", &mi->hr );  mi->hr  = 20 - mi->hr;
        fscanf(fp, " %d ", &mi->ac );  mi->ac *= 10;      
        fscanf(fp, " %dd%d+%d ", &mi->hndice, &mi->hsdice, &mi->hbase );
        fscanf(fp, " %dd%d+%d \n", &mi->ndice, &mi->sdice, &mi->dr );

        fscanf(fp, " %ld ", &mi->gold );
        fscanf(fp, " %ld \n", &mi->exp ); 
        fscanf(fp, " %d ", &mi->position );
        fscanf(fp, " %d ", &mi->defaultpos );
        fscanf(fp, " %d \n", &mi->sex );
    }
    else
    {
        FATAL( "read_a_mobile> mobile of Old format data encountered. vNr = %d", virtual );
    }
}

static void write_a_mobile( FILE * fp, mobIndexType * mi )
{
	fprintf( fp, "#%d\n", mi->virtual );
	fwrite_string( fp, mi->name );
	fwrite_string( fp, mi->moved );
	fwrite_string( fp, mi->roomd );
	fwrite_string( fp, mi->description );
	fprintf( fp, "%ld %ld %d S\n", mi->act, mi->affected, mi->align );

	fprintf( fp, "%d %d %d %dd%d+%d %dd%d+%d\n", 
					mi->level, 20 - mi->hr, mi->ac / 10,
					mi->hndice, mi->hsdice, mi->hbase,
					mi->ndice, mi->sdice, mi->dr );

	fprintf( fp, "%ld %ld\n", mi->gold, mi->exp );
	fprintf( fp, "%d %d %d\n", mi->position, mi->defaultpos, mi->sex );
}

void write_mobiles( FILE * fp, int zone, int bottom, int upper )
{
	int				i;

	fprintf( fp, "#MOBILES\n" );

	for( i = MOBILE_INTERNAL + 1; i < mob_index_info.used; i++ )
	{
		if( mobiles[i].virtual >= bottom ) break;
	}

	if( i < 1 ) i = 1;

	if( i != mob_index_info.used )
	{
		for( ; mobiles[i].virtual <= upper && mobiles[i].virtual >= bottom; i++ )
		{
			write_a_mobile( fp, &mobiles[i] );
		}
	}

	fprintf( fp, "#%d\n\n", LAST_MOB_INDEX );
}

void boot_mobiles( FILE * fp )
{
	char			buf[MAX_STRING_LENGTH + 1];
	int				nr;
	int				oldscan, scan;

   	do
   	{
   		fgets( buf, MAX_STRING_LENGTH, fp );

	} while( buf[0] != '#' );

    for( scan = -1, nr = mob_index_info.used;; nr++ )
    {
        fgets( buf, MAX_STRING_LENGTH - 1, fp );

        oldscan = scan;
        sscanf( buf, "#%d", &scan );

        if( oldscan >= scan )
        {
            FATAL( "boot_mobiles> Numbering mobile index corrupted. #%d -> #%d", oldscan, scan );
        }                                         
   
        if( scan >= LAST_MOB_INDEX || buf[0] == '$' ) break;
   
        if( buf[0] == '#' )
        {
			if( nr >= mob_index_info.max ) FATAL( "boot_mobiles> mob index overflows." );

			read_a_mobile( fp, &mobiles[nr], nr, scan );
        }
        else
            FATAL( "MobileTable> error in mobile file at (virtualNr) #%ld.", scan );

		mob_index_info.used++;
    }
	fscanf( fp, "\n" );
}

int real_mobileNr( int vNr )
{
	int		top = mob_index_info.used - 1;
	int		mid;
	int		bot = 0;
	int		rval;

    while( 1 )
    {
        mid = bot + (top - bot) / 2;
   
        if( rval = vNr - mobiles[ mid ].virtual, !rval ) return mid;

        if( bot == mid && top == mid ) 
		{
			DEBUG( "real_mobileNr> #%d mobile not found in mobile table.", vNr );
			return MOBILE_NULL;
		}
   
        if( rval < 0 )
        {
            if( top == mid ) mid--;
            top = mid; continue;
        }
        else
        {
            if( bot == mid ) mid++;
            bot = mid; continue;
        }
    }
}

charType * load_a_mobile( int nr, int isVirtual )
{
	charType 		*	mob;
	mobIndexType	*	mi;
	int					i;

	mob = alloc_char();

	if( !isVirtual ) 
	{
		if( nr <= MOBILE_NULL || nr >= mob_index_info.used )
		{
			FATAL( "load_a_mobile> real number %d is out of range.", nr );
		}
		mi  = &mobiles[nr];
	}
	else		 	 
	{
		if( i = real_mobileNr( nr ), i == NIL )
		{
			FATAL( "load_a_mobile> #%d index is out of range.", nr );
		}
		mi  = &mobiles[ i ];
	}

  	mob->nr = mi->nr;

	mob->name  		 	= mi->name;
	mob->moved		 	= mi->moved;
	mob->roomd		 	= mi->roomd;
	mob->description 	= mi->description;

    mob->max_hit  = mob->hit  = dice(mi->hsdice, mi->hndice) + mi->hbase;
    mob->max_mana = mob->mana = 100 + (mob->hit/10); 
	mob->max_move = mob->move =  80 + (mob->hit/8);

	mob->gold	= mi->gold;
	mob->exp    = mi->exp;
	mob->level  = mi->level;
	mob->align	= mi->align;
	mob->armor	= mi->ac;
	mob->hr		= mi->hr;
	mob->dr		= mi->dr;
	mob->ndice	= mi->ndice;
	mob->sdice  = mi->sdice;
	mob->sex	= mi->sex;
	mob->act	= mi->act;

	mob->affects    = mi->affected;
	mob->position 	= mi->position;
	mob->defaultpos = mi->defaultpos;

	mob->base_stat.str 		= 18;
	mob->base_stat.str_add 	= 0;
	mob->base_stat.dex 		= 18;
	mob->base_stat.wis 		= 18;
	mob->base_stat.intel	= 18;
	mob->base_stat.con 		= 18;
  	mob->regen 				= 0;

	mob->temp_stat = mob->base_stat;

    mob->weight = 200;
    mob->height = 198;
	mob->birth  = time(0);
	mob->logon  = time(0);

    for( i = 0; i < MAX_CONDITIONS; i++) mob->conditions[i] 	 = -1;
    for( i = 0; i < MAX_SAVINGS; i++) 	 mob->saving_throw[i] = 0;

  	if(  IS_SET( mob->act,ACT_FIGHTER ) 
      || IS_SET( mob->act,ACT_SPITTER ) 
      || IS_SET( mob->act,ACT_PALADIN ) )
	{      
    	mob->skills[SKILL_KICK] = 95;
    	mob->skills[SKILL_BASH] = 95;
    	mob->skills[SKILL_MULTI_KICK] = 95;  
    	mob->skills[SKILL_PUNCH] = 95;  
    	mob->skills[SKILL_CHAIN_PUNCH] = 95;  
  	}
  	if( IS_SET(mob->act,ACT_THIEF)) 
	{      
    	mob->skills[SKILL_PARRY] = 95;
    	mob->skills[SKILL_FLASH] = 95;       
    	mob->base_stat.dex = 25;
  	}
  	if( IS_SET(mob->act,ACT_PALADIN))
	{     
    	mob->skills[SKILL_PARRY] = 60;
    	mob->base_stat.str_add = 20;
  	}

  	if( IS_SET(mob->act,ACT_FIGHTER) )
	{     
    	mob->skills[SKILL_PARRY] = 30;
    	mob->base_stat.str_add = 30;
  	}
  	if( IS_SET(mob->act,ACT_CLERIC) || IS_SET(mob->act, ACT_HELPER) )
  	{
    	mob->base_stat.str_add = 50;
    	mob->base_stat.wis = 25;
    }
  	if(IS_SET(mob->act,ACT_MAGE ) )       
  	{
    	mob->base_stat.intel = 25;
    }

  	mob->next = char_list;
  	char_list = mob;

  	mob->next_char = mob_list;
  	mob_list = mob;

	mob->in_room = NOWHERE;
	mob->was_in_room = NOWHERE;

  	mobiles[nr].in_world++;
  	mobs_in_world++;
  	chars_in_world++;

	return mob;
}

void free_a_char( charType * ch )
{
	affectType		*	af;
	roundAffType	*	rf, * rf_next;

	if( IS_NPC(ch) )
	{
		if( ch->nr < 0 || ch->nr >= mob_index_info.used )
		{
			DEBUG( "free_a_mobile> requested mob has a wrong index(%d vs max %d).",
					ch->nr, mob_index_info.used );
			return;
		}
		if( ch->name 		 != mobiles[ch->nr].name  ) 		errFree( ch->name );
		if( ch->roomd 		 != mobiles[ch->nr].roomd ) 		errFree( ch->roomd );
		if( ch->moved 		 != mobiles[ch->nr].moved ) 		errFree( ch->moved );
		if( ch->description  != mobiles[ch->nr].description ) 	errFree( ch->description );

		mobiles[ch->nr].in_world--;
	}
	else if( ch->nr != -3 )
	{
		if( ch->name )			errFree( ch->name );
		if( ch->title ) 		errFree( ch->title );
		if( ch->description ) 	errFree( ch->description );
	}
	else
	{
		log( "free_a_char> player char nr == -3 called." );
	}

 	for( af = ch->affected;  af; af = af->next ) affect_remove( ch, af ); 
 	for( rf = ch->roundAffs; rf; rf = rf_next )
 	{
 		rf_next = rf->my_next;
 		roundAff_remove( rf ); 
 	}

	free_char( ch );
}
