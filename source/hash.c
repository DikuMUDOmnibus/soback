#include "defines.h"
#include "hash.h"
#include "allocate.h"

fixedHashTable * init_fixedHashTable( void )
{
	fixedHashTable * table;

	table = (fixedHashTable *)errCalloc( sizeof( fixedHashTable ) );

	return table;
}

void destroy_fixedHashTable( fixedHashTable * table )
{
	hashType	*	tmp, * tmp_next;
	int				i;

	for( i = 0; i < FIXED_HASH_SIZE; i++ )
	{
		for( tmp = table->table[i]; tmp; tmp = tmp_next )
		{
			tmp_next = tmp->next;

			free_hashType( tmp );
		}
	}

	errFree( table );
}

void fixedHash_add( fixedHashTable * table, int data )
{
	hashType	*	tmp;
	int		i;

	i = FIXED_HVAL( data );

	tmp = alloc_hashType();

	tmp->key  = data;

	tmp->next = table->table[i];
	table->table[i] = tmp;
}

int fixedHash_has( fixedHashTable * table, int key )
{
	hashType	*	tmp;
	int				i;

	i = FIXED_HVAL( key );

	for( tmp = table->table[i]; tmp; tmp = tmp->next )
			if( tmp->key == key ) return 1;

	return 0;
}
