#ifndef __SHIP_H__
#define __SHIP_H__

#define maxShips	100

#include "entity.h"

typedef struct Ship_S
{
	int inuse;
	
	Entity *hull;
	Entity *turret;
	Entity *gun;
}Ship;

Ship *newShip();

#endif
