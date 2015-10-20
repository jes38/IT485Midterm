#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ship.h"
#include "simple_logger.h"
#include "entity.h"
#include "vector.h"
#include "collisions.h"
#include "mgl_callback.h"
#include "space.h"

Ship *shipList = NULL;
int shipAcc = 0;
static int shipsInitd = 0;

void initShips()
{
    if (shipsInitd == 0)
	{
		shipList = (Ship *)malloc(sizeof(Ship)*maxShips);
		memset(shipList,0,sizeof(Ship)*maxShips);
	}
    shipsInitd = 1;
}

Ship *newShip()
{
	Ship *ship;
	Entity *temp;

	int i;
    for (i = 0; i < maxShips;i++)
    {
        if (!shipList[i].inuse)
        {
			memset(&shipList[i],0,sizeof(Ship));
            shipList[i].inuse = 1;
            ship = &shipList[i];
			i = maxShips + 10;
        }
	}
	if(ship == NULL) return NULL;

	ship->hull = entity_new();
	ship->turret = entity_new();
	ship->gun = entity_new();

	return ship;
}

void freeShip(Ship *ship)
{
	if (!ship)
    {
        slog("passed a null ship");
        return;
    }
    ship->inuse = 0;

	entity_free(ship->hull);
	entity_free(ship->turret);
	entity_free(ship->gun);

	ship->hull = NULL;
	ship->turret = NULL;
	ship->gun = NULL;
}

void freeAllShips(int notPlayer)
{
	int i;
	if (!shipList)return;

	if(notPlayer == 0)
	{
		for (i = 0;i < maxShips;i++)
		{
		    if (shipList[i].inuse){freeShip(&shipList[i]);}
		}
	}
	else if(notPlayer == 1)
	{
		for (i = 0;i < maxShips;i++)
		{
			if (shipList[i].inuse && shipList[i].shipID != 0){freeShip(&shipList[i]);}
		}
	}
}

void updateShipPos(Ship *ship)
{
	if (!ship)return;

	if(ship->shipID == 0)
	{
		takeShipInput(ship);
	}

	vec3d_add(ship->vel,ship->vel,ship->acc);
	vec3d_cpy(ship->hull->body.velocity, ship->vel);
}

void componentInherit(Ship *ship) //all the bodies that the ship is composed of must have their positions updated before this is called
{
	if(turretRot < 0){realTurrRot = turretRot + 360;}
	else {realTurrRot = turretRot;}
	realTurrRot += shipRot;
	if (realTurrRot >= 360){realTurrRot -= 360;}

	if(ship->shipID == 0)
	{
		ship->turret->body.position.x = ((ship->turrOff.z * -sin(shipRot * DEGTORAD)) + ship->hull->body.position.x);
		ship->turret->body.position.z = ((ship->turrOff.z * cos(shipRot * DEGTORAD)) + ship->hull->body.position.z);

		ship->gun->body.position.x = ((ship->gunOff.z * -sin(realTurrRot * DEGTORAD)) + ship->turret->body.position.x);
		ship->gun->body.position.z = ((ship->gunOff.z * cos(realTurrRot * DEGTORAD)) + ship->turret->body.position.z);
	}
	else
	{
		ship->turret->body.position.x = ((ship->turrOff.z * -sin(0 * DEGTORAD)) + ship->hull->body.position.x);
		ship->turret->body.position.z = ((ship->turrOff.z * cos(0 * DEGTORAD)) + ship->hull->body.position.z);

		ship->gun->body.position.x = ((ship->gunOff.z * -sin(0 * DEGTORAD)) + ship->turret->body.position.x);
		ship->gun->body.position.z = ((ship->gunOff.z * cos(0 * DEGTORAD)) + ship->turret->body.position.z);
	}
}

void updateAllShipPos()
{
	int i;
	if (!shipList)return;
    for (i = 0;i < maxShips;i++)
    {
        if (shipList[i].inuse)
        {
            updateShipPos(&shipList[i]);
        }
    }
}

void updateAllShipComp()
{
	int i;
    for (i = 0;i < maxShips;i++)
    {
        if (shipList[i].inuse)
        {
            componentInherit(&shipList[i]);
        }
    }
}

void takeShipInput(Ship *ship)
{
	float realTurretRot;
	
	ship->vel.x = (shipVel * -sin(shipRot * DEGTORAD));
	ship->vel.z = (shipVel * cos(shipRot * DEGTORAD));

	//ROTATE THE SHIP
	ship->hull->rotation.y = -shipRot;
	if(ship->hull->rotation.y < 0){ship->hull->rotation.y += 360;}
	if(ship->hull->rotation.y >= 360){ship->hull->rotation.y -= 360;}
	
	//SET TURRET ROTATION
	if(turretRot < 0){realTurretRot = turretRot + 360;}
	else {realTurretRot = turretRot;}
	realTurretRot += shipRot;
	if (realTurretRot >= 360){realTurretRot -= 360;}
	realTurretRot *= -1;

	ship->turret->rotation.y = realTurretRot;
	ship->gun->rotation.y = realTurretRot;
 
	//SET GUN ELEVATION ...So apparently I need something called Oiler's formula :P
	ship->gun->rotation.x = (-gunElev * cos(-ship->turret->rotation.y * DEGTORAD));
	//ship->gun->rotation.z = (-gunElev * sin(-ship->turret->rotation.y * DEGTORAD));
	
}

Ship *spawnShip(Space *space, Vec3D spawnPt, int shipType)
{
	Ship *ship;
	ship = newShip();

	ship->inuse = 1;
	ship->shipType = shipType;
	ship->shipID = numShips;
	numShips++;

	ship->acc = vec3d(0,0,0);
	ship->vel = vec3d(0,0,0);


	//Hull
	ship->hull->objModel = obj_load("models/cube.obj");
	ship->hull->texture = LoadSprite("models/cube_text.png",1024,1024);
	vec3d_cpy(ship->hull->body.position,spawnPt);
	if(shipType == 1)
	{
		cube_set(ship->hull->body.bounds,-2,-2,-10,4,4,20);
		ship->hull->scale = vec3d(2,2,10);
	}
	if(shipType == 2)
	{
		cube_set(ship->hull->body.bounds,-2,-2,-10,4,4,20);
		ship->hull->scale = vec3d(2,2,10);
	}
	if(shipType == 3)
	{
		cube_set(ship->hull->body.bounds,-2,-2,-2,4,4,4);
		ship->hull->scale = vec3d(2,2,2);
	}
	mgl_callback_set(&ship->hull->body.touch,touch_callback,ship->hull);
	ship->hull->body.id = ship->shipID;


	//Turret
	ship->turret->objModel = obj_load("models/cube.obj");
	ship->turret->texture = LoadSprite("models/cube_text.png",1024,1024);
	vec3d_cpy(ship->turret->body.position,spawnPt);
	if(shipType == 1)
	{
		cube_set(ship->turret->body.bounds,-1,-1,-1.5,2,2,3);
		ship->turret->scale = vec3d(1,1,1.5);
	}
	if(shipType == 2)
	{
		cube_set(ship->turret->body.bounds,-1,-1,-1.5,2,2,3);
		ship->turret->scale = vec3d(1,1,1.5);
	}
	if(shipType == 3)
	{
		cube_set(ship->turret->body.bounds,-1,-1,-1.5,2,2,3);
		ship->turret->scale = vec3d(1,1,1.5);
	}
	mgl_callback_set(&ship->turret->body.touch,touch_callback,ship->turret);
	ship->turret->body.id = ship->shipID;


	//Gun
	ship->gun->objModel = obj_load("models/cube.obj");
	ship->gun->texture = LoadSprite("models/cube_text.png",1024,1024);
	vec3d_cpy(ship->gun->body.position,spawnPt);
	if(shipType == 1)
	{
		cube_set(ship->gun->body.bounds,0,0,0,0.1,0.1,0.1);
		ship->gun->scale = vec3d(0.25,0.25,2);
	}
	if(shipType == 2)
	{
		cube_set(ship->gun->body.bounds,0,0,0,0.1,0.1,0.1);
		ship->gun->scale = vec3d(0.25,0.25,0.25);
	}
	if(shipType == 3)
	{
		cube_set(ship->gun->body.bounds,0,0,0,0.1,0.1,0.1);
		ship->gun->scale = vec3d(0.25,0.25,2);
	}
	mgl_callback_set(&ship->gun->body.touch,touch_callback,ship->gun);
	ship->gun->body.id = ship->shipID;


	space_add_body(space,&ship->hull->body);
	space_add_body(space,&ship->turret->body);
	space_add_body(space,&ship->gun->body);

	ship->hull->uid = ship->shipID;
	ship->turret->uid = ship->shipID;
	ship->gun->uid = ship->shipID;

	if(shipType == 1)
	{
		ship->turrOff = vec3d(0,2.75,5);
		ship->gunOff = vec3d(0,0,1.25);
		ship->turret->body.position.y = 2.75;
		ship->gun->body.position.y = 3;
	}
	if(shipType == 2)
	{
		ship->turrOff = vec3d(0,2.75,0);
		ship->gunOff = vec3d(0,0,0);
		ship->hull->body.position.y = -10;
		ship->turret->body.position.y = -7.25;
		ship->gun->body.position.y = -7;
	}
	if(shipType == 3)
	{
		ship->turrOff = vec3d(0,2.75,0);
		ship->gunOff = vec3d(0,0,1.25);
		ship->turret->body.position.y = 2.75;
		ship->gun->body.position.y = 3;
	}

	return ship;
}