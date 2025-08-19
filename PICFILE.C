/*
 *	Picture-file routines for Duff/Mitchell format
 *	D. P. Mitchell  87/12/25.
 *	T. Duff
 *
 *  SYNOPSIS
 *
 *	#include "/usr/don/include/picfile.h"
 *
 *	PICFILE *picopen_r(picfilename)
 *	   char *picfilename;
 *
 *	PICFILE *picopen_w(picfilename, type, xorigin, yorigin, width, height,
 *				channels, argv, colormap)
 *	   char *picfilename;
 *	   int type;			-- DUMPFORMAT, RUNCODED or BITMAP
 *	   char *channels;		-- e.g. "rgb", "m", "rgba"
 *	   char *argv[];		-- from main(argc, argv)
 *	   Cmap *colormap;
 *
 *	picread(pic, buffer)
 *	   PICFILE *pic;
 *	   char *buffer;
 *
 *	picwrite(pic, buffer)
 *
 *	picclose(pic)
 *
 *  NOTE
 *	magic picfilenames "IN" and "OUT" signify standard input and output.
 */

#include "picfile.h"

#define qputs(s, f) {(void) write(f, s, strlen(s)); (void) write(f, "\n", 1);}
#define qwrite(b, s, n, f) write(f, b, (s)*(n))
#define pic_salloc(S) ((S) ? strcpy(malloc((unsigned)strlen(S) + 1), S) : 0)
#define MAXHIST		200
#define MINRUNS		4096

extern char	*malloc();
extern char	*strcpy();
extern char	*strcat();
extern char	*sprintf();
extern long	lseek();

static PICFILE	blankpicfile;
static int	npicfiles = 0;
static PICFILE	*openpic[100];
static unsigned	size_runsbuffer = 0;
static char	*runsbuffer;
static char	odd_end[1];

/*
 *	like fread, but using ordinary file descriptors
 */
static int
qread(dst, size, nitems, fid)
char *dst;
int size, nitems, fid;
{
	register i, try, amountin;

	try = nitems * size;
	if (try == 0)
		return 0;
	amountin = 0;
	for (;;) {
		i = read(fid, dst, try);
		if (i <= 0)
			return amountin / size;
		amountin += i;
		if (i == try)
			return amountin / size;
		dst += i;
		try -= i;
	}
}

/*
 *	read a string from a file descriptor (like fgets)
 */
static char *
qgets(buff, size, fid)
char *buff;
int size, fid;
{
	char *cp;
	int rval;

	cp = buff;
	while ((rval = read(fid, cp, 1)) == 1 && --size > 0)
		if (*cp++ == '\n')
			break;
	if (rval <= 0 && cp == buff)
		return (char *)0;
	*--cp = 0;
	return buff;
}

/*
 *	get a property value from a picture-file descriptor
 */
static char *
getprop(name, f)
char *name;
PICFILE *f;
{
	int i, n;

	n = strlen(name);
	for (i = 0; i < f->argc; i++)
		if (strncmp(f->argv[i], name, n) == 0 && f->argv[i][n] == '=')
			return f->argv[i] + n + 1;
	return 0;
}

/*
 *	unpack a bitmap scanline into bytes of 0 or 255
 */
bit_unpack(f, buf)
PICFILE *f;
unsigned char *buf;
{
	register byte;
	register unsigned char *src, *dst;
	int i, initial_shift;
	static unsigned char table[2] = { 0, 255 };

	if (f->width == 0)
		return;
	src = buf + f->dwidth;
	dst = buf + f->width*f->nchan;
	byte = *--src;
	initial_shift = f->dwidth*8 - f->width*f->nchan;
	byte >>= initial_shift;
	i = f->dwidth;
	switch (initial_shift) {

		do {
		case 0:	*--dst = table[byte & 1]; byte >>= 1;
		case 1:	*--dst = table[byte & 1]; byte >>= 1;
		case 2:	*--dst = table[byte & 1]; byte >>= 1;
		case 3:	*--dst = table[byte & 1]; byte >>= 1;
		case 4:	*--dst = table[byte & 1]; byte >>= 1;
		case 5:	*--dst = table[byte & 1]; byte >>= 1;
		case 6:	*--dst = table[byte & 1]; byte >>= 1;
		case 7:	*--dst = table[byte & 1];
			byte = *--src;
		} while (--i > 0);
	}
}

/*
 *	pack a scanline of bytes into a bitmap
 */
bit_pack(f, buf)
PICFILE *f;
unsigned char *buf;
{
	register byte;
	register unsigned char *dst, *src;
	register i;
	int wide;

	src = dst = buf;
	wide = f->width*f->nchan;
	byte = 0;
	for (i = 0; i < wide; i++) {
		if (*src++ > 128)
			byte |= 1;
		if (i == wide - 1 || (i & 7) == 7) {
			byte <<= 7 - (i & 7);
			*dst++ = byte;
			byte = 0;
		}
		byte += byte;
	}
}

/*
 *	Open a picture file for reading
 */
PICFILE *
picopen_r(file)
char *file;
{
	PICFILE *f;
	char line[512], *cp;
	int i;
	char *chan;

	f = (PICFILE *)malloc(sizeof(PICFILE));
	*f = blankpicfile;
	f->file = pic_salloc(file);
	if (strcmp(f->file, "IN") == 0)
		f->fd = 0;
	else if ((f->fd = open(f->file, 0)) < 0)
		goto abort_open;
	for (f->argc = 0; f->argc < MAX_PICARGV; f->argc++) {
		if (qgets(line, sizeof(line), f->fd) == 0)
			goto abort_open;
		if (strcmp(line, "") == 0)
			break;
		f->argv[f->argc] = pic_salloc(line);
	}
	if (f->argc == MAX_PICARGV)
		goto abort_open;
	if (getprop("TYPE", f) == 0)
		goto abort_open;
	if (cp = getprop("NCHAN", f))
		f->nchan = atoi(cp);
	else
		goto abort_open;
	chan = getprop("CHAN", f);
	if (chan == 0) {		/* 1127 pictures */
		switch (f->nchan) {

		case 1:	chan = "m";
			break;
		case 2: chan = "ma";
			break;
		case 3: chan = "rgb";
			break;
		case 4: chan = "rgba";
			break;
		default:
			goto abort_open;
		}
	}
	f->chan = pic_salloc(chan);
	cp = getprop("TYPE", f);
	if (strcmp(cp, "runcode") == 0) {
		f->type = RUNCODED;
		f->first = (unsigned char *)malloc((unsigned)f->nchan * MINRUNS);
		f->in = f->out = f->first;
		f->limit = f->first + MINRUNS * f->nchan;
	}
	if (strcmp(cp, "dump") == 0)
		f->type = DUMPFORMAT;
	if (strcmp(cp, "bitmap") == 0)
		f->type = BITMAP;
	i = sscanf(getprop("WINDOW", f), "%d%d%d%d", &(f->xorg), &(f->yorg),
			&(f->width), &(f->height));
	if (i != 4)
		goto abort_open;
	f->x = f->xorg;
	f->y = f->yorg;
	f->width -= f->x;
	f->height -= f->y;
	if (getprop("CMAP", f)) {
		f->cmap = (Cmap *)malloc(sizeof(Cmap));
		if (qread((char *)f->cmap, sizeof(Cmap), 1, f->fd) != 1)
			goto abort_open;
	}
	f->dwidth = f->width * f->nchan;
	if (f->type == BITMAP)
		f->dwidth = (f->dwidth + 7)/8;
	for (i = 0; i < npicfiles; i++)
		if (openpic[i] == 0)
			openpic[i] = f;
	if (i == npicfiles)
		openpic[npicfiles++] = f;
	f->top = lseek(f->fd, (long)0, 1);
	return f;
abort_open:
	perror("picopen_r");
	picclose(f);
	return 0;
}

/*
 *	read one scan line from a picture file
 */
picread(f, buf)
PICFILE *f;
register unsigned char *buf;
{
	int status, q, quanta, n;
	register unsigned char *out;
	register count, nchan, i;

	switch (f->type) {

	case DUMPFORMAT:
		status = qread((char *)buf, f->dwidth, 1, f->fd);
		break;
	case BITMAP:
		status = qread((char *)buf, f->dwidth, 1, f->fd);
		/*
		 *	BITMAP scanlines are multiples of 16 bits long
		 */
		if (f->dwidth & 1)
			(void) qread(odd_end, 1, 1, f->fd);
		break;
	case RUNCODED:
		nchan = f->nchan;
		out = f->out;
		for (i = 0; i < f->width; i++) {
			if (out >= f->in) {
				quanta = MINRUNS / (nchan + 1);
				f->in = out = f->first;
				q = qread((char *)f->in, nchan + 1, quanta, f->fd);
				if (q <= 0)
					return 0;
				f->in += q * (nchan + 1);
			}
			if (count = *out++) {
				i += count;
				switch (nchan) {
	
				default: for (n = 0; n < nchan; n++)
						*buf++ = *out++;
					 break;
				case 4:	*buf++ = *out++;
				case 3:	*buf++ = *out++;
				case 2:	*buf++ = *out++;
				case 1:	*buf++ = *out++;
				}
				f->out = out;
				out = buf - nchan;
				count *= nchan;
				switch (count & 7) {

					do {
						*buf++ = *out++;
				case 7:		*buf++ = *out++;
				case 6:		*buf++ = *out++;
				case 5:		*buf++ = *out++;
				case 4:		*buf++ = *out++;
				case 3:		*buf++ = *out++;
				case 2:		*buf++ = *out++;
				case 1:		*buf++ = *out++;
				case 0:		count -= 8;
					} while (count >= 0);
				}
				out = f->out;
			} else {
				switch (nchan) {
	
				default: for (n = 0; n < nchan; n++)
						*buf++ = *out++;
					 break;
				case 4:	*buf++ = *out++;
				case 3:	*buf++ = *out++;
				case 2:	*buf++ = *out++;
				case 1:	*buf++ = *out++;
				}
			}
		}
		f->out = out;
		status = 1;
		break;
	}
	if (f->type == BITMAP)
		bit_unpack(f, buf);
	if (status > 0)
		f->y++;
	if (status < 0)
		status = 0;
	return status;
}

/*
 *	build a command-line history if possible
 */
static
history(f, argv)
PICFILE *f;
char *argv[];
{
	char line[512];
	int i, j, total;

	if (argv == 0)
		return;
	for ((void) strcpy(line, "COMMAND= "), i = 0; argv[i]; i++) {
		(void) strcat(line, argv[i]);
		(void) strcat(line, " ");
	}
	f->argv[f->argc++] = pic_salloc(line);
	total = 0;
	for (i = 0; i < npicfiles; i++)
		if (openpic[i])
			for (j = 0; j < openpic[i]->argc; j++)
				if (!strncmp(openpic[i]->argv[j], "COMMAND=", 8)) {
					if (total++ > MAXHIST)
						return;
					(void) strcpy(line, "COMMAND= ");
					(void) strcat(line, openpic[i]->argv[j] + 8);
					f->argv[f->argc++] = pic_salloc(line);
				}
}

/*
 *	open (create) a picture file for writing
 */
PICFILE *
picopen_w(file, type, xorg, yorg, width, height, chan, argv, cmap)
char *file;
int type, xorg, yorg, width, height;
char *chan;
char *argv[];
Cmap *cmap;
{
	PICFILE *f;
	int i;
	char line[512];

	f = (PICFILE *)malloc(sizeof(PICFILE));
	*f = blankpicfile;
	f->file = pic_salloc(file);
	if (strcmp(f->file, "OUT") == 0)
		f->fd = 1;
	else if ((f->fd = creat(file, 0644)) < 0)
		goto abort_open;
	f->type = type;
	f->x = f->xorg = xorg;
	f->y = f->yorg = yorg;
	f->width = width;
	f->height = height;
	f->chan = pic_salloc(chan);
	f->nchan = strlen(chan);
	f->argc = 0;
	switch (f->type) {

	case RUNCODED:
		f->argv[f->argc++] = pic_salloc("TYPE=runcode");
		break;
	case DUMPFORMAT:
		f->argv[f->argc++] = pic_salloc("TYPE=dump");
		break;
	case BITMAP:
		f->argv[f->argc++] = pic_salloc("TYPE=bitmap");
		break;
	}
	(void) sprintf(line, "WINDOW=%d %d %d %d", xorg, yorg, xorg+width, yorg+height);
	f->argv[f->argc++] = pic_salloc(line);
	(void) sprintf(line, "NCHAN=%d", f->nchan);
	f->argv[f->argc++] = pic_salloc(line);
	(void) sprintf(line, "CHAN=%s", chan);
	f->argv[f->argc++] = pic_salloc(line);
	if (cmap) {
		f->cmap = (Cmap *)malloc(sizeof(Cmap));
		*(f->cmap) = *cmap;
		f->argv[f->argc++] = pic_salloc("CMAP=");
	}
	history(f, argv);
	for (i = 0; i < f->argc; i++)
		qputs(f->argv[i], f->fd);
	qputs("", f->fd);
	if (f->cmap)
		(void) qwrite((char *)f->cmap, sizeof(Cmap), 1, f->fd);
	f->dwidth = f->width * f->nchan;
	if (f->type == BITMAP)
		f->dwidth = (f->dwidth + 7)/8;
	f->top = lseek(f->fd, (long)0, 1);
	return f;
abort_open:
	perror("picopen_w");
	picclose(f);
	return 0;
}

/*
 *	write one scan line of a picture
 */
picwrite(f, buf)
PICFILE *f;
char *buf;
{
	int status, width, n;
	register char *p, *q, *out, *r;
	char *runs;

	if (f->type == RUNCODED && size_runsbuffer <= f->width*(f->nchan + 1)) {
		if (runsbuffer)
			free(runsbuffer);
		size_runsbuffer = f->width*(f->nchan + 1);
		runsbuffer = malloc(size_runsbuffer);
	}
	runs = runsbuffer;
	if (f->type == BITMAP)
		bit_pack(f, (unsigned char *)buf);
	switch (f->type) {

	case DUMPFORMAT:
		status = qwrite(buf, f->dwidth, 1, f->fd) > 0;
		break;
	case BITMAP:
		status = qwrite(buf, f->dwidth, 1, f->fd) > 0;
		if (f->dwidth & 1)
			(void) qwrite(odd_end, 1, 1, f->fd);
		break;
	case RUNCODED:
		out = runs;
		for (width = f->width; width > 0; width -= n) {
			q = r = buf + f->nchan;
			for (n = 1; n < width && n < 256; n++)
				for (p = buf; p < r; )
					if (*p++ != *q++)
						goto GotRun;
		GotRun:
			*out++ = n - 1;
			for (p = buf; p < r; )
				*out++ = *p++;
			buf += n * f->nchan;
		}
		status = qwrite(runs, (int)(out - runs), 1, f->fd) > 0;
		break;
	}
	if (status > 0)
		f->y++;
	return status;
}

/*
 *	Close a picture file and free storage
 */
picclose(f)
PICFILE *f;
{
	int i;

	for (i = 0; i < npicfiles; i++)
		if (openpic[i] == f)
			openpic[i] = 0;
	if (f->fd >= 0)
		(void) close(f->fd);
	if (f->file)
		free(f->file);
	if (f->chan)
		free(f->chan);
	if (f->cmap)
		free((char *)f->cmap);
	if (f->first)
		free((char *)f->first);
	for (i = 0; i < f->argc; i++)
		free(f->argv[i]);
	free((char *)f);
}

picusage(mess)
char *mess;
{
	char line[128];

	(void) strcpy(line, "Usage: ");
	(void) strcat(line, mess);
	(void) strcat(line, "\n");
	(void) write(2, line, strlen(line));
	exit(1);
}
