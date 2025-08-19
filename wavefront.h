#ifndef WAVEFRONTDEFS
#define WAVEFRONTDEFS

#include "point3.h"
#include "surface.h"

typedef struct {
    Point3 U, V, N;
    float ku, kv;
} Wavefront;

extern Wavefront *WavePlane( Point3 *D );
extern Wavefront *WaveSphere( Point3 *D );

extern Wavefront *WaveTransfer( Wavefront *w, float d );
extern Wavefront *WaveRefract( Wavefront *w, Surface *s, Point3 *P, float eta);
extern Wavefront *WaveReflect( Wavefront *w, Surface *s, Point3 *P );

extern float WaveIntensity( Wavefront *w );

#endif
