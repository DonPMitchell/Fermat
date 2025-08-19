#ifndef CURVATUREDEFS
#define CURVATUREDEFS

#include "surface.h"
#include "point3.h"

extern void Euler( 
    float k1, float k2, float c, float s, 
    float *ku, float *kv, float *kuv );

extern void EulerInv( 
    float ku, float kv, float kuv,
    float *k1, float *k2, float *cosa, float *sina );

extern void
SurfaceCurvature( 
    Bytecode *code, Point3 *P, Point3 *Pu, Point3 *Pv,
    float *ku, float *kv, float *kuv );

extern float
SurfaceKMCurvature( 
    Bytecode *code, Point3 *P, Point3 *Pu, Point3 *Pv,
    float *k1, float *k2, float *cosu, float *sinu );

#endif 
