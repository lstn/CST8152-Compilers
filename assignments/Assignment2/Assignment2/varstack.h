/* File name:	stable.h
*  Compiler:	MS Visual Studio 2013
*  Author:		Lucas Estienne, 040 819 959
*  Course:		CST 8152 - Compilers, Lab Section 012
*  Assignment:	03
*  Date:		1 December 2016
*  Professor:   Svillen Ranev
*  Purpose:		Symbol Table DB implementation declarations and ST manager prototypes
*  Function List: st_create(), st_install(), st_lookup(), st_update_type(), st_update_value(),
*			     st_get_type(), st_destroy(), st_print(), st_store(), st_sort()
*  Constants: RFAIL_INVALID_ST, RFAIL_ST_FULL, ST_BUF_INIT_CAP, RFAIL_BUF_MEMCHANGED, ST_BUF_INIT_MODE,
*			  ST_BUF_INIT_INC, ST_INIT_OFFSET, ST_FILENAME, SYMBOL_NOT_FOUND, SF_INIT, SF_DEFAULT,
*             SF_INT_TYPE, SF_STRING_TYPE, SF_FLOAT_TYPE, SF_UPDATE, SF_TYPEMASK
*/

#ifndef  VARSTACK_H_
#define  VARSTACK_H_ 

#ifndef BUFFER_H_
#include "buffer.h"
#endif

#ifndef NULL
#include <_null.h> /* NULL pointer constant is defined there */
#endif

/* constants */
#define ST_DEF_SIZE 100

/* flags */


/* implementation */
typedef struct VarStack{
	int var_ids[ST_DEF_SIZE];
	int top;
} VarStack;

/* function prototypes */
VarStack * vs_create();
int vs_empty(VarStack * var_stack);
int vs_full(VarStack * var_stack);
int vs_peek(VarStack * var_stack);
int vs_pop(VarStack * var_stack);
int vs_push(VarStack * var_stack, int var_id);
int vs_size(VarStack * var_stack);
int vs_contains(VarStack * var_stack, int var_id);
int vs_at(VarStack * var_stack, int i);
int vs_remove(VarStack * var_stack, int i);

#endif
