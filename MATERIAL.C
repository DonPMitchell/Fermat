#include <math.h>
#include "surface.h"
#include "light.h"
#include "ray.h"

extern Color Trace( Ray *r, int depth );

/* Assumes N & L unitized */
static void
Diffuse( Point3 *N, Point3 *L, Color *Cd, Color *Cl, Color *C )
{
    float d;

    d = Pt3Dot(N, L) ;
    if (d > 0.0)
	CoAccum( d, Cd, Cl, C );
}

/* Assumes I & L, N unitized */
static void
Specular( Point3 *I, Point3 *N, Point3 *L, float shininess,
    Color *Cs, Color *Cl, Color *C )
{
    Point3 H;
    float d;

    Pt3Sub( L, I, &H );
    Pt3Unit( &H, &H );
    d = pow( Pt3Dot( N, &H ), shininess );
    CoAccum( d, Cs, Cl, C );
}

Color
Shade( Surface *s, Point3 *I, Point3 *P, Point3 *N, int depth)
{
    return (*s->mat->shade)(s, I, P, N, depth);
}

Color
Whitted( Surface *s, Point3 *I, Point3 *P, Point3 *Np, int depth)
{
    int l;
    Color C, Cl;
    Point3 L, R, T, N;
    Ray r;
    int in;

    Pt3Copy( Np, &N );
    Pt3Unit( &N, &N );
    in = Pt3Dot( I, &N ) > 0;
    if( in )
	Pt3Neg( &N, &N );

    IlluminateBegin( s->mat->lights );
	CoCopy( &s->mat->Ca, &C );
	while( Illuminate( P, &L, &Cl ) ) {
	    Pt3Unit( &L, &L );
	    Diffuse( &N, &L, &s->mat->Cd, &Cl, &C );
	    Specular( I, &N, &L, s->mat->shininess, &s->mat->Cs, &Cl, &C );
	}
    IlluminateEnd();

    if( --depth > 0 ) {
	/* Ideal reflection */
	if( s->mat->kr != 0. ) {
	    RayReflection( I, &N, &R );
printf(" %f %f %f  %f %f %f", P->x, P->y, P->z, R.x, R.y, R.z);
	    RayFrom( &r, P, &R );
	    Cl = Trace( &r, depth );
	    CoAccum( s->mat->kr, &s->mat->Cs, &Cl, &C );
	}
	/* Ideal transmission */
	if( s->mat->kt != 0. ) {
	    if( RayTransmission( in ? s->mat->n : 1/s->mat->n, I, &N, &T ) ) {
		RayFrom( &r, P, &T );
		Cl = Trace( &r, depth );
		CoAccum( s->mat->kt, &s->mat->Ct, &Cl, &C );
	    }
	}
    }
    return C;
}

Color
CheckerWhitted( Surface *s, Point3 *I, Point3 *P, Point3 *Np, int depth)
{
    int l;
    Color C, Cd, Ct, Cl;
    Point3 L, R, T, N;
    Ray r;
    int in;

    Pt3Copy( Np, &N );
    Pt3Unit( &N, &N );
    in = Pt3Dot( I, &N ) > 0;
    if( in )
	Pt3Neg( &N, &N );

    SolidChecker( P, &Ct );

    /* Ambient */
    CoCopy( &s->mat->Ca, &C );
    CoFilter( &C, &Ct, &C );
    CoCopy( &s->mat->Cd, &Cd );
    CoFilter( &Cd, &Ct, &Cd );
    IlluminateBegin( s->mat->lights );
	while( Illuminate( P, &L, &Cl ) ) {
	    Pt3Unit( &L, &L );
	    Diffuse( &N, &L, &Cd, &Cl, &C );
	    Specular( I, &N, &L, s->mat->shininess, &s->mat->Cs, &Cl, &C );
	}
    IlluminateEnd();

    if( --depth > 0 ) {
	/* Ideal reflection */
	if( s->mat->kr != 0. ) {
	    RayReflection( I, &N, &R );
	    RayFrom( &r, P, &R );
	    Cl = Trace( &r, depth );
	    CoAccum( s->mat->kr, &s->mat->Cs, &Cl, &C );
	}
	/* Ideal transmission */
	if( s->mat->kt != 0. ) {
	    if( RayTransmission( in ? s->mat->n : 1/s->mat->n, I, &N, &T ) ) {
		RayFrom( &r, P, &T );
		Cl = Trace( &r, depth );
		CoAccum( s->mat->kt, &s->mat->Ct, &Cl, &C );
	    }
	}
    }
    return C;
}

Color
Checker( 
   Surface *s, Point3 *I, Point3 *P, Point3 *Np, int depth)
{
    int l;
    Color C, Cd, Ct, Cl;
    Point3 L, N;
    int in;

    Pt3Copy( Np, &N );
    Pt3Unit( &N, &N );
    in = Pt3Dot( I, &N ) > 0;
    if( in )
	Pt3Neg( &N, &N );

    SolidChecker( P, &Ct );

    /* Ambient */
    CoCopy( &s->mat->Ca, &C );
    CoFilter( &C, &Ct, &C );
    CoCopy( &s->mat->Cd, &Cd );
    CoFilter( &Cd, &Ct, &Cd );

    IlluminateBegin( s->mat->lights );
	while( Illuminate( P, &L, &Cl ) ) {
	    Pt3Unit( &L, &L );
	    Diffuse( &N, &L, &Cd, &Cl, &C );
	    Specular( I, &N, &L, s->mat->shininess, &s->mat->Cs, &Cl, &C );
	}
    IlluminateEnd();
    return C;
}

Color
Gaussian( 
   Surface *s, Point3 *I, Point3 *P, Point3 *Np, int depth)
{
    Color C, Cd, Ct, Cl;
    Point3 L, N, Pu, Pv;
    float k, k1, k2, cosu, sinu;
    static Color R = {0.8,0.2,0.2};
    static Color G = {0.2,0.8,0.2};
    static Color B = {0.2,0.2,0.8};
    int in;

    Pt3Copy( Np, &N );
    Pt3Unit( &N, &N );
    in = Pt3Dot( I, &N ) > 0;
    if( in )
	Pt3Neg( &N, &N );

    Pt3Cross( I, &N, &Pu );
    Pt3Cross( &N, &Pu, &Pv );
    Pt3Unit( &Pu, &Pu );
    Pt3Unit( &Pv, &Pv );
    k = SurfaceKMCurvature( s->code, P, &Pu, &Pv, &k1, &k2, &cosu, &sinu );
    if(      k1 <= 0 && k2 <= 0 ) 
	Ct = R;
    else if( k1 >= 0 && k2 >= 0 )
	Ct = B;
    else
	Ct = G;

    /* Ambient */
    CoCopy( &s->mat->Ca, &C );
    CoFilter( &C, &Ct, &C );
    CoCopy( &s->mat->Cd, &Cd );
    CoFilter( &Cd, &Ct, &Cd );

    IlluminateBegin( s->mat->lights );
	while( Illuminate( P, &L, &Cl ) ) {
	    Pt3Unit( &L, &L );
	    Diffuse( &N, &L, &Cd, &Cl, &C );
	    Specular( I, &N, &L, s->mat->shininess, &s->mat->Cs, &Cl, &C );
	}
    IlluminateEnd();
    return C;
}
