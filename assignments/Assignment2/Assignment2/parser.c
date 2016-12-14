/* File name:	parser.c
*  Compiler:	MS Visual Studio 2013
*  Author:		Lucas Estienne, 040 819 959
*  Course:		CST 8152 - Compilers, Lab Section 012
*  Assignment:	02
*  Date:		15 December 2016
*  Professor:   Svillen Ranev
*  Purpose:		Implements the functions and logic required for the Parser component of the compiler.
*  Function List:  
*/

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

/**********************/
/* syntax productions */
/**********************/

/*
	<program>  ->
		PLATYPUS { <opt_statements> }

	FIRST set: { PLATYPUS }
*/
void program(void){
	match(KW_T, PLATYPUS); match(LBR_T, NO_ATTR); opt_statements();
	match(RBR_T, NO_ATTR);
	gen_incode("PLATY: Program parsed");
}

/*
	<opt_statements>  ->
		<statements> | e

	FIRST set: { AVID_T, SVID_T, KW_T(but not PLATYPUS, ELSE, THEN, REPEAT), e }
*/
void opt_statements(void){
	switch (lookahead.code){
		case AVID_T:
		case SVID_T: statements(); break;
		case KW_T:
			/* check for PLATYPUS, ELSE, THEN, REPEAT here and in
			statements_p()*/
			if (lookahead.attribute.get_int != PLATYPUS && lookahead.attribute.get_int != ELSE
					&& lookahead.attribute.get_int != THEN && lookahead.attribute.get_int != REPEAT){
				statements();
				break;
			}
		default: /*empty string - optional statements*/
			gen_incode("PLATY: Opt_statements parsed");
	}
}

/*
	<statements>  ->
		<statement><statements_p>

	FIRST set: { AVID_T, SVID_T, KW_T(but not PLATYPUS, ELSE, THEN, REPEAT) }
*/
void statements(void){
	statement(); statements_p();
}

/*
	<statements_p>  ->
		<statement><statements_p> | e

	FIRST set: { AVID_T, SVID_T, KW_T(but not PLATYPUS, ELSE, THEN, REPEAT), e}
*/
void statements_p(void){
	switch (lookahead.code){
		case AVID_T:
		case SVID_T: statement(); statements_p(); break;
		case KW_T:
			/* check for PLATYPUS, ELSE, THEN, REPEAT */
			if (lookahead.attribute.get_int != PLATYPUS && lookahead.attribute.get_int != ELSE
					&& lookahead.attribute.get_int != THEN && lookahead.attribute.get_int != REPEAT) {
				statement(); 
				statements_p(); 
				break;
			} else {
				return; 
			} /* USING, IF, INPUT, OUTPUT */
		default: /* no match */
			syn_printe();
	}
}

/*
	<statement>  ->
		<assignment_statement>
		| <iteration_statement>
		| <selection_statement>
		| <input_statement>
		| <output_statement>

	FIRST set: { AVID_T, SVID_T, KW_T(but not PLATYPUS, ELSE, THEN, REPEAT)}
*/
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

/*
	<assignment_statement> ->
		<assignment_expression>

	FIRST set: { AVID_T, SVID_T }
*/
void assignment_statement(void){
	assignment_expression(); match(EOS_T, NO_ATTR);
	gen_incode("PLATY: Assignment_statement parsed");
}

/*
	<iteration statement> ->
		USING  (<assignment_expression> , <conditional_expression> , <assignment_expression>)
		REPEAT {
			<opt_statements>
		};

	FIRST set: { USING }
*/
void iteration_statement(void){
	match(KW_T, USING); match(LPR_T, NO_ATTR);
	assignment_expression(); match(COM_T, NO_ATTR); conditional_expression();
	match(COM_T, NO_ATTR); assignment_expression(); match(RPR_T, NO_ATTR);

	match(KW_T, REPEAT); match(LBR_T, NO_ATTR); 
	opt_statements(); match(RBR_T, NO_ATTR); match(EOS_T, NO_ATTR);
	gen_incode("PLATY: Iteration_statement parsed");
}

/*
	<input_statement> ->
		INPUT (<variable_list>);

	FIRST set: { INPUT }
*/
void input_statement(void){
	match(KW_T, INPUT); match(LPR_T, NO_ATTR); variable_list();
	match(RPR_T, NO_ATTR); match(EOS_T, NO_ATTR);
	gen_incode("PLATY: Input statement parsed");
}

/*
	<output statement> ->
		OUTPUT (<out list>);

	FIRST set: { OUTPUT }
*/
void output_statement(void){
	match(KW_T, OUTPUT); match(LPR_T, NO_ATTR);
	out_list();
	match(RPR_T, NO_ATTR); match(EOS_T, NO_ATTR);
	gen_incode("PLATY: Output_statement parsed");
}

/*
	<selection_statement> ->
		IF (<conditional_expression>)  THEN
			<opt_statements>
		ELSE { <opt_statements> } ;

	FIRST set: { IF }
*/
void selection_statement(void){
	match(KW_T, IF); match(LPR_T, NO_ATTR);
	conditional_expression(); /* condition */
	match(RPR_T, NO_ATTR);match(KW_T, THEN);
	opt_statements(); /* if true */
	match(KW_T, ELSE); match(LBR_T, NO_ATTR);
	opt_statements(); /* else */
	match(RBR_T, NO_ATTR); match(EOS_T, NO_ATTR);
	gen_incode("PLATY: Selection_statement parsed");
}

void out_list(void){

}

void variable_list(void){

}

/*
	<assignment_expression> ->
		AVID = <arithmetic_expression>
		| SVID = <string_expression>

	FIRST set: { AVID_T, SVID_T }
*/
void assignment_expression(void){
	switch (lookahead.code){
	case AVID_T:
		match(AVID_T, NO_ATTR); match(ASS_OP_T, NO_ATTR); /* AVID = ... */
		arithmetic_expression();
		break;
	case SVID_T:
		match(SVID_T, NO_ATTR); match(ASS_OP_T, NO_ATTR); /* SVID = ... */
		string_expression();
		break;
	default: /* no match */
		syn_printe();
		return;
	}
	gen_incode("PLATY: Assignment_expression parsed");
}

void arithmetic_expression(void){

}

void conditional_expression(void){

}

void string_expression(void){

}

