#ifndef COLORDEFS
#define COLORDEFS

typedef float CoCoord;
typedef struct { CoCoord r, g, b;   } Color;

extern Color White;
extern Color Black;

void CoPrint( Color *c );
void CoCopy( Color *c1, Color *c2 );
void CoFrom( Color *c, CoCoord r, CoCoord g, CoCoord b );
void CoAdd( Color *c1, Color *c2, Color *c3 );
void CoSub( Color *c1, Color *c2, Color *c3 );
void CoFilter( Color *c1, Color *c2, Color *c3 );
void CoAccum( CoCoord s, Color *c1, Color *c2, Color *c3 );
void CoScale( CoCoord s, Color *c1, Color *c2 );
#endif
