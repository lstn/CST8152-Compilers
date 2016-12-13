
/* The #define _CRT_SECURE_NO_WARNINGS should be used in MS Visual Studio projects
* to suppress the warnings about using "unsafe" functions like fopen()
* and standard sting library functions defined in string.h.
* The define does not have any effect in Borland compiler projects.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>   /* standard input / output */
#include <ctype.h>   /* conversion functions */
#include <stdlib.h>  /* standard library functions and constants */
#include <string.h>  /* string functions */
#include <limits.h>  /* integer types constants */
#include <float.h>   /* floating-point types constants */

/*#define NDEBUG        to suppress assert() call */
#include <assert.h>  /* assert() prototype */

/* project header files */
#include "buffer.h"
#include "token.h"
#include "parser.h"

#define DEBUG  /* for conditional processing */
#undef  DEBUG


void parser(Buffer* in_buf){
	sc_buf = in_buf;
	lookahead = mlwpar_next_token(sc_buf);
	program(); match(SEOF_T, NO_ATTR);
	gen_incode("PLATY: Source file parsed");
}

void match(int pr_token_code, int pr_token_attribute){
	if (lookahead.code == pr_token_code){
		switch (pr_token_code){
			case KW_T:
			case LOG_OP_T:
			case ART_OP_T:
			case REL_OP_T:
				if (lookahead.attribute.get_int != pr_token_attribute)
					break; /* attributes do not match, break out */
			default: /* match */
				if (lookahead.attribute.get_int == SEOF_T)
					return;

				/* code was not SEOF, advance to next input*/
				lookahead = mlwpar_next_token(sc_buf);
				if (lookahead.attribute.get_int == ERR_T){
					syn_printe();
					lookahead = mlwpar_next_token(sc_buf);
					++synerrno;
				}
				return; /* successfully matched */
		}
	}
	
	syn_eh(pr_token_code); /* unsuccessful match */
}

void syn_eh(int sync_token_code){
	syn_printe();
	++synerrno;

	while (lookahead.code != SEOF_T){ /* panic recovery, advance until we have sync_token_code match */
		lookahead = mlwpar_next_token(sc_buf);
		if (lookahead.code == sync_token_code){ /* match */
			if (lookahead.code != SEOF_T) /* check if we need to advance one more time */
				lookahead = mlwpar_next_token(sc_buf);
			return; /* done */
		}
	}

	if (sync_token_code != SEOF_T){  /* reached end of source with no match, exit */
		exit(synerrno);
	}

}

/* Parser error printing function, Assignment 4, F16
*/
void syn_printe(){
	Token t = lookahead;

	printf("PLATY: Syntax error:  Line:%3d\n", line);
	printf("*****  Token code:%3d Attribute: ", t.code);
	switch (t.code){
	case  ERR_T: /* ERR_T     0   Error token */
		printf("%s\n", t.attribute.err_lex);
		break;
	case  SEOF_T: /*SEOF_T    1   Source end-of-file token */
		printf("NA\n");
		break;
	case  AVID_T: /* AVID_T    2   Arithmetic Variable identifier token */
	case  SVID_T:/* SVID_T    3  String Variable identifier token */
		printf("%s\n", sym_table.pstvr[t.attribute.get_int].plex);
		break;
	case  FPL_T: /* FPL_T     4  Floating point literal token */
		printf("%5.1f\n", t.attribute.flt_value);
		break;
	case INL_T: /* INL_T      5   Integer literal token */
		printf("%d\n", t.attribute.get_int);
		break;
	case STR_T:/* STR_T     6   String literal token */
		printf("%s\n", b_cbhead(str_LTBL) + t.attribute.str_offset);
		break;
	case SCC_OP_T: /* 7   String concatenation operator token */
		printf("NA\n");
		break;
	case  ASS_OP_T:/* ASS_OP_T  8   Assignment operator token */
		printf("NA\n");
		break;
	case  ART_OP_T:/* ART_OP_T  9   Arithmetic operator token */
		printf("%d\n", t.attribute.get_int);
		break;
	case  REL_OP_T: /*REL_OP_T  10   Relational operator token */
		printf("%d\n", t.attribute.get_int);
		break;
	case  LOG_OP_T:/*LOG_OP_T 11  Logical operator token */
		printf("%d\n", t.attribute.get_int);
		break;
	case  LPR_T: /*LPR_T    12  Left parenthesis token */
		printf("NA\n");
		break;
	case  RPR_T: /*RPR_T    13  Right parenthesis token */
		printf("NA\n");
		break;
	case LBR_T: /*    14   Left brace token */
		printf("NA\n");
		break;
	case RBR_T: /*    15  Right brace token */
		printf("NA\n");
		break;
	case KW_T: /*     16   Keyword token */
		printf("%s\n", kw_table[t.attribute.get_int]);
		break;
	case COM_T: /* 17   Comma token */
		printf("NA\n");
		break;
	case EOS_T: /*    18  End of statement *(semi - colon) */
		printf("NA\n");
		break;
	default:
		printf("PLATY: Scanner error: invalid token code: %d\n", t.code);
	}
}

void gen_incode(char *in_code){
	printf("%s\n", in_code);
}

/* syntax prototypes */

void program(void){
	match(KW_T, PLATYPUS); match(LBR_T, NO_ATTR); opt_statements();
	match(RBR_T, NO_ATTR);
	gen_incode("PLATY: Program parsed");
}

void opt_statements(void){
	/* FIRST set: {AVID_T,SVID_T,KW_T(but not … see above),e} */
	switch (lookahead.code){
		case AVID_T:
		case SVID_T: statements(); break;
		case KW_T:
			/* check for PLATYPUS, ELSE, THEN, REPEAT here and in
			statements_p()*/
			if (lookahead.attribute.get_int != PLATYPUS
				&& lookahead.attribute.get_int != ELSE
				&& lookahead.attribute.get_int != THEN
				&& lookahead.attribute.get_int != REPEAT){
				statements();
				break;
			}
		default: /*empty string – optional statements*/
			gen_incode("PLATY: Opt_statements parsed");
	}
}

void statements(void){
	statement(); statements_p();
}

void statements_p(void){
	switch (lookahead.code){
		case AVID_T:
		case SVID_T: statement(); statements_p(); break;
		case KW_T:
			/* check for PLATYPUS, ELSE, THEN, REPEAT */
			if (lookahead.attribute.get_int != PLATYPUS && lookahead.attribute.get_int != ELSE
					&& lookahead.attribute.get_int != THEN && lookahead.attribute.get_int != REPEAT)
			{ return; } else { break; } /* USING, IF, INPUT, OUTPUT */
		default: /* no match */
			syn_printe();
	}
}

void statement(void){
	switch (lookahead.code){
		case AVID_T:
		case SVID_T: assignment_statement(); break;
		case KW_T:
			switch (lookahead.attribute.get_int){
				case USING: iteration_statement(); break;
				case IF: selection_statement(); break;
				case INPUT: input_statement(); break;
				case OUTPUT: output_statement(); break;
				default: syn_printe(); return; /* no match */
			}
			break;
		default: /* no match */
			syn_printe();
	}
}

void assignment_statement(void){

}

void iteration_statement(void){

}

void input_statement(void){
	match(KW_T, INPUT); match(LPR_T, NO_ATTR); variable_list();
	match(RPR_T, NO_ATTR); match(EOS_T, NO_ATTR);
	gen_incode("PLATY: Input statement parsed");
}

void output_statement(void){

}

void selection_statement(void){

}

void variable_list(void){

}

