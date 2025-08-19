#ifndef MATERIALDEFS
#define MATERIALDEFS

#include "point3.h"
#include "color.h"

typedef struct {
    Color Cd, Ca, Cs, Ct;
    float shininess;
    float kr, kt;
    float n;
    Color (*shade)();
    int lights;
} Material;

#endif
