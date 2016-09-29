/* File name:	buffer.c
*  Compiler:	MS Visual Studio 2013
*  Author:		Lucas Estienne, 040 819 959
*  Course:		CST 8152 - Compilers, Lab Section 012
*  Assignment:	01
*  Date:		29 September 2016
*  Professor:   Svillen Ranev
*  Purpose:		This implements the functions required for the Buffer component of the Compiler.
*  Function List: b_create(), b_addc(), b_reset(), b_free(), b_isfull(), b_size(), b_capacity(),
*				  b_setmark(), b_mark(), b_mode(), b_incfactor(), b_load(), b_isempty(), b_eob(),
*				  b_getc(), b_print(), b_pack(), b_rflag(), b_retract(), b_retract_to_mark(),
*				  b_getcoffset(), b_cbhead()
*/
#include "buffer.h"


/* Purpose:			 
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions: 
*  Parameters:		 
*  Return value:	 
*  Algorithm:		 
*/
Buffer * b_create(short init_capacity, char inc_factor, char o_mode){
	Buffer *pBuf;
	char *cb;

	if (init_capacity < sizeof(char)) return NULL; /* cannot have buffer with no or negative capacity */

	pBuf = (Buffer*)calloc(1, sizeof(Buffer));
	switch (o_mode){ /* checks for a valid o_mode: f, a or m and assigns proper values to mode and inc_factor */
		case 'f':
			pBuf->mode = 0;
			inc_factor = 0; /* always 0 in f mode */
			break;
		case 'a':
			if (inc_factor > 0 && inc_factor < 256){ /* inc factor must be between 1 and 255 inclusive */
				pBuf->mode = 1;
			}
			else {
				return NULL; 
			}
			break;
		case 'm':
			if (inc_factor > 0 && inc_factor < 101){ /* inc factor must be between 1 and 100 inclusive */
				pBuf->mode = -1;
			}
			else {
				return NULL;
			}
			break;
		default:
			return NULL;
	}

	cb = (char*)malloc(init_capacity);

	pBuf->cb_head = cb;
	pBuf->capacity = init_capacity;
	pBuf->inc_factor = inc_factor;
	pBuf->addc_offset = 0;

	return pBuf;

}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
pBuffer b_addc(pBuffer const pBD, char symbol){
	char **temp_loc;
	short avail_space, new_capacity;
	unsigned short new_inc;
	pBD->r_flag = 0;
	if (!b_isfull(pBD)){
		b_cbhead(pBD)[pBD->addc_offset] = symbol;
		++pBD->addc_offset;
		return pBD;
	}

	switch (pBD->mode){
		case -1:
			avail_space = SHRT_MAX - pBD->capacity;
			new_inc = (unsigned short) (avail_space * (pBD->inc_factor / 100.0));
			new_capacity = pBD->capacity + new_inc;
			break;
		case 0:
			return NULL;
		case 1:
			new_capacity = pBD->capacity + (pBD->inc_factor * sizeof(char));
			if (new_capacity < 0) return NULL;
			break;
		default:
			return NULL;
	}
	temp_loc = &pBD->cb_head;
	pBD->cb_head = realloc(pBD->cb_head, new_capacity);
	if (!b_cbhead(pBD)) return NULL;
	if (temp_loc != &pBD->cb_head) pBD->r_flag = SET_R_FLAG;

	temp_loc = NULL;

	b_cbhead(pBD)[pBD->addc_offset] = symbol;
	++pBD->addc_offset;
	pBD->capacity = new_capacity;
	return pBD;
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
int b_reset(Buffer * const pBD){
	if (!pBD) return R_FAIL1;

	pBD->eob = 0;
	pBD->addc_offset = 0;
	pBD->getc_offset = 0;
	pBD->mark_offset = 0;

	return 0;
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
void b_free(Buffer * const pBD){
	if (!pBD) return;
	if (b_cbhead(pBD)){
		free(pBD->cb_head);
		pBD->cb_head = NULL;
	}
	free(pBD);
}

#  ifndef B_FULL /* checks if the user chose to undef the B_FULL macro in order to use the function instead */
/* Purpose:			 
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions: 
*  Parameters:		 
*  Return value:	 
*  Algorithm:
*/
int b_isfull(Buffer * const pBD){
	if (!pBD->addc_offset && pBD->addc_offset != 0) return R_FAIL1; /* check that offset exists */
	if (!pBD->capacity) return R_FAIL1; /* capacity must exist */

	return ((short)(pBD->addc_offset*sizeof(char) + sizeof(char)) > pBD->capacity) ? 1 : 0;
}
#  endif

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
short b_size(Buffer * const pBD){
	if (!pBD->addc_offset && pBD->addc_offset != 0) return R_FAIL1; /* check that offset exists */
	
	return pBD->addc_offset;
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
short b_capacity(Buffer * const pBD){
	if (!pBD->capacity) return R_FAIL1;
	return pBD->capacity;
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
short b_setmark(Buffer * const pBD, short mark){
	if (!pBD) return R_FAIL1;
	if (!pBD->addc_offset && pBD->addc_offset != 0) return R_FAIL1; /* check that offset exists */
	if (mark < 0 || mark > pBD->addc_offset) return R_FAIL1; /* 0 to addc_offset inclusive */

	pBD->mark_offset = mark;
	return pBD->mark_offset;
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
short b_mark(Buffer * const pBD){
	if (!pBD->mode && pBD->mode != 0) return R_FAIL1; /* check that offset exists */
	return pBD->mark_offset;
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
int b_mode(Buffer * const pBD){
	if (!pBD->mode && pBD->mode != 0) return R_FAIL1; /* check that offset exists */
	return pBD->mode;
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
size_t b_incfactor(Buffer * const pBD){
	if (!pBD || (!pBD->inc_factor && pBD->inc_factor != 0 ) || pBD->inc_factor < 0) return 256;
	return pBD->inc_factor;
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
int b_load(FILE * const fi, Buffer * const pBD){
	char to_add;
	short added_num = 0;
	if (!fi || !pBD) return R_FAIL1;

	for (;;added_num++){
		to_add = (char)fgetc(fi);
		if (feof(fi))
			break;
		if (b_addc(pBD, to_add) == NULL) return LOAD_FAIL;
	}

	return added_num;
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
int b_isempty(Buffer * const pBD){
	if (!pBD->addc_offset && pBD->addc_offset != 0) return R_FAIL1; /* check that offset exists */

	if (pBD->addc_offset == 0) return 1; /* buffer empty */
	return 0; /* buffer not element */
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
int b_eob(Buffer * const pBD){
	if (!pBD->eob && pBD->eob != 0) return R_FAIL1; /* checks eob exists */

	return pBD->eob;
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
char b_getc(Buffer * const pBD){
	if (!pBD->addc_offset && pBD->addc_offset != 0) return R_FAIL2; /* check that offset exists */
	if (!pBD->getc_offset && pBD->getc_offset != 0) return R_FAIL2; /* check that offset exists */

	if (pBD->getc_offset == pBD->addc_offset){
		pBD->eob = 1;
		return R_FAIL1;
	} else {
		pBD->eob = 0;
		++pBD->getc_offset;
		return b_cbhead(pBD)[pBD->getc_offset - 1];
	}
}

/* Purpose:
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions:
*  Parameters:
*  Return value:
*  Algorithm:
*/
int b_print(Buffer  * const pBD){
	short temp_getc_offset;
	int eob;
	char symbol;

	if (!pBD->getc_offset && pBD->getc_offset != 0) return R_FAIL1; /* check that offset exists */
	if (!b_cbhead(pBD)) return -1; /* checks character buffer is initialized */

	if (b_isempty(pBD)){
		printf("The buffer is empty.\n");
	}
	temp_getc_offset = pBD->getc_offset;
	pBD->getc_offset = 0;

	do { /* loop until we reach the end of the buffer */
		symbol = b_getc(pBD);
		eob = b_eob(pBD);
		if (!eob) printf("%c", symbol);
	} while (!eob);

	pBD->getc_offset = temp_getc_offset;

	printf("\n");

	return pBD->getc_offset;
}

/* Purpose:			 Packs the passed Buffer by shrinking (or in some cases, expanding) the 
*					 buffer. The new capacity is the current size + 1.  Returns NULL if a
*                    runtime error occurs.
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions: realloc(), b_cbhead()
*  Parameters:		 [pBD: (Buffer * const)]
*  Return value:	 [ (Buffer *), (NULL) ]
*  Algorithm:		 
*					 +ASSIGN address of Buffer character buffer to temp_loc
*					 +REALLOC memory for Buffer character buffer to current size+1
*                    +IF failed reallocation
*                     >RETURN NULL
*                    +ELIF temp_loc does not match address of Buffer character buffer after reallocation
*					  >ASSIGN SET_R_FLAG to Buffer r_flag to indicate that memory location has changed
*					 +ELSE
*					  >ASSIGN 0 to Buffer r_flag to indicate that memory location has not changed
*					 +RETURN pointer to Buffer
*/
Buffer *b_pack(Buffer * const pBD){
	char **temp_loc;

	if (!pBD || !pBD->cb_head) return NULL;

	pBD->r_flag = 0;
	temp_loc = &pBD->cb_head;

	pBD->cb_head = realloc(pBD->cb_head, (pBD->addc_offset+1)*sizeof(char));
	pBD->capacity = pBD->addc_offset + 1;

	if (!b_cbhead(pBD)) { 
		temp_loc = NULL; /* dangling pointer */
		return NULL; 
	}
	if (temp_loc != &pBD->cb_head){
		pBD->r_flag = SET_R_FLAG; 
	} else { pBD->r_flag = 0; }

	temp_loc = NULL;
	return pBD;
}

/* Purpose:			 Returns the r_flag member of the passed Buffer. Returns R_FAIL1
*					 if a runtime error occurs.
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions: none
*  Parameters:		 [pBD: (Buffer * const)]
*  Return value:	 [ (char, values: {0, 1}), (R_FAIL1) ]
*  Algorithm:
*/
char b_rflag(Buffer * const pBD){
	if (!pBD || (!pBD->r_flag && pBD->r_flag != 0)) return R_FAIL1; /* check that offset exists */

	return pBD->r_flag;
}

/* Purpose:			 Retracts the getc_offset member of the passed Buffer by 1 position.
*					 Returns R_FAIL1 if a runtime error occurs.
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions: none
*  Parameters:		 [pBD: (Buffer * const)]
*  Return value:	 [ (short), (R_FAIL1) ]
*  Algorithm:
*/
short b_retract(Buffer * const pBD){
	if (!pBD || (!pBD->getc_offset && pBD->getc_offset != 0)) return R_FAIL1; /* check that offset exists */
	if (pBD->getc_offset - 1 < 0) return R_FAIL1; /* can't go negative */

	return --pBD->getc_offset;
}

/* Purpose:			 Retracts the getc_offset member of the passed Buffer to the position of
*                    the mark_offset member of the passed Buffer. Returns R_FAIL1 if a runtime
*					 error occurs.
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions: none
*  Parameters:		 [pBD: (Buffer * const)]
*  Return value:	 [ (short), (R_FAIL1) ]
*  Algorithm:
*/
short b_retract_to_mark(Buffer * const pBD){
	if (!pBD || (!pBD->getc_offset && pBD->getc_offset != 0)) return R_FAIL1; /* check that offset exists */
	if (!pBD->mark_offset || pBD->mark_offset < 0) return R_FAIL1; /* can't go negative */

	pBD->getc_offset = pBD->mark_offset;
	return pBD->getc_offset;
}

/* Purpose:			 Returns the getc_offset member of the passed Buffer. Returns R_FAIL1
*					 if a runtime error occurs.
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions: none
*  Parameters:		 [pBD: (Buffer * const)]
*  Return value:	 [ (short), (R_FAIL1) ]
*  Algorithm:
*/
short b_getcoffset(Buffer * const pBD){
	if (!pBD || (!pBD->getc_offset && pBD->getc_offset != 0)) return R_FAIL1; /* check that offset exists */

	return pBD->getc_offset;
}

/* Purpose:			 Returns the pointer to the beginning of the character buffer array in the passed
*					 Buffer. Returns NULL if a runtime error occurs.
*  Author:			 Lucas Estienne
*  History/Versions: [1.0 - 9/29/2016]
*  Called functions: none
*  Parameters:       [pBD: (Buffer * const)]
*  Return value:	 [ (char *), (NULL) ]
*  Algorithm:
*/
char * b_cbhead(Buffer * const pBD){
	if (pBD && pBD->cb_head) return pBD->cb_head;
	return NULL; /* returns NULL if pBD or cb_head are not defined */
}
