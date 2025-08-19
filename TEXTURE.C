#include <math.h>
#include "point3.h"
#include "color.h"

void 
SolidChecker( Point3 *P, Color *C )
{
    int x, y, z;
    static Color Red = { 0.8, 0.8, 0.8 };

    x = P->x - 100;
    y = P->y - 100;
    z = P->z - 100;
    *C = (x+y+z)%2 ? White : Red;
}
