#ifndef __NANNY_H
#define __NANNY_H

#ifndef __SOCKETS_H
#include "sockets.h"
#endif

#define MENU        \
"\n\rWelcome to SobackMUD\n\r\n\
0) Exit from DikuMud.\n\r\
1) Enter the game.\n\r\
2) Enter description.\n\r\
3) Change password.\n\r\
4) Delete your character.\n\r\n\r\
   Make your choice: "


#define GREETINGS \
"\n\r\n\r\
                    W E L C O M E  T O  S O B A C K - M U D\n\r\
\n\r\
                        Soback-MUD, derived from\n\r\n\r\
                               DikuMUD\n\r\n\r\
                             Created  by\n\r \
                Hans Henrik Staerfeldt, Katja Nyboe,\n\r \
         Tom Madsen, Michael Seifert, and Sebastian Hammer\n\r\n\r"

#define WELC_MESSG \
"\n\rWelcome to the land of DikuMUD. May your visit here be... Interesting.\
\n\rIf you are new player, type 'info' and 'help'\n\r"


#define STORY     \
"Once upon a time ....\n\r\n\r"

int  nan_check_name( char * arg, char * name );
void nanny( descriptorType * d, char * argum );

extern int	no_echo;

#endif/*__NANNY_H*/
