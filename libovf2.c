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

#include "libovf2.h"
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define BUFLEN 2047 /* maximum header line length */

/* internal: alloc line buffer. */
static char* ovf2_buf(){
	return (char*)malloc(BUFLEN+1);
}

/* internal: string equals */
static bool ovf2_strEq(const char *a, const char *b) {
	assert(a!=NULL && b!=NULL);
	return (strcmp(a, b) == 0);
}

/* internal: s has prefix prefix? */
static bool ovf2_hasPrefix(const char *s, const char *prefix) {
	assert(s!=NULL && prefix!=NULL);
	return (strstr(s, prefix) == s);
}

/* internal: overwrite s with lowercase version */
static void ovf2_toLower(char *s) {
	int i;
	for(i=0; s[i] != 0; i++) {
		s[i] = tolower(s[i]);
	}
}

/* internal: header whitespace characters */
static bool ovf2_isSpace(char c){
	return c == ' ' || c == '\t';
}

/* internal: wrap around sprintf to catch buffer overflow.
   goal is to mimic snprintf which is not available below C99.
*/
static void ovf2_sn(int n){
	assert(n < BUFLEN);
}

/* 
   internal: read header line, make lowercase and trim whitespace and comment characters.
   Set possible error message in d->err. E.g.:
   	# XNodes:  7 ## comment
   becomes
    xnodes: 7 
*/
static void ovf2_readLine(ovf2_data * d, char *line, FILE* in) {
	char *result = fgets(line, BUFLEN, in);
	if (result != line){
		d->err = ovf2_buf();
		ovf2_sn(sprintf(d->err, "ovf2_read: input error: %s", strerror(errno)));
		return;
	}

	/* remove leading '#' */
	if (line[0] != '#'){
		d->err = ovf2_buf();
		ovf2_sn(sprintf(d->err, "ovf2_read: invalid header line: \"%s\"", line));
	}
	line[0] = ' '; /* replace leading '#' by space which will be trimmed. */

	/* remove leading whitespace */
	int start=0; 
	while(ovf2_isSpace(line[start])){
		start++;
	}
	memmove(line, &line[start], BUFLEN - start);
	
	/* remove tailing comments and newline
	   we treat a single '#' as a comment, even though OOMMF specifies '##' */
	int end=0;
	while(line[end] != '#' && line[end] != '\n'){
		end++;
	}
	line[end] = 0;

	/* trim trailing whitespace */
	end--;
	while(end >= 0 && isspace(line[end])){
		line[end] = 0;
		end--;
	}

	/* finally, lowercase to avoid confusion between End Data, End data, end Data,... */
	ovf2_toLower(line);
}

/* internal: retrieves value from "key: value" pair. 
   trims value leading whitespace */
static const char* ovf2_hdrVal(const char *line){
	int start = 0;
	while(line[start] != ':'){
		start++;
	}
	start++; /* skip the ':' */
	/* trim key whitespace */
	while(ovf2_isSpace(line[start])){
		start++;
	}
	return &line[start];
}

static float ovf2_readFloat(ovf2_data *d, FILE *in){
	float buf = 0;
		int ret = fread(&buf, sizeof(float), 1, in);
		if (ret != 1){
			d->err = ovf2_buf();
			ovf2_sn(sprintf(d->err, "ovf2_read: input error: %s", strerror(errno)));
			return 0.0f;
		}
	return buf;
}

/* internal: read nfloat floats from in to d->data.
   store possible error message in d->err. */
static void ovf2_readData(ovf2_data *d, FILE *in){
	int iz, iy, ix, ic;

	for(iz=0; iz<d->znodes; iz++){
	for(iy=0; iy<d->ynodes; iy++){
	for(ix=0; ix<d->xnodes; ix++){
	for(ic=0; ic<d->valuedim; ic++){
		float v = ovf2_readFloat(d, in);
		if (d->err != NULL){
			return;
		}
		*ovf2_addr(d, ic, ix, iy, iz) = v;
	}
	}
	}
	}
}



/* internal: construct zero value. */
static ovf2_data ovf2_makeData(){
	ovf2_data d = {err: NULL, valuedim: 0, xnodes: 0, ynodes: 0, znodes: 0, data: NULL};
	return d;
}

ovf2_data ovf2_read(FILE* in) {
	ovf2_data d = ovf2_makeData();
	char line[BUFLEN+1] = {}; 
	
	ovf2_readLine(&d, line, in);
	if( !ovf2_strEq(line, "oommf ovf 2.0") ){
		d.err = ovf2_buf();
		ovf2_sn(sprintf(d.err, "ovf2_read: invalid format: \"%s\"", line));
		return d;
	}
	
	for(; d.err == NULL; ovf2_readLine(&d, line, in)){
		
		/* stop at begin data section */
		if(ovf2_hasPrefix(line, "begin:") && ovf2_hasPrefix(ovf2_hdrVal(line), "data")){
			break;
		}
		
		if(ovf2_hasPrefix(line, "valuedim:")) {
			d.valuedim = atoi(ovf2_hdrVal(line));
			continue;
		}
		if(ovf2_hasPrefix(line, "xnodes:")) {
			d.xnodes = atoi(ovf2_hdrVal(line));
			continue;
		}
		if(ovf2_hasPrefix(line, "ynodes:")) {
			d.ynodes = atoi(ovf2_hdrVal(line));
			continue;
		}
		if(ovf2_hasPrefix(line, "znodes:")) {
			d.znodes = atoi(ovf2_hdrVal(line));
			continue;
		}
		if(ovf2_hasPrefix(line, "meshtype:")){
			if(!ovf2_strEq(ovf2_hdrVal(line), "rectangular")){
				d.err = ovf2_buf();	
				ovf2_sn(sprintf(d.err, "ovf2_read: unsupported meshtype: \"%s\"", ovf2_hdrVal(line)));
				return d;
			}
			continue;
		}
		if(ovf2_hasPrefix(line, "segment count:")){
			if(!ovf2_strEq(ovf2_hdrVal(line), "1")){
				d.err = ovf2_buf();	
				ovf2_sn(sprintf(d.err, "ovf2_read: unsupported segment count: \"%s\"", ovf2_hdrVal(line)));
				return d;
			}
			continue;
		}
    	}

	if(d.valuedim <= 0){
		d.err = ovf2_buf();
		ovf2_sn(sprintf(d.err, "ovf2_read: invalid valuedim: %d", d.valuedim));
	}

	if(d.xnodes <= 0 || d.ynodes <= 0 || d.znodes <= 0){
		d.err = ovf2_buf();
		ovf2_sn(sprintf(d.err, "ovf2_read: invalid grid size: %d x %d x %d", d.xnodes, d.ynodes, d.znodes));
	}

	if (!ovf2_strEq(line, "begin: data binary 4")){
		d.err = ovf2_buf();	
		ovf2_sn(sprintf(d.err, "ovf2_read: expected \"Begin: Data Binary 4\", got: \"%s\"", line));
	}

	if (d.err != NULL){
		return d;
	}

	size_t nfloat = ovf2_datalen(d);
	assert(nfloat > 0);
	d.data = (float*)malloc(nfloat * sizeof(float));
	
	/* read control number into data array, overwrite with actual data later. */
	float control = ovf2_readFloat(&d, in);
	if (d.err != NULL){
		return d;
	}
	if (control != OVF2_CONTROL_NUMBER) {
		d.err = ovf2_buf();
		ovf2_sn(sprintf(d.err, "invalid ovf control number: %f:", d.data[0]));
		free(d.data);
		d.data = NULL;
		return d;
	}
	
	/* read rest of data */
	ovf2_readData(&d, in);
	if (d.err != NULL){
   		return d;
	}

	ovf2_readLine(&d, line, in);
	if (!ovf2_hasPrefix(line, "end: data")) {
		d.err = ovf2_buf();
		ovf2_sn(sprintf(d.err, "ovf2_read: expected \"end: data <format>\", got: \"%s\"", line));
		return d;
	}
	
	/* ignore End: Segment */
	return d;
}


ovf2_data ovf2_readfile(const char *filename) {
	FILE *in = fopen(filename, "r");
	if(in == NULL) {
		ovf2_data d = ovf2_makeData();
		d.err = ovf2_buf();
		ovf2_sn(sprintf(d.err, "ovf2_readfile: failed to open \"%s\": %s\n", filename, strerror(errno)));
		return d;
	}
	
	ovf2_data data = ovf2_read(in);
	fclose(in);
	return data;
}


float ovf2_get(ovf2_data *data, int c, int x, int y, int z) {
	return *ovf2_addr(data, c, x, y, z);
}

float *ovf2_addr(ovf2_data *data, int c, int x, int y, int z) {
	int Nx = data->xnodes;
	int Ny = data->ynodes;
	int Nz = data->znodes;
	int Nc = data->valuedim;
	
	assert(x >= 0 && x < Nx &&
	       y >= 0 && y < Ny &&
	       z >= 0 && z < Nz &&
	       c >= 0 && c < Nc);
	
	return &(data->data[((c*Nz+z)*Ny + y)*Nx + x]);
}


static void efputs(const char *str, FILE *stream) {
    int n = fputs(str, stream);
    if (n < 0) {
        fprintf(stderr, "fputs(\"%s, %p\"): error %d\n", str, stream, n);
        abort();
    }
}



void ovf2_write(FILE* out, ovf2_data data) {
	int x, y, z, c;
	float v;

	if(data.err != NULL) {
	    efputs(data.err, out);
	    return;
	}

	efputs("# OOMMF OVF 2.0\n", out);
	efputs("# Segment count: 1\n", out);
	efputs("# Begin: Segment\n", out);
	efputs("# Begin: Header\n", out);
	fprintf(out, "# valuedim: %d\n", data.valuedim); /* TODO: e */
	fprintf(out, "# xnodes: %d\n", data.xnodes); 
	fprintf(out, "# ynodes: %d\n", data.ynodes); 
	fprintf(out, "# znodes: %d\n", data.znodes); 

	fprintf(out, "# End: Header\n");
	fprintf(out, "# Begin: Data Text\n");
	
	for(z=0; z<data.znodes; z++) {
		for(y=0; y<data.ynodes; y++) {
			for(x=0; x<data.xnodes; x++) {
				for(c=0; c<data.valuedim; c++) {
					v = ovf2_get(&data, c, x, y, z);
					fprintf(out, "%f ", v);
				}
			}
			efputs("\n", out);
	    	}
		efputs("\n", out);
	}

	fprintf(out, "# End: Data Text\n");
	fprintf(out, "# End: Segment\n");
}

void ovf2_writefile(const char *filename, ovf2_data data) {
    FILE *out = fopen(filename, "w+");

    if(out == NULL) {
        fprintf(stderr, "ovf2_writefile: failed to open %s: %s\n", filename, strerror(errno));
        abort();
    }
	ovf2_write(out, data);
    fclose(out);
}

void ovf2_free(ovf2_data *d) {
	if(d->err != NULL) {
		free(d->err);
		d->err = NULL;
	}
	if(d->data != NULL) {
		free(d->data);
		d->data = NULL;
	}
	d->valuedim = 0;
	d->xnodes = 0;
	d->ynodes = 0;
	d->znodes = 0;
}


int ovf2_datalen(ovf2_data data) {
	return data.valuedim * data.xnodes * data.ynodes * data.znodes;
}
