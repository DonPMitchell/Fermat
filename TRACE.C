#include <math.h>
#include <stdio.h>
#include "picfile.h"
#include "surface.h"

Surface *scene;
extern Surface checkerfloor;

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

static int
between( Point3 *P1, Point3 *P2, Point3 *P )
{
#define EPSILON 1e-5

    if( min(P1->x,P2->x)+EPSILON > P->x ) return 0;
    if( max(P1->x,P2->x)-EPSILON < P->x ) return 0;
    if( min(P1->y,P2->y)+EPSILON > P->y ) return 0;
    if( max(P1->y,P2->y)-EPSILON < P->y ) return 0;
    if( min(P1->z,P2->z)+EPSILON > P->z ) return 0;
    if( max(P1->z,P2->z)-EPSILON < P->z ) return 0;
    return 1;
}

int
Shadow( Point3 *P1, Point3 *P2 )
{
    Point3 P, N, D;
    Ray r;
    double t;

    Pt3Sub( P2, P1, &D );
    RayFrom( &r, P1, &D );

    if (SurfaceIntersect(scene, &r, &t, &P, &N ))
	if( between( P1, P2, &P) ) 
	    return 1;
    if (FloorIntersect( &r, &t, &P, &N ))
	if( between( P1, P2, &P ) )
	    return 1;
    return 0;
}

int
ShadowD( Point3 *O, Point3 *D )
{
    Point3 P, N;
    Ray r;
    double t;

    RayFrom( &r, O, D );

    if (SurfaceIntersect(scene, &r, &t, &P, &N ))
	return 1;
    if (FloorIntersect( &r, &t, &P, &N ))
	return 1;
    return 0;
}

Color
Trace( Ray *r, int depth )
{
    double t;
    Point3 P, N;

    if (SurfaceIntersect(scene, r, &t, &P, &N ))
	return Shade( scene, &r->d, &P, &N, depth );
    else if( FloorIntersect( r, &t, &P, &N ) )
	return Shade( &checkerfloor, &r->d, &P, &N, depth );

    return Black;
}

Color
Shoot( Point3 *e, Point3 *xv, Point3 *yv, Point3 *zv, 
       float x, float y, int depth )
{
    Ray ray;

    Pt3Copy( e, &ray.o );
    Pt3Comb(x, xv, y, yv, &ray.d) ;
    Pt3Add(&ray.d, zv, &ray.d) ;
    Pt3Unit(&ray.d,&ray.d) ;

    return Trace(&ray, depth) ;
}

/*
static short    rbuf[4096] ;
static short    gbuf[4096] ;
static short    bbuf[4096] ;
*/
static char pbuf[4096*3];

#define clamp(a,min,max) ((a) < (min) ? (min) : ((a) > (max) ? (max) : (a)))

Render( s,
    eye, coi, up, fov, xres, yres, xmin, xmax, ymin, ymax, filename, depth)
 Surface *s;
 Point3 *eye, *coi, *up ;
 float   fov ;
 int    xres, yres ;
 int depth;
{
    Ray    ray ;
    Point3 xvec, yvec ;
    Point3 view ;
    float  fwidth ;    /* width of the view frustrum */
    PICFILE *image ;
    float  x, y, dx, dy ;
    float  r, g, b ;
    int    i, j ;
    Color  C ;
    extern int Debug;

    scene = s;

    Pt3Sub(eye, coi, &view) ;
    Pt3Cross(up, &view, &xvec) ;
    Pt3Unit(&xvec,&xvec) ;

    Pt3Cross(&view, &xvec, &yvec) ;
    Pt3Unit(&yvec,&yvec) ;

    fwidth = (float) tan((double) fov) ;

    Pt3Mul(fwidth * (float) yres / (float) xres, &yvec, &yvec) ;
    Pt3Mul(fwidth, &xvec, &xvec) ;

    dx = 2.0 / (float) (xres - 1) ;
    dy = 2.0 / (float) (yres - 1) ;

    Pt3Sub(coi, eye, &view) ;
    Pt3Unit(&view,&view) ;

    image = picopen_w(filename, RUNCODED, 0, 0, xmax-xmin+1, ymax-ymin+1,
			"rgb", 0, 0);
/*  for (i = 0, y = -1.0  ; i < yres ; i++, y += dy)	*/
printf("%f %f %f\n", eye->x, eye->y, eye->z);
    for (i = yres-1, y = 1.0  ; i >= 0 ; --i, y -= dy)
      if( ymin <= i && i <= ymax ) {
        for (j = 0, x = -1.0 ; j < xres ; j++, x += dx)
	  if( xmin <= j && j <= xmax ) {
printf("%d %d ", i, j);
	    C = Shoot( eye, &xvec, &yvec, &view, x, y, depth );
printf("\n");

/*
            rbuf[j-xmin] = (short) (255.0 * clamp(C.r,0.,1.)) ;
            gbuf[j-xmin] = (short) (255.0 * clamp(C.g,0.,1.)) ;
            bbuf[j-xmin] = (short) (255.0 * clamp(C.b,0.,1.)) ;
*/
	    pbuf[3*(j-xmin)+0] = (255.0 * clamp(C.r,0.,1.));
	    pbuf[3*(j-xmin)+1] = (255.0 * clamp(C.g,0.,1.));
	    pbuf[3*(j-xmin)+2] = (255.0 * clamp(C.b,0.,1.));
         }
	picwrite(image, pbuf);
        fprintf(stderr, "[%d]\n", i) ;
      }
    picclose(image) ;
}
