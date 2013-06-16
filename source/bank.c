#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "character.h"
#include "object.h"
#include "mobile.h"
#include "world.h"
#include "interpreter.h"
#include "strings.h"
#include "comm.h"
#include "spec.rooms.h"
#include "nanny.h"

#define	 BANK_MAX_ALLOW 	20
#define	 BANK_ACC_NAME		15
#define	 BANK_ACC_PASS	    15

typedef struct __accountType__
{
	char		*	name;
	char		* 	passwd;
	char		*	allow[20];
	int				using;
	unsigned int	amount;

	struct __accountType__	*	next;

} accountType;

static accountType 	*	accounts         = 0;
static int				accounts_in_bank = 0;

static accountType * find_account( char * name )
{
	accountType * find;

	for( find = accounts; find; find = find->next )
	{
		if( stricmp( name, find->name ) == 0 ) return find;
	}
	return 0;
}

static int find_member( accountType * find, charType * ch, char * member )
{
	int		i;

	for( i = 0; i < find->using; i++ )
	{
		if( find->allow[i] )
		{
			if( member )
			{
				 if( stricmp( find->allow[i], member   ) == 0 ) return i;
			}
			else if( stricmp( find->allow[i], ch->name ) == 0 ) return i;
		}
	}
	return -1;
}

static int load_bank( void )
{
	FILE		*	fp;
	char			fname[200]; 
	char			buf[100]; 
	int				i, j, nr;
	unsigned int	amount;
	accountType	*	new;

	sprintf( fname, "%s/bank", ROOM_STASH_DIR );

	if( fp = errOpen( fname, "r" ), !fp )
	{
        creat( fname, S_IRUSR | S_IWUSR );
        if( fp = errOpen( fname, "w+" ), !fp )
        {
            FATAL( "load_bank> failed to create new %s.", fname );
        }
        else
        {
            fprintf( fp, "#0 accounts\n" );
            log( "load_bank> new %s file created.", fname );
            fclose( fp );
        }
        return 1;
	}

	nr = fscanf( fp, "#%d accounts\n", &accounts_in_bank );

	if( nr != 1 )
	{
		ERROR( "load_bank> bank accounts file corrupted." );
		errClose( fp );
		return 0;
	}

	for( i = 0; i < accounts_in_bank; i++ )
	{
		if( new = errCalloc( sizeof( accountType ) ), !new )
		{
			ERROR( "load_bank> alloc failure." );
			errClose( fp );
			return 0;
		}

		fscanf( fp, "#%d\n", &nr );

		if( nr != i + 1 )
		{
			ERROR( "load_bank> bank accounts file corrupted." );
			errClose( fp );
			return 0;
		}

		fscanf( fp, "NAME   : %s\n", buf );		new->name   = strdup( buf );
        fgets( buf, 99, fp );
		new->passwd = strdup( remove_newline(buf + 9) );
		fscanf( fp, "AMOUNT : %u\n", &amount );	new->amount = amount;
		fscanf( fp, "USING  : %d\n", &nr  );  	new->using  = nr;

		for( j = 0; j < nr; j++ )
		{
		fscanf( fp, "ALLOW  : %s\n", buf ); new->allow[j] = strdup( buf );
		}

		new->next = accounts;
		accounts  = new;
	}
	errClose( fp );
	return 1;
}

static void save_accounts( void )
{
	FILE		*	fp;
	char			fname[200]; 
	int				i, j;
	accountType	*	curr;

	sprintf( fname, "%s/bank", ROOM_STASH_DIR );

	if( fp = errOpen( fname, "w+" ), !fp )
	{
		ERROR( "save_accounts> can't open %s.", fname );
		return;
	}

	fprintf( fp, "#%d accounts\n", accounts_in_bank );

	for( i = 0, curr = accounts; curr && i < accounts_in_bank; curr = curr->next, i++ )
	{
		fprintf( fp, "#%d\n", i + 1 );

		fprintf( fp, "NAME   : %s\n", curr->name );
		fprintf( fp, "PASSWD : %s\n", curr->passwd );
		fprintf( fp, "AMOUNT : %u\n", curr->amount );
		fprintf( fp, "USING  : %d\n", curr->using );

		for( j = 0; j < curr->using; j++ )
		{
			if( curr->allow[j] ) fprintf( fp, "ALLOW  : %s\n", curr->allow[j] );
			else				 break;
		}
	}
	errClose( fp );
}

static void bank_list( charType * ch, int cmd, char * arg )
{
   	sendf( ch, "At the bank you may ----------------\n\r\n\r"
   	           "list                               - get this list\n\r"
   	           "balance  [account] [all]           - check balance of [your | named account]\n\r"
   	           "deposit  <amount>  [account]       - deposit money in bank [ named account]\n\r"
   	           "withdraw <amount>  [account]       - withdraw money from bank [ named account]\n\r"
			   "allow    [account] [name] [passwd] - allow a player to use named account\n\r"
			   "disallow [account] [name] [passwd] - disallow a player to use named account\n\r"
   			   "account  [account] [passwd]        - make a family account.\n\r"
			   "close    [account] [passwd]        - close a named account\n\r" );
}

static void bank_account( charType * ch, int cmd, char * arg )
{
	char				dummy[MAX_INPUT_LENGTH+1];
	char				name[MAX_INPUT_LENGTH+1]; 	
	char				passwd[MAX_INPUT_LENGTH+1];
	accountType		*	new;

	halfchop( arg, name, passwd );

	if( !*name )
	{
		sendf( ch, "Hmm.. Need a account name." ); return;
	}

	if( !*passwd )
	{
		sendf( ch, "Hmm.. Need a password." ); return;
	}

	if( (strlen( name ) > BANK_ACC_NAME ) || (strlen( passwd ) > BANK_ACC_PASS ) )
	{
		sendf( ch, "Account name & passwd can't exceed 15 characters." );
		return;
	}

	if( nan_check_name( name, dummy ) )
	{
		sendf( ch, "Illegal name.. Try another." );
		return;
	}

	if( find_account( name ) )
	{
		sendf( ch, "That name is already used. Try other name." );
		return;
	}

	if( new = errCalloc( sizeof( accountType ) ), !new )
	{
		ERROR( "bank_account> malloc failed." );
		return;
	}

	new->name     = strdup( name );
	new->passwd   = strdup( passwd );
	new->allow[0] = strdup( ch->name );
	new->using	  = 1;

	new->next     = accounts;
	accounts      = new;

	accounts_in_bank++;

	save_accounts();

	sendf( ch, "Ok. New account." );
}

static void bank_allow( charType * ch, int cmd, char * arg )
{
	char				name[MAX_INPUT_LENGTH+1];
	char				passwd[MAX_INPUT_LENGTH+1];
	char				allow[MAX_INPUT_LENGTH+1];
	accountType	*		find;
	int					nr;

	arg = oneArgument( arg, name );	halfchop( arg, allow, passwd );

	if( !*name || !*allow || !*passwd )
	{
		sendf( ch, "Try list for usage." ); return;
	}

	if( find  = find_account( name ), !find )
	{
		sendf( ch, "No account by that name." );
		return;
	}

	if( stricmp( passwd, find->passwd ) != 0 ) 
	{
		sendf( ch, "Password mismatch." );
		WAIT_STATE( ch, PULSE_ROUND );
		senddf( 0, 44, "bank_allow> passwd mismatch (%s -> %s)", ch->name, name );
		return;
	}

	if( find->using >= BANK_MAX_ALLOW )
	{
		sendf( ch, "Your account is full of member. Not more than %d.", BANK_MAX_ALLOW );
		return;
	}

	if( stricmp( find->allow[0], ch->name ) != 0 )
	{
		sendf( ch, "That account is not yours." ); return;
	}

	if( find_member( find, 0, allow ) != -1 )
	{
		sendf( ch, "That member already has a permission." ); return;
	}

	if( nr = find_name( allow ), nr == -1 )
	{
		sendf( ch, "There is no player by that name." ); return;
	}

	find->allow[find->using++] = capitalize( strdup( player_table[nr].name ) );

	save_accounts();

	sendf( ch, "Ok. New member." );
}

static void bank_disallow( charType * ch, int cmd, char * arg )
{
	char				name[MAX_INPUT_LENGTH + 1];
	char				passwd[MAX_INPUT_LENGTH + 1];
	char				disallow[MAX_INPUT_LENGTH + 1];
	int					i;
	accountType	*		find;

	arg = oneArgument( arg, name );	halfchop( arg, disallow, passwd );

	if( !*name || !*disallow || !*passwd )
	{
		sendf( ch, "Try list for usage." ); return;
	}

	if( find = find_account( name ), !find )
	{
		sendf( ch, "No account by that name." );
		return;
	}

	if( stricmp( passwd, find->passwd ) != 0 ) 
	{
		sendf( ch, "Password mismatch." );
		WAIT_STATE( ch, PULSE_ROUND );
		senddf( 0, 44, "bank_disallow> passwd mismatch (%s -> %s)", ch->name, name );
		return;
	}

	if( stricmp( find->allow[0], ch->name ) != 0 )
	{
		sendf( ch, "That account is not yours." ); return;
	}

	if( i = find_member( find, 0, disallow ), i == -1 )
	{
		sendf( ch, "That account has no such member." );
		return;
	}

	if( i == 0 )
	{
		sendf( ch, "You can not do that. Close account instead of disallow owner." );
		return;
	}

	errFree( find->allow[i] );

	for(; i < BANK_MAX_ALLOW - 1; i++ )	find->allow[i] = find->allow[i+1];

	find->allow[BANK_MAX_ALLOW-1] = 0;
	find->using--;

	save_accounts();

	sendf( ch, "Ok. Done." );
}

static int bank_close( charType * ch, int cmd, char * arg )
{
	char				name[MAX_INPUT_LENGTH+1]; 	
	char				passwd[MAX_INPUT_LENGTH+1];
	accountType		*	find, * prev;
	int					i;

	halfchop( arg, name, passwd );

	if( !*name || !*passwd ) return 0; 

	if( find = find_account( name ), !find )
	{
		sendf( ch, "No account by that name." ); return 1;
	}

	if( !IMPL( ch ) )
	{
		if( stricmp( passwd, find->passwd ) != 0 ) 
		{
			sendf( ch, "Password mismatch." );
			WAIT_STATE( ch, PULSE_ROUND );
			return 1;
		}

		if( stricmp( find->allow[0], ch->name ) != 0 )
		{
			sendf( ch, "That account is not yours." );
			return 1;
		}

		if( find->amount > 0 )
		{
			sendf( ch, "That account is not empty." );
			return 1;
		}
	}

	if( accounts == find ) accounts = find->next;
	else
	{
		for( prev = accounts; prev->next != find; prev = prev->next )
		;

		if( !prev ||  prev->next != find )
		{
			ERROR( "bank_close> account not found." );
			return 1;
		}

		prev->next = find->next;
	}

	errFree( find->passwd );

	for( i = 0; i < find->using; i++ )
	{
		if( find->allow[i] ) errFree( find->allow[i] );
	}

	errFree( find );

	accounts_in_bank--;

	save_accounts();

	sendf( ch, "Ok. %s is closed.", name );
	return 1;
}

static void bank_balance( charType * ch, int cmd, char * arg )
{
	char				name[MAX_INPUT_LENGTH+1];
	char				all[MAX_INPUT_LENGTH+1];
	int					i;
	accountType		*	find;

	twoArgument( arg, name, all );

	if( !*name )
	{
		sendf( ch, "You have %s coins in the bank.", numfstr(ch->bank) );
		return;
	}

	if( find = find_account( name ), !find )
	{
		sendf( ch, "No account by that name." ); return;
	}

	if( i = find_member( find, ch, 0 ), i == -1 )
	{
		sendf( ch, "You are not a member of that account." ); return;
	}

	sendf( ch, "The account, %s, has %s gold coins.", find->name, numfstr(find->amount) );

	if( *all && stricmp( all, "all" ) == 0 )
	{
		sendf( ch, "The account, %s, is used by :", find->name );
		for( i = 0; i < find->using; i++ )
		{
			if( find->allow[i] ) sendf( ch, "-- %s", find->allow[i] );
			else				 break;
		}
	}
}

static void bank_dep_with( charType * ch, int cmd, char * arg )
{
	char				amount[MAX_INPUT_LENGTH+1];
	char				name[MAX_INPUT_LENGTH+1];
	unsigned int		gold;
	accountType		*	find = 0;

	twoArgument( arg, amount, name );

	if( getnumber( amount, &gold ) <= 0 || gold <= 0 )
	{
		sendf( ch, "Need a positive number for amount of gold coins." );
		return;
	}

	if( *name )
	{
		if( find = find_account( name ), !find )
		{
			sendf( ch, "No account by that name." ); return;
		}

		if( cmd == COM_WITHDRAW && find_member( find, ch, 0 ) < 0 )
		{
			sendf( ch, "You are not a member of that account." ); return;	
		}
	}

	if( cmd == COM_DEPOSIT )
	{
		if( ch->gold < gold )
		{
			sendf( ch, "You don't have that much gold coins." );
			return;
		}

		ch->gold -= gold;

		if( find ) 
		{
			if( (find->amount + gold) < find->amount )
			{
				sendf( ch, "That account is full of gold." );
				return;
			}

			find->amount += gold;
		}
		else
		{
			if( (ch->bank + gold) < ch->bank )
			{
				sendf( ch, "Your account is full of gold." );
				return;
			}

			ch->bank     += gold;
		}
	}
	else
	{
		if( find )
		{
			if( find->amount < gold )
			{
				sendf( ch, "That account don't has that much gold coins." ); return;
			}

			if( (ch->gold + gold) < ch->gold )
			{
				sendf( ch, "You are full of gold." ); return;
			}

			find->amount -= gold;
			ch->gold     += gold;
		}
		else
		{
			if( ch->bank < gold )
			{
				sendf( ch, "You don't have that much gold coins in the bank." ); return;
			}

			if( (ch->gold + gold) < ch->gold )
			{
				sendf( ch, "You are full of gold." ); return;
			}

			ch->bank -= gold;
			ch->gold += gold;
		}
	}

	save_accounts();

	sendf( ch, "Ok. Done." );
}

static int bank( charType * ch, int cmd, char * arg )
{
  	if( IS_NPC(ch) ) return(FALSE);

	switch( cmd )
	{
		case	COM_LIST 	:	bank_list( ch, cmd, arg );		return 1;
		case	COM_BALANCE	:	bank_balance( ch, cmd, arg );	return 1;
		case	COM_DEPOSIT :
		case	COM_WITHDRAW:	bank_dep_with( ch, cmd, arg );	return 1;
		case	COM_ALLOW 	:	bank_allow( ch, cmd, arg );		return 1;
		case	COM_DISALLOW:	bank_disallow( ch, cmd, arg );	return 1;
		case	COM_ACCOUNT	:	bank_account( ch, cmd, arg );	return 1;
		case	COM_CLOSE 	:	return bank_close( ch, cmd, arg );
	}
	return 0;
}

void init_bank( void )
{
	int		rNr;

	if( rNr = real_roomNr( ROOM_BANK ), rNr == NOWHERE )
	{
		DEBUG( "init_bank> can't find bank room." );
		return;
	}

	if( load_bank() ) world[rNr].func = bank;
}

void do_bank( charType * ch, char * argument, int cmd )
{
	int					found;
	int 				i;
	accountType		*	find;

    if( IS_NPC(ch) ) return;

	if( cmd != COM_BANK )
	{
		send_to_char( "You can only do that at the bank.\n\r", ch ); return;
	}

	if( !OMNI( ch ) )
	{
		sendf( ch, "You have %s gold coins in the bank.", numfstr( ch->bank ) );
		sendf( ch, "And you are a member of following accounts -----" );

		for( found = 0, find = accounts; find; find = find->next )
		{
			if( find_member( find, ch, 0 ) >= 0 )
			{
				sendf( ch, "%15s - %2d members - %15s coins.", 
									find->name, find->using, numfstr( find->amount ) );
				found++;
			}
		}
		if( !found ) sendf( ch, "No account." );
	}
	else
	{
		if( !*argument )
		{
			sendf( ch, "Accounts in bank : %d\n\r", accounts_in_bank );

			for( find = accounts; find; find = find->next )
			{
				sendf( ch, "%15s - %2d members - %15s gold coins", 
								find->name, find->using, numfstr( find->amount ) );
			}
		}
		else
		{
			for( found = 0, find = accounts; find; find = find->next )
			{
    			for( i = 0; i < find->using; i++ )
    			{
        			if( find->allow[i] && stricmp( find->allow[i], argument ) == 0 )
					{
						sendf( ch, "%15s - %2d members - %15s gold coins", 
								find->name, find->using, numfstr( find->amount ) );
						found++;
        			}
    			}
			}
			if( !found ) sendf( ch, "%s has no membership of any account.", argument );
		}
	}
}

int coins_from_bank( charType * ch, char * name, int amount )
{
	accountType		*	find;

	if( find = find_account( name ), !find ) return BANK_NO_ACCOUNT;
	if( find_member( find, ch, 0 ) < 0     ) return BANK_NO_MEMBER;
	if( amount < 0                         ) return BANK_NEGATIVE;
	if( find->amount < amount              ) return BANK_NO_AMOUNT;

	find->amount -= amount;

	return 0;
}

int cost_from_player( charType * ch, charType * keeper, char * name, int amount, int silent )
{
	accountType		* 	find;
	char			*	teller;

	if( keeper ) teller = keeper->moved;
	else		 teller = "Someone";

	if( amount < 0 )
	{
		sendf( ch, "%s tells you, 'Ooops.. Minor bug! Plz, report this to wizard.'", teller );
		return 0;
	}

	if( *name )
	{
		if( find = find_account( name ), !find )
		{
			sendf( ch, "%s tells you, 'Hmmm.. Let me see. No account by that name.'", teller );
			return 0;
		}

		if( find_member( find, ch, 0 ) < 0 )
		{
			sendf( ch, "%s tells you, 'It's not a your account.'", teller );
			return 0;
		}

		if( find->amount < amount )
		{
			sendf( ch, "%s tells you, 'That account does not have that much coins.'", teller );
			return 0;
		}

		find->amount -= amount;

		if( !silent )
		sendf( ch, "%s tells you, 'I'll take %s coins from your account.'",	teller, numfstr(amount) );
	}
	else
	{
		if( ch->gold < amount )
		{
			sendf( ch, "%s tells you, 'You don't have that much coins.'", teller );
			return 0;
		}

		ch->gold -= amount;

		if( !silent )
		sendf( ch, "%s tells you, 'That'll be %s gold coins.'",	teller, numfstr(amount) );
	}
	return 1;
}

int coins_to_char( charType * ch, int amount )
{
    if( amount < 0 )
	{
		DEBUG( "coins_to_char> from %s with % command at %s",
			ch->name, ch->desc ? ch->desc->last_input : "...", world[ch->in_room].name );
		return -2;
	}

    if( ch->gold + amount < ch->gold )
    {
        sendf( ch, "You are full of money." ); 
		DEBUG( "===> %s is full of money.", ch->name ); return -1;
    }

    ch->gold += amount;

    return 1;
}

int coins_from_char( charType * ch, int amount )
{
	if( amount < 0 )
	{
		DEBUG( "coins_from_char> from %s with % command at %s",
			ch->name, ch->desc ? ch->desc->last_input : "...", world[ch->in_room].name );
		return -2;
	}

	if( ch->gold < amount ) return -1;

	ch->gold -= amount;

	return 1;
}
