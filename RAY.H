#ifndef RAYDEFS
#define RAYDEFS

#include "point3.h"

typedef struct {
    Point3 o, d;
} Ray;

#define RAY_EPSILON 1e-5

extern void RayFrom( Ray *r, Point3 *o, Point3 *d );
extern void RayHit( Ray *r, double t, Point3 *p );
extern void RayReflection(Point3 *I, Point3 *N, Point3 *R);
/* eta = n1 / n2 */
extern int RayTransmission(float eta, Point3 *I, Point3 *N, Point3 *T);

#endif
