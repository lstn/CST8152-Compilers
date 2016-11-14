/* Filename: scanner.c
/* PURPOSE:
 *    SCANNER.C: Functions implementing a Lexical Analyzer (Scanner)
 *    as required for CST8152, Assignment #2
 *    scanner_init() must be called before using the scanner.
 *    The file is incomplete;
 *    Provided by: Svillen Ranev
 *    Version: 1.16.02
 *    Date: 29 September 2016
 *******************************************************************
 *    REPLACE THIS HEADER WITH YOUR HEADER
 *******************************************************************
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
#include "token.h"
#include "table.h"

#define DEBUG  /* for conditional processing */
#undef  DEBUG

/* Global objects - variables */
/* This buffer is used as a repository for string literals.
   It is defined in platy_st.c */
extern Buffer * str_LTBL; /*String literal table */
int line; /* current line number of the source code */
extern int scerrnum;     /* defined in platy_st.c - run-time error number */

/* Local(file) global objects - variables */
static Buffer *lex_buf;/*pointer to temporary lexeme buffer*/

/* No other global variable declarations/definitiond are allowed */

/* scanner.c static(local) function  prototypes */ 
static int char_class(char c); /* character class function */
static int get_next_state(int, char, int *); /* state machine function */
static int iskeyword(char * kw_lexeme); /*keywords lookup functuion */
static long atool(char * lexeme); /* converts octal string to decimal value */

int scanner_init(Buffer * sc_buf) {
  	if(b_isempty(sc_buf)) return EXIT_FAILURE;/*1*/
	/* in case the buffer has been read previously  */
	b_setmark(sc_buf, 0);
	b_retract_to_mark(sc_buf);
	b_reset(str_LTBL);
	line = 1;
	return EXIT_SUCCESS;/*0*/
/*   scerrnum = 0;  *//*no need - global ANSI C */
}

Token mlwpar_next_token(Buffer * sc_buf)
{
	Token t; /* token to return after recognition */
	unsigned char c; /* input symbol */
	int state = 0; /* initial state of the FSM */
	short lexstart;  /*start offset of a lexeme in the input buffer */
	short lexend;    /*end   offset of a lexeme in the input buffer */
	int accept = NOAS; /* type of state - initially not accepting */                                     
	/* 
	lexstart is the offset from the beginning of the char buffer of the
	input buffer (sc_buf) to the first character of the current lexeme,
	which is being processed by the scanner.
	lexend is the offset from the beginning of the char buffer of the
	input buffer (sc_buf) to the last character of the current lexeme,
	which is being processed by the scanner.

	*/ 
        
        
		/*DECLARE YOUR VARIABLES HERE IF NEEDED */
	int continue_loop = 1;
	unsigned int j;
	unsigned char nextc;
	char and_op_check[] = "ND.";
	char or_op_check[] = "R.";
        
                
	while (continue_loop){ /* endless loop broken by token returns it will generate a warning */
        c = b_getc(sc_buf);
		switch (c){
			case ' ':
			case '\t':
			case '\v':
			case '\f':
			case '\r':
				break;
			case '\n':
				++line;
				break;
			case '\0':
				t.code = SEOF_T;
				continue_loop = 0; break;
			case ';':
				t.code = EOS_T;
				continue_loop = 0; break;
			case '=': /* also == */
			{
				if (b_getcoffset(sc_buf) < b_size(sc_buf)){
					nextc = b_getc(sc_buf);
					if (nextc == '='){
						t.code = REL_OP_T;
						t.attribute.rel_op = EQ;
						continue_loop = 0; break;
					}
					b_retract(sc_buf); /* go back 1, next char was not '=' */
				}
				t.code = ASS_OP_T;
				continue_loop = 0; break;
			}
			case '!': /*!<*/
			{
				lexstart = b_getcoffset(sc_buf);
				b_setmark(sc_buf, lexstart);
				if (lexstart < b_size(sc_buf)){
					nextc = b_getc(sc_buf);
					if (nextc == '<'){
						discard_line(sc_buf);
						continue_loop = 0; break;
					}
					b_retract(sc_buf); /* go back 1, next char was not '<' */
				}

				/* syntax error */
				t.code = ERR_T;
				b_retract_to_mark(sc_buf);
				t.attribute.err_lex[0] = c;
				t.attribute.err_lex[1] = b_getc(sc_buf);
				t.attribute.err_lex[2] = '\0';
				continue_loop = 0; break;
			}
			case '<': /* also <> */
			{
				if (b_getcoffset(sc_buf) < b_size(sc_buf)){
					nextc = b_getc(sc_buf);
					if (nextc == '>'){
						t.code = REL_OP_T;
						t.attribute.rel_op = NE;
						continue_loop = 0; break;
					}
					b_retract(sc_buf); /* go back 1, next char was not '>' */
				}
				t.code = REL_OP_T;
				t.attribute.rel_op = LT;
				continue_loop = 0; break;
			}
			case '>':
				t.code = REL_OP_T;
				t.attribute.rel_op = GT;
				continue_loop = 0; break;
			case '+':
				t.code = ART_OP_T;
				t.attribute.arr_op = PLUS;
				continue_loop = 0; break;
			case '-':
				t.code = ART_OP_T;
				t.attribute.arr_op = MINUS;
				continue_loop = 0; break;
			case '*':
				t.code = ART_OP_T;
				t.attribute.arr_op = MULT;
				continue_loop = 0; break;
			case '/':
				t.code = ART_OP_T;
				t.attribute.arr_op = DIV;
				continue_loop = 0; break;
			case '.': /* .AND. .OR. */
			{
				lexstart = b_getcoffset(sc_buf);
				b_setmark(sc_buf, lexstart);
				if (b_getcoffset(sc_buf) < b_size(sc_buf)){
					nextc = b_getc(sc_buf);
					if (nextc == 'O'){
						for (j = 1; j < sizeof(or_op_check); j++)
							if (b_getc(sc_buf) != or_op_check[j - 1]) break;
						if (j == 3){
							t.code = LOG_OP_T;
							t.attribute.log_op = OR;
							continue_loop = 0; break;
						}
					}
					if (nextc == 'A'){
						for (j = 1; j < sizeof(and_op_check); j++)
							if (b_getc(sc_buf) != and_op_check[j - 1]) break;
						if (j == 4){
							t.code = LOG_OP_T;
							t.attribute.log_op = AND;
							continue_loop = 0; break;
						}
					}
				}

				/* syntax error */
				t.code = ERR_T;
				b_retract_to_mark(sc_buf);
				t.attribute.err_lex[0] = c;
				t.attribute.err_lex[1] = b_getc(sc_buf);
				t.attribute.err_lex[2] = '\0';
				continue_loop = 0; break;
			}
			case '"':
			{
				lexstart = b_getcoffset(sc_buf)+1;
				b_setmark(sc_buf, lexstart);
				if (b_setmark(str_LTBL, b_size(str_LTBL)) == -1){
					scerrnum = 1;
					t = aa_table[ES]("RUN TIME ERROR: ");
					continue_loop = 0; break;
				}

				while (b_getcoffset(sc_buf) < b_size(sc_buf)){
					nextc = b_getc(sc_buf);
					if (nextc == '\0'){
						t.code = ERR_T;
						b_retract(sc_buf);
						/* add error handling */
						continue_loop = 0; break;
					}
					if (nextc == '"'){
						t.code = STR_T;
						break;
					}
					if (nextc == '\n') line++;
				}
				if (continue_loop == 0) break;
				lexend = b_getcoffset(sc_buf);

				b_retract_to_mark(lexstart);
				while (b_getcoffset(sc_buf) < lexend) b_addc(str_LTBL, b_getc(sc_buf));

				b_getc(sc_buf);

				if (!b_addc(str_LTBL, '\0')){
					scerrnum = 2;
					t = aa_table[ES]("RUN TIME ERROR: ");
					continue_loop = 0; break;
				}

				t.attribute.str_offset = b_mark(str_LTBL);
				continue_loop = 0; break;
			}
			case '#':
				t.code = SCC_OP_T;
				continue_loop = 0; break;
			case ',':
				t.code = COM_T;
				continue_loop = 0; break;
			case '{':
				t.code = LBR_T;
				continue_loop = 0; break;
			case '}':
				t.code = RBR_T;
				continue_loop = 0; break;
			case '(':
				t.code = LPR_T;
				continue_loop = 0; break;
			case ')':
				t.code = RPR_T;
				continue_loop = 0; break;
			default:
				if (!isalnum(c)){
					t.code = ERR_T;
					t.attribute.err_lex[0] = c;
					t.attribute.err_lex[1] = '\0';
					break;
				}
				lexstart = b_getcoffset(sc_buf);
				b_setmark(sc_buf, lexstart);

				do{ /* FSM 0 */
					state = get_next_state(state, c, &accept); /* FSM 1 */
					if (accept != NOAS) break; /* FSM 3 */
					c = b_getc(sc_buf); /* FSM 2 */
				} while (accept = NOAS); /* FSM 3 */
				lexend = b_getcoffset(sc_buf);

				lex_buf = b_create(lexend-lexstart+2, 0, 'f');
				if (!lex_buf){
					scerrnum = 3;
					t = aa_table[ES]("RUN TIME ERROR: ");
					continue_loop = 0; break;
				}

				if (accept == ASWR) --lexend;

				b_retract_to_mark(sc_buf);
				while (b_getcoffset(sc_buf) < lexend) b_addc(lex_buf, b_getc(sc_buf));
				if (!b_addc(lex_buf, '\0')){
					scerrnum = 4;
					t = aa_table[ES]("RUN TIME ERROR: ");
					continue_loop = 0; break;
				}

				t = aa_table[state](b_cbhead(lex_buf));

				b_free(lex_buf);
				continue_loop = 0; break;
		}
   }
}

int get_next_state(int state, char c, int *accept)
{
	int col;
	int next;
	col = char_class(c);
	next = st_table[state][col];
#ifdef DEBUG
	printf("Input symbol: %c Row: %d Column: %d Next: %d \n",c,state,col,next);
#endif
    assert(next != IS);
#ifdef DEBUG
	if(next == IS){
		printf("Scanner Error: Illegal state:\n");
		printf("Input symbol: %c Row: %d Column: %d\n", c, state, col);
		exit(1);
	}
#endif
	*accept = as_table[next];
	return next;
}

int char_class (char c)
{
		if (isalpha(c))
			return (c == 'i' || c == 'o' || c == 'w' || c == 'd') ? PVAR_INT : PVAR_FLT;
		
		if (isdigit(c))
			return (c == 0) ? PVAL_ZERO : (c < '8') ? PVAL_OCTAL : PVAL_DEC;
		
		return (c == '%') ? PVAR_STR : (c == '.') ? PVAL_DOT : PVAL_DEFAULT;
}




Token aa_func05(char lexeme[]){
	Token t;
	int is_kw;

	is_kw = iskeyword(lexeme);
	if (is_kw != -1){
		t.code = KW_T;
		t.attribute.kwt_idx = is_kw;
		return t;
	}


/*
WHEN CALLED THE FUNCTION MUST
1. CHECK IF THE LEXEME IS A KEYWORD.
   IF YES, IT MUST RETURN A TOKEN WITH THE CORRESPONDING ATTRIBUTE
   FOR THE KEYWORD. THE ATTRIBUTE CODE FOR THE KEYWORD
   IS ITS INDEX IN THE KEYWORD LOOKUP TABLE (kw_table in table.h).
   IF THE LEXEME IS NOT A KEYWORD, GO TO STEP 2.

2. SET a AVID TOKEN.
   IF THE lexeme IS LONGER than VID_LEN (see token.h) CHARACTERS,
   ONLY FIRST VID_LEN CHARACTERS ARE STORED 
   INTO THE VARIABLE ATTRIBUTE ARRAY vid_lex[](see token.h) .
   ADD \0 AT THE END TO MAKE A C-type STRING.*/
	return t;
}

ACCEPTING FUNCTION FOR THE string variable identifier (VID - SVID)
REPLACE XX WITH THE CORRESPONDING ACCEPTING STATE NUMBER

Token aa_funcXX(char lexeme[]){

WHEN CALLED THE FUNCTION MUST
1. SET a SVID TOKEN.
   IF THE lexeme IS LONGER than VID_LEN characters,
   ONLY FIRST VID_LEN-1 CHARACTERS ARE STORED
   INTO THE VARIABLE ATTRIBUTE ARRAY vid_lex[],
   AND THEN THE % CHARACTER IS APPENDED TO THE NAME.
   ADD \0 AT THE END TO MAKE A C-type STRING.
  
  return t;
}

ACCEPTING FUNCTION FOR THE floating-point literal (FPL)

Token aa_funcXX(char lexeme[]){

THE FUNCTION MUST CONVERT THE LEXEME TO A FLOATING POINT VALUE,
WHICH IS THE ATTRIBUTE FOR THE TOKEN.
THE VALUE MUST BE IN THE SAME RANGE AS the value of 4-byte float in C.
IN CASE OF ERROR (OUT OF RANGE) THE FUNCTION MUST RETURN ERROR TOKEN
THE ERROR TOKEN ATTRIBUTE IS  lexeme. IF THE ERROR lexeme IS LONGER
than ERR_LEN caharacters, only the first ERR_LEN character are
stored in err_lex.
  return t;
}

ACCEPTING FUNCTION FOR THE integer literal(IL) - decimal constant (DIL) AND ZERO (0)

Token aa_funcXX(char lexeme[]){

THE FUNCTION MUST CONVERT THE LEXEME REPRESENTING A DECIMAL CONSTANT AND 0
TO A DECIMAL INTEGER VALUE, WHICH IS THE ATTRIBUTE FOR THE TOKEN.
THE VALUE MUST BE IN THE SAME RANGE AS the value of 2-byte integer in C.
IN CASE OF ERROR (OUT OF RANGE) THE FUNCTION MUST RETURN ERROR TOKEN
THE ERROR TOKEN ATTRIBUTE IS  lexeme. IF THE ERROR lexeme IS LONGER
than ERR_LEN caharacters, only the first ERR_LEN character are
stored in err_lex.
  return t;
}

ACCEPTING FUNCTION FOR THE integer literal(IL) - octal constant (OIL)

Token aa_funcXX(char lexeme[]){

THE FUNCTION MUST CONVERT THE LEXEME REPRESENTING AN OCTAL CONSTANT
TO A DECIMAL INTEGER VALUE WHICH IS THE ATTRIBUTE FOR THE TOKEN.
THE VALUE MUST BE IN THE SAME RANGE AS the value of 2-byte integer in C.
THIS FUNCTION IS SIMILAR TO THE FUNCTION ABOVE AND THEY CAN BE
COMBINED INTO ONE FUNCTION
THE MAIN DIFFERENCE IE THAT THIS FUNCTION CALLS
THE FUNCTION atool(char * lexeme) WHICH CONVERTS AN ASCII STRING
REPRESENTING AN OCTAL NUMBER TO INTEGER VALUE
IN CASE OF ERROR (OUT OF RANGE) THE FUNCTION MUST RETURN ERROR TOKEN
THE ERROR TOKEN ATTRIBUTE IS  lexeme. IF THE ERROR lexeme IS LONGER
than ERR_LEN caharacters, only the first ERR_LEN character are
stored in err_lex.

  return t;
}

ACCEPTING FUNCTION FOR THE ERROR TOKEN 

Token aa_funcXX(char lexeme[]){

THE FUNCTION SETS THE ERROR TOKEN. lexeme[] CONTAINS THE ERROR
THE ATTRIBUTE OF THE ERROR TOKEN IS THE lexeme ITSELF
AND IT MUST BE STORED in err_lex.  IF THE ERROR lexeme IS LONGER
than ERR_LEN caharacters, only the first ERR_LEN character are
stored in err_lex.

  return t;
}


CONVERSION FUNCTION

long atool(char * lexeme){

THE FUNCTION CONVERTS AN ASCII STRING
REPRESENTING AN OCTAL INTEGER CONSTANT TO INTEGER VALUE
}

int iskeyword(char * kw_lexeme){
	int i;
	for (i = 0; i < KWT_SIZE; i++){
		if (strcmp(lexeme, kw_table[i]) == 0) {
			return i;
		}
	}
	return -1;
}

int discard_line(Buffer * sc_buf){
	unsigned char c;
	while (b_getcoffset(sc_buf) < b_size(sc_buf)){
		c = b_getc(sc_buf);
		if (c == '\n'){
			++line;
			break;
		}
		if (c == '\0'){
			b_retract(sc_buf);
			break;
		}
	}
	return 0;
}