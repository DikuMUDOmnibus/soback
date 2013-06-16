/* 
 *  File: sund_procs.c                          Part of Exile MUD
 *  
 *  Special procedures for the mobs and objects of Sundhaven.
 *
 *  Exile MUD is based on CircleMUD, Copyright (C) 1993, 1994.
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.       
 *
 */


/*  Mercy's Procs for the Town of Sundhaven  */

#include "defines.h"
#include "character.h"
#include "object.h"
#include "world.h"
#include "affect.h"
#include "magics.h"
#include "interpreter.h"
#include "specials.h"
#include "comm.h"
#include "utils.h"
#include "fight.h"

int silktrader( charType * ch, int cmd, char * argu )
{
  	if( cmd ) return 0;

  	if( world[ch->in_room].sector == SECT_CITY )
  	switch( number(0, 30) ) 
  	{
   	case 0:
      act("$n eyes a passing woman.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Come, m'lady, and have a look at this precious silk!", 0);
      return(1);
   	case 1:
      act("$n says to you, 'Wouldn't you look lovely in this!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n shows you a gown of indigo silk.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 2:
      act("$n holds a pair of silk gloves up for you to inspect.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 3:
      act("$n cries out,"
	  	  " 'Have at this fine silk from exotic corners of the world you will likely never see!", 
			FALSE, ch, 0, 0,TO_ROOM);
      act("$n smirks.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 4:
      do_say(ch, "Step forward, my pretty locals!", 0);
      return(1);
   	case 5:
      act("$n shades his eyes with his hand.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 6:
      do_say(ch, "Have you ever seen an ogre in a silken gown?", 0);
      do_say(ch, "I didn't *think* so!", 0);
      act("$n throws his head back and cackles with insane glee!", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 7:
      act("$n hands you a glass of wine.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Come, have a seat and view my wares.", 0);
      return(1);
   	case 8:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n shakes his head sadly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 9:
      act("$n fiddles with some maps.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 10:
      do_say(ch, "Here here! Beggars and nobles alike come forward and make your bids!", 0);
      return(1);
   	case 11:
      do_say(ch, "I am in this bourgeois hamlet for a limited time only!", 0);
      act("$n swirls some wine in a glass.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
  	}

  	if( world[ch->in_room].sector != SECT_CITY )
  	switch( number(0, 20) ) 
  	{
   	case 0:
      do_say(ch, "Ah! Fellow travellers!"
				 " Come have a look at the finest silk this side of the infamous Ched Razimtheth!", 0);
      return(1);
   case 1:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "You are feebly attired for the danger that lies ahead.", 0);
      do_say(ch, "Silk is the way to go.", 0);
      act("$n smiles warmly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 2:
      do_say(ch, "Worthy adventurers, hear my call!", 0);
      return(1);
   case 3:
      act("$n adjusts his cloak.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 4:
      act("$n says to you, 'Certain doom awaits you, therefore shall you die in silk.'", 
		  FALSE, ch, 0, 0,TO_ROOM);
      act("$n bows respectfully.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 5:
      do_say(ch, "Can you direct me to the nearest tavern?", 0);
      return(1);
   case 6:
      do_say(ch, "Heard the latest ogre joke?", 0);
      act("$n snickers to himself.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 7:
      do_say(ch, "What ho, traveller!"
				 " Rest your legs here for a spell and peruse the latest in fashion!", 0);
      return(1);
   case 8:
      do_say(ch, "Beware ye, traveller, lest ye come to live in Exile!", 0);
      act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 9:
      act("$n touches your shoulder.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "A word of advice. Beware of any ale labled 'mushroom' or 'pumpkin'.", 0);
      act("$n shivers uncomfortably.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
  	}
  	return(0);
}

int athos( charType * ch, int cmd, char * argu )
{
  	if( cmd ) return 0;
    switch (number(0, 20)) 
	{
    case 0:
      act("$n gazes into his wine gloomily.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
    case 1:
      act("$n grimaces.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
    case 2:
      act("$n asks you, 'Have you seen the lady, pale and fair, with a heart of stone?'", 
	  	FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "That monster will be the death of us all.", 0);
      return(1);
    case 3:
      do_say(ch, "God save the King!", 0);
      return(1);
    case 4:
      do_say(ch, "All for one and .. one for...",  0);
      act("$n drowns himself in a swig of wine.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
    case 5:
      act("$n looks up with a philosophical air.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Women - God's eternal punishment on man.", 0);
      return(1);
    case 6:
      act("$n downs his glass and leans heavily on the oaken table.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "You know, we would best band together"
				 " and wrestle the monstrous woman from her lair and home!", 0);
      return(1);
	}
    return(0);
}

int hangman( charType * ch, int cmd, char * argu )
{
	if(cmd) return 0;

  	switch( number(1, 20) ) 
  	{
  	case 5:
    	act("$n whirls his noose like a lasso and it lands neatly around your neck.", 
			FALSE, ch, 0, 0,TO_ROOM);
    	do_say(ch, "You're next, you ugly rogue!", 0);
    	do_say(ch, "Just kidding.", 0);
    	act("$n pats you on your head.", FALSE, ch, 0, 0,TO_ROOM);
    	return(1);
  	case 1:
    	do_say(ch, "I was conceived in SOBACK and have been integrated into society!", 0);
    	do_say(ch, "Muahaha!", 0);
    	return(1);
  	case 2:
    	do_say(ch, "Anyone have a butterknife I can borrow?", 0);
    	return(1);
  	case 3:
    	act("$n suddenly pulls a lever.", FALSE, ch, 0, 0,TO_ROOM);
    	act("With the flash of light on metal a giant guillotine comes crashing down!", 
			FALSE, ch, 0, 0,TO_ROOM);
    	act("A head drops to the ground from the platform.", FALSE, ch, 0, 0,TO_ROOM);
    	act("$n looks up and shouts wildly.", FALSE, ch, 0, 0,TO_ROOM);
    	act("$n shouts, 'Next!'", FALSE, ch, 0, 0, TO_ROOM); 
    	return(1);
  	case 4:
   		act("$n whistles a local tune.", FALSE, ch, 0, 0,TO_ROOM);
   		return(1);
  	}
  	return(0);
}   

int butcher( charType * ch, int cmd, char * argu )
{
	if( cmd ) return 0;

  	switch( number(0, 40) ) 
  	{
   	case 0:
      do_say(ch, "I need a Union.", 0);
      act("$n glares angrily.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n rummages about for an axe.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 1:
      act("$n gnaws on a toothpick.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 2:
      act("$n runs a finger along the edge of a giant meat cleaver.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 3:
      do_say(ch, "Pork for sale!", 0);
      return(1);
   	case 4:
      act("$n whispers to you,"
	  	  " 'I've got some great damage eq in the back room. Wanna see?'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n throws back his head and cackles with insane glee!", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 5:
      act("$n yawns.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 6:
      act("$n throws an arm around the headless body"
		  " of an ogre and asks to have his picture taken.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 7:
      act("$n listlessly grabs a cleaver and hurls"
		  " it into the wall behind your head.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 8:
      act("$n juggles some fingers.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 9:
      act("$n eyes your limbs.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n chuckles.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 10:
      do_say(ch, "Hi, Alice.", 0);
      return(1);
   	case 11:
      do_say(ch, "Everyone looks like food to me these days.", 0);
      act("$n sighs loudly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 12:
      act("$n throws up his head and shouts wildly.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n shouts, 'Bring out your dead!'", FALSE, ch, 0, 0, TO_ROOM);
      return(1);
   	case 13:
      do_say(ch, "The worms crawl in, the worms crawl out..", 0);
      return(1);
   	case 14:
      act("$n sings 'Brave, brave Sir Patton...'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n whistles a tune.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n smirks.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   	case 15:
      do_say(ch, "Get Lurch to bring me over a case"
				 " and I'll sport you a year's supply of grilled ogre.", 0);
      return(1);
	}
    return(0);
}

int stu( charType * ch, int cmd, char * argu )
{
  	if( cmd ) return 0;

  	switch(number(0, 60)) 
  	{
    case 0:
      do_say(ch, "I'm so damn cool, I'm too cool to hang out with myself!", 0);
      break;
    case 1:
      do_say(ch, "I'm really the NICEST guy you ever MEET!", 0);
      break;
    case 2:
      do_say(ch, "Follow me for exp, gold and lessons in ADVANCED C!", 0);
      break;
    case 3:
      do_say(ch, "Mind if I upload 200 megs of"
				 " pregnant XXX gifs with no descriptions to your bbs?", 0);
      break;
    case 4:
      do_say(ch, "Sex? No way! I'd rather jog 20 miles!", 0);
      break;
    case 5:
      do_say(ch, "I'll take you OUT!!   ...tomorrow", 0);
      break;
    case 6:
      do_say(ch, "I invented Mud you know...", 0);
      break;
    case 7:
      do_say(ch, "Can I have a cup of water?", 0);
      break;
    case 8:
      do_say(ch, "I'll be jogging down ventnor ave in 10 minutes if you want some!", 0);
      break;
    case 9:
      do_say(ch, "Just let me pull a few strings"
				 " and I'll get ya a site, they love me! - doesnt everyone?", 0);
      break;
    case 10:
      do_say(ch, "Pssst! Someone tell Mercy to sport me some levels.", 0);
      act("$n nudges you with his elbow.", FALSE, ch, 0, 0,TO_ROOM);
      break;
    case 11:
      do_say(ch, "Edgar! Buddy! Let's group and hack some ogres to tiny quivering bits!", 0);
      break;
    case 12:
      act("$n tells you, 'Skylar has bad taste in women!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n screams in terror!", FALSE, ch, 0, 0,TO_ROOM);
      do_flee(ch, 0, 0);
      break;
    case 13:
      act("$n whispers to you, 'Dude! If you fucking say 'argle bargle'"
		  " to the glowing fido he'll raise you a level!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n flexes.", FALSE, ch, 0, 0,TO_ROOM);
	}
  	return 0;
}

int sund_earl( charType * ch, int cmd, char * argu )
{
  	if( cmd ) return(FALSE);

  	switch( number(0, 20) ) 
  	{
   	case 0:
      	do_say(ch, "Lovely weather today.", 0);
      	return(1);
   	case 1:
    	act("$n practices a lunge with an imaginary foe.", FALSE, ch, 0, 0,TO_ROOM);
      	return(1);
   	case 2:
      	do_say(ch, "Hot performance at the gallows tonight.", 0);
     	act("$n winks suggestively.", FALSE, ch, 0, 0,TO_ROOM); 
     	return(1);
   	case 3:
      	do_say(ch, "Must remember to up the taxes at my convenience.", 0);
      	return(1);
   	case 4:
      	do_say(ch, "Sundhaven is impermeable to the enemy!", 0);
      	act("$n growls menacingly.", FALSE, ch, 0, 0,TO_ROOM);
      	return(1);
 	case 5:
      	do_say(ch, "Decadence is the credence of the abominable.", 0);
      	return(1);
 	case 6:
      	do_say(ch, "I look at you and get a wonderful sense of impending doom.", 0);
      	act("$n chortles merrily.", FALSE, ch, 0, 0,TO_ROOM);
      	return(1);
 	case 7:
      	act("$n touches his goatee ponderously.", FALSE, ch, 0, 0,TO_ROOM);
      	return(1);
 	case 8:
      	do_say(ch, "It's Mexican Madness night at Maynards!", 0);
      	act("$n bounces around.", FALSE, ch, 0, 0, TO_ROOM);
      	return(1); 
 	}
 	return 0;
}

int blinder( charType * ch, int cmd, char * argu )
{
	affectType		af;

  	if(cmd) return FALSE;

  	if(  is_fighting(ch) 
	  && (number(0, 100)+GET_LEVEL(ch) >= 50) ) 
	{
    	act("$n whispers, 'So, $N! You wouldst share my affliction!", 1, ch, 0, ch->fight, TO_NOTVICT);
    	act("$n whispers, 'So, $N! You wouldst share my affliction!", 1, ch, 0, ch->fight, TO_VICT);
    	act("$n's frayed cloak blows as he points at $N.", 1, ch, 0, ch->fight, TO_NOTVICT);
    	act("$n's frayed cloak blows as he aims a bony finger at you.", 1, ch, 0, ch->fight, TO_VICT);
    	act("A flash of pale fire explodes in $N's face!", 1, ch, 0, ch->fight, TO_NOTVICT);
    	act("A flash of pale fire explodes in your face!", 1, ch, 0, ch->fight, TO_VICT);
	
    	af.type      = SPELL_BLINDNESS;
    	af.location  = APPLY_HITROLL;
    	af.modifier  = -10;  /* Make hitroll worse */
    	af.duration  = 1;
    	af.bitvector = AFF_BLIND;
    	affect_to_char( ch->fight, &af );

    	af.location = APPLY_AC;
    	af.modifier = 50;
    	af.duration  = 1;
    	affect_to_char( ch->fight, &af );
      
    	act("$n seems to be blinded!", TRUE, ch->fight, 0, 0, TO_ROOM);
    	sendf( ch->fight, "You have been blinded!" );

    	return TRUE;
  	}
  	return FALSE;
}

int idiot( charType * ch, int cmd, char * argu )
{
	if(cmd) return FALSE;

  	switch (number(0, 40)) 
  	{
   	case 0:
      do_say(ch, "even if idiot = god", 0);
      do_say(ch, "and Stu = idiot", 0);
      do_say(ch, "Stu could still not = god.", 0);
      act("$n smiles.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   	case 1:
      act("$n balances a newbie sword on his head.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   	case 2:
      act("$n doesn't think you could stand up to him in a duel.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   	case 3:
      do_say(ch, "Rome really was built in a day.", 0);
      act("$n snickers.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   	case 4:
      act("$n flips over and walks around on his hands.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   	case 5:
      act("$n cartwheels around the room.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   	case 6:
      do_say(ch, "How many ogres does it take to screw in a light bulb?", 0);
      act("$n stops and whaps himself upside the head.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   	case 7:
      do_say(ch, "Uh huh. Uh huh huh.", 0);
      return TRUE;
   	case 8:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n whistles quietly.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   	case 9:
      act("$n taps out a tune on your forehead.", FALSE, ch, 0, 0, TO_ROOM);
      return TRUE;
   	case 10:
      act("$n has a battle of wits with himself and comes out unharmed.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   	case 11:
      do_say(ch, "All this and I am just a number.", 0);
      act("$n cries on your shoulder.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   	case 12:
      do_say(ch, "A certain hunchback I know dresses very similar to you, very similar...", 0);
      return TRUE;
  	}
 	return FALSE;
}
     
int marbles( charType * ch, int cmd, char * argu )
{
	if(  cmd == COM_NORTH
	  || cmd == COM_EAST
	  || cmd == COM_SOUTH
	  || cmd == COM_WEST
	  || cmd == COM_UP 	  
	  || cmd == COM_DOWN )
	{
    	if( number(1, 100) + GET_DEX(ch) > 50)
		{
      		act("You slip on marbles and fall.", FALSE, ch, 0, 0, TO_CHAR);
      		act("$n slips on marbles and falls.", FALSE, ch, 0, 0, TO_ROOM);
      		GET_POS(ch) = POSITION_SITTING;
      		return 1;
    	}
    	else 
		{
      		act("You slip on marbles, but manage to retain your balance.", FALSE, ch, 0, 0, TO_CHAR);
      		act("$n slips on marbles, but manages to retain $s balance.", FALSE, ch, 0, 0, TO_ROOM);
    	}
  	}
  	return 0;
}
