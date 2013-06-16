#ifndef __STASH_H
#define __STASH_H

int is_stashable( objectType * obj );
int stash_char( charType * ch, char * filename );
void unstash_char( charType * ch, char * filename );

void wipe_stash( char * filaname, int remake );
void move_stash( char * playername );

void stash_off_world( char * rname, objectType * list );
void unstash_off_world( char * rname, objectType ** list );

#endif/*__STASH_H*/
