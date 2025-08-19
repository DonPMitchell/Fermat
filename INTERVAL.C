/*
 *	Interval-arithmetic & automatic differentiation routines
 *	D. P. Mitchell  91/12/13.
 */

#include "interval.h"
#include <stdio.h>

#define EPSILON  0.0001

extern double	zeroin();
extern double	sqrt();
extern double	fabs();

static Interval		izero;
static Interval		ione = {1.0, 1.0};
static Gradient		gzero;
static Hessian		hzero;
static IntervalGradient igzero;
static IntervalHessian	ihzero;
static IntervalDirectional idzero;
static double	intervalEpsilon;

static double	rayorg[DIM];
static double	raydir[DIM];
static Bytecode	*program;

void
mult(c, a, b)
register Interval *a, *b, *c;
{
	register double p, q;
	Interval *tmp;

	if (a->lo < 0.0 && b->lo >= 0.0) {
		tmp = a;
		a = b;
		b = tmp;
	}
	if (a->lo >= 0.0) {
		if (b->lo >= 0.0) {
			c->lo = a->lo * b->lo;
			c->hi = a->hi * b->hi;
			return;
		}
		c->lo = a->hi * b->lo;
		if (b->hi >= 0.0)
			c->hi = a->hi * b->hi;
		else
			c->hi = a->lo * b->hi;
		return;
	}
	if (a->hi > 0.0) {
		if (b->hi > 0.0) {
			p = a->lo * b->hi;
			q = a->hi * b->lo;
			c->lo = (p < q) ? p : q;
			p = a->lo * b->lo;
			q = a->hi * b->hi;
			c->hi = (p > q) ? p : q;
			return;
		}
		c->lo = a->hi * b->lo;
		c->hi = a->lo * b->lo;
		return;
	}
	c->hi = a->lo * b->lo;
	if (b->hi <= 0.0)
		c->lo = a->hi * b->hi;
	else
		c->lo = a->lo * b->hi;
}

void
div( c, a, b )
register Interval *a, *b, *c;
{
    Interval rec;

    if( b->lo > 0. || b->hi < 0. ) {
        rec.lo = 1./b->hi;
        rec.hi = 1./b->lo;
        mult( c, a, &rec );
    }
    else {
        *c = *a;
        fprintf( stderr, "Interval divide by 0.\n" );
    }
}

void
absval(c, a)
register Interval *a, *c;
{

	if (a->lo >= 0.0) {
		*c = *a;
	} else if (a->hi < 0.0) {
		c->lo = -a->hi;
		c->hi = -a->lo;
	} else {
		c->lo = 0.0;
		c->hi = -a->lo;
		setmax(c->hi, a->hi);
	}
}

void
square(c, a)
Interval *a;
register Interval *c;
{

	absval(c, a);
	c->lo = c->lo * c->lo;
	c->hi = c->hi * c->hi;
}

void
squareroot(c, a)
Interval *a, *c;
{
    Interval rec;

    if( a->lo > 0. ) {
        c->lo = sqrt(a->lo);
        c->hi = sqrt(a->hi);
    }
    else {
        *c = *a;
        fprintf( stderr, "Square root of a negative number.\n" );
    }
}

/*
 *	stack machine for finding value and gradient of F
 */
void
gradientMachine(code, x, N, grad)
register Bytecode *code;
double x[DIM];
int N;
Gradient *grad;
{
	register i;
	register Gradient *sp, *q1, *q2;
	Gradient stack[16];
	Gradient qtmp;
	Gradient X[DIM];
	Gradient a, b;

	sp = stack + 1;
	for (i = 0; i < N; i++) {
		X[i] = gzero;
		X[i].f = x[i];
		X[i].dfdx[i] = 1.0;
	}
	for (;;) {
	switch ((int)*code++) {

	case RET:
		*grad = *--sp;
		return;
	case LDX:
		i = *code++;
		*sp++ = X[i];
		continue;
	case STX:
		i = *code++;
		X[i] = *--sp;
		continue;
        case LDA:
                *sp++ = a;
                continue;
        case STA:
                a = *--sp;
                continue;
        case LDB:
                *sp++ = b;
                continue;
        case STB:
                b = *--sp;
                continue;
	case ADD:
		q1 = --sp;
		q2 = --sp;
		q2->f += q1->f;
		for (i = 0; i < N; i++)
			q2->dfdx[i] += q1->dfdx[i];
		sp++;
		continue;
	case SUB:
		q1 = --sp;
		q2 = --sp;
		q2->f -= q1->f;
		for (i = 0; i < N; i++)
			q2->dfdx[i] -= q1->dfdx[i];
		sp++;
		continue;
	case NEG:
		q1 = --sp;
		q1->f = - q1->f;
		for (i = 0; i < N; i++)
			q1->dfdx[i] = - q1->dfdx[i];
		sp++;
		continue;
	case DUP:
		qtmp = *--sp;
		sp++;
		*sp++ = qtmp;
		continue;
	case MUL:
		q1 = --sp;
		q2 = --sp;
		qtmp.f = q1->f * q2->f;
		for (i = 0; i < N; i++)
			GMULT(qtmp, i, *q1, *q2);
		*sp++ = qtmp;
		continue;
	case SQR:
		q1 = --sp;
		qtmp.f = q1->f * q1->f;
		for (i = 0; i < N; i++)
			GSQR(qtmp, i, *q1);
		*sp++ = qtmp;
		continue;
	case SQRT:
		q1 = --sp;
		if (q1->f >= 0.0) {
			qtmp.f = sqrt(q1->f);
			for (i = 0; i < N; i++)
				GSQRT(qtmp, i, *q1);
		}
		*sp++ = qtmp;
		continue;
	case NUM:
		qtmp = gzero;
		qtmp.f = *code++;
		*sp++ = qtmp;
		continue;
	default:
		printf("illegal bytecode in gradientMachine\n");
		exit(1);
	}}
}

/*
 *	Interval extension of function and gradient value
 */
void
intervalGradientMachine(code, x, N, igrad)
Bytecode *code;
Interval x[DIM];
int N;
IntervalGradient *igrad;
{
	register IntervalGradient *sp, *q1, *q2;
	register i;
	IntervalGradient stack[16];
	IntervalGradient qtmp;
	IntervalGradient X[DIM];
        IntervalGradient a, b;

	sp = stack + 1;
	for (i = 0; i < N; i++) {
		X[i] = igzero;
		X[i].f = x[i];
		X[i].dfdx[i] = ione;
	}
	for (;;) {
	switch ((int)*code++) {

	case RET:
		*igrad = *--sp;
		return;
	case LDX:
		i = *code++;
		*sp++ = X[i];
		continue;
	case STX:
		i = *code++;
		X[i] = *--sp;
		continue;
        case LDA:
                *sp++ = a;
                continue;
        case STA:
                a = *--sp;
                continue;
        case LDB:
                *sp++ = b;
                continue;
        case STB:
                b = *--sp;
                continue;
	case ADD:
		q1 = --sp;
		q2 = --sp;
		add(q2->f, q2->f, q1->f);
		for (i = 0; i < N; i++)
			add(q2->dfdx[i], q2->dfdx[i], q1->dfdx[i]);
		sp++;
		continue;
	case SUB:
		q1 = --sp;
		q2 = --sp;
subtract:	sub(qtmp.f, q2->f, q1->f);
		for (i = 0; i < N; i++)
			sub(qtmp.dfdx[i], q2->dfdx[i], q1->dfdx[i]);
		*sp++ = qtmp;
		continue;
	case NEG:
		q1 = --sp;
		q2 = &igzero;
		goto subtract;
	case DUP:
		qtmp = *--sp;
		sp++;
		*sp++ = qtmp;
		continue;
	case MUL:
		q1 = --sp;
		q2 = --sp;
		mult(&qtmp.f, &q1->f, &q2->f);
		for (i = 0; i < N; i++)
			IGMULT(qtmp, i, *q1, *q2);
		*sp++ = qtmp;
		continue;
	case SQR:
		q1 = --sp;
		square(&qtmp.f, &q1->f);
		for (i = 0; i < N; i++)
			IGSQR(qtmp, i, *q1);
		*sp++ = qtmp;
		continue;
        case SQRT:
                q1 = --sp;
                if( q1->f.lo >= 0. ) {
                    squareroot(&qtmp.f, &q1->f);
                    for (i = 0; i < N; i++)
                        IGSQRT(qtmp, i, *q1);
                }
                *sp++ = qtmp;
                continue;
	case NUM:
		qtmp = igzero;
		qtmp.f.lo = qtmp.f.hi = *code++;
		*sp++ = qtmp;
		continue;
	default:
		printf("illegal bytecode in intervalGradient\n");
		exit(1);
	}}
}

/*
 *	evaluate function, gradient and hessian
 */
void
hessianMachine(code, x, N, hess)
Bytecode *code;
double x[DIM];
int N;
Hessian *hess;
{
	register Hessian *sp, *q1, *q2;
	register i, j;
	Hessian stack[16];
	Hessian qtmp;
	Hessian X[DIM];
        Hessian a, b;

	sp = stack + 1;
	for (i = 0; i < N; i++) {
		X[i] = hzero;
		X[i].g.f = x[i];
		X[i].g.dfdx[i] = 1.;
	}
	for (;;) {
	switch ((int)*code++) {

	case RET:
		*hess = *--sp;
		return;
	case LDX:
		i = *code++;
		*sp++ = X[i];
		continue;
	case STX:
		i = *code++;
		X[i] = *--sp;
		continue;
        case LDA:
                *sp++ = a;
                continue;
        case STA:
                a = *--sp;
                continue;
        case LDB:
                *sp++ = b;
                continue;
        case STB:
                b = *--sp;
                continue;
	case ADD:
		q1 = --sp;
		q2 = --sp;
		q2->g.f += q1->g.f;
		for (i = 0; i < N; i++) {
			q2->g.dfdx[i] += q1->g.dfdx[i];
			for (j = 0; j < N; j++)
				q2->ddfdxdx[i][j] += q1->ddfdxdx[i][j];
		}
		sp++;
		continue;
	case SUB:
		q1 = --sp;
		q2 = --sp;
subtract:	qtmp.g.f = q2->g.f - q1->g.f;
		for (i = 0; i < N; i++) {
			qtmp.g.dfdx[i] = q2->g.dfdx[i] - q1->g.dfdx[i];
			for (j = 0; j < N; j++)
			    qtmp.ddfdxdx[i][j] =
				  q2->ddfdxdx[i][j] - q1->ddfdxdx[i][j];
		}
		*sp++ = qtmp;
		continue;
	case NEG:
		q1 = --sp;
		q2 = &hzero;
		goto subtract;
	case DUP:
		qtmp = *--sp;
		sp++;
		*sp++ = qtmp;
		continue;
	case MUL:
		q1 = --sp;
		q2 = --sp;
		qtmp.g.f =  q1->g.f * q2->g.f;
		for (i = 0; i < N; i++)
			GMULT(qtmp.g, i, q1->g, q2->g);
		for (i = 0; i < N; i++)
			for (j = 0; j <= i; j++)
				HMULT(qtmp, i, j, *q1, *q2);
		*sp++ = qtmp;
		continue;
	case SQR:
		q1 = --sp;
		qtmp.g.f = q1->g.f * q1->g.f;
		for (i = 0; i < N; i++)
			GSQR(qtmp.g, i, q1->g);
		for (i = 0; i < N; i++)
			for (j = 0; j <= i; j++)
				HSQR(qtmp, i, j, *q1);
		*sp++ = qtmp;
		continue;
        case SQRT:
                q1 = --sp;
                if( q1->g.f >= 0 ) {
		    qtmp.g.f = sqrt(q1->g.f);
                    for (i = 0; i < N; i++)
                            GSQRT(qtmp.g, i, q1->g );
                    for (i = 0; i < N; i++)
                            for (j = 0; j <= i; j++)
                                    HSQRT(qtmp, i, j, *q1 );
                }
                *sp++ = qtmp;
                continue;
	case NUM:
		qtmp = hzero;
		qtmp.g.f = *code++;
		*sp++ = qtmp;
		continue;
	default:
		printf("illegal bytecode in HessianMachine\n");
		exit(1);
	}}
}

/*
 *	evaluate interval extension of function, gradient and hessian
 */
void
intervalHessianMachine(code, x, N, ihess)
Bytecode *code;
Interval x[DIM];
int N;
IntervalHessian *ihess;
{
	register IntervalHessian *sp, *q1, *q2;
	register i, j;
	IntervalHessian stack[16];
	IntervalHessian qtmp;
	IntervalHessian X[DIM];
        IntervalHessian a, b;

	sp = stack + 1;
	for (i = 0; i < N; i++) {
		X[i] = ihzero;
		X[i].g.f = x[i];
		X[i].g.dfdx[i] = ione;
	}
	for (;;) {
	switch ((int)*code++) {

	case RET:
		*ihess = *--sp;
		return;
	case LDX:
		i = *code++;
		*sp++ = X[i];
		continue;
	case STX:
		i = *code++;
		X[i] = *--sp;
		continue;
        case LDA:
                *sp++ = a;
                continue;
        case STA:
                a = *--sp;
                continue;
        case LDB:
                *sp++ = b;
                continue;
        case STB:
                b = *--sp;
                continue;
	case ADD:
		q1 = --sp;
		q2 = --sp;
		add(q2->g.f, q2->g.f, q1->g.f);
		for (i = 0; i < N; i++) {
			add(q2->g.dfdx[i], q2->g.dfdx[i], q1->g.dfdx[i]);
			for (j = 0; j < N; j++)
				add(q2->ddfdxdx[i][j], q2->ddfdxdx[i][j], q1->ddfdxdx[i][j]);
		}
		sp++;
		continue;
	case SUB:
		q1 = --sp;
		q2 = --sp;
subtract:	sub(qtmp.g.f, q2->g.f, q1->g.f);
		for (i = 0; i < N; i++) {
			sub(qtmp.g.dfdx[i], q2->g.dfdx[i], q1->g.dfdx[i]);
			for (j = 0; j < N; j++)
				sub(qtmp.ddfdxdx[i][j], q2->ddfdxdx[i][j], q1->ddfdxdx[i][j]);
		}
		*sp++ = qtmp;
		continue;
	case NEG:
		q1 = --sp;
		q2 = &ihzero;
		goto subtract;
	case DUP:
		qtmp = *--sp;
		sp++;
		*sp++ = qtmp;
		continue;
	case MUL:
		q1 = --sp;
		q2 = --sp;
		mult(&qtmp.g.f, &q1->g.f, &q2->g.f);
		for (i = 0; i < N; i++)
			IGMULT(qtmp.g, i, q1->g, q2->g);
		for (i = 0; i < N; i++)
			for (j = 0; j <= i; j++)
				IHMULT(qtmp, i, j, *q1, *q2);
		*sp++ = qtmp;
		continue;
	case SQR:
		q1 = --sp;
		square(&qtmp.g.f, &q1->g.f);
		for (i = 0; i < N; i++)
			IGSQR(qtmp.g, i, q1->g);
		for (i = 0; i < N; i++)
			for (j = 0; j <= i; j++)
				IHSQR(qtmp, i, j, *q1);
		*sp++ = qtmp;
		continue;
        case SQRT:
                q1 = --sp;
                if( q1->g.f.lo >= 0 ) {
                    squareroot(&qtmp.g.f, &q1->g.f);
                    for (i = 0; i < N; i++)
                            IGSQRT(qtmp.g, i, q1->g );
                    for (i = 0; i < N; i++)
                            for (j = 0; j <= i; j++)
                                    IHSQRT(qtmp, i, j, *q1 );
                }
                *sp++ = qtmp;
                continue;
	case NUM:
		qtmp = ihzero;
		qtmp.g.f.lo = qtmp.g.f.hi = *code++;
		*sp++ = qtmp;
		continue;
	default:
		printf("illegal bytecode in intervalHessianMachine\n");
		exit(1);
	}}
}

/*
 *	stack machine for interval extension of (f, df/dt), the
 *	value and directional derivative along a ray.
 */
void
intervalDirectionalMachine(code, org, dir, t, id)
Bytecode *code;
double org[DIM];
double dir[DIM];
Interval t;
IntervalDirectional *id;
{
	register i;
	register IntervalDirectional *sp, *q1, *q2;
	IntervalDirectional stack[16];
	IntervalDirectional qtmp;
	IntervalDirectional X[DIM];
        IntervalDirectional a, b;

	sp = stack + 1;
	for (i = 0; i < DIM; i++) {
		itmp1.lo = itmp1.hi = dir[i];
		mult(&X[i].f, &itmp1, &t);
		X[i].f.lo += org[i];
		X[i].f.hi += org[i];
		X[i].dfdt.lo = X[i].dfdt.hi = dir[i];
	}
	for (;;) {
	switch ((int)*code++) {

	case RET:
		*id = *--sp;
		return;
	case LDX:
		i = *code++;
		*sp++ = X[i];
		continue;
	case STX:
		i = *code++;
		X[i] = *--sp;
		continue;
        case LDA:
                *sp++ = a;
                continue;
        case STA:
                a = *--sp;
                continue;
        case LDB:
                *sp++ = b;
                continue;
        case STB:
                b = *--sp;
                continue;
	case ADD:
		q1 = --sp;
		q2 = --sp;
		add(q2->f, q2->f, q1->f);
		add(q2->dfdt, q2->dfdt, q1->dfdt);
		sp++;
		continue;
	case SUB:
		q1 = --sp;
		q2 = --sp;
subtract:	sub(qtmp.f, q2->f, q1->f);
		sub(qtmp.dfdt, q2->dfdt, q1->dfdt);
		*sp++ = qtmp;
		continue;
	case NEG:
		q1 = --sp;
		q2 = &idzero;
		goto subtract;
	case DUP:
		qtmp = *--sp;
		sp++;
		*sp++ = qtmp;
		continue;
	case MUL:
		q1 = --sp;
		q2 = --sp;
		mult(&qtmp.f, &q1->f, &q2->f);
		mult(&itmp1,      &q1->f, &q2->dfdt);
		mult(&qtmp.dfdt, &q1->dfdt, &q2->f);
		add(qtmp.dfdt, qtmp.dfdt, itmp1);
		*sp++ = qtmp;
		continue;
	case SQR:
		q1 = --sp;
		square(&qtmp.f, &q1->f);
		mult(&qtmp.dfdt, &q1->f, &q1->dfdt);
		add(qtmp.dfdt, qtmp.dfdt, qtmp.dfdt);
		*sp++ = qtmp;
		continue;
        case SQRT:
                q1 = --sp;
                qtmp = *q1;
                if( q1->f.lo >= 0. ) {
                    squareroot( &qtmp.f, &q1->f );
                    add( itmp1, qtmp.f, qtmp.f );
                    div( &qtmp.dfdt, &q1->dfdt, &itmp1 );
                }
                *sp++ = qtmp;
                continue;
	case NUM:
		qtmp = idzero;
		qtmp.f.lo = qtmp.f.hi = *code++;
		*sp++ = qtmp;
		continue;
	default:
		printf("illegal bytecode in intervalDirDiv\n");
		exit(1);
	}}
}

/*
 *	vanilla stack machine for F(x,y,z)
 */
double
scalarMachine(code, x, N)
register Bytecode *code;
double x[DIM];
int N;
{
	double stack[16];
	register double *sp;
	register double tmp;
	register i;
	double a, b;

	sp = stack;
	for (;;) switch ((int)*code++) {

	case RET:	return *--sp;
	case LDX:	i = *code++;
			*sp++ = x[i];
			continue;
	case STX:	i = *code++;
			x[i] = *sp++;
			continue;
        case LDA:
                *sp++ = a;
                continue;
        case STA:
                a = *--sp;
                continue;
        case LDB:
                *sp++ = b;
                continue;
        case STB:
                b = *--sp;
                continue;
	case ADD:	tmp = *--sp;
			tmp += *--sp;
			*sp++ = tmp;
			continue;
	case SUB:	tmp = *--sp;
			tmp = *--sp - tmp;
			*sp++ = tmp;
			continue;
	case NEG:	tmp = *--sp;
			*sp++ = -tmp;
			continue;
	case MUL:	tmp = *--sp;
			tmp *= *--sp;
			*sp++ = tmp;
			continue;
	case SQR:	tmp = *--sp;
			*sp++ = tmp*tmp;
			continue;
	case SQRT:	tmp = *--sp;
			tmp = sqrt(fabs(tmp));
			*sp++ = tmp;
			continue;
	case NUM:	*sp++ = *code++;
			continue;
	case DUP:	tmp = *--sp;
			*sp++ = tmp;
			*sp++ = tmp;
			continue;
	default:	printf("illegal bytecode in scalarMachine\n");
			exit(1);
	}
}

double
implicitFun(t)
double t;
{
	static double X[DIM];

	X[0] = rayorg[0] + t*raydir[0];
	X[1] = rayorg[1] + t*raydir[1];
	X[2] = rayorg[2] + t*raydir[2];
	return scalarMachine(program, X, 3);
}

void
implicitGrad(code,x,n)
Bytecode *code;
float x[];
float n[];
{
	static double X[DIM];
	Gradient grad;

	X[0] = x[0];
	X[1] = x[1];
	X[2] = x[2];
	gradientMachine(code, X, 3, &grad);
	n[0] = grad.dfdx[0];
	n[1] = grad.dfdx[1];
	n[2] = grad.dfdx[2];
}

/*
 *	apply Moore's algorithm to isolate intervals with a single root
 */
int
intervalIsolate(t, hit)
register Interval *t;
double *hit;
{
	Interval nt;
	IntervalDirectional id;
	double m;

	for (;;) {
		intervalDirectionalMachine(program, rayorg, raydir, *t, &id);
		if (id.f.lo * id.f.hi > 0.0)
			return 0;			/* no root */
		if (id.dfdt.lo * id.dfdt.hi > 0.0)
			return intervalRefine(t,hit);	/* monotonic */
		m = t->lo + 0.5*(t->hi - t->lo);
		if (t->hi - t->lo < intervalEpsilon)
			return intervalRefine(t,hit);	/* tiny interval */
		nt.lo = t->lo;			/* bisect, recurse */
		nt.hi = m;
		t->lo = m;
		/*
		 *	find first root not "on" ray origin
		 */
		if (intervalIsolate(&nt,hit) && *hit > EPSILON)
			return 1;
	}
}

/*
 *	refine an isolated root to machine accuracy
 */
int
intervalRefine(t,hit)
Interval *t;
double *hit;
{
	double lo, hi;

	lo = implicitFun(t->lo);
	hi = implicitFun(t->hi);
	if (lo*hi <= 0.0) {
		if (lo == 0.0)
			return 0;		/* (t->lo, t->hi] */
		*hit = zeroin(t->lo, t->hi, implicitFun);
		return 1;
	}
	return 0;
}

int
intervalIntersect(code, o, d, tmin, tmax, t)
Bytecode *code;
float o[];
float d[];
double tmin, tmax;
double *t;
{
	Interval T;

	program = code;
	rayorg[0] = o[0];
	rayorg[1] = o[1];
	rayorg[2] = o[2];
	raydir[0] = d[0];
	raydir[1] = d[1];
	raydir[2] = d[2];

	T.lo = tmin;
	T.hi = tmax;
	intervalEpsilon = T.hi - T.lo;
	if (intervalEpsilon < 1.0)
		intervalEpsilon = 1.0;
	intervalEpsilon *= MACHEPS;

	return intervalIsolate(&T,t);
}
