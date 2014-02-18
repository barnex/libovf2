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


#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "libovf2.h"

#define BUFLEN 2047 // maximum header line length

char* newLineBuf(){
return (char*)malloc(BUFLEN+1);
}

void panic(const char *msg) {
    fprintf(stderr, "panic: %s\n", msg);
    abort();
}

bool streq(const char *a, const char *b) {
    if(a==NULL || b==NULL) {
        panic("streq: NULL input");
    }
    return (strcmp(a, b) == 0);
}

bool hasprefix(const char *s, const char *prefix) {
    if(s==NULL || prefix==NULL) {
        panic("hasprefix: NULL input");
    }
    return (strstr(s, prefix) == s);
}

void strToLower(char *s) {
    for(int i=0; s[i] != 0; i++) {
        s[i] = tolower(s[i]);
    }
}

void efputs(const char *str, FILE *stream) {
    int n = fputs(str, stream);
    if (n < 0) {
        fprintf(stderr, "fputs(\"%s, %p\"): error %d\n", str, stream, n);
        abort();
    }
}

void efread(void* ptr, size_t size, size_t count, FILE *stream) {
    int ret = fread(ptr, size, count, stream);
    if (ret < count) {
        panic("fread");
    }
}


char* SprintD(const char *format, int d){
	char *buf = (char*)malloc(BUFLEN+1);
	snprintf(buf, BUFLEN, format, d);
	return buf;
}


char* SprintS(const char *format, const char * s){
	char *buf = (char*)malloc(BUFLEN+1);
	snprintf(buf, BUFLEN, format, s);
	return buf;
}

/* Read header line, make lowercase and trim whitespace and comment characters.
 * Set possible error message in d->err. 
 * E.g.:
 * 	# XNodes:  7 ## comment
 * ->
 *  xnodes: 7 
 */
void ovf2_readLine(ovf2_data * d, char *line, FILE* in) {
    char *result = fgets(line, BUFLEN, in);
	if (result != line){
		d->err = SprintD("ovf2_readline: errno %d", errno);
		return;
	}

	// remove leading '#'
	if (line[0] != '#'){
		d->err = SprintS("ovf2: invalid header line: \"%s\"", line);
	}
	line[0] = ' '; // replace leading '#' by space that will be trimmed.

	// remove leading whitespace
	int start=0; 
	while(line[start] == ' ' || line[start] == '\t'){
		start++;
	}
	memmove(line, &line[start], BUFLEN - start);
	
	// remove tailing comments, whitespace and newline
	// we treat a single '#' as a comment, even though OOMMF specifies '##'
	int end=0;
	while(line[end] != '#' && line[end] != '\n'){
		end++;
	}
	line[end] = 0;

	// trim trailing whitespace
	end--;
	while(end >= 0 && isspace(line[end])){
		line[end] = 0;
		end--;
	}

	// finally, lowercase to avoid confusion between End Data, End data, end Data,...
    strToLower(line);
}


ovf2_data ovf2_read(FILE* in) {
	ovf2_data d = {};
    char line[BUFLEN+1] = {}; 

	for(ovf2_readLine(&d, line, in); d.err == NULL && !hasprefix(line, "begin: data"); ovf2_readLine(&d, line, in)){
		printf("\"%s\"\n", line);
	}

	if (d.err != NULL){
		return d;
	}

	
    // read header
    //for (ovf2_gets(line, in); !hasprefix(line, "# begin: data"); ovf2_gets(line, in)) {
    //    // TODO: strip extra ## comments

    //    if(hasprefix(line, "# valuedim:")) {
    //        data.valuedim = atoi(&line[11]);
    //        continue;
    //    }
    //    if(hasprefix(line, "# xnodes:")) {
    //        data.xnodes = atoi(&line[9]);
    //        continue;
    //    }
    //    if(hasprefix(line, "# ynodes:")) {
    //        data.ynodes = atoi(&line[9]);
    //        continue;
    //    }
    //    if(hasprefix(line, "# znodes:")) {
    //        data.znodes = atoi(&line[9]);
    //        continue;
    //    }
    //}


    //if (!streq(line, "# begin: data binary 4")){
    //	panic(line);
    //}

    // control number
    //float control = 0;
    //efread(&control, sizeof(float), 1, in);
    //if (control != OVF2_CONTROL_NUMBER) {
    //    panic("invalid ovf control number");
    //}

    //size_t nfloat = ovf2_datalen(data);
    //assert(nfloat > 0);
    //data.data = (float*)malloc(nfloat * sizeof(float));
    //efread(data.data, sizeof(float), nfloat, in);

    //ovf2_gets(line, in);
    ////if (!streq(line, "# end: data")) {
    ////    panic(line);
    ////}

    return d;
}


ovf2_data ovf2_readfile(const char *filename) {
    FILE *in = fopen(filename, "r");
    if(in == NULL) {
		char *buf = newLineBuf();
        snprintf(buf, BUFLEN, "ovf2_readfile: failed to open \"%s\": errno %d\n", filename, errno);
		ovf2_data d = {err: buf};
		return d;
    }

    ovf2_data data = ovf2_read(in);
    fclose(in);
    return data;
}


float ovf2_get(ovf2_data *data, int c, int x, int y, int z) {
    int Nx = data->xnodes;
    int Ny = data->ynodes;
    int Nz = data->znodes;
    int Nc = data->valuedim;

    assert(x >= 0 && x < Nx &&
           y >= 0 && y < Ny &&
           z >= 0 && z < Nz &&
           c >= 0 && c < Nc);

    return data->data[((c*Nz+z)*Ny + y)*Nx + x];
}


void ovf2_write(FILE* out, ovf2_data data) {
    if(data.err != NULL) {
        efputs(data.err, out);
        return;
    }

    efputs("# OOMMF OVF 2.0\n", out);
    efputs("# Segment count: 1\n", out);
    efputs("# Begin: Segment\n", out);
    efputs("# Begin: Header\n", out);
    fprintf(out, "# valuedim: %d\n", data.valuedim); // TODO: e
    fprintf(out, "# xnodes: %d\n", data.xnodes); // TODO: e
    fprintf(out, "# ynodes: %d\n", data.ynodes); // TODO: e
    fprintf(out, "# znodes: %d\n", data.znodes); // TODO: e

    for(int z=0; z<data.znodes; z++) {
        for(int y=0; y<data.ynodes; y++) {
            for(int x=0; x<data.xnodes; x++) {
                for(int c=0; c<data.valuedim; c++) {
                    float v = ovf2_get(&data, c, x, y, z);
                    fprintf(out, "%f ", v);
                }
            }
            efputs("\n", out);
        }
        efputs("\n", out);
    }
}

void ovf2_writeffile(const char *filename, ovf2_data data) {
    FILE *out = fopen(filename, "w+");

    if(out == NULL) {
        fprintf(stderr, "ovf2_writefile: failed to open %s: errno %d\n", filename, errno);
        abort();
    }

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
