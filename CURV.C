#include <stdio.h>
#include "surface.h"
#include "curvature.h"

Bytecode sphere[] = {
	LDX,0.,NUM,0.,SUB,SQR,
	LDX,1.,NUM,0.,SUB,SQR,
	LDX,2.,NUM,0.,SUB,SQR,
	ADD,ADD,
	NUM,1.0,SUB,
	RET,
};
Bytecode ellipse[] = {
	LDX,0.,SQR,NUM,1./1.,MUL,
	LDX,1.,SQR,NUM,1./4.,MUL,
	LDX,2.,SQR,NUM,1./9.,MUL,
	ADD,ADD,
	NUM,1.0,SUB,
	RET
};
#define TA 1
#define TB 5
Bytecode torus[] = {
	NUM,3.0,SQR,STA,
	NUM,1.0,SQR,STB,
	LDX,0.,SQR,
	LDX,1.,SQR,
	LDX,2.,SQR,
	ADD,ADD,
	LDA,SUB,
	LDB,SUB,
	SQR,
	NUM,4.0,LDA,MUL,
	LDB,LDX,2.0,SQR,SUB,MUL,
	SUB,
	RET
};

main( int ac, char *av[] )
{
    Point3 P, Pu, Pv;
    float ku, kv, kuv;
    float k1, k2, k;
    float cosu, sinu;
#define M_SQRT1_2	0.70710678118654752440

    torus[TA] = 2;
    torus[TB] = 1;
    Pu.x = 0.;
    Pu.y = M_SQRT1_2;
    Pu.z = M_SQRT1_2;
    Pv.x = 0.;
    Pv.y =  M_SQRT1_2;
    Pv.z = -M_SQRT1_2;

    P.x = 3.;
    P.y = 0.;
    P.z = 0.;
    SurfaceCurvature( torus, &P, &Pu, &Pv, &ku, &kv, &kuv );
    printf("ku=%g, kv=%g, kuv=%g\n", ku, kv, kuv );
    k = SurfaceKMCurvature( torus, &P, &Pu, &Pv, &k1, &k2, &cosu, &sinu );
    printf("k1=%g, k2=%g, k=%g, cosu = %g, sinu = %g\n",
        k1, k2, k, cosu, sinu );
    Pt3Comb( cosu, &Pu, sinu, &Pv, &P );
    printf( "k1 = %g, Pu1 = (%g, %g, %g)\n", k1, P.x, P.y, P.z );
    Pt3Comb(-sinu, &Pu, cosu, &Pv, &P );
    printf( "k2 = %g, Pu2 = (%g, %g, %g)\n", k2, P.x, P.y, P.z );

    P.x = 1.;
    P.y = 0.;
    P.z = 0.;
    SurfaceCurvature( torus, &P, &Pu, &Pv, &ku, &kv, &kuv );
    printf("ku=%g, kv=%g, kuv=%g\n", ku, kv, kuv );
    k = SurfaceKMCurvature( torus, &P, &Pu, &Pv, &k1, &k2, &cosu, &sinu );
    printf("k1=%g, k2=%g, k=%g, cosu = %g, sinu = %g\n",
        k1, k2, k, cosu, sinu );
    Pt3Comb( cosu, &Pu, sinu, &Pv, &P );
    printf( "k1 = %g, Pu1 = (%g, %g, %g)\n", k1, P.x, P.y, P.z );
    Pt3Comb(-sinu, &Pu, cosu, &Pv, &P );
    printf( "k2 = %g, Pu2 = (%g, %g, %g)\n", k2, P.x, P.y, P.z );
}
