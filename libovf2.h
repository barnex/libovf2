#ifndef _LIBOVF2_H_
#define _LIBOVF2_H_

#include <stdio.h>

#define OVF2_CONTROL_NUMBER 1234567.0

typedef struct {
    char *err;                  // error message, if any
	int valuedim;               // number of data components
    int xnodes, ynodes, znodes; // grid size
	float *data;
} ovf2_data;


ovf2_data ovf2_readfile(const char *filename);

ovf2_data ovf2_read(FILE* in);

void ovf2_write(FILE* out, ovf2_data data);

void ovf2_writeffile(const char *filename, ovf2_data data);

int ovf2_datalen(ovf2_data data);

float ovf2_get(ovf2_data *data, int c, int x, int y, int z);

#endif

