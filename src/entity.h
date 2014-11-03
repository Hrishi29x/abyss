#ifndef _ENTITY_
#define _ENTITY_

#include "graphics.h"
#include "audio.h"


/*
  This file contains the data structures and function prototypes for handling entities.
  Entities all contain information about the associate sprite that may be unique to itself
  as well as the animation information, if it has any.
  
*/

#define MAXPATH       20
#define MAXTRAIL      16
#define QMAX          10
#define MAXENTITIES   8192
#define MAXSTATES     10
#define SOUNDSPERENT  4
#define PERSISTENCE   8

/*random functions stolen shamelessly from quake2*/
#define random()  ((rand () & 0x7fff) / ((float)0x7fff))
#define crandom() (2.0 * (random() - 0.5))

enum ENTTYPES   {ET_Temp,ET_WorldBullet,ET_WorldEnt,ET_PlayerBullet,ET_Player};
enum SOUNDTYPE  {SF_ALERT,SF_IMPACT,SF_PAIN,SF_DYING}; 
enum ENTCLASS   {EC_NULL,EC_AI,EC_PLAYER,EC_BULLET};
enum DAMAGE_T   {DT_Physical,DT_Magical};
enum STATES     {ST_IDLE,ST_WALK, ST_DIE, ST_DEAD,ST_WSWITCH,ST_ALERT,ST_ATTACK1,ST_ATTACK2,ST_ATTACK3};
enum FACE       {F_South, F_SW, F_West, F_NW,F_North, F_NE, F_East, F_SE,F_NULL};
enum ORDERS     {OR_NONE,OR_WALK,OR_WATTACK,OR_HALT, OR_GATHER, OR_BUILD, OR_REPAIR, OR_DEFEND, OR_RALLEY, OR_FOLLOW};

typedef struct POINT_T
{
  int x,y;
}Point;

typedef struct COORD_T
{
  float x,y;
}Coord;


typedef struct ENTITY_T
{
  int used;                 /*used is for keeping track of what entities are free and should NEVER be touched.*/
  Uint32 LastDrawn;         /*another one that should never be touched by anything but the maintainence functions*/
  int Player;               /*references the player controlling the unit, NUMPLAYERS implies it is owned by the 'world'*/
  int EntClass;             /***/
  int Unit_Type;            /*ET_* for hit detection masking*/
  char EntName[40];         /*the name of the entity*/
  Sprite *sprite;           /*the sprite for the main part of the entity*/
  Sprite *legs;             /*if the entity has a separate sprite for the legs it goes here*/
  Sound *sound[SOUNDSPERENT];/*a list of pointers to the wav files that this entity will produce*/
  struct ENTITY_T *owner;   /*for bullets, drones and other things spawned by other entities*/
  struct ENTITY_T *target;  /*used for MANY units: attack target, healing target, etc*/
  void (*think) (struct ENTITY_T *self);    /*called by the engine every so often to handle input and make decisions*/
  void (*update) (struct ENTITY_T *self);   /*called by the engine every so often to update the position and check for collisions*/
  Uint32 NextThink;         /*used for how often the entity thinks*/
  Uint16 ThinkRate;         /*used for incrementing above*/
  Uint32 NextUpdate;        /*used for how often the entity is updated, updating is merely animations*/
  Uint16 UpdateRate;        /*used for incrementing above*/
  int totaloffset;          /*I forget, I think I use it for... I dunno I really forget: this is why you should comment when you make new variables*/
  int gravityent;           /*if we are affected by gravity*/
  int bounce;               /*if we can bounce off of walls or not*/
  int grounded;             /*set to 1 when the entity is standing on the ground*/
  int Color;                /*the index color for bullets and trails and such*/
  int shown;                /*if 1 then it will be rendered when it is on screen*/
  int frame;                /*current frame to render*/
  int fcount;               /*used for animation, the loop variable*/
  int frate;                /*how often we update our frames*/
  Uint32 framedelay;        /*ammount of delay between frames*/
  int legframe;             /*the frame to draw our legs from in the next draw phase*/
  int legstate;             /*used for keeping track of what state our leg sprite is in... if we have one*/
  int face;                 /*the direction we are moving*/
  int aimdir;               /*the direction we are shooting at*/
  int state;                /*making each entity a finite state machine.*/
  SDL_Rect  Boundingbox;    /*the bounding box for collision detection*/
  SDL_Rect  ClipBox;        /*the bounding box for hit detection*/
  Coord Ls;                 /*offset coordinates to draw the legs at*/
  Coord s;                  /*screen coordinates*/
  Coord a;                  /*acceleration*/
  Coord v;                  /*vector values*/
  Point m;                  /*Map coordinate*/
  Point size;               /*mirrors the sprite width and height*/
  Point origin;             /*offset of the center from s*/
  int movespeed;            /*current movement speed*/
  int maxspeed;             /*maximum movement speed*/
  int ordercount;           /*AI use counting variable*/
  /*for drawing trails*/
  Point trail[MAXTRAIL];    /*the last 8 places that we've been: -1 means unused.*/
  int trailhead;            /*the point in the trail list we are acessing next*/
  int traillen;             /*how long our trail is currently*/
  int maxtraillen;          /*the most we will make our trail*/
  int thick;                /*how thick our trail is*/
  int trailtype;            /*enumerated*/
  
  int sightrange;           /*how big my sigh radius is for detection of the player*/
  int accel;                /*how much our movement is increased when we accellerate*/
  int kick;                 /*for weapons, how much we can push something*/
  int damage;               /*how much damage I can inflict for things that can deal damage*/
  int raddamage;            /*anyone in the radius of explosion will take this much damage*/
  int dtype;                /*what type of Damage I will inflict*/
  /*               Weapon maintanence data                 */
  int switchdelay;          /*a delay counter so we don't switch weapons too fast*/
  int KillCount;            /*how many things I killed this frame*/
  Uint32 Score;             /*how big our score is*/
  int currentweapon;        /*which is our current weapon*/
  int ChargedCells;         /*how many ultimate attacks we have charged up*/
  int PowerLevel;           /*how powerful our ultimate attacks are*/
  int Cooldown;             /*decrements automatically.  Can only fire when it is zero*/
  /*               armor and health                 */
  int Parmor,Marmor;		/*Physical and Magical Resistance*/
  int health,healthmax;     /*everything will need health.  Resources will use it for how much of a resource it has left*/
  int takedamage;           /*a flag as to weather or not we can take damage*/
  int lifespan;             /*some entities will expire after a time...hmm drone helpers that expire....*/
}Entity;

/*startup and clean up*/
void InitEntityList();
void ClearEntities();

/*creation and destruction*/
Entity *NewEntity();
void FreeEntity(Entity *ent);

/*update functions*/
void DrawEntity(Entity *ent);
void DrawEntities();
void UpdateEntities();
void ThinkEntities();


void DrawBBoxEntities();/*draw only the bounding box of the entity*/


void DrawEntityTrail(Entity *ent);

/*generic entity support functions*/
int OnScreen(Entity *self);
void DamageTarget(Entity *attacker,Entity *inflictor,Entity *defender,int damage,int dtype);
void DamageRadius(Entity *attacker,Entity *inflictor,int damage,int radius,int dtype);
Entity *GetClosestEntity(Entity *self,int rx,int ry,int depth);
Entity *GetNextEntByRad(Entity *self,int radius,int rx,int ry,Entity *start,int ETMask);
int MoveUnit(Entity *self);
int GetNextCount(Entity *self);
void  VectorScaleTo(int magnitude, float *xdir,float *ydir);
void ScaleVectors(float *vx, float *vy);
int VectorLength(float vx,float vy);    /*only useful for integer comparisons, bu faster than float answer*/
int Collide(SDL_Rect box1,SDL_Rect box2);/*returns 1 if the two rectangles are clipping*/
int WhatFace(Entity *self);/*return the enumerated value of the direction the entity is facing*/
void GetFace(Entity *self);
void Get16Face(Entity *self);
void ApplyFriction(Entity *self,float friction);
Entity *GetNextRegionEnt(int x, int y,int count);/*returns the next entity from the region mask,0 being the first*/
int UpdateEntityPosition(Entity *self,int bounce);
int DistanceBetween(Entity *self, Entity *target);/*returns the diference between two entities.*/
int AimAtTarget(Entity *self,Entity *target);
Coord AddVectors(Coord v1,Coord v2);
Coord FastAddVectors(Coord v1,Coord v2);

/*collision detection functions*/
int TraceHit(float sx, float sy, float vx, float vy, float *fx, float *fy,int *rx,int *ry);
void InitRegionMask(int sizex,int sizey); /*based on size of map*/
void ClearRegionMask();   /*de-allocate all allocated memory*/
Entity *GetEntByBox(SDL_Rect bbox,int rx,int ry,Entity *ignore,int ETMask);
Entity *GetNextEntByBox(SDL_Rect bbox,int rx,int ry,Entity *start,int ETMask);
int AddEntToRegion(Entity *ent,int rx,int ry);
void RemoveEntFromRegion(Entity *ent,int rx,int ry);
void DrawBuckets();
void AdjustOrbit(Entity *self);

#endif
