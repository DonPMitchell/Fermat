/*
 *	solve nonlinear system via interval analysis
 *	D. P. Mitchell  91/12/26.
 */
#include "interval.h"

extern double fabs(double);

static int	singularMatrix;
static double	macheps = 1.0;
static double	msTolerance;

/*
 *	estimate machine epsilon
 */
double machineEpsilon()
{
	double eps, tmp;

	eps = 1.0;
	do {
		eps *= 0.5;
		tmp = eps + 1.0;
	} while (tmp > 1.0);
	return eps;
}
/*
 *	y = A*x
 */
void matVec(Vector y, Matrix A, Vector x, int N)
{
	register i, j;

	for (i = 0; i < N; i++) {
		y[i] = 0.0;
		for (j = 0; j < N; j++)
			y[i] += A[i][j] * x[j];
	}
}
/*
 *	C = A*B
 */
void matMul(Matrix C, Matrix A, Matrix B, int N)
{
	register k, j, i;

	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++) {
			C[i][j] = 0.0;
			for (k = 0; k < N; k++)
				C[i][j] += A[i][k] * B[k][j];
		}
}
/*
 *	B = A**-1  (adapted from Numerical Recipes)
 */
#define SWAP(a,b) {double temp=(a);(a)=(b);(b)=temp;}

void matInv(Matrix B, Matrix A, int N)
{
	int indxc[DIM], indxr[DIM], ipiv[DIM];
	int i;
	register icol, irow, l, ll;
	double big,dum,pivinv;
	int n, m;

	n = m = N;
	singularMatrix = 0;
	for (l = 0; l < n; l++) {
		for (ll = 0; ll < m; ll++)
			B[l][ll] = 0.0;
		B[l][l] = 1.0;
	}
	for (ll=0;ll<n;ll++) ipiv[ll]=0;
	for (i=0;i<n;i++) {
		big=0.0;
		for (ll=0;ll<n;ll++)
			if (ipiv[ll] != 1)
				for (l=0;l<n;l++) {
					if (ipiv[l] == 0) {
						if (fabs(A[ll][l]) >= big) {
							big=fabs(A[ll][l]);
							irow=ll;
							icol=l;
						}
					} else if (ipiv[l] > 1) singularMatrix = 1;
				}
		++(ipiv[icol]);
		if (irow != icol) {
			for (l=0;l<n;l++) SWAP(A[irow][l],A[icol][l])
			for (l=0;l<m;l++) SWAP(B[irow][l],B[icol][l])
		}
		indxr[i]=irow;
		indxc[i]=icol;
		if (A[icol][icol] == 0.0) singularMatrix = 2;
		pivinv=1.0/A[icol][icol];
		A[icol][icol]=1.0;
		for (l=0;l<n;l++) A[icol][l] *= pivinv;
		for (l=0;l<m;l++) B[icol][l] *= pivinv;
		for (ll=0;ll<n;ll++)
			if (ll != icol) {
				dum=A[ll][icol];
				A[ll][icol]=0.0;
				for (l=0;l<n;l++) A[ll][l] -= A[icol][l]*dum;
				for (l=0;l<m;l++) B[ll][l] -= B[icol][l]*dum;
			}
	}
	for (l=n-1;l>=0;l--) {
		if (indxr[l] != indxc[l])
			for (ll=0;ll<n;ll++)
				SWAP(A[ll][indxr[l]],A[ll][indxc[l]]);
	}
}
/*
 *	one step of simple Newton's method:  nx = x - Y*f(x)
 */
void newtonStep(Vector nx, void (*f)(Vector, Vector, int),
		Matrix Y, Vector x, int N)
{
	Vector fx;
	register i;

	f(fx, x, N);
	matVec(nx, Y, fx, N);
	for (i = 0; i < N; i++)
		nx[i] = x[i] - nx[i];
}
/*
 *	root refinement via simple Newton's method
 */
void newton(Vector root, void (*f)(Vector, Vector, int),
	    Matrix Y, Vector x0, int N)
{
	register i;
	Vector x;
	double norm, dx, tolerance;

	if (macheps > 0.5)
		macheps = machineEpsilon();
	for (i = 0; i < N; i++)
		x[i] = x0[i];
	norm = 0;
	for (i = 0; i < N; i++)
		if (fabs(x[i]) > norm)
			norm = fabs(x[i]);
	tolerance = 20.0*macheps*norm;
	do {
		newtonStep(root, f, Y, x, N);
		norm = 0.0;
		for (i = 0; i < N; i++) {
			dx = fabs(root[i] - x[i]);
			if (dx > norm)
				norm = dx;
		}
		for (i = 0; i < N; i++)
			x[i] = root[i];
	} while (norm > tolerance);
}
/*
 *	Find solutions to N-dimensional nonlinear system.
 *	Return number of solutions founds in interval X.
 */
int multiIsolate(Vector roots[],
	       void (*f)(Vector, Vector, int),
	       void (*igf)(IntervalGradient[], IntervalVector, int),
	       IntervalVector X, int N, int split)
{
	IntervalGradient IGF[DIM];
	IntervalVector X1, X2, KX;
	Matrix Y, mJacobian, absY, wJacobian, IYF;
	Vector fmx, mx, wx, wIYFX;
	double IYF_Norm, width, mid, rowsum;
	int nroots, doNewtonStep = 0;
	register i, j;
	int debug = 0;

/*
if(X[0].lo <= 0.707107 && X[0].hi >= 0.707107
   && X[1].lo <= 0.0 && X[1].hi >= 0.0
   && X[2].lo <= 0.707107 && X[2].hi >= 0.707107
   && X[3].lo <= 3.55765 && X[3].hi >= 3.55765) {
	printf("box wid %f ht %f with .sw at %f,%f\n",
		X[0].hi - X[0].lo, X[1].hi - X[1].lo, X[0].lo, X[1].lo);
	debug = 1;
}
*/
	/*
	 *	F(X) and F'(X).  Is interval infeasible?
	 */
	igf(IGF, X, N);
	for (i = 0; i < N; i++)
		if (IGF[i].f.lo > 0.0 || IGF[i].f.hi < 0.0) {
if (debug) printf("infeasbile[%d]\n", i);
			return 0;
		}
	/*
	 *	Y = m(F'(X))**-1
	 */
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++)
			mJacobian[i][j] =
				0.5*(IGF[i].dfdx[j].lo + IGF[i].dfdx[j].hi);
	matInv(Y, mJacobian, N);
	if (singularMatrix) {
		for (i = 0; i <N; i++)
			KX[i] = X[i];
if (debug) printf("singular matrix\n");
		goto bisect;
	}
	/*
	 *	f(m(X))
	 */
	for (i = 0; i < N; i++)
		mx[i] = 0.5*(X[i].lo + X[i].hi);
	f(fmx, mx, N);
	/*
	 *	Krawczyk's interval-newton-step operator,
	 *
	 *	K(X) = m(X) - Yf(m(X)) + {I - YF'}(X - m(X))
	 *				 -------------------
	 *					^
	 *			this part is symmetric, can be computed
	 *			without really doing interval arithmetic!
	 */
	for (i = 0; i < N; i++)
		for (j = 0; j < N; j++) {
			wJacobian[i][j] =
				0.5*(IGF[i].dfdx[j].hi - IGF[i].dfdx[j].lo);
			absY[i][j] = fabs(Y[i][j]);
		}
	matMul(IYF, absY, wJacobian, N);
	for (i = 0; i < N; i++)
		wx[i] = 0.5*(X[i].hi - X[i].lo);
	matVec(wIYFX, IYF, wx, N);
	matVec(wx, Y, fmx, N);
	for (i = 0; i < N; i++) {
		KX[i].lo = mx[i] - wx[i] - wIYFX[i];
		KX[i].hi = mx[i] - wx[i] + wIYFX[i];
	}
	/*
	 *	no root if K(X) doesn't intersect X
	 */
	for (i = 0; i < N; i++)
		if (X[i].lo > KX[i].hi || KX[i].lo > X[i].hi) {
if (debug) printf("K(X) & X == nil (no root)\n");
			return 0;
		}
	/*
	 *	|| I - YF'(X) ||  maximum-row-sum norm
	 */
	IYF_Norm = 0.0;
	for (i = 0; i < N; i++) {
		rowsum = 0.0;
		for (j = 0; j < N; j++)
			rowsum += IYF[i][j];
		if (rowsum > IYF_Norm)
			IYF_Norm = rowsum;
	}
	/*
	 *	safe starting point for Newton's method?
	 */
	if (IYF_Norm < 1.0) {
if (debug) printf("Norm < 1.0\n");
		for (i = 0; i < N; i++)
			if (KX[i].lo < X[i].lo || KX[i].hi > X[i].hi) {
				doNewtonStep = 1;
				goto bisect;
			}
if (debug) printf("Safe for Newton\n");
		newton(roots[0], f, Y, mx, N);
		return 1;
	}
bisect:
if (debug) printf("bisecting\n");
	/*
	 *	bisect intersection of K(X) and X
	 */
	for (i = 0; i < N; i++) {
		if (X[i].lo > KX[i].lo)
			KX[i].lo = X[i].lo;
		if (X[i].hi < KX[i].hi)
			KX[i].hi = X[i].hi;
		X1[i] = X2[i] = KX[i];
	}
	width = 0.0;
	for (i = 0; i < N; i++)
		if (KX[i].hi - KX[i].lo > width) {
			width = KX[i].hi - KX[i].lo;
		}
	if (width < msTolerance) {
		/*
		 *	assume a solution if one newton step stays inside
		 */
if (debug) printf("width %f < tolerance %f\n", width, msTolerance);
		newtonStep(roots[0], f, Y, mx, N);
		for (i = 0; i < N; i++)
			if (roots[0][i] < X[i].lo || roots[0][i] > X[i].hi)
				return 0;
if (debug) printf("assume root inside small box\n");
		return 1;
	}
	mid = KX[split].lo + 0.501*(KX[split].hi - KX[split].lo);
	X1[split].hi = mid;
	X2[split].lo = mid;
	nroots = multiIsolate(roots, f, igf, X1, N, (split+1)%N);
	return nroots + multiIsolate(roots+nroots, f, igf, X2, N, (split+1)%N);
}

int multiSolve(Vector roots[], void (*f)(double[], double[], int),
	       void (*igf)(IntervalGradient[], IntervalVector, int),
	       IntervalVector X0, int N)
{
	double dx, norm;
	int i;

	if (macheps > 0.5)
		macheps = machineEpsilon();
	norm = 0.0;
	for (i = 0; i < N; i++) {
		dx = fabs(X0[i].hi);
		if (fabs(X0[i].lo) > dx)
			dx = fabs(X0[i].lo);
		if (dx > norm)
			norm = dx;
	}
	msTolerance = 50.0*macheps*norm;
	return multiIsolate(roots, f, igf, X0, N, 0);
}

