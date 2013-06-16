#ifndef __PAGE_H
#define __PAGE_H

#define	PAGE_MODE	2

typedef struct __line__
{
    int                     nr;
    char                *   line;
    struct __line__     *   next;

} lineType; 

typedef struct __pagetype__      
{
    int                         hasLine;     
    lineType                *   lines;

    struct  __pagetype__    *   prev;
    struct  __pagetype__    *   next;

} pageType;

typedef struct __pageunit__
{
	struct __pagetype__		*	pages;
	struct __pagetype__		*	curr;

	int							hasPage;
	int							screen;
	char					*	who;

	struct __pageunit__		*	next;

} pageUnitType;

pageUnitType 	*	find_pageunit( charType * ch );
void				del_pageunit( pageUnitType * );
void				page_interpreter( charType * ch, char * argument );
void				page_prompt( descriptorType * );

pageUnitType    *	new_pageunit( charType * ch );

void				pagef( pageUnitType *, char *, ... );
void				print_page( charType * ch, char * );
void				show_page( charType *, pageUnitType * );

#endif/*__PAGE_H*/
