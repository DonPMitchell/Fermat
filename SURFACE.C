#include <math.h>
#include <stdio.h>
#include "surface.h"

#define R2 0.707
/*
Bytecode  monkey[] = {
	LDX,2,NUM, R2,MUL, LDX,1,NUM,-R2,MUL, ADD, STA,
	LDX,2,NUM, R2,MUL, LDX,1,NUM, R2,MUL, ADD, STB,
	LDA,
	LDX,0,SQR,LDX,0,MUL,SUB,
	LDX,0,LDB,SQR,NUM,3,MUL,SUB,
	RET
};
*/
Bytecode  monkey[] = {
	LDX,2,
	LDX,0,SQR,LDX,0,MUL,SUB,
	LDX,0,LDX,2,SQR,NUM,3,MUL,SUB,
	RET
};

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

Bytecode steiner[] = {
	LDX,0,SQR,LDX,1,SQR,MUL,
	LDX,1,SQR,LDX,2,SQR,MUL,ADD,
	LDX,2,SQR,LDX,0,SQR,MUL,ADD,
	LDX,0,LDX,1,LDX,2,MUL,MUL,ADD,
	RET,
};

/*
Bytecode  cuboid[] = {
	LDX,0.,SQR,SQR,
	LDX,1.,SQR,SQR,
	LDX,2.,SQR,SQR,
	ADD,ADD,
	LDX,0.,SQR,
	LDX,1.,SQR,
	LDX,2.,SQR,
	ADD,ADD,
	SUB,
	NUM, 0.01,
	SUB,
	RET,
};
*/
Bytecode cuboid[] = {
	LDX,0,SQR,DUP,NUM,1,SUB,MUL,
	LDX,1,SQR,DUP,NUM,1,SUB,MUL,
	LDX,2,SQR,DUP,NUM,1,SUB,MUL,
	ADD, ADD,
	RET
};


/*
Bytecode sphere[] = {
	LDX,0.,SQR,
	LDX,1.,SQR,
	LDX,2.,SQR,
	ADD,ADD,
	STA, LDA, STB, LDB,
	NUM,2.0,SUB,
	RET,
};
*/

Bytecode sphere[] = {
	LDX,0.,NUM,0.,SUB,SQR,
	LDX,1.,NUM,0.,SUB,SQR,
	LDX,2.,NUM,0.,SUB,SQR,
	ADD,ADD,
	NUM,1.0,SUB,
	RET,
};

#define X1 1
#define Y1 7
#define Z1 13
#define X2 25
#define Y2 31
#define Z2 37
#define R  50
Bytecode ellipse[] = {
	NUM,0,LDX,0,SUB,SQR,
	NUM,0,LDX,1,SUB,SQR,
	NUM,0,LDX,2,SUB,SQR,
	ADD, ADD, DUP, STA,
	LDX,0,NUM,0,SUB,SQR,
	LDX,1,NUM,0,SUB,SQR,
	LDX,2,NUM,0,SUB,SQR,
	ADD, ADD, DUP, STB,
	SUB, SQR,
	LDA, LDB, ADD, NUM, 0, SQR, DUP, STA, NUM, 2., MUL, MUL, SUB,
	LDA, SQR, ADD, NEG,
	RET
};

Surface *
SurfaceCreate( Bytecode *code, int n, Material *m )
{
    Surface *s = (Surface *)malloc(sizeof(Surface)+n*sizeof(Bytecode));

    s->mat = m;
    s->code = (Bytecode *)((char *)s + sizeof(Surface));
    bcopy( code, s->code, n*sizeof(Bytecode) );

    return s;
}

int
SurfaceIntersect(Surface *s, Ray *ray, double *t, Point3 *P, Point3 *N )
{
    if( intervalIntersect( s->code, &ray->o, &ray->d, 0.01, 20., t ) ) {
	RayHit( ray, *t, P );
	implicitGrad( s->code, P, N );
	return 1;
    }
    return 0;
}


Surface *
SphereCreate(Point3 *c, float r, Material *m)
{
    Surface *s;

    s = SurfaceCreate( sphere, sizeof(sphere)/sizeof(Bytecode), m );
    s->code[ 3] = c->x;
    s->code[ 9] = c->y;
    s->code[15] = c->z;
    s->code[21] = r;

    return s;
}

Surface *
EllipseCreate( Point3 *f1, Point3 *f2, float r, Material *m )
{
    Surface *s;

    s = SurfaceCreate( ellipse, sizeof(ellipse)/sizeof(Bytecode), m );
    s->code[X1] = f1->x;
    s->code[Y1] = f1->y;
    s->code[Z1] = f1->z;
    s->code[X2] = f2->x;
    s->code[Y2] = f2->y;
    s->code[Z2] = f2->z;
    s->code[ R] = r;

    return s;
}

Surface *
TorusCreate( float rmajor, float rminor, Material *m )
{
    Surface *s = SurfaceCreate( torus, sizeof(torus)/sizeof(Bytecode), m );

    s->code[1] = rmajor;
    s->code[5] = rminor;

    return s;
}

Surface *
MonkeyCreate( Material *m )
{
    return SurfaceCreate( monkey, sizeof(monkey)/sizeof(Bytecode), m );
}
Surface *
CuboidCreate( Material *m )
{
    return SurfaceCreate( cuboid, sizeof(cuboid)/sizeof(Bytecode), m );
}
Surface *
SteinerCreate( Material *m )
{
    return SurfaceCreate( steiner, sizeof(steiner)/sizeof(Bytecode), m );
}
   
   
/* y = -1.5 */
int
FloorIntersect( Ray *r, double *t, Point3 *P, Point3 *N )
{
    if( r->d.y != 0 ) {
	*t = (-1.5 - r->o.y) / r->d.y;
	if( *t > 0.001 ) {
	    RayHit( r, *t, P );
	    N->x = 0.;
	    N->y = 1.;
	    N->z = 0.;
	    return 1;
	}
    }
    return 0;
}
