#include <stdlib.h>
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

	ship->hull->body.velocity.x = ship->vel.x;
	ship->hull->body.velocity.y = ship->vel.y;
	ship->hull->body.velocity.z = ship->vel.z;
}

void componentInherit(Ship *ship) //all the bodies that the ship is composed of must have their positions updated before this is called
{
	float realGunRot;

	if(turretRot < 0){realGunRot = turretRot + 360;}
	else {realGunRot = turretRot;}
	realGunRot += shipRot;
	if (realGunRot >= 360){realGunRot -= 360;}

	ship->turret->body.position.x = ((ship->turrOff.z * -sin(shipRot * DEGTORAD)) + ship->hull->body.position.x);
	ship->turret->body.position.z = ((ship->turrOff.z * cos(shipRot * DEGTORAD)) + ship->hull->body.position.z);

	ship->gun->body.position.x = ((ship->gunOff.z * -sin(realGunRot * DEGTORAD)) + ship->turret->body.position.x);
	ship->gun->body.position.z = ((ship->gunOff.z * cos(realGunRot * DEGTORAD)) + ship->turret->body.position.z);
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
	
	//SET GUN ELEVATION
	ship->gun->rotation.x = -gunElev;
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

Ship *spawnShip(Space *space, Vec3D spawnPt, int shipType)
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
		cube_set(ship->hull->body.bounds,-1,-1,-5,2,2,10);
		ship->hull->scale = vec3d(2,2,10);
	}
	mgl_callback_set(&ship->hull->body.touch,touch_callback,ship->hull);

	ship->turret->objModel = obj_load("models/cube.obj");
	ship->turret->texture = LoadSprite("models/cube_text.png",1024,1024);
	vec3d_cpy(ship->turret->body.position,spawnPt);
	if(shipType == 1){
		cube_set(ship->turret->body.bounds,-0.5,-0.5,-0.75,1,1,1.5);
		ship->turret->scale = vec3d(1,1,1.5);
	}
	mgl_callback_set(&ship->turret->body.touch,touch_callback,ship->turret);

	ship->gun->objModel = obj_load("models/cube.obj");
	ship->gun->texture = LoadSprite("models/cube_text.png",1024,1024);
	vec3d_cpy(ship->gun->body.position,spawnPt);
	if(shipType == 1){
		cube_set(ship->gun->body.bounds,0,0,0,0.1,0.1,0.1);
		ship->gun->scale = vec3d(0.25,0.25,2);
	}
	mgl_callback_set(&ship->gun->body.touch,touch_callback,ship->gun);


	space_add_body(space,&ship->hull->body);
	space_add_body(space,&ship->turret->body);
	space_add_body(space,&ship->gun->body);

	if(shipType == 1)
	{
		ship->turrOff = vec3d(0,2.5,5);
		//ship->turrOff.x = 1;
		//ship->turrOff.y = 2;
		//ship->turrOff.z = 9;

		ship->gunOff = vec3d(0,0,1.25);
		//ship->gunOff.x = 0.5;
		//ship->gunOff.y = 1;
		//ship->gunOff.z = 0.5;
	}

	ship->turret->body.position.y = 2.75;
	ship->gun->body.position.y = 3;

	return ship;
}