#
# Makefile for Fermat Wave Tracer
#
# Pat Hanrahan	Jan 1992
#

LIBS = -lm

CFLAGS= -I/usr/graphics/include -O

OBJS= main.o trace.o surface.o material.o light.o texture.o interval.o krawczyk.o rootRefine.o wavefront.o curvature.o color.o point3.o ray.o  bcopy.o picfile.o
SRCS= main.c trace.c surface.c material.c light.o texture.c interval.c krawczyk.c rootRefine.c wavefront.c curvature.c color.c point3.c ray.c bcopy.c picfile.c

trace: 	${OBJS}
	cc -g -o trace ${OBJS} ${LIBS}

wc:
	wc ${SRCS}

kiss: 	kiss.o krawczyk.o interval.o rootRefine.o
	cc -g -o kiss kiss.o krawczyk.o interval.o rootRefine.o -lm

junk: 	junk.o interval.o rootRefine.o
	cc -g -o junk junk.o interval.o rootRefine.o -lm

curv: 	curv.o curvature.o interval.o rootRefine.o
	cc -g -o curv curv.o curvature.o interval.o rootRefine.o -lm

clean: 
	rm -f *.o kiss junk curv
