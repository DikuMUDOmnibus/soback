/* ************************************************************************
*  file: spec_assign.c , Special module.                  Part of DIKUMUD *
*  Usage: Procedures assigning funfunfunc pointers.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include "defines.h"
#include "character.h"
#include "world.h"
#include "mobile.h"
#include "misc.h"
#include "variables.h"
#include "strings.h"
#include "specials.h"

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
  	mobiles[real_mobileNr(1   )].func = puff;

  	/* Moksha */
  	mobiles[real_mobileNr(1650)].func = spell_blocker;
/*  mobiles[real_mobileNr(2836)].func = spell_blocker; */
  	mobiles[real_mobileNr(11364)].func = spell_blocker;

  	/* Wasteland */
  	mobiles[real_mobileNr(2123)].func = fido;
  
  	/* Midgaard */
  	mobiles[real_mobileNr(3061)].func = janitor;
  	mobiles[real_mobileNr(3066)].func = fido;
  	mobiles[real_mobileNr(3062)].func = fido;

  	mobiles[real_mobileNr(3020)].func = guild;
  	mobiles[real_mobileNr(3143)].func = mayor;
/* 	mobiles[real_mobileNr(3125)].func = deathcure;
*/
  	mobiles[real_mobileNr(3135)].func = super_deathcure;
  	mobiles[real_mobileNr(3126)].func = mud_message;
  	mobiles[real_mobileNr(3127)].func = mud_message;
  	mobiles[real_mobileNr(3128)].func = mud_message;
  	mobiles[real_mobileNr(3129)].func = mud_message;
  	mobiles[real_mobileNr(3130)].func = mud_message;
  	mobiles[real_mobileNr(3131)].func = mud_message;
  	mobiles[real_mobileNr(3132)].func = mud_message;
  	mobiles[real_mobileNr(3134)].func = Quest_bombard; 
/* 	mobiles[real_mobileNr(3145)].func = mom;
*/
  	mobiles[real_mobileNr(1455)].func = singer;

  	/* MORIA */
  	mobiles[real_mobileNr(4000)].func = snake;
  	mobiles[real_mobileNr(4001)].func = snake;
  	mobiles[real_mobileNr(4053)].func = snake;
/*  mobiles[real_mobileNr(1660)].func = snake;
*/
  	mobiles[real_mobileNr(5004)].func = snake;
  	mobiles[real_mobileNr(4102)].func = snake;

  	/* SEWERS */
  	mobiles[real_mobileNr(7006)].func = snake;

  	/* FOREST */
  	mobiles[real_mobileNr(6113)].func = snake;
  	mobiles[real_mobileNr(6114)].func = snake;


  	/* Death Kingdom : cyb */
  	mobiles[real_mobileNr(9528)].func = helper;   /* barbaror */
  	mobiles[real_mobileNr(9562)].func = helper;	/* yuria */
  	mobiles[real_mobileNr(9509)].func = laiger;   /* laiger */
  	mobiles[real_mobileNr(9525)].func = gaiot;    /* gaiot */
  	mobiles[real_mobileNr(9561)].func = december; /* gaiot */
/*  mobiles[real_mobileNr(9539)].func = black_yacha;  black yacha 
*/

  	/* Robo city : big cyb */
  	mobiles[real_mobileNr(15182)].func = great_mazinga ;	/* great mazinga */
/* 	mobiles[real_mobileNr(15023)].func = teleport_machine ;   teleport machine */

  	/* Mud School : big cyb */
  	mobiles[real_mobileNr(2904)].func = school_cold;	
  	mobiles[real_mobileNr(2903)].func = school_gracia;
  	mobiles[real_mobileNr(2905)].func = school_nara;
/*
  	mobiles[real_mobileNr(3155)].func = teof; 
*/
	mobiles[real_mobileNr(3068)].func = mikesmith;

	mobiles[real_mobileNr(6600)].func = sund_earl;
	mobiles[real_mobileNr(6602)].func = hangman;
	mobiles[real_mobileNr(6606)].func = fido;
	mobiles[real_mobileNr(6666)].func = stu;
	mobiles[real_mobileNr(6648)].func = butcher;
	mobiles[real_mobileNr(6661)].func = blinder;
	mobiles[real_mobileNr(6637)].func = silktrader;
	mobiles[real_mobileNr(6615)].func = idiot;
	mobiles[real_mobileNr(6653)].func = athos;

	mobiles[real_mobileNr(18216)].func = sphinx;

	mobiles[real_mobileNr(10000)].func = daimyo;
  	mobiles[real_mobileNr(3133)].func = musashi; 
}

void assign_objects(void)
{
  	objects[real_objectNr(3077)].func = information;
  	objects[real_objectNr(3099)].func = board;
  	objects[real_objectNr(3098)].func = mbox;
  	objects[real_objectNr(1311)].func = totem;
  	objects[real_objectNr(2157)].func = magicseed;

  	objects[real_objectNr(6647)].func = marbles;

/* 	objects[real_objectNr(12100)].func = chamber;
*/
}

void assign_rooms(void)
{
  	world[real_roomNr(3030)].func = dump;
/*
  	world[real_roomNr(1453)].func = level_gate;
  	world[real_roomNr(2535)].func = level_gate;
  	world[real_roomNr(3001)].func = level_gate;
  	world[real_roomNr(3500)].func = level_gate;
  	world[real_roomNr(5200)].func = level_gate;
  	world[real_roomNr(6001)].func = level_gate;
  	world[real_roomNr(9400)].func = level_gate;
*/
  	world[real_roomNr(3031)].func = pet_shops;
  	world[real_roomNr(3060)].func = hospital;
  	world[real_roomNr(3065)].func = metahospital;
  	world[real_roomNr(   1)].func = safe_house;
  	world[real_roomNr(3001)].func = safe_house;
  	world[real_roomNr(3308)].func = safe_house;
  	world[real_roomNr(6599)].func = death_house;
  	world[real_roomNr(3096)].func = death_house;
  	world[real_roomNr(2158)].func = portal;
  	world[real_roomNr(2707)].func = neverland;

  	world[real_roomNr(15125)].func = electric_shock;
  	world[real_roomNr(15135)].func = electric_shock;
  	world[real_roomNr(15138)].func = electric_shock;
  	world[real_roomNr(15148)].func = electric_shock;
  	world[real_roomNr(15116)].func = electric_shock;
  	world[real_roomNr(15117)].func = electric_shock;
  	world[real_roomNr(15126)].func = electric_shock;
  	world[real_roomNr(15127)].func = electric_shock;
  	world[real_roomNr(15128)].func = electric_shock;
  	world[real_roomNr(15129)].func = electric_shock;
  	world[real_roomNr(15136)].func = electric_shock;
  	world[real_roomNr(15140)].func = electric_shock;
  	world[real_roomNr(15139)].func = electric_shock;
  	world[real_roomNr(15149)].func = electric_shock;
  	world[real_roomNr(15150)].func = electric_shock;
  	world[real_roomNr(15151)].func = electric_shock;
  	world[real_roomNr(15152)].func = electric_shock;
  	world[real_roomNr(15162)].func = electric_shock;

/* world[real_roomNr(3095)].func = control_center;
*/
}
