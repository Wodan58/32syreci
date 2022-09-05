/*
    module  : 32syrecc.c
    version : 1.2
    date    : 09/05/22
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>

/* ----------------------------- D E F I N E S ----------------------------- */

/*
    maximum significant length of a variable name or keyword.
*/
#define MAXVAR	16

/*
    maximum keyword index; keywords are 0 .. MAXKEY.
*/
#define MAXKEY	18
#define MAXIDX	24
#define MINCHR	33

/*
    maximum size of the symbol table. There will be an error when the table is
    exceeded.
*/
#define MAXSYM	100

/*
    maximum size of the instruction table. The size is taken from 32syreci.c
*/
#define MAXPRG	200

/*
    Symbol types. The numbers 0-18 are keywords. Valid single characters are:
    ( ) * + - / . ; < = >
*/
enum {
    typ_and,
    typ_begin,
    typ_boolean,
    typ_do,
    typ_end,
    typ_endif,
    typ_endwhile,
    typ_false,
    typ_if,
    typ_iff,
    typ_integer,
    typ_mod,
    typ_not,
    typ_or,
    typ_procedure,
    typ_then,
    typ_true,
    typ_while,
    typ_write,

    typ_variable,	/* 19 */
    typ_number,		/* 20 */
    typ_assign,		/* := */
    typ_unequal,	/* <> */
    typ_lesseql,	/* <= */
    typ_moreeql		/* >= */
};

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

typedef struct symbol_t {
    char *name;
    int type,	/* 0=boolean, 1=integer, >=2 address */
	parm;	/* number of local variables */
} symbol_t;

typedef struct instruction {
    operator op;
    int64_t adr1, adr2;
} instruction;

/* --------------------------- V A R I A B L E S --------------------------- */

static char *operator_NAMES[] = { "ADD", "SUB", "MUL", "DVD", "MDL", "EQL",
    "NEQ", "GTR", "GEQ", "LSS", "LEQ", "ORR", "NEG", "LOADGLOBL", "LOADLOCAL",
    "LOADIMMED", "STORGLOBL", "STORLOCAL", "WRITEBOOL", "WRITEINT", "CAL",
    "RET", "JMP", "JIZ", "HLT" };

char *keywords[] = {
    "AND",
    "BEGIN",
    "BOOLEAN",
    "DO",
    "END",
    "ENDIF",
    "ENDWHILE",
    "FALSE",
    "IF",
    "IFF",
    "INTEGER",
    "MOD",
    "NOT",
    "OR",
    "PROCEDURE",
    "THEN",
    "TRUE",
    "WHILE",
    "WRITE",

    "VARIABLE",
    "NUMBER",
    ":=",
    "<>",
    "<=",
    ">="
};

int code_idx = 1;
instruction code[MAXPRG];

int function_idx, global_idx, local_idx;
symbol_t functions[MAXSYM], globals[MAXSYM], locals[MAXSYM];

char val_variable[MAXVAR + 1];
int linenum = 1, symbol, val_number;

/* --------------------------- F U N C T I O N S --------------------------- */

/*
    error prints a message to stderr, while reporting what was seen instead.
*/
void error(char *msg)
{
    fprintf(stderr, "%d: %s\nsymbol=", linenum, msg);
    if (symbol <= MAXIDX)
	fprintf(stderr, "%s", keywords[symbol]);
    else
	fprintf(stderr, symbol >= MINCHR ? "%c" : "%d", symbol);
    fprintf(stderr, ", variable=%s, number=%d\n", val_variable, val_number);
}

void enterprog(operator op, int64_t adr1, int64_t adr2)
{
    if (++code_idx >= MAXPRG) {
	error("Exceeding code array");
	return;
    }
    code[code_idx].op = op;
    code[code_idx].adr1 = adr1;
    code[code_idx].adr2 = adr2;
}

void enterglobal(int type)
{
    if (global_idx >= MAXSYM) {
	error("Exceeding global symbol table");
	return;
    }
    globals[global_idx].name = strdup(val_variable);
    globals[global_idx].type = type;
    globals[global_idx].parm = 0;
    global_idx++;
}

void enterfunction(int type)
{
    if (function_idx >= MAXSYM) {
	error("Exceeding function symbol table");
	return;
    }
    functions[function_idx].name = strdup(val_variable);
    functions[function_idx].type = type;
    functions[function_idx].parm = 0;
    function_idx++;
}

void enterlocal(int type)
{
    if (local_idx >= MAXSYM) {
	error("Exceeding local symbol table");
	return;
    }
    locals[local_idx].name = strdup(val_variable);
    locals[local_idx].type = type;
    locals[local_idx].parm = 0;
    local_idx++;
}

/*
    lookup locates a symbol in the symbol table. Found = 1 means found in local
    table; found = 0 means found in global table; found = 2 means found in
    function table, and found = -1 means not found at all.
*/
int lookup(char *str, int *found, int *type)
{
    int i;

    for (i = local_idx - 1; i >= 0; i--)
	if (!strcmp(str, locals[i].name)) {
	    *found = 1;
	    *type = locals[i].type;
	    return i;
	}
    for (i = global_idx - 1; i >= 0; i--)
	if (!strcmp(str, globals[i].name)) {
	    *found = 0;
	    *type = globals[i].type;
	    return i;
	}
    for (i = function_idx - 1; i >= 0; i--)
	if (!strcmp(str, functions[i].name)) {
	    *found = 2;
	    *type = functions[i].type;
	    return i;
	}
    *found = -1;
    return -1;
}

/*
    getch reads a single character. Stops at end of file.
*/
int getch()
{
    int ch;

    if ((ch = getchar()) == EOF)
	exit(0);
    if (ch == '\n')
	linenum++;
    return ch;
}

/*
    ungetch unreads a character. Whitespace need not be unread.
*/
void ungetch(int ch)
{
    if (!isspace(ch))
	ungetc(ch, stdin);
}

/*
    getsym reads a symbol, stores the text of the symbol in the global variable
    val_variable if it is a variable or val_number if it is a number and
    returns the type.
*/
void getsym()
{
    int ch, i = 0;
    char str[MAXVAR + 1];

    do
	ch = getch();
    while (isspace(ch));
    if (ch == '-') {			/* deal with hyphen first, complex */
	ch = getch();
	if (isspace(ch)) {
	    symbol = '-';
	    return;
	}
	if (isalpha(ch)) {
	    str[i++] = '-';
	    goto alpha;
	}
	if (isdigit(ch)) {
	    str[i++] = '-';
	    goto digit;
	}
	ungetch(ch);			/*  1 character too many, read again */
	symbol = '-';
	return;
    }
    if (ch == '<') {			/* deal with unequal next */
	ch = getch();
	if (ch == '>') {
	    symbol = typ_unequal;
	    return;
	}
	if (ch == '=') {
	    symbol = typ_lesseql;
	    return;
	}
	ungetch(ch);
	symbol = '<';
	return;
    }
    if (ch == '>') {
	ch = getch();
	if (ch == '=') {
	    symbol = typ_moreeql;
	    return;
	}
	ungetch(ch);
	symbol = '>';
	return;
    }
    if (ch == ':') {
	ch = getch();
	if (ch == '=') {
	    symbol = typ_assign;
	    return;
	}
	ungetch(ch);			/* not assignment, single colon */
	symbol = ':';			/* unexpected character */
	return;
    }
    if (strchr("()*+/.;=", ch)) {	/* rest of single characters */
	symbol = ch;
	return;
    }
    if (isalpha(ch))
	goto alpha;
    if (isdigit(ch))
	goto digit;
    symbol = ch;			/* unexpected character */
    return;
alpha:
    do {
	if (i < MAXVAR)
	    str[i++] = ch;
	ch = getch();
    } while (isalnum(ch) || ch == '-');
    str[i] = 0;
    ungetch(ch);			/* undo last  */
    for (i = 0; i <= MAXKEY; i++)
	if (!strcmp(str, keywords[i])) {
	    symbol = i;			/* keyword */
	    return;
	}
    strcpy(val_variable, str);
    symbol = typ_variable;		/* variable */
    return;
digit:
    do {
	if (i < MAXVAR)
	    str[i++] = ch;
	ch = getch();
    } while (isdigit(ch));
    str[i] = 0;
    ungetch(ch);			/* undo last  */
    val_number = atoi(str);
    symbol = typ_number;		/* number */
}

void expr2(int, int *);

/*
factor ::= variable | number | "FALSE" | "TRUE" | "NOT" factor | "(" expr2 ")"
*/
void factor(int regnum, int *type)
{
    int index, found;

    switch (symbol) {
    case typ_variable:
	index = lookup(val_variable, &found, type);
	if (found == -1)
	    error("variable not found");	/* undeclared variable */
	else if (found == 1)
	    enterprog(loadlocal, regnum, index);
	else if (found == 0)
	    enterprog(loadglobl, regnum, index);
	getsym();
	break;
    case typ_number:
	*type = 1;
	enterprog(loadimmed, regnum, val_number);
	getsym();
	break;
    case typ_false:
	*type = 0;
	enterprog(loadimmed, regnum, 0);
	getsym();
	break;
    case typ_true:
	*type = 0;
	enterprog(loadimmed, regnum, 1);
	getsym();
	break;
    case typ_not:
	getsym();
	factor(regnum, type);
	if (*type != 0)
	    error("boolean type expected for operator not");
	if (code[code_idx].op == loadimmed)
	    code[code_idx].adr2 = -code[code_idx].adr2;
	else
	    enterprog(neg, regnum, 0);
	break;
    case '(':
	getsym();
	expr2(regnum, type);
	if (symbol != ')')
	    error("')' expected at end of parenthesized expression");
	getsym();
	break;
    default:
	error("illegal start of factor");
    }
}

/*
term1  ::= factor [ ( "*" | "/" | "MOD" ) factor ]
*/
void term1(int regnum, int *type)
{
    int oper, type2;

    factor(regnum, type);
    while (symbol == '*' || symbol == '/' || symbol == typ_mod) {
	oper = symbol;
	getsym();
	factor(regnum + 1, &type2);
	if (*type != 1 || type2 != 1)
	    error("integer type expected for *,/,MOD");
	if (oper == '*') {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 *= code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(mul, regnum, regnum + 1);
	} else if (oper == '/') {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 /= code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(dvd, regnum, regnum + 1);
	} else if (oper == typ_mod) {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 %= code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(mdl, regnum, regnum + 1);
	}
    }
}

/*
expr1  ::= term1 [ ( "+" | "-" ) term1 ]
*/
void expr1(int regnum, int *type)
{
    int oper, type2;

    term1(regnum, type);
    while (symbol == '+' || symbol == '-') {
	oper = symbol;
	getsym();
	term1(regnum + 1, &type2);
	if (*type != 1 || type2 != 1)
	    error("integer type expected for +,-");
	if (oper == '+') {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 += code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(add, regnum, regnum + 1);
	} else if (oper == '-') {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 -= code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(sub, regnum, regnum + 1);
	}
    }
}

/*
compar ::= expr1 [ ( "<" | "=" | ">" | "<>" | "<=" | ">=" ) expr1 ]
*/
void compar(int regnum, int *type)
{
    int oper, type2;

    expr1(regnum, type);
    while (symbol == '<' || symbol == '=' || symbol == '>' ||
	   symbol == typ_unequal || symbol == typ_lesseql ||
	   symbol == typ_moreeql) {
	oper = symbol;
	getsym();
	expr1(regnum + 1, &type2);
	if (*type != type2)
	    error("same type expected in comparison");
	*type = 0;
	if (oper == '<') {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 = code[code_idx - 1].adr2 <
					  code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(lss, regnum, regnum + 1);
	} else if (oper == '=') {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 = code[code_idx - 1].adr2 ==
					  code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(eql, regnum, regnum + 1);
	} else if (oper == '>') {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 = code[code_idx - 1].adr2 >
					  code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(gtr, regnum, regnum + 1);
	} else if (oper == typ_unequal) {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 = code[code_idx - 1].adr2 !=
					  code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(neq, regnum, regnum + 1);
	} else if (oper == typ_lesseql) {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 = code[code_idx - 1].adr2 <=
					  code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(leq, regnum, regnum + 1);
	} else if (oper == typ_moreeql) {
	    if (code[code_idx].op == loadimmed &&
		code[code_idx - 1].op == loadimmed) {
		code[code_idx - 1].adr2 = code[code_idx - 1].adr2 >=
					  code[code_idx].adr2;
		code_idx--;
	    } else
		enterprog(geq, regnum, regnum + 1);
	}
    }
}

/*
term2  ::= compar [ "AND" compar ]
*/
void term2(int regnum, int *type)
{
    int type2;

    compar(regnum, type);
    while (symbol == typ_and) {
	getsym();
	compar(regnum + 1, &type2);
	if (*type != 0 || type2 != 0)
	    error("boolean types expected for operator and");
	if (code[code_idx].op == loadimmed &&
	    code[code_idx - 1].op == loadimmed) {
	    code[code_idx - 1].adr2 &= code[code_idx].adr2;
	    code_idx--;
	} else
	    enterprog(mul, regnum, regnum + 1);	/* no 'and' available */
    }
}

/*
sexpr  ::= term2 [ "OR" term2 ]
*/
void sexpr(int regnum, int *type)
{
    int type2;

    term2(regnum, type);
    while (symbol == typ_or) {
	getsym();
	term2(regnum + 1, &type2);
	if (*type != 0 || type2 != 0)
	    error("boolean types expected for operator or");
	if (code[code_idx].op == loadimmed &&
	    code[code_idx - 1].op == loadimmed) {
	    code[code_idx - 1].adr2 |= code[code_idx].adr2;
	    code_idx--;
	} else
	    enterprog(orr, regnum, regnum + 1);
    }
}

/*
expr2  ::= sexpr [ "IFF" sexpr ]
*/
void expr2(int regnum, int *type)
{
    sexpr(regnum, type);
    while (symbol == typ_iff) {
	getsym();
	sexpr(regnum + 1, type);
	/* not sure what code to generate */
    }
}

void statementseq(int, int *);

/*
statement ::=	variable ":=" expr2 |
		procedure |
		"WRITE" expr2 |
		"IF" expr2 "THEN" statementseq "ENDIF" |
		"WHILE" expr2 "DO" statementseq "ENDWHILE"
*/
void statement(int regnum, int *type)
{
    int index, found, type2, target[2];

    if (symbol == typ_variable) {
	index = lookup(val_variable, &found, type);
	if (found == -1)
	    error("variable/function not found");
	getsym();
	if (symbol == typ_assign) {
	    getsym();
	    expr2(regnum, &type2);
	    if (*type != type2)
		error("same type expected in assignment");
	    if (found == 1)
		enterprog(storlocal, index, regnum);
	    else if (found == 0)
		enterprog(storglobl, index, regnum);
	} else {
	    if (found != 2)
		error("procedure name expected");
	    enterprog(cal, *type, functions[index].parm);
	}
    } else if (symbol == typ_write) {
	getsym();
	expr2(regnum, type);
	if (*type == 0)
	    enterprog(writebool, 0, regnum);
	else if (*type == 1)
	    enterprog(writeint, 0, regnum);
    } else if (symbol == typ_if) {
	getsym();
	expr2(regnum, type);
	if (*type != 0)
	    error("boolean condition expected in if");
	if (symbol != typ_then)
	    error("THEN expected after if-condition");
	enterprog(jiz, 0, regnum);
	target[0] = code_idx;			/* to be fixed */
	getsym();
	statementseq(regnum, type);
	if (symbol != typ_endif)
	    error("ENDIF expected at end of if statement");
	code[target[0]].adr1 = code_idx + 1;	/* fixing */
	getsym();
    } else if (symbol == typ_while) {
	getsym();
	target[0] = code_idx + 1;		/* target of jump */
	expr2(regnum, type);
	if (*type != 0)
	    error("boolean condition expected in while");
	if (symbol != typ_do)
	    error("DO expected after while-condition");
	enterprog(jiz, 0, regnum);
	target[1] = code_idx;			/* to be fixed */
	getsym();
	statementseq(regnum, type);
	if (symbol != typ_endwhile)
	    error("ENDWHILE expected at end of while statement");
	enterprog(jmp, target[0], 0);
	code[target[1]].adr1 = code_idx + 1;	/* fixing */
	getsym();
    }
}

/*
statementseq ::= statement [ ";" statement ]
*/
void statementseq(int regnum, int *type)
{
    statement(regnum, type);
    while (symbol == ';') {
	getsym();
	statement(regnum, type);
	/* no extra code needs to be generated for sequential evaluation */
    }
}

/*
body ::= "BEGIN" statementseq "END"
*/
void body(int regnum, int *type)
{
    if (symbol != typ_begin)
	error("BEGIN expected at start of body");
    getsym();
    statementseq(regnum, type);
    if (symbol != typ_end)
	error("END expected at end of body");
    getsym();
}

/*
program ::= [ ( "BOOLEAN" | "INTEGER" ) [ identifier ] |
		"PROCEDURE" identifier
		[ ( "BOOLEAN" | "INTEGER" ) [ identifier ] ]
		body ]
		body "."
*/
void program()
{
    int type, index, found, type2, target, regnum = 0;

    getsym();
    while (symbol == typ_boolean || symbol == typ_integer ||
	   symbol == typ_procedure) {
	if (symbol == typ_boolean || symbol == typ_integer) {
	    type = symbol == typ_integer;
	    do {
		getsym();
		if (symbol == typ_variable) {
		    lookup(val_variable, &found, &type2);
		    if (found == -1)
			enterglobal(type);
		    else
			error("global variable already exists");
		}
	    } while (symbol == typ_variable);
	} else {
	    getsym();	/* name of procedure */
	    lookup(val_variable, &found, &type2);
	    if (found == -1)
		enterfunction(code_idx + 1);
	    else
		error("procedure name already exists");
	    target = function_idx - 1;
	    getsym();
	    index = local_idx;
	    while (symbol == typ_boolean || symbol == typ_integer) {
		type = symbol == typ_integer;
		do {
		    getsym();
		    if (symbol == typ_variable) {
			lookup(val_variable, &found, &type2);
			if (found == -1)
			    enterlocal(type);
			else
			    error("local variable already exists");
		    }
		} while (symbol == typ_variable);
	    }
	    functions[target].parm = 2 + local_idx - index;
	    body(regnum, &type);
	    local_idx = index;
	    enterprog(ret, 0, 0);
	}
    }
    code[1].op = cal;
    code[1].adr1 = code_idx + 1;
    code[1].adr2 = global_idx;
    body(regnum, &type);
    if (symbol != '.')
	error("full stop expected at end of program");
    enterprog(hlt, 0, 0);
}

/*
    This program writes to stdout that is then transformed to a binary file
    by the dump program.
*/
int main()
{
    int i;

    program();
    for (i = 1; i <= code_idx; i++)
	printf("%8d%15s%12" PRId64 "%12" PRId64 "\n",
		i, operator_NAMES[code[i].op], code[i].adr1, code[i].adr2);
    return 0;
}
