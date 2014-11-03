#include <stdlib.h>
#include <string.h>
#include "audio.h"
#include "menu.h"
#include "player.h"
#include "spawn.h"
#include "space.h"

#define MAXTILES 9

extern Entity *ThePlayer;
extern SDL_Surface *screen;
extern SDL_Surface *bgimage;
extern SDL_Rect Camera;
extern PlayerStats ThisPlayer_Stats;
extern Level level;
extern Mix_Music *BgMusic;
extern SDL_Surface *background;
extern Spawn GameSpawns[];
extern Mouse_T Mouse;
extern Uint8 *keys;
extern int MaxSpawns;


struct
{
  Sprite *dasharrows;       /*sprite for the buttons*/
  Sprite *dashbutton;       /*sprite for the buttons*/
  Sprite *tiles;            /*the tile sheet for the map tiles*/
  Sprite *spawn;            /*this is loaded for spawn points*/
  SDL_Surface *dashboard;   /*the dashboard itself*/
  SDL_Rect dest;            /*the drawing location of the dashboard*/
  int buttonstate[4];       /*maintains which button is pressed*/
  char buttonname[4][20];   /*the test that appears on each button*/
  int MenuType;             /*which button is pressed*/
  int changed;
  int musicplaying;
  int tilebrush;
  int spawnindex;
}Dashboard;

void DrawConfigMenu(int);



/**************************************************************
 *
 *        Main Menu
 *
 **************************************************************/

void MainMenu()
{
  SDL_Surface *temp;
  SDL_Surface *temp2;
  Window *window;
  int done = 0;
  temp = IMG_Load("images/newsplash.png");
  if(temp == NULL)
  {
    fprintf(stderr,"unable to load dashboard: %s\n",SDL_GetError());
    exit(0);
  }
  temp2 = SDL_DisplayFormat(temp);
  SDL_BlitSurface(temp2,NULL,background,NULL);
  SDL_FreeSurface(temp);
  SDL_FreeSurface(temp2);
  LoadMouse();
  window = NewWindow("Main Menu",NULL,30,120, 512,512,LightGrey,Red,0,DrawWindowGeneric,NULL);
  NewWindowButton(window,NewButton(NULL,"New Game",40,140, 100, 60,SDLK_n,0,Blue,White));
  do
  {
    ResetBuffer();
    SDL_PumpEvents();
    keys = SDL_GetKeyState(NULL);

    UpdateMouse();
    UpdateAllWindows();

    DrawWindowGeneric(window);
    DrawMouse();
    if(keys[SDLK_ESCAPE] == 1)done = 1;
    NextFrame();
  }
  while(!done);
  CloseMouse();
}

/**************************************************************
 *
 *        Dashboard 
 *
 **************************************************************/

void MapDraw_ALL()
{
  DrawSpawnPoints();
  DrawMessages();
  DrawDashboard();
  DrawMouse();
}

void LoadDashboard()
{
  SDL_Surface *temp;
  Camera.h = screen->h - 104;
  temp = IMG_Load("images/dashboard.png");
  if(temp == NULL)
  {
    fprintf(stderr,"unable to load dashboard: %s\n",SDL_GetError());
    exit(0);
  }
  Dashboard.dashboard = SDL_DisplayFormat(temp);
  SDL_FreeSurface(temp);
  /*sets a transparent color for blitting.*/
  SDL_SetColorKey(Dashboard.dashboard, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(Dashboard.dashboard->format, 0,0,0));
  Dashboard.dashbutton = LoadSprite("images/dashbutton.png",128,24);
  Dashboard.dasharrows = LoadSprite("images/arrows.png",16,16);
  Dashboard.buttonstate[0] = 1;
  strcpy(Dashboard.buttonname[0],"Level Info\0");
  strcpy(Dashboard.buttonname[1],"Tile Buttons\0");
  strcpy(Dashboard.buttonname[2],"Spawn Points\0");
  strcpy(Dashboard.buttonname[3],"Control Tags\0");
  Dashboard.tiles = LoadSprite("images/tileset1.png",64,64);
  SDL_SetColorKey(Dashboard.tiles->image, SDL_SRCCOLORKEY , SDL_MapRGB(Dashboard.tiles->image->format, 255,255,255));
  atexit(CloseDashboard);
}

void CloseDashboard()
{
  if(Dashboard.dashboard != NULL)SDL_FreeSurface(Dashboard.dashboard);
  if(Dashboard.dashbutton != NULL)FreeSprite(Dashboard.dashbutton);
}

void DrawSpawnInfo()
{
  DrawFilledRect(48,Dashboard.dest.y + 48, 68, 68, 0x010101,screen);
  DrawSpawn(Dashboard.spawnindex,50,Dashboard.dest.y + 52);
  DrawSprite(Dashboard.dasharrows,screen,30,Dashboard.dest.y + 76,1);  
  DrawSprite(Dashboard.dasharrows,screen,48 + 68 + 2,Dashboard.dest.y + 76,3);  
}

void DrawTileInfo()
{
  DrawFilledRect((screen->w>>1)-34,Dashboard.dest.y + 52, 68, 68, 0x010101,screen);
  DrawSprite(Dashboard.tiles,screen,(screen->w>>1) - 32,Dashboard.dest.y + 54,Dashboard.tilebrush);
  DrawSprite(Dashboard.dasharrows,screen,(screen->w>>1) - 50,Dashboard.dest.y + 76,1);  
  DrawSprite(Dashboard.dasharrows,screen,(screen->w>>1) + 36,Dashboard.dest.y + 76,3);  
}

void DrawLevelInfo()
{
  char text[60];
  DrawFilledRect(20,Dashboard.dest.y + 52, 464, Dashboard.dashboard->h - 60, 0x010101,screen);
  sprintf(text,"Level Name:       %s",level.levelname);
  DrawText(text,screen,24,Dashboard.dest.y + 56,IndexColor(Green),F_Small);
  sprintf(text,"Background Image: %s",level.bgimage);
  DrawText(text,screen,24,Dashboard.dest.y + 76,IndexColor(Green),F_Small);
  sprintf(text,"Background Music: %s",level.bgmusic);
  DrawText(text,screen,24,Dashboard.dest.y + 96,IndexColor(Green),F_Small);
  DrawFilledRect((screen->w>>1) + 32,Dashboard.dest.y + 52, 464, Dashboard.dashboard->h - 60, 0x010101,screen);
  sprintf(text,"Level Width: %i",level.width);
  DrawText(text,screen,(screen->w>>1) + 36,Dashboard.dest.y + 56,IndexColor(Green),F_Small);  
  sprintf(text,"Level Height: %i",level.height);
  DrawText(text,screen,(screen->w>>1) + 36,Dashboard.dest.y + 76,IndexColor(Green),F_Small);  
  if(!Dashboard.musicplaying)DrawSprite(Dashboard.dasharrows,screen,(screen->w>>1) - 8,Dashboard.dest.y + 96,3);  
  else DrawSprite(Dashboard.dasharrows,screen,(screen->w>>1) - 8,Dashboard.dest.y + 96,4);
}

void DrawDashboard()
{
  int i;
/*  int w;
  int h;
  w = GameSpawns[Dashboard.spawnindex].mapsprite->w;
  h = GameSpawns[Dashboard.spawnindex].mapsprite->h;*/
  if(Mouse.my < (screen->h - Dashboard.dashboard->h + 24))
  {
    switch(Dashboard.MenuType)
    {
      case 1:
        DrawRect((((Mouse.mx + Camera.x)>>6)<<6) - Camera.x,(((Mouse.my + Camera.y)>>6)<<6) - Camera.y, 64, 64, IndexColor(LightBlue), screen);
        break;
      case 2:
        DrawSpawn(Dashboard.spawnindex,Mouse.mx,Mouse.my);
        break;
    }
  }
  Dashboard.dest.x = 0;
  Dashboard.dest.y = screen->h - Dashboard.dashboard->h;
  if(Dashboard.dashboard != NULL)SDL_BlitSurface(Dashboard.dashboard,NULL,screen,&Dashboard.dest);
  if(Dashboard.dashbutton != NULL)
  {
    for(i = 0;i < 4;i++)
    {
      DrawSprite(Dashboard.dashbutton,screen,(i * 160) + 128 + 64 + 16,Dashboard.dest.y + 26,Dashboard.buttonstate[i]);
      DrawTextCentered(Dashboard.buttonname[i],screen,(i * 160) + 256 + 16,Dashboard.dest.y + 27 + (Dashboard.buttonstate[i] * 2),0x030303,F_Small);
    }
  }
  switch(Dashboard.MenuType)
  {
    case 0:
      DrawLevelInfo();
      break;
    case 1:
      DrawTileInfo();
      break;
    case 2:
      DrawSpawnInfo();
      break;
  }
}

void UpdateDashboard()/*input handling for dashboard*/
{
  Uint8 *keys;
  SDLMod mod;
  int i;
  SDL_Surface *bg = NULL;
  char tempstring[40];
  int tempw,temph;
/*  int w;
  int h;
  w = GameSpawns[Dashboard.spawnindex].mapsprite->w;
  h = GameSpawns[Dashboard.spawnindex].mapsprite->h;*/
  keys = SDL_GetKeyState(NULL);    
  mod = SDL_GetModState();
  tempstring[0] = '\0';
  if((Mouse.buttons & SDL_BUTTON(1))&&(Mouse.oldbuttons == 0))
  {/*mouse was pressed, lets find out where*/
    switch(Dashboard.MenuType)
    {
      case 0:/*level info*/
        if(MouseIn(24,Dashboard.dest.y + 56,128, 16))/*enter new name for the map*/
        {
          if(GetString(MapDraw_ALL,"Enter New Level Name",tempstring,40))
            strcpy(level.levelname,tempstring);
        }  
        else if(MouseIn((screen->w>>1) - 8,Dashboard.dest.y + 96,16, 16))
        {     /*stop/play music*/
          if(Dashboard.musicplaying)
          {
            Mix_HaltMusic();
            Dashboard.musicplaying = 0;
          }
          else
          {
            Mix_PlayMusic(BgMusic,-1);
            Dashboard.musicplaying = 1;            
          }
        }
        else if(MouseIn(24,Dashboard.dest.y + 76,128, 16))
        {       /*get the name of the new background image*/
          if(GetString(MapDraw_ALL,"Enter Path and Name of image",tempstring,40))
          {
            if(TryAndOpen(tempstring))
            {
              strcpy(level.bgimage,tempstring);
              bg = IMG_Load(tempstring);
              if(bg != NULL)
              {
                SDL_BlitSurface(bg,NULL,bgimage,NULL);
                SDL_FreeSurface(bg);
              }
              Dashboard.changed = 1;
            }
            else NewMessage("Unable to open background image.",IndexColor(LightRed));
          }          
        }
        else if(MouseIn(24,Dashboard.dest.y + 96,128, 16))
        {   /*Get the name of the new background music file*/
          if(GetString(MapDraw_ALL,"Enter Path and Name of Music file",tempstring,40))
          {
            if(TryAndOpen(tempstring))
            {
              strcpy(level.bgmusic,tempstring);
              Mix_FreeMusic(BgMusic);
              BgMusic = Mix_LoadMUS(tempstring);
              Dashboard.musicplaying = 1;
              Mix_PlayMusic(BgMusic,-1);
              Dashboard.musicplaying = 1;
              Dashboard.changed = 1;
            }
            else NewMessage("Unable to open background music.",IndexColor(LightRed));
          }          
        }
        else if(MouseIn((screen->w>>1) + 36,Dashboard.dest.y + 56,128, 32))
        {       /*get the new size of the map / Resets map*/
          NewMessage("Warning : Operation Resets All Other Data!",IndexColor(Magenta));
          if(GetNumber(MapDraw_ALL,"Enter New Map Width in Tiles.",&tempw,40))
          {
            if((tempw >= 16)&&(tempw <= SPACE_W))
            {
              if(GetNumber(MapDraw_ALL,"Enter New Map Height in Tiles.",&temph,40))
              {
                if((temph >= 16)&&(temph <= SPACE_W))
                {
                  ClearRegionMask();   /*de-allocate all allocated memory*/
                  GenerateLevel(tempw,temph);
                  InitRegionMask(tempw,temph); /*based on size of map*/
                  DrawLevel();
                  Dashboard.musicplaying = 1;
                }
                else NewMessage("Invalid Size.",IndexColor(LightRed));
              }         
            }
            else NewMessage("Invalid Size.",IndexColor(LightRed));
          } 
        }
        break;
        case 1:/*tile drawing*/
        if(MouseIn((screen->w>>1) - 50,Dashboard.dest.y + 76,16,16))
        {
          Dashboard.tilebrush--;
          if(Dashboard.tilebrush < 0)Dashboard.tilebrush = MAXTILES - 1;
        }
        else if(MouseIn((screen->w>>1) + 36,Dashboard.dest.y + 76,16,16))
        {
          Dashboard.tilebrush = (Dashboard.tilebrush + 1) % MAXTILES;
        }
        if(MouseIn(0,0,screen->w,screen->h - Dashboard.dashboard->h + 24))
        {       /*put a tile in the game.*/
          DrawFilledRect((((Mouse.mx + Camera.x)>>6)<<6),(((Mouse.my + Camera.y)>>6)<<6), 64, 64, SDL_MapRGB(background->format,0,0,0),background);
          DrawSprite(Dashboard.tiles,background,(((Mouse.mx + Camera.x)>>6)<<6),(((Mouse.my + Camera.y)>>6)<<6),Dashboard.tilebrush);
          level.tilemap[(Mouse.my + Camera.y)>>6][(Mouse.mx + Camera.x)>>6] = Dashboard.tilebrush + 1;
        }
        break;        
        case 2:/*spawn points*/
          if(MouseIn(30,Dashboard.dest.y + 76,16,16))
          {
            Dashboard.spawnindex--;
            if(Dashboard.spawnindex < 0)Dashboard.spawnindex = MaxSpawns - 1;
          }
          else if(MouseIn(48 + 68 + 2,Dashboard.dest.y + 76,16,16))
          {
            Dashboard.spawnindex = (Dashboard.spawnindex + 1) % MaxSpawns;
          }
          if(MouseIn(0,0,screen->w,screen->h - Dashboard.dashboard->h + 24))
          {
            if(level.spawncount + 1 < MAX_ENT)
            {
              strncpy(level.spawnlist[level.spawncount].name,GameSpawns[Dashboard.spawnindex].EntName,40);
              level.spawnlist[level.spawncount].sx = Mouse.mx + Camera.x;
              level.spawnlist[level.spawncount].sy = Mouse.my + Camera.y;
              level.spawnlist[level.spawncount].UnitInfo = 1;
              level.spawnlist[level.spawncount].UnitType = ET_WorldEnt;
              level.spawncount++;
            }
            else NewMessage("Too many spawn points for this map.",IndexColor(LightRed));
          }
        break;
    }
    for(i = 0;i < 4;i++)
    {
      if(MouseIn((i * 160) + 128 + 64 + 16,Dashboard.dest.y + 26,128, 24))
      {
        Dashboard.MenuType = i;
        memset(Dashboard.buttonstate,0,sizeof(Dashboard.buttonstate));
        Dashboard.buttonstate[Dashboard.MenuType] = 1;
        break;/*mouse can only be in one area at once*/
      }
    }
  }
  if((Mouse.buttons & SDL_BUTTON(3))&&(Mouse.oldbuttons == 0))
  {   /*RMB press*/
    switch(Dashboard.MenuType)
    {
      case 1:
        if(MouseIn(0,0,screen->w,screen->h - Dashboard.dashboard->h + 24))
        {       /*put a tile in the game.*/
          DrawFilledRect((((Mouse.mx + Camera.x)>>6)<<6),(((Mouse.my + Camera.y)>>6)<<6), 64, 64, SDL_MapRGB(background->format,0,0,0),background);
          level.tilemap[(Mouse.my + Camera.y)>>6][(Mouse.mx + Camera.x)>>6] = 0;
        }
        break;
    }
  }
}




/**************************************************************
 *
 *        Player Config Menu functions
 *
 **************************************************************/


/*Customize the player's preferences*/
void PlayerConfig(Entity *self)
{
  int done = 0;
  int menuoption = 0; /*place in menu where we are currently editing*/
  int resetimage = 0;
  Uint8 *keys;
  SDLMod mod;
  do
  {
    ResetBuffer();
    SDL_PumpEvents();
    keys = SDL_GetKeyState(NULL);    
    mod = SDL_GetModState();
    DrawEntities();
    DrawHUD(ThePlayer);
    DrawConfigMenu(menuoption);
    DrawMessages();
    if(keys[SDLK_RETURN] == 1)done = 1;
    if(keys[SDLK_ESCAPE] == 1)done = 1;
    if(keys[SDLK_RIGHT] == 1)
    {
      ThisPlayer_Stats.color[menuoption]++;
      if(ThisPlayer_Stats.color[menuoption] > 32)ThisPlayer_Stats.color[menuoption]= 1;
      if(ThisPlayer_Stats.color[menuoption] == Black)ThisPlayer_Stats.color[menuoption]++;
      resetimage = 1;
    }
    if(keys[SDLK_LEFT] == 1)
    {
      ThisPlayer_Stats.color[menuoption]++;
      if(ThisPlayer_Stats.color[menuoption] < 1)ThisPlayer_Stats.color[menuoption]= 32;
      if(ThisPlayer_Stats.color[menuoption] == Black)ThisPlayer_Stats.color[menuoption]--;
      resetimage = 1;
    }
    if(keys[SDLK_DOWN] == 1)
    {
      menuoption++;
      if(menuoption > PC_Trail)menuoption = PC_Core1;
    }
    if(keys[SDLK_UP] == 1)
    {
      menuoption--;
      if(menuoption < PC_Core1)menuoption = PC_Trail;
    }
    if(resetimage)
    {
      resetimage = 0;
      switch(menuoption)
      {
        case PC_Core1:
        case PC_Core2:
        case PC_Gun:
          FreeSprite(self->sprite);
          self->sprite = LoadSwappedSprite("images/core1.png",48,48,ThisPlayer_Stats.color[PC_Core1],ThisPlayer_Stats.color[PC_Core2],ThisPlayer_Stats.color[PC_Gun]);
          break;
        case PC_Leg1:
        case PC_Leg2:
        case PC_Leg3:
          FreeSprite(self->legs);
          self->legs = LoadSwappedSprite("images/legs1.png",48,48,ThisPlayer_Stats.color[PC_Leg1],ThisPlayer_Stats.color[PC_Leg2],ThisPlayer_Stats.color[PC_Leg3]);
          break;
        case PC_Trail:
          self->Color = ThisPlayer_Stats.color[PC_Trail];
          break;
      }
    }
    NextFrame();
    FrameDelay(63);
  }while(!done);

}

void DrawConfigMenu(int menuoption)
{
  char text[40];
  int top,left;
  left = (screen->w>>1) - 150;
  top = (screen->h>>1) - 150;
  DrawFilledRect(left,top, 300, 300, IndexColor(Grey), screen);
  DrawRect(left,top, 300, 300, IndexColor(Silver), screen);
  sprintf(text,"Mech Paint Customizer");
  DrawText(text,screen,(left + 150) - (strlen(text)* 4),top + 20,IndexColor(DarkGrey),F_Medium);
  {
  sprintf(text,"Primary Core Color:");
  DrawText(text,screen,left + 40,top + 50,IndexColor(DarkGrey),F_Medium);
  DrawFilledRect(left + 210,top + 50, 20, 20, IndexColor(ThisPlayer_Stats.color[PC_Core1]), screen);  
  DrawRect(left + 210,top + 50, 20, 20, IndexColor(DarkGrey), screen);  
  }
  {
    sprintf(text,"Secondary Core Color:");
    DrawText(text,screen,left + 40,top + 80,IndexColor(DarkGrey),F_Medium);
    DrawFilledRect(left + 210,top + 80, 20, 20, IndexColor(ThisPlayer_Stats.color[PC_Core2]), screen);  
    DrawRect(left + 210,top + 80, 20, 20, IndexColor(DarkGrey), screen);  
  }
  {
    sprintf(text,"Weapon Color:");
    DrawText(text,screen,left + 40,top + 110,IndexColor(DarkGrey),F_Medium);
    DrawFilledRect(left + 210,top + 110, 20, 20, IndexColor(ThisPlayer_Stats.color[PC_Gun]), screen);  
    DrawRect(left + 210,top + 110, 20, 20, IndexColor(DarkGrey), screen);  
  }
  {
    sprintf(text,"Primary Leg Color:");
    DrawText(text,screen,left + 40,top + 140,IndexColor(DarkGrey),F_Medium);
    DrawFilledRect(left + 210,top + 140, 20, 20, IndexColor(ThisPlayer_Stats.color[PC_Leg1]), screen);  
    DrawRect(left + 210,top + 140, 20, 20, IndexColor(DarkGrey), screen);  
  }
  {
    sprintf(text,"Secondary Leg Color:");
    DrawText(text,screen,left + 40,top + 170,IndexColor(DarkGrey),F_Medium);
    DrawFilledRect(left + 210,top + 170, 20, 20, IndexColor(ThisPlayer_Stats.color[PC_Leg2]), screen);  
    DrawRect(left + 210,top + 170, 20, 20, IndexColor(DarkGrey), screen);  
  }
  {
    sprintf(text,"Leg Detail Color:");
    DrawText(text,screen,left + 40,top + 200,IndexColor(DarkGrey),F_Medium);
    DrawFilledRect(left + 210,top + 200, 20, 20, IndexColor(ThisPlayer_Stats.color[PC_Leg3]), screen);  
    DrawRect(left + 210,top + 200, 20, 20, IndexColor(DarkGrey), screen);  
  }
  {
    sprintf(text,"Weapon Trail Color:");
    DrawText(text,screen,left + 40,top + 230,IndexColor(DarkGrey),F_Medium);
    DrawFilledRect(left + 210,top + 230, 20, 20, IndexColor(ThisPlayer_Stats.color[PC_Trail]), screen);  
    DrawRect(left + 210,top + 230, 20, 20, IndexColor(DarkGrey), screen);  
  }
  if(ThePlayer->sprite != NULL)
    DrawSprite(ThePlayer->sprite,screen,left + 245,top + 70,18);
  if(ThePlayer->legs != NULL)
    DrawSprite(ThePlayer->legs,screen,left + 240,top + 130,0);
  DrawThickLine(left + 240,top + 190,left + 298, top + 270,3,IndexColor(ThisPlayer_Stats.color[PC_Trail]),screen);
  DrawThickLine(left + 230,top + 298,left + 298, top + 270,3,IndexColor(ThisPlayer_Stats.color[PC_Trail]),screen);
  DrawRect(left + 210,top + 50 + (menuoption * 30), 20, 20, IndexColor(Yellow), screen);  
}