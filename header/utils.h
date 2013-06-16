#ifndef __UTILS_H
#define __UTILS_H

#ifndef __STDIO_H
#include <stdio.h>
#endif

#ifndef __CHARACTER_H
#include "character.h"
#endif

#ifndef __OBJECT_H
#include "object.h"
#endif

int		dice		( int ndice, int sdice );
int		number		( int from, int to );

char * 	sprinttype	(int type, char *names[], char *result);
char * 	sprintbit	(long vektor, char *names[], char *result);

char *  how_good	( int percent );
char *  itemstatus  ( objectType * obj );

void	set_title	( charType * ch );

char *  see_whom	( charType * ch, charType * vict );

void 	name_from_drinkcon	( objectType * obj );
void 	name_to_drinkcon	( objectType * obj, int type );

#endif/*__UTILS_H*/
