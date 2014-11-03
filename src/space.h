#ifndef __space__
#define __space__

/*
  *   Donald Kehoe
  *   Created : 10/22/05
  *   Last modified: 10/22/05
  *   Header file to define the function prototype and datastructures for backgrounds
  *   Last change : Added comments.
*/

#include "SDL.h"
/*
 *  This file will define the data structures and function prototypes needed for rendering and animating
 *  the scrolling backgrounds for this game.
 *  The functions will create teh entities needed for an interactive level.
 */

#define SPACE_W     128
#define SPACE_H     128
#define MAX_OBJ     128
#define MAX_ENT     128

typedef struct INFO_TAG
{
  char   name[40];      /*the name of the tag, to keep index hazards away*/
  Uint16 sx,sy;         /*coordinates of spawn point or info tag*/
  Uint16 UnitInfo;      /*used by some spawn types.*/
  Uint16 UnitType;      /*faction affiliation*/
  char   info[40];      /*info tags will need more info*/
}Tag;

typedef struct LEVEL_STRUCT
{
  Uint8   tilemap[SPACE_H][SPACE_W];
  Uint16  width,height;
  Tag  infolist[MAX_OBJ];
  int infocount;
  Tag  spawnlist[MAX_ENT];
  int spawncount;
  Uint8 tileset;
  char levelname[40];
  char bgimage[40];
  char bgmusic[40];
}Level;

void GenerateLevel(int x, int y);
void LoadLevel(char *filename);
void SaveLevel(char *filename);
void DrawLevel();
void CloseLevel();

#endif