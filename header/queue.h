#ifndef __QUEUE_H
#define __QUEUE_H

typedef struct __queuenode__
{
	int							unit;
	int							nr;
	void					*	data;
	struct __queuenode__	* 	prev;
	struct __queuenode__	* 	next;

} queueNode;


typedef struct __queuetype__
{
	int							hasNode;
	struct __queuenode__	* 	head;
	struct __queuenode__	* 	tail;

} queueType;

struct __queuetype__ * 	init_queue( void );
void					destroy_queue( struct __queuetype__ * );

void					queue_add( struct __queuetype__*, void * data, int nr );
void					queue_delete_head( struct __queuetype__ * );
void					queue_delete_tail( struct __queuetype__ * );

#endif/*__QUEUE_H*/
