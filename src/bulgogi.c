/**
* bulgogi.c
*
* Brief: Build system assistant cli tool.
* Author: Alex Amellal
*
*/

// Standard C Libraries
#include <stdlib.h>
#include <argp.h>

const char *bulgogi_program_version = "bulgogi 1.0.0";
const char *bulgogi_program_bug_address = "<bulgogi@alexamellal.com>";

static char doc[] = "bulgogi -- a handy build system assistant";

static struct argp argp = { 0, 0, 0, doc };

int main(int argc, char *argv[]) {
	argp_parse(&argp, argc, argv, 0, 0, 0);

	return 0;
}
