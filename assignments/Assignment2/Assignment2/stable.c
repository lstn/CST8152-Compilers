/* File name:	stable.c
*  Compiler:	MS Visual Studio 2013
*  Author:		Lucas Estienne, 040 819 959
*  Course:		CST 8152 - Compilers, Lab Section 012
*  Assignment:	03
*  Date:		1 December 2016
*  Professor:   Svillen Ranev
*  Purpose:		Implements the symbol table manager
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
#include "stable.h"
#include "buffer.h"

#define DEBUG  /* for conditional processing */
#undef  DEBUG

/* "internal" function prototypes */
static void st_setsize(void);
static void st_incoffset(void);

/* get global sym_table from main program */
extern STD sym_table;

/* symbol table manager functions implementations*/

STD st_create(int st_size){
	STD new_st;

	new_st.st_size = 0; /* value in case passed size is invalid */
	if (st_size < 1) return new_st; /* check passed size is valid */

	new_st.pstvr = malloc(st_size*sizeof(STVR));
	if (new_st.pstvr == NULL) return new_st;

	new_st.plsBD = b_create(ST_BUF_INIT_CAP, ST_BUF_INIT_INC, ST_BUF_INIT_MODE);
	if (new_st.plsBD == NULL) { 
		/* have to free the pstvr we allocated, or leak */
		free(new_st.pstvr);
		return new_st;
	}

	new_st.st_offset = ST_INIT_OFFSET;


	new_st.st_size = st_size; /* nothing failed, can set st_size to indicate st is valid */
	return new_st;
}
int st_install(STD sym_table, char *lexeme, char type, int line){
	int temp_offset, i, lexlen;

	if (!sym_table.st_size || sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return RFAIL_INVALID_ST;

	lexlen = strlen(lexeme);

	temp_offset = st_lookup(sym_table, lexeme);
	if (temp_offset != SYMBOL_NOT_FOUND) return temp_offset; /* symbol already exists*/
	if (sym_table.st_offset >= sym_table.st_size) return RFAIL_ST_FULL; /* table is full */

	sym_table.pstvr[sym_table.st_offset].plex = b_cbhead(sym_table.plsBD) + b_size(sym_table.plsBD);

	b_setmark(sym_table.plsBD, b_size(sym_table.plsBD)); /* recovery mark if we fail */

	for (i = 0; i < lexlen - 1; i++){
		b_addc(sym_table.plsBD, lexeme[i]);
		/*if (b_rflag(sym_table.plsBD)){}*/ /* buffer has changed memory location, so fail*/
	}

	if ((b_mark(sym_table.plsBD) + lexlen) != (b_size(sym_table.plsBD) - 1)){ /* failed to add */
		sym_table.pstvr[sym_table.st_offset].plex = NULL; /*dangling pointer*/
		b_retract_to_mark(sym_table.plsBD); 
		return R_FAIL2;
	}

	sym_table.pstvr[sym_table.st_offset].o_line = line;
	sym_table.pstvr[sym_table.st_offset].status_field = sym_table.pstvr[sym_table.st_offset].status_field & SF_INIT | SF_DEFAULT;

	switch (type){
		case 'I':
			sym_table.pstvr[sym_table.st_offset].status_field |= SF_INT_TYPE;
			sym_table.pstvr[sym_table.st_offset].i_value.int_val = 0;
			break;
		case 'F':
			sym_table.pstvr[sym_table.st_offset].status_field |= SF_FLOAT_TYPE;
			sym_table.pstvr[sym_table.st_offset].i_value.fpl_val = 0.0;
			break;
		case 'S':
			sym_table.pstvr[sym_table.st_offset].status_field |= SF_STRING_TYPE | SF_UPDATE;
			sym_table.pstvr[sym_table.st_offset].i_value.str_offset = -1;
			break;
	}

	st_incoffset();

	return sym_table.st_offset;
}

int st_lookup(STD sym_table, char *lexeme){
	int current_offset;

	if (!sym_table.st_size || sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return RFAIL_INVALID_ST;

	/* looking up backwards, so start at last entry */
	current_offset = sym_table.st_offset - 1;
	while (current_offset > -1){
		if (!strcmp(sym_table.pstvr[current_offset++].plex, lexeme)) /* todo make sure this works*/
			return current_offset;
	}
	return SYMBOL_NOT_FOUND; /* not found */
}

int st_update_type(STD sym_table, int vid_offset, char v_type){
	if (!sym_table.st_size || sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return RFAIL_INVALID_ST;
	if (vid_offset < 0 || vid_offset > sym_table.st_offset) return R_FAIL2;

	if (sym_table.pstvr[vid_offset].status_field & SF_UPDATE)
		return -1;

	sym_table.pstvr[vid_offset].status_field &= SF_UPDATE;
	switch (v_type){
		case 'I':
			sym_table.pstvr[sym_table.st_offset].status_field |= SF_INT_TYPE;
			break;
		case 'F':
			sym_table.pstvr[sym_table.st_offset].status_field |= SF_FLOAT_TYPE;
			break;
		default:
			return R_FAIL2;
	}

	sym_table.pstvr[vid_offset].status_field |= SF_UPDATE;
	return vid_offset;
}
int st_update_value(STD sym_table, int vid_offset, InitialValue i_value){
	if (!sym_table.st_size || sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return RFAIL_INVALID_ST;
	if (vid_offset < 0 || vid_offset > sym_table.st_offset) return R_FAIL2;

	sym_table.pstvr[vid_offset].i_value = i_value;
	return vid_offset;

}
char st_get_type(STD sym_table, int vid_offset){
	if (!sym_table.st_size || sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return RFAIL_INVALID_ST;
}
void st_destroy(STD sym_table){
	if (!sym_table.st_size || sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return RFAIL_INVALID_ST;
}
int st_print(STD sym_table){
	if (!sym_table.st_size || sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return RFAIL_INVALID_ST;
}
int st_store(STD sym_table){
	if (!sym_table.st_size || sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return RFAIL_INVALID_ST;
}
int st_sort(STD sym_table, char s_order){
	if (!sym_table.st_size || sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return RFAIL_INVALID_ST;

	return 0;
}

/* internal functions */
static void st_setsize(void){
	if (sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return;
	sym_table.st_size = 0;
}
static void st_incoffset(void){
	if (!sym_table.st_size || sym_table.pstvr == NULL || sym_table.plsBD == NULL) /* check st is valid */
		return;
	++sym_table.st_offset;
}