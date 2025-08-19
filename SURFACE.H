#ifndef SURFACEDEF
#define SURFACEDEF

#include "ray.h"
#include "interval.h"
#include "material.h"

typedef struct surface {
    Material *mat;
    Bytecode *code;
} Surface;

Surface *EllipseCreate( Point3 *f1, Point3 *f2, float d, Material *m );
Surface *SphereCreate( Point3 *cen, float r, Material *m );
Surface *TorusCreate( float major, float minor, Material *m );
Surface *CuboidCreate( Material *m );
Surface *SteinerCreate( Material *m );
Surface *MonkeyCreate( Material *m );

Surface *SurfaceCreate( Bytecode *c, int n, Material *m );
int SurfaceIntersect( Surface *s, Ray *r, double *t, Point3 *P, Point3 *N );

extern Color Shade(Surface *s,  Point3 *I,Point3 *P,Point3 *N,int d);
extern Color Whitted(Surface *s,Point3 *I,Point3 *P,Point3 *N,int d);
extern Color Checker(Surface *s,Point3 *I,Point3 *P,Point3 *N,int d);
extern Color CheckerWhitted(Surface *s,Point3 *I,Point3 *P,Point3 *N,int d);
extern Color Gaussian(Surface *s,Point3 *I,Point3 *P,Point3 *N,int d);

#endif /* SURFACEDEF */
