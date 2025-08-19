#ifndef POINT3DEFS
#define POINT3DEFS
/*
** Definitions for 3D float points:
*/

typedef float Pt3Coord;
typedef struct { Pt3Coord x, y, z; } Point3;

extern Point3 Pt3Origin;

#define Pt3X(pt) (pt)->x
#define Pt3Y(pt) (pt)->y
#define Pt3Z(pt) (pt)->z

extern void Pt3Print( Point3 *v );
extern void Pt3From( Point3 *v, Pt3Coord x, Pt3Coord y, Pt3Coord z);
extern void Pt3Copy( Point3 *v1, Point3 *v2 );

extern void Pt3Add(Point3 *v1, Point3 *v2, Point3 *v3);
extern void Pt3Sub(Point3 *v1, Point3 *v2, Point3 *v3);
extern void Pt3Mul(Pt3Coord s, Point3 *v1, Point3 *v2);
extern void Pt3Neg(Point3 *v1, Point3 *v2);

extern Pt3Coord Pt3Dot( Point3 *v1, Point3 *v2 );
extern Pt3Coord Pt3TripleDot( Point3 *v1, Point3 *v2, Point3 *v3 );

extern void Pt3Cross( Point3 *v1, Point3 *v2, Point3 *v3);
extern void Pt3TripleCross( Point3 *v1, Point3 *v2, Point3 *v3, Point3 *v4 );

extern Pt3Coord Pt3Length( Point3 *v );
extern Pt3Coord Pt3Distance( Point3 *v1, Point3 *v2 );
extern void Pt3Unit( Point3 *v1, Point3 *v2);

extern void Pt3Lerp( Pt3Coord t, Point3 *v1, Point3 *v2, Point3 *v3 );
extern void Pt3Comb( Pt3Coord t1,Point3 *v1,Pt3Coord t2,Point3 *v2,Point3 *v3);

extern void Pt3AddS( Pt3Coord s, Point3 *v1, Point3 *v2, Point3 *v3 );

#endif
