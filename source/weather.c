#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "utils.h"
#include "comm.h"
#include "find.h"
#include "interpreter.h"
#include "variables.h"
#include "weather.h"

struct time_info_data time_info;  /* the infomation about the time   */
struct weather_data weather_info; /* the infomation about the weather*/

/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{   
  long secs;
  struct time_info_data now;
    
  secs = (long) (t2 - t1);
    
  now.hours = (secs/SECS_PER_REAL_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR*now.hours;
    
  now.day = (secs/SECS_PER_REAL_DAY);          /* 0..34 days  */
  secs -= SECS_PER_REAL_DAY*now.day;

  now.month = -1;
  now.year  = -1; 
 
  return now;
}
 
  
/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{   
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs/SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR*now.hours;

  now.day = (secs/SECS_PER_MUD_DAY) % 35;     /* 0..34 days  */
  secs -= SECS_PER_MUD_DAY*now.day;

  now.month = (secs/SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
  secs -= SECS_PER_MUD_MONTH*now.month;

  now.year = (secs/SECS_PER_MUD_YEAR);        /* 0..XX? years */

  return now;
}

struct time_info_data age(charType *ch)  
{
  struct time_info_data player_age;

  player_age = mud_time_passed( time(0), ch->birth );
  player_age.year += 17 + ch->age_mod;   /* All players start at 17 */

  player_age.year = MAX( 17,  player_age.year );

  return player_age;
}

void another_hour(int mode)
{
  	time_info.hours++;

  	if( mode ) 
 	{
    	switch (time_info.hours) 
		{
      	case 5 :
      	{
        weather_info.sunlight = SUN_RISE;
        send_to_outdoor("The sun rises in the east.\n\r");
        break;
      	}
      	case 6 :
      	{
        weather_info.sunlight = SUN_LIGHT;
        send_to_outdoor("The day has begun.\n\r");
        break;  
      	}
      	case 21 :
      	{
        weather_info.sunlight = SUN_SET;
        send_to_outdoor(
        "The sun slowly disappears in the west.\n\r");
        break;
      	}
      	case 22 :
      	{
        weather_info.sunlight = SUN_DARK;
        send_to_outdoor("The night has begun.\n\r");
        break;
      	}
      	default : break;
    	}
  	}

  	if( time_info.hours > 23 )  /* Changed by HHS due to bug ???*/
  	{
    	time_info.hours -= 24;
    	time_info.day++;

    	if( time_info.day > 34 )
    	{
      		time_info.day = 0;
      		time_info.month++;

      		if( time_info.month > 16 )
      		{
        		time_info.month = 0;
        		time_info.year++;
      		}
    	}
  	}
}

void weather_change(void)
{
  	int 		diff, change;

  	if((time_info.month>=9)&&(time_info.month<=16))
    	diff=(weather_info.pressure > 985 ? -2 : 2);
  	else
    	diff=(weather_info.pressure > 1015 ? -2 : 2);

  	weather_info.change += (dice(1,4)*diff+dice(2,6)-dice(2,6));
	
  	weather_info.change = MIN(weather_info.change,12);
  	weather_info.change = MAX(weather_info.change,-12);

  	weather_info.pressure += weather_info.change;

  	weather_info.pressure = MIN(weather_info.pressure,1040);
  	weather_info.pressure = MAX(weather_info.pressure,960);

  	change = 0;

	switch(weather_info.sky)
  	{
    	case SKY_CLOUDLESS :
    	{
      		if (weather_info.pressure<990)
        		change = 1;
      		else if (weather_info.pressure<1010)
        		if(dice(1,4)==1)
          			change = 1;
      		break;
    	}
    	case SKY_CLOUDY :
    	{
      		if (weather_info.pressure<970)
        		change = 2;
      		else if (weather_info.pressure<990)
        		if(dice(1,4)==1)
          			change = 2;
        	else
          		change = 0;
      		else if (weather_info.pressure>1030)
        		if(dice(1,4)==1)
          			change = 3;

      		break;
    	}
    	case SKY_RAINING :
    	{
      		if (weather_info.pressure<970)
        		if(dice(1,4)==1)
          			change = 4;
        		else
          			change = 0;
      		else if (weather_info.pressure>1030)
          		change = 5;
      		else if (weather_info.pressure>1010)
        		if(dice(1,4)==1)
          			change = 5;
	
      		break;
    	}
    	case SKY_LIGHTNING :
    	{
      		if (weather_info.pressure>1010)
          		change = 6;
      		else if (weather_info.pressure>990)
        		if(dice(1,4)==1)
          			change = 6;

      		break;
    	}
    	default : 
    	{
      		change = 0;
      		weather_info.sky=SKY_CLOUDLESS;
      		break;
    	}
  	}

  	switch( change )
  	{
    	case 0 : break;
    	case 1 :
    	{
      	send_to_outdoor( "The sky is getting cloudy.\n\r" );
      	weather_info.sky=SKY_CLOUDY;
      	break;
    	}
    	case 2 :
    	{
      	send_to_outdoor( "It starts to rain.\n\r" );
      	weather_info.sky=SKY_RAINING;
      	break;
    	}
    	case 3 :
    	{
      	send_to_outdoor( "The clouds disappear.\n\r" );
      	weather_info.sky=SKY_CLOUDLESS;
      	break;
    	}
    	case 4 :
    	{
      	send_to_outdoor( "Lightning starts to show in the sky.\n\r" );
      	weather_info.sky=SKY_LIGHTNING;
      	break;
    	}
    	case 5 :
    	{
      	send_to_outdoor( "The rain stopped.\n\r" );
      	weather_info.sky=SKY_CLOUDY;
      	break;
    	}
    	case 6 :
    	{
      	send_to_outdoor( "The lightning has stopped.\n\r" );
      	weather_info.sky=SKY_RAINING;
      	break;
    	}
    	default : break;
  	}
}

void weather_and_time(int mode)
{
  	another_hour( mode );

  	if( mode ) weather_change();
}

void reset_time( void )
{
  	long beginning_of_time = 650336715;

  	time_info = mud_time_passed( time(0), beginning_of_time );

  	switch( time_info.hours )
  	{
    	case 0 : case 1 : case 2 : case 3 : case 4 : 
    	{
      	weather_info.sunlight = SUN_DARK;
      	break;
    	}
    	case 5 :
    	{
      	weather_info.sunlight = SUN_RISE;
      	break;
    	}
    	case 6 : case 7 : case 8 : case 9 : case 10 : case 11 : case 12 : case 13 :
    	case 14 : case 15 : case 16 : case 17 : case 18 : case 19 : case 20 :
    	{
      	weather_info.sunlight = SUN_LIGHT;
      	break;
    	}
    	case 21 :
    	{
      	weather_info.sunlight = SUN_SET;
      	break;
    	}
    	case 22 : case 23 : 
		default :
    	{
      	weather_info.sunlight = SUN_DARK;
      	break;
    	}
  	}

  	log("   Current Gametime: %dH %dD %dM %dY.",
          	time_info.hours, time_info.day, time_info.month, time_info.year);

  	weather_info.pressure = 960;
  	if ((time_info.month>=7)&&(time_info.month<=12)) 	weather_info.pressure += dice(1,50);
  	else 												weather_info.pressure += dice(1,80);

  	weather_info.change = 0;

  	if      (weather_info.pressure<=980)  weather_info.sky = SKY_LIGHTNING;
  	else if (weather_info.pressure<=1000) weather_info.sky = SKY_RAINING;
  	else if (weather_info.pressure<=1020) weather_info.sky = SKY_CLOUDY;
  	else 								  weather_info.sky = SKY_CLOUDLESS;
}
