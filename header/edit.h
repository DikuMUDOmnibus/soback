#ifndef __EDIT_H

/* mode */
#define		EDIT_MODE	1

/* sub mode 1 */
#define     EDIT_NONE   0
#define     EDIT_OBJ    1
#define     EDIT_MOB    2
#define     EDIT_ROOM   3
#define     EDIT_ZONE   4

/* sub mode 3 */
#define     EDIT_YESNO  1
#define     EDIT_YES    2
#define     EDIT_NO     3

typedef struct __edit_unit__
{
    char            	*   who;
    int                 	obj;
    int                 	mob;
    int                 	room;
    int                 	zone;
	int						dir;
	int						extr;
	char				*	prompt;
    objIndexType    	*   oe;
    roomType        	*   re;  
    mobIndexType    	*   me;
	zoneType			*	ze;
	exdescriptionType 	* 	xe;
	directionType		*	de;

    struct __edit_unit__    *   next;
 
} editUnitType;

void 			ed_new_prompt		( editUnitType * eu, char * prompt );
void 			ed_save_edit		( charType * ch, editUnitType * eu );
int				ed_is_modified		( charType * ch, editUnitType * eu );
void 			edit_interpreter	( charType * ch, char * argument );
void			edit_prompt			( descriptorType * );
int				find_editing		( charType * ch, int type, int nr );
editUnitType * 	new_editing			( charType * ch, int type, int nr );
void 			del_editing			( editUnitType * del );
editUnitType * 	find_editunit		( charType * ch );

void 	ed_help 	( charType * ch, char * argu, int cmd );       
void 	ed_quit 	( charType * ch, char * argu, int cmd );       
void 	ed_iedit	( charType * ch, char * argu, int cmd ); 
void 	ed_icopy	( charType * ch, char * argu, int cmd );
void 	ed_medit	( charType * ch, char * argu, int cmd );
void 	ed_mcopy	( charType * ch, char * argu, int cmd );
void 	ed_redit	( charType * ch, char * argu, int cmd );
void 	ed_rcopy	( charType * ch, char * argu, int cmd );
void 	ed_zedit	( charType * ch, char * argu, int cmd );
   
#endif/*__EDIT_H*/
