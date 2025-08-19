/*
 *	test the multiSolve function
 */
#include "interval.h"

extern int multiSolve();

Bytecode c0[] = {
	LDX, 0., SQR,
	LDX, 1., NUM, 0.50, ADD, SQR,
	LDX, 2., SQR,
	ADD,
	ADD,
	NUM, 1.0, SUB,
	RET,
};

Bytecode c1[] = {
	LDX, 0., SQR,
	LDX, 1., NUM, -0.50, ADD, SQR,
	LDX, 2., SQR,
	ADD,
	ADD,
	NUM, 1.0, SUB,
	RET,
};

Bytecode c2[] = {
	LDX, 0,  SQR,
	LDX, 1,  SQR,
	ADD,
	NUM, 0.25, SUB,
	RET,
};

Bytecode f0[] = {
	LDX, 0, SQR, LDX, 1, SQR, ADD, NUM, 1, SUB, RET,
};

Bytecode f1[] = {
	LDX, 0, LDX, 1, SQR, SUB, RET,
};

Bytecode g0[] = {
	LDX, 0, SQR,
	LDX, 1, SQR,
	LDX, 2, SQR,
	ADD, ADD,
	NUM, 1.0, SUB,
	RET,
};

Bytecode g1[] = { LDX, 3, LDX, 0, NUM, -2.0, MUL, MUL, RET, };
Bytecode g2[] = { LDX, 3, LDX, 1, NUM, -2.0, MUL, MUL, RET, };
Bytecode g3[] = { LDX, 3, LDX, 2, NUM, -2.0, MUL, MUL, NUM, 1.0, ADD, RET, };
void foo(Vector y, Vector x, int N)
{

	y[0] = scalarMachine(g0, x, 4);
	y[1] = scalarMachine(g1, x, 4);
	y[2] = scalarMachine(g2, x, 4);
	y[3] = scalarMachine(g3, x, 4);
}

void igfoo(IntervalGradient y[], IntervalVector x, int N)
{

	intervalGradientMachine(g0, x, 4, &y[0]);
	intervalGradientMachine(g1, x, 4, &y[1]);
	intervalGradientMachine(g2, x, 4, &y[2]);
	intervalGradientMachine(g3, x, 4, &y[3]);
}

void printvec(Vector v, int N)
{
	int i;

	for (i = 0; i < N; i++)
		printf("%20.14f ", v[i]);
	printf("\n");
}

main()
{
	IntervalVector ivec;
	Vector roots[16];
	int i, nroots;

/*
		scanf("%lf%lf", x+0, x+1);
		r2i(ix[0], x[0]);
		r2i(ix[1], x[1]);
		foo(y, x, 2);
		igfoo(igy, ix, 2);
		printf("%f %f\n", y[0], y[1]);
		printf("[%f %f], <[%f %f], [%f %f]>\n",
			igy[0].f.lo, igy[0].f.hi,
			igy[0].dfdx[0].lo, igy[0].dfdx[0].hi,
			igy[0].dfdx[1].lo, igy[0].dfdx[1].hi );
		printf("[%f %f], <[%f %f], [%f %f]>\n",
			igy[1].f.lo, igy[1].f.hi,
			igy[1].dfdx[0].lo, igy[1].dfdx[0].hi,
			igy[1].dfdx[1].lo, igy[1].dfdx[1].hi );
*/
	ivec[0].lo = -2;
	ivec[0].hi = 3;
	ivec[1].lo = -2;
	ivec[1].hi = 3;
	ivec[2].lo = -2;
	ivec[2].hi = 3;
	ivec[3].lo = -2;
	ivec[3].hi = 3;
	for (i = 0; i < 10; i++)
		nroots = multiSolve(roots, foo, igfoo, ivec, 4);
	printf("%d roots:\n", nroots);
	for (i = 0; i < nroots; i++)
		printvec(roots[i], 4);
}
