#include	<stdio.h>
#include	<string.h>

#include	"defines.h"
#include	"character.h"
#include	"weather.h"

#define		TOUPPER( ch )	(((ch)>='a'&&(ch)<='z') ? ch - 'a' + 'A' : ch)

void ch_lev( struct	__storechar__ *u, FILE *f );
void ch_passwd(	struct	__storechar__	*u, 	FILE	*f );
void ch_class(	struct	__storechar__	*u, 	FILE	*f );
void list_players( struct __storechar__	*u, 	FILE 	*f );
void delete_players_group( struct __storechar__	*u, 	FILE 	*f );
void delete_player( struct __storechar__	*u, 	FILE 	*f );
void ch_age(	struct	__storechar__	*u, 	FILE	*f );
void ch_char(	struct	__storechar__	*u, 	FILE	*f );
void print_char( struct	__storechar__ *u );

char	*get_class ( int class );

struct	instruct {
	char	name[ 80 ];
	void	(*func)();
};	

struct instruct instruction[] =
		{ { "change level", ch_lev },
		  { "change password", ch_passwd },
		  { "change class", ch_class },	
		  { "display char", print_char },
		  { "list players above pointed level", list_players },
		  { "delete players above pointed level", delete_players_group },
		  { "delete one player", delete_player },
		  { "change player's age", ch_age },
		  { "change character", ch_char },
		  { "", NULL } };

int		num_of_inst;
char	pl_f_name[50];

void print_char( struct	__storechar__ *u )
{
	printf("Name  : %s\n", u->name );
	printf("Title : %s\n", u->title );
	printf("Desc  : %s\n", u->description );
	printf("Times : life %d, logon %d, birth %d, played %d",
                    u->life, u->last_logon, -u->birth, u->played );
	printf("Str: %d,  Int: %d,  Wis: %d,  Dex: %d,  Con: %d\n",
						u->stat.str,
						u->stat.intel,
						u->stat.wis,
						u->stat.dex,
						u->stat.con	);
	printf("Mana: %d(%d),  Hit: %d(%d),  Move: %d(%d)\n\
		\rGold: %d,  Exp: %d,  Full: %d,  Thirst: %d\n",
						u->mana, u->max_mana,
						u->hit, u->max_hit,
						u->move, u->max_move,
						u->gold, u->exp,
						u->conditions[1], u->conditions[2]	);
}

void get_new_char( struct __storechar__	*u )
{
	int	num, out = 0;

	while ( !out )	{
		printf("[1] change strength\n");
		printf("[2] change password\n");
		printf("[3] change intelegence\n");
		printf("[4] change wisdom\n");
		printf("[5] change dexterity\n");
		printf("[6] change constitute\n");
		printf("[7] change mana point\n");
		printf("[8] change hit point\n");
		printf("[9] change move point\n");
		printf("[10] change gold\n");
		printf("[11] change exprience\n");
		printf("[12] change full\n");
		printf("[13] change thirst\n");
		printf("[14] print character\n");
		printf("[15] exit from this function\n");
		printf("Enter your choice: ");
		num = get_num( 1, 15 );
		switch( num )	{
		        case	1:	printf("Enter new strength: ");
						u->stat.str = get_num( 0, 25 );	break;
			case	3:	printf("Enter new intelegence: ");
						u->stat.intel = get_num( 0, 25 );	break;
			case	4:	printf("Enter new wisdom: ");
						u->stat.wis = get_num( 0, 25 );	break;
			case	5:	printf("Enter new dexterity: ");
						u->stat.dex = get_num( 0, 25 );	break;
			case	6:	printf("Enter new constitute: ");
						u->stat.con = get_num( 0, 25 );	break;
			case	7:	printf("Enter new mana point: ");
						u->mana = get_num( 0, 30000 );
						u->max_mana = u->mana;	break;
			case	8:	printf("Enter new hit point: ");
						u->hit = get_num( 0, 30000 );
						u->max_hit = u->hit;	break;
			case	9:	printf("Enter new move point: ");
						u->move = get_num( 0, 30000 );
						u->max_move = u->move;	break;
			case	10:	printf("Enter new gold: ");
						u->gold =
							get_num( 0, 900000000 );	break;
			case	11:	printf("Enter new exprience: ");
						u->exp =
							get_num( 0, 1000000000 );	break;
			case	12:	printf("Enter new full: ");
						u->conditions[1] = get_num( -1, 20 );	break;
			case	13:	printf("Enter new thirst: ");
						u->conditions[2] = get_num( -1, 20 );	break;
			case	14:	print_char( u );	break;
			case	15:	out = 1;	break;
			default:	printf("Error\n");	break;
		}
	}
}

struct time_info_data* imud_time_passed( time_t	t2, time_t t1 )
{
	long secs;
	struct time_info_data *now;

	now = (struct time_info_data *) malloc( sizeof( struct time_info_data ) );
	secs = (long) (t2 - t1);
	now->hours = (secs/SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
	secs -= SECS_PER_MUD_HOUR*now->hours;

	now->day = (secs/SECS_PER_MUD_DAY) % 35;     /* 0..44 days  */
	secs -= SECS_PER_MUD_DAY*now->day;

	now->month = (secs/SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
	secs -= SECS_PER_MUD_MONTH*now->month;

	now->year = (secs/SECS_PER_MUD_YEAR);        /* 0..XX? years */

	return now;
}

struct time_info_data * iage( struct __storechar__ *u ) 
{
  long secs;
  struct time_info_data *player_age;

  player_age = imud_time_passed( time(0) , u->birth);
  player_age->year += 17;   /* All players start at 17 */
  return player_age;
}

void print_age( struct	__storechar__	*u )
{
	printf("%d years %d months %d days %d hours\n",	iage( u )->year,
													iage( u )->month,
													iage( u )->day,
													iage( u )->hours );
}

void
struct_to_time( u, new )
	struct	__storechar__	*u;
	struct	time_info_data	*new;
{
	long secs;

	secs = (new->year-17)*SECS_PER_MUD_YEAR;
	secs += new->month*SECS_PER_MUD_MONTH;
	secs += new->day*SECS_PER_MUD_DAY;
	secs += new->hours*SECS_PER_MUD_HOUR;

	u->birth = time(0) - secs;
}

void
get_new_age( u )
	struct	__storechar__	*u;
{
	int	num, out = 0;
	struct	time_info_data	*t;

	while ( !out )	{
		printf("[1] chage age\n");
		printf("[2] chage month\n");
		printf("[3] chage day\n");
		printf("[4] chage hour\n");
		printf("[5] exit from this function\n");
		printf("Enter your choice: ");
		num = get_num( 1, 5 );
		t = iage( u );
		switch( num )	{
			case	1:	printf("Enter new age: ");
						t->year = get_num( 0, 100 );	break;
			case	2:	printf("Enter new months: ");
						t->month = get_num( 0, 16 );	break;
			case	3:	printf("Enter new days: ");
						t->day = get_num( 0, 44 );	break;
			case	4:	printf("Enter new hours: ");
						t->hours = get_num( 0, 23 );	break;
			case	5:	out = 1;	break;
			default:	printf("Error\n");	break;
		}
		struct_to_time( u, t );
	}
}

get_name( str )
	char	*str;
{
	printf("Enter name: ");
	gets( str );
	*str = TOUPPER( *str );
}

int	search_name( str, f )
	char	*str;
	FILE	*f;
{
	struct	__storechar__	u;

	fseek( f, 0, 0 );
	while( !feof( f ) )	{
		fread( &u, sizeof( struct __storechar__ ), 1, f );
		if ( strcmp( str, u.name ) ) continue;
		fseek( f, -sizeof( struct __storechar__ ), 1 );
		break;
	}
	if ( feof( f ) )
		return -1;
	return 1;
}

void isave_char( struct	__storechar__	*u,	FILE	*f )
{
	fwrite( u, sizeof( struct __storechar__ ), 1, f );
}

void ch_char( struct __storechar__	*u, 	FILE	*f)
{
	char	str[30];

	get_name( str );
	if ( search_name( str, f ) < 0 ) {
		printf("No such player\n");
		return;
	}
	fread( u, sizeof( struct __storechar__ ), 1, f );
	print_char( u );
	get_new_char( u );
	fseek( f, -sizeof( struct __storechar__ ), 1 );
	isave_char( u, f );
	print_age( u );
}

void ch_age( 	struct	__storechar__	*u, 	FILE	*f)
{
	char	str[30];

	get_name( str );
	if ( search_name( str, f ) < 0 ) {
		printf("No such player\n");
		return;
	}
	fread( u, sizeof( struct __storechar__ ), 1, f );
	print_age( u );
	get_new_age( u );
	fseek( f, -sizeof( struct __storechar__ ), 1 );
	isave_char( u, f );
	print_age( u );
}

void
delete_player( u, f )
	struct	__storechar__	*u;
	FILE	*f;
{
	FILE	*fn;
	char	str[30];
	static	char	tmp_f_name[] = "mud.frob.tmp";

	if ( ( fn = fopen ( tmp_f_name, "a+" ) ) == NULL )
		return;
	get_name( str );
	while ( !feof( f ) ) {
		fread( u, sizeof( struct __storechar__ ), 1, f );
		if ( strcmp( str, u->name ) && !feof( f )) {
			fwrite( u, sizeof( struct __storechar__ ), 1, fn );
			printf("%s\n", u->name );
		}
	}
	fflush( fn );
	fclose( f );
	if( unlink( pl_f_name ) < 0 )	{
		unlink( tmp_f_name );
		return;
	}
	if ( ( f = fopen( pl_f_name, "a+" ) ) == NULL )	{
		unlink( tmp_f_name );
		return;
	}
	fseek( fn, 0, 0 );
	for( fread( u, sizeof( struct __storechar__ ), 1, fn );
		!feof( fn ); fread( u, sizeof( struct __storechar__ ), 1, fn ) )
		if ( !feof( fn ) )
			fwrite( u, sizeof( struct __storechar__ ), 1, f );
	fflush( f );
	fclose( fn );
	unlink( tmp_f_name );
}

void
delete_players_group( u, f )
	struct	__storechar__	*u;
	FILE	*f;
{
	FILE	*fn;
	int		l_num = 1, h_num = 104;
	static	char	tmp_f_name[] = "mud.frob.tmp";

	if ( ( fn = fopen ( tmp_f_name, "a+" ) ) == NULL )
		return;

	printf("Enter lowest level: ");
	l_num = get_num( 1, 104 );
	printf("Enter highst level: ");
	h_num = get_num( l_num, 104 );

	while ( !feof( f ) )	{
		fread( u, sizeof( struct __storechar__ ), 1, f );
		if ( (u->level < l_num || u->level > h_num) && !feof( f ) )
			fwrite( u, sizeof( struct __storechar__ ), 1, fn );
	}
	fflush( fn );
	fclose( f );
	if( unlink( pl_f_name ) < 0 )	{
		unlink( tmp_f_name );
		return;
	}
	if ( ( f = fopen( pl_f_name, "a+" ) ) == NULL )	{
		unlink( tmp_f_name );
		return;
	}
	for( fseek( fn, 0, 0 ), fread( u, sizeof( struct __storechar__ ), 1, fn );
		!feof( fn ); fread( u, sizeof( struct __storechar__ ), 1, fn ) )
		fwrite( u, sizeof( struct __storechar__ ), 1, f );
	fflush( f );
	fclose( fn );
	unlink( tmp_f_name );
}

print_one_player( u )
	struct	__storechar__	*u;
{
	printf("[%d] %s %s (%s)\n", u->level,
								u->name,
								u->title,
								get_class( u->class ) );
}

void list_players( struct __storechar__	*u, FILE *f)
{
	int		l_num = 1, h_num = 104;

	printf("Enter lowest level: ");
	l_num = get_num( 1, 104 );
	printf("Enter highst level: ");
	h_num = get_num( l_num, 104 );

	while ( !feof( f ) )	{
		fread( u, sizeof( struct __storechar__ ), 1, f );
		if ( (u->level >= l_num && u->level <= h_num) && !feof( f ) )
			print_one_player( u );
	}
}

char	*get_class ( int class )
{
	char	*str;

	str = (char *) malloc( 10 );
	if ( class == 1 )
		strcpy(	str, "Magic");
	else if ( class == 2 )
		strcpy(	str, "Cleric");
	else if ( class == 3 )
		strcpy(	str, "Thief");
	else if ( class == 4 )
		strcpy( str, "Warrior");
	else
		strcpy( str, "Unknown");
	return str;
}

void
ch_class( u, f )
	struct	__storechar__	*u;
	FILE	*f;
{
	char	str[30];
	char	tmp[30];

	get_name( str );
	if ( search_name( str, f ) < 0 ) {
		printf("No such player\n");
		return;
	}
	fread( u, sizeof( *u ), 1, f );
	printf ("class [ %10s ] : ", get_class ( (int)u->class ) );
	gets(tmp);
	if ( !(!*tmp || *tmp == '\n') )	{
		if ( *tmp == 'M' || *tmp == 'm' )
			u->class = 1;
		else	if ( *tmp == 'C' || *tmp == 'c' )
			u->class = 2;
		else	if ( *tmp == 'T' || *tmp == 't' )
			u->class = 3;
		else	if ( *tmp == 'W' || *tmp == 'w' )
			u->class = 4;
		else	if ( *tmp == 'U' || *tmp == 'u' )
			u->class = 0;
	}
	fseek( f, -sizeof( struct __storechar__ ), 1 );
	isave_char( u, f );
}

void ch_passwd(	struct	__storechar__	*u, 	FILE	*f )
{
	char	str[ 30 ];
	char	*p, *q;

	get_name( str );
	if ( search_name( str, f ) < 0 )	{
		printf("No such player\n");
		return;
	}
	fread( u, sizeof( struct __storechar__ ), 1, f );
	p = (char *)getpass("Enter new password: ");
	q = (char *)getpass("ReEnter new password: ");
	if ( strcmp( (char *)crypt( p, str ), (char *)crypt( q, str ) ) )	{
		printf("Don't match. sorry..\n");
	} else	{
		strncpy( u->pass, (char *)crypt( p, str ), 10 );
		fseek( f, -sizeof( struct __storechar__ ), 1 );
		isave_char( u, f );
	}
}

void ch_lev( struct	__storechar__ *u, FILE *f )
{
	char	str[ 30 ];
	int		num;

	get_name( str );
	if ( search_name( str, f ) < 0 ) {
		printf("No such player\n");
		return;
	}
	fread( u, sizeof( struct __storechar__ ), 1, f );
	printf("%s [%d]\n", u->name, u->level);
	printf("To what level?(1-104): ");
	num = get_num( 1, 104 );
	u->level = num;
	fseek( f, -sizeof( struct __storechar__ ), 1 );
	isave_char( u, f );
}

main ( argc, argv )
	int	argc;
	char	*argv[];
{
	char	tmp[80];
	FILE	*f;

	f = fopen ( argv[1], "r+" );
	if ( !f )	{
		printf("No such file ...\n");
		exit();
	}
	strcpy( pl_f_name, argv[1] );

	init_inst();	/*	Check the total number of instructions  */

	frob ( f );
}

init_inst()
{
	int	inst = 0;

	while ( *(instruction[ inst ].name) != NULL )
		inst++;
	num_of_inst = inst;
}

int	y_or_no ()
{
	char	tmp[80];

	do	gets ( tmp );
	while ( !tmp[0] || tmp[0] == '\n' );

	if ( tmp[0] == 'y' || tmp[0] == 'Y' )
		return	1;
	else	return	0;
}

pr_menu()
{
	int	inst = 0;

	while ( *(instruction[ inst ].name) != NULL )	{
		printf("[%d] %s\n", inst + 1,
							instruction[ inst ].name );
		inst++;
	}
	printf("[%d] Quit from edit menu.\n", inst + 1 );
}

int	get_num( lower, upper )
	int	lower;
	int	upper;
{
	char	str[30];
	int		num;

	do	{
		gets( str );
		num = atoi( str );
		if ( num < lower || num > upper )
			printf("Invalid number, reenter new number: ");
	}	while ( num < lower || num > upper );

	return	num;
}

void frob ( FILE * f )
{
	struct	__storechar__	u;
	int	y_n, num;
	int	out = 0;

	while ( !out ) {
		pr_menu();
		printf("Enter number (%d - %d): ", 1, num_of_inst + 1 );
		num = get_num( 1, num_of_inst + 1);
		if ( num == num_of_inst + 1 )	{
			out = 1;
		} else	{
			(*(instruction[ num-1 ].func))( &u, f );
		}
		fclose( f );
		if ( ( f = fopen( pl_f_name, "r+" ) ) == NULL )
			return;
		printf("\n");
	}
}
/*
change_affect ( u )
	struct	__storechar__	*u;
{
	int	count = 0;

	printf ("Do you want clear ability? (Y/n) ");
	if ( y_or_no () )	{
		for ( count = 0; count < 10; count++ )
			set_bit ( &u->affected[count], 0, 0, 0, 0, 0 );
		return;
	}
	printf ("Do you want sanctuary? (Y/n) ");
	if ( y_or_no () )
	set_bit ( &u->affected[count++], SPELL_SANCTUARY
			, 1000, 0, APPLY_NONE, AFF_SANCTUARY );
	printf ("Do you want detect invisible? (Y/n) ");
	if ( y_or_no () )
	set_bit ( &u->affected[count++], SPELL_DETECT_INVISIBLE, 1000, 0
			, APPLY_NONE, AFF_DETECT_INVISIBLE );
	printf ("Do you want armor? (Y/n) ");
	if ( y_or_no () )
	set_bit ( &u->affected[count++], SPELL_ARMOR, 1000, -20, APPLY_AC, 0 );
	printf ("Do you want detect evil? (Y/n) ");
	if ( y_or_no () )
	set_bit ( &u->affected[count++], SPELL_DETECT_EVIL,
			1000, 0, APPLY_NONE, AFF_DETECT_EVIL );
	printf ("Do you want to be a SUPER MAN? (Y/n) ");
	if ( y_or_no () )	{
	set_bit ( &u->affected[count++], SPELL_DETECT_MAGIC,
			1000, -120, APPLY_AC, 0 );
	set_bit ( &u->affected[count++], SPELL_DETECT_MAGIC,
			1000, 30, APPLY_HITROLL, 0 );
	set_bit ( &u->affected[count++], SPELL_DETECT_MAGIC,
			1000, 30, APPLY_DAMROLL, 0 );
	}
}

set_bit ( af, tp, du, mo, ap, aftp )
	struct	affected_type	*af;
	int	tp, du, mo, ap, aftp;
{
	af->type = tp;
	af->duration = du;
	af->modifier = mo;
	af->location = ap;
	af->bitvector = aftp;
	af->next = NULL;
}
*/
