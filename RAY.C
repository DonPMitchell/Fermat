#include <math.h>
#include "ray.h"

void
RayFrom( Ray *r, Point3 *o, Point3 *d )
{
    Pt3Copy( o, &r->o );
    Pt3Copy( d, &r->d );
    Pt3Unit( &r->d, &r->d );
}

void
RayHit( Ray *r, double t, Point3 *p )
{
    Pt3AddS(t, &r->d, &r->o, p);
}

/* Assumes N unitized */
void
RayReflection(Point3 *I, Point3 *N, Point3 *R)
{
    float eta, gam;

    eta = 1.0;
    gam = -2.0*Pt3Dot(I,N);
    Pt3Comb(eta, I, gam, N, R);
}

/* Assumes I & N unitized */
/* eta = n(I)/n(T) */

/* Note: if we set eta=-1, and reverse the final direction of T
   we get the reflection law. */
int
RayTransmission(float eta, Point3 *I, Point3 *N, Point3 *T)
{
    float cosi, cost;
    float gam;

    cosi = -Pt3Dot(I,N);
    cost = 1.0 - eta*eta*(1.0 - cosi*cosi);
    if (cost < 0.0)
        return 0;
    cost = sqrt(cost);
    gam = eta*cosi - cost;

    Pt3Comb(eta, I, gam, N, T);
    return 1;
}
