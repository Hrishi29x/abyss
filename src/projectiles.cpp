#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "particle.h"
#include "projectiles.h"

void UpdateBullet(Entity *self);
void UpdateGrenade(Entity *self);
void UpdateRocket(Entity *self);
void UpdateExplode(Entity *self);
void UpdateMissile(Entity *self);
void MissileThink(Entity *self);
void UpdateLaser(Entity *self);
void UpdateFlame(Entity *self);
void UpdateSmoke(Entity *self);
void UpdateCBomb(Entity *self);
void FadingLaser(Entity *self);
void UpdateMine(Entity *self);
void UpdateBlade(Entity *self);

extern Uint32 NOW;


/*this will be a generic spawn function that will set some basic info to save code*/
Entity *SpawnProjectile(int sx,int sy,float angle,float speed,float accel,int damage,int dtype,float kick)
{
  float cosine,sine;
  Entity *newent = NULL;
  newent = NewEntity();
  if(newent == NULL)return NULL;
  cosine = cos(angle);
  sine = sin(angle);
  newent->dtype = dtype;
  newent->kick = kick;
  newent->damage = damage;
  newent->gravityent = 0;
  newent->shown = 1;
  newent->legstate = -1;    /*needed if we don't have separate legs*/
  newent->s.x = sx;
  newent->s.y = sy;
  newent->v.x = speed * cosine;
  newent->v.y = speed * sine;
  if(accel != 0)
  {
    newent->v.x = speed * cosine;
    newent->v.y = speed * sine;
    newent->accel = accel;
  } 
  newent->movespeed = (int)speed;
  /*binding the entity to the engine should happen here as well*/
  return newent;
}

Entity *SpawnExplosion(Entity *owner,int sx, int sy,float vx,float vy,int damage)
{
  Entity *newent;
  newent = NewEntity();
  if(newent == NULL)return NULL;
  strcpy(newent->EntName,"Explosion\0");
  newent->sprite = LoadSprite("images/explode1.png",32,32);
  SDL_SetColorKey(newent->sprite->image, SDL_SRCCOLORKEY , SDL_MapRGB(newent->sprite->image->format, 0,0,0));
  newent->frame = 0;
  newent->Unit_Type = 0;
  newent->owner = owner;
  newent->Color = IndexColor(LightYellow);
  newent->legstate = -1;    /*needed if we don't have separate legs*/
  newent->s.x = sx - 16;
  newent->s.y = sy - 16;
  newent->v.x = vx;
  newent->v.y = vy;
  newent->update = UpdateExplode;
  newent->UpdateRate = 60;
  newent->shown = 1;
  newent->lifespan = 12;
  return newent;
}

/*************************************************************
              
                       Sword Blade

 *************************************************************/

Entity *SpawnBlade(Entity *owner,int sx,int sy,float direction,float speed,int length,int damage,float kick,int color,int UType)
{
  Entity *newent = NULL;
  newent = SpawnProjectile(sx - 24,sy - 24,direction,speed,0,damage,DT_Physical,kick);
  if(newent == NULL)return NULL;
  strcpy(newent->EntName,"Blade\0");
  newent->sprite = LoadSwappedSprite("images/blade1.png",48,48,color,0,0);
  newent->sound[SF_ALERT] = LoadSound("sounds/grenlb1b.wav",SDL_MIX_MAXVOLUME>>3);
  newent->update = UpdateBlade;
  newent->UpdateRate = 30;
  newent->movespeed = speed;
  newent->owner = owner;
  newent->Color = color;
  newent->legstate = -1;    /*needed if we don't have separate legs*/
  newent->Unit_Type = UType;
  newent->lifespan = speed;
  newent->legstate = 1;
  newent->Ls.x = (newent->v.x * 2);
  newent->Ls.y = (newent->v.y * 2);
  newent->origin.x = 24 + (newent->v.x * 2);
  newent->origin.y = 24 + (newent->v.y * 2);  
  Get16Face(newent);
  newent->frame = newent->face - 1;
  if(newent->frame < 0)newent->frame = 15;
  newent->trailhead = 0;
  newent->traillen = 0;
  newent->trail[0].x = sx - 24;
  newent->trail[0].y = sy - 24;
  newent->maxtraillen = MAXTRAIL>>1;
  newent->thick = 4;
  newent->Boundingbox.x = 2;
  newent->Boundingbox.y = 2;
  newent->Boundingbox.w = 44;
  newent->Boundingbox.h = 44;
  newent->size.x = 44;
  newent->size.y = 44;
  newent->m.x = (sx + newent->origin.x) >> 6;
  newent->m.y = (sy + newent->origin.y) >> 6;
  AddEntToRegion(newent,newent->m.x,newent->m.y);
  return newent;
  
}

void UpdateBlade(Entity *self)
{
  Entity *target = NULL;
  Coord temp;
  temp.x = self->v.x;
  temp.y = self->v.y;
  self->v.x += self->owner->v.x;
  self->v.y += self->owner->v.y;
  self->lifespan--;
  if(UpdateEntityPosition(self,0))
  {
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_ALERT]->sound,0);
  }
  self->v.x = temp.x;
  self->v.y = temp.y;
  if(self->lifespan == self->movespeed>>1)
  {
    self->v.x *= -1;
    self->v.y *= -1;
    self->frame = (self->frame + 1)%16;
  }
  if(self->lifespan <= 0)
  {
    FreeEntity(self);
    return;
  }
  target = GetTouchingEnt(self);
  if(target != NULL)
  {
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_ALERT]->sound,0);
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
  }
}

/*************************************************************
              
                       Orbital Defense Mines

 *************************************************************/

Entity *SpawnOrbitMine(Entity *owner,int sx,int sy,float radius,float speed,int damage,float kick,int color,int UType)
{
  Entity *newent = NULL;
  newent = SpawnProjectile(sx,sy,0,speed,0,damage,DT_Physical,kick);
  if(newent == NULL)return NULL;
  strcpy(newent->EntName,"Mine\0");
  newent->sprite = LoadSwappedSprite("images/minibomb.png",8,8,color,0,0);
  newent->sound[SF_ALERT] = LoadSound("sounds/deadexplode.wav",SDL_MIX_MAXVOLUME>>3);
  newent->update = UpdateMine;
  newent->UpdateRate = 30;
  newent->movespeed = speed;
  newent->owner = owner;
  newent->target = owner;
  newent->Color = color;
  newent->legstate = -1;    /*needed if we don't have separate legs*/
  newent->Unit_Type = UType;
  newent->lifespan = 600;
  newent->origin.x = 4;
  newent->origin.y = 4;  
  newent->frame = 8;
  newent->sightrange = radius;
  newent->trailhead = 0;
  newent->traillen = 0;
  newent->trail[0].x = sx;
  newent->trail[0].y = sy;
  newent->maxtraillen = MAXTRAIL>>1;
  newent->thick = 1;
  newent->Boundingbox.x = 2;
  newent->Boundingbox.y = 2;
  newent->Boundingbox.w = 6;
  newent->Boundingbox.h = 6;
  newent->size.x = 6;
  newent->size.y = 6;
  newent->m.x = (sx + newent->origin.x) >> 6;
  newent->m.y = (sy + newent->origin.y) >> 6;
  AddEntToRegion(newent,newent->m.x,newent->m.y);
  return newent;
}

void UpdateMine(Entity *self)
{
  Entity *target = NULL;
  self->lifespan--;
  AdjustOrbit(self);
  if((self->lifespan <= 0)||(UpdateEntityPosition(self,1)))
  {
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_ALERT]->sound,0);
    FreeEntity(self);
    return;
  }
  target = GetTouchingEnt(self);
  if(target != NULL)
  {
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_ALERT]->sound,0);
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    FreeEntity(self);    
    return;
  }
}

/*************************************************************
              
                      Dumb Fire Rockets

 *************************************************************/

Entity *SpawnRocket(Entity *owner,int sx,int sy,float angle,float speed,float accel,int damage,float kick,int color,int UType)
{
  Entity *newent = NULL;
  newent = SpawnProjectile(sx,sy,angle,speed,accel,damage,DT_Physical,kick);
  if(newent == NULL)return NULL;
  strcpy(newent->EntName,"Rocket\0");
  newent->sprite = LoadSwappedSprite("images/smallmissile.png",16,16,color,0,0);
  newent->sound[SF_ALERT] = LoadSound("sounds/deadexplode.wav",SDL_MIX_MAXVOLUME>>3);
  newent->update = UpdateRocket;
  newent->UpdateRate = 30;
  newent->movespeed = speed;
  newent->owner = owner;
  newent->Color = color;
  newent->legstate = -1;    /*needed if we don't have separate legs*/
  newent->Unit_Type = UType;
  newent->lifespan = 60;
  newent->origin.x = 8;
  newent->origin.y = 8;  
  Get16Face(newent);
  newent->frame = newent->face;
  if(newent->frame < 0)newent->face = 8;
  newent->Boundingbox.x = 2;
  newent->Boundingbox.y = 2;
  newent->Boundingbox.w = 12;
  newent->Boundingbox.h = 12;
  newent->m.x = (sx + newent->origin.x) >> 6;
  newent->m.y = (sy + newent->origin.y) >> 6;
  AddEntToRegion(newent,newent->m.x,newent->m.y);
  return newent;
}

void UpdateRocket(Entity *self)
{
  Entity *target = NULL;
  self->lifespan--;
 // ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,self->v.x * -1,self->v.y * -1);
  if((self->lifespan <= 0)||(UpdateEntityPosition(self,1)))
  {
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_ALERT]->sound,0);
    DamageRadius(self->owner,self,self->damage,self->damage*2,DT_Physical);
    FreeEntity(self);
    return;
  }
  target = GetTouchingEnt(self);
  if(target != NULL)
  {
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_ALERT]->sound,0);
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    DamageRadius(self->owner,self,self->damage,self->damage*2,DT_Physical);
    FreeEntity(self);    
  }
}

/*************************************************************
              
                      Seeking Missiles

 *************************************************************/

Entity *SpawnMissile(Entity *owner,int sx,int sy,float angle,float speed,float accel,int damage,float kick,int color,int depth,int UType)
{
  Entity *newent = NULL;
  newent = SpawnProjectile(sx,sy,angle,speed,accel,damage,DT_Physical,kick);
  if(newent == NULL)return NULL;
  strcpy(newent->EntName,"Missile\0");
  newent->sprite = LoadSwappedSprite("images/smallmissile.png",16,16,color,0,0);
  newent->sound[SF_ALERT] = LoadSound("sounds/deadexplode.wav",SDL_MIX_MAXVOLUME>>3);
  newent->update = UpdateRocket;
  newent->UpdateRate = 30;
  newent->think = MissileThink;
  newent->ThinkRate = 100;
  newent->NextThink = NOW + 80 + (int)crandom()*4;
  newent->target = NULL;
  newent->movespeed = speed;
  newent->PowerLevel = depth;
  newent->owner = owner;
  newent->Color = color;
  newent->legstate = -1;    /*needed if we don't have separate legs*/
  newent->Unit_Type = UType;
  newent->lifespan = 60;
  newent->origin.x = 8;
  newent->origin.y = 8;  
  Get16Face(newent);
  newent->frame = newent->face;
  if(newent->frame < 0)newent->face = 8;
  newent->Boundingbox.x = 2;
  newent->Boundingbox.y = 2;
  newent->Boundingbox.w = 12;
  newent->Boundingbox.h = 12;
  newent->m.x = (sx + newent->origin.x) >> 6;
  newent->m.y = (sy + newent->origin.y) >> 6;
  AddEntToRegion(newent,newent->m.x,newent->m.y);
  return newent;
}

void UpdateMissile(Entity *self)
{
  Entity *target = NULL;
  self->lifespan--;
 // ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,self->v.x * -1,self->v.y * -1);
  if((self->lifespan <= 0)||(UpdateEntityPosition(self,1)))
  {
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_ALERT]->sound,0);
    DamageRadius(self->owner,self,self->damage,self->damage*2,DT_Physical);
    FreeEntity(self);
    return;
  }
  target = GetTouchingEnt(self);
  if(target != NULL)
  {
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_ALERT]->sound,0);
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    DamageRadius(self->owner,self,self->damage,self->damage*2,DT_Physical);
    FreeEntity(self);    
  }
}

void MissileThink(Entity *self)
{
  Entity *target = NULL;
  if(self->target == NULL)
  {
    target = GetClosestEntity(self,self->m.x,self->m.y,self->PowerLevel);
    if(target != NULL)
    {
      self->target = target;
    }
    else return;
  }
  else if(self->target->health <= 0)
  {
    self->target = NULL;
    return;    
  }
/*  self->a.x = self->target->s.x - self->s.x;
  self->a.y = self->target->s.y - self->s.y;
    VectorScaleTo((int)(self->accel), &self->a.x,&self->a.y);*/
    self->v.x = self->target->s.x - self->s.x;
    self->v.y = self->target->s.y - self->s.y;
    VectorScaleTo(self->movespeed, &self->v.x,&self->v.y);
    Get16Face(self);
    self->frame = self->face;
  
}

/*************************************************************
              
                       Any Size grenade or bomb

 *************************************************************/

Entity *SpawnGrenade(Entity *owner,int sx,int sy,float angle,float speed,int damage,float kick,int size,int color,int UType,int bounce,int gravity)
{
  Entity *newent = NULL;
  newent = SpawnProjectile(sx,sy,angle,speed,0,damage,DT_Physical,kick);
  if(newent == NULL)return NULL;
  strcpy(newent->EntName,"grenade\0");
  switch(size)
  {
    case B_Tiny:
      newent->sprite = LoadSwappedSprite("images/minibomb.png",8,8,color,0,0);
      newent->origin.x = 4;
      newent->origin.y = 4;
      newent->Boundingbox.x = 1;
      newent->Boundingbox.y = 1;
      newent->Boundingbox.w = 6;
      newent->Boundingbox.h = 6;
      newent->sound[SF_DYING] = LoadSound("sounds/bang.wav",SDL_MIX_MAXVOLUME>>4);
      break;
    case B_Small:
      newent->sprite = LoadSwappedSprite("images/bomb.png",16,16,color,Silver,Silver);
      newent->origin.x = 8;
      newent->origin.y = 8;
      newent->Boundingbox.x = 2;
      newent->Boundingbox.y = 2;
      newent->Boundingbox.w = 12;
      newent->Boundingbox.h = 12;
      newent->sound[SF_DYING] = LoadSound("sounds/deadexplode.wav",SDL_MIX_MAXVOLUME>>4);
      break;
    case B_Medium:
      newent->sprite = LoadSwappedSprite("images/giantbomb.png",32,32,color,0,0);
      newent->origin.x = 16;
      newent->origin.y = 16;
      newent->Boundingbox.x = 4;
      newent->Boundingbox.y = 4;
      newent->Boundingbox.w = 24;
      newent->Boundingbox.h = 24;
      newent->sound[SF_DYING] = LoadSound("sounds/ric3.wav",SDL_MIX_MAXVOLUME>>3);
      break;
    default:
      newent->sound[SF_DYING] = LoadSound("sounds/xfire.wav",SDL_MIX_MAXVOLUME>>4);
  }
  newent->sound[SF_PAIN] = LoadSound("sounds/grenlb1b.wav",SDL_MIX_MAXVOLUME>>2);
  newent->sound[SF_IMPACT] = LoadSound("sounds/hgrenb1a.wav",SDL_MIX_MAXVOLUME>>2);
  newent->update = UpdateGrenade;
  newent->owner = owner;
  newent->frame = rand()%16;
  newent->gravityent = gravity;
  newent->bounce = bounce;
  newent->UpdateRate = 35;
  newent->Color = color;
  newent->Unit_Type = UType;
  newent->lifespan = 90;
  newent->trailhead = 0;
  newent->traillen = 0;
  newent->trail[0].x = sx;
  newent->trail[0].y = sy;
  newent->maxtraillen = MAXTRAIL>>1;
  newent->thick = size << 1;
  newent->m.x = (sx + newent->origin.x) >> 6;
  newent->m.y = (sy + newent->origin.y) >> 6;
  newent->v.y -= fabs(newent->v.y) * 0.5;
  AddEntToRegion(newent,newent->m.x,newent->m.y);
  return newent;
}

void UpdateGrenade(Entity *self)
{
  Entity *target = NULL;
  if(UpdateEntityPosition(self,self->bounce))
  {
    if(self->bounce)
    {
      Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_IMPACT + (rand()%2)]->sound,0);
    }
    else
    {
      ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
      Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_DYING]->sound,0);
      DamageTarget(self->owner,self,target,self->damage,self->dtype);
      FreeEntity(self);    
      return;
    }
  }
  self->frame = (self->frame + 1)%16;
  if(self->gravityent)self->v.y += 1.5;
  self->lifespan--;
  if(self->lifespan <= 0)
  {
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_DYING]->sound,0);
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    FreeEntity(self);    
    return;
  }
  target = GetTouchingEnt(self);
  if(target != NULL)
  {
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_DYING]->sound,0);
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    FreeEntity(self);    
    return;
  } 
}
    

/*************************************************************
              
                       Cluster Bomb & Clusters

 *************************************************************/

Entity *SpawnCluster(Entity *owner,int sx,int sy,float angle,float speed,int damage,float kick,int size,int color,int UType,int bounce,int gravity)
{
  Entity *newent = NULL;
  newent = SpawnProjectile(sx,sy,angle,speed,0,damage,DT_Physical,kick);
  if(newent == NULL)return NULL;
  strcpy(newent->EntName,"cluster\0");
  switch(size)
  {
    case B_Small:
      newent->lifespan = 30 + (rand()%5);
      newent->update = UpdateGrenade;
      newent->sprite = LoadSwappedSprite("images/minicluster.png",16,16,color,0,0);
      newent->origin.x = 8;
      newent->origin.y = 8;
      newent->Boundingbox.x = 2;
      newent->Boundingbox.y = 2;
      newent->Boundingbox.w = 12;
      newent->Boundingbox.h = 12;
      newent->sound[SF_DYING] = LoadSound("sounds/bang.wav",SDL_MIX_MAXVOLUME>>4);
      break;
    case B_Medium:
      newent->lifespan = 40 + (rand()%10);
      newent->update = UpdateCBomb;
      newent->sprite = LoadSwappedSprite("images/cluster.png",32,32,color,0,0);
      newent->origin.x = 16;
      newent->origin.y = 16;
      newent->Boundingbox.x = 4;
      newent->Boundingbox.y = 4;
      newent->Boundingbox.w = 24;
      newent->Boundingbox.h = 24;
      newent->sound[SF_DYING] = LoadSound("sounds/ric3.wav",SDL_MIX_MAXVOLUME>>3);
      break;
    default:
      newent->sound[SF_DYING] = LoadSound("sounds/xfire.wav",SDL_MIX_MAXVOLUME>>4);
  }
  newent->sound[SF_PAIN] = LoadSound("sounds/grenlb1b.wav",SDL_MIX_MAXVOLUME>>2);
  newent->sound[SF_IMPACT] = LoadSound("sounds/hgrenb1a.wav",SDL_MIX_MAXVOLUME>>2);
  newent->owner = owner;
  newent->frame = rand()%16;
  newent->gravityent = gravity;
  newent->bounce = bounce;
  newent->UpdateRate = 35;
  newent->Color = color;
  newent->Unit_Type = UType;
  newent->trailhead = 0;
  newent->traillen = 0;
  newent->trail[0].x = sx;
  newent->trail[0].y = sy;
  newent->maxtraillen = MAXTRAIL>>1;
  newent->thick = size << 1;
  newent->m.x = (sx + newent->origin.x) >> 6;
  newent->m.y = (sy + newent->origin.y) >> 6;
  newent->v.y -= fabs(newent->v.y) * 0.5;
  AddEntToRegion(newent,newent->m.x,newent->m.y);
  return newent;
}

void ExplodeCluster(Entity *self,int num)
{
  int i;
  for(i = 0; i < num;i++)
  {
    SpawnCluster(self->owner,self->s.x + self->origin.x + crandom(),self->s.y + self->origin.y + crandom(),GetRadFromDir(rand()%8),self->movespeed + crandom(),self->damage, self->kick, B_Small, self->Color, self->Unit_Type, self->bounce, self->gravityent);
  }

}

void UpdateCBomb(Entity *self)
{
  Entity *target = NULL;
  if(UpdateEntityPosition(self,self->bounce))
  {
    if(self->bounce)
    {
      Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_IMPACT + (rand()%2)]->sound,0);
    }
    else
    {
      ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
      Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_DYING]->sound,0);
      DamageTarget(self->owner,self,target,self->damage,self->dtype);
      ExplodeCluster(self,12);
      FreeEntity(self);    
      return;
    }
  }
  self->frame = (self->frame + 1)%16;
  if(self->gravityent)self->v.y += 1.8;
  self->lifespan--;
  if(self->lifespan <= 0)
  {
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_DYING]->sound,0);
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    ExplodeCluster(self,12);
    FreeEntity(self);    
    return;
  }
  target = GetTouchingEnt(self);
  if(target != NULL)
  {
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_DYING]->sound,0);
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    ExplodeCluster(self,12);
    FreeEntity(self);    
    return;
  } 
}
    
/*************************************************************
              
                       Any Size bullet

 *************************************************************/


Entity *SpawnBullet(Entity *owner,int sx,int sy,float angle,float speed,int damage,float kick,int size,int color,int UType)
{
  Entity *newent = NULL;
  newent = SpawnProjectile(sx,sy,angle,speed,0,damage,DT_Physical,kick);
  if(newent == NULL)return NULL;
  newent->maxtraillen = 2;
  switch(size)
  {
    case B_Tiny:
      newent->sprite = LoadSwappedSprite("images/tinybullet.png",5,5,color,0,0);
      newent->origin.x = 3;
      newent->origin.y = 3;
      newent->Boundingbox.x = 1;
      newent->Boundingbox.y = 1;
      newent->Boundingbox.w = 3;
      newent->Boundingbox.h = 3;
      newent->sound[SF_ALERT] = LoadSound("sounds/ric1.wav",SDL_MIX_MAXVOLUME>>4);
      break;
    case B_Small:
      newent->sprite = LoadSwappedSprite("images/smallbullet.png",8,8,color,0,0);
      newent->origin.x = 4;
      newent->origin.y = 4;
      newent->Boundingbox.x = 1;
      newent->Boundingbox.y = 1;
      newent->Boundingbox.w = 6;
      newent->Boundingbox.h = 6;
      newent->sound[SF_ALERT] = LoadSound("sounds/ric2.wav",SDL_MIX_MAXVOLUME>>4);
      break;
    case B_Medium:
      newent->sprite = LoadSwappedSprite("images/bullet.png",12,12,color,0,0);
      newent->origin.x = 6;
      newent->origin.y = 6;
      newent->Boundingbox.x = 2;
      newent->Boundingbox.y = 2;
      newent->Boundingbox.w = 8;
      newent->Boundingbox.h = 8;
      newent->sound[SF_ALERT] = LoadSound("sounds/ric3.wav",SDL_MIX_MAXVOLUME>>3);
      break;
    case B_Large:
      newent->maxtraillen = 4;
      newent->sprite = LoadSwappedSprite("images/Largebullet.png",16,16,color,0,0);
      newent->origin.x = 8;
      newent->origin.y = 8;
      newent->Boundingbox.x = 3;
      newent->Boundingbox.y = 3;
      newent->Boundingbox.w = 10;
      newent->Boundingbox.h = 10;
      newent->sound[SF_ALERT] = LoadSound("sounds/xfire.wav",SDL_MIX_MAXVOLUME>>2);
      break;
    case B_Huge:
      newent->sprite = LoadSwappedSprite("images/Hugebullet.png",24,24,color,0,0);
      newent->origin.x = 12;
      newent->origin.y = 12;
      newent->Boundingbox.x = 4;
      newent->Boundingbox.y = 4;
      newent->Boundingbox.w = 16;
      newent->Boundingbox.h = 16;
      newent->sound[SF_ALERT] = LoadSound("sounds/deadexplode.wav",SDL_MIX_MAXVOLUME>>1);
      newent->maxtraillen = 5;
      break;
    case B_Atomic:
      newent->maxtraillen = 6;
      newent->sprite = LoadSwappedSprite("images/Atomicbullet.png",32,32,color,0,0);
      newent->origin.x = 16;
      newent->origin.y = 16;
      newent->Boundingbox.x = 5;
      newent->Boundingbox.y = 5;
      newent->Boundingbox.w = 22;
      newent->Boundingbox.h = 22;
      newent->sound[SF_ALERT] = LoadSound("sounds/machinexplode.wav",SDL_MIX_MAXVOLUME);
      break;
    default:
      newent->sound[SF_ALERT] = LoadSound("sounds/ric1.wav",SDL_MIX_MAXVOLUME>>4);
  }
  SDL_SetColorKey(newent->sprite->image, SDL_SRCCOLORKEY , SDL_MapRGB(newent->sprite->image->format, 0,0,0));
  newent->frame = 0;
  newent->owner = owner;
  newent->update = UpdateBullet;
  newent->UpdateRate = 30;
  newent->Color = color;
  newent->Unit_Type = UType;
  newent->lifespan = 60;
  newent->thick = 1;
  newent->trailhead = 0;
  newent->traillen = 0;
  newent->trail[0].x = sx;
  newent->trail[0].y = sy;
  newent->m.x = (sx + newent->origin.x) >> 6;
  newent->m.y = (sy + newent->origin.y) >> 6;
  AddEntToRegion(newent,newent->m.x,newent->m.y);
  return newent;
}


void UpdateBullet(Entity *self)
{
  Entity *target = NULL;
  self->lifespan--;
  if(self->lifespan <= 0)
  {
    FreeEntity(self);
    return;  
  }
  if(UpdateEntityPosition(self,0))
  {
    if(self->sound[SF_ALERT]->sound != NULL)Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_ALERT]->sound,0);
    FreeEntity(self);
    return;  
  }
  target = GetTouchingEnt(self);
  if(target != NULL)
  {
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    FreeEntity(self);    
  }
}

/*************************************************************
              
                       Basic Laser Projectile

 *************************************************************/

Entity *SpawnLaser(Entity *owner,int sx,int sy,float angle,float speed,int damage,float kick,int size, int color,int UType,int gravity)
{
  Entity *newent = NULL;
  newent = SpawnProjectile(sx,sy,angle,speed,0,damage,DT_Magical,kick);
  if(newent == NULL)return NULL;
  strcpy(newent->EntName,"Laser\0");
  newent->sound[SF_ALERT] = LoadSound("sounds/bluehit.wav",SDL_MIX_MAXVOLUME>>3);
  newent->origin.x = 2;
  newent->origin.y = 2;
  newent->owner = owner;
  newent->frame = 0;
  newent->gravityent = gravity;
  newent->update = UpdateLaser;
  newent->UpdateRate = 30;
  newent->Color = color;
  newent->Unit_Type = UType;
  newent->lifespan = 90;
  newent->trailhead = 0;
  newent->traillen = 0;
  newent->trail[0].x = sx;
  newent->trail[0].y = sy;
  newent->maxtraillen = MAXTRAIL - 1;
  newent->Boundingbox.x = 1;
  newent->Boundingbox.y = 1;
  newent->thick = size;
  newent->Boundingbox.w = 2;
  newent->Boundingbox.h = 2;
  newent->m.x = (sx + newent->origin.x) >> 6;
  newent->m.y = (sy + newent->origin.y) >> 6;
  AddEntToRegion(newent,newent->m.x,newent->m.y);
  return newent;
}


void UpdateLaser(Entity *self)
{
  Entity *target = NULL;
  if(UpdateEntityPosition(self,1))Mix_PlayChannel(Mix_GroupAvailable(2),self->sound[SF_ALERT]->sound,0);
  
  if(self->gravityent)self->v.y += 1.5;
  self->lifespan--;
  if(self->lifespan <= 0)
  {
    self->update = FadingLaser;
    self->v.x = 0;
    self->v.y = 0;
    self->a.x = 0;
    self->a.y = 0;
    self->fcount = self->maxtraillen;
    return;
  }
  target = GetTouchingEnt(self);
  if(target != NULL)
  {
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    self->update = FadingLaser;
    self->v.x = 0;
    self->v.y = 0;
    self->a.x = 0;
    self->a.y = 0;
    self->fcount = self->maxtraillen;
  }
}

void FadingLaser(Entity *self)
{
  UpdateEntityPosition(self,0);
  self->fcount--;
  if(self->fcount <= 0)
      FreeEntity(self);
}

/*************************************************************
              
                       Flame Thrower segments

 *************************************************************/

Entity *SpawnFlame(Entity *owner,int sx,int sy,float angle,float speed,int damage, float kick, int length,int UType)
{
  Entity *newent;
  newent = SpawnProjectile(sx,sy,angle,speed,10,damage,DT_Magical,kick);
  if(newent == NULL)return NULL;
  strcpy(newent->EntName,"Flame\0");
  newent->sprite = LoadSprite("images/flame1.png",48,48);
  SDL_SetColorKey(newent->sprite->image, SDL_SRCCOLORKEY , SDL_MapRGB(newent->sprite->image->format, 0,0,0));
  newent->update = UpdateFlame;
  newent->UpdateRate = 30;
  newent->owner = owner;
  newent->movespeed = speed;
  newent->Color = LightOrange;
  newent->legstate = -1;    /*needed if we don't have separate legs*/
  newent->Unit_Type = UType;
  newent->lifespan = length;
  newent->origin.x = 24;
  newent->origin.y = 24;  
  newent->fcount = 8;
  newent->frate = 8;
  newent->frame = 0;
  newent->Boundingbox.x = 22;
  newent->Boundingbox.y = 22;
  newent->Boundingbox.w = 4;
  newent->Boundingbox.h = 4;
  newent->m.x = (sx + newent->origin.x) >> 6;
  newent->m.y = (sy + newent->origin.y) >> 6;
  AddEntToRegion(newent,newent->m.x,newent->m.y);
  return newent;
}


void UpdateFlame(Entity *self)
{
  Entity *target = NULL;
  UpdateEntityPosition(self,0);
  if(self->fcount <= 0)
  {
    self->fcount = self->frate;
//    SpawnSpriteParticle("images/flame1.png",48,48,(self->frame>>2) + 1,self->s.x + self->origin.x - (self->v.x * 2),self->s.y + self->origin.y - (self->v.y * 2), self->v.x * 0.5, self->v.y * 0.5, self->lifespan>>2);
    
    if(self->frame < 7)
    {
      self->frame++;
      self->Boundingbox.x-=2;
      self->Boundingbox.y-=2;
      self->Boundingbox.w += 4;
      self->Boundingbox.h += 4;
    }
  }
  else self->fcount--;
  self->lifespan--;
  if(self->lifespan <= 0)
  {
    self->Unit_Type = ET_Temp;
    self->frame += 8;
    self->v.x += crandom();
    self->v.y = (fabs(self->v.y) * -1) + crandom();
    self->update = UpdateSmoke;
    return;
  }
  target = GetTouchingEnt(self);
  if(target != NULL)
  {
    DamageTarget(self->owner,self,target,self->damage,self->dtype);
    self->Unit_Type = ET_Temp;
    self->frame += 8;
    self->update = UpdateSmoke;
    return;
  }
//  SpawnSpriteParticle("images/flame1.png",48,48,(self->frame>>1) + 1,self->s.x + self->origin.x - (self->v.x * 2),self->s.y + self->origin.y - (self->v.y * 2), self->v.x * 0.5, self->v.y * 0.5, self->lifespan>>2);
}

void UpdateSmoke(Entity *self)
{
  self->s.x += self->v.x;
  self->s.y += self->v.y;
  self->frame--;
  if(self->frame <= 7)FreeEntity(self);
}
void UpdateExplode(Entity *self)
{
  self->s.x += self->v.x;
  self->s.y += self->v.y;
  self->frame++;
  if(self->frame > 12)FreeEntity(self);
}

/*************************************************************
              
                       Projectile Support Functions

 *************************************************************/
void PrecacheProjSounds()
{
  LoadSound("sounds/deadexplode.wav",SDL_MIX_MAXVOLUME>>2);
  LoadSound("sounds/explode.wav",SDL_MIX_MAXVOLUME>>3);
  LoadSound("sounds/bluehit.wav",SDL_MIX_MAXVOLUME>>3);
  LoadSound("sounds/machgf1b.wav",SDL_MIX_MAXVOLUME>>3);
  LoadSound("sounds/machinexplode.wav",SDL_MIX_MAXVOLUME);
  LoadSound("sounds/xfire.wav",SDL_MIX_MAXVOLUME>>2);
  LoadSound("sounds/ric1.wav",SDL_MIX_MAXVOLUME>>4);
  LoadSound("sounds/ric2.wav",SDL_MIX_MAXVOLUME>>3);
  LoadSound("sounds/ric3.wav",SDL_MIX_MAXVOLUME>>2);
  LoadSound("sounds/hgrenb1a.wav",SDL_MIX_MAXVOLUME>>4);
  LoadSound("sounds/grenlb1b.wav",SDL_MIX_MAXVOLUME>>4);
  LoadSound("sounds/bang.wav",SDL_MIX_MAXVOLUME>>3);
}

/*I am not going to calculate the radians based on the 8 basic directions each time,
  These are the numbers and I see no reason not use them.*/
  
Entity *GetTouchingEnt(Entity *self)
{
  Entity *target = NULL;
  SDL_Rect bbox;
  bbox.w = self->Boundingbox.w;
  bbox.h = self->Boundingbox.h;
  bbox.x = self->Boundingbox.x + (int)self->s.x;
  bbox.y = self->Boundingbox.y + (int)self->s.y;
  target = GetEntByBox(bbox,self->m.x,self->m.y,self,self->Unit_Type);
  if(target != NULL)return target;
  target = GetEntByBox(bbox,self->m.x + 1,self->m.y - 1,self,self->Unit_Type);
  if(target != NULL)return target;
  target = GetEntByBox(bbox,self->m.x + 1,self->m.y + 1,self,self->Unit_Type);
  if(target != NULL)return target;
  target = GetEntByBox(bbox,self->m.x + 1,self->m.y,self,self->Unit_Type);
  if(target != NULL)return target;
  target = GetEntByBox(bbox,self->m.x - 1,self->m.y - 1,self,self->Unit_Type);
  if(target != NULL)return target;
  target = GetEntByBox(bbox,self->m.x - 1,self->m.y + 1,self,self->Unit_Type);
  if(target != NULL)return target;
  target = GetEntByBox(bbox,self->m.x - 1,self->m.y,self,self->Unit_Type);
  if(target != NULL)return target;
  target = GetEntByBox(bbox,self->m.x,self->m.y - 1,self,self->Unit_Type);
  if(target != NULL)return target;
  target = GetEntByBox(bbox,self->m.x,self->m.y + 1,self,self->Unit_Type);
  if(target != NULL)return target;
  return NULL;
}


float GetRadFromDir(int dir)
{
  switch(dir)
  {
    case F_South:
      return 1.57;
    case F_SW:
      return 2.3561;
    case F_West:
      return 3.14159;
    case F_NW:
      return 3.926;
    case F_North:
      return 4.712;
    case F_NE:
      return 5.4977;
    case F_East:
      return 0;
    case F_SE:
      return  0.7853;
  }
  return 0;
}