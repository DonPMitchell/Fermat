#include <math.h>
#include "curvature.h"
#include "interval.h"


/* 
 * Given the two principal curvatures, and an angle (c=cos(a)),
 * Find normal curvatures at that angle.
 *
 * x' =  x c + y s
 * y' = -x s + y c
 */
void
Euler( float k1, float k2, float c, float s, float *ku, float *kv, float *kuv )
{
    float c2, s2, cs;

    /* c == cos(a); s = sin(a) */
    c2 = c*c;
    s2 = s*s;
    cs = c*s;

    *ku = k1 * c2 + k2 * s2;
    *kv = k1 * s2 + k2 * c2;
    *kuv = (k1 - k2) * cs;
}

void
EulerInv( float ku, float kv, float kuv, float *k1, float *k2, 
    float *cosa, float *sina )
{
    float a, c, s, c2, s2, cs;
    float lu, lv, luv;

    if( kuv == 0. ) {
	*cosa = 1.;
	*sina = 0.;
	*k1 = ku;
	*k2 = kv;
    }
    else {
	a = atan2(-2*kuv,kv-ku)/2;
	c = *cosa = cos(a);
	s = *sina = sin(a);
	c2 = c*c;
	s2 = s*s;
	cs = c*s;

	*k1 = ku*c2 + 2*kuv*cs + kv*s2;
	*k2 = ku*s2 - 2*kuv*cs + kv*c2;

	/*
	Euler( *k1, *k2, c, s, &lu, &lv, &luv );
	printf( "%g==%g, %g==%g, %g==%g\n", ku, lu, kv, lv, kuv, luv );
	*/
    }
}

static float
curvature( double Q[3][3], float s[3], float t[3] )
{
    int i, j;
    float c;

    c = 0;
    for( i=0; i<3; i++ )
	for( j=0; j<3; j++ )
	    c += s[i] * Q[i][j] * t[j];
    return -c;
}

void
SurfaceCurvature( 
    Bytecode *code, Point3 *P, Point3 *Pu, Point3 *Pv,
    float *ku, float *kv, float *kuv )
{
    int i, j, k;
    Hessian F;
    double L, M, x[3], Q[3][3], V[3][3], H[3][3];

    x[0] = P->x;
    x[1] = P->y;
    x[2] = P->z;
    hessianMachine( code, x, 3, &F );
#ifdef DEBUG
    printf( "f = %g \n", F.g.f );
    for( i=0; i<3; i++ )
	printf( "df/dx[%d] = %g \n", i, F.g.dfdx[i] );
    for( i=0; i<3; i++ )
	for( j=0; j<3; j++ )
	    printf( "ddf/dx[%d][%d] = %g \n", i, j, F.ddfdxdx[i][j] );
#endif

    L = 0;
    for( i=0; i<3; i++ )
	L += F.g.dfdx[i] * F.g.dfdx[i];
    M = L * sqrt(L);

    for( i=0; i<3; i++ ) {
	for( j=0; j<3; j++ ) {
	    V[i][j] = -F.g.dfdx[i] * F.g.dfdx[j];
	    if( i == j )
		V[i][j] += L;
	    H[i][j] = F.ddfdxdx[i][j];
	}
    }

    for( i=0; i<3; i++ )
	for( j=0; j<3; j++ ) {
	    Q[i][j] = 0;
	    for( k=0; k<3; k++ )
		Q[i][j] += V[i][k] * H[k][j];
	    Q[i][j] /= M;
	}
    
    *ku = curvature( Q, (float *)Pu, (float *)Pu );
    *kv = curvature( Q, (float *)Pv, (float *)Pv );
    *kuv = curvature( Q, (float *)Pu, (float *)Pv );
}

float
SurfaceKMCurvature( 
    Bytecode *code, Point3 *P, Point3 *Pu, Point3 *Pv,
    float *k1, float *k2, float *cosa, float *sina )
{
    float ku, kv, kuv;

    SurfaceCurvature( code, P, Pu, Pv, &ku, &kv, &kuv );

    EulerInv( ku, kv, kuv, k1, k2, cosa, sina );

    return (*k1) * (*k2);
}
