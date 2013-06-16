#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include "structs.h"
#include "defines.h"
#include "macros.h"
#include "db.h"

FILE *obj_f;                     

struct index_data *obj_index;   

int top_of_objt = 0;           

char outfile[128];

int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
		c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;

    if ( c == '+' ) 		c = getc( fp );
    else if ( c == '-' ) 	c = getc( fp ), sign = 1;

    if ( !isdigit(c) )
    {
	 	fprintf( stderr, "fread_number> bad format.", 0 );
		exit( 1 );
    }

    while ( isdigit(c) )
    {
		number = number * 10 + c - '0';
		c      = getc( fp );
    }

    if( sign )	number = 0 - number;

    if( c == '|' ) 		number += fread_number( fp );
    else if( c != ' ' ) 	ungetc( c, fp );

    return number;
}


char *fread_string( FILE *fl)
{
  char buf[MAX_STRING_LENGTH], tmp[500];
  char *rslt;
  register char *point;
  int flag;

  memset(buf, 0, MAX_STRING_LENGTH);
  do
  {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl))
    {
      perror("fread_str");
      exit(1);
    }
    if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
    {
      printf("fread_string: string too large (db.c)");
      buf[70]=0;
      fprintf(stderr,"%s\n",buf);
      exit(1);
    }
    else
      strcat(buf, tmp);

    for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
      point--);    
    if((flag = (*point == '~')))
      if (*(buf + strlen(buf) - 3) == '\n')
      {
        *(buf + strlen(buf) - 2) = '\r';
        *(buf + strlen(buf) - 1) = '\0';
      }
      else
        *(buf + strlen(buf) -2) = '\0';
    else
    {
      *(buf + strlen(buf) + 1) = '\0';
      *(buf + strlen(buf)) = '\r';
    }
  }
  while (!flag);

  /* do the allocate boogie  */

  if (strlen(buf) > 0)
  {
	rslt = malloc(strlen(buf) + 1) ;
    strcpy(rslt, buf);
  }
  else
    rslt = 0;
  return(rslt);
}

struct index_data *generate_indices(FILE *fl, int *top)
{
  int i = 0;
  struct index_data *index = 0;
  char buf[82];

  rewind(fl);

  for (;;)
  {
    if (fgets(buf, 81, fl))
    {
      if (*buf == '#')
      {
        /* allocate new cell */
        
        if (!i)             /* first cell */
          index = malloc( sizeof(struct index_data) );
        else
          if (!(index = 
            (struct index_data*)realloc(index, 
            (i + 1) * sizeof(struct index_data))))
          {
            perror("load indices");
            exit(1);
           }
        sscanf(buf, "#%d", &index[i].virtual);
        index[i].pos = ftell(fl);
        index[i].number = 0;
        index[i].func = 0;
        i++;
      }
      else 
        if (*buf == '$')  /* EOF */
          break;
    }
    else
    {
      perror("generate indices");
      exit(1);
    }
  }
  *top = i - 2;
  return(index);
}

struct obj_data *read_object( int nr, int a )
{
  	struct obj_data *obj;
  	int tmp, i;
  	char chk[50], buf[100];
  	struct extra_descr_data *new_descr;

  	i = nr;

  	fseek(obj_f, obj_index[nr].pos, 0);

  	/* create(obj, struct obj_data, 1); */
	obj = (struct obj_data *)calloc( sizeof(struct obj_data), 1 );

  	/* *** string data *** */

	obj->item_number = obj_index[nr].virtual;
  	obj->name = fread_string(obj_f);
  	obj->short_description = fread_string(obj_f);
  	obj->description = fread_string(obj_f);
  	obj->action_description = fread_string(obj_f);

  	/* *** numeric data *** */

  	fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.type_flag = tmp; 
  	obj->obj_flags.extra_flags = fread_number( obj_f );
  	obj->obj_flags.wear_flags  = fread_number( obj_f );
  	fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.value[0] = tmp;
  	fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.value[1] = tmp;
  	fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.value[2] = tmp;
  	fscanf(obj_f, " %d\n", &tmp);  obj->obj_flags.value[3] = tmp;

  	fgets( buf, 100, obj_f );

  	if( sscanf( buf, "%ld %ld %ld %d", 
  			&obj->obj_flags.weight, &obj->obj_flags.cost, 
			&obj->obj_flags.level,  &obj->obj_flags.unused ) == 3 )
		;
	else obj->obj_flags.unused = 0;

  	/**** extra descriptions *** */

  	obj->ex_description = 0;

  	while (fscanf(obj_f, " %s \n", chk), *chk == 'E')
  	{
		new_descr = (struct extra_descr_data *)malloc( sizeof(struct
					 extra_descr_data) );

		new_descr->keyword = fread_string(obj_f);
		new_descr->description = fread_string(obj_f);

		new_descr->next = obj->ex_description;
		obj->ex_description = new_descr;
	}

  	for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)
  	{
    	fscanf(obj_f, " %d ", &tmp);
    	obj->affected[i].location = tmp;
    	fscanf(obj_f, " %d \n", &tmp);
    	obj->affected[i].modifier = tmp;
    	fscanf(obj_f, " %s \n", chk);
  	}

  for (;(i < MAX_OBJ_AFFECT);i++)
  {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }

  obj_index[nr].number++;

  return (obj);  
}

void fwrite_string( FILE * fp, char * str )
{
	char	buf[MAX_STRING_LENGTH];
	char	* cp, *wp;

	if( !str )
	{
		fprintf( fp, "~\n", buf );
	}
	else
	{
		wp = buf; 		cp = str;

		for(; *cp; cp++ ) if( *cp != '\r' ) *wp++ = *cp;

		*wp = 0;

		fprintf( fp, "%s~\n", buf );
	}
}

void object_to_file( struct obj_data * obj, FILE * fp )
{
	int i;
	struct extra_descr_data * wext;

	fprintf( fp, "#%d\n", obj->item_number ); 
    fwrite_string( fp, obj->name);
    fwrite_string( fp, obj->short_description);
    fwrite_string( fp, obj->description);
    fwrite_string( fp, obj->action_description);
    
   	fprintf( fp, "%u %lu %lu\n",
		obj->obj_flags.type_flag,
		obj->obj_flags.extra_flags,
		obj->obj_flags.wear_flags);

   	fprintf( fp, "%d %d %d %d\n", 
		obj->obj_flags.value[0],
		obj->obj_flags.value[1],
		obj->obj_flags.value[2],
		obj->obj_flags.value[3]	);

	if( obj->obj_flags.level < 0 || obj->obj_flags.level > 40 )
		obj->obj_flags.level = 0;

    fprintf( fp, "%u %lu %d %d\n",
        obj->obj_flags.weight,obj->obj_flags.cost,
        obj->obj_flags.level, obj->obj_flags.unused );
      

    wext = obj->ex_description;
    while(wext!=NULL) 
    {
    	fprintf( fp,"E\n");
    	fwrite_string( fp, wext->keyword);
    	fwrite_string( fp, wext->description);
    	wext=wext->next;
    }
    
    for( i = 0 ; (i < MAX_OBJ_AFFECT); i++)
    {
    	if(obj->affected[i].location!=0)
    	{
      		fprintf( fp, "A\n");
      		fprintf( fp, "%d %d\n",
				obj->affected[i].location,
				obj->affected[i].modifier); 
    	}       
   }     
}

void convert( void )
{
	struct obj_data * obj;
	FILE * fp;
	int    i;


	if( fp = fopen( outfile, "w" ), !fp )
	{
		perror( "outfile." );
		exit(1);
	}


	for( i = 0; i <= top_of_objt; i++ )
	{
		obj = read_object( i, 0 );
		object_to_file( obj, fp );
	}

	fprintf( fp, "#20000\n$~\n\n" );
}

void main( int argc, char ** argv )
{
	char fname[100];
  	int i;

  	if( argc == 2 )
  	{
  		strcpy( fname, argv[1] );
  		strcat( fname, ".obj" );
  		strcpy( outfile, fname );
  		strcat( outfile, ".new" );
  	}
  	else if( argc == 1 )
  	{
  		strcpy( fname, "tinyworld.obj" );
  		strcpy( outfile, "tinyworld.obj.new" );
  	}
  	else
  	{
  		printf( "%s > obj file prefix.\n" );
  		return;
  	}

  	printf("Boot db -- BEGIN.\n");

  	if(!(obj_f = fopen( fname ,  "r")))
  	{
  		perror( "Opening input file : " );
    	exit(1);
  	}

  	printf("Generating index tables for %s files.\n", fname );
  	obj_index = generate_indices(obj_f, &top_of_objt);
  	printf("Index   -- done.\n");
	printf("%s ===> %s.", fname, outfile );
  	convert();
  	printf("Convert -- DONE.\n");
}
