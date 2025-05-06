/*
 * QR Code Generator Library: Basic Header
 *
 * Core routines were originally written by Junn Ohta.
 * Based on qr.c Version 0.1: 2004/4/3 (Public Domain)
 *
 * @package     libqr
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @copyright   2006-2013 Ryusuke SEKIYAMA
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 */

#ifndef _QR_H_
#define _QR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdio.h>

#if defined(WIN32) && !defined(QR_STATIC_BUILD)
#ifdef QR_DLL_BUILD
#define QR_API __declspec(dllexport)
#else
#define QR_API __declspec(dllimport)
#endif
#else
#define QR_API
#endif

/*
 * library version
 */
#define LIBQR_VERSION "0.3.1"

/*
 * error code
 */
typedef enum {
	/* Generic error code */
	QR_ERR_NONE             = 0,
	QR_ERR_USAGE            = 0x68,
	QR_ERR_NOT_IMPL         = 0x69,
	QR_ERR_SEE_ERRNO        = 0x6e,
	QR_ERR_FOPEN            = 0x6f,
	QR_ERR_FREAD            = 0x72,
	QR_ERR_STATE            = 0x73,
	QR_ERR_UNKNOWN          = 0x75,
	QR_ERR_FWRITE           = 0x77,
	QR_ERR_MEMORY_EXHAUSTED = 0x78,

	/* Error Code for Parameter */
	QR_ERR_INVALID_ARG     = 0x01,
	QR_ERR_INVALID_VERSION = 0x02,
	QR_ERR_INVALID_MODE    = 0x03,
	QR_ERR_INVALID_ECL     = 0x04,
	QR_ERR_INVALID_MPT     = 0x05,
	QR_ERR_INVALID_MAG     = 0x06,
	QR_ERR_INVALID_SEP     = 0x07,
	QR_ERR_INVALID_SIZE    = 0x08,
	QR_ERR_INVALID_FMT     = 0x09,
	QR_ERR_INVALID_OUT     = 0x0a,
	QR_ERR_INVALID_MAXNUM  = 0x0b,
	QR_ERR_UNSUPPORTED_FMT = 0x0c,
	QR_ERR_EMPTY_PARAM     = 0x0f,

	/* Error codes for input data */
	QR_ERR_EMPTY_SRC   = 0x10,
	QR_ERR_LARGE_SRC   = 0x11,
	QR_ERR_NOT_NUMERIC = 0x12,
	QR_ERR_NOT_ALNUM   = 0x13,
	QR_ERR_NOT_KANJI   = 0x14,

	/* Image processing error codes */
	QR_ERR_IMAGE_TOO_LARGE  = 0x30,
	QR_ERR_WIDTH_TOO_LARGE  = 0x31,
	QR_ERR_HEIGHT_TOO_LARGE = 0x32,
	QR_ERR_IMAGECREATE      = 0x33,
	QR_ERR_IMAGEFORMAT      = 0x34,
	QR_ERR_IMAGEFRAME       = 0x35,

	/* Error codes for zlib */
	QR_ERR_DEFLATE = 0x40
} qr_err_t;

/*
 * internal state
 */
#define QR_STATE_BEGIN  0
#define QR_STATE_SET    1
#define QR_STATE_FINAL  2

/*
 * encoding mode
 */
typedef enum {
	QR_EM_AUTO    = -1, /* automatic selection */
	QR_EM_NUMERIC =  0, /* number */
	QR_EM_ALNUM   =  1, /* English and numbers: 0-9 A-Z SP $%*+-./: */
	QR_EM_8BIT    =  2, /* 8 bit byte */
	QR_EM_KANJI   =  3  /* Chinese character */
} qr_em_t;

/* Total number of modes */
#define QR_EM_COUNT 4

/*
 * error correction level
 */
typedef enum {
	QR_ECL_L = 0, /* Level L */
	QR_ECL_M = 1, /* Level M */
	QR_ECL_Q = 2, /* Level Q */
	QR_ECL_H = 3  /* Level H */
} qr_ecl_t;

/* Total number of levels */
#define QR_ECL_COUNT 4

/*
 * Output form
 */
typedef enum {
	QR_FMT_PNG   =  0, /* PNG */
	QR_FMT_BMP   =  1, /* BMP */
	QR_FMT_TIFF  =  2, /* TIFF */
	QR_FMT_PBM   =  3, /* PBM */
	QR_FMT_SVG   =  4, /* SVG */
	QR_FMT_JSON  =  5, /* JSON */
	QR_FMT_DIGIT =  6, /* string */
	QR_FMT_ASCII =  7, /* ascii art */
	QR_FMT_UNAVAILABLE = -1 /* Not available for use */
} qr_format_t;

/* Total number of output formats */
#define QR_FMT_COUNT 8

/*
 * module value mask
 */
#define QR_MM_DATA      0x01  /* Black module of encoded data */
#define QR_MM_BLACK     0x02  /* Printed black module */
#define QR_MM_FUNC      0x04  /* Functional pattern area (including model/part number information) */

/*
 * Functional Pattern Constants
 */
#define QR_DIM_SEP      4  /* Separation pattern width */
#define QR_DIM_FINDER   7  /* Length of one side of the position finder pattern */
#define QR_DIM_ALIGN    5  /* Length of one side of the alignment pattern */
#define QR_DIM_TIMING   6  /* Timing pattern offset position */

/*
 * size constant
 */
#define QR_SRC_MAX  7089  /* Maximum length of input data */
#define QR_DIM_MAX   177  /* Maximum number of modules on one side */
#define QR_VER_MAX    40  /* Maximum model number */
#define QR_DWD_MAX  2956  /* Maximum length of data code word (Model 40/Level L) */
#define QR_ECW_MAX  2430  /* Maximum length of error correction codeword (Model 40/Level H) */
#define QR_CWD_MAX  3706  /* Maximum length of codeword (model number 40) */
#define QR_RSD_MAX   123  /* Maximum length of RS block data codeword */
#define QR_RSW_MAX    68  /* Maximum length of RS block error correcting codeword */
#define QR_RSB_MAX     2  /* Maximum number of RS block types */
#define QR_MPT_MAX     8  /* Total number of mask pattern types */
#define QR_APL_MAX     7  /* Maximum number of alignment pattern coordinates */
#define QR_FIN_MAX    15  /* Number of bits of format information */
#define QR_VIN_MAX    18  /* Number of bits in model number information */
#define QR_MAG_MAX    16  /* Maximum pixel display magnification */
#define QR_SEP_MAX    16  /* Maximum width of separation pattern */
#define QR_ERR_MAX  1024  /* Maximum length of error information */
#define QR_STA_MAX    16  /* Maximum number of connections (splits/connections) to construct */
#define QR_STA_LEN    20  /* Number of bits in the structured concatenation header */

/*
 * Other constants
 */
#define NAV            0  /* Not available */
#define PADWORD1    0xec  /* Filler Codeword 1: 11101100 */
#define PADWORD2    0x11  /* Filler codeword 2: 00010001 */
#define VERPOINT1      9  /* Model number 1 immediately before the bit number of the character count indicator changes */
#define VERPOINT2     26  /* Model number 2 immediately before the bit number of the character count indicator changes */

/*
 * 8-bit binary data type
 */
typedef unsigned char qr_byte_t;

/*
 * Information for each RS block
 */
typedef struct qr_rsblock_t {
  int rsbnum;     /* Number of RS blocks */
  int totalwords; /* RS block total code word count */
  int datawords;  /* RS block data code word count */
  int ecnum;      /* RS block error correction count (unused) */
} qr_rsblock_t;

/*
 * Information for each error correction level
 */
typedef struct qr_eclevel_t {
  int datawords;                /* Number of data code words (all RS blocks) */
  int capacity[QR_EM_COUNT];    /* Data capacity for each encoding mode */
  int nrsb;                     /* RS block type (1 or 2) */
  qr_rsblock_t rsb[QR_RSB_MAX]; /* Information for each RS block */
} qr_eclevel_t;

/*
 * Information for each model number
 */
typedef struct qr_vertable_t {
  int          version;           /* Model number */
  int          dimension;         /* Number of modules per side */
  int          totalwords;        /* Total code words */
  int          remainedbits;      /* Remainder Bits */
  int          nlen[QR_EM_COUNT]; /* Number of bits in the character count indicator */
  qr_eclevel_t ecl[QR_ECL_COUNT]; /* Information for each error correction level */
  int          aplnum;            /* Number of alignment pattern center coordinates */
  int          aploc[QR_APL_MAX]; /* Center coordinates of alignment pattern */
} qr_vertable_t;

/*
 * coordinate data type
 */
typedef struct qr_coord_t { int ypos, xpos; } qr_coord_t;

/*
 * parameter structure
 */
typedef struct qr_param_t {
  int version;              /* Model number */
  int mode;                 /* encoding mode */
  int eclevel;              /* error correction level */
  int masktype;             /* Mask pattern type */
} qr_param_t;

/*
 * QR code object
 */
typedef struct qrcode_t {
  qr_byte_t *dataword;      /* Data code word area address */
  qr_byte_t *ecword;        /* Address of the error correction code word area */
  qr_byte_t *codeword;      /* Address of the code word area for symbol placement */
  qr_byte_t *_symbol;       /* Symbol data area address */
  qr_byte_t **symbol;       /* Pointer to the address of the beginning of each line of symbol data */
  qr_byte_t *source;        /* Input data area address */
  size_t srcmax;            /* Maximum capacity of input data area */
  size_t srclen;            /* Input data area usage */
  int enclen;               /* Total bit length of data code word */
  int delta1, delta2;       /* Bit length difference used to assist in automatic part number selection */
  int dwpos;                /* Additional byte positions for data code words */
  int dwbit;                /* Additional bit positions in data code words */
  int xpos, ypos;           /* Coordinate position to place the module */
  int xdir, ydir;           /* Module placement movement direction */
  int state;                /* Processing progress */
  int errcode;              /* The number of the last error that occurred */
  char errinfo[QR_ERR_MAX]; /* Details of the last error that occurred */
  qr_param_t param;         /* Output parameters */
} QRCode;

/*
 * Structurally linked QR code object
 */
typedef struct qrcode_sa_t {
  QRCode *qrs[QR_STA_MAX];  /* QR code object pointer array */
  QRCode *cur;              /* The QR code object to input the value */
  int num;                  /* number of symbols */
  int max;                  /* Maximum number of symbols */
  int parity;               /* parity */
  int state;                /* Processing progress */
  qr_param_t param;         /* Output parameters */
} QRStructured;

/*
 * QR code output function type
 */
typedef qr_byte_t *(*QRConverter)(QRCode *, int, int, int *);
typedef qr_byte_t *(*QRsConverter)(QRStructured *, int, int, int, int *);

#define qrIsBlacke(qr, i, j) (((qr)->symbol[(i)][(j)] & QR_MM_BLACK) != 0)

/*
 * Basic Function Prototypes
 */
QR_API QRCode *qrInit(int version, int mode, int eclevel, int masktype, int *errcode);
QR_API void qrDestroy(QRCode *qr);
QR_API int qrGetErrorCode(QRCode *qr);
QR_API char *qrGetErrorInfo(QRCode *qr);
QR_API int qrAddData(QRCode *qr, const qr_byte_t *source, int size);
QR_API int qrAddData2(QRCode *qr, const qr_byte_t *source, int size, int mode);
QR_API int qrFinalize(QRCode *qr);
QR_API int qrIsFinalized(const QRCode *qr);
QR_API int qrHasData(const QRCode *qr);
QR_API QRCode *qrClone(const QRCode *qr, int *errcode);

/*
 * Function prototypes for structural concatenation operations
 */
QR_API QRStructured *qrsInit(int version, int mode, int eclevel, int masktype, int maxnum, int *errcode);
QR_API void qrsDestroy(QRStructured *st);
QR_API int qrsGetErrorCode(QRStructured *st);
QR_API char *qrsGetErrorInfo(QRStructured *st);
QR_API int qrsAddData(QRStructured *st, const qr_byte_t *source, int size);
QR_API int qrsAddData2(QRStructured *st, const qr_byte_t *source, int size, int mode);
QR_API int qrsFinalize(QRStructured *st);
QR_API int qrsIsFinalized(const QRStructured *st);
QR_API int qrsHasData(const QRStructured *st);
QR_API QRStructured *qrsClone(const QRStructured *st, int *errcode);

/*
 * Output function prototype
 */
QR_API int qrOutputSymbol(QRCode *qr, FILE *fp, int fmt, int sep, int mag);
QR_API int qrOutputSymbol2(QRCode *qr, const char *pathname, int fmt, int sep, int mag);
QR_API qr_byte_t *qrGetSymbol(QRCode *qr, int fmt, int sep, int mag, int *size);
QR_API qr_byte_t *qrSymbolToDigit(QRCode *qr, int sep, int mag, int *size);
QR_API qr_byte_t *qrSymbolToASCII(QRCode *qr, int sep, int mag, int *size);
QR_API qr_byte_t *qrSymbolToJSON(QRCode *qr, int sep, int mag, int *size);
QR_API qr_byte_t *qrSymbolToPBM(QRCode *qr, int sep, int mag, int *size);
QR_API qr_byte_t *qrSymbolToBMP(QRCode *qr, int sep, int mag, int *size);

/*
 * Function prototype for structural concatenation output
 */
QR_API int qrsOutputSymbols(QRStructured *st, FILE *fp, int fmt, int sep, int mag, int order);
QR_API int qrsOutputSymbols2(QRStructured *st, const char *pathname, int fmt, int sep, int mag, int order);
QR_API qr_byte_t *qrsGetSymbols(QRStructured *st, int fmt, int sep, int mag, int order, int *size);
QR_API qr_byte_t *qrsSymbolsToDigit(QRStructured *st, int sep, int mag, int order, int *size);
QR_API qr_byte_t *qrsSymbolsToASCII(QRStructured *st, int sep, int mag, int order, int *size);
QR_API qr_byte_t *qrsSymbolsToJSON(QRStructured *st, int sep, int mag, int order, int *size);
QR_API qr_byte_t *qrsSymbolsToPBM(QRStructured *st, int sep, int mag, int order, int *size);
QR_API qr_byte_t *qrsSymbolsToBMP(QRStructured *st, int sep, int mag, int order, int *size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _QR_H_ */
