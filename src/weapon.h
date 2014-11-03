#ifndef __WEAPONS__
#define __WEAPONS__

#include "entity.h"

/*
    Weapons in this game will be a separate data structure that entities will access through an index.
*/

#define NumWeapons    11


typedef struct WEAPON_T
{
  char Name[40];        /*weapon's name*/
  int cooldown;         /*how long it will take to cool down between shots*/
  int Ucooldown;        /*Ultimate cooldown*/
  int damage;           /*how much damage each projectile will deal*/
  float kick;             /*how much kick the weapon carries*/
  int speed;
  float spread;         /*how much variation can be expeted between each bullet.*/
  int volume;           /*how much is produced at once*/
  int gravity;          /*if the projectiles produced will be affected by gravity*/
  int bounce;
  int size;             /*some weapons have different sized projectiles*/
  void (*fireweapon)   (Entity *self,struct WEAPON_T *weapon,int direction); /*call this when you fire the weapon*/
  void (*fireultimate) (Entity *self,struct WEAPON_T *weapon);               /*call this when you fure the ultimate attack*/
  char firesound[40];   /*paths to the sound files that the weapons use*/
  char ultimatesound[40];
}Weapon;

void PrecacheWeaponSounds();                    /*load sounds that the weapons use*/
void FireWeapon(Entity *self,int Direction);    /*fires the entity's current equipped weapon in the desired direction*/
void FireUltimateWeapon(Entity *self);          /*fires the entity's curreny equipped weapon's ultimate attack*/
int GetWeaponByName(char name[40]);             /*return the index of the weapon in the list by the weapon's name*/
char *PrintWeaponName(int index);
#endif