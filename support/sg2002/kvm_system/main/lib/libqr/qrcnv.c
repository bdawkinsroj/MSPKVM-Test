/*
 * QR Code Generator Library: Symbol Converters for Basic Output Formats
 *
 * Core routines were originally written by Junn Ohta.
 * Based on qr.c Version 0.1: 2004/4/3 (Public Domain)
 *
 * @package     libqr
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @copyright   2006-2013 Ryusuke SEKIYAMA
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 */

#include "qrcnv.h"

/* {{{ utility macro */

#define repeat(m, n) for ((m) = 0; (m) < (n); (m)++)

/* }}} */
/* {{{ symbol writing macro */

#define qrInitRow(filler) { \
	memset(rbuf, (filler), (size_t)rsize); \
	rptr = rbuf; \
}

#define qrWriteRow(m, n) { \
	wsize = (int)(rptr - rbuf); \
	for ((m) = 0; (m) < (n); (m)++) { \
		memcpy(sptr, rbuf, (size_t)wsize); \
		sptr += wsize; \
	} \
	if (wsize < rsize) { \
		*size -= (rsize - wsize) * (n); \
	} \
}

/*
 * A macro that summarizes the standard tasks when outputting symbols.
*
* Before calling this macro, define the following four macros and undefine them after calling them.
* qrWriteBOR(): Writes the beginning of the line
* qrWriteEOR(): Writes the end of the line
* qrWriteBLM(m, n): Writes the light module
* qrWriteDKM(m, n): Writes the dark module
*/
#define qrWriteSymbol(qr, filler) { \
	/* Separation pattern (top) */ \
	if (sepdim > 0) { \
		qrInitRow(filler); \
		qrWriteBOR(); \
		qrWriteBLM(j, imgdim); \
		qrWriteEOR(); \
		qrWriteRow(i, sepdim); \
	} \
	for (i = 0; i < dim; i++) { \
		/* initialize row */ \
		qrInitRow(filler); \
		/* Outfit */ \
		qrWriteBOR(); \
		/* Separation pattern (left) */ \
		qrWriteBLM(j, sepdim); \
		/* symbol body */ \
		for (j = 0; j < dim; j++) { \
			if (qrIsBlack((qr), i, j)) { \
				qrWriteDKM(jx, mag); \
			} else { \
				qrWriteBLM(jx, mag); \
			} \
		} \
		/* Separation pattern (right) */ \
		qrWriteBLM(j, sepdim); \
		/* end of line */ \
		qrWriteEOR(); \
		/* Write the line mag times */ \
		qrWriteRow(ix, mag); \
	} \
	/* Separation pattern (bottom) */ \
	if (sepdim > 0) { \
		qrInitRow(filler); \
		qrWriteBOR(); \
		qrWriteBLM(j, imgdim); \
		qrWriteEOR(); \
		qrWriteRow(i, sepdim); \
	} \
}

/* }}} */
/* {{{ Structured append symbol writing macro */

#define qrsWriteSymbols(st, filler) { \
	for (k = 0; k < rows; k++) { \
		/* Separation pattern (top) */ \
		if (sepdim > 0) { \
			qrInitRow(filler); \
			qrWriteBOR(); \
			qrWriteBLM(j, xdim); \
			qrWriteEOR(); \
			qrWriteRow(i, sepdim); \
		} \
		for (i = 0; i < dim; i++) { \
			/* initialize row */ \
			qrInitRow(filler); \
			/* Outfit */ \
			qrWriteBOR(); \
			for (kx = 0; kx < cols; kx++) { \
				/* Separation pattern (left) */ \
				qrWriteBLM(j, sepdim); \
				/* symbol body */ \
				if (order < 0) { \
					pos = k + rows * kx; \
				} else { \
					pos = cols * k + kx; \
				} \
				if (pos < (st)->num) { \
					for (j = 0; j < dim; j++) { \
						if (qrIsBlack((st)->qrs[pos], i, j)) { \
							qrWriteDKM(jx, mag); \
						} else { \
							qrWriteBLM(jx, mag); \
						} \
					} \
				} else { \
					qrWriteBLM(j, zdim); \
				} \
			} \
			/* Separation pattern (right) */ \
			qrWriteBLM(j, sepdim); \
			/* end of line */ \
			qrWriteEOR(); \
			/* Write the line mag times */ \
			qrWriteRow(ix, mag); \
		} \
	} \
	/* Separation pattern (bottom) */ \
	if (sepdim > 0) { \
		qrInitRow(filler); \
		qrWriteBOR(); \
		qrWriteBLM(j, xdim); \
		qrWriteEOR(); \
		qrWriteRow(i, sepdim); \
	} \
}

/* }}} */
/* {{{ Digit formatted symbol writing macro */

#define qrWriteBOR_Digit()
#define qrWriteEOR_Digit() { *rptr++ = ' '; }
#define qrWriteBLM_Digit(m, n) { rptr += (n); }
#define qrWriteDKM_Digit(m, n) { repeat(m, n) { *rptr++ = '1'; } }

/* }}} */
/* {{{ Ascii art formatted symbol writing macro */

#define qrWriteBOR_ASCII()
#ifdef WIN32
#define qrWriteEOR_ASCII() { *rptr++ = '\r'; *rptr++ = '\n'; }
#else
#define qrWriteEOR_ASCII() { *rptr++ = '\n'; }
#endif
#ifdef _QRCNV_AA_STYLE_U2588
#define QRCNV_AA_UNIT 3
#define qrWriteBLM_ASCII(m, n) { rptr += (n) * 2; }
#define qrWriteDKM_ASCII(m, n) { repeat(m, n) { *rptr++ = 0xe2; *rptr++ = 0x96; *rptr++ = 0x88; } }
#else
#define QRCNV_AA_UNIT 2
#define qrWriteBLM_ASCII(m, n) { rptr += (n) * 2; }
#define qrWriteDKM_ASCII(m, n) { repeat(m, n) { *rptr++ = 'X'; *rptr++ = 'X'; } }
#endif

/* }}} */
/* {{{ JSON formatted symbol writing macro */

#define qrWriteBOR_JSON() { *rptr++ = '['; }
#define qrWriteEOR_JSON() { rptr--; *rptr++ = ']'; rptr++; }
#ifdef _QR_JSON_STYLE_BOOLELAN
#define QRCNV_JSON_UNIT 6
#define qrWriteBLM_JSON(m, n) { repeat(m, n) { memcpy(rptr, "false", 5); rptr += 6; } }
#define qrWriteDKM_JSON(m, n) { repeat(m, n) { memcpy(rptr, "true", 4); rptr += 5; } }
#else
#define QRCNV_JSON_UNIT 2
#define qrWriteBLM_JSON(m, n) { repeat(m, n) { *rptr++ = '0'; rptr++; } }
#define qrWriteDKM_JSON(m, n) { repeat(m, n) { *rptr++ = '1'; rptr++; } }
#endif

/* }}} */
/* {{{ PBM formatted symbol writing macro */

#define qrWriteBOR_PBM()
#define qrWriteEOR_PBM() { *rptr++ = '\n'; }
#define qrWriteBLM_PBM(m, n) { repeat(m, n) { rptr++; *rptr++ = '0'; } }
#define qrWriteDKM_PBM(m, n) { repeat(m, n) { rptr++; *rptr++ = '1'; } }

/* }}} */
/* {{{ qrSymbolToDigit() */

/*
 * Convert the generated QR code symbol into a string consisting of 0, 1 and spaces
 */
QR_API qr_byte_t *
qrSymbolToDigit(QRCode *qr, int sep, int mag, int *size)
{
	qr_byte_t *rbuf, *rptr;
	qr_byte_t *sbuf, *sptr;
	int rsize, wsize;
	int i, j, ix, jx, dim, imgdim, sepdim;

	QRCNV_CHECK_STATE();
	QRCNV_GET_SIZE();

	/*
	 * Calculate the size after conversion and allocate memory
	 */
	rsize = imgdim + 1;
	*size = rsize * imgdim - 1;
	QRCNV_MALLOC(rsize, *size + 1);

	sptr = sbuf;

#define qrWriteBOR qrWriteBOR_Digit
#define qrWriteEOR qrWriteEOR_Digit
#define qrWriteBLM qrWriteBLM_Digit
#define qrWriteDKM qrWriteDKM_Digit

	/*
	 * write symbol
	 */
	qrWriteSymbol(qr, '0');

	/*
	 * Replace the last character (space) with the terminator
	 */
	*(--sptr) = '\0';

#undef qrWriteBOR
#undef qrWriteEOR
#undef qrWriteBLM
#undef qrWriteDKM

	free(rbuf);

	return sbuf;
}

/* }}} */
/* {{{ qrSymbolToASCII() */

/*
 * Convert the generated QR code symbol into a string consisting of 0, 1 and spaces
 */
QR_API qr_byte_t *
qrSymbolToASCII(QRCode *qr, int sep, int mag, int *size)
{
	qr_byte_t *rbuf, *rptr;
	qr_byte_t *sbuf, *sptr;
	int rsize, wsize;
	int i, j, ix, jx, dim, imgdim, sepdim;

	QRCNV_CHECK_STATE();
	QRCNV_GET_SIZE();

	/*
	 * Calculate the size after conversion and allocate memory
	 */
	rsize = imgdim * QRCNV_AA_UNIT + QRCNV_EOL_SIZE;
	*size = rsize * imgdim;
	QRCNV_MALLOC(rsize, *size + 1);

	sptr = sbuf;

#define qrWriteBOR qrWriteBOR_ASCII
#define qrWriteEOR qrWriteEOR_ASCII
#define qrWriteBLM qrWriteBLM_ASCII
#define qrWriteDKM qrWriteDKM_ASCII

	/*
	 * write symbol
	 */
	qrWriteSymbol(qr, ' ');

	/*
	 * Write the termination character
	 */
	*sptr = '\0';

#undef qrWriteBOR
#undef qrWriteEOR
#undef qrWriteBLM
#undef qrWriteDKM

	free(rbuf);

	return sbuf;
}

/* }}} */
/* {{{ qrSymbolToJSON() */

/*
 * Convert the generated QR code symbol into a JSON format string 
 * Decode the JSON to get a two-dimensional array
 */
QR_API qr_byte_t *
qrSymbolToJSON(QRCode *qr, int sep, int mag, int *size)
{
	qr_byte_t *rbuf, *rptr;
	qr_byte_t *sbuf, *sptr;
	int rsize, wsize;
	int i, j, ix, jx, dim, imgdim, sepdim;

	QRCNV_CHECK_STATE();
	QRCNV_GET_SIZE();

	/*
	 * Calculate the size after conversion and allocate memory
	 */
	rsize = 1 + imgdim * QRCNV_JSON_UNIT + 1;
	*size = 1 + rsize * imgdim - 1 + 1;
	QRCNV_MALLOC(rsize, *size + 1);

	sptr = sbuf;

#define qrWriteBOR qrWriteBOR_JSON
#define qrWriteEOR qrWriteEOR_JSON
#define qrWriteBLM qrWriteBLM_JSON
#define qrWriteDKM qrWriteDKM_JSON

	/*
	 * write header
	 */
	*sptr++ = '[';

	/*
	 * write symbol
	 */
	qrWriteSymbol(qr, ',');

	/*
	 * Write the footer and terminator
	 */
	sptr--;
	*sptr++ = ']';
	*sptr = '\0';

#undef qrWriteBOR
#undef qrWriteEOR
#undef qrWriteBLM
#undef qrWriteDKM

	free(rbuf);

	return sbuf;
}

/* }}} */
/* {{{ qrSymbolToPBM() */

/*
 * Convert the generated QR code symbol into a monochrome binary
 * ASCII format Portable Bitmap (PBM)
 */
QR_API qr_byte_t *
qrSymbolToPBM(QRCode *qr, int sep, int mag, int *size)
{
	qr_byte_t *rbuf, *rptr;
	qr_byte_t *sbuf, *sptr;
	int rsize, wsize;
	int i, j, ix, jx, dim, imgdim, sepdim;
	char header[64];
	int hsize;

	QRCNV_CHECK_STATE();
	QRCNV_GET_SIZE();

	/*
	 * Calculate the size after conversion and allocate memory
	 */
	hsize = snprintf(&(header[0]), sizeof(header), "P1\n%d %d\n", imgdim, imgdim);
	if (hsize == -1 || header[hsize - 1] != '\n') {
		QRCNV_RETURN_FAILURE(QR_ERR_UNKNOWN, _QR_FUNCTION);
	}
	rsize = imgdim * 2 + 1;
	*size = hsize + rsize * imgdim;
	QRCNV_MALLOC(rsize, *size + 1);

	sptr = sbuf;

#define qrWriteBOR qrWriteBOR_PBM
#define qrWriteEOR qrWriteEOR_PBM
#define qrWriteBLM qrWriteBLM_PBM
#define qrWriteDKM qrWriteDKM_PBM

	/*
	 * write header
	 */
	memcpy(sptr, header, (size_t)hsize);
	sptr += hsize;

	/*
	 * write symbol
	 */
	qrWriteSymbol(qr, ' ');

	/*
	 * Write the termination character
	 */
	*sptr = '\0';

#undef qrWriteBOR
#undef qrWriteEOR
#undef qrWriteBLM
#undef qrWriteDKM

	free(rbuf);

	return sbuf;
}

/* }}} */
/* {{{ qrsSymbolsToDigit() */

/*
 * qrSymbolToDigit() for structural concatenation
 * order is ignored
 */
QR_API qr_byte_t *
qrsSymbolsToDigit(QRStructured *st, int sep, int mag, int order, int *size)
{
	QRCode *qr = st->cur;
	qr_byte_t *rbuf, *rptr;
	qr_byte_t *sbuf, *sptr;
	int rsize, wsize;
	int i, j, k, ix, jx;
	int cols, rows, xdim, ydim, zdim;
	int dim, imgdim, sepdim;

	QRCNV_SA_CHECK_STATE();
	QRCNV_SA_IF_ONE(qrSymbolToDigit);
	QRCNV_SA_GET_SIZE();

	/*
	 * Calculate the size after conversion and allocate memory
	 */
	rsize = imgdim + 1;
	*size = rsize * imgdim * st->num - 1;
	QRCNV_MALLOC(rsize, *size + 1);

	sptr = sbuf;

#define qrWriteBOR qrWriteBOR_Digit
#define qrWriteEOR qrWriteEOR_Digit
#define qrWriteBLM qrWriteBLM_Digit
#define qrWriteDKM qrWriteDKM_Digit

	/*
	 * write symbol
	 */
	for (k = 0; k < st->num; k++) {
		qrWriteSymbol(st->qrs[k], '0');
		*(--sptr) = '\n';
		sptr++;
	}

	/*
	 * Replace the last character (LF) with a terminator
	 */
	*(--sptr) = '\0';

#undef qrWriteBOR
#undef qrWriteEOR
#undef qrWriteBLM
#undef qrWriteDKM

	free(rbuf);

	return sbuf;
}

/* }}} */
/* {{{ qrsSymbolsToASCII() */

/*
 * Constructed connection using qrSymbolToASCII()
 */
QR_API qr_byte_t *
qrsSymbolsToASCII(QRStructured *st, int sep, int mag, int order, int *size)
{
	QRCode *qr = st->cur;
	qr_byte_t *rbuf, *rptr;
	qr_byte_t *sbuf, *sptr;
	int rsize, wsize;
	int i, j, k, ix, jx, kx;
	int cols, rows, pos, xdim, ydim, zdim;
	int dim, imgdim, sepdim;

	QRCNV_SA_CHECK_STATE();
	QRCNV_SA_IF_ONE(qrSymbolToASCII);
	QRCNV_SA_GET_SIZE();

	/*
	 * Calculate the size after conversion and allocate memory
	 */
	rsize = xdim * QRCNV_AA_UNIT + QRCNV_EOL_SIZE;
	*size = rsize * ydim;
	QRCNV_MALLOC(rsize, *size + 1);

	sptr = sbuf;

#define qrWriteBOR qrWriteBOR_ASCII
#define qrWriteEOR qrWriteEOR_ASCII
#define qrWriteBLM qrWriteBLM_ASCII
#define qrWriteDKM qrWriteDKM_ASCII

	/*
	 * write symbol
	 */
	qrsWriteSymbols(st, ' ');

	/*
	 * Write the termination character
	 */
	*sptr = '\0';

#undef qrWriteBOR
#undef qrWriteEOR
#undef qrWriteBLM
#undef qrWriteDKM

	free(rbuf);

	return sbuf;
}

/* }}} */
/* {{{ qrsSymbolsToJSON() */

/*
 * qrSymbolToJSON() for structural concatenation
 * order is ignored
 */
QR_API qr_byte_t *
qrsSymbolsToJSON(QRStructured *st, int sep, int mag, int order, int *size)
{
	QRCode *qr = st->cur;
	qr_byte_t *rbuf, *rptr;
	qr_byte_t *sbuf, *sptr;
	int rsize, wsize;
	int i, j, k, ix, jx;
	int cols, rows, xdim, ydim, zdim;
	int dim, imgdim, sepdim;

	QRCNV_SA_CHECK_STATE();
	/*QRCNV_SA_IF_ONE(qrSymbolToJSON);*/
	QRCNV_SA_GET_SIZE();

	/*
	 * Calculate the size after conversion and allocate memory
	 */
	rsize = 1 + imgdim * QRCNV_JSON_UNIT + 1;
	*size = 1 + (1 + rsize * imgdim + 1) * st->num - 1 + 1;
	QRCNV_MALLOC(rsize, *size + 1);

	sptr = sbuf;

#define qrWriteBOR qrWriteBOR_JSON
#define qrWriteEOR qrWriteEOR_JSON
#define qrWriteBLM qrWriteBLM_JSON
#define qrWriteDKM qrWriteDKM_JSON

	/*
	 * write header
	 */
	*sptr++ = '[';

	/*
	 * write symbol
	 */
	for (k = 0; k < st->num; k++) {
		*sptr++ = '[';
		qrWriteSymbol(st->qrs[k], ',');
		sptr--;
		*sptr++ = ']';
		*sptr++ = ',';
	}

	/*
	 * Write the footer and terminator
	 */
	sptr--;
	*sptr++ = ']';
	*sptr = '\0';

#undef qrWriteBOR
#undef qrWriteEOR
#undef qrWriteBLM
#undef qrWriteDKM

	free(rbuf);

	return sbuf;
}

/* }}} */
/* {{{ qrsSymbolsToPBM() */

/*
 * Constructed connection using qrSymbolToPBM()
 */
QR_API qr_byte_t *
qrsSymbolsToPBM(QRStructured *st, int sep, int mag, int order, int *size)
{
	QRCode *qr = st->cur;
	qr_byte_t *rbuf, *rptr;
	qr_byte_t *sbuf, *sptr;
	int rsize, wsize;
	int i, j, k, ix, jx, kx;
	int cols, rows, pos, xdim, ydim, zdim;
	int dim, imgdim, sepdim;
	char header[64];
	int hsize;

	QRCNV_SA_CHECK_STATE();
	QRCNV_SA_IF_ONE(qrSymbolToPBM);
	QRCNV_SA_GET_SIZE();

	/*
	 * Calculate the size after conversion and allocate memory
	 */
	hsize = snprintf(&(header[0]), 64, "P1\n%d %d\n", xdim, ydim);
	if (hsize >= 64) {
		QRCNV_RETURN_FAILURE(QR_ERR_UNKNOWN, _QR_FUNCTION);
	}
	rsize = xdim * 2 + 1;
	*size = hsize + rsize * ydim;
	QRCNV_MALLOC(rsize, *size + 1);

	sptr = sbuf;

#define qrWriteBOR qrWriteBOR_PBM
#define qrWriteEOR qrWriteEOR_PBM
#define qrWriteBLM qrWriteBLM_PBM
#define qrWriteDKM qrWriteDKM_PBM

	/*
	 * write header
	 */
	memcpy(sptr, header, (size_t)hsize);
	sptr += hsize;

	/*
	 * write symbol
	 */
	qrsWriteSymbols(st, ' ');

	/*
	 * Append a terminator
	 */
	*sptr = '\0';

#undef qrWriteBOR
#undef qrWriteEOR
#undef qrWriteBLM
#undef qrWriteDKM

	free(rbuf);

	return sbuf;
}

/* }}} */
