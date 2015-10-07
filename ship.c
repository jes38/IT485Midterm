#include <stdlib.h>
#include "ship.h"
#include "simple_logger.h"
#include "entity.h"

Ship *shipList = NULL;
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

	return ship;
}
