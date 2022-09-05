/*
    module  : 32syreci.c
    version : 1.4
    date    : 09/05/22
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

/* SYmboltable, RECursion, Interpreter only,
   interprets a file of instructions produced by syrecc */

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
    int64_t adr1, adr2;
} instruction;

static char *operator_NAMES[] = { "ADD", "SUB", "MUL", "DVD", "MDL", "EQL",
    "NEQ", "GTR", "GEQ", "LSS", "LEQ", "ORR", "NEG", "LOADGLOBL", "LOADLOCAL",
    "LOADIMMED", "STORGLOBL", "STORLOCAL", "WRITEBOOL", "WRITEINT", "CAL",
    "RET", "JMP", "JIZ", "HLT" };

void debug(instruction *pc, instruction *code)
{
    printf("%12" PRId64 "%12.12s%12" PRId64 "%12" PRId64 "\n",
	pc - code, operator_NAMES[pc->op], pc->adr1, pc->adr2);
}

int main(int argc, char *argv[])
{ /* main */
    FILE *fp;
    instruction code[maxcode], *pc;

    int64_t stack[maxstack + 1];
    int64_t stacktop = 0;
    int64_t reg[topregister + 1];
    int64_t baseregister = 0;

    printf("SYRECI ...\n");

    if ((fp = fopen(inputfile, "rb")) == NULL) {
	fprintf(stderr, "%s (file not found)\n", inputfile);
	exit(EXIT_FAILURE);
    }
    for (pc = &code[1]; fread(pc, sizeof(instruction), 1, fp); pc++)
	if (showcode)
	    debug(pc, code);
    fclose(fp);

    /* interpret: */
    if (tracing)
	printf("interpreting ...\n");
    pc = &code[1];
    for (;;) {
	if (tracing)
	    debug(pc, code);
	switch (pc->op) {
	case add:
	    reg[pc->adr1] += reg[pc->adr2];
	    pc++;
	    break;

	case sub:
	    reg[pc->adr1] -= reg[pc->adr2];
	    pc++;
	    break;

	case mul:
	    reg[pc->adr1] *= reg[pc->adr2];
	    pc++;
	    break;

	case dvd:
	    reg[pc->adr1] /= reg[pc->adr2];
	    pc++;
	    break;

	case mdl:
	    reg[pc->adr1] %= reg[pc->adr2];
	    pc++;
	    break;

	case eql:
	    reg[pc->adr1] = reg[pc->adr1] == reg[pc->adr2];
	    pc++;
	    break;

	case neq:
	    reg[pc->adr1] = reg[pc->adr1] != reg[pc->adr2];
	    pc++;
	    break;

	case gtr:
	    reg[pc->adr1] = reg[pc->adr1] > reg[pc->adr2];
	    pc++;
	    break;

	case geq:
	    reg[pc->adr1] = reg[pc->adr1] >= reg[pc->adr2];
	    pc++;
	    break;

	case lss:
	    reg[pc->adr1] = reg[pc->adr1] < reg[pc->adr2];
	    pc++;
	    break;

	case leq:
	    reg[pc->adr1] = reg[pc->adr1] <= reg[pc->adr2];
	    pc++;
	    break;

	case orr:
	    reg[pc->adr1] = reg[pc->adr1] == 1 || reg[pc->adr2] == 1;
	    pc++;
	    break;

	case neg:
	    reg[pc->adr1] = 1 - reg[pc->adr1];
	    pc++;
	    break;

	case loadglobl:
	    reg[pc->adr1] = stack[pc->adr2];
	    pc++;
	    break;

	case loadlocal:
	    reg[pc->adr1] = stack[pc->adr2 + baseregister];
	    pc++;
	    break;

	case loadimmed:
	    reg[pc->adr1] = pc->adr2;
	    pc++;
	    break;

	case storglobl:
	    stack[pc->adr1] = reg[pc->adr2];
	    pc++;
	    break;

	case storlocal:
	    stack[pc->adr1 + baseregister] = reg[pc->adr2];
	    pc++;
	    break;

	case writebool:
	    puts(reg[pc->adr2] == 1 ? "TRUE" : "FALSE");
	    pc++;
	    break;

	case writeint:
	    printf("%12" PRId64 "\n", reg[pc->adr2]);
	    pc++;
	    break;

	case cal:
	    if (stacktop + pc->adr2 > maxstack) {
		printf("stack overflow, PC=%" PRId64 ", execution aborted\n",
			pc - code);
		exit(EXIT_FAILURE);
	    }
	    stack[stacktop + 1] = baseregister;
	    stack[stacktop + 2] = pc + 1 - code;
	    baseregister = stacktop;
	    stacktop += pc->adr2;
	    pc = &code[pc->adr1];
	    break;

	case ret:
	    stacktop = baseregister;
	    baseregister = stack[stacktop + 1];
	    pc = &code[stack[stacktop + 2]];
	    break;

	case jmp:
	    pc = &code[pc->adr1];
	    break;

	case jiz:
	    if (reg[pc->adr2] == 0)
		pc = &code[pc->adr1];
	    else
		pc++;
	    break;

	case hlt:
	    exit(EXIT_SUCCESS);

	default:
#ifdef _MSC_VER
	    __assume(0);
#else
	    __builtin_unreachable();
#endif
	}
    }
    exit(EXIT_SUCCESS);
} /* main */

/* End. */
