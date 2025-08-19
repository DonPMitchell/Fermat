#ifndef LIGHTDEFS
#define LIGHTDEFS

#include "point3.h"
#include "color.h"

typedef struct {
    Point3 P;
    Color C;
    int (*illum)();
    void *data;
} Light;

extern int DistantLight( Light *, Point3 *P, Point3 *L, Color *C);
extern int PointLight( Light *, Point3 *P, Point3 *L, Color *C);
extern int VirtualPointLight( Light *, Point3 *P, Point3 *L, Color *C);
extern int VirtualDistantLight( Light *, Point3 *P, Point3 *L, Color *C);

extern void LightCreate( Point3 *P, Color *C, int (*illum)(), void *data );

#define LIGHT1  0x1
#define LIGHT2  0x2
#define LIGHT3  0x4
#define LIGHT4  0x8

extern void IlluminateBegin( int );
extern int Illuminate( Point3 *P, Point3 *L, Color *C );
extern void IlluminateEnd( );

#endif
