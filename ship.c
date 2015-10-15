#include <stdlib.h>
#include <math.h>
#include "ship.h"
#include "simple_logger.h"
#include "entity.h"
#include "vector.h"
#include "collisions.h"
#include "mgl_callback.h"

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

	temp = entity_new();
	ship->hull = temp;
	ship->turret = entity_new();
	ship->gun = entity_new();

	/*
	ship->turret->parent = ship->hull;
	ship->gun->parent = ship->turret;
	ship->hull->parent = NULL;
	*/

	return ship;
}

void updateShipPos(Ship *ship)
{
	if (!ship)return;

	if(ship->shipID == 0)
	{
		takeShipInput(ship);
	}

	ship->vel.x = (ship->vel.x + ship->acc.x);
	ship->vel.y = (ship->vel.y + ship->acc.y);
	ship->vel.z = (ship->vel.z + ship->acc.z);

	ship->hull->body.velocity = ship->vel;
}

void componentInherit(Ship *ship) //all the bodies that the ship is composed of must have their positions updated before this is called
{
	ship->turret->body.position.x = ((ship->turrOff.x * cos(ship->hull->rotation.x * DEGTORAD)) + ship->hull->body.position.x);
	ship->turret->body.position.y = ((ship->turrOff.y * cos(ship->hull->rotation.y * DEGTORAD)) + ship->hull->body.position.y);
	ship->turret->body.position.z = ((ship->turrOff.z * sin(ship->hull->rotation.z * DEGTORAD)) + ship->hull->body.position.z);

	ship->gun->body.position.x = ((ship->gunOff.x * cos(ship->turret->rotation.x * DEGTORAD)) + ship->turret->body.position.x);
	ship->gun->body.position.y = ((ship->gunOff.y * sin(ship->turret->rotation.y * DEGTORAD)) + ship->turret->body.position.y);
	ship->gun->body.position.z = ((ship->gunOff.z * sin(ship->turret->rotation.z * DEGTORAD)) + ship->turret->body.position.z);
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
	slog("ship vel is %s", shipVel);
	ship->hull->body.velocity.x = ((shipVel * cos(ship->hull->rotation.x * DEGTORAD)) + ship->hull->body.velocity.x);
	//ship->hull->body.velocity.y = ((shipVel * cos(ship->hull->rotation.y * DEGTORAD)) + ship->hull->body.position.y);
	ship->hull->body.velocity.z = ((shipVel * sin(ship->hull->rotation.z * DEGTORAD)) + ship->hull->body.velocity.z);


	//ROTATE THE SHIP
	if(shipRot != 0)
	{
		ship->hull->rotation.y += shipRot;
		if(ship->hull->rotation.y < 0){ship->hull->rotation.y += 360;}
		if(ship->hull->rotation.y >= 360){ship->hull->rotation.y -= 360;}
	}


	//SET GUN ELEVATION
	/*
	if(gunElev > 0)
	{
		ship->gun->rotation.x = gunElev;
	}
	else
	{
		ship->gun->rotation.x = (gunElev + 360);
	}
	*/

	/*
	//SET TURRET ROTATION
	if(turretRot > 0)
	{
		ship->turret->rotation.y = turretRot;
	}
	else
	{
		ship->turret->rotation.y = (turretRot + 360);
	}
	*/
}

static void touch_callback(void *data, void *context) //put this here temporarily
{
    Entity *me,*other;
    Body *obody;
    if ((!data)||(!context))return;
    me = (Entity *)data;
    obody = (Body *)context;
    if (entity_is_entity(obody->touch.data))
    {
        other = (Entity *)obody->touch.data;
    }
}

Ship *spawnShip(Vec3D spawnPt, int shipType)
{
	Ship *ship;
	ship = newShip();

	ship->inuse = 1;
	ship->shipType = shipType;
	ship->shipID = numShips;
	numShips++;

	ship->acc.x = 0;
	ship->acc.y = 0;
	ship->acc.z = 0;

	ship->vel.x = 0;
	ship->vel.y = 0;
	ship->vel.z = 0;

	ship->hull->objModel = obj_load("models/cube.obj");
	ship->hull->texture = LoadSprite("models/cube_text.png",1024,1024);
	vec3d_cpy(ship->hull->body.position,spawnPt);
	if(shipType == 1){
		cube_set(ship->hull->body.bounds,-1,-1,-1,2,2,10);
		ship->hull->scale = vec3d(2,2,10);
	}
	mgl_callback_set(&ship->hull->body.touch,touch_callback,ship->hull);

	ship->turret->objModel = obj_load("models/cube.obj");
	ship->turret->texture = LoadSprite("models/cube_text.png",1024,1024);
	vec3d_cpy(ship->turret->body.position,spawnPt);
	if(shipType == 1){
		cube_set(ship->turret->body.bounds,-1,-1,-1,1,1.5,1);
		ship->turret->scale = vec3d(1,1.5,1);
	}
	mgl_callback_set(&ship->turret->body.touch,touch_callback,ship->turret);

	ship->gun->objModel = obj_load("models/cube.obj");
	ship->gun->texture = LoadSprite("models/cube_text.png",1024,1024);
	vec3d_cpy(ship->gun->body.position,spawnPt);
	if(shipType == 1){
		cube_set(ship->gun->body.bounds,-1,-1,-1,0.5,0.5,2);
		ship->gun->scale = vec3d(0.5,0.5,2);
	}
	mgl_callback_set(&ship->gun->body.touch,touch_callback,ship->gun);

	if(shipType == 1)
	{
		ship->turrOff.x = 1;
		ship->turrOff.y = 2;
		ship->turrOff.z = 9;

		ship->gunOff.x = 0.5;
		ship->gunOff.y = 1;
		ship->gunOff.z = 0.5;
	}
}