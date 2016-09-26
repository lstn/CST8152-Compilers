/* REPLACE the header below with your file header (see CST8152_ASSAMG.pdf for details).
 * File Name: buffer.h
 * Version: 1.16.2
 * Author: S^R
 * Date: 6 September 2016
 * Preprocessor directives, type declarations and prototypes necessary for buffer implementation 
 * as required for CST8152, Assignment #1, Fall 2016.
 * The file is not completed.
 * You must add your function declarations (prototypes).
 * You must also add your constant definitions and macros,if any.
 */
#ifndef BUFFER_H_
#define BUFFER_H_

/*#pragma warning(1:4001) *//*to enforce C89 type comments  - to make //comments an warning */

/*#pragma warning(error:4001)*//* to enforce C89 comments - to make // comments an error */

/* standard header files */
#include <stdio.h>  /* standard input/output */
#include <malloc.h> /* for dynamic memory allocation*/
#include <limits.h> /* implementation-defined data type ranges and limits */

/* constant definitions */
/* You may add your own constant definitions here */
#define R_FAIL1 -1         /* fail return value */
#define R_FAIL2 -2         /* fail return value */
#define LOAD_FAIL -2       /* load fail error */
#define SET_R_FLAG 1       /* realloc flag set value */


/* user data type declarations */
typedef struct BufferDescriptor {
    char *cb_head;   /* pointer to the beginning of character array (character buffer) */
    short capacity;    /* current dynamic memory size (in bytes) allocated to character buffer */
    short addc_offset;  /* the offset (in chars) to the add-character location */
    short getc_offset;  /* the offset (in chars) to the get-character location */
    short mark_offset; /* the offset (in chars) to the mark location */
    char  inc_factor; /* character array increment factor */
    char  r_flag;     /* reallocation flag */
    char  mode;       /* operational mode indicator*/
    int   eob;       /* end-of-buffer flag */
} Buffer, *pBuffer;
/*typedef Buffer *pBuffer;*/

/* function declarations */
Buffer * b_create(short init_capacity, char inc_factor, char o_mode);
pBuffer b_addc(pBuffer const pBD, char symbol);
int b_reset(Buffer * const pBD);
void b_free(Buffer * const pBD);
int b_isfull(Buffer * const pBD);
short b_size(Buffer * const pBD);
short b_capacity(Buffer * const pBD);
short b_setmark(Buffer * const pBD, short mark);
int b_mode(Buffer * const pBD);
size_t  b_incfactor(Buffer * const pBD);
int b_load(FILE * const fi, Buffer * const pBD);
int b_isempty(Buffer * const pBD);
int b_eob(Buffer * const pBD);
char b_getc(Buffer * const pBD);
int b_print(Buffer  * const pBD);
Buffer *b_pack(Buffer * const pBD);
char b_rflag(Buffer * const pBD);
short b_retract(Buffer * const pBD);
short b_retract_to_mark(Buffer * const pBD);
short b_getcoffset(Buffer * const pBD);
char * b_cbhead(Buffer * const pBD);
/*
Place your function declarations here.
Do not include the function header comments here.
Place them in the buffer.c file
*/

#endif

