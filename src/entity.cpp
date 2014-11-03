#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "entity.h"
#include "space.h"
/*
  Entity function definitions
*/
#define   Depth       16
#define   RPixel_W    64
#define   RPixel_H    64

/*
  With this design I am intending to create a 2 dimensional bucket sort for collision detection.
  Each region will have a list of buckets.  We will start off with a simple bucket depth of 16 and increase the bucket depth as needed.  When the map is done, we will de-allocate the memory allocated by all buckets.
 */

typedef struct T_BUCKET
{
  Entity *bucket[Depth];
  struct T_BUCKET *next;
}Bucket;

typedef struct T_REGION
{
  Bucket *base;   /*pointer to our bucket list, if our count goes higher than 16, the next will be allocated*/
  int count;
}Region;

extern SDL_Surface *screen;
extern SDL_Surface *clipmask;
extern SDL_Surface *background;
extern SDL_Rect Camera;
extern Uint32 NOW;
extern Level level;

Entity EntityList[MAXENTITIES];  /*the last column is the world*/

Region RegionMask[SPACE_H + 1][SPACE_W + 1];/*Pointers to buckets, which point to entities, by location, with chaining*/
int Region_W = 0;
int Region_H = 0;

/*unit and building information*/
/*I will make this like I make sprites, each "peon" will point to the same peon information, per player*/


int NumEnts = 0;
int MOUSEMOVE = 1;


/************************************************************************

                      Entity Creation and Management Functions

 ************************************************************************/
void InitEntityList()
{
  int i,j;
  NumEnts = 0;
  for(i = 0;i < MAXENTITIES; i++)
  {
    EntityList[i].sprite = NULL;
    EntityList[i].owner = NULL;
    EntityList[i].think = NULL;
    EntityList[i].target = NULL;
    EntityList[i].update = NULL;
    EntityList[i].legs = NULL;
    for(j = 0;j < SOUNDSPERENT;j++)
    {
      EntityList[i].sound[j] = NULL;
    }
    EntityList[i].shown = 0;
    EntityList[i].used = 0;
  }
  /*lets make sure our deadspace entity is blank and safe.*/
  NewEntity();/*just to start things off*/
}


/*
  draw all of the active entities in the view of the camera.  
  I my come up with a more eficient algorithm
 */
 
void DrawBBoxEntities()
{
  int i;
  int count = 0;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if(EntityList[i].used == 1)
    {
      if(((EntityList[i].s.x + EntityList[i].size.x) >= Camera.x)&&(EntityList[i].s.x <= (Camera.x + Camera.w))&&((EntityList[i].s.y + EntityList[i].size.y) >= Camera.y)&&(EntityList[i].s.y <= (Camera.y + Camera.h)))
      {
        DrawRect(EntityList[i].Boundingbox.x + (int)EntityList[i].s.x - Camera.x,EntityList[i].Boundingbox.y + (int)EntityList[i].s.y - Camera.y, EntityList[i].Boundingbox.w, EntityList[i].Boundingbox.h, IndexColor(EntityList[i].Color), screen);
      }
      count++;
    }
  }
}
    
 
void DrawEntities()
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if(EntityList[i].used == 1)
    {
      if(((EntityList[i].s.x + EntityList[i].size.x) >= Camera.x - 200)&&(EntityList[i].s.x <= (Camera.x + Camera.w + 200))&&((EntityList[i].s.y + EntityList[i].size.y) >= Camera.y - 200)&&(EntityList[i].s.y <= (Camera.y + Camera.h) + 200))
      {
        DrawEntity(&EntityList[i]);
      }
    }
  }
}

void ThinkEntities()
{
  int i;
  int checked = 0;
  for(i = 0;i < MAXENTITIES;i++)
  {
      if(EntityList[i].used)
      {
        checked++;
        if(EntityList[i].NextThink < NOW)
        {
          if(EntityList[i].think != NULL)
          {
            EntityList[i].think(&EntityList[i]);
            EntityList[i].NextThink = NOW + EntityList[i].ThinkRate;
          }
        }
      }
  }
}

void UpdateEntities()
{
  int i;
  int checked = 0;
  for(i = 0;i < MAXENTITIES;i++)
  {
      if(EntityList[i].used)
      {
        checked++;
        if(EntityList[i].NextUpdate < NOW)
        {
          if(EntityList[i].update != NULL)
          {
            EntityList[i].update(&EntityList[i]);
            EntityList[i].NextUpdate = NOW + EntityList[i].UpdateRate;
          }
        }
      }
  }
  
}

void DrawEntity(Entity *ent)
{
  if(ent->trailhead != -1)
  {
    DrawEntityTrail(ent);
  }
  if(ent->legstate == -1)
  {
    if(ent->sprite != NULL)
      DrawSprite(ent->sprite,screen,ent->s.x - Camera.x,ent->s.y - Camera.y + ent->totaloffset,ent->frame);
    return;    
  }
  else
  {
    if(ent->legs != NULL)
      DrawSprite(ent->legs,screen,ent->s.x - Camera.x,ent->s.y - Camera.y + ent->totaloffset,ent->legframe);
    if(ent->sprite != NULL)
      DrawSprite(ent->sprite,screen,ent->s.x + ent->Ls.x - Camera.x,ent->s.y + ent->Ls.y - Camera.y + ent->totaloffset,ent->frame);
  }
}

/*this is just the prep for generic Buildings, each unit will have more specific data set later*/

/*
  returns NULL if all filled up, or a pointer to a newly designated Entity.
  Its up to the other function to define the data.
*/
Entity *NewEntity()
{
  int i;
  if(NumEnts + 1 >= MAXENTITIES)
  {
    return NULL;
  }
  NumEnts++;
  for(i = 0;i < MAXENTITIES;i++)
  {
    if(!EntityList[i].used)break;
  }
  EntityList[i].used = 1;
  EntityList[i].EntClass = EC_AI;
  EntityList[i].totaloffset = 0;
  return &EntityList[i];
}

/*done with an entity, now give back its water..I mean resources*/
void FreeEntity(Entity *ent)
{
  int j;
  /*  fprintf(stdout,"Freeing %s\n",ent->EntName);*/
  ent->used = 0;
  NumEnts--;
  RemoveEntFromRegion(ent,ent->m.x,ent->m.y);
  if(ent->sprite != NULL)FreeSprite(ent->sprite);
  if(ent->legs != NULL)FreeSprite(ent->legs);
  for(j = 0;j < SOUNDSPERENT;j++)
  {
    if(ent->sound[j] != NULL)FreeSound(ent->sound[j]);
    ent->sound[j] = NULL;
  }
  ent->legs = NULL;
  ent->sprite = NULL;
  ent->owner = NULL;
  ent->think = NULL;
  ent->target = NULL;
  ent->update = NULL;
  ent->trailhead = -1;
      /*delete the infor first, then close the pointer*/
}

/*kill them all*/
void ClearEntities()
{
  int i;
  for(i = 0;i < MAXENTITIES;i++)
    {
      FreeEntity(&EntityList[i]);
    }
}

/************************************************************************

                      Entity Maintanence functions

 ************************************************************************/
void DamageTarget(Entity *attacker,Entity *inflictor,Entity *defender,int damage,int dtype)
{     /*inflictor is the entity that is physically doing the damage, attacker is the entity that initiated it.*/
      /*Exampe: the player is the attacker, but the shotgun shell is the inflictor*/
  if(defender == NULL)return;   /*you never know*/
  if(!defender->takedamage)return;/*lets not hurt anything that can't be hurt*/
  switch(dtype)
  {
    case DT_Physical:
      damage -= defender->Parmor;
      break;
    case DT_Magical:
      damage -= defender->Marmor;
      break;
  }
  if(damage <= 0)damage = 1;/*you will at LEAST take 1 damage*/
  defender->health -= damage;
  if(defender->EntClass == EC_AI)defender->state = ST_ALERT;
  if(defender->health <= 0)
  {
    defender->state = ST_DIE;
    defender->takedamage = 0;
    if(defender->EntClass == EC_AI)
    {
      attacker->KillCount++;
    }
  }
  
  /*adding Id style obituary code here*/
}


void DamageRadius(Entity *attacker,Entity *inflictor,int damage,int radius,int dtype)
{ 
  int width;
  int startx,starty;
  int i,j;
  Entity *target = NULL;
  width = (radius>>6) + 2;
  startx = inflictor->m.x - (width>>1);
  starty = inflictor->m.y - (width>>1);
  for(j = starty;j < width;j++)
  {
    for(i = startx;i < width;i++)
    {
      target = NULL;
      do
      {
        target = GetNextEntByRad(inflictor,radius,i,j,target,inflictor->Unit_Type);
        if(target != NULL)
        {
          DamageTarget(attacker,inflictor,target,damage,dtype);
        }
      }while(target != NULL);
    }
  }
}


int AimAtTarget(Entity *self,Entity *target)
{
  int vx,vy;
  vx = ((int)target->s.x - (int)self->s.x) + (target->origin.x - self->origin.x);
  vy = ((int)target->s.y - (int)self->s.y) + (target->origin.y - self->origin.y);
  if((vx == 0)&&(vy == 0))
  {
    return F_NULL;
  }
  if(vx < -8)
  {
    if(vy < -8)return F_NW;
    else if(vy > 8)return F_SW;
    else return F_West;
  }
  else if(vx > 8)
  {
    if(vy < -8)return F_NE;
    else if(vy > 8)return F_SE;
    else return F_East;
  }
  else
  {
    if(vy < 8)return F_North;
    else if(vy > 8)return F_South;
  }
  return F_NULL;
}


int WhatFace(Entity *self)
{
  if((self->v.x == 0)&&(self->v.y == 0))
  {
    return F_NULL;
  }
  if(self->v.x < -2)
  {
    if(self->v.y < -2)return F_NW;
    else if(self->v.y > 2)return F_SW;
    else return F_West;
  }
  else if(self->v.x > 2)
  {
    if(self->v.y < -2)return F_NE;
    else if(self->v.y > 2)return F_SE;
    else return F_East;
  }
  else
  {
    if(self->v.y < 0)return F_North;
    else if(self->v.y > 0)return F_South;
  }
  return -1;
}


void GetFace(Entity *self)
{
  if((self->v.x == 0)&&(self->v.y == 0))
  {
    self->face = F_NULL;
    return;
  }
  if(self->v.x < -2)
  {
    if(self->v.y < -2)self->face = F_NW;
    else if(self->v.y > 2)self->face = F_SW;
    else self->face = F_West;
  }
  else if(self->v.x > 2)
  {
    if(self->v.y < -2)self->face = F_NE;
    else if(self->v.y > 2)self->face = F_SE;
    else self->face = F_East;
  }
  else
  {
    if(self->v.y < 0)self->face = F_North;
    else if(self->v.y > 0)self->face = F_South;
  }
}

void Get16Face(Entity *self)
{
  int small,large;
  small = 0.25 * self->movespeed;
  large = 0.5 * self->movespeed;
  if((self->v.x == 0)&&(self->v.y == 0))
  {
    self->face = 1;
    return;
  }
  if(self->v.x < (-1 * large))
  {
    if(self->v.y < (-1 * large))self->face = 6;
    else if(self->v.y < (-1 * small))self->face = 5;
    else if(self->v.y > large)self->face = 2;
    else if(self->v.y > small)self->face = 3;
    else self->face = 4;
  }
  else if(self->v.x < (-1 * small))
  {
    if(self->v.y < (-1 * large))self->face = 7;
    else if(self->v.y < (-1 * small))self->face = 6;
    else if(self->v.y > large)self->face = 1;
    else if(self->v.y > small)self->face = 2;
    else self->face = 4;
  }
  else if(self->v.x > large)
  {
    if(self->v.y < (-1 * large))self->face = 10;
    else if(self->v.y < (-1 * small))self->face = 11;
    else if(self->v.y > large)self->face = 14;
    else if(self->v.y > small)self->face = 13;
    else self->face = 12;
  }
  else if(self->v.x > small)
  {
    if(self->v.y < (-1 * large))self->face = 9;
    else if(self->v.y < (-1 * small))self->face = 13;
    else if(self->v.y > large)self->face = 15;
    else if(self->v.y > small)self->face = 14;
    else self->face = 12;
  }
  else
  {
    if(self->v.y <= 0)self->face = 8;
    else if(self->v.y > 0)self->face = 0;
  }
  
}

void DrawEntityTrail(Entity *self)
{
  int i = self->trailhead;
  int count = 0;
  int sx,sy;
  int dx,dy;
  int length;
  Uint32 color;
  Uint8 r,g,b;
  color = IndexColor(self->Color);
  SDL_GetRGB(color, screen->format, &r, &g, &b);
  if(self->traillen > self->maxtraillen)length = self->maxtraillen;
  else length = self->traillen;
  sx = self->trail[i].x;
  sy = self->trail[i].y;
  while(count < length)
  {
    i--;
    if(i < 0)i = self->traillen;
    dx = self->trail[i].x;
    dy = self->trail[i].y;
    DrawThickLine(sx - Camera.x + self->origin.x,sy - Camera.y + self->origin.y,dx - Camera.x + self->origin.x, dy - Camera.y  + self->origin.y,self->thick,SDL_MapRGB(screen->format,r,g,b),screen);
    if(r > length)r-=length;
    if(g > length)g-=length;
    if(b > length)b-=length;
    sx = dx;
    sy = dy;
    count++;
  }
}

Coord AddVectors(Coord v1,Coord v2)
{
  Coord result;
  result.x = (v1.x + v2.x)*0.5;
  result.y = (v1.y + v2.y)*0.5;
  return result;
}

Coord FastAddVectors(Coord v1,Coord v2)
{
  Coord result;
  result.x = (int)(v1.x + v2.x)>>1;
  result.y = (int)(v1.y + v2.y)>>1;
  return result;
}




/*for entities that orbit other entities*/
void AdjustOrbit(Entity *self)
{
  Coord direction;  /*recalulated vector*/
  Coord Radius;     /*vector between orbiter and orbitee*/
  int distance;
  Radius.x =((self->target->s.x + self->target->origin.x) - (self->s.x + self->origin.x));
  Radius.y =((self->target->s.y + self->target->origin.y) - (self->s.y + self->origin.y));
  direction.x = Radius.y;
  direction.y = Radius.x * -1;
  ScaleVectors(&direction.x, &direction.y);
  ScaleVectors(&Radius.x,&Radius.y);
  self->v.x = (direction.x * self->movespeed);
  self->v.y = (direction.y * self->movespeed);
  distance = DistanceBetween(self, self->target);
  if(distance < 20)
  {
    self->v.x -= Radius.x;
    self->v.y -= Radius.y;
  }
  else if(distance > 20)
  {
    self->v.x += Radius.x;
    self->v.y += Radius.y;
  }
  self->v.x += self->target->v.x;
  self->v.y += self->target->v.y;
}

/*return 0 if normal, 1 if it clipped a surface x-wise, 2 if clipped a surface y-wise, 3 if both*/
int failedUpdateEntityPosition(Entity *self,int bounce)
{
  int tsx,tsy;   /*temporary positions*/
  Uint32 start = SDL_GetTicks();
  int xdir = 0;
  int ydir = 0;
  float temp;
  double vx,vy = 0;
  float fx,fy;
  float gx,gy;
  float osx,osy;
  int rxtype = 0;
  int rytype = 0;
  int buckx,bucky;
  int i;
  int xhit = 0;
  int yhit = 0;
  int xyhit = 0;
  /*if((self->s.x >= background->w)||(self->s.y >= background->h)||(self->s.y + self->size.y <= 0)||(self->s.x + self->size.x <= 0))
  {   //we've gone off the map, get rid of us.
  FreeEntity(self);
  return 0;
}*/
  osx = self->s.x;
  osy = self->s.y;
  vx = self->v.x;
  vy = self->v.y;
  if(self->v.x < 0)
  {
    tsx  = ((int)self->s.x + self->Boundingbox.x);
    xdir = -1;
  }
  else if(self->v.x > 0)
  {
    tsx  = ((int)self->s.x + self->Boundingbox.x + self->Boundingbox.w);
    xdir = 1;
  }
  else
  {
    xdir = 0;
    tsx  = 0;
  }
  if(self->v.y < 0)
  {
    tsy  = ((int)self->s.y + self->Boundingbox.y);
    ydir = -1;
  }
  else if(self->v.y > 0)
  {
    tsy  = ((int)self->s.y + self->Boundingbox.y + self->Boundingbox.h);
    ydir = 1;
  }
  else
  {
    tsy  = 0;
    ydir = 0;    
  }
  
  
  start = SDL_GetTicks();
  if(clipmask != NULL)if(SDL_MUSTLOCK(clipmask))
  {
    if ( SDL_LockSurface(clipmask) < 0 )
    {
      fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
      exit(1);
    }
  }
  
  if(xdir)
  {
    for(i = 0;i < self->Boundingbox.h;i++)
    {
      if(TraceHit(self->s.x + self->Boundingbox.x +  i , tsy, vx, vy, &gx, &gy,NULL,&rytype))
      {
        yhit = 1;
        break;
      }
    }
  }
  
  if(ydir)
  {
    for(i = 0;i < self->Boundingbox.w;i++)
    {
      if(TraceHit(tsx,self->s.y + self->Boundingbox.y +  i ,  vx, vy, &fx, &fy,&rxtype,NULL))
      {
        xhit = 1;
        break;
      }
    }
  }
  
  if(xdir && ydir)
  {
      if(TraceHit(tsx , tsy, vx, vy, &gx, &gy,NULL,&rytype))
      {
        xyhit = 1;
      }
  }
  
  if((clipmask != NULL)&&(SDL_MUSTLOCK(clipmask)))SDL_UnlockSurface(clipmask);
 
  if((xhit&yhit)||(xyhit))/*hit both, find the clostest hit*/
  {
    if((abs(fx - (int)self->s.x) + abs(fy - (int)self->s.y)) > (abs(gx - (int)self->s.x) + abs(gy - (int)self->s.y)))
    {
      self->s.x += gx;
      self->s.y += gy;
    } 
    else
    {
      self->s.x += fx;
      self->s.y += fy;
    }
  }
  else if(yhit)/*hit just y*/
  {
    self->s.x += gx;
    self->s.y += gy;
  }
  else if(xhit)/*hit just x*/
  {
    self->s.x += fx;
    self->s.y += fy;
  }
  if(!(xhit|yhit))/*never hit a thing*/
  {
    self->s.x += vx;
    self->s.y += vy;
  }
  
  
  if(!bounce)
  {
    if((ydir == 1)&&(rytype))
    {
      if(vy < 0)
      {
        self->grounded = 1;
      }
    }
  }
  else
  {
    if((xhit!=yhit)&&(xyhit) && (xhit|yhit))
    {
      temp = self->v.x;
      self->v.x = self->v.y;
      self->v.y = temp;
      temp = self->a.x;
      self->a.x = self->a.y;
      self->a.y = temp;
      if(xhit)
      {
        self->v.x *= -1;
        self->a.x *= -1;
      }
      if(yhit)
      {
        self->v.y *= -1;
        self->a.y *= -1;
      }
    }
    else
    {
      if(xhit)
      {
        self->v.x *= -1;
        self->a.x *= -1;
      }
      if(yhit)
      {
        self->v.y *= -1;
        self->a.y *= -1;
      }
    }
  }
  /*update trails*/
  if(self->trailhead != -1)
  {
    self->trailhead++;
    if(self->trailhead >= self->maxtraillen)self->trailhead = 0;
    self->trail[self->trailhead].x = self->s.x;
    self->trail[self->trailhead].y = self->s.y;
    if(self->traillen < (self->maxtraillen - 1))self->traillen++;
  }
  self->v.x += self->a.x;
  self->v.y += self->a.y;
  /*region mask placement*/
  if(xdir)
  {
    buckx = ((int)self->s.x + self->origin.x) >> 6;
  }
  else buckx = self->m.x;
  if(ydir)
  {
    bucky = ((int)self->s.y + self->origin.y) >> 6;
  }
  else bucky = self->m.y;
  if((buckx != self->m.x)||(bucky != self->m.y))
  {
    if(AddEntToRegion(self,buckx,bucky))
    {
      RemoveEntFromRegion(self,self->m.x,self->m.y);
      self->m.x = buckx;
      self->m.y = bucky;
     
    }
    else /*we didn't try for a legitmate slot*/
    {
      self->s.x = osx;
      self->s.y = osy;
    }
  }
  return (rxtype | rytype);
}


/*return 0 if normal, 1 if it clipped a surface x-wise, 2 if clipped a surface y-wise, 3 if both*/
int UpdateEntityPosition(Entity *self,int bounce)
{
  int tsx,tsy;   /*temporary positions*/
  Uint32 start = SDL_GetTicks();
  Uint32 clear = SDL_MapRGB(background->format,0,0,0);
  int xdir = 0;
  int ydir = 0;
  int updated_s = 0;
  float temp = 0;
  double vx,vy = 0;
  int rxtype = 0;
  int rytype = 0;
  int buckx,bucky;
  float fx,fy,gx,gy,hx,hy = 0;
  vx = self->v.x;
  vy = self->v.y;
  if(self->v.x < 0)
  {
    tsx  = ((int)self->s.x + self->Boundingbox.x);
    xdir = 1;
  }
  else if(self->v.x > 0)
  {
    tsx  = ((int)self->s.x + self->Boundingbox.x + self->Boundingbox.w);
    xdir = -1;
  }
  else
  {
    xdir = 0;
    tsx  = ((int)self->s.x + self->Boundingbox.x + (self->Boundingbox.w >> 1));
  }
  if(self->v.y < 0)
  {
    tsy  = ((int)self->s.y + self->Boundingbox.y);
    ydir = 1;
  }
  else if(self->v.y > 0)
  {
    tsy  = ((int)self->s.y + self->Boundingbox.y + self->Boundingbox.h);
    ydir = -1;
  }
  else
  {
    tsy  = ((int)self->s.y + self->Boundingbox.y + (self->Boundingbox.h >> 1));
    ydir = 0;    
  }
  start = SDL_GetTicks();
/*  if(xdir)while((getpixel(clipmask, (int)(tsx + vx) ,tsy) != clear)&&(vx * xdir < 0))
  { there be something here
    vx += xdir;
    rxtype = 1;
}*/
  if(xdir&ydir)
  {
    if((TraceHit(tsx , tsy, vx, vy, &hx, &hy,&rxtype,&rytype))&&(updated_s == 0))
    {
/*      fprintf(stdout,"hx %f, hy %f\n",hx,hy);
      fprintf(stdout,"vx %f, vy %f\n",vx,vy);*/
      self->s.x += hx;
      self->s.y += hy;
      updated_s = 1;
    }
}
  if(xdir)
  {
    if((TraceHit(tsx , tsy, vx, 0, &fx, &fy,&rxtype,NULL))&&(updated_s == 0))
    {
      hx = vx;
      hy = vy;
      VectorScaleTo(sqrt((fx * fx) + (fy * fy)),&hx,&hy);
      self->s.x += hx;
      self->s.y += hy;
      updated_s = 1;
    }
  }
  if(ydir)
  {
    if((TraceHit(tsx , tsy, 0, vy, &gx, &gy,NULL,&rytype))&&(updated_s == 0))
    {
      hx = vx;
      hy = vy;
      VectorScaleTo(sqrt((gx * gx) + (gy * gy)),&hx,&hy);
      self->s.x += hx;
      self->s.y += hy;
      updated_s = 1;
    }
  }
  if((clipmask != NULL)&&(SDL_MUSTLOCK(clipmask)))SDL_UnlockSurface(clipmask);
  if(updated_s == 0)
  {
    self->s.x += vx;
    self->s.y += vy;
  }
  if(!bounce)
  {
    self->v.x = vx;
    self->v.y = vy;
    if(ydir == -1)
    {
      if(vy <= 0.2)
      {
        self->grounded = 1;
      }
    }
  }
  else
  {
    if(rxtype)
    {
      self->v.x *= -1;
      self->a.x *= -1;
    }
    if(rytype)
    {
      self->v.y *= -1;
      self->a.y *= -1;
    }
    if(rxtype && rytype)
    {
      temp = self->v.x;
      self->v.x = self->v.y;
      self->v.y = temp;
      temp = self->a.x;
      self->a.x = self->a.y;
      self->a.y = temp;
      if(xdir != ydir)
      {
        self->v.x *= -1;
        self->a.x *= -1;
        self->v.y *= -1;
        self->a.y *= -1;
      }
    }
  }
  /*update trails*/
  if(self->trailhead != -1)
  {
    self->trailhead++;
    if(self->trailhead >= self->maxtraillen)self->trailhead = 0;
    self->trail[self->trailhead].x = self->s.x;
    self->trail[self->trailhead].y = self->s.y;
    if(self->traillen < (self->maxtraillen - 1))self->traillen++;
  }
  self->v.x += self->a.x;
  self->v.y += self->a.y;
  /*region mask placement*/
  if(xdir)
  {
    buckx = ((int)self->s.x + self->origin.x) >> 6;
  }
  else buckx = self->m.x;
  if(ydir)
  {
    bucky = ((int)self->s.y + self->origin.y) >> 6;
  }
  else bucky = self->m.y;
  if((buckx != self->m.x)||(bucky != self->m.y))
  {
    RemoveEntFromRegion(self,self->m.x,self->m.y);
    AddEntToRegion(self,buckx,bucky);
    self->m.x = buckx;
    self->m.y = bucky;
  }
  return (rxtype | rytype);
}

/************************************************************************

                      Simple generic helper functions follow

 ************************************************************************/
 
int TraceHit(float sx, float sy, float vx, float vy, float *fx, float *fy,int *rx,int *ry)
{
  Uint32 clear = SDL_MapRGBA(background->format,0,0,0,0);
  Uint32 pixelColor;
  int deltax,deltay;
  float x,y;
  int curpixel;
  float ox,oy;    /*old x, old y*/
  int den,num,numadd,numpixels;
  int xinc1,xinc2,yinc1,yinc2;
  deltax = fabs(vx);        // The difference between the x's
  deltay = fabs(vy);        // The difference between the y's
  x = sx;                       // Start x off at the first pixel
  y = sy;                       // Start y off at the first pixel

  if (vx >= 0)                 // The x-values are increasing
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          // The x-values are decreasing
  {
    xinc1 = -1;
    xinc2 = -1;
  }

  if (vy >= 0)                 // The y-values are increasing
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          // The y-values are decreasing
  {
    yinc1 = -1;
    yinc2 = -1;
  }

  if (deltax >= deltay)         // There is at least one x-value for every y-value
  {
    xinc1 = 0;                  // Don't change the x when numerator >= denominator
    yinc2 = 0;                  // Don't change the y for every iteration
    den = deltax;
    num = deltax >> 1;
    numadd = deltay;
    numpixels = deltax;         // There are more x-values than y-values
  }
  else                          // There is at least one y-value for every x-value
  {
    xinc2 = 0;                  // Don't change the x for every iteration
    yinc1 = 0;                  // Don't change the y when numerator >= denominator
    den = deltay;
    num = deltay >> 1;
    numadd = deltax;
    numpixels = deltay;         // There are more y-values than x-values
  }
  ox = x;
  oy = y;
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    pixelColor = getpixel(clipmask, x ,y);
    fprintf(stdout,"pixel color: %i, clear color: %i\n",pixelColor,clear);
    if(pixelColor != clear)/*check to see if the pixel is clear or not*/
    {
      *fx = x - sx;
      *fy = y - sy;
      if (num >= den)             // Check if numerator >= denominator
      {

      if(getpixel(clipmask, (int)(ox + xinc1),(int)oy) != clear)/*reflect x*/
      {
        if(rx != NULL) *rx = 1;
      }
      
      if(getpixel(clipmask, (int)ox ,(int)(oy + yinc1)) != clear)/*reflect y*/
      {
        if(ry != NULL)*ry = 1;
      }
      
      }
      else
      {
        if(getpixel(clipmask, (int)(ox + xinc2),(int)oy) != clear)/*reflect x*/
        {
          if(rx != NULL) *rx = 1;
        }
//      else if(rx != NULL)*rx = 0;
      
        if(getpixel(clipmask, (int)ox ,(int)(oy + yinc2)) != clear)/*reflect y*/
        {
          if(ry != NULL)*ry = 1;
        }

      }
      fprintf(stdout,"We hit something\n");
      return 1;/*we hit shit*/
    }
    num += numadd;              // Increase the numerator by the top of the fraction
    if (num >= den)             // Check if numerator >= denominator
    {
      num -= den;               // Calculate the new numerator value
      x += xinc1;               // Change the x as appropriate
      y += yinc1;               // Change the y as appropriate
    }
    x += xinc2;                 // Change the x as appropriate
    y += yinc2;                 // Change the y as appropriate
    ox = x;
    oy = y;
  }
  /*clean trace, nothing hit*/
  return 0;
}


int OnScreen(Entity *self)
{
  if(((self->s.x + self->size.x) >= Camera.x)&&(self->s.x <= (Camera.x + Camera.w))&&((self->s.y + self->size.y) >= Camera.y)&&(self->s.y <= (Camera.y + Camera.h)))
    return 1;
  return 0;
}


/*returns the relative diference between two entities' positions.*/
int DistanceBetween(Entity *self, Entity *target)
{
  int difx,dify;
  difx = (int)abs((int)self->s.x - (int)target->s.x);
  dify = (int)abs((int)self->s.y - (int)target->s.y);
  return (difx + dify)>>1;
}

int Collide(SDL_Rect box1,SDL_Rect box2)
{
  /*check to see if box 1 and box 2 clip, then check to see if box1 is in box or vice versa*/
  if((box1.x + box1.w >= box2.x) && (box1.x <= box2.x+box2.w) && (box1.y + box1.h >= box2.y) && (box1.y <= box2.y+box2.h))
    return 1;
  return 0;
}

int VectorLength(float vx,float vy)
{
  return (int)((vx * vx) + (vy *vy)) >> 1;
}

void ApplyFriction(Entity *self,float friction)
{
  if(self->v.x > 0)self->v.x -= friction;
  else if(self->v.x < 0) self->v.x += friction;
  if(self->v.y > 0)self->v.y -= friction;
  else if(self->v.y < 0)self->v.y += friction;  
}

void  VectorScaleTo(int magnitude, float *xdir,float *ydir)
{
  ScaleVectors(xdir,ydir);
  *xdir *= magnitude;
  *ydir *= magnitude;  
}

void ScaleVectors(float *vx, float *vy)
{
  double hyp;
  hyp = sqrt((*vx * *vx) + (*vy * *vy));
  if(hyp == 0)return;
  hyp = 1 / hyp;
  *vx = (*vx * hyp);
  *vy = (*vy * hyp);
}


/************************************************************************

                      Region Buckets for hit detection follow

 ************************************************************************/
 
  /*based on size of map*/
void InitRegionMask(int sizex,int sizey)
{
  int i,j,k;
  if((sizex > SPACE_W)||(sizey > SPACE_H))
  {
    fprintf(stderr,"Attemped to create more map space than we are allotted.\n");
    exit(0);
  }
  if(clipmask != NULL)if(SDL_MUSTLOCK(clipmask))
  {
    if ( SDL_LockSurface(clipmask) < 0 )
    {
      fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
      exit(1);
    }
  }
  
  Region_W = sizex;
  Region_H = sizey;
  for(j = 0;j < sizey;j++)
  {
    for(i = 0;i < sizex;i++)
    {
      RegionMask[j][i].count = 0;
      RegionMask[j][i].base = (Bucket*)malloc(sizeof(Bucket));
      if(RegionMask[j][i].base == NULL)
      {
        fprintf(stderr,"failed to allocate data for hit detection.\n");
        exit(0);
      }
      RegionMask[j][i].base->next = NULL; /*always keep your damned pointers CLEAN*/
      for(k = 0;k < Depth;k++)RegionMask[j][i].base->bucket[k] = NULL;
    }
  }
}
  
/*de-allocate all allocated memory*/
/*recursively delete our allocated memory*/
void ClearBucket(Bucket *target)
{
  if(target->next != NULL)ClearBucket(target->next);
  free(target);
}

void ClearRegionMask()
{
  int i,j;
  for(j = 0;j < Region_H;j++)
  {
    for(i = 0;i < Region_W;i++)
    {
      if(RegionMask[j][i].base->next != NULL)ClearBucket(RegionMask[j][i].base->next);
      free(RegionMask[j][i].base);
      RegionMask[j][i].base = NULL;
    }
  }

}

/*Called by Move entity when we cross over into a new region
  Returns 1 if successful 0 otherwise
*/

int AddEntToRegion(Entity *ent,int rx,int ry)
{
  int i,k;
  int done = 0;
  Bucket *target;
  if((rx < 0)||(ry < 0)||(rx >= Region_W)||(ry >= Region_H))return 0;
  target = RegionMask[ry][rx].base;
  while(!done)
  {
    for(i = 0; i < Depth;i++)
    {
      if(target->bucket[i] == NULL)/*we hit a hole, add us to it*/
      {
        target->bucket[i] = ent;
        RegionMask[ry][rx].count++;
        return 1;
      }
    }
    if(!done)
    {
      if(target->next != NULL)target = target->next;/*go to the next level*/
      else /*the next level needs to be created, and hence we know where we are putting us: at the beginning*/
      {
        target->next = (Bucket*)malloc(sizeof(Bucket));
        if(target->next == NULL)
        {
          fprintf(stderr,"failed to allocate data for hit detection.\n");
          exit(0);
        }
        target->next->next = NULL; /*always keep your damned pointers CLEAN*/
        for(k = 1;k < Depth;k++)target->next->bucket[k] = NULL;
        target->next->bucket[0] = ent;
        RegionMask[ry][rx].count++;
        return 1;
      }
    }
  }
  return 0;
}

int oldAddEntToRegion(Entity *ent,int rx,int ry)
{
  int i,k;
  int done = 0;
  Bucket *target;
  target = RegionMask[ry][rx].base;
  if((rx < 0)||(rx >= SPACE_W)||(ry < 0)||(ry >= SPACE_H))
  {
    fprintf(stderr,"trying to add myself to a non existant region!\n");
    exit(0);
    return 0;/*we can't add ourselves to a non-existant region*/
  }
  while(!done)
  {
    for(i = 0; i < Depth;i++)
    {
      if(target->bucket[i] == NULL)/*we hit a hole, add us to it*/
      {
        target->bucket[i] = ent;
        RegionMask[ry][rx].count++;
        return 1;
      }
    }
    if(!done)
      {
        if(target->next != NULL)target = target->next;/*go to the next level*/
        else /*the next level needs to be created, and hence we know where we are putting us: at the beginning*/
        {
          target->next = (Bucket*)malloc(sizeof(Bucket));
          if(target->next == NULL)
          {
            fprintf(stderr,"failed to allocate data for hit detection.\n");
            exit(0);
          }
          target->next->next = NULL; /*always keep your damned pointers CLEAN*/
          for(k = 1;k < Depth;k++)target->next->bucket[k] = NULL;
          target->next->bucket[0] = ent;
          RegionMask[ry][rx].count++;
          return 1;
        }
      }
  }
  return 0;
}

void RemoveEntFromRegion(Entity *ent,int rx,int ry)
{
  int i;
  int done = 0;
  Bucket *target;
  if((rx < 0)||(ry < 0)||(rx >= Region_W)||(ry >= Region_H))return;
  target = RegionMask[ry][rx].base;
  while(!done)
  {
    for(i = 0; i < Depth;i++)
    {
      if(target->bucket[i] == ent)/*we found us, we're done*/
      {
        RegionMask[ry][rx].count--;
        target->bucket[i] = NULL;
        return;
      }
    }
    if(!done)
    {
      if(target->next != NULL)target = target->next;/*go to the next level*/
      else
      {
        //fprintf(stderr,"I tried to remove myself from my region, but I wasn't there.");
        return;
      }
    }
  }    
}

Entity *GetClosestEntity(Entity *self,int rx,int ry,int depth)
{
  int i;
  int j;
  int done = 0;
  Entity *temp = NULL;
  Bucket *target;
  if((rx < 0)||(rx >= SPACE_W)||(ry < 0)||(ry >= SPACE_H))
  {
    return NULL;
  }
  target = RegionMask[ry][rx].base;
  if(target == NULL)return NULL;

  while(!done)
  {
    for(i = 0; i < Depth;i++)
    {
      if((target->bucket[i] != NULL)&&(target->bucket[i] != self))/*we found a potential, we might be done*/
      {
        if((target->bucket[i] != self->owner)&&(target->bucket[i]->Unit_Type != self->Unit_Type))
        {
          return target->bucket[i];
        }
      }
    }
    if(!done)
    {
      if(target->next != NULL)target = target->next;/*go to the next level*/
      else
      {
        goto recurse;
      }
    }
  }
  recurse:
      if(depth <= 0)return NULL;/*wa ain't find anything in the area of effect*/
  j = WhatFace(self);
  for(i = 0;i < 8;i++)
  {
    switch(j)
    {
      case F_South:
          temp = GetClosestEntity(self,rx,ry + 1,depth - 1);
      break;
      case F_SW:
          temp = GetClosestEntity(self,rx - 1, ry + 1,depth - 1);
      break;
      case F_West:
          temp = GetClosestEntity(self,rx -1, ry,depth - 1);
      break;
      case F_NW:
          temp = GetClosestEntity(self,rx - 1, ry - 1,depth - 1);
      break;
      case F_North:
          temp = GetClosestEntity(self,rx,ry - 1,depth - 1);
      break;
      case F_NE:
          temp = GetClosestEntity(self,rx + 1, ry - 1,depth - 1);
      break;
      case F_East:
          temp = GetClosestEntity(self, rx + 1, ry,depth - 1);
      break;
      case F_SE:
          temp = GetClosestEntity(self, rx + 1, ry + 1,depth - 1);
      break;
    }
    if(temp != NULL)return temp;
    j = (j+1)%8;
  }
  return NULL;
}

Entity *GetNextEntByRad(Entity *self,int radius,int rx,int ry,Entity *start,int ETMask)
{
  int i;
  int done = 0;
  Bucket *target;
  if((rx < 0)||(rx >= SPACE_W)||(ry < 0)||(ry >= SPACE_H))
  {
    return NULL;
  }
  target = RegionMask[ry][rx].base;
  if(target == NULL)return NULL;
  
  while(!done)
  {
    for(i = 0; i < Depth;i++)
    {
      if(target->bucket[i] == start)/*we found us, we're done*/
      {
        done = 1;
        i++;
        if(i == Depth)
        {
          i = 0;
          if(target->next != NULL)target = target->next;/*go to the next level*/
          else
          {
            return NULL;
          }          
        }
        break;
      }
    }
    if(!done)
    {
      if(target->next != NULL)target = target->next;/*go to the next level*/
      else
      {
        return NULL;
      }
    }
  }    

  while(!done)
  {
    for(; i < Depth;i++)
    {
      if(target->bucket[i]!=NULL)
      {
        if((target->bucket[i]->Unit_Type != ETMask)&&(target->bucket[i]->Unit_Type != ET_Temp))
        {
          if(DistanceBetween(self, target->bucket[i]) <= radius);
              return target->bucket[i];
        }
      }
    }
    if(!done)
    {
      if(target->next != NULL)target = target->next;/*go to the next level*/
      else return NULL;
    }    
  }
  return NULL;  /*we ain't find shit*/
}


Entity *GetEntByBox(SDL_Rect bbox,int rx,int ry,Entity *ignore,int ETMask)
{
  int i;
  int done = 0;
  int deep = 0;
  SDL_Rect bbox2;
  Bucket *target;
  if((rx < 0)||(rx >= SPACE_W)||(ry < 0)||(ry >= SPACE_H))
  {
    return NULL;
  }
  target = RegionMask[ry][rx].base;
  if(target == NULL)return NULL;
  while(!done)
  {
    for(i = 0; i < Depth;i++)
    {
      if(target->bucket[i]!=NULL)
      {
        if((target->bucket[i]->Unit_Type != ETMask)&&(target->bucket[i]->Unit_Type != ET_Temp))
        {
          if(target->bucket[i] != ignore)
          {
            bbox2.w = target->bucket[i]->Boundingbox.w;
            bbox2.h = target->bucket[i]->Boundingbox.h;
            bbox2.x = target->bucket[i]->Boundingbox.x + (int)target->bucket[i]->s.x;
            bbox2.y = target->bucket[i]->Boundingbox.y + (int)target->bucket[i]->s.y;
            
            if(Collide(bbox,bbox2)==1)
            return target->bucket[i];
          }
        }
      }
    }
    if(!done)
    {
      if(target->next != NULL)target = target->next;/*go to the next level*/
      else return NULL;
      deep++;
    }    
  }
  return NULL;  /*we ain't find shit*/
}

void DrawBuckets()
{
  char text[10];
  int i,j;
  for(j = 0;j < Region_H;j++)
  {
    for(i = 0;i < Region_W;i++)
    {
      sprintf(text,"%i",RegionMask[j][i].count);
      DrawText(text,screen,i * 64 - Camera.x,j * 64 - Camera.y,IndexColor(Magenta),F_Small);
    }
  }  
}