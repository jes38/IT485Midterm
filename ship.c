#include <stdlib.h>
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
	int i;
    for (i = 0; i < maxShips;i++)
    {
        if (!shipList[i].inuse)
        {
			memset(&shipList[i],0,sizeof(Entity));
            shipList[i].inuse = 1;
            ship = &shipList[i];
        }
	}
	if(ship == NULL) return NULL;

	ship->hull = entity_new();
	ship->turret = entity_new();
	ship->gun = entity_new();

	ship->turret->parent = ship->hull;
	ship->gun->parent = ship->turret;
	ship->hull->parent = NULL;

	return ship;
}

void updateShipPos(Ship *ship)
{
	ship->vel.x = (ship->vel.x + ship->acc.x);
	ship->vel.y = (ship->vel.y + ship->acc.y);
	ship->vel.z = (ship->vel.z + ship->acc.z);

	/* not sure if we need this
	ship->pos.x = (ship->pos.x + ship->vel.x);
	ship->pos.y = (ship->pos.y + ship->vel.y);
	ship->pos.z = (ship->pos.z + ship->vel.z);
	*/


	ship->hull->body.velocity.x = ship->vel.x;
	ship->hull->body.velocity.y = ship->vel.y;
	ship->hull->body.velocity.z = ship->vel.z;

	ship->turret->body.velocity.x = ship->vel.x;
	ship->turret->body.velocity.y = ship->vel.y;
	ship->turret->body.velocity.z = ship->vel.z;

	ship->gun->body.velocity.x = ship->vel.x;
	ship->gun->body.velocity.y = ship->vel.y;
	ship->gun->body.velocity.z = ship->vel.z;

	/* we are not using entity acceleration yet

	ship->hull->acceleration.x = ship->acc.x;
	ship->hull->acceleration.y = ship->acc.y;
	ship->hull->acceleration.z = ship->acc.z;

	ship->turret->acceleration.x = ship->acc.x;
	ship->turret->acceleration.y = ship->acc.y;
	ship->turret->acceleration.z = ship->acc.z;

	ship->gun->acceleration.x = ship->acc.x;
	ship->gun->acceleration.y = ship->acc.y;
	ship->gun->acceleration.z = ship->acc.z;
	*/
}

void componentInherit(Ship *ship) //all the bodies that the ship is composed of must have their positions updated before this is called
{
	ship->turret->body.position.x = (ship->hull->body.position.x + ship->turrOff.x);
	ship->turret->body.position.y = (ship->hull->body.position.y + ship->turrOff.y);
	ship->turret->body.position.z = (ship->hull->body.position.z + ship->turrOff.z);

	ship->gun->body.position.x = (ship->turret->body.position.x + ship->gunOff.x);
	ship->gun->body.position.y = (ship->turret->body.position.y + ship->gunOff.y);
	ship->gun->body.position.z = (ship->turret->body.position.z + ship->gunOff.z);

	if (ship->shipID == 0)
	{
		ship->turret->rotation.x = ship->turret->rotation.x;
		ship->turret->rotation.y = (ship->turret->rotation.y + turretRot);
		ship->turret->rotation.z = ship->turret->rotation.z;
	
		ship->gun->rotation.x = (ship->turret->rotation.x + gunElev);
		ship->gun->rotation.y = ship->turret->rotation.y;
		ship->gun->rotation.z = ship->turret->rotation.z;
	}
	else
	{
		ship->turret->rotation.x = ship->turret->rotation.x;
		ship->turret->rotation.y = ship->turret->rotation.y;
		ship->turret->rotation.z = ship->turret->rotation.z;
	
		ship->gun->rotation.x = ship->turret->rotation.x;
		ship->gun->rotation.y = ship->turret->rotation.y;
		ship->gun->rotation.z = ship->turret->rotation.z;
	}
}

void updateAllShipPos()
{
	int i;
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
	float realVel;

	//SET SHIP ACCELERATION
	realVel = (ship->vel.x * ship->vel.x) + (ship->vel.z * ship->vel.z); 
	if(ship->vel.z < 0){realVel *= -1;}//for now I'll assume that the ship only moves along the z

	if(shipVel > 0 && realVel < (shipVel * shipVel))
	{
		ship->acc.z = 2;
	}
	else if(shipVel < 0 && realVel > (shipVel * shipVel * -1))
	{
		ship->acc.z = -2;
	}
	else
	{
		ship->acc.z = 0;
	}


	//ROTATE THE SHIP
	if(shipRot != 0)
	{
		ship->hull->rotation.y += shipRot;
		if(ship->hull->rotation.y < 0){ship->hull->rotation.y += 360;}
		if(ship->hull->rotation.y >= 360){ship->hull->rotation.y -= 360;}
	}


	//SET GUN ELEVATION
	if(gunElev > 0)
	{
		ship->gun->rotation.x = gunElev;
	}
	else
	{
		ship->gun->rotation.x = (gunElev + 360);
	}


	//SET TURRET ROTATION
	if(turretRot > 0)
	{
		ship->turret->rotation.y = turretRot;
	}
	else
	{
		ship->turret->rotation.y = (turretRot + 360);
	}
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
	if(shipType == 1){cube_set(ship->hull->body.bounds,-1,-1,-1,2,2,10);}
	mgl_callback_set(&ship->hull->body.touch,touch_callback,ship->hull);

	ship->turret->objModel = obj_load("models/cube.obj");
	ship->turret->texture = LoadSprite("models/cube_text.png",1024,1024);
	vec3d_cpy(ship->turret->body.position,spawnPt);
	if(shipType == 1){cube_set(ship->turret->body.bounds,-1,-1,-1,1,1.5,1);}
	mgl_callback_set(&ship->turret->body.touch,touch_callback,ship->turret);

	ship->gun->objModel = obj_load("models/cube.obj");
	ship->gun->texture = LoadSprite("models/cube_text.png",1024,1024);
	vec3d_cpy(ship->gun->body.position,spawnPt);
	if(shipType == 1){cube_set(ship->gun->body.bounds,-1,-1,-1,0.5,0.5,2);}
	mgl_callback_set(&ship->gun->body.touch,touch_callback,ship->gun);

	if(shipType == 1)
	{
		ship->turrOff.x = 0;
		ship->turrOff.y = -4;
		ship->turrOff.z = 4;

		ship->gunOff.x = 0;
		ship->gunOff.y = 0;
		ship->gunOff.z = 0;
	}
}