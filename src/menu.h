#ifndef __MENUS__
#define __MENUS__

#include "entity.h"
#include "window.h"

void MainMenu();
void PlayerConfig(Entity *self);/*Customize the player's preferences*/
void LoadDashboard();
void CloseDashboard();
void DrawDashboard();
void UpdateDashboard();/*input handling for dashboard*/


#endif