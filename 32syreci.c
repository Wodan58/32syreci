/*
    module  : 32syreci.c
    version : 1.3
    date    : 07/11/22
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* SYmboltable, RECursion, INTerpreter only,
   interprets a file of instruction produced by syrecc */

#define inputfile "32syreci.tmp"

#define showcode false
#define tracing false

#define maxcode 200
#define maxstack 1000
#define topregister 7

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

typedef struct instruction {
    operator op;
    long adr1, adr2;
} instruction;

static char *operator_NAMES[] = { "ADD", "SUB", "MUL", "DVD", "MDL", "EQL",
    "NEQ", "GTR", "GEQ", "LSS", "LEQ", "ORR", "NEG", "LOADGLOBL", "LOADLOCAL",
    "LOADIMMED", "STORGLOBL", "STORLOCAL", "WRITEBOOL", "WRITEINT", "CAL",
    "RET", "JMP", "JIZ", "HLT" };

int main(int argc, char *argv[])
{ /* main */
    FILE *infile;
    instruction code[maxcode];
    long pc;

    long stack[maxstack + 1];
    long stacktop = 0;
    long reg[topregister + 1];
    long baseregister = 0;

    printf("SYRECI ...\n");

    if ((infile = fopen(inputfile, "rb")) == NULL) {
	fprintf(stderr, "%s (file not found)\n", inputfile);
	exit(EXIT_FAILURE);
    }
    for (pc = 1; fread(&code[pc], sizeof(instruction), 1, infile); pc++) {
	if (showcode)
	    printf("%12ld%12.12s%12ld%12ld\n", pc,
		operator_NAMES[code[pc].op], code[pc].adr1, code[pc].adr2);
    }
    fclose(infile);

    /* interpret: */
    if (tracing)
	printf("interpreting ...\n");
    for (pc = 1;;) {
again:
	if (tracing)
	    printf("%12ld%12.12s%12ld%12ld\n", pc,
		operator_NAMES[code[pc].op], code[pc].adr1, code[pc].adr2);
	switch (code[pc].op) {
	case add:
	    reg[code[pc].adr1] += reg[code[pc].adr2];
	    break;

	case sub:
	    reg[code[pc].adr1] -= reg[code[pc].adr2];
	    break;

	case mul:
	    reg[code[pc].adr1] *= reg[code[pc].adr2];
	    break;

	case dvd:
	    reg[code[pc].adr1] /= reg[code[pc].adr2];
	    break;

	case mdl:
	    reg[code[pc].adr1] %= reg[code[pc].adr2];
	    break;

	case eql:
	    reg[code[pc].adr1] = reg[code[pc].adr1] == reg[code[pc].adr2];
	    break;

	case neq:
	    reg[code[pc].adr1] = reg[code[pc].adr1] != reg[code[pc].adr2];
	    break;

	case gtr:
	    reg[code[pc].adr1] = reg[code[pc].adr1] > reg[code[pc].adr2];
	    break;

	case geq:
	    reg[code[pc].adr1] = reg[code[pc].adr1] >= reg[code[pc].adr2];
	    break;

	case lss:
	    reg[code[pc].adr1] = reg[code[pc].adr1] < reg[code[pc].adr2];
	    break;

	case leq:
	    reg[code[pc].adr1] = reg[code[pc].adr1] <= reg[code[pc].adr2];
	    break;

	case orr:
	    reg[code[pc].adr1] = reg[code[pc].adr1] == 1 ||
				 reg[code[pc].adr2] == 1;
	    break;

	case neg:
	    reg[code[pc].adr1] = 1 - reg[code[pc].adr1];
	    break;

	case loadglobl:
	    reg[code[pc].adr1] = stack[code[pc].adr2];
	    break;

	case loadlocal:
	    reg[code[pc].adr1] = stack[code[pc].adr2 + baseregister];
	    break;

	case loadimmed:
	    reg[code[pc].adr1] = code[pc].adr2;
	    break;

	case storglobl:
	    stack[code[pc].adr1] = reg[code[pc].adr2];
	    break;

	case storlocal:
	    stack[code[pc].adr1 + baseregister] = reg[code[pc].adr2];
	    break;

	case writebool:
	    puts(reg[code[pc].adr2] == 1 ? "TRUE" : "FALSE");
	    break;

	case writeint:
	    printf("%12ld\n", reg[code[pc].adr2]);
	    break;

	case jmp:
	    pc = code[pc].adr1;
	    goto again;

	case jiz:
	    if (reg[code[pc].adr2] == 0) {
		pc = code[pc].adr1;
		goto again;
	    }
	    break;

	case hlt:
	    pc = 0;
	    goto done;

	case cal:
	    if (stacktop + code[pc].adr2 > maxstack) {
		printf("stack overflow, PC=%6ld, execution aborted\n", pc);
		goto done;
	    }
	    stack[stacktop + 1] = baseregister;
	    stack[stacktop + 2] = pc + 1;
	    baseregister = stacktop;
	    stacktop += code[pc].adr2;
	    pc = code[pc].adr1;
	    goto again;

	case ret:
	    stacktop = baseregister;
	    baseregister = stack[stacktop + 1];
	    pc = stack[stacktop + 2];
	    goto again;
	}
	pc++;
    }
done:
    exit(EXIT_SUCCESS);
} /* main */

/* End. */
