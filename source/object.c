#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "object.h"
#include "mobile.h"
#include "comm.h"
#include "utils.h"
#include "find.h"
#include "allocate.h"
#include "strings.h"
#include "transfer.h"
#include "affect.h"

#define LAST_OBJ_INDEX	99999

objIndexType *	objects  = 0;
objectType	 *	obj_list = 0;
int				objs_in_world = 0;
int				objs_off_world = 0;

void read_a_object( FILE * fp, objIndexType * oi, int rNr, int vNr, char * buf )
{
	int					i;
	int					loc, mod;
	exdescriptionType *	new_exd;

    oi->nr = rNr, oi->virtual = vNr;
   
    oi->name  = fread_string(fp);
    oi->wornd = fread_string(fp);
    oi->roomd = fread_string(fp);
    oi->usedd = fread_string(fp);

	oi->type  = fread_number( fp );
	oi->extra = fread_number( fp );
	oi->wear  = fread_number( fp );

    	fscanf(fp, " %d %d %d %d \n", &oi->value[0], &oi->value[1], &oi->value[2], &oi->value[3]);
	i = fscanf(fp, " %d %d %d %d \n", &oi->weight, &oi->cost, &oi->level, &oi->magic );

	if( i < 4 ) oi->magic = 10;
	if( oi->level > 40 ) oi->level = 1;

    while( fscanf(fp, " %s \n", buf), buf[0] == 'E')  
    {
    	new_exd = (exdescriptionType *) errMalloc( sizeof( exdescriptionType ) );

        new_exd->keyword  	 = fread_string(fp);
        new_exd->description = fread_string(fp);
        new_exd->next 	  	 = oi->extrd;
        oi->extrd     	  	 = new_exd;
    }

    for( i = 0 ; (i < MAX_APPLY) && (buf[0] == 'A') ; i++)
    {
        fscanf(fp, " %d %d \n", &loc, &mod );  
		oi->apply[i].location = loc; oi->apply[i].modifier = mod;
        fscanf(fp, " %s \n", buf);
    }
   
    for( ; i < MAX_APPLY; i++ )
    {
        oi->apply[i].location = APPLY_NONE;
        oi->apply[i].modifier = 0;
    }

	oi->in_world = 0;
}

void write_extrd( FILE * fp, exdescriptionType * exd )
{
	exdescriptionType 	*	table[20];
	int						i;


	for( i = 0; exd; i++, exd = exd->next ) table[i] = exd;

	for( i--; i >= 0; i-- )
	{
		fprintf( fp, "E\n" );
		fwrite_string( fp, table[i]->keyword );
		fwrite_string( fp, table[i]->description );
	}
}

static void write_a_object( FILE * fp, objIndexType * oi )
{
	int					i;

	fprintf( fp, "#%d\n", oi->virtual );
	fwrite_string( fp, oi->name );
	fwrite_string( fp, oi->wornd );
	fwrite_string( fp, oi->roomd );
	fwrite_string( fp, oi->usedd );
	fprintf( fp, "%d %d %d\n", (int)oi->type, oi->extra, oi->wear );
	fprintf( fp, "%d %d %d %d\n", oi->value[0], oi->value[1], oi->value[2], oi->value[3] );
	fprintf( fp, "%d %d %d %d\n", oi->weight, oi->cost, oi->level, oi->magic );

	if( oi->extrd ) write_extrd( fp, oi->extrd );

	for( i = 0; i < MAX_APPLY; i++ )
	{
		if( oi->apply[i].location != APPLY_NONE )
		{
			fprintf( fp, "A\n" );
			fprintf( fp, "%d %d\n", oi->apply[i].location, oi->apply[i].modifier );
		}	
	}
}

void write_objects( FILE * fp, int zone, int bottom, int upper )
{   
    int             i;
        
    fprintf( fp, "#OBJECTS\n" );
        
    for( i = 0; i < obj_index_info.used; i++ )
    {   
        if( objects[i].virtual >= bottom ) break;
    }

	if( i <= OBJECT_INTERNAL ) i = OBJECT_INTERNAL + 1;
        
    if( i != obj_index_info.used )
    {   
        for( ; objects[i].virtual <= upper && objects[i].virtual >= bottom; i++ )
        {
            write_a_object( fp, &objects[i] );
        }
    }   
    
    fprintf( fp, "#%d\n\n", LAST_OBJ_INDEX );
}

void boot_objects( FILE * fp )
{
    int         scan, oldscan;
    int         nr;
    char        buf[MAX_STRING_LENGTH + 1];
   
   	do
   	{
   		fgets( buf, MAX_STRING_LENGTH, fp );

   	} while( buf[0] != '#' );

   	fgets( buf, MAX_STRING_LENGTH, fp );

    for( scan = -1, nr = obj_index_info.used ;; nr++ )
    {
		oldscan = scan;

        sscanf( buf, "#%d", &scan );

        if( oldscan >= scan )
        {
            FATAL( "boot_objects> Numbering object index corrupted. #%d -> #%d", oldscan, scan );
        }

        if( scan == LAST_OBJ_INDEX || buf[0] == '$' ) break;

        if( buf[0] == '#' )
        {
			if( nr >= obj_index_info.max ) FATAL( "boot_objects> obj index overflows." );
 
			read_a_object( fp, &objects[nr], nr, scan, buf );
        }
        else
            FATAL( "boot_objects> error in item next to (virtualNr) #%d.", oldscan );

		obj_index_info.used++;
    }
	fscanf( fp, "\n" );
}

int real_objectNr( int vNr )         
{   
    int     top = obj_index_info.used - 1;
    int     mid;
    int     bot = 0;
    int     rval;
    
    while( 1 )
    {
        mid = bot + (top - bot) / 2;
    
        if( rval = vNr - objects[ mid ].virtual, !rval ) return mid; 

        if( bot == mid && top == mid ) 
		{
			DEBUG( "real_objectNr> #%d object not found in objects table.", vNr );
			return OBJECT_NULL;
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

objectType * load_a_object( int nr, int isVirtual, int in_world )
{
	int					i;
	objIndexType 	*	oi;
	objectType 		*	obj;

	if( !isVirtual ) 
	{
		if( nr <= OBJECT_INTERNAL || nr >= obj_index_info.used )
		{
			FATAL( "load_a_object> real number %d out of range.", nr );
		}
		oi = &objects[nr];
	}
	else			
	{
		if( i = real_objectNr( nr ), i == NIL ) 		
		{
			FATAL( "load_a_object> #%d index out of range.", nr );
		}
		oi = &objects[ i ]; 
	}

	obj = alloc_object();

	obj->nr		= oi->nr;
	obj->virtual= oi->virtual;

	obj->name	= oi->name;
	obj->wornd  = oi->wornd;
	obj->roomd  = oi->roomd;
	obj->usedd  = oi->usedd;
	obj->extrd  = oi->extrd;

	obj->type   = oi->type;
	obj->wear   = oi->wear;
	obj->extra  = oi->extra;

	obj->cost	= oi->cost;
	obj->weight = oi->weight;

	if( obj->type == ITEM_DRINKCON )
	{
		obj->weight += obj->value[1];
	}

	if( obj->type == ITEM_TRAP )
	{
		obj->timer  = obj->value[0];
	}
	else
	{
		obj->timer  = MAX_OBJ_LIFE_SPAN;
	}

	obj->status = 100;
	obj->limit  = 100;
	obj->level  = oi->level;
	obj->magic  = oi->magic;

	for( i = 0; i < 4; i++ ) 			obj->value[i] = oi->value[i];
	for( i = 0; i < MAX_APPLY; i++ ) 	obj->apply[i] = oi->apply[i];

	obj->in_room = NOWHERE;

	obj->next    = obj_list;
	obj_list     = obj;

	if( in_world )
	{
		oi->in_world++; 	objs_in_world++;
	}
	else
	{
		oi->off_world++; 	objs_off_world++;
	}

	return obj;
}

static void destroy_object( objectType * obj, int in_world )
{
	objectType		* 	curr;

	if( obj->nr < 0 || obj->nr >= obj_index_info.used )
	{
		DEBUG( "destroy_object> obj nr (%d) is out of range( %d ).", obj->nr, obj_index_info.used );
		return;
	}		

	if( obj_list == obj ) obj_list = obj->next;
	else
	{
	  	for( curr = obj_list; curr && (curr->next != obj); curr = curr->next )
		;
	  	if( curr )	curr->next = obj->next;
		else
		{
			DEBUG( "destroy_object> object (%s) not found in obj_list.", obj->name );
		}
  	}

   	if( obj->name  != objects[obj->nr].name ) 	errFree(obj->name);
   	if( obj->wornd != objects[obj->nr].wornd ) 	errFree(obj->wornd);
  	if( obj->usedd != objects[obj->nr].usedd ) 	errFree(obj->usedd);
   	if( obj->roomd != objects[obj->nr].roomd ) 	errFree(obj->roomd);

	if( in_world )
	{
		objs_in_world--; 	objects[obj->nr].in_world--;
	}
	else
	{
		objs_off_world--; 	objects[obj->nr].off_world--;
	}
	free_object( obj );
}

void extract_obj( objectType * obj, int in_world )
{
	objectType 	*	container, * curr;

	if( obj->in_room != NOWHERE ) obj_from_room(obj);
	else if( obj->carried_by )    obj_from_char(obj);
	else if( obj->in_obj )
	{
	  	container = obj->in_obj;
	  	if( container->contains == obj )   /* head of list */
			container->contains = obj->next_content;
	  	else
	  	{
			for( curr = container->contains ; curr && (curr->next_content != obj);
		  												curr = curr->next_content )
			;
			if(curr) curr->next_content = obj->next_content; 
			else
			{
				DEBUG("extract_obj> obj(%s) is in %s?? not found.", obj->name, obj->in_obj->name );	
			}
	  	}
	}

	for( ; obj->contains; extract_obj(obj->contains, in_world ) )
	; 

  	destroy_object( obj, in_world );
}

void wipe_obj_list( objectType * obj )
{
  	if( !obj ) return;

   	wipe_obj_list( obj->contains );
   	wipe_obj_list( obj->next_content );

   	if( obj->in_obj ) obj_from_obj(obj);

   	extract_obj( obj , 1 );
} 

#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 40

void make_p_corpse( charType * ch, char * name, int wearbits, int major )
{
 	objectType * 	corpse, *o;
 	objectType * 	money,  * otmp; 
 	objectType * 	next_obj, *tmp_obj; 
 	char 			buf[MAX_STRING_LENGTH];
 	int 			i, tmp, b_num = 0;

	corpse = alloc_object();
 
 	sprintf(buf, "%s corpse %s", (IS_NPC(ch) ? ch->moved : GET_NAME(ch)), name );
 	corpse->name  = strdup(buf);

 	sprintf(buf, "%s of %s is lying here.", name, (IS_NPC(ch) ? ch->moved : GET_NAME(ch)));
 	corpse->roomd = strdup(buf);

 	sprintf(buf, "%s of %s", name, (IS_NPC(ch) ? ch->moved : GET_NAME(ch)));
 	corpse->wornd = strdup(buf);

	corpse->nr		 = OBJECT_CORPSE;
 	corpse->in_room  = NOWHERE;
 	corpse->type  	 = ITEM_CONTAINER;
 	corpse->wear  	 = ITEM_TAKE;
 	corpse->value[0] = 0; /* You can't store stuff in a corpse */
 	corpse->weight   = ch->weight + ch->carry_weight;
 	corpse->level	 = ch->level;
 	corpse->magic	 = ch->class;

 	if( IS_NPC( ch ) ) corpse->cost = -1;
 	else			   corpse->cost	= ch->nr;

 	if (IS_NPC(ch)) corpse->timer = MAX_NPC_CORPSE_TIME;
 	else 			corpse->timer = MAX_PC_CORPSE_TIME;

 	if( major )	
 	{
 		corpse->contains = ch->carrying;
 		ch->carrying 	 = 0;
 		ch->carry_weight = 0;
 		ch->carry_items  = 0;

 		if( GET_GOLD(ch) > 0 ) 
 		{
  			money = create_money(GET_GOLD(ch));
  			obj_to_obj( money, corpse );
 		}

		GET_GOLD(ch) = 0;
 	}

 	for( i = 0; i < MAX_WEAR; i++ ) 
 	{
  		if( ch->equipment[i] && ((1 << i) & wearbits) )
  		{
   			otmp = unequip_char(ch,i) ;

   			if( !IS_NPC(ch) ) 
   			{
     			tmp = number(10, 25);
     			if( IS_OBJ(otmp, ITEM_GLOW) ) tmp -= 10 ;  /* harder to break */
     			if( IS_OBJ(otmp, ITEM_HUM)  ) tmp += 9 ;    /* easier to break */

	 			tmp = (otmp->status -= tmp);

     			if( tmp <= 0 ) 
	 			{
       				b_num++;
       				act( "The $o is broken by bitter death !!!.", TRUE, ch, otmp, 0, TO_ROOM);
       				log( "Lost item(killed): %s", otmp->wornd );
       				senddf( 0, 42, "===> Lost item : %s", otmp->wornd);
       				if( otmp->type == ITEM_CONTAINER ) 
	   				{
          				act("As $o is smashed, some items dumped.", 0,ch,otmp,0,TO_ROOM) ;
          				for( tmp_obj = otmp->contains ; tmp_obj ; tmp_obj = next_obj ) 
		  				{
							next_obj = tmp_obj->next_content ;
							obj_from_obj(tmp_obj) ;
							obj_to_room(tmp_obj, ch->in_room) ;
            			}
          			}
       				extract_obj( otmp, 1 ) ;
       			}
     			else /* not broken */
       				obj_to_obj(otmp, corpse);
     		}
   			else 
   			{
     			obj_to_obj(otmp, corpse);
     		}
   		}
  	}

	for(o = corpse->contains; o; o->in_obj = corpse, o = o->next_content);
 	obj_list_new_owner( corpse, 0 );

 	corpse->next = obj_list;
 	obj_list = corpse;

 	obj_to_room(corpse, ch->in_room);

	objects[corpse->nr].in_world++;
	objs_in_world++;
}

void make_d_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_LIGHT) 
         & (1 << WEAR_FINGER_L)
         & (1 << WEAR_KNEE_L)
         & (1 << WEAR_WRIST_L)
         & (1 << WEAR_NECK_1)
         & (1 << WEAR_BODY)
         & (1 << WEAR_HEAD)
         & (1 << WEAR_LEGS)
         & (1 << WEAR_FEET)
		 & (1 << WEAR_HANDS)
		 & (1 << WEAR_ARMS)
		 & (1 << WEAR_SHIELD)
		 & (1 << WEAR_ABOUT)
		 & (1 << WEAR_WAISTE)
		 & (1 << WIELD)
		 & (1 << HOLD)
		 & (1 << WEAR_ABOUTLEGS);

	make_p_corpse( ch, "A disembodied left body", part, 0 );
	make_p_corpse( ch, "A disembodied right body", ~part, 1 );
}

void make_c_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_LIGHT) 
         & (1 << WEAR_FINGER_L)
         & (1 << WEAR_FINGER_R)
         & (1 << WEAR_WRIST_L)
         & (1 << WEAR_NECK_1)
         & (1 << WEAR_BODY)
         & (1 << WEAR_HEAD)
		 & (1 << WEAR_HANDS)
		 & (1 << WEAR_ARMS)
		 & (1 << WEAR_SHIELD)
		 & (1 << WEAR_ABOUT)
		 & (1 << WEAR_WAISTE)
		 & (1 << WIELD)
		 & (1 << HOLD);

	make_p_corpse( ch, "A disembodied upper body", part, 0 );
	make_p_corpse( ch, "A disembodied lower body", ~part, 1 );
}

void make_t_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_HEAD);

	make_p_corpse( ch, "A disembodied head", part, 0 );
	make_p_corpse( ch, "A headless body", ~part, 1 );
}

void make_nt_corpse( charType * ch, int level )
{
	objectType	*	obj;
	int				part;

	part = (1 << WEAR_HEAD);

	if( ch->equipment[WEAR_HEAD] )
	{
		obj = unequip_char( ch, WEAR_HEAD );
		obj_to_room( obj, ch->in_room );
	}
		
	make_p_corpse( ch, "A headless body", ~part, 1 );
}

void make_l_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_LEGS)
		 & (1 << WEAR_ABOUTLEGS)
         & (1 << WEAR_KNEE_L)
         & (1 << WEAR_KNEE_R);

	make_p_corpse( ch, "A pair of disembodied legs", part, 0 );
	make_p_corpse( ch, "A disembodied body", ~part, 1 );
}

void make_h_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_FINGER_L)
		 & (1 << WEAR_FINGER_R)
		 & (1 << WEAR_HANDS)
		 & (1 << WEAR_ARMS)
		 & (1 << WIELD)
		 & (1 << HOLD);

	make_p_corpse( ch, "A pair of disembodied hands", part, 0 );
	make_p_corpse( ch, "A disembodied body", ~part, 1 );
}

void make_lf_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_FINGER_L);

	make_p_corpse( ch, "A disembodied left finger", part, 0 );
	make_p_corpse( ch, "A disembodied body", ~part, 1 );
}

void make_rf_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_FINGER_R);

	make_p_corpse( ch, "A disembodied right finger", part, 0 );
	make_p_corpse( ch, "A disembodied body", ~part, 1 );
}

void make_ll_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_KNEE_L);

	make_p_corpse( ch, "A pair of disembodied hands", part, 0 );
	make_p_corpse( ch, "A disembodied body", ~part, 1 );
}

void make_rl_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_KNEE_R);

	make_p_corpse( ch, "A pair of disembodied hands", part, 0 );
	make_p_corpse( ch, "A disembodied body", ~part, 1 );
}

void make_lh_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_WRIST_L)
		 & (1 << WEAR_FINGER_L)
		 & (1 << HOLD);

	make_p_corpse( ch, "A pair of disembodied hands", part, 0 );
	make_p_corpse( ch, "A disembodied body", ~part, 1 );
}

void make_rh_corpse( charType * ch, int level )
{
	int		part;

	part = (1 << WEAR_WRIST_R)
		 & (1 << WEAR_FINGER_R)
		 & (1 << WIELD);

	make_p_corpse( ch, "A pair of disembodied hands", part, 0 );
	make_p_corpse( ch, "A disembodied body", ~part, 1 );
}

void make_vh_corpse( charType * ch, int level )
{
	int		part;

	part = -1;

	make_p_corpse( ch, "A disemboweled body", -1, 1 );	
	make_p_corpse( ch, "Some disemboweled organs", 0, 0 );
}

void make_a_corpse( charType *ch, int level )
{
 	objectType * 	corpse, *o;
 	objectType * 	money,  * otmp; 
 	objectType * 	next_obj, *tmp_obj; 
 	char 			buf[MAX_STRING_LENGTH];
 	int 			i, tmp, b_num = 0;

	corpse = alloc_object();
 
 	sprintf(buf,"corpse %s", (IS_NPC(ch) ? ch->moved : GET_NAME(ch)));
 	corpse->name  = strdup(buf);

 	sprintf(buf, "Corpse of %s is lying here.", (IS_NPC(ch) ? ch->moved : GET_NAME(ch)));
 	corpse->roomd = strdup(buf);

 	sprintf(buf, "Corpse of %s", (IS_NPC(ch) ? ch->moved : GET_NAME(ch)));
 	corpse->wornd = strdup(buf);

	corpse->nr		 = OBJECT_CORPSE;
 	corpse->in_room  = NOWHERE;
 	corpse->type  	 = ITEM_CONTAINER;
 	corpse->wear  	 = ITEM_TAKE;
 	corpse->value[0] = 0; /* You can't store stuff in a corpse */
 	corpse->weight   = ch->weight + ch->carry_weight;
 	corpse->contains = ch->carrying;
 	corpse->level	 = ch->level;
 	corpse->magic	 = ch->class;
 	if( IS_NPC( ch ) ) corpse->cost = -1;
 	else			   corpse->cost	= ch->nr;

 	if( GET_GOLD(ch) > 0 ) 
 	{
  		money = create_money(GET_GOLD(ch));
  		obj_to_obj( money,corpse );
 	}

	GET_GOLD(ch)=0;

 	if (IS_NPC(ch)) corpse->timer = MAX_NPC_CORPSE_TIME;
 	else 			corpse->timer = MAX_PC_CORPSE_TIME;

 	for( i = 0; i < MAX_WEAR; i++ ) 
 	{
  		if( ch->equipment[i] ) 
  		{
   			otmp = unequip_char(ch,i) ;
   			if( !IS_NPC(ch) ) 
   			{
     			tmp = number(10, 25);
     			if( IS_OBJ(otmp, ITEM_GLOW) ) tmp -= 10 ;  /* harder to break */
     			if( IS_OBJ(otmp, ITEM_HUM)  ) tmp += 9 ;    /* easier to break */

	 			tmp = (otmp->status -= tmp);

     			if( tmp <= 0 ) 
	 			{
       				b_num++;
       				act( "The $o is broken by bitter death !!!.", TRUE, ch, otmp, 0, TO_ROOM);
       				log( "Lost item(killed): %s", otmp->wornd );
       				senddf( 0, 42, "===> Lost item : %s", otmp->wornd);
       				if( otmp->type == ITEM_CONTAINER ) 
	   				{
          				act("As $o is smashed, some items dumped.", 0,ch,otmp,0,TO_ROOM) ;
          				for( tmp_obj = otmp->contains ; tmp_obj ; tmp_obj = next_obj ) 
		  				{
							next_obj = tmp_obj->next_content ;
							obj_from_obj(tmp_obj) ;
							obj_to_room(tmp_obj, ch->in_room) ;
            			}
          			}
       				extract_obj( otmp, 1 ) ;
       			}
     			else /* not broken */
       				obj_to_obj(otmp, corpse);
     		}
   			else 
   			{
     			obj_to_obj(otmp, corpse);
     		}
   		}
  	}

 	ch->carrying 	 = 0;
 	ch->carry_weight = 0;
 	ch->carry_items  = 0;

 	corpse->next = obj_list;
 	obj_list = corpse;

 	for(o = corpse->contains; o; o->in_obj = corpse, o = o->next_content);

 	obj_list_new_owner( corpse, 0 );
 	obj_to_room(corpse, ch->in_room);

	objects[corpse->nr].in_world++;
	objs_in_world++;
}

objectType * create_money( int amount )
{
  	objectType 			*	obj;
  	exdescriptionType 	*	new_descr;
  	char 					buf[80];

  	if( amount < 0 )
  	{
    	FATAL( "create_money> Try to create %d money.", amount );
  	}

  	obj = alloc_object();

 	new_descr=(exdescriptionType*)errCalloc(sizeof(exdescriptionType)); 

  	if( amount == 1 )
  	{
    	obj->name  = strdup("coin gold");
    	obj->wornd = strdup("a gold coin");
    	obj->roomd = strdup("One miserable gold coin.");

    	new_descr->keyword = strdup("coin gold");
    	new_descr->description = strdup("One miserable gold coin.");
  	}
  	else
  	{
    	obj->name  = strdup("coins gold");
    	obj->wornd = strdup("gold coins");
    	obj->roomd = strdup("A pile of gold coins.");

    	new_descr->keyword = strdup("coins gold");
    	if( amount < 10 ) 
		{
      		sprintf(buf,"There is %d coins.",amount);
      		new_descr->description = strdup(buf);
    	} 
    	else if( amount < 100 ) 
		{
      		sprintf(buf,"There is about %d coins",10*(amount/10));
      		new_descr->description = strdup(buf);
    	}
    	else if( amount < 1000 ) 
		{
      		sprintf(buf,"It looks like something round %d coins",100*(amount/100));
      		new_descr->description = strdup(buf);
    	}
    	else if( amount < 100000 ) 
		{
      		sprintf(buf,"You guess there is %d coins",1000*((amount/1000)+ number(0,(amount/1000))));
      		new_descr->description = strdup(buf);
    	}
    	else 
      		new_descr->description = strdup("There is A LOT of coins");      
  	}

  	new_descr->next = 0;
  	obj->extrd = new_descr;

  	obj->nr   	  = OBJECT_COINS;
  	obj->in_room  = NOWHERE;
  	obj->type 	  = ITEM_MONEY;
  	obj->wear 	  = ITEM_TAKE;
  	obj->value[0] = amount;
  	obj->cost     = amount;

  	obj->next = obj_list;
  	obj_list  = obj;

	objects[obj->nr].in_world++;
	objs_in_world++;

  	return(obj);
}

objectType * create_food( void )
{
	objectType	*	obj;

	obj = alloc_object();

	obj->nr 		= OBJECT_FOOD;
	obj->in_room 	= NOWHERE;

    obj->name  		= strdup("mushroom");
    obj->wornd 		= strdup("A Magic Mushroom");
    obj->roomd 		= strdup("A really delicious looking magic mushroom lies here.");

    obj->type 		= ITEM_FOOD;
    obj->wear 		= ITEM_TAKE | ITEM_HOLD;
    obj->value[0] 	= 5;
    obj->weight 	= 1;
    obj->cost 		= 10;

    obj->next 		= obj_list;
    obj_list  		= obj;

	objects[obj->nr].in_world++;
	objs_in_world++;

	return obj;
}

objectType * create_portal( charType * ch, int to_room )
{
	char			buf[MAX_STRING_LENGTH];
	char		*	align;
	objectType	*	obj;

	obj = alloc_object();

	obj->nr 		= OBJECT_PORTAL;
	obj->in_room 	= NOWHERE;

	if     ( IS_SAINT( ch ) ) 	align = "White";
	else if( IS_GOOD( ch ) ) 	align = "Blue";
	else if( IS_NEUTRAL( ch ) ) align = "Yellow";
	else if( IS_DEVIL( ch ) ) 	align = "Black";
	else						align = "Red";

    obj->name  		= strdup( "portal magicportal" );

	sprintf( buf, "a %s Magic Portal", align );
    obj->wornd 		= strdup( buf );

	sprintf( buf, ">>>>> %s Portal to %s <<<<<", align, world[to_room].name );
    obj->roomd 		= strdup( buf );

    obj->type 		= ITEM_OTHER; 
    obj->value[0] 	= to_room;
	obj->value[1]   = ch->align;
	obj->value[2]   = ch->level / 2;
    obj->next 		= obj_list;
    obj_list  		= obj;

	objects[obj->nr].in_world++;
	objs_in_world++;

	return obj;
}
