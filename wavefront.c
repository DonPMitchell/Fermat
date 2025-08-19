#include <math.h>
#include "curvature.h"
#include "wavefront.h"

static Wavefront w;

Wavefront *
WavePlane( Point3 *D )
{
    Pt3Unit( D, &w.N );
    w.ku = 0.;
    w.kv = 0.;

    return &w;
}

Wavefront *
WaveSphere( Point3 *D )
{
    Pt3Unit( D, &w.N );
    w.ku = INFINITY;
    w.kv = INFINITY;

    return &w;
}

float
WaveIntensity( Wavefront *w )
{
    return fabs(w->ku * w->kv);
}

Wavefront *
WaveTransfer( Wavefront *w, float s )
{
    if( w->ku == INFINITY )
	w->ku = -1/s;
    else if( w->ku*s == 1. )
	w->ku = INFINITY;
    else
	w->ku /= (1-s*w->ku);

    if( w->kv == INFINITY )
	w->kv = -1/s;
    else if( w->kv*s == 1. )
	w->kv = INFINITY;
    else
	w->kv /= (1-s*w->kv);

    return w;
}

Wavefront *
WaveRefract( Wavefront *w, Surface *s, Point3 *P, float eta )
{
    float cosi, cost;
    float gam;
    float cosu, sinu;
    float ku1, kv1, kuv1;
    float ku2, kv2, kuv2;
    float ku, kv, kuv;
    Point3 U, V, N;

    implicitGrad( s->code, P, &N );

    cosi = -Pt3Dot( &w->N, &N );
    cost = 1.0 - eta * eta*(1.0 - cosi*cosi);
    if (cost < 0.0)
        return 0;
    cost = sqrt(cost);
    gam = eta*cosi - cost;

    /* Establish surface coordinate system */
    Pt3Cross( &w->N, &N, &U );
    Pt3Unit( &U, &U );
    Pt3Cross( &N, &U, &V );

    /* Rotate principal curvatures of incident */
    cosu = Pt3Dot( &w->U, &U );
    sinu = Pt3Dot( &w->V, &V );
    Euler( w->ku, w->kv, cosu, sinu, &ku1, &kv1, &kuv1 ); 

    /* Evaluate curvatures of refracting surface */
    SurfaceCurvature( s->code, P, &U, &V, &ku2, &kv2, &kuv2 ); 

    ku = eta*ku1 + gam*ku2;
    kuv = (cosi/cost)*(eta*kuv1 + (gam/cosi)*kuv2);
    kv = ((cosi*cosi)/(cost*cost))*(eta*kv1 + (gam/(cosi*cosi))*kv2);

    /* Compute new principal curvatures */
    EulerInv( ku, kv, kuv, &w->ku, &w->kv, &cosu, &sinu );
    Pt3Comb(  cosu, &U, sinu, &V, &w->U );
    Pt3Comb( -sinu, &U, cosu, &V, &w->V );

    return w;
}

Wavefront *
WaveReflect( Wavefront *w, Surface *s, Point3 *P )
{
    float cosi;
    float cosu, sinu;
    float ku1, kv1, kuv1;
    float ku2, kv2, kuv2;
    float ku, kv, kuv;
    Point3 U, V, N;
    float t;

    /* Establish surface coordinate system */
    implicitGrad( s->code, P, &N );
    Pt3Unit( &N, &N );
    cosi = Pt3Dot( &w->N, &N );
    if( cosi < 0 ) {
	cosi = -cosi;
	Pt3Neg( &N, &N ); /* s->N is in the direction of w->N! */
    }

    Pt3Cross( &w->N, &N, &U );
    Pt3Unit( &U, &U );
    Pt3Cross( &N, &U, &V );

    /* Rotate principal curvatures of incident wave
    cosu = Pt3Dot( &w->U, &U );
    sinu = Pt3Dot( &w->U, &V );
    */
    cosu = 1;
    sinu = 0;
    Euler( w->ku, w->kv, cosu, sinu, &ku1, &kv1, &kuv1 ); 
#ifdef DEBUG
    printf( "wave.ku = %g, wave.kv = %g, wave.kuv = %g\n", ku1, kv1, kuv1 );
#endif

    /* Evaluate curvatures of reflecting surface */
    SurfaceCurvature( s->code, P, &U, &V, &ku2, &kv2, &kuv2 ); 
#ifdef DEBUG
    printf( "surf.ku = %g, surf.kv = %g, surf.kuv = %g\n", ku2, kv2, kuv2 );
#endif

    /* eta = -1, gam = 2*cosi */
    ku =   ku1 + (2*cosi)*ku2;
    kuv = -kuv1 - 2*kuv2;
    kv =   kv1 + (2/cosi)*kv2;
#ifdef DEBUG
    printf( "cosi=%g, refl.ku = %g, refl.kv = %g, refl.kuv = %g\n", 
       cosi, ku, kv, kuv );
#endif

    /* Compute new wavefront */
    EulerInv( ku, kv, kuv, &w->ku, &w->kv, &cosu, &sinu );
    /* Flip normal and curvatures, eta=1, gam=-2*cosi */
    Pt3Comb(1.0, &w->N, -2.0*cosi, &N, &w->N);
#ifdef DEBUG
    printf( "k1 = %g, k2 = %g, intensity = %g\n", w->ku, w->kv, w->ku*w->kv );
    Pt3Unit(&w->N,&w->N);
    Pt3Copy( &U, &w->U );
    Pt3Cross(&w->N, &w->U, &w->V );
    Pt3Comb(  cosu, &U,  sinu, &V, &w->U );
    Pt3Comb( -sinu, &U,  cosu, &V, &w->V );
#endif

    return w;
}
