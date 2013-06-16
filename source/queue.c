#include "defines.h"
#include "allocate.h"
#include "queue.h"

static queueNode 		_NihilNode;
static queueNode 	*	_NilPtr;

#undef  NIL
#define NIL		_NilPtr

queueType * init_queue( void )
{
	queueType 		*	queue;

	NIL = &_NihilNode;

	queue = (queueType *)errCalloc( sizeof( queueType ) );

	queue->head = queue->tail = NIL;

	NIL->prev = NIL->next = NIL;

	return queue;
}

void destroy_queue( queueType * queue )
{
	while( queue->hasNode ) queue_delete_tail( queue );
}

void queue_delete_tail( queueType * queue )
{
	queueNode		*	node;

	if( queue->hasNode == 0 ) return;

	node = queue->tail;

	node->prev->next = NIL;

	queue->tail = node->prev;

	free_queue( node );

	queue->hasNode--;
}

void queue_delete_head( queueType * queue )
{
	queueNode		*	node;

	if( queue->hasNode == 0 ) return;

	node = queue->head;

	node->next->prev = NIL;

	queue->head = node->next;

	free_queue( node );

	queue->hasNode--;
}

void queue_add( queueType * queue, void * data, int nr )
{
	queueNode		* node;

	node = alloc_queue();
	node->nr = nr;
	node->data = data;

	node->next = NIL;
	node->prev = NIL;

	if( queue->hasNode == 0 )
	{
		queue->head = node;
		queue->tail = node;
	}
	else
	{
		node->next = queue->head;
		queue->head->prev = node;
		queue->head = node;
	}
	queue->hasNode++;
}
