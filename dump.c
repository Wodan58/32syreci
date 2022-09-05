/*
    module  : dump.c
    version : 1.3
    date    : 09/05/22
*/
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define MAXSTR	80
#define INSCNT	25

#define inputfile "32syreci.tmp"

typedef enum {
    add,
    sub,
    mul,
    dvd,
    mdl,
    eql,
    neq,
    gtr,
    geq,
    lss,
    leq,
    orr,
    neg,
    loadglobl,
    loadlocal,
    loadimmed,
    storglobl,
    storlocal,
    writebool,
    writeint,
    cal,
    ret,
    jmp,
    jiz,
    hlt
} operator;

char *opstrings[] = {
    "ADD",
    "SUB",
    "MUL",
    "DVD",
    "MDL",
    "EQL",
    "NEQ",
    "GTR",
    "GEQ",
    "LSS",
    "LEQ",
    "ORR",
    "NEG",
    "LOADGLOBL",
    "LOADLOCAL",
    "LOADIMMED",
    "STORGLOBL",
    "STORLOCAL",
    "WRITEBOOL",
    "WRITEINT",
    "CAL",
    "RET",
    "JMP",
    "JIZ",
    "HLT"
};

typedef struct instruction {
    operator op;
    int64_t adr1, adr2;
} instruction;

/*
    instruction, adr1, adr2;
*/
int main()
{
    int i;
    FILE *fp;
    int64_t l;
    instruction ins;
    char str[MAXSTR];

    if ((fp = fopen(inputfile, "wb")) == 0) {
	fprintf(stderr, "%s cannot create\n", inputfile);
	return 1;
    }
    while (scanf("%" SCNd64 "%s %" SCNd64 "%" SCNd64,
		&l, str, &ins.adr1, &ins.adr2) == 4) {
	for (i = 0; i < INSCNT; i++)
	    if (!strcmp(str, opstrings[i]))
		break;
	if (i == INSCNT)
	    fprintf(stderr, "%s not found\n", str);
	else {
	    ins.op = i;
	    fwrite(&ins, sizeof(ins), 1, fp);
	}
    }
    fclose(fp);
    return 0;
}
