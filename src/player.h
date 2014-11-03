#ifndef __PLAYER__
#define __PLAYER__

#include "entity.h"


#define MAXITEMS  16
#define MAXEVENTS 128

enum P_Colors {PC_Core1,PC_Core2,PC_Gun,PC_Leg1,PC_Leg2,PC_Leg3,PC_Trail};

/*
    notes :
    TO keep track of accomplishments, the player's data structure will need to keep track of 
    who asked for an event, its complete status (successful, failed, abandoned)
*/

typedef struct Mission_Data
{
  int   Client;       /*handle to the guy who requested the mission*/
  int   Faction;      /*sometimes a client may ask for something in another faction's name...  though rare*/
  int   Type;         /*THe mission type: Fetch, kill, protect,*/
  int   Target;       /*the ID of the thing that needs to be fetched killed or protected*/
  int   Location;     /*the map id where the target will be found.*/
}Mission;

typedef struct PLAYER_T
{
  int  previoussave;        /*-1 if this character has not yet been saved, the number of the save file, otherwise*/
  char playername[40];      /*the pilot or player's name*/
  char vwpname[40];         /*the name of the mech*/
  int  color[7];            /*the colors that the player's Mech is */
  int  weapon[3];           /*the 2 offensive and 1 defensive weapon that is equipped.  -1 implies no weapon*/
  int  weaponindex;         /*the current weapon is this index*/
  int  core;                /*which core we are using.*/
  int  legs;                /*which leg unit we are using*/
  int  generator;           /*which generator is installed*/
  int  PowerCells;          /*how many ultimate attacks that we CAN have*/
  int  inventory[MAXITEMS]; /*Indexed list of items that the player owns.*/
  int  map;                 /*the map that the player saved in*/
  int  mx,my;               /*the coordinates that the player is in.*/
  Uint32  age;              /*how long the player has had this character, in game world days*/
  int  history[128][2];     /*list of completed events.  Only the 128 most significant will be kept track of.*/
  int  onmission;            /*if we are on a mission*/
  Mission mission;          /*current mission we are on*/
}PlayerStats;

void SpawnPlayer(int x, int y);
void LoadHUD();
void DrawHUD(Entity *self);
void DefaultConfig();
void LoadKeyConfig();
void SaveKeyConfig();
void SavePlayer(PlayerStats *player);
void LoadPlayer(char filename[40],PlayerStats *player);
void LoadPlayerScreen();

/*below is the required end of line*/
#endif