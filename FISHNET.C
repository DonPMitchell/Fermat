/*
 *	read ray paths and form wavefront mesh
 */
#include <math.h>

float xl, yl, zl;				/* light source */
float xp[64][64], yp[64][64], zp[64][64];	/* point on mirror */
float xd[64][64], yd[64][64], zd[64][64];	/* reflect direction */
int N;						/* N x N mesh */

loadData()
{
	int i, j;

	scanf("%f%f%f", &xl, &yl, &zl);
	while (scanf("%d%d", &i, &j) == 2) {
		if (i+1 > N) N = i+1;
		scanf("%f%f%f%f%f%f", &xp[i][j], &yp[i][j], &zp[i][j],
				      &xd[i][j], &yd[i][j], &zd[i][j]);
	}
}

wavefront(double t)
{
	double dist, dx, dy, dz, s;
	double x, y, z;
	int i, j;

	printf("MESH\n%d %d\n", N, N);
	for (j = 0; j < N; j++)
		for (i = 0; i < N; i++) {
			dx = xp[i][j] - xl;
			dy = yp[i][j] - yl;
			dz = zp[i][j] - zl;
			dist = sqrt(dx*dx + dy*dy + dz*dz);
			if (t < dist) {
				s = t/dist;
				x = s*xp[i][j] + (1.0 - s)*xl;
				y = s*yp[i][j] + (1.0 - s)*yl;
				z = s*zp[i][j] + (1.0 - s)*zl;
			} else {
				x = xp[i][j] + t*xd[i][j];
				y = yp[i][j] + t*yd[i][j];
				z = zp[i][j] + t*zd[i][j];
			}
			printf("%f %f %f\n", x, y, z);
		}
}

main(int argc, char *argv[])
{

	loadData();
	wavefront(atof(argv[1]));
}
