#include "color.h"

Color White = { 1., 1., 1. };
Color Black = { 0., 0., 0. };

void
CoPrint( Color *c )
{
}
void
CoCopy( Color *c1, Color *c2 )
{
    bcopy( (char *)c1, (char *)c2, sizeof(Color) );
}
void
CoFrom( Color *c, CoCoord r, CoCoord g, CoCoord b )
{
    c->r = r;
    c->g = g;
    c->b = b;
}
void
CoAdd( Color *c1, Color *c2, Color *c3 )
{
    c3->r = c1->r + c2->r;
    c3->g = c1->g + c2->g;
    c3->b = c1->b + c2->b;
}
void
CoSub( Color *c1, Color *c2, Color *c3 )
{
    c3->r = c1->r - c2->r;
    c3->g = c1->g - c2->g;
    c3->b = c1->b - c2->b;
}
void
CoFilter( Color *c1, Color *c2, Color *c3 )
{
    c3->r = c1->r * c2->r;
    c3->g = c1->g * c2->g;
    c3->b = c1->b * c2->b;
}
void
CoAccum( CoCoord s, Color *c1, Color *c2, Color *c3 )
{
    c3->r += s * c1->r * c2->r;
    c3->g += s * c1->g * c2->g;
    c3->b += s * c1->b * c2->b;
}

void
CoScale( CoCoord s, Color *c1, Color *c2 )
{
    c2->r = s * c1->r;
    c2->g = s * c1->g;
    c2->b = s * c1->b;
}
