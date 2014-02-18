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

void ovf2_write(FILE* out, ovf2_data data);

void ovf2_writeffile(const char *filename, ovf2_data data);

/* frees all allocations associated with data, 
 * and sets all fields to 0 to avoid accidental use.*/
void ovf2_free(ovf2_data *data);

/* returns the length of the data array (valuedim * xnodes * ynodes * znodes). */
int ovf2_datalen(ovf2_data data);

/* utility function for getting element data[c][z][y][x],
 * with bound checks. */
float ovf2_get(ovf2_data *data, int c, int x, int y, int z);

/* First number in data section.
 * OVF specification, OOMMF user's guide 1.2a4, page 223.  */
#define OVF2_CONTROL_NUMBER 1234567.0


#endif

