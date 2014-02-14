#ifndef _LIBOVF2_H_
#define _LIBOVF2_H_

#include <stdio.h>

/* First number in data section.
 * OVF specification, OOMMF user's guide 1.2a4, page 223.
 */
#define OVF2_CONTROL_NUMBER 1234567.0

/* Holds stripped-down content of an OVF file
 * (relevant header + data).
 */
typedef struct {
    char *err;                  // error message, if any
    int valuedim;               // number of data components
    int xnodes, ynodes, znodes; // grid size
    float *data;                // data
} ovf2_data;


/* Reads the named OVF file.
 * ovf2_data.err contains an error message if something went wrong.
 */
ovf2_data ovf2_readfile(const char *filename);

/* Reads OVF data from input stream, and does not close it.
 * ovf2_data.err contains an error message if something went wrong.
 */
ovf2_data ovf2_read(FILE* in);

void ovf2_write(FILE* out, ovf2_data data);

void ovf2_writeffile(const char *filename, ovf2_data data);

int ovf2_datalen(ovf2_data data);

float ovf2_get(ovf2_data *data, int c, int x, int y, int z);

void ovf2_free(ovf2_data *data);

#endif

