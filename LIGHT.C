#include <stdio.h>
#include <math.h>
#include "interval.h"
#include "light.h"
#include "ray.h"
#include "wavefront.h"

#define MAXLIGHTS 16
static Light light[MAXLIGHTS];
static int nlights = 0;

static Light vlight[MAXLIGHTS];
static int nvlights = 0;

static int lights;
static int nl, nv;

extern Color copper;		/* kludge */
void
LightCreate( Point3 *P, Color *C, int (*illum)(), void *data )
{
    light[nlights].P = *P;
    if( illum == DistantLight || illum == VirtualDistantLight )
	Pt3Unit( &light[nlights].P, &light[nlights].P );

    light[nlights].C = *C;
    light[nlights].illum = illum;
    light[nlights].data = data;
    nlights++;
}

void
IlluminateBegin( int mask )
{
    nl = 0;
    nv = nvlights = 0;
    lights = mask;
}

void
IlluminateEnd()
{
}

int
Illuminate( Point3 *P, Point3 *L, Color *C )
{
    while( nl++ < nlights ) {
	if( lights & (1 << (nl-1)) ) {
	    if ( (light[nl-1].illum)(&light[nl-1], P, L, C ) )
		return 1;
	}
    }
    while( nv++ < nvlights ) {
	if ( (vlight[nv-1].illum)(&vlight[nv-1], P, L, C ) )
	    return 1;
    }
    return 0;
}

int
PointLight( Light *light, Point3 *P, Point3 *L, Color *Cl )
{
    if( !Shadow( P, &light->P ) ) {
	Pt3Sub( &light->P, P, L );
	CoCopy( &light->C, Cl );
	return 1;
    }
    return 0;
}

int
DistantLight( Light *light, Point3 *P, Point3 *L, Color *Cl )
{
    if( !ShadowD( P, &light->P ) ) {
	Pt3Copy( &light->P, L );
	CoCopy(  &light->C, Cl );
	return 1;
    }
    return 0;
}

#define EX1 1
#define EY1 7
#define EZ1 13
#define EX2 24
#define EY2 30
#define EZ2 36
static Bytecode ellipse[] = {
	NUM,0,LDX,0,SUB,SQR,
	NUM,0,LDX,1,SUB,SQR,
	NUM,0,LDX,2,SUB,SQR,
	ADD, ADD, SQRT,
	LDX,0,NUM,0,SUB,SQR,
	LDX,1,NUM,0,SUB,SQR,
	LDX,2,NUM,0,SUB,SQR,
	ADD, ADD, SQRT,
	ADD,
	RET
};

#define PX1 1
#define PY1 7
#define PZ1 13
#define PX2 24
#define PY2 29
#define PZ2 34
Bytecode parabola[] = {
	NUM,0,LDX,0,SUB,SQR,
	NUM,0,LDX,1,SUB,SQR,
	NUM,0,LDX,2,SUB,SQR,
	ADD, ADD, SQRT,
	LDX,0,NUM,0,MUL,
	LDX,1,NUM,0,MUL,
	LDX,2,NUM,0,MUL,
	ADD, ADD, SUB,
	RET
};

static Bytecode *fcode, *ecode;

static void
func( double F[], double x[], int n )
{
    Gradient df, de;

    gradientMachine( fcode, x, 3, &df );
    gradientMachine( ecode, x, 3, &de );

    F[0] = df.dfdx[0] + x[3]*de.dfdx[0];
    F[1] = df.dfdx[1] + x[3]*de.dfdx[1];
    F[2] = df.dfdx[2] + x[3]*de.dfdx[2];
    F[3] = df.f;
}

/* r = a - b * c */
static Interval
comb( Interval *a, Interval *b, Interval *c )
{
    Interval r;

    mult( &r, b, c );
    add( r, *a, r );
    return r;
}

#define COMB(DF,DE,I) comb(&(DF).g.dfdx[I],&x[3],&(DE).g.dfdx[I])
#define DCOMB(DF,DE,I,J) comb(&(DF).ddfdxdx[I][J],&x[3],&(DE).ddfdxdx[I][J])

static void
dfunc( IntervalGradient F[], Interval x[], int n )
{
    int i, j;
    IntervalHessian ddf, dde;

    intervalHessianMachine( fcode, x, 3, &ddf );
    intervalHessianMachine( ecode, x, 3, &dde );

    F[3].f = ddf.g.f;
    F[3].dfdx[3].lo = 0.;
    F[3].dfdx[3].hi = 0.;
    for( i=0; i<3; i++ ) {
	F[i].f = COMB(ddf,dde,i);
	for( j=0; j<3; j++ )
	    F[i].dfdx[j] = DCOMB(ddf,dde,i,j);
	F[i].dfdx[3] = dde.g.dfdx[i];
	F[3].dfdx[i] = ddf.g.dfdx[i];
    }
}

int
VirtualLight( Light *light, Point3 *P, Point3 L[], Bytecode *lcode )
{
    Interval X[4];
    IntervalGradient F[4];
    int i, nroots;
    double R[32][DIM];
    Surface *s;

    X[0].lo = X[1].lo = X[2].lo = -1.12;
    X[0].hi = X[1].hi = X[2].hi =  1.11;
    X[2].lo = 0.25; /* Assume the light source is along +x */
    X[3].lo = 0.50;
    X[3].hi = 20.0;

    /* Approximate intensity distribution */

    s = (Surface*) light->data;
    ecode = lcode;
    fcode = s->code;
    nroots = multiSolve( R, func, dfunc, X, 4 );
	/*
	 *	take care of the +X side also
	 */
/*
	X[0].lo = X[1].lo = X[2].lo = -1.12;
	X[0].hi = X[1].hi = X[2].hi =  1.11;
	X[3].lo = 0.01;
	X[3].hi = 20.0;
	X[0].lo = 0.25;
	X[3].hi = 0.2499999;
	nroots += multiSolve( &R[nroots], func, dfunc, X, 4);
*/

    for( i=0; i<nroots; i++ ) {
	L[i].x = R[i][0];
	L[i].y = R[i][1];
	L[i].z = R[i][2];
    }
    return nroots;
}

extern void Check( Surface *s, Point3 *L, Point3 *R, Point3 *P );

int
VirtualPointLight( Light *light, Point3 *P, Point3 *L, Color *Cl )
{
    Wavefront *w;
    Point3 D, H[10];
    int i, n;
    extern int wavetrace;

    ellipse[EX1] = P->x;
    ellipse[EY1] = P->y;
    ellipse[EZ1] = P->z;
    ellipse[EX2] = light->P.x;
    ellipse[EY2] = light->P.y;
    ellipse[EZ2] = light->P.z;

    n = VirtualLight( light, P, H, ellipse );
    for( i=0; i<n; i++ ) {
	vlight[nvlights].P = H[i];
	if( PointLight( light,
		&vlight[nvlights].P, L, &vlight[nvlights].C ) ) {
	    Check( (Surface *)light->data, L, &H[i], P );
	    if( wavetrace ) {
		Pt3Sub( &vlight[nvlights].P, &light->P, &D );
		w = WaveSphere( &D );
		w = WaveTransfer( w, 
			Pt3Distance( &vlight[nvlights].P, &light->P ) );
		w = WaveReflect( w, (Surface *)light->data, &H[i] );
		w = WaveTransfer( w, Pt3Distance( &H[i], P ) );
		printf("Intensity = %g\n", 2*WaveIntensity(w) );
		CoScale(2*WaveIntensity(w),
		    &vlight[nvlights].C,&vlight[nvlights].C);
	    }
	    vlight[nvlights].illum = PointLight;
	    nvlights++;
	}
    }
    return PointLight( light, P, L, Cl );
}

int
VirtualDistantLight( Light *light, Point3 *P, Point3 *L, Color *Cl )
{
    Wavefront *w;
    Point3 H[64];
    Point3 D;
    int i, n;
    extern int wavetrace;

    parabola[PX1] = P->x;
    parabola[PY1] = P->y;
    parabola[PZ1] = P->z;
    parabola[PX2] = light->P.x;
    parabola[PY2] = light->P.y;
    parabola[PZ2] = light->P.z;
/*
    printf( "Checking for virtual lights between %g %g %g -> %g %g %g\n",
	light->P.x, light->P.y, light->P.z,
	P->x, P->y, P->z );
*/

    n = VirtualLight( light, P, H, parabola );
    for( i=0; i<n; i++ ) {
	vlight[nvlights].P = H[i];
	if( DistantLight(light,&vlight[nvlights].P,L,&vlight[nvlights].C)) {
	    Check( (Surface *)light->data, L, &H[i], P );
	    if( wavetrace ) {
		Pt3Neg( &light->P, &D );
		w = WavePlane( &D );
		w = WaveReflect( w, (Surface *)light->data, &H[i] );
		/* printf( "S: ru = %g, rv = %g \n", 1/w->ku, 1/w->kv ); */
		/* printf( "S->P: %g \n", Pt3Distance( &H[i], P ) ); */
		w = WaveTransfer( w, Pt3Distance( &H[i], P ) );
		/* printf( "P: ru = %g, rv = %g \n", 1/w->ku, 1/w->kv ); */

		CoFilter(&copper, &vlight[nvlights].C,&vlight[nvlights].C);
		CoScale(WaveIntensity(w),
		    &vlight[nvlights].C,&vlight[nvlights].C);
	    }
	    vlight[nvlights].illum = PointLight;
	    nvlights++;
	}
    }
    return DistantLight( light, P, L, Cl );
}

static void Check( Surface *s, Point3 *L, Point3 *S, Point3 *P )
{
    Point3 N;
    Point3 I, R, H;
    extern Point3 E;
    float v[2];
    extern int interactive;

/*
printf( "%g %g %g   %g %g %g   %g %g %g\n",
	       L->x, L->y, L->z,
	       S->x, S->y, S->z,
	       P->x, P->y, P->z );
*/
#ifdef GL
if(interactive) {
bgnline();
    R.x = S->x + 4*L->x;
    R.y = S->y + 4*L->y;
    R.z = S->z + 4*L->z;
    glscreen( P, &v[0], &v[1] ); v2f(v);
    glscreen( S, &v[0], &v[1] ); v2f(v);
    glscreen(&R, &v[0], &v[1] ); v2f(v);
endline();
}
#endif
/*
printf( "Virtual: L(%g,%g,%g) -> S(%g,%g,%g) -> P(%g,%g,%g) -> E(%g.%g,%g)\n",
	       L->x, L->y, L->z,
	       S->x, S->y, S->z,
	       P->x, P->y, P->z,
	       E.x,  E.y,  E.z );
*/
    implicitGrad( s->code, S, &N );
    Pt3Unit( &N, &N );
    /*
    Pt3Sub( S, L, &I );
    */
    Pt3Copy( L, &I );
    Pt3Unit( &I, &I );
    Pt3Sub( S, P, &R );
    Pt3Unit( &R, &R );

    Pt3Cross( &R, &I, &H );
/*
    printf( "cosi = %g, cosr = %g, plane=%g\n", 
	Pt3Dot( &N, &R ), Pt3Dot( &N, &R ), Pt3Dot( &N, &H ) );
*/
}
