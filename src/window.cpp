#include <stdlib.h>
#include <string.h>
#include "window.h"


#define MAXTILES 30
#define MAXWINDOWS 32
#define MAXBUTTONS MAXWINDOWS*WINBUTTONS


extern SDL_Surface *screen;
extern SDL_Surface *bgimage;
extern SDL_Rect Camera;

extern SDL_Surface *background;

Mouse_T Mouse;

Button ButtonList[MAXBUTTONS];
Window WindowList[MAXWINDOWS];

/**************************************************************
 *
 *        Window System
 *
 **************************************************************/

void InitWindowList()
{
  int i;
  for (i = 0;i < MAXWINDOWS;i++)
  {
    WindowList[i].inuse = 0;
    WindowList[i].sprite = NULL;
    WindowList[i].title[0] = '\0';
  }
}

void FreeWindow(Window *window)
{
  window->inuse = 0;
  if(window->sprite != NULL)FreeSprite(window->sprite);
  window->sprite = NULL;
  window->title[0] = '\0';
}

void FreeWindowList()
{
  int i;
  for (i = 0;i < MAXWINDOWS;i++)
  {
    FreeWindow(&WindowList[i]);
  }
}

Window *NewWindow(char *title,char *spritename,int x,int y, int w,int h,int c1,int c2,int layer,void (*drawwindow)(Window *self),void (*updatewindow)(Window *self))
{
  int i;
  for(i = 0;i < MAXWINDOWS;i++)
  {
    if(!WindowList[i].inuse)
    {
      WindowList[i].inuse = 1;
      if(spritename != NULL)
      {
        WindowList[i].sprite = LoadSprite(spritename,w,h);
        if(WindowList[i].sprite != NULL)WindowList[i].usesprite = 1;
        else WindowList[i].usesprite = 0;
      }
      else WindowList[i].usesprite = 0;
      if(title != NULL)
      {
        strcpy(WindowList[i].title,title);
      }
      WindowList[i].rect.x = x;
      WindowList[i].rect.y = y;
      WindowList[i].rect.w = w;
      WindowList[i].rect.h = h;
      WindowList[i].layer = layer;
      WindowList[i].fg = c1;
      WindowList[i].bg = c2;
      WindowList[i].drawwindow = drawwindow;
      WindowList[i].updatewindow = updatewindow;
      return &WindowList[i];
    }
  }
  return NULL;
}

void DrawWindowGeneric(Window *self)
{
  int i;
  if(self->sprite != NULL)
  {
    DrawSprite(self->sprite,screen,self->rect.x,self->rect.y,0);
  }
  else
  {
    DrawFilledRect(self->rect.x,self->rect.y, self->rect.w, self->rect.h, IndexColor(self->fg), screen);
    DrawFilledRect(self->rect.x,self->rect.y, 1, self->rect.h, IndexColor(self->fg), screen);
    DrawFilledRect(self->rect.x,self->rect.y, self->rect.w, 1, IndexColor(self->fg), screen);
    DrawFilledRect(self->rect.x + self->rect.w,self->rect.y, 1, self->rect.h, IndexColor(self->fg), screen);
    DrawFilledRect(self->rect.x,self->rect.y + self->rect.h, self->rect.w, 1, IndexColor(self->fg), screen);
  }
  DrawTextCentered(self->title,screen,self->rect.x + (self->rect.w /2),self->rect.y + 2,IndexColor(self->bg),F_Small);
  for(i = 0; i < self->buttoncount;i++)
  {
    DrawButton(self->buttonlist[i]);
  }
}

void UpdateAllWindows()
{
  int i;
  for(i = 0;i < MAXWINDOWS;i++)
  {
    if(WindowList[i].inuse)
    {
      if(WindowList[i].updatewindow != NULL)WindowList[i].updatewindow(&WindowList[i]);
    }
  } 
}


void NewWindowButton(Window *window, Button *button)
{
  if(window->buttoncount >= WINBUTTONS)return;/*no more buttons available*/
  window->buttonlist[window->buttoncount++] = button;
}

/**************************************************************
 *
 *        Button System
 *
 **************************************************************/

void InitButtonList()
{
  int i;
  for (i = 0;i < MAXBUTTONS;i++)
  {
    ButtonList[i].inuse = 0;
    ButtonList[i].sprite = NULL;
    ButtonList[i].text[0] = '\0';
  }
}

void FreeButton(Button *button)
{
  button->inuse = 0;
  if(button->sprite != NULL)FreeSprite(button->sprite);
  button->sprite = NULL;
  button->text[0] = '\0';
}

void FreeButtonList()
{
  int i;
  for (i = 0;i < MAXBUTTONS;i++)
  {
    FreeButton(&ButtonList[i]);
  }
}

int GetButtonState(Button *button)
{
  return button->State;
}

Button *NewButton(char *spritename,char *text,int x, int y, int w, int h,Uint8 hotkey,int layer,int c1,int c2)
{
  int i;
  for(i = 0;i < MAXBUTTONS;i++)
  {
    if(!ButtonList[i].inuse)
    {
      ButtonList[i].inuse = 1;
      if(spritename != NULL)
      {
        ButtonList[i].sprite = LoadSprite(spritename,w,h);
        if(ButtonList[i].sprite != NULL)ButtonList[i].usesprite = 1;
        else ButtonList[i].usesprite = 0;
      }
      else ButtonList[i].usesprite = 0;
      if(text != NULL)
      {
        strcpy(ButtonList[i].text,text);
      }
      ButtonList[i].rect.x = x;
      ButtonList[i].rect.y = y;
      ButtonList[i].rect.w = w;
      ButtonList[i].rect.h = h;
      ButtonList[i].Hotkey = hotkey;
      ButtonList[i].layer = layer;
      ButtonList[i].fg = c1;
      ButtonList[i].bg = c2;
      return &ButtonList[i];
    }
  }
  return NULL;
}

void DrawButton(Button *button)
{
  if(button->usesprite)
  {
    if(!button->State)DrawSprite(button->sprite,screen,button->rect.x,button->rect.y,0);
    else DrawSprite(button->sprite,screen,button->rect.x,button->rect.y,1);
  }
  else
  {
    DrawFilledRect(button->rect.x,button->rect.y, button->rect.w, button->rect.h, IndexColor(button->bg), screen);
    if(!button->State)
    {
      DrawFilledRect(button->rect.x,button->rect.y, 1, button->rect.h, IndexColor(Black), screen);
      DrawFilledRect(button->rect.x,button->rect.y, button->rect.w, 1, IndexColor(White), screen);
      DrawFilledRect(button->rect.x + button->rect.w,button->rect.y, 1, button->rect.h, IndexColor(White), screen);
      DrawFilledRect(button->rect.x,button->rect.y + button->rect.h, button->rect.w, 1, IndexColor(Black), screen);
    }
    else
    {
      DrawFilledRect(button->rect.x,button->rect.y, 1, button->rect.h, IndexColor(White), screen);
      DrawFilledRect(button->rect.x,button->rect.y, button->rect.w, 1, IndexColor(Black), screen);
      DrawFilledRect(button->rect.x + button->rect.w,button->rect.y, 1, button->rect.h, IndexColor(Black), screen);
      DrawFilledRect(button->rect.x,button->rect.y + button->rect.h, button->rect.w, 1, IndexColor(White), screen);
    }
  }
  if(button->text != NULL)
  {
    DrawTextCentered(button->text,screen,button->rect.x + (button->rect.w/2),button->rect.y + 2,IndexColor(button->fg),F_Small);
  }
}

void UpdateButton(Button *button)
{
  Uint8 *keys;
  keys = SDL_GetKeyState(NULL);
  if((MouseIn(button->rect.x,button->rect.y,button->rect.w,button->rect.h))&&(Mouse.buttons))
  {
    button->State++;
  }else if(keys[button->Hotkey])
  {
    button->State++;
  }
  else button->State = 0;
}

void UpdateButtonsByLayer(int layer)
{
  int i;
  for(i = 0; i < MAXBUTTONS;i++)
  {
    if((ButtonList[i].inuse)&&(ButtonList[i].layer == layer))UpdateButton(&ButtonList[i]);
  }
}

void UpdateAllButtons()
{
  int i;
  for(i = 0; i < MAXBUTTONS;i++)
  {
    if(ButtonList[i].inuse)UpdateButton(&ButtonList[i]);
  }
}


/**************************************************************
 *
 *        Mouse
 *
 **************************************************************/
 
int  MouseIn(int x,int y,int w, int h)
{
  if((Mouse.mx >= x)&&(Mouse.mx <= x + w)&&(Mouse.my >= y)&&(Mouse.my <= y + h))return 1;
  return 0;
}

void LoadMouse()
{
  Mouse.sprite = LoadSwappedSprite("images/cursor.png",24,24,LightGreen,0,0);
  Mouse.mframe = 0;
  Mouse.mdelay = 8;
  Mouse.mrate = 4;
}

void CloseMouse()
{
  FreeSprite(Mouse.sprite);
}

void DrawMouse()
{
  DrawSprite(Mouse.sprite,screen,Mouse.mx,Mouse.my,Mouse.mframe);
}

void UpdateMouse()
{
  Mouse.oldbuttons = Mouse.buttons;
  Mouse.buttons = SDL_GetMouseState(&Mouse.mx,&Mouse.my);
  Mouse.mdelay--;
  if(Mouse.mdelay <= 0)
  {
    Mouse.mdelay = Mouse.mrate;
    Mouse.mframe++;
    if(Mouse.mframe >= 8)Mouse.mframe = 0;
  }
}


/**************************************************************
 *
 *        String retrieval menu
 *
 **************************************************************/

void DrawStringBox(char *title,char *text,int slength)
{
  SDL_Rect rect;
  rect.x = (screen->w>>1) - (slength * 4) - 20;
  rect.y = (screen->h>>1) - 30;
  rect.w = (slength * 8) + 40;
  rect.h = 60;
  DrawFilledRect(rect.x,rect.y,rect.w , rect.h, IndexColor(Silver), screen);
  DrawRect(rect.x,rect.y,rect.w , rect.h, IndexColor(DarkBlue), screen);
  DrawTextCentered(title,screen,screen->w>>1,rect.y + 8,IndexColor(DarkGrey),F_Medium);
  DrawFilledRect(rect.x + 10,rect.y + 30,rect.w -20, 20, IndexColor(White), screen);
  DrawRect(rect.x + 10,rect.y + 30,rect.w -20, 20, IndexColor(DarkBlue), screen);
  if(strlen(text) > 0)DrawText(text,screen,rect.x + 12,rect.y + 32,IndexColor(DarkGrey),F_Medium);
}

int GetNumber(void (*loop)(void),char *title,int *number,int slength)
{
  int done = 0;
  int position = 0;
  char text[64];
  Uint8 *keys;
  SDLMod mod;
  SDL_Event event;
  text[0] = '\0';
  do
  {
    ResetBuffer();
    loop();
    DrawStringBox(title,text,slength);
    while(SDL_PollEvent(&event))
    {
      keys = SDL_GetKeyState(NULL);    
      mod = SDL_GetModState();
      if(keys[SDLK_RETURN] == 1)
      {
        *number = atoi(text);
        return 1;
      }
      else if(keys[SDLK_ESCAPE] == 1)return 0;
      else
      {
        if(event.type == SDL_KEYDOWN)
        {
          if((event.key.keysym.sym == SDLK_BACKSPACE)&&(position > 0))
          {
            text[--position] = '\0';
            if(position == 0)
            {
              text[0] = ' ';
              text[1] = '\0';
            }
          }
        }
        if(position < slength - 1)
        {
          switch(event.type)
          {
            case SDL_KEYDOWN:/*lets check against ANY possible character*/
              if(((event.key.keysym.sym >= SDLK_0)&&(event.key.keysym.sym <= SDLK_9)))
              {
                text[position++] = (event.key.keysym.sym - SDLK_0) + '0';
                text[position] = '\0';
              }
              else switch(event.key.keysym.sym)
              {
                case SDLK_PERIOD:
                  text[position++] = '.';
                  text[position] = '\0';
                  break;
                case SDLK_MINUS:
                  text[position++] = '-';
                  text[position] = '\0';
                  break;
                default:
                  break;
              }
              break;
          }
        }
      }
    }
    NextFrame();
    FrameDelay(63);
  }while(!done);
}

    
int GetString(void (*loop)(void),char *title,char *text,int slength)
{
  int done = 0;
  int position = 0;
  Uint8 *keys;
  SDLMod mod;
  SDL_Event event;
  do
  {
    ResetBuffer();
    loop();
    DrawStringBox(title,text,slength);
    while(SDL_PollEvent(&event))
    {
      keys = SDL_GetKeyState(NULL);    
      mod = SDL_GetModState();
      if(keys[SDLK_RETURN] == 1)return 1;
      else if(keys[SDLK_ESCAPE] == 1)return 0;
      else
      {
       if(event.type == SDL_KEYDOWN)
       {
         if((event.key.keysym.sym == SDLK_BACKSPACE)&&(position > 0))
         {
           text[--position] = '\0';
           if(position == 0)
           {
             text[0] = ' ';
             text[1] = '\0';
           }
         }
       }
       if(position < slength - 1)
       {
        switch(event.type)
        {
          case SDL_KEYDOWN:/*lets check against ANY possible character*/
            if(((event.key.keysym.sym >= SDLK_0)&&(event.key.keysym.sym <= SDLK_9)))
            {
              text[position++] = (event.key.keysym.sym - SDLK_0) + '0';
              text[position] = '\0';
            }
            else if((event.key.keysym.sym >= SDLK_a)&&(event.key.keysym.sym <= SDLK_z))
            {
              if(mod & KMOD_SHIFT)text[position++] = (event.key.keysym.sym - SDLK_a) + 'A';
                else text[position++] = (event.key.keysym.sym - SDLK_a) + 'a';
                text[position] = '\0';
            }
            else switch(event.key.keysym.sym)
            {
              case SDLK_SPACE:
                text[position++] = ' ';
                text[position] = '\0';
                    break;
              case SDLK_PERIOD:
                text[position++] = '.';
                text[position] = '\0';
                    break;
              case SDLK_UNDERSCORE:
                text[position++] = '_';
                text[position] = '\0';
                    break;
              case SDLK_MINUS:
                text[position++] = '-';
                text[position] = '\0';
                    break;
              case SDLK_SLASH:
                text[position++] = '/';
                text[position] = '\0';
                    break;
              default:
                break;
            }
            break;
        }
       }
      }
    }
    NextFrame();
    FrameDelay(63);
  }while(!done);

}

int TryAndOpen(char filename[40])
{
  FILE *file;
  file = fopen(filename, "r");
  if(file == NULL)
  {
    return 0;
  }
  fclose(file);
  return 1;
}