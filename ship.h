#ifndef __SHIP_H__
#define __SHIP_H__

#define maxShips	100

#include "entity.h"

typedef struct Ship_S
{
	int inuse;
	int shipID; //0 is player
	int shipType; //what kind of ship is it
	//Vec3D pos;  not sure if we need this
	Vec3D vel;
    Vec3D acc;
    //Vec3D rot; not sure if we need this either

	Vec3D turrOff;
	Vec3D gunOff;
	
	Entity *hull;
	Entity *turret;
	Entity *gun;
}Ship;

float shipVel;
float shipRot;
float turretRot;
float gunElev;
int numShips; //does not include player

Ship *newShip();
void initShips();
void updateShipPos(Ship *ship);
void componentInherit(Ship *ship);
void updateAllShipPos();
void updateAllShipComp();
void takeShipInput(Ship *ship);
Ship *spawnShip(Vec3D spawnPt, int shipType);

#endif
