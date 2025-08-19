/*
 *	C version of Dekker-Brent root-refinement algorithm
 *	D. P. Mitchell  88/09/22.
 *
 *	Procedure zeroin returns a zero x of the function f in the
 *	given interval [a,b], to within a tolerance 6*macheps*FABS(x)
 *	where macheps is the relative machine precision.  The procedure
 *	assumes that f(a) and f(b) have different signs.
 *
 *		- Algorithms for Minimization without Derivatives
 *			by Richard P. Brent
 */

#define FABS(X) ((X) >= 0.0 ? (X) : -(X))

double
zeroin(a, b, f)
double a;
double b;
double (*f)();
{
	static double macheps = 1.0;
	double c, step, lastStep;
	register double p, q;
	double r, s;
	double fa, fb, fc;
	double bisectStep, tolerance, macheps1;

	if (macheps > 0.5) {
		do {
			macheps *= 0.5;
			macheps1 = macheps + 1.0;
		} while (macheps1 > 1.0);
	}
	fa = (*f)(a);
	fb = (*f)(b);
	/*
	 *	Zero lies between b and c.  b is better approximation.
	 */
	for (;;) {
		c = a;					/* Interpolate */
		fc = fa;
		lastStep = step = b - a;
extrapolate:	if (FABS(fc) < FABS(fb)) {
			a = b;
			b = c;
			c = a;
			fa = fb;
			fb = fc;
			fc = fa;
		}
		tolerance = 2.0*macheps*FABS(b);
		bisectStep = 0.5*(c - b);
		/*
		 *	b is probably better than (b+c)/2, so return it
		 */
		if (FABS(bisectStep) <= tolerance || fb == 0.0)
			return b;
		/*
		 *	See if a bisection is forced.  This will guarantee
		 *	convergence in log**2((b-a)/tolerance) steps, unlike
		 *	linear interpolation alone.
		 */
		if (FABS(lastStep) < tolerance || FABS(fa) <= FABS(fb))
			lastStep = step = bisectStep;
		else {
			if (a == c) {
				s = fb/fa;		/* Linear interp */
				p = 2.0*bisectStep*s;
				q = 1.0 - s;
			} else {
				q = fa/fc;		/* Inv quad interp */
				r = fb/fc;
				s = fb/fa;
				p = s*(2.0*bisectStep*q*(q-r)-(b-a)*(r-1.0));
				q = (q - 1.0)*(r - 1.0)*(s - 1.0);
			}
			if (p > 0.0)			/* Right solution */
				q = -q;
			else
				p = -p;
			if (2.0*p >= 3.0*bisectStep*q - FABS(tolerance*q)
			    || p >= FABS(0.5*lastStep*q))
				lastStep = step = bisectStep;
			else {
				lastStep = step;	/* Interp OK */
				step = p/q;
			}
		}
		a = b;
		fa = fb;
		/*
		 *	Be careful not to loop forever by adding a step
		 *	that is too small to change b.
		 */
		if (FABS(step) > tolerance)
			b = b + step;
		else if (bisectStep < 0.0)
			b = b - tolerance;
		else
			b = b + tolerance;
		fb = (*f)(b);
		if ((fb > 0.0) != (fc > 0.0))
			goto extrapolate;
	}
}
