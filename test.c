/* Unit tests. */

#include "libovf2.h"

int testScalarIndex() {
	ovf2_data d = ovf2_readfile("testdata_good/scalar123.ovf");
	if(d.err != NULL) {
		printf("%s", d.err);
		return 1;
	}
	float val = ovf2_get(&d, 0, 1, 2, 3);
	if(val != 4.0) {
		printf("want 4, got: %f\n", val);
		return 1;
	}
	return 0;
}


int testVectorIndex() {
	ovf2_data d = ovf2_readfile("testdata_good/vector123.ovf");
	if(d.err != NULL) {
		printf("%s", d.err);
		return 1;
	}
	float val = ovf2_get(&d, 0, 1, 2, 3);
	if(val != 4.0) {
		printf("want 4, got: %f\n", val);
		return 1;
	}
	val = ovf2_get(&d, 1, 1, 2, 3);
	if(val != 5.0) {
		printf("want 5, got: %f\n", val);
		return 1;
	}
	val = ovf2_get(&d, 2, 1, 2, 3);
	if(val != 6.0) {
		printf("want 6, got: %f\n", val);
		return 1;
	}
	return 0;
}


static int overall_status = 0;

void check(char *name, int status) {
	printf("%s: %s\n", name, ((status==0) ? "OK": "FAIL"));
	if (status != 0) {
		overall_status = 1;
	}
}

int main() {
	check("testScalarIndex", testScalarIndex());
	check("testVectorIndex", testVectorIndex());
	return overall_status;
}
