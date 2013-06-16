#ifndef __WEATHER_H
#define __WEATHER_H

#ifndef __DEFINES_H
#include "defines.h"
#endif

#ifndef __CHARACTER_H
#include "character.h"
#endif

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR  60
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

/* Sun clock */

#define SUN_DARK    0
#define SUN_RISE    1
#define SUN_LIGHT   2
#define SUN_SET     3

/* And how is the sky ? */

#define SKY_CLOUDLESS   0
#define SKY_CLOUDY      1
#define SKY_RAINING     2
#define SKY_LIGHTNING   3

struct time_info_data  
{ 
  	unsigned char 	hours, 
					day, 
					month;
	int 			year;
};
	  
struct weather_data 
{   
  	int 	pressure;  	/* How is the pressure ( Mb ) */
  	int 	change;  	/* How fast and what way does it change. */
  	int 	sky;  		/* How is the sky. */
  	int 	sunlight;  	/* And how much sun. */
};
  
void 					weather_and_time	( int mode );
void 					reset_time			( void );
struct time_info_data 	mud_time_passed		( time_t t2, time_t t1 );
struct time_info_data 	real_time_passed	( time_t t2, time_t t1 );
struct time_info_data 	age					( charType  *	ch );

extern struct time_info_data time_info;  /* the infomation about the time   */
extern struct weather_data weather_info; /* the infomation about the weather*/

#endif
