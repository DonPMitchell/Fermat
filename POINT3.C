/*
**	point3.c - 3D vector arithmetic module.
**
**	pat hanrahan
**
*/
# include <math.h>
# include "point3.h"

Point3 Pt3Origin = { 0., 0., 0. };

/* print vector */
void
Pt3Print( Point3 *v )
{
    /* Only works if Pt3Coord is a float */
    printf("[%g %g %g]\n", v->x, v->y, v->z);
}

void
Pt3From( Point3 *v, Pt3Coord x, Pt3Coord y, Pt3Coord z)
{
    v->x = x;
    v->y = y;
    v->z = z;
}

/* v2 = v1 */
void
Pt3Copy( Point3 *v1, Point3 *v2 )
{
    v2->x = v1->x;
    v2->y = v1->y;
    v2->z = v1->z;
}

/* v3 = v1 + v2 */
void
Pt3Add(Point3 *v1, Point3 *v2, Point3 *v3)
{
    v3->x = v1->x + v2->x;
    v3->y = v1->y + v2->y;
    v3->z = v1->z + v2->z;
}

/* v3 = v1 - v2 */
void
Pt3Sub(Point3 *v1, Point3 *v2, Point3 *v3)
{
    v3->x = v1->x - v2->x;
    v3->y = v1->y - v2->y;
    v3->z = v1->z - v2->z;
}

/* v2 = s * v1  */
void
Pt3Mul(Pt3Coord s, Point3 *v1, Point3 *v2)
{
    v2->x = s*v1->x;
    v2->y = s*v1->y;
    v2->z = s*v1->z;
}

void
Pt3Neg(Point3 *v1, Point3 *v2)
{
    v2->x = -v1->x;
    v2->y = -v1->y;
    v2->z = -v1->z;
}


/* v1 . v2 */
Pt3Coord
Pt3Dot( Point3 *v1, Point3 *v2 )
{
    return v1->x*v2->x+v1->y*v2->y+v1->z*v2->z;
}

/* v3 = v1 x v2 */
void
Pt3Cross( Point3 *v1, Point3 *v2, Point3 *v3)
{
    v3->x = v1->y*v2->z-v1->z*v2->y; 
    v3->y = v1->z*v2->x-v1->x*v2->z; 
    v3->z = v1->x*v2->y-v1->y*v2->x;
}

/* v1 . (v2 x v3) */
Pt3Coord 
Pt3TripleDot( Point3 *v1, Point3 *v2, Point3 *v3 )
{
    return v1->x*(v2->y*v3->z-v2->z*v3->y) 
         + v1->y*(v2->z*v3->x-v2->x*v3->z) 
         + v1->z*(v2->x*v3->y-v2->y*v3->x);
}

/* v4 = (v1 x v2) x v3 */
void
Pt3TripleCross( Point3 *v1, Point3 *v2, Point3 *v3, Point3 *v4 )
{
    Point3 v;
 
    Pt3Cross( v1, v2, &v );
    Pt3Cross( &v, v3, v4 );
}

Pt3Coord
Pt3Length( Point3 *v )
{
    return sqrt( v->x*v->x + v->y*v->y + v->z*v->z );
}

Pt3Coord 
Pt3Distance( Point3 *v1, Point3 *v2 )
{
    Point3 v12;
	
    Pt3Sub(v1,v2,&v12);

    return Pt3Length(&v12);
}

void
Pt3Unit( Point3 *v1, Point3 *v2)
{
    Pt3Coord len;

    len = Pt3Length(v1);
    if( len != 0. )
	Pt3Mul( 1./len, v1, v2 );
}


/*
 * lerp - linear interpolation
 *
 * v3 = (1-t)*v1 + t*v2
 */
void
Pt3Lerp( Pt3Coord t, Point3 *v1, Point3 *v2, Point3 *v3 )
{
    v3->x = (1.-t)*v1->x + t*v2->x;
    v3->y = (1.-t)*v1->y + t*v2->y;
    v3->z = (1.-t)*v1->z + t*v2->z;
}

void
Pt3Comb( Pt3Coord t1, Point3 *v1, Pt3Coord t2, Point3 *v2, Point3 *v3 )
{
    v3->x = t1*v1->x + t2*v2->x;
    v3->y = t1*v1->y + t2*v2->y;
    v3->z = t1*v1->z + t2*v2->z;
}

void
Pt3AddS( Pt3Coord s, Point3 *v1, Point3 *v2, Point3 *v3 )
{
   v3->x = s * v1->x + v2->x;
   v3->y = s * v1->y + v2->y;
   v3->z = s * v1->z + v2->z;
}
