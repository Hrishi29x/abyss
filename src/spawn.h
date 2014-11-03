#ifndef __SPAWN__
#define __SPAWN__

#include "entity.h"


typedef struct
{
  SDL_Rect bbox;
  char EntName[40];         /*the name of the entity*/
  char sprite[40];           /*the sprite for the main part of the entity*/
  int sw,sh;                 /*width and height of sprite*/
  char sound[SOUNDSPERENT][40];/*a list of pointers to the wav files that this entity will produce*/
  void (*spawn) (Entity *owner, int sx, int sy,int subtype, int faction);
  Coord Ls;                 /*offset coordinates to draw the legs at*/
  Sprite *mapsprite;        /*Always initialized to NULL*/
}Spawn;

int GetSpawnIndexByName(char EntName[40]);     /*utility function*/
void LoadSpawnSprites();
void DrawSpawnPoints();                        /*map editor function*/
void DrawSpawn(int index,int sx, int sy);      /*draws the desired spawn candidate at the location*/
void PrecacheSpawns();                         /*only after a map's info has been loaded*/
void SpawnAll(int initial);                               /*after map is loaded, start all entities*/

#endif