#ifndef __HASH_H
#define __HASH_H

#define FIXED_HASH_SIZE  (1<<11) /* 2048 */
#define FIXED_HVAL( a )  (a & (FIXED_HASH_SIZE-1))

typedef struct __hashtype__
{
	int						unit;
	int						key;
  	struct __hashtype__ *	next;
  	void				*	data;

} hashType;

typedef struct __hashtable__
{
	int						datasize;
	int					(*  hash_value)( void * );
  	int						table_size;
  	int					 *  keys;
  	struct __hashtype__	 **	table;

} hashTable;

typedef struct __fixedhashtable__
{
  	struct __hashtype__	*	table[FIXED_HASH_SIZE];

} fixedHashTable;

struct __fixedhashtable__* 	init_fixedHashTable( void );
void 				destroy_fixedHashTable( struct __fixedhashtable__ * );
void 				fixedHash_add( struct __fixedhashtable__ *, int data );
int  				fixedHash_has( struct __fixedhashtable__ *, int key );

struct __hashtable__ 	 *	init_hash_table( int sz, int dz, int (*)(void *) );
void 				destroy_hash_table( struct __hashtable__ * );
void 				hash_add( struct __hashtable__ *, void * );
void 				hash_del( struct __hashtable__ *, void * );
int  				hash_has( struct __hashtable__ *, void * );
void 			*	hash_get( struct __hashtable__ *, int ky );

#endif/*__HASH_H*/
