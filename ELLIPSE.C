#include <math.h>
#include <stdio.h>
#include "sphere.h"

#define X1 0
#define Y1 1
#define Z1 2
#define X2 3
#define Y2 4
#define Z2 5
#define W2 6
#define X3 7
#define Y3 8
#define Z3 9

Bytecode ellipse123[] = {
	LDX,X1,LDX,X2,SUB,SQR,
	LDX,Y1,LDX,Y2,SUB,SQR,
	LDX,Z1,LDX,Z2,SUB,SQR,
	LDX,X2,LDX,X3,SUB,SQR,
	LDX,Y2,LDX,Y3,SUB,SQR,
	LDX,Z2,LDX,Z3,SUB,SQR,
	DUP, STA,
	DUP, STB,
	SUB, SQR,
	LDA, LDB, ADD, LDX, W2, SQR, DUP, STA, NUM, 2., MUL, SUB,
	LDA, SQR, SUB,
	RET
};

Bytecode ellipse12[] = {
	LDX,X1,LDX,X2,SUB,SQR,
	LDX,Y1,LDX,Y2,SUB,SQR,
	LDX,Z1,LDX,Z2,SUB,SQR,
	LDX,X2,NUM,X3,SUB,SQR,
	LDX,Y2,NUM,Y3,SUB,SQR,
	LDX,Z2,NUM,Z3,SUB,SQR,
	DUP, STA,
	DUP, STB,
	SUB, SQR,
	LDA, LDB, ADD, LDX, W2, SQR, DUP, STA, NUM, 2., MUL, SUB,
	LDA, SQR, SUB,
	RET
};

Bytecode ellipse23[] = {
	NUM,X1,LDX,X2,SUB,SQR,
	NUM,Y1,LDX,Y2,SUB,SQR,
	NUM,Z1,LDX,Z2,SUB,SQR,
	LDX,X2,LDX,X3,SUB,SQR,
	LDX,Y2,LDX,Y3,SUB,SQR,
	LDX,Z2,LDX,Z3,SUB,SQR,
	DUP, STA,
	DUP, STB,
	SUB, SQR,
	LDA, LDB, ADD, LDX, W2, SQR, DUP, STA, NUM, 2., MUL, SUB,
	LDA, SQR, SUB,
	RET
};

Bytecode ellipse2[] = {
	NUM,X1,LDX,X2,SUB,SQR,
	NUM,Y1,LDX,Y2,SUB,SQR,
	NUM,Z1,LDX,Z2,SUB,SQR,
	LDX,X2,NUM,X3,SUB,SQR,
	LDX,Y2,NUM,Y3,SUB,SQR,
	LDX,Z2,NUM,Z3,SUB,SQR,
	DUP, STA,
	DUP, STB,
	SUB, SQR,
	LDA, LDB, ADD, LDX, W2, SQR, DUP, STA, NUM, 2., MUL, SUB,
	LDA, SQR, SUB,
	RET
};

Sphere *
SphereCreate(Point3 *c, float r, Material *m)
{
    int i;
    Sphere *sph = (Sphere *)malloc(sizeof(Sphere)+sizeof(sphere));

    sph->mat = m;
    sph->P = *c;
    sph->r = r;

    sph->code = (Bytecode *)((char *)sph + sizeof(Sphere));
    for( i=0; i<sizeof(sphere)/sizeof(Bytecode); i++ )
       sph->code[i] = sphere[i];
    sph->code[SPHX] = c->x;
    sph->code[SPHY] = c->y;
    sph->code[SPHZ] = c->z;
    sph->code[SPHR] = r;

    return sph;
}
   
int
SphereIntersect(Sphere *sph, Ray *ray, double *t, Point3 *N )
{
    /*
    int res = intersect( sph->code, ray, 0., 6., t, N );
    return res;
    */

    Point3 A, P;
    float b, d, s;

    Pt3Sub(&sph->P, &ray->o, &A);

    /*
     * Solve quadratic equation.
     */
    b = Pt3Dot( &A, &ray->d );
    d = b * b + sph->r*sph->r - Pt3Dot( &A, &A );
    if (d < 0.)
        return 0;

    d = sqrt(d);
    s = b - d;
    if (s > RAY_EPSILON) {
	RayHit( ray, s, &P );
	Pt3Sub( &P, &sph->P, N );
	*t = s;
        return 1;
    }
    s = b + d;
    if (s > RAY_EPSILON) {
	RayHit( ray, s, &P );
	Pt3Sub( &P, &sph->P, N );
	*t = s;
        return 1;
    }
    return 0;
}
