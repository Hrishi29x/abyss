#include <stdlib.h>
#include <string.h>
#include "space.h"
#include "entity.h"
#include "audio.h"

extern ScreenData  S_Data;
extern Uint32 rmask,gmask,bmask,amask;
extern SDL_Rect Camera;
extern SDL_Surface *bgimage;
extern SDL_Surface *background;
SDL_Surface *clipmask;
Level level;
Mix_Music *BgMusic = NULL;


void CloseLevel()
{
  if(BgMusic != NULL)
  {
    Mix_FreeMusic(BgMusic);
  }
  if(bgimage != NULL)
  {
    SDL_FreeSurface(bgimage);
  }
  if(clipmask != NULL)
  {
    SDL_FreeSurface(clipmask);
  }
}


void GenerateLevel(int x, int y)
{
  int i,j;
  if(x > SPACE_W)x = SPACE_W;
  if(y > SPACE_H)y = SPACE_H;
  level.width = x;
  level.height = y;
  level.tileset = 1;
  level.infocount = 0;
  level.spawncount = 0;
  strcpy(level.levelname,"Test Level");
  strcpy(level.bgimage,"images/bgimage1.png");
  strcpy(level.bgmusic,"sounds/bgmusic1.ogg");
  memset(level.tilemap,0,sizeof(level.tilemap));
  for(j = 0;j < y;j++)
  {
    level.tilemap[j][0] = 1;
    level.tilemap[j][x - 1] = 1;    
  }
  for(i = 0;i < x;i++)
  {
    level.tilemap[0][i] = 1;
    level.tilemap[y - 1][i] = 1;    
  }
}

/*load the graphics and draw the graphics to the background buffer*/
void DrawLevel()
{
  Sprite *tiles = NULL;
  SDL_Surface *temp;
  int i,j;
  /*first we load the sprite sheet for the background tiles*/
  switch(level.tileset)
  {
    case 1:
    default:
      tiles = LoadSprite("images/tileset1.png",64,64);
      SDL_SetColorKey(tiles->image, SDL_SRCCOLORKEY , SDL_MapRGB(tiles->image->format, 255,255,255));
  }
  if(tiles == NULL)
  {
    fprintf(stderr,"Unable to load tile set for map: %s \n",SDL_GetError());
    exit(0);
  }
  if(BgMusic != NULL)
  {
    Mix_HaltMusic();
    Mix_FreeMusic(BgMusic);
  }
  /*then we load the background music*/
  BgMusic = Mix_LoadMUS(level.bgmusic);
  if(BgMusic == NULL)
  {
    fprintf(stderr,"Unable to load music: %s \n",SDL_GetError());
    exit(0);
  }
  
  /*And we load the background image*/
  if(bgimage != NULL)SDL_FreeSurface(bgimage);
  temp = IMG_Load(level.bgimage);
  if(temp == NULL)
  {
    fprintf(stderr,"Unable to load background image: %s \n",SDL_GetError());
    exit(0);
  }
  bgimage = SDL_DisplayFormat(temp);
  SDL_SetColorKey(bgimage, SDL_RLEACCEL, 0);
  SDL_FreeSurface(temp);
  
  /*then we check to see if the background image is already in memory and if so, then we shall destroy it*/
  if(background != NULL)SDL_FreeSurface(background);
  temp = SDL_CreateRGBSurface(SDL_HWSURFACE, level.width * 64, level.height * 64, S_Data.depth,rmask, gmask,bmask,amask);
  if(temp == NULL)
  {
    fprintf(stderr,"Couldn't initialize background buffer: %s\n", SDL_GetError());
    exit(1);
  }
  /* Just to make sure that the surface we create is compatible with the screen*/
  background = SDL_DisplayFormatAlpha(temp);
  SDL_FreeSurface(temp);
  
  if(clipmask != NULL)SDL_FreeSurface(clipmask);
  temp = SDL_CreateRGBSurface(SDL_HWSURFACE, level.width * 64, level.height * 64, S_Data.depth,rmask, gmask,bmask,amask);
  if(temp == NULL)
  {
    fprintf(stderr,"Couldn't initialize background buffer: %s\n", SDL_GetError());
    exit(1);
  }
  /* Just to make sure that the surface we create is compatible with the screen*/
  clipmask = SDL_DisplayFormatAlpha(temp);
  SDL_FreeSurface(temp);
  DrawFilledRect(0,0, background->w,background->h, SDL_MapRGB(background->format,0,0,0), background);
  for(j = 0;j < level.height;j++)
  {
    for(i = 0;i < level.width;i++)
    {
      DrawSprite(tiles,background,i * tiles->w,j * tiles->h,level.tilemap[j][i] - 1);
    }
  }
  SDL_BlitSurface(background,NULL,clipmask,NULL);
  SDL_SetColorKey(background,SDL_RLEACCEL, SDL_MapRGB(background->format,0,0,0));
  FreeSprite(tiles);
  Mix_VolumeMusic(MIX_MAX_VOLUME>>3);
  Mix_PlayMusic(BgMusic, -1);
}

void SaveLevel(char *filename)
{
  FILE *file;
  file = fopen(filename, "wb");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open file for writing: %s",SDL_GetError());
    exit(0);
  }
  if(fwrite(&level, sizeof(level), 1, file)!= 1)
  {
    fprintf(stderr,"Unable to write to file: %s",SDL_GetError());
    exit(0);
  }
  fclose(file);
}

void LoadLevel(char *filename)
{
  FILE *file;
  file = fopen(filename, "rb");
  if(file == NULL)
  {
    fprintf(stderr,"Unable to open file for reading: %s",SDL_GetError());
    exit(0);
  }
  if(fread(&level, sizeof(level), 1, file)!= 1)
  {
    fprintf(stderr,"Unable to read from file: %s",SDL_GetError());
    exit(0);    
  }
  fclose(file);
}