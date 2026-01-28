#include "dmtx.h"

/*
libdmtx - Data Matrix Encoding/Decoding Library

Copyright (c) 2002-2004 Phil Karn, KA9Q
Copyright (c) 2008 Mike Laughton

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

----------------------------------------------------------------------
This file is derived from various portions of the fec library
written by Phil Karn available at http://www.ka9q.net. It has
been modified to include only the specific cases used by Data
Matrix barcodes, and to integrate with the rest of libdmtx.
----------------------------------------------------------------------

Contact: mike@dragonflylogic.com
*/

/* $Id: dmtxfec.c 398 2008-08-06 18:05:53Z mblaughton $ */

/**
 * @file dmtxfec.c
 * @brief Forward Error Correction
 */

#include <stdlib.h>
#include <string.h>

typedef unsigned char data_t;

#define DMTX_RS_MM         8 /* Bits per symbol */
#define DMTX_RS_NN       255 /* Symbols per RS block */
#define DMTX_RS_GFPOLY 0x12d /* Field generator polynomial coefficients */

#undef NULL
#define NULL ((void *)0)

#undef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* Reed-Solomon codec control block */
struct rs {
   data_t *alpha_to;    /* log lookup table */
   data_t *index_of;    /* Antilog lookup table */
   data_t *genpoly;     /* Generator polynomial */
   int nroots;          /* Number of generator roots = number of parity symbols */
   int pad;             /* Padding bytes in shortened block */
};

/* General purpose RS codec, 8-bit symbols */
void encode_rs_char(struct rs *rs, unsigned char *data, unsigned char *parity);
int decode_rs_char(struct rs *rs, unsigned char *data, int *eras_pos, int no_eras, int max_fixes);
void *init_rs_char(int nroots, int pad);
void free_rs_char(struct rs *rs);
