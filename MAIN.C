#include <math.h>
#include "surface.h"
#include "light.h"

int Debug = 0;
int wavetrace = 0;
#ifdef GL
int interactive = 1;
#else
int interactive = 0;
#endif

/* Viewing parameters */
       Point3	E = {0.0, 1.5, 3.0} ;
static Point3   O = {0.0, 0.0, 0.0} ;
static Point3	U = {0.0, 1.0, 0.0} ;

/* Lighting parameters */
static Point3 Pl = {2.0, 0.0, 0.0} ;
static Color  Cl = {0.8, 0.8, 0.8} ;
int (*lightfunc)() = DistantLight;

static Material material;
Material checker;
Surface checkerfloor;
Color copper;

#define GetInt(i) \
    i = atoi(av[1]); \
    ac -= 1; \
    av += 1;

#define GetFloat(f) \
    f = atof(av[1]); \
    ac -= 1; \
    av += 1;

#define GetColor(C) \
    C.r = atof(av[1]); \
    C.g = atof(av[2]); \
    C.b = atof(av[3]); \
    ac -= 3; \
    av += 3;

#define GetPoint(P) \
    P.x = atof(av[1]); \
    P.y = atof(av[2]); \
    P.z = atof(av[3]); \
    ac -= 3; \
    av += 3;

main( int ac, char *av[] )
{
    Surface *scene;
    Point3 c; float r;
    Point3 f1, f2;
    char *name = "ray.rgb";
    int depth = 3;
    int virtual = 0;
    float fov = M_PI/5;
    int xmin, xmax, ymin, ymax;
    int xres, yres;

#define RES 128
    xmin = ymax = 0;
    xmin = ymax = RES-1;
    xres = yres = RES;

    CoFrom( &checker.Ct, 0.0, 0.0, 0.0 );
    CoFrom( &checker.Cs, 0.0, 0.0, 0.0 );
    CoFrom( &checker.Cd, 1.0, 1.0, 1.0 );
    CoFrom( &checker.Ca, 0.02, 0.02, 0.02 );
    checker.shininess = 30.0;
    checker.kr = 0.0;
    checker.kt = 0.0;
    checker.n = 1.;
    checker.shade = Checker;
    checker.lights = LIGHT1;
    checkerfloor.mat = &checker;


    CoFrom( &material.Ct, 1.0, 1.0, 1.0 );
    CoFrom( &material.Cs, 0.7, 0.7, 0.7 );
    CoFrom( &material.Cd, 1.0, 0.0, 0.0 );
    CoFrom( &material.Ca, 0.0, 0.0, 0.0 );
    material.shininess = 128.0;
    material.kr = 0.35;
    material.kt = 0.0;
    material.shade = Whitted;
    /*
    CoFrom( &material.Ct, 1.0, 1.0, 1.0 );
    CoFrom( &material.Cs, 0.7, 0.7, 0.7 );
    CoFrom( &material.Cd, 0.0, 0.0, 0.0 );
    CoFrom( &material.Ca, 0.0, 0.0, 0.0 );
    material.shininess = 128.0;
    material.kr = 0.7;
    material.kt = 0.0;
    material.shade = Whitted;
    */
#ifdef CURVATURE
    CoFrom( &material.Cs, 0.7, 0.7, 0.7 );
    CoFrom( &material.Cd, 0.5, 0.5, 0.5 );
    CoFrom( &material.Ca, 0.2, 0.2, 0.2 );
    material.shininess = 128.0;
    material.kr = 0.0;
    material.kt = 0.0;
    material.shade = Gaussian;
#endif
    material.n = 2.;
    material.lights = LIGHT1;

    Pt3From( &c, 0., 0., 0. );
    r = 1.0;

    while( --ac ) {
	++av;
	if( av[0][0] == '-' )
	    switch( av[0][1] ) {
		case 'r':
		    interactive = 1;
		    break;
		case 'g':
		    Debug = 1;
		    break;
		case 'd':
		    depth = atoi(av[1]);
		    ac -= 1;
		    av += 1;
		    break;

		case 's':
		    switch( av[0][2] ) {
			case 'p':
			    GetPoint(c);
			    break;
			case 'r':
			    GetFloat(r);
			    break;
		    }
		    break;

		case 'l': 
		    switch( av[0][2] ) {
			case 'p':
			    GetPoint(Pl);
			    break;
			case 'c':
			    GetColor(Cl);
			    break;
			case 'r':
			    switch( av[0][3] ) {
				case 'i':
				case 'd':
				    lightfunc = DistantLight;
				    break;
				case 'f':
				case 'p':
				default:
				    lightfunc = PointLight;
				    break;
			    }
			    break;
			case 'v':
			    virtual = 1;
			    break;
			case 'w':
			    virtual = 1;
			    wavetrace = 1;
		    }
		    break;

		case 'm':
		    switch( av[0][2] ) {
			case 'n':
			    GetFloat(material.n);
			    break;
			case 't':
			    GetFloat(material.kt);
			    break;
			case 'r':
			    GetFloat(material.kr);
			    break;
			case 's':
			    GetColor(material.Cs);
			    break;
			case 'd':
			    GetColor(material.Cd);
			    break;
			case 'a':
			    GetColor(material.Ca);
			    break;
			case 'h':
			    GetFloat(material.shininess);
			    break;
		    }
		    break;

		case 'i':
		    name = av[1];
		    ac -= 1;
		    av += 1;
		    break;
		case 'p':
		    GetInt(xres);
		    GetInt(yres);
		    xmin = ymin = 0;
		    xmax = xres - 1;
		    ymax = yres - 1;
		    break;
		case 'w':
		    GetInt(xmin);
		    GetInt(xmax);
		    GetInt(ymin);
		    GetInt(ymax);
		    break;
		case 'f':
		    GetFloat(fov);
		    fov = fov * (M_PI/180);
		    break;
		case 'e':
		    GetPoint(E);
		    break;
		case 'u':
		    GetPoint(U);
		    break;
		case 'o':
		    GetPoint(O);
		    break;
	    }
    }

copper = material.Cs;
    scene = CuboidCreate( &material ) ;
    LightCreate( &Pl, &Cl, lightfunc, 0 );
    if( virtual ) {
	LightCreate( &Pl, &Cl, 
	    lightfunc == PointLight ? VirtualPointLight : VirtualDistantLight,
	    (void *)scene );
	checker.lights = LIGHT2;
    }

#ifdef GL
    if( interactive )
	Interactive(scene,
	    &E, &O, &U, fov, xres, yres, xmin, xmax, ymin, ymax, name, depth ) ;
    else
#endif
	Render(scene,
	    &E, &O, &U, fov, xres, yres, xmin, xmax, ymin, ymax, name, depth ) ;
    /*

    scene = TorusCreate( 1.0, 0.5, &material );
    scene = SphereCreate( &c, r, &material ) ;
    Pt3From( &f1, -2.5, 0., 0. );
    Pt3From( &f2,  2.5, 0., 0. );
    scene = EllipseCreate( &f1, &f2, 6., &material );
    scene = SteinerCreate( &material ) ;
    scene = MonkeyCreate( &material ) ;
    */
}
