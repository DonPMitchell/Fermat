void
bcopy(b1, b2, length)
register char *b1, *b2;
register length;
{

	if (length) {
		do {
			*b2++ = *b1++;
		} while (--length);
	}
}

void
bzero(b, length)
register char *b;
register length;
{

	if (length) {
		do {
			*b++ = 0;
		} while (--length);
	}
}
