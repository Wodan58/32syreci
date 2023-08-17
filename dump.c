/*
    module  : dump.c
    version : 1.4
    date    : 08/17/23
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "32syreci.h"

#define MAXSTR	80
#define INSCNT	25

/*
    instruction, adr1, adr2;
*/
int main(int argc, char *argv[])
{
    int i;
    FILE *fp;
    int64_t l;
    instruction ins;
    char str[MAXSTR], *filename;

    filename = argc == 2 ? argv[1] : inputfile;
    if ((fp = fopen(filename, "wb")) == 0) {
	fprintf(stderr, "%s (cannot create)\n", filename);
	exit(EXIT_FAILURE);
    }
    while (scanf("%" SCNd64 "%s %" SCNd64 "%" SCNd64,
		&l, str, &ins.adr1, &ins.adr2) == 4) {
	for (i = 0; i < INSCNT; i++)
	    if (!strcmp(str, operator_NAMES[i]))
		break;
	if (i == INSCNT)
	    fprintf(stderr, "%s not found\n", str);
	else {
	    ins.op = i;
	    fwrite(&ins, sizeof(ins), 1, fp);
	}
    }
    fclose(fp);
    exit(EXIT_SUCCESS);
}
