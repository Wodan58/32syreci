/*
    module  : 32syreci.h
    version : 1.1
    date    : 08/17/23
*/

/* ----------------------------- D E F I N E S ----------------------------- */

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

/* ------------------------------- T Y P E S ------------------------------- */

typedef struct instruction {
    operator op;
    int64_t adr1, adr2;
} instruction;

/* --------------------------- V A R I A B L E S --------------------------- */

char *operator_NAMES[] = {
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
