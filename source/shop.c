/* ************************************************************************
*  file: shop.c , Shop module.                            Part of DIKUMUD *
*  Usage: Procedures handling shops and shopkeepers.                      *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "character.h"
#include "mobile.h"
#include "world.h"
#include "comm.h"
#include "find.h"
#include "interpreter.h"
#include "utils.h"
#include "limits.h"
#include "misc.h"
#include "variables.h"
#include "fight.h"
#include "strings.h"
#include "weather.h"
#include "transfer.h"
#include "spec.rooms.h"

#define MAX_TRADE 	10
#define MAX_PROD  	15

typedef struct shop_data
{
  int 		producing[MAX_PROD];/* Which item to produce (virtual)      */
  float 	profit_buy;       	/* Factor to multiply cost with.        */
  float 	profit_sell;      	/* Factor to multiply cost with.        */
  byte 		type[MAX_TRADE];   	/* Which item to trade.                 */
  char *	no_such_item1;    	/* Message if keeper hasn't got an item */
  char *	no_such_item2;    	/* Message if player hasn't got an item */
  char *	missing_cash1;    	/* Message if keeper hasn't got cash    */
  char *	missing_cash2;    	/* Message if player hasn't got cash    */
  char *	do_not_buy;      	/* If keeper dosn't buy such things.    */
  char *	message_buy;      	/* Message when player buys item        */
  char *	message_sell;     	/* Message when player sells item       */
  int 		temper1;            /* How does keeper react if no money    */
  int 		temper2;            /* How does keeper react when attacked  */
  int 		keeper;             /* The mobil who owns the shop (virtual)*/
  int 		with_who;    		/* Who does the shop trade with?  		*/
  int 		in_room;    		/* Where is the shop?      				*/
  int 		open1,open2;  		/* When does the shop open?    			*/
  int 		close1,close2;  	/* When does the shop close?    		*/

} shopType;


struct shop_data *	shop_index;
int 				number_of_shops;

int is_ok( charType * keeper, charType * ch, int shop_nr )
{
  	if( shop_index[shop_nr].open1 > time_info.hours )
  	{
    	do_say( keeper, "Come back later!", COM_SAY );
    	return(FALSE);
  	} 
  	else if( shop_index[shop_nr].close1 < time_info.hours )
  	{
    	if( shop_index[shop_nr].open2 > time_info.hours )
		{
      		do_say(keeper, "Sorry, we have closed, but come back later.", COM_SAY);
      		return(FALSE);
    	} 
		else if( shop_index[shop_nr].close2 < time_info.hours )
		{
      		do_say( keeper, "Sorry, come back tomorrow.", COM_SAY );
      		return(FALSE);
    	}
	}

  	if( !(can_see(keeper,ch)) )
  	{
    	do_say(keeper, "I don't trade with someone I can't see!", COM_SAY );
    	return(FALSE);
  	}

  	switch( shop_index[shop_nr].with_who )
  	{
    	case 0 :  return(TRUE);
    	case 1 :  return(TRUE);
    	default : return(TRUE);
  	}
}

int trade_with(objectType *item, int shop_nr)
{
  	int counter;

   	if( item->cost < 1 ) return(FALSE);

  	for( counter=0; counter < MAX_TRADE; counter++ )
  	{
    	if( shop_index[shop_nr].type[counter] == item->type ) return(TRUE);
    	if( shop_index[shop_nr].type[counter] == 0 ) 		  break;
	}

  	return(FALSE);
}

int shop_producing(objectType *item, int shop_nr)
{
  	int counter;

  	if( item->nr < 0 ) return(FALSE);

  	for( counter = 0; counter < MAX_PROD; counter++ )
  	{
    	if( shop_index[shop_nr].producing[counter] == item->nr )	return(TRUE);
    	if( shop_index[shop_nr].producing[counter] == OBJECT_NULL ) break;
	}

  	return(FALSE);
}

static int _buy_one_of( charType * ch, char * argu, char * card, charType * keeper, int shop_nr )
{
	int						cost;
	objectType			*	obj;
	char					buf[MAX_STRING_LENGTH];

  	if( !(obj = find_obj_list( ch, argu, keeper->carrying, 1 )) )
  	{
    	sprintf( buf, shop_index[shop_nr].no_such_item1 ,GET_NAME(ch) );
    	do_tell(keeper,buf,COM_TELL);
    	return -1;
  	}

  	if( obj->cost <= 0 )
  	{
    	sprintf( buf, shop_index[shop_nr].no_such_item1 ,GET_NAME(ch) );
    	do_tell( keeper, buf, COM_TELL );
    	extract_obj( obj, 1 );
    	return -2;
  	}

  	cost = obj->cost * shop_index[shop_nr].profit_buy;
  	cost = cost * obj->status / 100;
  
  	if( (ch->carry_items + 1 > can_carry_number(ch)) )
  	{
    	sendf( ch, "%s : You can't carry that many items.", onewordc( obj->name ) );
    	return -3;
  	}

  	if ((ch->carry_weight + obj->weight) > can_carry_weight(ch))
  	{
    	sendf( ch, "%s : You can't carry that much weight.", onewordc( obj->name ) );
    	return -4;
  	}

	if( !cost_from_player( ch, keeper, card, cost, 1 ) ) return -5;

   	GET_GOLD(keeper) += cost/3;

  	if( shop_producing( obj,shop_nr ) )  	obj = load_a_object( obj->nr, REAL, 1 );
  	else 									obj_from_char( obj );

  	obj_to_char( obj, ch );

  	return 0;
}

void shopping_buy( char * arg, charType * ch, charType * keeper, int shop_nr )
{
  	char 				card[100]; 
  	char				argm[100]; 
  	char				buf[MAX_STRING_LENGTH];
  	objectType 		*	temp1;
  	int					cost;
  	int					nr, bought = 0;

  	if( !(is_ok( keeper, ch, shop_nr)) ) return;

	arg = oneArgument( arg, argm );

	if( isnumstr( argm ) )
	{
		if( getnumber( argm, &nr ) < 0 )
		{
    		sendf( ch, "%s tells you, 'What do you want to buy??" , keeper->moved );
    		do_tell( keeper, buf, COM_TELL );
			return;
		}
		arg = oneArgument( arg, argm );
	}
	else nr = 1;

  	oneArgument( arg, card );

  	if(!(*argm))
  	{
    	sendf( ch, "%s tells you, 'What do you want to buy??" , keeper->moved );
    	do_tell(keeper,buf,COM_TELL);
    	return;
  	}

  	if( !(temp1 = find_obj_list( ch, argm, keeper->carrying, 1 )) )
  	{
    	sprintf( buf, shop_index[shop_nr].no_such_item1 ,GET_NAME(ch));
    	do_tell(keeper,buf,COM_TELL);
    	return;
  	}

  	if( temp1->cost <= 0)
  	{
    	sprintf( buf, shop_index[shop_nr].no_such_item1 ,GET_NAME(ch));
    	do_tell( keeper, buf, COM_TELL );
    	extract_obj( temp1, 1 );
    	return;
  	}

  	cost = temp1->cost * shop_index[shop_nr].profit_buy;
  	cost = cost * temp1->status / 100;
  
  	if( nr > 1 )
  	{
  		while( nr-- )
  		{
  			if( _buy_one_of( ch, argm, card, keeper, shop_nr ) < 0 ) break;
  			bought++;
  		}

  		if( bought )
  		{
  			sprintf( buf, "$n buys %d $p.", bought );
  			act( buf, FALSE, ch, temp1, 0, TO_ROOM );
  			sendf( ch, "You now have %d %s.", bought, temp1->wornd );

  			if( *card )
		  		sendf( ch, "%s tells you, 'I'll take %s coins from your account.'", 
					GET_NAME(keeper), numfstr( cost * bought ) );
			else
        		sendf( ch, "%s tells you, 'That'll be %s gold coins.'", 
					GET_NAME(keeper), numfstr( cost * bought ) );
  		}
  		return;
  	}

  	if( (ch->carry_items + 1 > can_carry_number(ch)) )
  	{
    	sendf( ch, "%s : You can't carry that many items.", onewordc( temp1->name ) );
    	return;
  	}

  	if ((ch->carry_weight + temp1->weight) > can_carry_weight(ch))
  	{
    	sendf( ch, "%s : You can't carry that much weight.", onewordc( temp1->name ) );
    	return;
  	}

	if( !cost_from_player( ch, keeper, card, cost, 1 ) ) return;

  	act( "$n buys $p.", FALSE, ch, temp1, 0, TO_ROOM );
  	sendf( ch, "You now have %s.", temp1->wornd );

   	GET_GOLD(keeper) += cost/3;

  	if( shop_producing( temp1,shop_nr ) )  	temp1 = load_a_object( temp1->nr, REAL, 1 );
  	else 									obj_from_char( temp1 );

  	obj_to_char( temp1, ch );

  	return;
}

void shopping_sell( char *arg, charType * ch, charType * keeper, int shop_nr )
{
  	char 			argm[100], buf[MAX_STRING_LENGTH];
  	objectType 	* 	temp1;
  	int				cost;

  	if( !(is_ok( keeper, ch, shop_nr )) ) return;

  	oneArgument(arg, argm);

  	if( !*argm )
  	{
    	sprintf( buf, "%s What do you want to sell??" ,GET_NAME(ch));
    	do_tell( keeper, buf, COM_TELL );
    	return;
  	}

  	if( !( temp1 = find_obj_inven( ch, argm )) )
  	{
    	sprintf( buf, shop_index[shop_nr].no_such_item2 ,GET_NAME(ch) );
    	do_tell( keeper,buf, COM_TELL );
    	return;
  	}

  	if( !(trade_with(temp1,shop_nr)) || (temp1->cost<1) )
  	{
    	sprintf( buf, shop_index[shop_nr].do_not_buy, GET_NAME(ch));
    	do_tell(keeper,buf,COM_TELL);
    	return;
  	}

  	cost = temp1->cost * shop_index[shop_nr].profit_sell;
  	cost = cost * temp1->status / 100;

  	if( GET_GOLD(keeper) < cost )
  	{
    	sprintf( buf, shop_index[shop_nr].missing_cash1 ,GET_NAME(ch) );
    	do_tell( keeper, buf, COM_TELL );
    	return;
  	}

  	act( "$n sells $p.", FALSE, ch, temp1, 0, TO_ROOM );

  	sprintf( buf, shop_index[shop_nr].message_sell, GET_NAME(ch), cost );
  	do_tell( keeper, buf, COM_TELL );

  	sendf( ch, "The shopkeeper now has %s.", temp1->wornd );

  	GET_GOLD(ch) 	 += cost; 
  	GET_GOLD(keeper) -= cost;

  	if( (find_obj_inven( keeper, argm )) || ( temp1->type == ITEM_TRASH) ) 
  		extract_obj( temp1, 1 );
  	else
  	{
    	obj_from_char( temp1 );
    	obj_to_char( temp1, keeper );
  	}

  	return;
}

void shopping_value( char *arg, charType *ch, charType *keeper, int shop_nr)
{
  	char 			argm[100], buf[MAX_STRING_LENGTH];
  	objectType 	*	temp1;
  	int				cost;

  	if( !(is_ok(keeper,ch,shop_nr)) ) return;

  	oneArgument( arg, argm );

  	if( !*argm )
  	{
    	sprintf( buf, "%s What do you want me to valuate??", GET_NAME(ch) ); 
		do_tell( keeper, buf, COM_TELL );
    	return;
  	}

  	if( !( temp1 = find_obj_inven( ch, argm )) )
  	{
    	sprintf( buf, shop_index[shop_nr].no_such_item2, GET_NAME(ch) );
    	do_tell( keeper, buf, COM_TELL );
    	return;
  	}

  	if( !(trade_with( temp1, shop_nr )) )
  	{
    	sprintf( buf, shop_index[shop_nr].do_not_buy, GET_NAME(ch) );
    	do_tell( keeper, buf, COM_TELL );
    	return;
  	}

  	cost = temp1->cost * shop_index[shop_nr].profit_sell;
  	cost = cost * temp1->status / 100;

  	sprintf( buf, "%s I'll give you %d gold coins for that!", GET_NAME(ch), cost );
  	do_tell( keeper, buf, COM_TELL );

  	return;
}

void shopping_list( char *arg, charType *ch, charType *keeper, int shop_nr )
{
  	char 			buf[MAX_STRING_LENGTH], buf2[100],buf3[100];
  	objectType *	temp1;
  	int 			cost;
  	int 			found_obj;

  	if( !(is_ok(keeper,ch,shop_nr)) ) return;

  	strcpy( buf, "You can buy:\n\r" );
  	found_obj = FALSE;

	for( temp1 = keeper->carrying; temp1; temp1 = temp1->next_content)
      	if( (can_see_obj( ch, temp1) ) && ( temp1->cost > 0) )
    	{
      		found_obj = TRUE; 
	  		cost = temp1->cost * shop_index[shop_nr].profit_buy;
	  		cost = cost * temp1->status / 100;
      		if( temp1->type != ITEM_DRINKCON ) 
        		sprintf( buf2, "%s for %d gold coins.\n\r" , (temp1->wornd) , cost );
      		else 
	  		{
        		if( temp1->value[1] )
          			sprintf( buf3, "%s of %s",(temp1->wornd), drinks[temp1->value[2]] );
        		else
          			sprintf( buf3, "%s", temp1->wornd );

        		sprintf( buf2, "%s for %d gold coins.\n\r", buf3, cost );
      		}
      		strcat( buf, capitalize(buf2) );
    	}

  	if( !found_obj ) strcat( buf, "Nothing!\n\r" );

  	send_to_char(buf,ch);
  	return;
}

int shop_keeper( charType * ch, int cmd, char * arg )
{
  	charType 	*	temp;
  	charType 	*	keeper;
  	int 			shop_nr;

  	keeper = 0;

  	for( temp = world[ch->in_room].people; (!keeper) && (temp); temp = temp->next_in_room )
  		if( IS_MOB(temp) )
    		if( mobiles[temp->nr].func == shop_keeper ) keeper = temp;

  	for( shop_nr = 0 ; shop_index[shop_nr].keeper != keeper->nr; shop_nr++ )
  	;

  	if( (cmd == COM_BUY) && (ch->in_room == shop_index[shop_nr].in_room) )
  	{
    	shopping_buy( arg, ch, keeper, shop_nr );
    	return( TRUE );
  	}

  	if( (cmd == COM_SELL ) && (ch->in_room == shop_index[shop_nr].in_room) )
  	{
    	shopping_sell( arg, ch, keeper, shop_nr );
    	return(TRUE);
  	}
  	if( (cmd == COM_VALUE ) && (ch->in_room == shop_index[shop_nr].in_room) )
  	{
    	shopping_value( arg, ch, keeper, shop_nr );
    	return(TRUE);
  	}
  	if( (cmd== COM_LIST) && (ch->in_room == shop_index[shop_nr].in_room) )
  	{
    	shopping_list(arg,ch,keeper,shop_nr);
    	return(TRUE);
  	}
  	if( cmd == COM_STEAL )
  	{
    	sendf( ch, "Oops." );
    	hit( keeper, ch, -1 );
    	return(TRUE);
  	}
  	if( (cmd== COM_CAST) || (cmd== COM_RECITE) || (cmd== COM_USE) ) 
  	{
    	act( "$N tells you 'No magic here - kid!'.", FALSE, ch, 0, keeper, TO_CHAR);
    	return TRUE;
  	}
  	return(FALSE);
}

void assign_the_shopkeepers( void )
{
  	int 	nr, count;

  	for( nr = 0 ; nr < number_of_shops ; nr++ )
  	{
    	shop_index[nr].keeper = real_mobileNr( shop_index[nr].keeper );
		shop_index[nr].in_room = real_roomNr( shop_index[nr].in_room );
    	mobiles[shop_index[nr].keeper].func = shop_keeper;

		for( count = 0; count < MAX_PROD; count++ )
  		{ 
   			if( shop_index[nr].producing[count] > 0 ) 
   				shop_index[nr].producing[count]= real_objectNr(shop_index[nr].producing[count]);
   			else 		   break;
   		}
	}
}

void read_a_shop( FILE * fp, int nr )
{
	int					temp;
	int					count;

	for( count = 0; count < MAX_PROD; count++ )
  	{ 
   		fscanf( fp, "%d \n", &temp );
   		if( temp > 0 ) shop_index[nr].producing[count]= temp;
   		else 		   break;
   	}

   	if( temp != -1 )
   	{
   		while( fscanf( fp, "%d \n", &temp ), temp != -1 );
   	}


   	for(; count < MAX_PROD; count++ ) shop_index[nr].producing[count] = OBJECT_NULL;

	fscanf( fp, "%f \n", &shop_index[nr].profit_buy  );
	fscanf( fp, "%f \n", &shop_index[nr].profit_sell );

   	for( count = 0; count < MAX_TRADE; count++ )
   	{
   		fscanf( fp,"%d \n", &temp); 
   		if( temp > 0 ) shop_index[nr].type[count] = (byte)temp;
   		else 		   break;
   	}

   	if( temp != -1 )
   	{
   		while( fscanf( fp, "%d \n", &temp ), temp != -1 );
   	}

   	for(; count < MAX_TRADE; count++ ) shop_index[nr].type[count] = OBJECT_NULL;

	shop_index[nr].no_such_item1 	= fread_string( fp );
    shop_index[nr].no_such_item2 	= fread_string( fp );
    shop_index[nr].do_not_buy 		= fread_string( fp );
	shop_index[nr].missing_cash1 	= fread_string( fp );
	shop_index[nr].missing_cash2 	= fread_string( fp );
	shop_index[nr].message_buy 		= fread_string( fp );
	shop_index[nr].message_sell 	= fread_string( fp );

	fscanf( fp,"%d \n", &shop_index[nr].temper1 );
	fscanf( fp,"%d \n", &shop_index[nr].temper2 );
	fscanf( fp,"%d \n", &shop_index[nr].keeper );

    fscanf( fp,"%d \n", &shop_index[nr].with_who );
    fscanf( fp,"%d \n", &shop_index[nr].in_room );


    fscanf( fp,"%d \n", &shop_index[nr].open1 );
    fscanf( fp,"%d \n", &shop_index[nr].close1 );
    fscanf( fp,"%d \n", &shop_index[nr].open2 );
    fscanf( fp,"%d \n", &shop_index[nr].close2 );
}

void boot_shops( FILE * fp )
{
  	char				buf[MAX_STRING_LENGTH+1];

   	do
   	{
   		fgets( buf, MAX_STRING_LENGTH, fp );

	} while( buf[0] != '#' );

  	for(;;)
  	{
		fgets( buf, MAX_STRING_LENGTH, fp );

    	if( *buf == '#' )  /* a new shop */
    	{
      		if( !number_of_shops )  /* first shop */
        		shop_index = (struct shop_data *)errCalloc( sizeof(struct shop_data) );
      		else
        	if(!(shop_index= (struct shop_data*)
						 errRealloc( shop_index,(number_of_shops + 1)* sizeof(struct shop_data))))
        	{
          		FATAL( "boot_the_shops> Error in boot shop" );
        	}
			
			read_a_shop( fp, number_of_shops );

      		number_of_shops++;
    	}
    	else if(*buf == '$')  break;	/* EOF */
		else if(*buf == '*')  continue;	/* comment */
		else
		{
			DEBUG( "boot_shop> file corrupted?, after %d shops.", number_of_shops );
		}
  	}
}

static void write_a_shop( FILE * fp, shopType * shop, int nr )
{
	int			count;

	fprintf( fp, "#%d %s (%s)\n", nr, mobiles[shop->keeper].name, world[shop->in_room].name );
	
   	for( count = 0; count < MAX_PROD; count++ )
  	{
  		if( shop->producing[count] != OBJECT_NULL )
   			fprintf( fp, "%d\n", objects[shop->producing[count]].virtual );
   		else
   		{
   			fprintf( fp, "-1\n" );
   			break;
   		}
	}

	fprintf( fp, "%f\n", shop->profit_buy  );
   	fprintf( fp, "%f\n", shop->profit_sell );

 	for( count = 0; count < MAX_TRADE; count++ )
   	{
   		if( shop->type[count] != 0 )
   			fprintf( fp, "%d\n", (int) shop->type[count] );
   		else
   		{
   			fprintf( fp, "-1\n" );
   			break;
   		}
   	}

    fwrite_string( fp, shop->no_such_item1 );
    fwrite_string( fp, shop->no_such_item2 );
    fwrite_string( fp, shop->do_not_buy );
    fwrite_string( fp, shop->missing_cash1 );
    fwrite_string( fp, shop->missing_cash2 );
    fwrite_string( fp, shop->message_buy );
    fwrite_string( fp, shop->message_sell );

    fprintf( fp, "%d\n", shop->temper1 );
    fprintf( fp, "%d\n", shop->temper2 );
 	fprintf( fp, "%d\n", mobiles[shop->keeper].virtual );

    fprintf( fp, "%d\n", shop->with_who );
    fprintf( fp, "%d\n", world[shop->in_room].virtual );
  	fprintf( fp, "%d\n", shop->open1 );
   	fprintf( fp, "%d\n", shop->close1 );
    fprintf( fp, "%d\n", shop->open2 );
    fprintf( fp, "%d\n", shop->close2 );
}

void write_shops( FILE * fp, int zone )
{
	int			i;

	fprintf( fp, "#SHOPS\n" );

	for( i = 0; i < number_of_shops; i++ )
	{
		if( world[shop_index[i].in_room].zone == zone )
			write_a_shop( fp, &shop_index[i], i );
	}

	fprintf( fp, "$\n" );
}
