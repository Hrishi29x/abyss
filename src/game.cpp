#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "SDL.h"
#include "graphics.h"
#include "audio.h"
#include "entity.h"
#include "spawn.h"
#include "player.h"
#include "menu.h"
#include "space.h"
#include "weapon.h"
#include "projectiles.h"

#define MAXSTATE 1

extern SDL_Surface *screen;
extern SDL_Surface *bgimage;
extern SDL_Surface *background;
extern SDL_Rect Camera;
extern Entity *ThePlayer;
extern Level level;
extern SDL_Surface *clipmask;
Sound *gamesounds[4];
Uint8 *keys;
int windowed = 1;
int drawents = 1;


void Init_All();
void Update_ALL();
int Think_ALL();
void Draw_ALL();
void DrawSplashScreen();
void GiveInfo();
void PopUpWindow(Sprite *sprite,char *text,Uint32 Color,int pwait);
void UpdateCamera();
void UpdateMapCamera();

int main(int argc, char *argv[])
{
  FILE *file = NULL;
  int done = 0;
  char string[40];
  SDLMod mod;
  string[0] = '\0';
  Init_All();
  GiveInfo();
  SpawnAll(1);
  if(ThePlayer == NULL) SpawnPlayer(128,128);
  do
  {
    ResetBuffer();
    SDL_PumpEvents();
    keys = SDL_GetKeyState(NULL);    
    mod = SDL_GetModState();
    Draw_ALL();
    if(keys[SDLK_ESCAPE] == 1) done = 1;
    Update_ALL();     
    Think_ALL();   
    NextFrame();
  }while(!done);
    
  exit(0);
  return 0;
}

void CleanUpAll()
{
  CloseSprites();
  ClearEntities();
  ClearRegionMask();
  ClearSoundList();
  SDL_FreeSurface(clipmask);
  /*any other cleanup functions can be added here*/ 
}

void Init_All()
{
  Init_Graphics(windowed);
  InitSpriteList();
  Init_Audio();
  InitSoundList();
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);
  atexit(CleanUpAll);
  LoadFonts();
  InitMessages();
  InitEntityList();
  //ResetAllParticles();
  LoadHUD();
  MainMenu();
  LoadLevel("maps/newmap.amp");
  InitRegionMask(level.width,level.height); /*based on size of map*/
  LoadKeyConfig();
  DrawSplashScreen();
  DrawLevel();
  PrecacheProjSounds();
  PrecacheWeaponSounds();
}

void GiveInfo()
{
  NewMessage("Press Esc to Quit",IndexColor(White));
}

void PopUpWindow(Sprite *splash,char *text,Uint32 Color,int frame)
{
  Sprite *popup;
  int len = strlen(text);
  popup = LoadSprite("images/popup.png",400,300);
  DrawSprite(popup,screen,(screen->w - 400)/2,(screen->h - 300)/2,0);
  if(splash != NULL)
    DrawSprite(splash,screen,(screen->w - splash->w)/2,(screen->h - splash->h)/2,frame);
  if(text != NULL)
    DrawText(text,screen,(screen->w - (len *11))/2 ,(screen->h - 300)/2 + 50,Color,F_Large);
  DrawText("Hit Spacebar to continue",screen,(screen->w)/2 - 136 ,(screen->h/2) + 100,Color,F_Large);
  NextFrame();
  FreeSprite(splash);
  FreeSprite(popup);
  do
  {
    SDL_PumpEvents();
    keys = SDL_GetKeyState(NULL);
  }
  while(keys[SDLK_SPACE] != 1);
  
}

void DrawSplashScreen()
{
  SDL_Surface *splash;
  splash = IMG_Load("images/splash1.png");
  if(splash != NULL)
  {
    SDL_BlitSurface(splash,NULL,screen,NULL);
    NextFrame();
  }
  SDL_FreeSurface(splash);
  do
  {
    SDL_PumpEvents();
    keys = SDL_GetKeyState(NULL);
  }
  while((SDL_GetTicks() < 2000)&&(keys[SDLK_SPACE] != 1));
}



/*calls all of the update functions for everything*/
void Update_ALL()
{
  UpdateMouse();
  UpdateEntities();
  UpdateCamera();
}

/*calls all of the think function for everything*/
int Think_ALL()
{
  int done = 0;
  ThinkEntities();
  return done;
}

/*calls all of the draw functions for everything*/
void Draw_ALL()
{
  //DrawAllParticles();
  if(drawents)
  {
    DrawEntities();
    if(ThePlayer != NULL) DrawEntity(ThePlayer);
  }
  DrawMessages();
  DrawHUD(ThePlayer);
  DrawMouse();
}

void UpdateMapCamera()
{
  int mx,my;
  SDL_GetMouseState(&mx,&my);
  if((mx < 10)&&(Camera.x >= 10))  Camera.x -= 10;
  if((mx > screen->w - 10)&&(Camera.x <= (background->w - screen->w) - 10))Camera.x += 10;
  if((my < 10)&&(Camera.y >= 10))  Camera.y -= 10;
  if((my > screen->h - 10)&&(Camera.y <= (background->h - (screen->h - 100)) - 10))Camera.y += 10;
}

void UpdateCamera()
{
  Camera.x = (int)ThePlayer->s.x - (Camera.w >> 1);
  Camera.y = (int)ThePlayer->s.y - (Camera.h >> 1);
  if(Camera.x > background->w - Camera.w)Camera.x = background->w - Camera.w;
  if(Camera.x < 0)Camera.x = 0;
  if(Camera.y > background->h - Camera.h)Camera.y = background->h - Camera.h;
  if(Camera.y < 0)Camera.y = 0;
  
}