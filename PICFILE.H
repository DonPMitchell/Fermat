/*
 *	Macros and data structures for Duff/Mitchell picture files
 *	D. P. Mitchell  87/12/25.
 */

#define RUNCODED	1
#define DUMPFORMAT	2
#define BITMAP		3
#define NCHAN(P) ((P)->nchan)
#define WIDTH(P) ((P)->width)
#define HEIGHT(P) ((P)->height)
#define CHAN(P) ((P)->chan)
#define CMAP(P) ((P)->cmap)

#define MAX_PICARGV	256
#define SAME_AS(P)	(P)->type, (P)->xorg, (P)->yorg, (P)->width, (P)->height, (P)->chan, 0, (P)->cmap
#define SAME_ARGV(P)	(P)->type, (P)->xorg, (P)->yorg, (P)->width, (P)->height, (P)->chan, argv, (P)->cmap
#define WINDOW(P)	(P)->xorg, (P)->yorg, (P)->width, (P)->height
#define ASSERTBW(P) if((P)->nchan!=1){(void)write(2,"Must be B/W pic\n",16);exit(1);}
#define ASSERTRGB(P) if((P)->nchan!=3){(void)write(2,"Must be RGB pic\n",16);exit(1);}
#define ASSERTRGBA(P) if((P)->nchan!=4){(void)write(2,"Must be RGBA pic\n",17);exit(1);}

/*
 *	Color map structure
 */
typedef struct Cmap {
	unsigned char	map[256][3];
} Cmap;

/*
 *	Picture-file structure
 */
typedef struct PICFILE {
	int	fd;
	char	*file;
	int	xorg, yorg, width, height;		/* window dimensions */
	int	dwidth;					/* sizeof raster */
	int	x, y;					/* current position */
	int	nchan;
	char	*chan;
	int	type;
	Cmap	*cmap;
	unsigned char	*first, *in, *out, *limit;	/* runcode buffer */
	long	top;					/* start of pixels */
	int	argc;
	char	*argv[MAX_PICARGV];			/* all header lines */
} PICFILE;


extern PICFILE *picopen_r(), *picopen_w();
