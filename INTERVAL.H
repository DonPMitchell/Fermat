#ifndef INTERVALDEF
#define INTERVALDEF
/*
 *	Interval-arithmetic & automatic differentiation routines
 *	D. P. Mitchell  91/12/13.
 */

#define DIM	4		/* maximum number of dimensions */
#define INFINITY	1.0e6
#define MACHEPS	0.00002
#define PI	3.14159265358979323844
#define setmin(C, A) { if((A) < (C)) (C) = (A); }
#define setmax(C, A) { if((A) > (C)) (C) = (A); }
#define add(C, A, B) { (C).lo=(A).lo+(B).lo; (C).hi=(A).hi+(B).hi; }
#define sub(C, A, B) { double tmp; tmp=(A).lo-(B).hi; (C).hi=(A).hi-(B).lo; (C).lo=tmp; }
#define r2i(C,R) { (C).lo = (C).hi = (R); }

typedef struct Interval {		/* interval function values */
	double	lo;
	double	hi;
} Interval;

typedef struct Gradient {		/* function/gradient values */
	double	f;
	double	dfdx[DIM];
} Gradient;

typedef struct Hessian {       /* interval function/gradient/hessian */
	Gradient g;
	double ddfdxdx[DIM][DIM];
} Hessian;

typedef struct IntervalGradient {	/* interval function/gradient values */
	Interval f;
	Interval dfdx[DIM];
} IntervalGradient;

typedef struct IntervalDirectional {	/* interval directional derivative */
	Interval f;
	Interval dfdt;
} IntervalDirectional;

typedef struct IntervalHessian {       /* interval function/gradient/hessian */
	IntervalGradient g;
	Interval ddfdxdx[DIM][DIM];
} IntervalHessian;

typedef double Matrix[DIM][DIM];
typedef double Vector[DIM];
typedef Interval IntervalMatrix[DIM][DIM];
typedef Interval IntervalVector[DIM];

/*
 *	macros for automatic differentiation
 */
static Interval itmp1, itmp2, itmp3, itmp4;

#define GMULT(W,I,U,V) {(W).dfdx[I] = (U).f*(V).dfdx[I] + (V).f*(U).dfdx[I];}
#define HMULT(W,I,J,U,V) { (W).ddfdxdx[I][J] = (W).ddfdxdx[J][I] = \
	(U).ddfdxdx[I][J]*(V).g.f + (U).g.dfdx[I]*(V).g.dfdx[J] +   \
	(U).g.dfdx[J]*(V).g.dfdx[I] + (U).g.f*(V).ddfdxdx[I][J]; }

#ifdef XXX
#define GDIV(W,I,U,V,T) \
	(W).dfdx[I] = ((U).dfdx[I] - (T).f*(V).dfdx[I})/(V).f
#define HDIV(W,I,J,U,V,T) \
	(W).ddfdxdx[I][J] = (W).ddfdxdx[J][I] = 
	    ((U).ddfdxdx[I][J] - \
	       (T).g.dfdx[J]*(V).dfdx[I}-(T).g.dfdx[I]*(U).g.dfdx[J] + \
		  (T).f*(V).dfdxdx[I][J])/(V).f
#endif

#define GSQR(W,I,U) {(W).dfdx[I] = 2.0*(U).dfdx[I]*(U).f;}
#define HSQR(W,I,J,U) { (W).ddfdxdx[I][J] = (W).ddfdxdx[J][I] = \
	2.0*((U).g.dfdx[I]*(U).g.dfdx[J] + (U).ddfdxdx[I][J]*(U).g.f); }

#define GSQRT(W,I,U) (W).dfdx[I] = (U).dfdx[I] / (2 * (W).f)
#define HSQRT(W,I,J,U) \
        (W).ddfdxdx[I][J] = (W).ddfdxdx[J][I] = \
	    ((U).ddfdxdx[I][J]*(W).g.f + \
	    (U).g.dfdx[J]*(W).g.dfdx[I] - \
	    2*(U).g.dfdx[I]*(W).g.dfdx[J]) / (2*(U).g.f)
	

#define IGMULT(W,I,U,V) { mult(&itmp1, &(U).f, &(V).dfdx[I]); \
			mult(&itmp2, &(V).f, &(U).dfdx[I]); \
			add((W).dfdx[I], itmp1, itmp2); }
#define IHMULT(W,I,J,U,V) { mult(&itmp1, &(U).ddfdxdx[I][J], &(V).g.f);  \
			mult(&itmp2, &(U).g.dfdx[I], &(V).g.dfdx[J]);  \
			mult(&itmp3, &(U).g.dfdx[J], &(V).g.dfdx[I]);  \
			mult(&itmp4, &(U).g.f, &(V).ddfdxdx[I][J]);  \
			add(itmp1, itmp1, itmp2);  \
			add(itmp2, itmp3, itmp4);  \
			add(itmp1, itmp1, itmp2);  \
			(W).ddfdxdx[I][J] = (W).ddfdxdx[J][I] = itmp1; }

#define IGSQR(W,I,U) { mult(&itmp1, &(U).f, &(U).dfdx[I]); \
		       add((W).dfdx[I], itmp1, itmp1); }
#define IHSQR(W,I,J,U) { mult(&itmp1, &(U).ddfdxdx[I][J], &(U).g.f); \
			mult(&itmp2, &(U).g.dfdx[I], &(U).g.dfdx[J]);  \
			add(itmp1, itmp1, itmp1); \
			add(itmp2, itmp2, itmp2); \
			add(itmp1, itmp1, itmp2); \
			(W).ddfdxdx[I][J] = (W).ddfdxdx[J][I] = itmp1; }

#define IGSQRT(W,I,U) { \
	add(itmp1, (W).f, (W).f); \
	div(&(W).dfdx[I], &(U).dfdx[I], &itmp1); }

#define IHSQRT(W,I,J,U)  { \
	mult(&itmp1, &(U).ddfdxdx[I][J], &(W).g.f); \
	mult(&itmp2, &(U).g.dfdx[J], &(W).g.dfdx[I]); \
	mult(&itmp3, &(U).g.dfdx[I], &(W).g.dfdx[J]); \
	add( itmp3, itmp3, itmp3 ); \
	add( itmp4, itmp1, itmp2 ); \
	sub( itmp4, itmp4, itmp3 ); \
	add( itmp1, (U).g.f, (U).g.f ); \
	div( &itmp4, &itmp4, &itmp1 ); \
	(W).ddfdxdx[I][J] = (W).ddfdxdx[J][I] = itmp4; }
/*
 *	bytecode for virtual stack machine
 */
typedef double Bytecode;

#define RET	0
#define LDX	1
#define ADD	2
#define SUB	3
#define NEG	4
#define DUP	5
#define MUL	6
#define NUM	7
#define SQR	8
#define STX	9
#define LDA	10
#define STA	11
#define LDB     12
#define STB     13
#define SQRT	14

/*
 *	interval-analysis routines
 */
extern void mult();
extern void absval();
extern void square();

extern double scalarMachine();
extern void gradientMachine();
extern void hessianMachine();
extern void intervalGradientMachine();
extern void intervalHessianMachine();
extern void intervalDirectionalMachine();

#endif
