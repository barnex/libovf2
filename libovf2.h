/*
The MIT License (MIT)

Copyright (c) 2014 Arne Vansteenkiste <arne@vansteenkiste.io>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef _LIBOVF2_H_
#define _LIBOVF2_H_


#include <stdio.h>

/* Holds stripped-down content of an OVF file
 * (relevant header sections + data).  */
typedef struct {
    char *err;                  // error message, if any
    int valuedim;               // number of data components
    int xnodes, ynodes, znodes; // grid size
    float *data;                // data array, without control number
} ovf2_data;


/* Reads the named OVF file.
 * ovf2_data.err contains an error message if something went wrong.  */
ovf2_data ovf2_readfile(const char *filename);

/* Reads OVF data from input stream, and does not close it.
 * ovf2_data.err contains an error message if something went wrong.  */
ovf2_data ovf2_read(FILE* in);

/* frees all allocations associated with data, 
 * and sets all fields to 0 to avoid accidental use.*/
void ovf2_free(ovf2_data *data);

/* returns the length of the data array (valuedim * xnodes * ynodes * znodes). */
int ovf2_datalen(ovf2_data data);

/* utility function for getting element data[c][z][y][x],
 * with bound checks. */
float ovf2_get(ovf2_data *data, int c, int x, int y, int z);


/* write for debug purposes, not OVF2 compatible. */
void ovf2_write(FILE* out, ovf2_data data);

/* write for debug purposes, not OVF2 compatible. */
void ovf2_writeffile(const char *filename, ovf2_data data);

/* First number in data section.
 * OVF specification, OOMMF user's guide 1.2a4, page 223. */
#define OVF2_CONTROL_NUMBER 1234567.0


#endif

