#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "particle.h"
 
#define  JT_Button  0
#define  JT_Axis    1
#define  JT_Inverse 2
#define  Wiggle     6000
#define  AWiggle    -6000

extern SDL_Surface *screen;
extern SDL_Joystick *joy;

enum PlayerInputs {PI_MovDown,PI_MovDownLeft,PI_MovLeft,PI_MovUpLeft,PI_MovUp,PI_MovUpRight,PI_MovRight,PI_MovDownRight,
                   PI_FireDown,PI_FireDownLeft,PI_FireLeft,PI_FireUpLeft,PI_FireUp,PI_FireUpRight,PI_FireRight,PI_FireDownRight,
                   PI_FireUltimate,PI_NextWeapon,PI_PreviousWeapon,PI_Pause,PI_NULL};

Uint32 KeyButtons[PI_NULL];

int PlayerCommands[3]; 
Entity *ThePlayer = NULL;
SDL_Surface *titlebar = NULL;
int attackleft = SDLK_LEFT;
void UpdatePlayer(Entity *self);
void PlayerThink(Entity *self);
void FinalOutput();
void UpdateInput();

PlayerStats ThisPlayer_Stats;


void SpawnPlayer(int x,int y)
{
  Entity *newent = NULL;
  newent = NewEntity();
  if(newent == NULL)
  {
    fprintf(stderr,"Unable to generate player entity; %s",SDL_GetError());
    exit(0);
  }
  strcpy(newent->EntName,"Player\0");
  newent->sprite = LoadSwappedSprite("images/core1.png",48,48,LightGreen,White,Grey);
  newent->legs = LoadSwappedSprite("images/legs1.png",48,48,LightGreen,White,Silver);
  ThisPlayer_Stats.color[PC_Core1] = Blue;
  ThisPlayer_Stats.color[PC_Core2] = DarkRed;
  ThisPlayer_Stats.color[PC_Leg1] = Blue;
  ThisPlayer_Stats.color[PC_Leg2] = DarkRed;
  ThisPlayer_Stats.color[PC_Leg3] = Silver;
  ThisPlayer_Stats.color[PC_Gun] = DarkGrey;
  ThisPlayer_Stats.color[PC_Trail] = LightBlue;
  SDL_SetColorKey(newent->sprite->image, SDL_SRCCOLORKEY , SDL_MapRGB(newent->sprite->image->format, 0,0,0));
  SDL_SetColorKey(newent->legs->image, SDL_SRCCOLORKEY , SDL_MapRGB(newent->legs->image->format, 0,0,0));
  newent->size.x = newent->sprite->w;
  newent->size.y = newent->sprite->h;
  newent->update = UpdatePlayer;
  newent->think = PlayerThink;
  newent->UpdateRate = 35;
  newent->ThinkRate = 35;
  newent->legstate = 1;
  newent->takedamage = 1;
  newent->Score = 0;
  newent->Unit_Type = ET_Player;
  newent->switchdelay = 0;
  newent->healthmax = 100;
  newent->health = 100;
  newent->frame = 0;
  newent->Color = LightBlue;
  newent->frate = 1;
  newent->legframe = 0;
  newent->state = ST_IDLE;
  newent->aimdir = F_East;
  newent->thick = 1;
  newent->trailhead = -1;
  newent->s.x = x;
  newent->s.y = y;
  newent->v.x = 0;
  newent->v.y = 0;
  newent->Ls.x = -6;
  newent->Ls.y = -16;
  newent->maxspeed = 12;
  newent->movespeed = 0;
  newent->accel = 4;
  newent->totaloffset = 24;
  newent->Boundingbox.x = 10;
  newent->Boundingbox.y = 10;
  newent->Boundingbox.w = 28;
  newent->Boundingbox.h = 50;  
  newent->origin.x = 24;
  newent->origin.y = 32;
  newent->currentweapon = 0;
  UpdatePlayer(newent);
  ThePlayer = newent;
  atexit(FinalOutput);
}

/*
 * Think Function handles giving the entity new orders and changing states
 */

void PlayerThink(Entity *self)
{
  float t;
  char text[40];
  Uint8 *keys = SDL_GetKeyState(NULL);
  UpdateInput();
  if(self->switchdelay <= 0)
  {
    if(PlayerCommands[1] == PI_NextWeapon)
    {
      //one command
    }
    if(PlayerCommands[1] == PI_PreviousWeapon)
    {
      //another command
    }    
  }
  else self->switchdelay--;
  if((self->state != ST_DEAD)&&(self->state != ST_DIE))
  {
    /*testing (cheating)*/
    if(keys[SDLK_1])self->PowerLevel = 0;
    else if(keys[SDLK_2])self->PowerLevel = 1;
    else if(keys[SDLK_3])self->PowerLevel = 2;
    else if(keys[SDLK_4])self->PowerLevel = 3;
    /*player movement follows*/
    else if(PlayerCommands[0] == PI_MovUpLeft)
    {     
      t = self->accel * 1.41; 
      self->grounded = 0;
      self->v.y -= t;
      self->v.x -= t;
      self->movespeed = VectorLength(self->v.x,self->v.y);
      if(self->movespeed > self->maxspeed)
      {
        self->v.y = self->maxspeed * 1.41 * -1;
        self->v.x = self->v.y;
        self->movespeed = self->maxspeed;
      }
      SpawnThrust(IndexColor(self->Color),self->s.x + 24,self->s.y + 24,1.4,1.4,self->movespeed,self->movespeed * 4);
    }
    else if(PlayerCommands[0] == PI_MovUpRight)
    {     
      t = self->accel * 1.41; 
      self->v.y -= t;
      self->v.x += t;
      self->grounded = 0;
      self->movespeed = VectorLength(self->v.x,self->v.y);
      if(self->movespeed > self->maxspeed)
      {
        self->v.x = self->maxspeed * 1.41;
        self->v.y = self->v.x * -1;
        self->movespeed = self->maxspeed;
      }
      SpawnThrust(IndexColor(self->Color),self->s.x + 24,self->s.y + 24,-1.4,1.4,self->movespeed,self->movespeed * 4);
    }
    else if(PlayerCommands[0] == PI_MovDownRight)
    {     
      t = self->accel * 1.41; 
      self->v.y += t;
      self->v.x += t;
      self->grounded = 0;
      self->movespeed = VectorLength(self->v.x,self->v.y);
      if(self->movespeed > self->maxspeed)
      {
        self->v.y = self->maxspeed * 1.41;
        self->v.x = self->v.y;
        self->movespeed = self->maxspeed;
      }
      SpawnThrust(IndexColor(self->Color),self->s.x + 24,self->s.y + 24,-1.4,-1.4,self->movespeed,self->movespeed * 4);
    }
    else if(PlayerCommands[0] == PI_MovDownLeft)
    {     
      t = self->accel * 1.41; 
      self->v.y += t;
      self->v.x -= t;
      self->grounded = 0;
      self->movespeed = VectorLength(self->v.x,self->v.y);
      if(self->movespeed > self->maxspeed)
      {
        self->v.y = self->maxspeed * 1.41;
        self->v.x = self->v.y * -1;
        self->movespeed = self->maxspeed;
      }
      SpawnThrust(IndexColor(self->Color),self->s.x + 24,self->s.y + 24,1.4,-1.4,self->movespeed,self->movespeed * 4);
    }
    else if(PlayerCommands[0] == PI_MovUp)
    {      
      self->v.y -= self->accel;
      self->movespeed = VectorLength(self->v.x,self->v.y);
      self->grounded = 0;
      if(self->movespeed > self->maxspeed)
      {
        self->v.x = 0;
        self->v.y = self->maxspeed * -1;
        self->movespeed = self->maxspeed;
      }
      SpawnThrust(IndexColor(self->Color),self->s.x + 24,self->s.y + 24,0,2,self->movespeed,self->movespeed * 4);
    }
    else if(PlayerCommands[0] == PI_MovDown)
    {      
      self->v.y += self->accel;
      self->movespeed = VectorLength(self->v.x,self->v.y);
      self->grounded = 0;
      if(self->movespeed > self->maxspeed)
      {
        self->v.x = 0;
        self->v.y = self->maxspeed;
        self->movespeed = self->maxspeed;
      }
      SpawnThrust(IndexColor(self->Color),self->s.x + 24,self->s.y + 24,0,-1,self->movespeed,self->movespeed * 4);
    }
    else if(PlayerCommands[0] == PI_MovLeft)
    {      
      self->v.x -= self->accel;
      self->movespeed = VectorLength(self->v.x,self->v.y);
      self->grounded = 0;
      if(self->movespeed > self->maxspeed)
      {
        self->v.y = 0;
        self->v.x = self->maxspeed * -1;
        self->movespeed = self->maxspeed;
      }
      SpawnThrust(IndexColor(self->Color),self->s.x + 24,self->s.y + 24,2,0,self->movespeed,self->movespeed * 4);
    }
    else if(PlayerCommands[0] == PI_MovRight)
    {      
      self->v.x += self->accel;
      self->movespeed = VectorLength(self->v.x,self->v.y);
      self->grounded = 0;
      if(self->movespeed > self->maxspeed)
      {
        self->v.y = 0;
        self->v.x = self->maxspeed;
        self->movespeed = self->maxspeed;
      }
      SpawnThrust(IndexColor(self->Color),self->s.x + 24,self->s.y + 24,-2,0,self->movespeed,self->movespeed * 4);
    }

    switch(self->state)
    {
      case ST_IDLE: 
        if(PlayerCommands[1] == PI_FireUltimate)
        {
          self->frame = 12;
          self->state = ST_ATTACK1;
          self->aimdir = F_North;
          self->Ls.x = -5;
          self->Ls.y = -26;
          //FireUltimateWeapon(self);
        }
        else if(keys[SDLK_r])
        {
          //SpawnOrbitMine(self,self->s.x + self->origin.x,self->s.y - 5,20,10,50,0,self->Color,self->Unit_Type);
          self->frame = 12;
          self->state = ST_ATTACK1;
          self->aimdir = F_North;
          self->Ls.x = -5;
          self->Ls.y = -26;
          self->Cooldown = 3;                    
        }
        else if(PlayerCommands[1] == PI_FireUpLeft)
        {
          self->frame = 9;
          self->state = ST_ATTACK1;
          self->aimdir = F_NW;
          self->Ls.x = -6;
          self->Ls.y = -26;
          //FireWeapon(self,F_NW);
        }
        else if(PlayerCommands[1] == PI_FireUp)
        {
          self->frame = 12;
          self->state = ST_ATTACK1;
          self->aimdir = F_North;
          self->Ls.x = -5;
          self->Ls.y = -26;
          //FireWeapon(self,F_North);
        }    
        else if(PlayerCommands[1] == PI_FireUpRight)
        {
          self->frame = 15;
          self->state = ST_ATTACK1;
          self->aimdir = F_NE;
          self->Ls.x = 0;
          self->Ls.y = -26;
          //FireWeapon(self,F_NE);
        }    
        else if(PlayerCommands[1] == PI_FireLeft)
        {
          self->frame = 6;
          self->state = ST_ATTACK1;
          self->aimdir = F_West;
          self->Ls.x = -8;
          self->Ls.y = -28;
          //FireWeapon(self,F_West);
        }    
        else if(PlayerCommands[1] == PI_FireRight)
        {
          self->frame = 18;
          self->state = ST_ATTACK1;
          self->aimdir = F_East;
          self->Ls.x = 4;
          self->Ls.y = -28;
          //FireWeapon(self,F_East);
        }    
        else if(PlayerCommands[1] == PI_FireDownLeft)
        {
          self->frame = 3;
          self->state = ST_ATTACK1;
          self->aimdir = F_SW;
          self->Ls.x = -5;
          self->Ls.y = -14;
          //FireWeapon(self,F_SW);
        }    
        else if(PlayerCommands[1] == PI_FireDown)
        {
          self->frame = 0;
          self->state = ST_ATTACK1;
          self->aimdir = F_South;
          self->Ls.x = -6;
          self->Ls.y = -14;
          //FireWeapon(self,F_South);
        }    
        else if(PlayerCommands[1] == PI_FireDownRight)
        {
          self->frame = 21;
          self->state = ST_ATTACK1;
          self->aimdir = F_SE;
          self->Ls.x = 4;
          self->Ls.y = -14;
          //FireWeapon(self,F_SE);
        }    
        
        break;
      case ST_ATTACK1:
      break;
    }
  }
}

/*updates the entity's postition and handles animation*/

void UpdatePlayer(Entity *self)
{
  int Goframe = 0;
  UpdateEntityPosition(self,0);
  
  if(self->grounded == 0)
  {
    if(self->v.y < 20);
    self->v.y += 2;     /*gravity at work*/
  }
  ApplyFriction(self,0.05);
  if(self->state == ST_DIE)
  {
    self->fcount = 10;
    self->state = ST_DEAD;
    return;
  }
  else if(self->state == ST_DEAD)
  {
    self->fcount--;
    if(self->fcount <= 0)
    {
      FreeEntity(self);
      exit(0);
    }
    ExplodingParticle(self->s.x + self->origin.x,self->s.y + self->origin.y,crandom(),crandom());
    return;
  }
  else if(self->Cooldown > 0)self->Cooldown--;/*weapons cooloff*/
  else
  {
    self->state = ST_IDLE;
  }
  if(self->KillCount > 0)
  {
    self->Score += (self->KillCount * 10) * ((self->KillCount>>1) + 1);
    self->KillCount = 0;
  }
  if(self->fcount <= 0)
  {
    Goframe = 1;
    self->fcount = self->frate;
  }
  else
  {
    self->fcount--;
  }
  if(!Goframe)return;
  else GetFace(self);/*check out movement vector to see what direction we are facing.*/
  /*only frame animations should take place next*/
  switch(self->face)
  {
    case F_NULL:
      self->legframe = 0;
    break;
    case F_North:
      self->legframe++;
      if((self->legframe > 9)||(self->legframe < 7))self->legframe = 7;
    break;
    case F_South:
      self->legframe++;
      if((self->legframe > 3)||(self->legframe < 1))self->legframe = 1;
    break;
    case F_SW:
    case F_NW:
    case F_West:
      self->legframe++;
      if((self->legframe > 6)||(self->legframe < 4))self->legframe = 4;
    break;
    case F_SE:
    case F_NE:
    case F_East:
      self->legframe++;
      if((self->legframe > 12)||(self->legframe < 10))self->legframe = 10;
    break;

      
  }
  if(self->state == ST_ATTACK1)
  {
    self->frame++;
    switch(self->aimdir)
    {
      case F_South:
        if(self->frame > 2)self->frame = 0;
        break;
      case F_SW:
        if((self->frame > 5)||(self->frame < 3))self->frame = 3;
        break;
      case F_West:
        if((self->frame > 8)||(self->frame < 6))self->frame = 6;
        break;
      case F_NW:
        if((self->frame > 11)||(self->frame < 9))self->frame = 9;
        break;
      case F_North:
        if((self->frame > 14)||(self->frame < 12))self->frame = 12;
        break;
      case F_NE:
        if((self->frame > 17)||(self->frame < 15))self->frame = 15;
        break;
      case F_East:
        if((self->frame > 20)||(self->frame < 18))self->frame = 18;
        break;
      case F_SE:
        if((self->frame > 23)||(self->frame < 21))self->frame = 21;
        break;
      default:
        self->frame = 0;
    }
  }
}

int CheckCommand(int i)
{
  Uint8 *keys = SDL_GetKeyState(NULL);
  if(keys[KeyButtons[i]])
    return 1;
  return 0;
}

void UpdateInput()
{
  int i;
  for(i = 0;i < 3;i++)PlayerCommands[i] = -1;
  
  i = PI_MovDown;
  if(CheckCommand(i))
    PlayerCommands[0] = i;
  
  i = PI_MovUp;
  if(CheckCommand(i))
    PlayerCommands[0] = i;
    
  i = PI_MovLeft;
  if(CheckCommand(i))
  {
    if(PlayerCommands[0] == PI_MovUp)
    {
      PlayerCommands[0] = PI_MovUpLeft; 
    }
    else if(PlayerCommands[0] == PI_MovDown)
    {
      PlayerCommands[0] = PI_MovDownLeft; 
    }
    else PlayerCommands[0] = i;
  }
    
  i = PI_MovRight;
  if(CheckCommand(i))
  {
    if(PlayerCommands[0] == PI_MovUp)
    {
      PlayerCommands[0] = PI_MovUpRight; 
    }
    else if(PlayerCommands[0] == PI_MovDown)
    {
      PlayerCommands[0] = PI_MovDownRight; 
    }
    else PlayerCommands[0] = i;
  }
  
  
  if(CheckCommand(PI_FireDown))
    PlayerCommands[1] = PI_FireDown;

  if(CheckCommand(PI_FireUp))
    PlayerCommands[1] = PI_FireUp;
  
  i = PI_FireLeft;
  if(CheckCommand(i))
  {
    if(PlayerCommands[1] == PI_FireUp)
    {
      PlayerCommands[1] = PI_FireUpLeft; 
    }
    else if(PlayerCommands[1] == PI_FireDown)
    {
      PlayerCommands[1] = PI_FireDownLeft; 
    }
    else PlayerCommands[1] = i;
  }

  i = PI_FireRight;
  if(CheckCommand(i))
  {
    if(PlayerCommands[1] == PI_FireUp)
    {
      PlayerCommands[1] = PI_FireUpRight; 
    }
    else if(PlayerCommands[1] == PI_FireDown)
    {
      PlayerCommands[1] = PI_FireDownRight; 
    }
    else PlayerCommands[1] = i;
  }
  
  if(CheckCommand(PI_FireUltimate))
    PlayerCommands[1] = PI_FireUltimate;  
  
  if(CheckCommand(PI_NextWeapon))
    PlayerCommands[1] = PI_NextWeapon;  
  
  if(CheckCommand(PI_PreviousWeapon))
    PlayerCommands[1] = PI_PreviousWeapon;
  
  
}

void DefaultConfig()
{
  
  /*PI_MovDown,PI_MovDownLeft,PI_MovLeft,PI_MovUpLeft,PI_MovUp,PI_MovUpRight,PI_MovRight,PI_MovDownRight,
  PI_FireDown,PI_FireDownLeft,PI_FireLeft,PI_FireUpLeft,PI_FireUp,PI_FireUpRight,PI_FireRight,PI_FireDownRight,
  PI_FireUltimate,PI_NextWeapon,PI_PreviousWeapon,PI_Pause*/ 
  KeyButtons[PI_MovDown] = SDLK_s;
  KeyButtons[PI_MovLeft] = SDLK_a;
  KeyButtons[PI_MovUp] = SDLK_w;
  KeyButtons[PI_MovRight] = SDLK_d;
  KeyButtons[PI_FireDown] = SDLK_KP2;
  KeyButtons[PI_FireDownLeft] = SDLK_KP1;
  KeyButtons[PI_FireLeft] = SDLK_KP4;
  KeyButtons[PI_FireUpLeft] = SDLK_KP7;
  KeyButtons[PI_FireUp] = SDLK_KP8;
  KeyButtons[PI_FireUpRight] = SDLK_KP9;
  KeyButtons[PI_FireRight] = SDLK_KP6;
  KeyButtons[PI_FireDownRight] = SDLK_KP3;
  KeyButtons[PI_NextWeapon] = SDLK_f;
  KeyButtons[PI_PreviousWeapon] = SDLK_g;
  KeyButtons[PI_Pause] = SDLK_ESCAPE;
  KeyButtons[PI_FireUltimate] = SDLK_KP5;
}

void LoadKeyConfig()
{
  FILE *file;
  file = fopen("saves/config.cfg","r");
  if(file == NULL)
  {
    fprintf(stderr,"unable to load key configuration, falling back on default\n");
    DefaultConfig();
    SaveKeyConfig();
    return;
  }
  if(fread(KeyButtons, sizeof(KeyButtons), 1, file)!= 1)
  {
    fprintf(stderr,"Unable to write config data to file: %s",SDL_GetError());
    return;
  }
  fclose(file);
}


void SaveKeyConfig()
{
  FILE *file;
  file = fopen("saves/config.cfg","w");
  if(file == NULL)
  {
    fprintf(stderr,"unable to save key configuration, error!\n");
    return;
  }
  if(fwrite(KeyButtons, sizeof(KeyButtons), 1, file)!= 1)
  {
    fprintf(stderr,"Unable to write config data to file: %s",SDL_GetError());
    return;
  }
  fclose(file);
}

void CloseHUD()
{
  if(titlebar != NULL)
  {
    SDL_FreeSurface(titlebar);
    titlebar = NULL;
  }
}

void LoadHUD()
{
  SDL_Surface *temp;
  temp = IMG_Load("images/titlebar1.png");
  if(temp == NULL)
  {
    fprintf(stderr,"Unable to load title bar: %s\n",SDL_GetError());
    exit(0);
  }
  titlebar = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  SDL_SetColorKey(titlebar, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(titlebar->format,0,0,0));
  atexit(CloseHUD);
}

void DrawHUD(Entity *self)
{
  SDL_Rect window;
  char text[60];
  window.x = 0;
  window.y = 0;
  if(titlebar == NULL)
  {
    window.w = screen->w;
    window.h = 32;
    SDL_FillRect(screen, &window, IndexColor(DarkBlue));
  }
  else
  {
    SDL_BlitSurface(titlebar,NULL,screen,NULL);
  }
  sprintf(text,"Score: %i",self->Score);
  DrawText(text,screen,5,5,IndexColor(self->Color),F_Medium);
  window.x = screen->w - 120;
  window.w = 100;
  window.y = 5;
  window.h = 16;
  SDL_FillRect(screen, &window, IndexColor(Red));
  if(self->health >= 0)
    window.w = ((float)self->health / (float)self->healthmax) * 100;
  else window.w = 0;
  SDL_FillRect(screen, &window, IndexColor(Green));
}

void FinalOutput()
{
  fprintf(stdout,"Final Score : %i\n",ThePlayer->Score);
  SDL_SaveBMP(screen,"finale.bmp");
}

/*try to open a saved game and if we fail, then */
int GetUnusedSaveIndex()
{
  FILE *file;
  int i = 0;
  char filename[50];
  sprintf(filename,"saves/SAVE%04i.vwg",i);
  file = fopen(filename,"r");
  while(file != NULL)
  {
    i++;
    fclose(file);    
    sprintf(filename,"saves/SAVE%04i.vwg",i);
    file = fopen(filename,"r");    
  }
  fclose(file);    
  return i;
}

void SavePlayer(PlayerStats *player)
{
  FILE *file;
  char filename[50];
  if(player->previoussave != -1)sprintf(filename,"saves/SAVE%04i.vwg",GetUnusedSaveIndex());
  else sprintf(filename,"saves/SAVE%04i.vwg",player->previoussave);
  file = fopen(filename,"wb");
  if(file == NULL)
  {
    fprintf(stderr,"unable to save game!\n");
    return;
  }
  if(fwrite(player, sizeof(PlayerStats), 1, file)!= 1)
  {
    fprintf(stderr,"Unable to write game data to file: %s",SDL_GetError());
    return;
  }
  fclose(file);
}

void LoadPlayer(char filename[40],PlayerStats *player)
{
  FILE *file;
  file = fopen(filename,"rb");
  if(file == NULL)
  {
    fprintf(stderr,"unable to open saved game!\n");
    return;
  }
  if(fread(player, sizeof(PlayerStats), 1, file)!= 1)
  {
    fprintf(stderr,"Unable to read game data from file: %s",SDL_GetError());
    return;
  }
  fclose(file);
}