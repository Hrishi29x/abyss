#ifndef __WINDOW__
#define __WINDOW__

#include "graphics.h"

#define WINBUTTONS  16

typedef struct
{
  Sprite *sprite;  
  int mx,my;
  int buttons;
  int oldbuttons;
  int mframe;
  int mdelay;
  int mrate;
}Mouse_T;

typedef struct S_BUTTON
{
  int inuse;
  Sprite *sprite;
  int usesprite;
  SDL_Rect rect;
  int State;
  int layer;
  Uint8 Hotkey;
  char text[80];
  int bg,fg;
}Button;


Button *NewButton(char *spritename,char *text,int x, int y, int w, int h,Uint8 hotkey,int layer,int c1,int c2);
void UpdateButton(Button *button);
void UpdateButtonsByLayer(int layer);
void UpdateAllButtons();
int GetButtonState(Button *button);
void FreeButton(Button *button);
void DrawButton(Button *button);
void InitButtonList();
void FreeButtonList();


typedef struct S_Window
{
  int inuse;
  Button *buttonlist[WINBUTTONS];
  int buttoncount;
  SDL_Rect  rect;
  Sprite *sprite;
  int usesprite;
  char title[80];
  int layer;
  void (*drawwindow)(struct S_Window *self);
  void (*updatewindow)(struct S_Window *self);
  int fg,bg;    /*colors*/
}Window;


void NewWindowButton(Window *window, Button *button);/*given a pointer to an existing window and button to assign that button to the window*/
void InitWindowList();
void FreeWindowList();
void FreeWindow(Window *window);
Window *NewWindow(char *title,char *spritepath,int x,int y, int w,int h,int fg,int bg,int layer,void (*drawwindow)(Window *self),void (*updatewindow)(Window *self));
void DrawWindowsByLayer(int layer);
void DrawAllWindows();
void UpdateWindowsByLayer(int layer);
void UpdateAllWindows();
void DrawWindowGeneric(Window *self);

void LoadMouse();
void CloseMouse();
void DrawMouse();
void UpdateMouse();
int  MouseIn(int x,int y,int w, int h);

int  TryAndOpen(char filename[40]);

int GetString(void (*loop)(void),char *title,char *text,int slength);
int GetNumber(void (*loop)(void),char *title,int *number,int slength);



#endif