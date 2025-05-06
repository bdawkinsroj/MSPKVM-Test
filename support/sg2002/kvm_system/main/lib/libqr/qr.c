/*
 * QR Code Generator Library
 *
 * Core routines were originally written by Junn Ohta.
 * Based on qr.c Version 0.1: 2004/4/3 (Public Domain)
 *
 * @package     libqr
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @copyright   2006-2013 Ryusuke SEKIYAMA
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qr.h"
#include "qr_util.h"
#include "qr_private.h"
#include "qr_dwtable.h"

#define qrIsData(qr, i, j)  (((qr)->symbol[i][j] & QR_MM_DATA) != 0)
#define qrIsFunc(qr, i, j)  (((qr)->symbol[i][j] & QR_MM_FUNC) != 0)

QR_API const char *(*qrGetCurrentFunctionName)(void) = NULL;

/*
 * returns the library version
 */
QR_API const char *
qrVersion(void)
{
	return LIBQR_VERSION;
}

/*
 * Create a QRCode object
 */
QR_API QRCode *
qrInit(int version, int mode, int eclevel, int masktype, int *errcode)
{
	QRCode *qr = NULL;

	/*
	 * Allocate memory
	 */
	qr = (QRCode *)calloc(1, sizeof(QRCode));
	if (qr == NULL) {
		*errcode = QR_ERR_MEMORY_EXHAUSTED;
		return NULL;
	}
	qr->dataword = (qr_byte_t *)calloc(1, QR_DWD_MAX);
	qr->ecword   = (qr_byte_t *)calloc(1, QR_ECW_MAX);
	qr->codeword = (qr_byte_t *)calloc(1, QR_CWD_MAX);
	if (qr->dataword == NULL || qr->ecword == NULL || qr->codeword == NULL) {
		*errcode = QR_ERR_MEMORY_EXHAUSTED;
		qrDestroy(qr);
		return NULL;
	}

	/*
	 * Initialize the internal state
	 */
	qr->_symbol = NULL;
	qr->symbol = NULL;
	qr->source = NULL;
	qr->srcmax = 0;
	qr->srclen = 0;
	qr->enclen = 0;
	qr->delta1 = 0;
	qr->delta2 = 0;
	qr->errcode = QR_ERR_NONE;
	qr->state = QR_STATE_BEGIN;

	/*
	 * Set model number
	*/
	if (version == -1 || (version >= 1 && version <= QR_VER_MAX)) {
		qr->param.version = version;
	} else {
		*errcode = QR_ERR_INVALID_VERSION;
		qrDestroy(qr);
		return NULL;
	}

	/*
	 * Set encoding mode
	*/
	if (mode == QR_EM_AUTO || (mode >= QR_EM_NUMERIC && mode < QR_EM_COUNT)) {
		qr->param.mode = mode;
	} else {
		*errcode = QR_ERR_INVALID_MODE;
		qrDestroy(qr);
		return NULL;
	}

	/*
	 * Setting the error correction level
	*/
	if (eclevel >= QR_ECL_L && eclevel < QR_EM_COUNT) {
		qr->param.eclevel = eclevel;
	} else {
		*errcode = QR_ERR_INVALID_ECL;
		qrDestroy(qr);
		return NULL;
	}

	/*
	 * Set mask pattern
	*/
	if (masktype == -1 || (masktype >= 0 && masktype < QR_MPT_MAX)) {
		qr->param.masktype = masktype;
	} else {
		*errcode = QR_ERR_INVALID_MPT;
		qrDestroy(qr);
		return NULL;
	}

	return qr;
}

/*
 * Create a QRStructured object
 */
QR_API QRStructured *
qrsInit(int version, int mode, int eclevel, int masktype, int maxnum, int *errcode)
{
	QRStructured *st = NULL;

	/*
	 * Allocate memory
	 */
	st = (QRStructured *)calloc(1, sizeof(QRStructured));
	if (st == NULL) {
		*errcode = QR_ERR_MEMORY_EXHAUSTED;
		return NULL;
	}

	/*
	 * Initialize the internal state
	 */
	st->parity = 0;
	st->state = QR_STATE_BEGIN;

	/*
	 * Set the maximum number of symbols
	*/
	if (maxnum >= 2 && masktype <= QR_STA_MAX) {
		st->max = maxnum;
	} else {
		*errcode = QR_ERR_INVALID_MAXNUM;
		qrsDestroy(st);
		return NULL;
	}

	/*
	 * Set model number
	 */
	if (version >= 1 && version <= QR_VER_MAX) {
		st->param.version = version;
	} else {
		*errcode = QR_ERR_INVALID_VERSION;
		qrsDestroy(st);
		return NULL;
	}

	/*
	 * Set encoding mode
	*/
	if (mode == QR_EM_AUTO || (mode >= QR_EM_NUMERIC && mode < QR_EM_COUNT)) {
		st->param.mode = mode;
	} else {
		*errcode = QR_ERR_INVALID_MODE;
		qrsDestroy(st);
		return NULL;
	}

	/*
	 * Setting the error correction level
	*/
	if (eclevel >= QR_ECL_L && eclevel < QR_EM_COUNT) {
		st->param.eclevel = eclevel;
	} else {
		*errcode = QR_ERR_INVALID_ECL;
		qrsDestroy(st);
		return NULL;
	}

	/*
	 * Set mask pattern
	*/
	if (masktype == -1 || (masktype >= 0 && masktype < QR_MPT_MAX)) {
		st->param.masktype = masktype;
	} else {
		*errcode = QR_ERR_INVALID_MPT;
		qrsDestroy(st);
		return NULL;
	}

	/*
	 * Initialize the first QR code object
	*/
	st->qrs[0] = qrInit(st->param.version, st->param.mode,
			st->param.eclevel, st->param.masktype, errcode);
	if (st->qrs[0] == NULL) {
		qrsDestroy(st);
		return NULL;
	}
	st->cur = st->qrs[0];
	st->num = 1;

	return st;
}

/*
 * Duplicate a QRCode object
 */
QR_API QRCode *
qrClone(const QRCode *qr, int *errcode)
{
	QRCode *cp = NULL;

	/*
	 * Allocate memory for the QRCode object and duplicate it.
	 */
	cp = (QRCode *)malloc(sizeof(QRCode));
	if (cp == NULL) {
		*errcode = QR_ERR_MEMORY_EXHAUSTED;
		return NULL;
	}
	memcpy(cp, qr, sizeof(QRCode));

	/*
	 * Set dynamically allocated members to NULL
	 */
	cp->dataword = NULL;
	cp->ecword = NULL;
	cp->codeword = NULL;
	cp->_symbol = NULL;
	cp->symbol = NULL;
	cp->source = NULL;

	/*
	 * After finalization, duplicate the symbol. Before finalization, duplicate the calculation area.
	 */
	if (cp->state == QR_STATE_FINAL) {
		int i, dim;

		dim = qr_vertable[cp->param.version].dimension;

		cp->_symbol = (qr_byte_t *)calloc((size_t)dim, (size_t)dim);
		if (cp->_symbol == NULL) {
			*errcode = QR_ERR_MEMORY_EXHAUSTED;
			qrDestroy(cp);
			return NULL;
		}
		memcpy(cp->_symbol, qr->_symbol, (size_t)(dim * dim));

		cp->symbol = (qr_byte_t **)malloc(sizeof(qr_byte_t *) * (size_t)dim);
		if (cp->symbol == NULL) {
			*errcode = QR_ERR_MEMORY_EXHAUSTED;
			qrDestroy(cp);
			return NULL;
		}
		for (i = 0; i < dim; i++) {
			cp->symbol[i] = cp->_symbol + dim * i;
		}
	} else {
		cp->dataword = (qr_byte_t *)malloc(QR_DWD_MAX);
		cp->ecword   = (qr_byte_t *)malloc(QR_ECW_MAX);
		cp->codeword = (qr_byte_t *)malloc(QR_CWD_MAX);
		if (cp->dataword == NULL || cp->ecword == NULL || cp->codeword == NULL) {
			*errcode = QR_ERR_MEMORY_EXHAUSTED;
			qrDestroy(cp);
			return NULL;
		}
		memcpy(cp->dataword, qr->dataword, QR_DWD_MAX);
		memcpy(cp->ecword  , qr->ecword  , QR_ECW_MAX);
		memcpy(cp->codeword, qr->codeword, QR_CWD_MAX);
	}

	/*
	 * Duplicate input data
	 */
	if (cp->srcmax > 0 && qr->source != NULL) {
		cp->source = (qr_byte_t *)malloc(cp->srcmax);
		if (cp->source == NULL) {
			*errcode = QR_ERR_MEMORY_EXHAUSTED;
			qrDestroy(cp);
			return NULL;
		}
		memcpy(cp->source, qr->source, cp->srclen);
	}

	return cp;
}

/*
 * Duplicating a QRStructured object
 */
QR_API QRStructured *
qrsClone(const QRStructured *st, int *errcode)
{
	QRStructured *cps = NULL;
	int i = 0;

	/*
	 * Allocate memory for the QRStructured object and duplicate it.
	 */
	cps = (QRStructured *)malloc(sizeof(QRStructured));
	if (cps == NULL) {
		*errcode = QR_ERR_MEMORY_EXHAUSTED;
		return NULL;
	}
	memcpy(cps, st, sizeof(QRStructured));

	/*
	 * Duplicate the QRCode object you are holding
	 */
	while (i < cps->num) {
		QRCode *cp = qrClone(st->qrs[i], errcode);
		if (cp == NULL) {
			while (i > 0) {
				qrDestroy(cps->qrs[--i]);
				free(cps);
			}
			return NULL;
		}
		cps->qrs[i++] = cp;
	}
	while (i < QR_STA_MAX) {
		cps->qrs[i++] = NULL;
	}
	cps->cur = cps->qrs[0] + (st->cur - st->qrs[0]);

	return cps;
}

/*
 * Free the QRCode object
 */
QR_API void
qrDestroy(QRCode *qr)
{
	if (qr == NULL) {
		return;
	}
	qrFree(qr->source);
	qrFree(qr->dataword);
	qrFree(qr->ecword);
	qrFree(qr->codeword);
	qrFree(qr->symbol);
	qrFree(qr->_symbol);
	free(qr);
}

/*
 * Freeing a QRStructured object
 */
QR_API void
qrsDestroy(QRStructured *st)
{
	int i;
	if (st == NULL) {
		return;
	}
	for (i = 0; i < st->num; i++) {
		qrDestroy(st->qrs[i]);
	}
	free(st);
}

/*
 * Returns the MIME type for the output format.
 */
QR_API const char *
qrMimeType(int format)
{
	switch (format) {
		case QR_FMT_PNG:    return "image/png";
		case QR_FMT_BMP:    return "image/bmp";
		case QR_FMT_TIFF:   return "image/tiff";
		case QR_FMT_PBM:    return "image/x-portable-bitmap";
		case QR_FMT_SVG:    return "image/svg+xml";
		case QR_FMT_JSON:   return "application/json";
		case QR_FMT_DIGIT:  return "text/plain";
		case QR_FMT_ASCII:  return "text/plain";
		default: return NULL;
	}
}

/*
 * Returns the extension (without the period) for the output format.
 */
QR_API const char *
qrExtension(int format)
{
	switch (format) {
		case QR_FMT_PNG:    return "png";
		case QR_FMT_BMP:    return "bmp";
		case QR_FMT_TIFF:   return "tiff";
		case QR_FMT_PBM:    return "pbm";
		case QR_FMT_SVG:    return "svg";
		case QR_FMT_JSON:   return "json";
		case QR_FMT_DIGIT:  return "txt";
		case QR_FMT_ASCII:  return "txt";
		default: return NULL;
	}
}

/*
 * Returns the error number registered in the QR code object.
 */
QR_API int
qrGetErrorCode(QRCode *qr)
{
	return qr->errcode;
}

/*
 * Returns the error information registered in the QR code object.
 */
QR_API char *
qrGetErrorInfo(QRCode *qr)
{
	return &(qr->errinfo[0]);
}

/*
 * Returns the error number registered in the last QR code object of the structural concatenation.
 */
QR_API int
qrsGetErrorCode(QRStructured *st)
{
	return st->cur->errcode;
}

/*
 * Returns the error information registered in the last QR code object of the structural link.
 */
QR_API char *
qrsGetErrorInfo(QRStructured *st)
{
	return &(st->cur->errinfo[0]);
}

/*
 * Returns the error information corresponding to the error number.
 */
QR_API const char *
qrStrError(int errcode)
{
	switch (errcode) {
	/* wide use errors */
	  case QR_ERR_NONE:
	  case QR_ERR_USAGE:
		return "";

	  case QR_ERR_SEE_ERRNO:
		return "For more information, check for errno";

	  case QR_ERR_NOT_IMPL:
		return "Not yet implemented";

	  case QR_ERR_STATE:
		return "Not allowed in the current state";

	  case QR_ERR_FOPEN:
		return "Failed to open file";

	  case QR_ERR_FREAD:
		return "Failed to read data";

	  case QR_ERR_FWRITE:
		return "Failed to write data";

	  case QR_ERR_MEMORY_EXHAUSTED:
		return "Memory exhausted";

	/* invalid parameter errors */
	  case QR_ERR_INVALID_ARG:
		return "Invalid argument";

	  case QR_ERR_INVALID_VERSION:
		return "Invalid version number";

	  case QR_ERR_INVALID_MODE:
		return "Invalid encoding mode";

	  case QR_ERR_INVALID_ECL:
		return "Invalid error correction level";

	  case QR_ERR_INVALID_MPT:
		return "Invalid mask pattern type";

	  case QR_ERR_INVALID_MAG:
		return "Invalid pixel magnifying ratio";

	  case QR_ERR_INVALID_SEP:
		return "Invalid separator width";

	  case QR_ERR_INVALID_SIZE:
		return "Invalid output size";

	  case QR_ERR_INVALID_FMT:
		return "Invalid output format";

	  case QR_ERR_INVALID_OUT:
		return "Invalid output pathname";

	  case QR_ERR_INVALID_MAXNUM:
		return "Invalid maximum symbol number";

	  case QR_ERR_UNSUPPORTED_FMT:
		return "Unsupported output format";

	  case QR_ERR_EMPTY_PARAM:
		return "Parameter required";

	/* input data size/type errors */
	  case QR_ERR_EMPTY_SRC:
		return "Input data is empty";

	  case QR_ERR_LARGE_SRC:
		return "Input data too large";

	  case QR_ERR_NOT_NUMERIC:
		return "Non decimal characters found";

	  case QR_ERR_NOT_ALNUM:
		return "Non alphanumeric characters found";

	  case QR_ERR_NOT_KANJI:
		return "Non JIS X 0208 kanji sequence found";

	/* imaging related errors */
	  case QR_ERR_IMAGE_TOO_LARGE:
		return "Output image size too large";

	  case QR_ERR_WIDTH_TOO_LARGE:
		return "Output image width too large";

	  case QR_ERR_HEIGHT_TOO_LARGE:
		return "Output image height too large";

	  case QR_ERR_IMAGECREATE:
		return "Failed to create image";

	  case QR_ERR_IMAGEFORMAT:
		return "Failed to convert image";

	  case QR_ERR_IMAGEFRAME:
		return "Failed to create frame";

	/* zlib related errors */
	  case QR_ERR_DEFLATE:
		return "Failed to deflate";

	/* unknown error(s) */
	  case QR_ERR_UNKNOWN:
	  default:
		return "Unknown error";
	}
}

/*
 * Set error information from libqr error number
 */
QR_API void
qrSetErrorInfo(QRCode *qr, int errnum, const char *param)
{
	qr->errcode = errnum;
	if (param != NULL) {
		snprintf(&(qr->errinfo[0]), QR_ERR_MAX, "%s: %s", param, qrStrError(errnum));
	} else {
		snprintf(&(qr->errinfo[0]), QR_ERR_MAX, "%s", qrStrError(errnum));
	}
}

/*
 * Sets error information from the system standard error number.
 */
QR_API void
qrSetErrorInfo2(QRCode *qr, int errnum, const char *param)
{
	char *info;
	int size = 0;
	info = &(qr->errinfo[0]);
	qr->errcode = QR_ERR_SEE_ERRNO;
	if (param != NULL) {
		size = snprintf(info, QR_ERR_MAX, "%s: ", param);
		if (size < 0 || size >= QR_ERR_MAX) {
			return;
		}
		info += size;
	}
#ifdef WIN32
	snprintf(info, (size_t)(QR_ERR_MAX - size), "%s", strerror(errnum));
#else
	strerror_r(errnum, info, (size_t)(QR_ERR_MAX - size));
#endif
}

/*
 * Set error information from libqr error number and variable length parameters
 */
QR_API void
qrSetErrorInfo3(QRCode *qr, int errnum, const char *fmt, ...)
{
	char info[QR_ERR_MAX];
	va_list ap;

	qr->errcode = errnum;
	va_start(ap, fmt);
	vsnprintf(&(info[0]), QR_ERR_MAX, fmt, ap);
	va_end(ap);
	snprintf(&(qr->errinfo[0]), QR_ERR_MAX, "%s%s", qrStrError(errnum), info);
}

/*
 * Finding the best encoding method
 */
QR_API int
qrDetectDataType(const qr_byte_t *source, int size)
{
	if (qrStrPosNotNumeric(source, size) == -1) {
		return QR_EM_NUMERIC;
	}
	if (qrStrPosNotAlnum(source, size) == -1) {
		return QR_EM_ALNUM;
	}
	if (qrStrPosNotKanji(source, size) == -1) {
		return QR_EM_KANJI;
	}
	return QR_EM_8BIT;
}

/*
 * Find the location of non-numeric data
 */
QR_API int
qrStrPosNotNumeric(const qr_byte_t *source, int size)
{
	int p = 0;

	while (p < size) {
		if (source[p] < '0' || source[p] > '9') {
			return p;
		}
		p++;
	}
	return -1;
}

/*
 * Check for the location of non-alphanumeric data
 */
QR_API int
qrStrPosNotAlnum(const qr_byte_t *source, int size)
{
	int p = 0;

	while (p < size) {
		if (qr_alnumtable[source[p]] == -1) {
			return p;
		}
		p++;
	}
	return -1;
}

/*
 * Check the position where data other than JIS X 0208 kanji appears
 */
QR_API int
qrStrPosNotKanji(const qr_byte_t *source, int size)
{
	qr_byte_t x, y;
	int p = 0;

	while (p < size - 1) {
		x = source[p++];
		if (x >= 0x81 && x <= 0x9f) {
			x -= 0x81;
		} else if (x >= 0xe0 && x <= 0xea) {
			x -= 0xc1;
		} else {
			/* Not the first byte of JIS X 0208 Kanji */
			return p - 1;
		}
		y = source[p++];
		if (y >= 0x40 && y <= 0xfc) {
			y -= 0x40;
		} else {
			/* JIS X 0208漢字の2バイトめでない */
			return p - 1;
		}
		if (qr_dwtable_kanji[x][y] == -1) {
			/* JIS X 0208 Undefined areas of Chinese characters */
			return p - 2;
		}
	}
	if (p < size) {
		return p;
	}
	return -1;
}

/*
 * Check the position where alphanumeric or JIS X 0208 kanji data appears
 */
QR_API int
qrStrPosNot8bit(const qr_byte_t *source, int size)
{
	qr_byte_t x, y;
	int p = 0;

	while (p < size) {
		x = source[p++];
		if (qr_alnumtable[x] != -1) {
			return p - 1;
		}
		if (p < size && ((x >= 0x81 && x <= 0x9f) || (x >= 0xe0 && x <= 0xea))) {
			if (x < 0xa0) {
				x -= 0x81;
			} else {
				x -= 0xc1;
			}
			y = source[p];
			if (y >= 0x40 && y <= 0xfc && qr_dwtable_kanji[x][y - 0x40] != -1) {
				return p - 1;
			}
		}
	}
	return -1;
}

/*
 * Returns the bit length when size bytes are encoded in the default encoding mode.
 */
QR_API int
qrGetEncodedLength(QRCode *qr, int size)
{
	return qrGetEncodedLength2(qr, size, qr->param.mode);
}

/*
 * Returns the length in bits when size bytes are encoded in a specific encoding mode.
 */
QR_API int
qrGetEncodedLength2(QRCode *qr, int size, int mode)
{
	int n, v;

	/*
	 * Mode and character count indicator sizes
	 */
	v = (qr->param.version == -1) ? QR_VER_MAX : qr->param.version;
	n = 4 + qr_vertable[v].nlen[mode];

	/*
	 * Data size for each encoding mode
	 */
	switch (mode) {
	  case QR_EM_NUMERIC:
		/*
		 * Number mode: 10 bits per 3 digits
		 * (remainder is 4 bits for 1 digit, 7 bits for 2 digits)
		 */
		n += (size / 3) * 10;
		switch (size % 3) {
		  case 1:
			n += 4;
			break;
		  case 2:
			n += 7;
			break;
		}
		break;
	  case QR_EM_ALNUM:
		/*
		 * Alphanumeric mode: 11 bits per 2 digits
		 * (6 bits per digit remainder)
		 */
		n += (size / 2) * 11;
		if (size % 2 == 1) {
			n += 6;
		}
		break;
	  case QR_EM_8BIT:
		/*
		 * 8-bit byte mode: 8 bits per digit
		 */
		n += size * 8;
		break;
	  case QR_EM_KANJI:
		/*
		 * Kanji mode: 13 bits per character (2 bytes)
		 */
		n += (size / 2) * 13;
		break;
	  default:
		qrSetErrorInfo(qr, QR_ERR_INVALID_MODE, NULL);
		return -1;
	}

	return n;
}

/*
 * Returns the maximum byte length that can be encoded in the default encoding mode, up to size bits.
 */
QR_API int
qrGetEncodableLength(QRCode *qr, int size)
{
	return qrGetEncodableLength2(qr, size, qr->param.mode);
}

/*
 * Returns the maximum length of bytes that can be encoded in a particular encoding mode, up to size bits.
 */
QR_API int
qrGetEncodableLength2(QRCode *qr, int size, int mode)
{
	int l, m, n, v;

	/*
	 * Mode and character count indicator sizes
	 */
	v = (qr->param.version == -1) ? QR_VER_MAX : qr->param.version;
	n = size - 4 - qr_vertable[v].nlen[mode];
	if (n <= 0) {
		return 0;
	}

	/*
	 * Data size for each encoding mode
	 */
	switch (mode) {
	  case QR_EM_NUMERIC:
		/*
		 * Number mode: 10 bits per 3 digits
 		 * (remainder is 4 bits for 1 digit, 7 bits for 2 digits)
		 */
		l = (n / 10) * 3;
		m = n % 10;
		if (m >= 7) {
			l += 2;
		} else if (m >= 4) {
			l += 1;
		}
		break;
	  case QR_EM_ALNUM:
		/*
		 * Alphanumeric mode: 11 bits per 2 digits
		 * (6 bits per digit remainder)
		 */
		l = (n / 11) * 2;
		m = n % 11;
		if (m >= 6) {
			l += 1;
		}
		break;
	  case QR_EM_8BIT:
		/*
		 * 8-bit byte mode: 8 bits per digit
		 */
		l = n / 8;
		break;
	  case QR_EM_KANJI:
		/*
		 * Kanji mode: 13 bits per character (2 bytes)
		 */
		l = (n / 13) * 2;
		break;
	  default:
		qrSetErrorInfo(qr, QR_ERR_INVALID_MODE, NULL);
		return -1;
	}

	return l;
}

/*
 * Add data
 */
QR_API int
qrAddData(QRCode *qr, const qr_byte_t *source, int size)
{
	if (qr->state == QR_STATE_FINAL) {
		qrSetErrorInfo(qr, QR_ERR_STATE, _QR_FUNCTION);
		return FALSE;
	}
	return qrAddData2(qr, source, size, qr->param.mode);
}

/*
 * Add data by specifying the encoding mode
 */
QR_API int
qrAddData2(QRCode *qr, const qr_byte_t *source, int size, int mode)
{
	int enclen, maxlen;
	int version;
	int pos, err;

	if (qr->state == QR_STATE_FINAL) {
		qrSetErrorInfo(qr, QR_ERR_STATE, _QR_FUNCTION);
		return FALSE;
	}

	if (size <= 0) {
		qrSetErrorInfo(qr, QR_ERR_EMPTY_SRC, NULL);
		return FALSE;
	}

	/*
	 * Choose the best encoding mode for your input data
	 */
	if (mode == QR_EM_AUTO) {
		mode = qrDetectDataType(source, size);
	} else if (mode < QR_EM_NUMERIC || mode >= QR_EM_COUNT) {
		qrSetErrorInfo(qr, QR_ERR_INVALID_MODE, NULL);
		return FALSE;
	}


	/*
	 * Calculate the length of the encoded data
	 */
	enclen = qrGetEncodedLength2(qr, size, mode);
	if (enclen == -1) {
		return FALSE;
	}
	version = (qr->param.version == -1) ? QR_VER_MAX : qr->param.version;
	maxlen = 8 * qr_vertable[version].ecl[qr->param.eclevel].datawords;
	if (qr->enclen + enclen > maxlen) {
		qrSetErrorInfo3(qr, QR_ERR_LARGE_SRC, ", %d total encoded bits"
				" (max %d bits on version=%d, ecl=%s)",
				qr->enclen + enclen, maxlen, version, qr_eclname[qr->param.eclevel]);
		return FALSE;
	}
	if (qr->param.version == -1) {
		qr->delta1 += qr_vertable[QR_VER_MAX].nlen[mode] - qr_vertable[VERPOINT1].nlen[mode];
		qr->delta2 += qr_vertable[QR_VER_MAX].nlen[mode] - qr_vertable[VERPOINT2].nlen[mode];
	}

	/*
	 * If a model number is specified, the input data is encoded directly without buffering.
	 */
	if (qr->param.version != -1) {
		qr->enclen += enclen;
		if (!qrHasData(qr)) {
			qrInitDataWord(qr);
		}
		if (qrEncodeDataWord(qr, source, size, mode) == TRUE) {
			qr->state = QR_STATE_SET;
			return TRUE;
		}
		return FALSE;
	}

	/*
	 * Validate input data
	 */
	pos = -1;
	err = QR_ERR_NONE;
	switch (mode) {
	  case QR_EM_NUMERIC:
		pos = qrStrPosNotNumeric(source, size);
		err = QR_ERR_NOT_NUMERIC;
		break;
	  case QR_EM_ALNUM:
		pos = qrStrPosNotAlnum(source, size);
		err = QR_ERR_NOT_ALNUM;
		break;
	  case QR_EM_KANJI:
		pos = qrStrPosNotKanji(source, size);
		err = QR_ERR_NOT_KANJI;
		break;
	}
	if (pos != -1) {
		qrSetErrorInfo3(qr, err, " at offset %d", pos);
		return FALSE;
	}
	qr->enclen += enclen;

	/*
	 * If the buffer capacity is insufficient, additional capacity is allocated.
	 */
	while (qr->srcmax < qr->srclen + size + 6) {
		qr->srcmax += QR_SRC_MAX;
		qr->source = (qr_byte_t *)realloc(qr->source, qr->srcmax);
		if (qr->source == NULL) {
			qr->srcmax = 0;
			qrSetErrorInfo2(qr, QR_ERR_MEMORY_EXHAUSTED, _QR_FUNCTION);
			return FALSE;
		}
	}

	/*
	 * Save data in a buffer
	 */
	qr->source[qr->srclen++] = (qr_byte_t)(mode | 0x80);
	qr->source[qr->srclen++] = (qr_byte_t)((size >> 24) & 0x7F);
	qr->source[qr->srclen++] = (qr_byte_t)((size >> 16) & 0xFF);
	qr->source[qr->srclen++] = (qr_byte_t)((size >> 8) & 0xFF);
	qr->source[qr->srclen++] = (qr_byte_t)(size & 0xFF);
	memcpy(&(qr->source[qr->srclen]), source, (size_t)size);
	qr->srclen += size;
	qr->source[qr->srclen] = '\0';

	qr->state = QR_STATE_SET;
	return TRUE;
}

/*
 * Add data to the last QR code object in the structural concatenation
 */
QR_API int
qrsAddData(QRStructured *st, const qr_byte_t *source, int size)
{
	if (st->state == QR_STATE_FINAL) {
		qrSetErrorInfo(st->cur, QR_ERR_STATE, _QR_FUNCTION);
		return FALSE;
	}
	return qrsAddData2(st, source, size, st->param.mode);
}

/*
 * Add data to the last QR code object in the structural concatenation by specifying the encoding mode.
 */
QR_API int
qrsAddData2(QRStructured *st, const qr_byte_t *source, int size, int mode)
{
	int enclen, maxlen, limit, remain;
	int sizes[QR_STA_MAX] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int i, j;
	int p;

	if (st->state == QR_STATE_FINAL) {
		qrSetErrorInfo(st->cur, QR_ERR_STATE, _QR_FUNCTION);
		return FALSE;
	}

	if (size <= 0) {
		qrSetErrorInfo(st->cur, QR_ERR_EMPTY_SRC, NULL);
		return FALSE;
	}

	/*
	 * Choose the best encoding mode for your input data
	 */
	if (mode == QR_EM_AUTO) {
		mode = qrDetectDataType(source, size);
	}

	/*
	 * Calculate remaining data capacity
	 */
	maxlen = 8 * qr_vertable[st->param.version].ecl[st->param.eclevel].datawords;
	limit = maxlen - QR_STA_LEN;
	if (!qrHasData(st->cur)) {
		remain = limit;
	} else {
		remain = qrRemainedDataBits(st->cur);
	}

	/*
	 * Calculate the length of the encoded data
	 */
	enclen = qrGetEncodedLength2(st->cur, size, mode);
	if (enclen == -1) {
		return FALSE;
	}
	j = 0;
	if (enclen > remain) {
		int l, r, s;
		r = remain;
		s = size;
		for (i = 0; i <= st->max - st->num; i++) {
			j++;
			l = qrGetEncodableLength2(st->cur, r, mode);
			if (s <= l) {
				sizes[i] = s;
				s = 0;
				break;
			} else {
				sizes[i] = l;
				s -= l;
				r = limit;
			}
		}
		if (s > 0) {
			int snum, reqlen;
			snum = (enclen + maxlen - 1) / maxlen - (st->num - 1);
			reqlen = maxlen * (st->num - 1);
			reqlen += maxlen - remain;
			reqlen += enclen;
			reqlen += qr_vertable[st->param.version].nlen[mode] * (snum - 1) + QR_STA_LEN * snum;
			qrSetErrorInfo3(st->cur, QR_ERR_LARGE_SRC, ", %d total encoded bits"
					" (max %d bits on version=%d, ecl=%s, num=%d)",
					reqlen, maxlen * st->max,
					st->param.version, qr_eclname[st->param.eclevel], st->max);
			return FALSE;
		}
	} else {
		j = 1;
		sizes[0] = size;
	}

	/*
	 * Encode input data
	 */
	p = 0;
	i = 0;
	while (i < j) {
		if (sizes[i] == 0 && i != 0) {
			break;
		}
		if (!qrHasData(st->cur)) {
			/*
			 * Initialize the data codeword and add a temporary structured concatenation header.
			 */
			qrInitDataWord(st->cur);
			qrAddDataBits(st->cur, 4, 0);
			qrAddDataBits(st->cur, 4, 0);
			qrAddDataBits(st->cur, 4, 0);
			qrAddDataBits(st->cur, 8, 0);
		}
		if (sizes[i] != 0) {
			st->cur->enclen += qrGetEncodedLength2(st->cur, sizes[i], mode);
			if (qrEncodeDataWord(st->cur, source + p, sizes[i], mode) == TRUE) {
				st->cur->state = QR_STATE_SET;
				st->state = QR_STATE_SET;
			} else {
				return FALSE;
			}
			p += sizes[i];
		}
		i++;
		if (i < j && sizes[i] > 0) {
			/*
			 * Initialize the following QR code object:
			 */
			int errcode;
			st->qrs[st->num] = qrInit(st->param.version, st->param.mode,
					st->param.eclevel, st->param.masktype, &errcode);
			if (st->qrs[st->num] == NULL) {
				qrSetErrorInfo(st->cur, errcode, NULL);
				return FALSE;
			}
			st->cur = st->qrs[st->num];
			st->num++;
		}
	}

	/*
	 * calculate parity
	 */
	p = 0;
	while (p < size) {
		st->parity ^= source[p++];
	}

	return TRUE;
}

/*
 * Initialize data codeword
 */
static int
qrInitDataWord(QRCode *qr)
{
	/*
	 * Clear the data code word area to zero
	 */
	memset(qr->dataword, '\0', QR_DWD_MAX);

	/*
	 * The addition position is the most significant bit of byte 0.
	 */
	qr->dwpos = 0;
	qr->dwbit = 7;

	return TRUE;
}

/*
 * Encode a data codeword
 */
static int
qrEncodeDataWord(QRCode *qr, const qr_byte_t *source, int size, int mode)
{
	int p = 0;
	int e = QR_ERR_NONE;
	int n = 0;
	int word = 0;
	int dwpos = qr->dwbit;
	int dwbit = qr->dwpos;

	if (mode < QR_EM_NUMERIC || mode >= QR_EM_COUNT) {
		e = QR_ERR_INVALID_MODE;
		goto err;
	}

	/*
	 * Add a mode indicator (4 bits)
	 */
	qrAddDataBits(qr, 4, qr_modeid[mode]);

	/*
	 * Adds a character count indicator (8 to 16 bits)
	 * The number of bits varies depending on the model and mode
	 */
	if (mode == QR_EM_KANJI) {
		qrAddDataBits(qr, qr_vertable[qr->param.version].nlen[mode], size / 2);
	} else {
		qrAddDataBits(qr, qr_vertable[qr->param.version].nlen[mode], size);
	}

	/*
	 * encode input data
	 */
	switch (mode) {
	  case QR_EM_NUMERIC:
		/*
		 * Number mode
		 * Converts every three digits into a 10-bit binary number
		 * The remainder is 4 bits if it is 1 digit, and 7 bits if it is 2 digits
		 */
		while (p < size) {
			qr_byte_t q = source[p];
			if (q < '0' || q > '9') {
				/* 数字でない */
				e = QR_ERR_NOT_NUMERIC;
				goto err;
			}
			word = word * 10 + (q - '0');
			/*
			 * When three digits are accumulated, add 10 bits.
			 */
			if (++n >= 3) {
				qrAddDataBits(qr, 10, word);
				n = 0;
				word = 0;
			}
			p++;
		}
		/*
		 * Add the remaining digits
		 */
		if (n == 1) {
			qrAddDataBits(qr, 4, word);
		} else if (n == 2) {
			qrAddDataBits(qr, 7, word);
		}
		break;

	  case QR_EM_ALNUM:
		/*
		 * Alphanumeric mode
		 * Converts two digits at a time into an 11-bit binary number
		 * Converts the remainder as a 6-bit number
		 */
		while (p < size) {
			signed char q = qr_alnumtable[source[p]];
			if (q == -1) {
				/* Not an encodable alphanumeric character */
				e = QR_ERR_NOT_ALNUM;
				goto err;
			}
			word = word * 45 + (int)q;
			/*
			 * When two digits are accumulated, add 11 bits.
			 */
			if (++n >= 2) {
				qrAddDataBits(qr, 11, word);
				n = 0;
				word = 0;
			}
			p++;
		}
		/*
		 * Add the remaining digits
		 */
		if (n == 1) {
			qrAddDataBits(qr, 6, word);
		}
		break;

	  case QR_EM_8BIT:
		/*
		 * 8-bit byte mode
		 * Add each byte directly as an 8-bit value
		 */
		while (p < size) {
			qrAddDataBits(qr, 8, (int)source[p++]);
		}
		break;

	  case QR_EM_KANJI:
		/*
		 * Kanji mode
		 * Convert 2 bytes to 13 bits and add
		 */
		while (p < size - 1) {
			qr_byte_t x, y;
			/*
			 * Processing the first byte
			 * If it is 0x81-0x9f, subtract 0x81
             * If it is 0xe0, 0xea, subtract 0xc1
			 */
			x = source[p++];
			if (x >= 0x81 && x <= 0x9f) {
				x -= 0x81;
			} else if (x >= 0xe0 && x <= 0xea) {
				x -= 0xc1;
			} else {
				/* Not the first byte of JIS X 0208 Kanji */
				p -= 1;
				e = QR_ERR_NOT_KANJI;
				goto err;
			}
			/*
			 * Process the second byte
			 * Subtract 0x40
			 */
			y = source[p++];
			if (y >= 0x40 && y <= 0xfc) {
				y -= 0x40;
			} else {
				/* Not the second byte of JIS X 0208 Kanji */
				p -= 1;
				e = QR_ERR_NOT_KANJI;
				goto err;
			}
			/*
			 * Add the result as a 13-bit value
			 */
			word = (int)qr_dwtable_kanji[x][y];
			if (word == -1) {
				/* JIS X 0208 Undefined areas of Chinese characters */
				p -= 2;
				e = QR_ERR_NOT_KANJI;
				goto err;
			}
			qrAddDataBits(qr, 13, word);
		}
		if (p < size) {
			/*
			 * Extra bytes at the end
			 */
			e = QR_ERR_NOT_KANJI;
			goto err;
		}
		break;

	  default:
		e = QR_ERR_INVALID_MODE;
		goto err;
	}

	return TRUE;

  err:
	qr->dwpos = dwpos;
	qr->dwbit = dwbit;
	if (e == QR_ERR_INVALID_MODE) {
		qrSetErrorInfo(qr, e, NULL);
	} else {
		qrSetErrorInfo3(qr, e, " at offset %d", p);
	}
	return FALSE;
}

/*
 * Fill in the remainder of the data code word
 */
static int
qrFinalizeDataWord(QRCode *qr)
{
	int n, m;
	int word;

	/*
	 * Add a termination pattern (up to 4 zeros)
	 */
	n = qrRemainedDataBits(qr);
	if (n < 4) {
		qrAddDataBits(qr, n, 0);
		n = 0;
	} else {
		qrAddDataBits(qr, 4, 0);
		n -= 4;
	}
	/*
	 * If all bits of the last data code word are not filled,
	 * fill the remainder with padding bits (0).
	 */
	m = n % 8;
	if (m > 0) {
		qrAddDataBits(qr, m, 0);
		n -= m;
	}

	/*
	 * Fill the remaining data code words with padding code words 1 and 2 alternately.
	 */
	word = PADWORD1;
	while (n >= 8) {
		qrAddDataBits(qr, 8, word);
		if (word == PADWORD1) {
			word = PADWORD2;
		} else {
			word = PADWORD1;
		}
		n -= 8;
	}

	return TRUE;
}

/*
 * Adding a bit string to a data code word
 */
static void
qrAddDataBits(QRCode *qr, int n, int word)
{
	/*
	 * Processes from the most significant bit (slow as it processes bit by bit)
	 */
	while (n-- > 0) {
		/*
		 * OR the nth lowest bit of the data into the bit addition position
		 */
		qr->dataword[qr->dwpos] |= ((word >> n) & 1) << qr->dwbit;
		/*
		 * Advance to the next bit addition position
		 */
		if (--qr->dwbit < 0) {
			qr->dwpos++;
			qr->dwbit = 7;
		}
	}
}

/*
 * Returns the number of remaining bits in a data code word.
 */
QR_API int
qrRemainedDataBits(QRCode *qr)
{
	int version;
	version = (qr->param.version == -1) ? QR_VER_MAX : qr->param.version;
	return (qr_vertable[version].ecl[qr->param.eclevel].datawords - qr->dwpos) * 8 - (7 - qr->dwbit);
}

/*
 * Calculate the error-correcting codeword for each RS block
 */
static int
qrComputeECWord(QRCode *qr)
{
	int i, j, k, m;
	int ecwtop, dwtop, nrsb, rsbnum;
	qr_byte_t rswork[QR_RSD_MAX];

	/*
	 * read the data codewords by RS block
	 * calculate the error correction codeword for each
	 * the RS blocks are divided into nrsb types based on their length
	 * there are rsbnum blocks for each length
	 */
	dwtop = 0;
	ecwtop = 0;
	nrsb = qr_vertable[qr->param.version].ecl[qr->param.eclevel].nrsb;
#define rsb qr_vertable[qr->param.version].ecl[qr->param.eclevel].rsb
#define gfvector qr_gftable[ecwlen]
	for (i = 0; i < nrsb; i++) {
		int dwlen, ecwlen;
		/*unsigned char *gfvector;*/
		/*qr_rsblock_t *rsbp;*/
		/*
		 * the number of RS blocks of this length (rsbnum)
		 * the length of the data code word in the RS block (dwlen)
		 * the length of the error correction code word (ecwlen) are calculated
		 * also, from the length of the error correction code word, the error correction generator polynomial (gfvector) to be used is selected.
		 */
		/*rsbp = &(qr_vertable[qr->param.version].ecl[qr->param.eclevel].rsb[i]);
		rsbnum = rsbp->rsbnum;
		dwlen = rsbp->datawords;
		ecwlen = rsbp->totalwords - rsbp->datawords;*/
		rsbnum = rsb[i].rsbnum;
		dwlen = rsb[i].datawords;
		ecwlen = rsb[i].totalwords - rsb[i].datawords;
		/*gfvector = qr_gftable[ecwlen];*/
		/*
		 * for each RS block, divide the data codeword by the error correction generator polynomial and use the result as the error correction codeword.
		 */
		for (j = 0; j < rsbnum; j++) {
			/*
			 * clear the work area for RS code calculation
			 * and treat the data code word of the RS block
			 * as a polynomial coefficient and put it into the work area
			 * (The size of the work area needs to be the same as the data code word or error correction code word of the RS block, whichever is longer)
			 */
			memset(&(rswork[0]), '\0', QR_RSD_MAX);
			memcpy(&(rswork[0]), &(qr->dataword[dwtop]), (size_t)dwlen);
			/*
			 * Perform polynomial division
			 * (for each degree, find the multiplier for the error correction generating polynomial from the coefficient of the first term of the data code word and repeat the process of finding the remainder by subtracting the polynomials)
			 */
			for (k = 0; k < dwlen; k++) {
				int e;
				if (rswork[0] == 0) {
					/*
					 * Since the coefficient of the first term is zero, shift each term coefficient to the left to proceed to the next order.
					 */
					for (m = 0; m < QR_RSD_MAX-1; m++) {
						rswork[m] = rswork[m+1];
					}
					rswork[QR_RSD_MAX-1] = 0;
					continue;
				}
				/*
				 * From the coefficient of the first term of the data code word (integer expression)
				 * find the multiplier (power expression) for the error correction generating polynomial
				 * shift the coefficient of each term of the data code word to the left to find the remainder for each remaining term
				 */
				e = qr_fac2exp[rswork[0]];
				for (m = 0; m < QR_RSD_MAX-1; m++) {
					rswork[m] = rswork[m+1];
				}
				rswork[QR_RSD_MAX-1] = 0;
				/*
				 * Multiply each term coefficient of the error correction generating polynomial by the multiplier calculated above (by adding the power expressions), 
				 * subtract it from each term of the data code word (by exclusive ORing the integer expressions), and calculate the remainder.
				 */
				for (m = 0; m < ecwlen; m++) {
					rswork[m] ^= qr_exp2fac[(gfvector[m] + e) % 255];
				}
			}
			/*
			 * The remainder of the polynomial division is used as the error-correcting code for the RS block.
			 */
			memcpy(&(qr->ecword[ecwtop]), &(rswork[0]), (size_t)ecwlen);
			/*
			 * Move the data code word read position and the error correction code word write position to the start position of the next RS block.
			 */
			dwtop += dwlen;
			ecwtop += ecwlen;
		}
	}
#undef rsb
#undef gfvector
	return TRUE;
}

/*
 * Generate the final codeword from the data codeword and the error correction codeword.
 */
static int
qrMakeCodeWord(QRCode *qr)
{
	int i, j, k, cwtop, pos;
	int dwlenmax, ecwlenmax;
	int dwlen, ecwlen, nrsb;
	/*qr_rsblock_t *rsb;*/

	/*
	 * Obtain the number of RS block size types (nrsb),
	 * the maximum number of data code words in an RS block (dwlenmax),
	 * and the number of error correction code words (ecwlenmax)
	 */
	nrsb = qr_vertable[qr->param.version].ecl[qr->param.eclevel].nrsb;
	/*rsb = qr_vertable[qr->param.version].ecl[qr->param.eclevel].rsb;*/
#define rsb qr_vertable[qr->param.version].ecl[qr->param.eclevel].rsb
	dwlenmax = rsb[nrsb-1].datawords;
	ecwlenmax = rsb[nrsb-1].totalwords - rsb[nrsb-1].datawords;
	/*
	 * Extract the data codeword from each RS block in order
	 * add it to the codeword area (qr->codeword)
	 */
	cwtop = 0;
	for (i = 0; i < dwlenmax; i++) {
		pos = i;
		/*
		 * Perform processing for each RS block size
		 */
		for (j = 0; j < nrsb; j++) {
			dwlen = rsb[j].datawords;
			/*
			 * RS blocks of the same size are processed in order.
			 */
			for (k = 0; k < rsb[j].rsbnum; k++) {
				/*
				 * Add the i-th byte of data
				 * of each RS block to the code word area
				 * (skip RS blocks that have already extracted all data code words)
				 */
				if (i < dwlen) {
					qr->codeword[cwtop++] = qr->dataword[pos];
				}
				/*
				 * Go to the i-th byte of the next RS block
				 */
				pos += dwlen;
			}
		}
	}
	/*
	 * Extract the error-correcting codeword from each RS block in turn
	 * add it to the codeword area (qr->codeword)
	 */
	for (i = 0; i < ecwlenmax; i++) {
		pos = i;
		/*
		 * Perform processing for each RS block size
		 */
		for (j = 0; j < nrsb; j++) {
			ecwlen = rsb[j].totalwords - rsb[j].datawords;
			/*
			 * RS blocks of the same size are processed in order.
			 */
			for (k = 0; k < rsb[j].rsbnum; k++) {
				/*
				 * Add the i-th byte of error correction
				 * codeword to the codeword area
				 * (skip RS blocks that have already removed all error correction codewords)
				 */
				if (i < ecwlen) {
					qr->codeword[cwtop++] = qr->ecword[pos];
				}
				/*
				 * Go to the i-th byte of the next RS block
				 */
				pos += ecwlen;
			}
		}
	}
#undef rsb
#undef nrsb
	return TRUE;
}

/*
 * Initialize symbols and place functional patterns
 */
static int
qrFillFunctionPattern(QRCode *qr)
{
	int i, j, n, dim, xpos, ypos;

	/*
	 * Find the length of one side of the symbol
	 */
	dim = qr_vertable[qr->param.version].dimension;
	/*
	 * Clear the entire symbol
	 */
	qrFree(qr->symbol);
	qrFree(qr->_symbol);
	qr->_symbol = (qr_byte_t *)calloc((size_t)dim, (size_t)dim);
	if (qr->_symbol == NULL) {
		return FALSE;
	}
	qr->symbol = (qr_byte_t **)malloc(sizeof(qr_byte_t *) * (size_t)dim);
	if (qr->symbol == NULL) {
		free(qr->_symbol);
		return FALSE;
	}
	for (i = 0; i < dim; i++) {
		qr->symbol[i] = qr->_symbol + dim * i;
	}
	/*
	 * Place the finder pattern in the top left, top right, and bottom left corners
	 */
	for (i = 0; i < QR_DIM_FINDER; i++) {
		for (j = 0; j < QR_DIM_FINDER; j++) {
			qr->symbol[i][j] = qr_finderpattern[i][j];
			qr->symbol[i][dim-1-j] = qr_finderpattern[i][j];
			qr->symbol[dim-1-i][j] = qr_finderpattern[i][j];
		}
	}
	/*
	 * Place the separation pattern of the position finder pattern
	 */
	for (i = 0; i < QR_DIM_FINDER+1; i++) {
		qr->symbol[i][QR_DIM_FINDER] = QR_MM_FUNC;
		qr->symbol[QR_DIM_FINDER][i] = QR_MM_FUNC;
		qr->symbol[i][dim-1-QR_DIM_FINDER] = QR_MM_FUNC;
		qr->symbol[dim-1-QR_DIM_FINDER][i] = QR_MM_FUNC;
		qr->symbol[dim-1-i][QR_DIM_FINDER] = QR_MM_FUNC;
		qr->symbol[QR_DIM_FINDER][dim-1-i] = QR_MM_FUNC;
	}
	/*
	 * Positioning the Alignment Pattern
	 */
	n = qr_vertable[qr->param.version].aplnum;
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			int x, y, x0, y0, xcenter, ycenter;
			/*
			 * Find the center and top left coordinates of the alignment pattern
			 */
			ycenter = qr_vertable[qr->param.version].aploc[i];
			xcenter = qr_vertable[qr->param.version].aploc[j];
			y0 = ycenter - QR_DIM_ALIGN / 2;
			x0 = xcenter - QR_DIM_ALIGN / 2;
			if (qrIsFunc(qr, ycenter, xcenter)) {
				/*
				 * Do not place if it overlaps with the position finder pattern
				 */
				continue;
			}
			for (y = 0; y < QR_DIM_ALIGN; y++) {
				for (x = 0; x < QR_DIM_ALIGN; x++) {
					qr->symbol[y0+y][x0+x] = qr_alignpattern[y][x];
				}
			}
		}
	}
	/*
	 * Placing a Timing Pattern
	 */
	for (i = QR_DIM_FINDER; i < dim-1-QR_DIM_FINDER; i++) {
		qr->symbol[i][QR_DIM_TIMING] = QR_MM_FUNC;
		qr->symbol[QR_DIM_TIMING][i] = QR_MM_FUNC;
		if ((i & 1) == 0) {
			qr->symbol[i][QR_DIM_TIMING] |= QR_MM_BLACK;
			qr->symbol[QR_DIM_TIMING][i] |= QR_MM_BLACK;
		}
	}
	/*
	 * Reserve space for format information
	 */
	for (i = 0; i < 2; i++) {
		for (j = 0; j < QR_FIN_MAX; j++) {
			xpos = (qr_fmtinfopos[i][j].xpos + dim) % dim;
			ypos = (qr_fmtinfopos[i][j].ypos + dim) % dim;
			qr->symbol[ypos][xpos] |= QR_MM_FUNC;
		}
	}
	xpos = (qr_fmtblackpos.xpos + dim) % dim;
	ypos = (qr_fmtblackpos.ypos + dim) % dim;
	qr->symbol[ypos][xpos] |= QR_MM_FUNC;
	/*
	 * If the model number information is valid (model number 7 or higher),
	 * Reserve the area for the model number information
	 */
	if (qr_verinfo[qr->param.version] != -1L) {
		for (i = 0; i < 2; i++) {
			for (j = 0; j < QR_VIN_MAX; j++) {
				xpos = (qr_verinfopos[i][j].xpos + dim) % dim;
				ypos = (qr_verinfopos[i][j].ypos + dim) % dim;
				qr->symbol[ypos][xpos] |= QR_MM_FUNC;
			}
		}
	}

	return TRUE;
}

/*
 * Place the encoded codeword into the symbol
 */
static int
qrFillCodeWord(QRCode *qr)
{
	int i, j;

	/*
	 * Start from the bottom right corner of the symbol
	 */
	qrInitPosition(qr);
	/*
	 * For every byte in the codeword area...
	 */
	for (i = 0; i < qr_vertable[qr->param.version].totalwords; i++) {
		/*
		 * Check each bit starting from the most significant bit...
		 */
		for (j = 7; j >= 0; j--) {
			/*
			 * If the bit is 1, place a black module.
			 */
			if ((qr->codeword[i] & (1 << j)) != 0) {
				qr->symbol[qr->ypos][qr->xpos] |= QR_MM_DATA;
			}
			/*
			 * Move to the next module placement position
			 */
			qrNextPosition(qr);
		}
	}

	return TRUE;
}

/*
 * Determine the initial position and direction of module placement
 */
static void
qrInitPosition(QRCode *qr)
{
	/*
	 * Start placement from the bottom right corner of the symbol
	 */
	qr->xpos = qr->ypos = qr_vertable[qr->param.version].dimension - 1;
	/*
	 * First movement direction is left, then upwards
	 */
	qr->xdir = -1;
	qr->ydir = -1;
}

/*
 * Decide where to place the next module
 */
static void
qrNextPosition(QRCode *qr)
{
	do {
		/*
		 * Move one module in the qr->xdir direction
		 * Reverse the direction of qr->xdir
		 * When moving to the right, also move one module in the qr->ydir direction
		 */
		qr->xpos += qr->xdir;
		if (qr->xdir > 0) {
			qr->ypos += qr->ydir;
		}
		qr->xdir = -qr->xdir;
		/*
		 * If the symbol overflows in the y direction,
    	 * move it two modules to the left in the x direction instead of the y direction,
	 	 * and reverse the direction of qr->ydir
		 * If qr->xpos is on the vertical timing pattern,
		 * move it one more module to the left
		 */
		if (qr->ypos < 0 || qr->ypos >= qr_vertable[qr->param.version].dimension) {
			qr->ypos -= qr->ydir;
			qr->ydir = -qr->ydir;
			qr->xpos -= 2;
			if (qr->xpos == QR_DIM_TIMING) {
				qr->xpos--;
			}
		}
		/*
		 * If the new position is on the functional pattern,
		 * move it aside and search for the next candidate position
		 */
	} while (qrIsFunc(qr, qr->ypos, qr->xpos));
}

/*
 * Mask the symbol with the optimal mask pattern
 */
static int
qrSelectMaskPattern(QRCode *qr)
{
	int type;
	long penalty, xpenalty;

	if (qr->param.masktype >= 0) {
		/*
		 * A mask pattern was specified as an argument, so mask with that pattern and exit.
		 */
		return qrApplyMaskPattern(qr);
	}
	/*
	 * Evaluate all mask patterns
	 */
	xpenalty = -1L;
	for (type = 0; type < QR_MPT_MAX; type++) {
		/*
		 * Mask with the mask pattern and evaluate
		 */
		qrApplyMaskPattern2(qr, type);
		penalty = qrEvaluateMaskPattern(qr);
		/*
		 * Record if the number of goals conceded is lower than before
		 */
		if (xpenalty == -1L || penalty < xpenalty) {
			qr->param.masktype = type;
			xpenalty = penalty;
		}
	}
	/*
	 * Mask with the pattern that will result in the least number of runs conceded
	 */
	return qrApplyMaskPattern(qr);
}

/*
 * Mask the symbol with the mask pattern of the configured reference
 */
static int
qrApplyMaskPattern(QRCode *qr)
{
	return qrApplyMaskPattern2(qr, qr->param.masktype);
}

/*
 * Masks the symbol with the mask pattern of the specified reference.
 */
static int
qrApplyMaskPattern2(QRCode *qr, int type)
{

	int i, j, dim;

	if (type < 0 || type >= QR_MPT_MAX) {
		qrSetErrorInfo3(qr, QR_ERR_INVALID_MPT, "%d", type);
		return FALSE;
	}

	dim = qr_vertable[qr->param.version].dimension;
	/*
	 * Clear the previous mask pattern and use the encoded data as the initial pattern.
	 */
	for (i = 0; i < dim; i++) {
		for (j = 0; j < dim; j++) {
			/*
			 * Printing black modules in the functional pattern area are left intact
			 */
			if (qrIsFunc(qr, i, j)) {
				continue;
			}
			/*
			 * The encoded data area is encoded data
			 * Black modules are printed black modules	
			 */
			if (qrIsData(qr, i, j)) {
				qr->symbol[i][j] |= QR_MM_BLACK;
			} else {
				qr->symbol[i][j] &= ~QR_MM_BLACK;
			}
		}
	}
	/*
	 * About the module in row i and column j...
	 */
	for (i = 0; i < dim; i++) {
		for (j = 0; j < dim; j++) {
			if (qrIsFunc(qr, i, j)) {
				/*
				 * Functional pattern areas (and format information, model number information) are excluded from masking.
				 */
				continue;
			}
			/*
			 * Flip modules that meet the specified criteria
			 */
			if ((type == 0 && (i + j) % 2 == 0) ||
				(type == 1 && i % 2 == 0) ||
				(type == 2 && j % 3 == 0) ||
				(type == 3 && (i + j) % 3 == 0) ||
				(type == 4 && ((i / 2) + (j / 3)) % 2 == 0) ||
				(type == 5 && (i * j) % 2 + (i * j) % 3 == 0) ||
				(type == 6 && ((i * j) % 2 + (i * j) % 3) % 2 == 0) ||
				(type == 7 && ((i * j) % 3 + (i + j) % 2) % 2 == 0))
			{
				qr->symbol[i][j] ^= QR_MM_BLACK;
			}
		}
	}

	return TRUE;
}

/*
 * Evaluates the mask pattern and returns the evaluation value.
 */
static long
qrEvaluateMaskPattern(QRCode *qr)
{
	int i, j, m, n, dim;
	long penalty;

	/*
	 * The evaluation value is multiplied by the penalty.
	 * Masking is done only for the coding area,
	 * but evaluation is done for the entire symbol.
	 */
	penalty = 0L;
	dim = qr_vertable[qr->param.version].dimension;
	/*
	 * Characteristics: Adjacent modules in the same row/column
	 * Evaluation criteria: Number of modules = (5+i)
	 * Loss: 3+i
	 */
	for (i = 0; i < dim; i++) {
		n = 0;
		for (j = 0; j < dim; j++) {
			if (j > 0 && qrIsBlack(qr, i, j) == qrIsBlack(qr, i, j-1)) {
				/*
				 * Module of the same color as the one immediately to the left
				 * Increase the length of the row of the same color by 1
				 */
				n++;
			} else {
				/*
				 * The color has changed. 
				 * Evaluate the length of the same-colored sequence that just ended.
				 */
				if (n >= 5) {
					penalty += (long)(3 + (n - 5));
				}
				n = 1;
			}
		}
		/*
		 * The sequence is over
		 * Evaluate the length of the same-color sequence that just ended
		 */
		if (n >= 5) {
			penalty += (long)(3 + (n - 5));
		}
	}
	for (i = 0; i < dim; i++) {
		n = 0;
		for (j = 0; j < dim; j++) {
			if (j > 0 && qrIsBlack(qr, j, i) == qrIsBlack(qr, j-1, i)) {
				/*
				 * Module of the same color as the one immediately above
				 * Increase the length of the row of the same color by 1
				 */
				n++;
			} else {
				/*
				 * The color has changed. 
				 * Evaluate the length of the same-colored sequence that just ended.
				 */
				if (n >= 5) {
					penalty += (long)(3 + (n - 5));
				}
				n = 1;
			}
		}
		/*
		 * The sequence is over
		 * Evaluate the length of the same-color sequence that just ended
		 */
		if (n >= 5) {
			penalty += (long)(3 + (n - 5));
		}
	}
	/*
	 * Characteristics: Module blocks of the same color
	 * Evaluation conditions: Block size = 2×2
	 * Losses: 3
	 */
	for (i = 0; i < dim - 1; i++) {
		for (j = 0; j < dim - 1; j++) {
			if (qrIsBlack(qr, i, j) == qrIsBlack(qr, i, j+1) &&
				qrIsBlack(qr, i, j) == qrIsBlack(qr, i+1, j) &&
				qrIsBlack(qr, i, j) == qrIsBlack(qr, i+1, j+1))
			{
				/*
				 * There was a 2x2 block of the same color.
				 */
				penalty += 3L;
			}
		}
	}
	/*
	 * Characteristics: A pattern with a 1:1:3:1:1 ratio (dark:light:dark:light:dark) in rows/columns
	 * Followed by a light pattern with a width of 4 or more
	 * Points lost: 40
	 * The pattern before and after must be outside the symbol boundary or a light module
	 * Whether or not a pattern like 2:2:6:2:2 should also be penalized
	 * It is not clear from the JIS standard. It is not given here
	 */
	for (i = 0; i < dim; i++) {
		for (j = 0; j < dim - 6; j++) {
			if ((j == 0 || !qrIsBlack(qr, i, j-1)) &&
				qrIsBlack(qr, i, j+0) &&
				!qrIsBlack(qr, i, j+1) &&
				qrIsBlack(qr, i, j+2) &&
				qrIsBlack(qr, i, j+3) &&
				qrIsBlack(qr, i, j+4) &&
				!qrIsBlack(qr, i, j+5) &&
				qrIsBlack(qr, i, j+6))
			{
				int k, l;
				l = 1;
				for (k = 0; k < dim - j - 7 && k < 4; k++) {
					if (qrIsBlack(qr, i, j + k + 7)) {
						l = 0;
						break;
					}
				}
				/*
				 * There was a pattern
				 */
				if (l) {
					penalty += 40L;
				}
			}
		}
	}
	for (i = 0; i < dim; i++) {
		for (j = 0; j < dim - 6; j++) {
			if ((j == 0 || !qrIsBlack(qr, j-1, i)) &&
				qrIsBlack(qr, j+0, i) &&
				!qrIsBlack(qr, j+1, i) &&
				qrIsBlack(qr, j+2, i) &&
				qrIsBlack(qr, j+3, i) &&
				qrIsBlack(qr, j+4, i) &&
				!qrIsBlack(qr, j+5, i) &&
				qrIsBlack(qr, j+6, i) &&
				(j == dim-7 || !qrIsBlack(qr, j+7, i)))
			{
				int k, l;
				l = 1;
				for (k = 0; k < dim - j - 7 && k < 4; k++) {
					if (qrIsBlack(qr, j + k + 7, i)) {
						l = 0;
						break;
					}
				}
				/*
				 * There was a pattern
				 */
				if (l) {
					penalty += 40L;
				}
			}
		}
	}
	/*
	 * Characteristics: Percentage of dark modules in the total
	 * Evaluation conditions: 50±(5×k)% to 50±(5×(k＋1))%
	 * Loss: 10×k
	 */
	m = 0;
	n = 0;
	for (i = 0; i < dim; i++) {
		for (j = 0; j < dim; j++) {
			m++;
			if (qrIsBlack(qr, i, j)) {
				n++;
			}
		}
	}
	penalty += (long)(abs((n * 100 / m) - 50) / 5 * 10);
	return penalty;
}

/*
 * Placing model and part number information on the symbol
 */
static int
qrFillFormatInfo(QRCode *qr)
{
	int i, j, dim, fmt, modulo, xpos, ypos;
	long v;

	dim = qr_vertable[qr->param.version].dimension;
	/*
	 * Calculate the format information
	 * 2 bits of error correction level (L:01, M:00, Q:11, H:10)
	 * 3 bits of mask pattern reference code are used for a total of 5 bits
	 * Using Bose-Chaudhuri-Hocquenghem (15,5) code
	 * 10 error correction bits are added to make 15 bits
	 * (The 5 bits are treated as polynomial coefficients of degree 14 to 10 of x
	 * Polynomial x^10+x^8+x^5+x^4+x^2+x+1 (coefficient 10100110111)
	 * Divide by and the 10 bits of remainder are used as error correction bits)
	 * Furthermore, XOR with 101010000010010 (0x5412) so that all bits do not become zero
	 */
	fmt = ((qr->param.eclevel ^ 1) << 3) | qr->param.masktype;
	modulo = fmt << 10;
	for (i = 14; i >= 10; i--) {
		if ((modulo & (1 << i)) == 0) {
			continue;
		}
		modulo ^= 0x537 << (i - 10);
	}
	fmt = ((fmt << 10) + modulo) ^ 0x5412;
	/*
	 * Putting formatting information on symbols
	 */
	for (i = 0; i < 2; i++) {
		for (j = 0; j < QR_FIN_MAX; j++) {
			if ((fmt & (1 << j)) == 0) {
				continue;
			}
			xpos = (qr_fmtinfopos[i][j].xpos + dim) % dim;
			ypos = (qr_fmtinfopos[i][j].ypos + dim) % dim;
			qr->symbol[ypos][xpos] |= QR_MM_BLACK;
		}
	}
	xpos = (qr_fmtblackpos.xpos + dim) % dim;
	ypos = (qr_fmtblackpos.ypos + dim) % dim;
	qr->symbol[ypos][xpos] |= QR_MM_BLACK;
	/*
	 * If the model number information is valid (model number 7 or higher),
	 * Place the model number information on the symbol
	 */
	v = qr_verinfo[qr->param.version];
	if (v != -1L) {
		for (i = 0; i < 2; i++) {
			for (j = 0; j < QR_VIN_MAX; j++) {
				if ((v & (1L << j)) == 0L) {
					continue;
				}
				xpos = (qr_verinfopos[i][j].xpos + dim) % dim;
				ypos = (qr_verinfopos[i][j].ypos + dim) % dim;
				qr->symbol[ypos][xpos] |= QR_MM_BLACK;
			}
		}
	}

	return TRUE;
}

/*
 * It performs all the processes in one go, from filling in the surplus bits of the data code word
 * to placing format information and model number information in the symbol
 */
QR_API int
qrFinalize(QRCode *qr)
{
	static qr_funcs funcs[] = {
		qrFinalizeDataWord,
		qrComputeECWord,
		qrMakeCodeWord,
		qrFillFunctionPattern,
		qrFillCodeWord,
		qrSelectMaskPattern,
		qrFillFormatInfo,
		NULL
	};
	int i = 0;
	int ret = TRUE;

	if (qrIsFinalized(qr)) {
		return TRUE;
	}

	/*
	 * Automatic model number selection
	 */
	if (qr->param.version == -1) {
		int maxlen, delta;
		int version = 0;
		while (++version <= QR_VER_MAX) {
			if (version <= VERPOINT1) {
				delta = qr->delta1;
			} else if (version <= VERPOINT2) {
				delta = qr->delta2;
			} else {
				delta = 0;
			}
			maxlen = 8 * qr_vertable[version].ecl[qr->param.eclevel].datawords;
			if (maxlen >= qr->enclen - delta) {
				break;
			}
		}
		if (version > QR_VER_MAX) {
			maxlen = 8 * qr_vertable[QR_VER_MAX].ecl[qr->param.eclevel].datawords;
			qrSetErrorInfo3(qr, QR_ERR_LARGE_SRC, ", %d total encoded bits"
					" (max %d bits on version=%d, ecl=%s)",
					qr->enclen, maxlen, QR_VER_MAX, qr_eclname[qr->param.eclevel]);
			return FALSE;
		}
		qr->param.version = version;
	}

	/*
	 * Register input data in the data code word
	 */
	if (qr->source != NULL) {
		qr_byte_t *source;
		int mode, size;

		qrInitDataWord(qr);
		source = qr->source;
		while ((mode = (int)(*source++)) != '\0') {
			mode ^= 0x80;
			size = ((int)*source++) << 24;
			size |= ((int)*source++) << 16;
			size |= ((int)*source++) << 8;
			size |= (int)*source++;
			if (qrEncodeDataWord(qr, source, size, mode) == FALSE) {
				return FALSE;
			}
			source += size;
		}

		qrFree(qr->source);
	}

	/*
	 * generate a symbol
	 */
	while (funcs[i] && ret == TRUE) {
		ret = funcs[i++](qr);
	}

	if (ret == TRUE) {
		qrFree(qr->dataword);
		qrFree(qr->ecword);
		qrFree(qr->codeword);
		qr->state = QR_STATE_FINAL;
	}
	return ret;
}

/*
 * Determine if it has been finalized
 */
QR_API int
qrIsFinalized(const QRCode *qr)
{
	if (qr->state == QR_STATE_FINAL) {
		return TRUE;
	}
	return FALSE;
}

/*
 * Determine if data has been set
 */
QR_API int qrHasData(const QRCode *qr)
{
	if (qr->state == QR_STATE_BEGIN) {
		return FALSE;
	}
	return TRUE;
}

/*
 * Overwrite the temporary structural link header of the last QR code object in the structural link with the correct information and finalize it.
 */
QR_API int
qrsFinalize(QRStructured *st)
{
	int m, n, r;

	if (!qrsHasData(st)) {
		qrSetErrorInfo(st->cur, QR_ERR_STATE, _QR_FUNCTION);
		return FALSE;
	} else if (qrsIsFinalized(st)) {
		return TRUE;
	}

	m = 0;
	n = st->num - 1;
	r = TRUE;
	while (m <= n && r == TRUE) {
		int dwpos, dwbit;
		dwpos = st->qrs[m]->dwpos;
		dwbit = st->qrs[m]->dwbit;
		st->qrs[m]->dwpos = 0;
		st->qrs[m]->dwbit = 7;
		qrAddDataBits(st->qrs[m], 4, 3);
		qrAddDataBits(st->qrs[m], 4, m);
		qrAddDataBits(st->qrs[m], 4, n);
		qrAddDataBits(st->qrs[m], 8, st->parity);
		st->qrs[m]->dwpos = dwpos;
		st->qrs[m]->dwbit = dwbit;
		r = qrFinalize(st->qrs[m]);
		m++;
	}

	if (r == TRUE) {
		st->state = QR_STATE_FINAL;
	}
	return r;
}

/*
 * Determine if it has been finalized
 */
QR_API int
qrsIsFinalized(const QRStructured *st)
{
	if (st->state == QR_STATE_FINAL) {
		return TRUE;
	}
	return FALSE;
}

/*
 * Determine if data has been set
 */
QR_API int qrsHasData(const QRStructured *st)
{
	if (st->state == QR_STATE_BEGIN) {
		return FALSE;
	}
	return TRUE;
}

/*
 * Convert the generated QR code symbol to the format specified by fmt 
 */
QR_API qr_byte_t *
qrGetSymbol(QRCode *qr, int fmt, int sep, int mag, int *size)
{
	qr_byte_t *buf;
	int _size;

	static const QRConverter cnv[QR_FMT_COUNT] = {
		qrSymbolToBMP,
		qrSymbolToPBM,
		qrSymbolToJSON,
		qrSymbolToDigit,
		qrSymbolToASCII
	};

	if (fmt < 0 || fmt >= QR_FMT_COUNT) {
		qrSetErrorInfo(qr, QR_ERR_INVALID_FMT, NULL);
		return NULL;
	}

	if (cnv[fmt] == NULL) {
		qrSetErrorInfo(qr, QR_ERR_UNSUPPORTED_FMT, NULL);
		return NULL;
	}

	buf = cnv[fmt](qr, sep, mag, &_size);
	if (buf == NULL) {
		return NULL;
	}

	if (size) {
		*size = _size;
	}
	return buf;
}

/*
 * Write the generated QR code symbol to the stream fp 
 */
QR_API int
qrOutputSymbol(QRCode *qr, FILE *fp, int fmt, int sep, int mag)
{
	qr_byte_t *buf;
	int size;

	buf = qrGetSymbol(qr, fmt, sep, mag, &size);
	if (buf == NULL) {
		return -1;
	}

	if (fp == NULL) {
		fp = stdout;
	}

	if (!fwrite(buf, (size_t)size, 1, fp)) {
		qrSetErrorInfo2(qr, QR_ERR_FWRITE, NULL);
		free(buf);
		return -1;
	}
	if (ferror(fp)) {
		qrSetErrorInfo(qr, QR_ERR_FWRITE, NULL);
		free(buf);
		return -1;
	}

	free(buf);

	return size;
}

/*
 * Write the generated QR code symbol to the file pathname
 */
QR_API int
qrOutputSymbol2(QRCode *qr, const char *pathname, int fmt, int sep, int mag)
{
	FILE *fp;
	int size;

	if (pathname == NULL || pathname[0] == '\0') {
		qrSetErrorInfo(qr, QR_ERR_EMPTY_PARAM, "(empty pathname)");
		return -1;
	}

	fp = fopen(pathname, "wb");
	if (fp == NULL) {
		qrSetErrorInfo2(qr, QR_ERR_FOPEN, pathname);
		return -1;
	}

	size = qrOutputSymbol(qr, fp, fmt, sep, mag);
	fclose(fp);

	return size;
}

/*
 * Convert all generated QR Code symbols to the format specified by fmt
 */
QR_API qr_byte_t *
qrsGetSymbols(QRStructured *st, int fmt, int sep, int mag, int order, int *size)
{
	qr_byte_t *buf;
	int _size;

	static QRsConverter cnv[QR_FMT_COUNT] = {
		qrsSymbolsToPBM,
		qrsSymbolsToJSON,
		qrsSymbolsToDigit,
		qrsSymbolsToASCII
	};

	if (fmt < 0 || fmt >= QR_FMT_COUNT) {
		qrSetErrorInfo(st->cur, QR_ERR_INVALID_FMT, NULL);
		return NULL;
	}

	buf = cnv[fmt](st, sep, mag, order, &_size);
	if (buf == NULL) {
		return NULL;
	}

	if (size) {
		*size = _size;
	}

	return buf;
}

/*
 * Write all generated QR code symbols to the stream fp
 */
QR_API int
qrsOutputSymbols(QRStructured *st, FILE *fp, int fmt, int sep, int mag, int order)
{
	qr_byte_t *buf;
	int size;


	buf = qrsGetSymbols(st, fmt, sep, mag, order, &size);
	if (buf == NULL) {
		return -1;
	}

	if (fp == NULL) {
		fp = stdout;
	}

	if (!fwrite(buf, (size_t)size, 1, fp)) {
		qrSetErrorInfo2(st->cur, QR_ERR_FWRITE, NULL);
		free(buf);
		return -1;
	}
	if (ferror(fp)) {
		qrSetErrorInfo(st->cur, QR_ERR_FWRITE, NULL);
		free(buf);
		return -1;
	}

	free(buf);

	return size;
}

/*
 * Write all generated QR Code symbols to the file pathname
 */
QR_API int
qrsOutputSymbols2(QRStructured *st, const char *pathname, int fmt, int sep, int mag, int order)
{
	FILE *fp;
	int size;

	if (pathname == NULL || pathname[0] == '\0') {
		qrSetErrorInfo(st->cur, QR_ERR_EMPTY_PARAM, "(empty pathname)");
		return -1;
	}

	fp = fopen(pathname, "wb");
	if (fp == NULL) {
		qrSetErrorInfo2(st->cur, QR_ERR_FOPEN, pathname);
		return -1;
	}

	size = qrsOutputSymbols(st, fp, fmt, sep, mag, order);
	fclose(fp);

	return size;
}
