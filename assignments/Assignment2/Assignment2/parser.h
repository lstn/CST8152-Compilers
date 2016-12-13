/* File name:	parser.h
*  Compiler:	MS Visual Studio 2013
*  Author:		Lucas Estienne, 040 819 959
*  Course:		CST 8152 - Compilers, Lab Section 012
*  Assignment:	04
*  Date:		15 December 2016
*  Professor:   Svillen Ranev
*  Purpose:		Parser implementation declarations & prototypes
*  Function List: 
*  Constants: 
*/

#ifndef  PARSER_H_
#define  PARSER_H_ 

#include "stable.h"

#ifndef NULL
#include <_null.h> /* NULL pointer constant is defined there */
#endif

/* constants */
#define NO_ATTR -1

/* declare globals */
static Token lookahead;
static Buffer *sc_buf;
int synerrno = 0;

/* externals */
extern int line; /* line number */
extern STD sym_table; /* symbol table */
extern Buffer * str_LTBL; /*String literal table */
extern char * kw_table[]; /* keyword table */
extern Token mlwpar_next_token(Buffer * sc_buf);


/* prototypes */
void parser(Buffer* in_buf);
void match(int pr_token_code, int pr_token_attribute);
void syn_eh(int sync_token_code);
void syn_printe(void);
void gen_incode(char *in_code);

/* syntax prototypes */
void program(void);

void opt_statements(void);
void statements(void);
void statements_p(void);
void statement(void);

void assignment_statement(void);
void iteration_statement(void);
void input_statement(void);
void output_statement(void);
void selection_statement(void);

void variable_list(void);


#endif
