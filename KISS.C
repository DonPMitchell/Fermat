/* given two points and a sphere, solve for the reflection point. */
#include <stdio.h>
#include <math.h>
#include "interval.h"
#define ROOT2 0.70710678118654752440

Bytecode sphere[] = {
	LDX,0.,SQR,
	LDX,1.,SQR,
	LDX,2.,SQR,
	ADD,ADD,
	NUM,1.0,SUB,
	RET
};
Bytecode old_cuboid[] = {
	LDX,0,SQR,SQR,
	LDX,1,SQR,SQR,
	LDX,2,SQR,SQR,
	ADD,ADD,
	LDX,0,SQR,
	LDX,1,SQR,
	LDX,2,SQR,
	ADD,ADD,
	SUB,
	NUM, 0.01, SUB,
	RET
};
Bytecode cuboid[] = {
	LDX,0,SQR,DUP,NUM,1,SUB,MUL,
	LDX,1,SQR,DUP,NUM,1,SUB,MUL,
	LDX,2,SQR,DUP,NUM,1,SUB,MUL,
	ADD, ADD,
	RET
};

#define X1 1
#define Y1 7
#define Z1 13
#define X2 24
#define Y2 30
#define Z2 36
Bytecode ellipse[] = {
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
Bytecode parabola[] = {			/* actually a paraboloid */
	NUM,0,LDX,0,SUB,SQR,
	NUM,0,LDX,1,SUB,SQR,
	NUM,0,LDX,2,SUB,SQR,
	ADD, ADD, SQRT,
	LDX,0,NUM,ROOT2,MUL,		/* light in (1,1,0) direction */
	LDX,1,NUM,ROOT2,MUL,
	ADD,SUB,
	RET
};

Bytecode *fcode, *ecode;

void
func( double F[], double x[], int n )
{
    Gradient df, de;

    gradientMachine( fcode, x, 3, &df );
    gradientMachine( ecode, x, 3, &de );

    F[0] = df.dfdx[0] + x[3]*de.dfdx[0];
    F[1] = df.dfdx[1] + x[3]*de.dfdx[1];
    F[2] = df.dfdx[2] + x[3]*de.dfdx[2];
    F[3] = df.f;

#ifdef DEBUG
    printf( "e = %g \n", de.f );
    printf( "f = %g \n", df.f );
    printf( "dfdx = %g; dedx = %g \n", df.dfdx[0], de.dfdx[0] );
    printf( "dfdy = %g; dedy = %g \n", df.dfdx[1], de.dfdx[1] );
    printf( "dfdz = %g; dedz = %g \n", df.dfdx[2], de.dfdx[2] );
#endif
}

/* r = a + b * c */
Interval
comb( Interval *a, Interval *b, Interval *c )
{
    Interval r;

    mult( &r, b, c );
    add( r, *a, r );
    return r;
}

#define COMB(DF,DE,I) comb(&(DF).g.dfdx[I],&x[3],&(DE).g.dfdx[I])
#define DCOMB(DF,DE,I,J) comb(&(DF).ddfdxdx[I][J],&x[3],&(DE).ddfdxdx[I][J])

void
dfunc( IntervalGradient F[], Interval x[], int n )
{
    int i, j;
    IntervalHessian ddf, dde;
	IntervalGradient df;

#ifdef DEBUG
printf("[%f %f] x [%f %f] x [%f %f] x [%f %f]\n",
  x[0].lo, x[0].hi, x[1].lo, x[1].hi, x[2].lo, x[2].hi, x[3].lo, x[3].hi);
#endif
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

#ifdef DEBUG
df = F[2];
    printf( "f = [%g,%g] \n", df.f.lo, df.f.hi );
    for( i=0; i<3; i++ ) {
	printf( "df/dx[%d] = [%g,%g] \n", i, df.dfdx[i].lo, df.dfdx[i].hi );
    }
#endif


}

main( int ac, char *av[] )
{
    IntervalVector X;
    IntervalGradient F[4];
    int i;
    int nroots;
    Vector R[10];


    if( ac == 7 ) {
	ellipse[X1] = atof(av[1]);
	ellipse[Y1] = atof(av[2]);
	ellipse[Z1] = atof(av[3]);
	ellipse[X2] = atof(av[4]);
	ellipse[Y2] = atof(av[5]);
	ellipse[Z2] = atof(av[6]);
    }
    else {
	fprintf( stderr,
	    "usage: %s <f1.x> <f1.y> <f1.z> <f2.x> <f2.y> <f2.z>\n", av[0] );
	exit(0);
    }

    X[0].lo = X[1].lo = X[2].lo = -1.2;
    X[0].hi = X[1].hi = X[2].hi =  1.25;
    X[3].lo =  1.0;
    X[3].hi = 5.0;

    fcode = cuboid;
    ecode = ellipse;

    nroots = multiSolve( R, func, dfunc, X, 4 );
    for( i=0; i<nroots; i++ )
       printf( "%g %g %g %g\n", R[i][0], R[i][1], R[i][2], R[i][3] );
}
