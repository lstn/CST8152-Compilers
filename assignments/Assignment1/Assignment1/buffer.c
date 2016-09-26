#include "buffer.h"



Buffer * b_create(short init_capacity, char inc_factor, char o_mode){
	Buffer *pBuf;
	char *cb;

	if (init_capacity < sizeof(char)) return NULL; /* cannot have buffer with no or negative capacity */

	switch (o_mode){ /* checks for a valid o_mode: f, a or m and assigns proper values to mode and inc_factor */
		case 'f':
			o_mode = 0;
			inc_factor = 0; /* always 0 in f mode */
			break;
		case 'a':
			if (inc_factor > 0 && inc_factor < 256){ /* inc factor must be between 1 and 255 inclusive */
				o_mode = 1;
			}
			else {
				return NULL; 
			}
			break;
		case 'm':
			if (inc_factor > 0 && inc_factor < 101){ /* inc factor must be between 1 and 100 inclusive */
				o_mode = -1;
			}
			else {
				return NULL;
			}
			break;
		default:
			return NULL;
	}
	pBuf = (Buffer*)calloc(1, sizeof(Buffer));
	cb = (char*)malloc(init_capacity);

	pBuf->cb_head = cb;
	pBuf->capacity = init_capacity;
	pBuf->inc_factor = inc_factor;
	pBuf->addc_offset = 0;

	return pBuf;

}

pBuffer b_addc(pBuffer const pBD, char symbol){
	char **temp_loc;
	short avail_space, new_capacity;
	char new_inc;

	pBD->r_flag = 0;
	if (!b_isfull(pBD)){
		pBD->cb_head[pBD->addc_offset] = symbol;
		++pBD->addc_offset;
		return pBD;
	}

	switch (pBD->mode){
		case -1:
			avail_space = SHRT_MAX - pBD->capacity;
			new_inc = (char) avail_space * (pBD->inc_factor * sizeof(char) / 100);
			new_capacity = pBD->capacity + new_inc;
			break;
		case 0:
			return NULL;
		case 1:
			new_capacity = pBD->capacity + pBD->inc_factor * sizeof(char);
			break;
		default:
			return NULL;
	}
	temp_loc = &pBD->cb_head;
	pBD->cb_head = realloc(pBD->cb_head, new_capacity);
	if (!pBD->cb_head) return NULL;
	if (temp_loc != &pBD->cb_head) pBD->r_flag = SET_R_FLAG;

	temp_loc = NULL;

	pBD->cb_head[pBD->addc_offset] = symbol;
	++pBD->addc_offset;
	pBD->capacity = new_capacity;
	return pBD;
}

int b_reset(Buffer * const pBD){
	if (!pBD) return R_FAIL1;

	pBD->eob = 0;
	pBD->addc_offset = 0;
	pBD->getc_offset = 0;
	pBD->mark_offset = 0;

	return 0;
}

void b_free(Buffer * const pBD){
	if (pBD->cb_head){
		free(pBD->cb_head);
		pBD->cb_head = NULL;
	}
	free(pBD);
}

int b_isfull(Buffer * const pBD){
	if (!pBD->addc_offset && pBD->addc_offset != 0) return R_FAIL1; /* check that offset exists */
	if (!pBD->capacity) return R_FAIL1; /* capacity must exist */

	if ((short) (pBD->addc_offset*sizeof(char) + sizeof(char)) >= pBD->capacity) return 1; /* buffer is full */
	return 0; /* buffer is not full*/
}

short b_size(Buffer * const pBD){
	if (!pBD->addc_offset && pBD->addc_offset != 0) return R_FAIL1; /* check that offset exists */
	
	return pBD->addc_offset+1;
}

short b_capacity(Buffer * const pBD){
	if (!pBD->capacity) return R_FAIL1;
	return pBD->capacity;
}

short b_setmark(Buffer * const pBD, short mark){
	if (!pBD) return R_FAIL1;
	if (!pBD->addc_offset && pBD->addc_offset != 0) return R_FAIL1; /* check that offset exists */
	if (mark < 0 || mark > pBD->addc_offset) return R_FAIL1; /* 0 to addc_offset inclusive */

	pBD->mark_offset = mark;
	return pBD->mark_offset;
}

int b_mode(Buffer * const pBD){
	if (!pBD->mode && pBD->mode != 0) return R_FAIL1; /* check that offset exists */
	return pBD->mode;
}

size_t  b_incfactor(Buffer * const pBD){
	if (!pBD || !pBD->inc_factor || pBD->inc_factor < 0) return 256;
	return pBD->inc_factor;
}

int b_load(FILE * const fi, Buffer * const pBD){

}

int b_isempty(Buffer * const pBD){
	if (!pBD->addc_offset && pBD->addc_offset != 0) return R_FAIL1; /* check that offset exists */

	if (pBD->addc_offset == 0) return 1; /* buffer empty */
	return 0; /* buffer not element */
}

int b_eob(Buffer * const pBD){
	if (!pBD->eob && pBD->eob != 0) return R_FAIL1; /* checks eob exists */

	return pBD->eob;
}

char b_getc(Buffer * const pBD){
	if (!pBD->addc_offset && pBD->addc_offset != 0) return R_FAIL2; /* check that offset exists */
	if (!pBD->getc_offset && pBD->getc_offset != 0) return R_FAIL2; /* check that offset exists */

	if (pBD->getc_offset == pBD->addc_offset){
		pBD->eob = 1;
		return R_FAIL1;
	} else {
		pBD->eob = 0;
		return (char) ++pBD->getc_offset;
	}
}

int b_print(Buffer  * const pBD){
	short temp_getc_offset;

	if (!pBD->getc_offset && pBD->getc_offset != 0) return R_FAIL1; /* check that offset exists */
	if (!pBD->cb_head) return -1; /* checks character buffer is initialized */

	if (b_isempty(pBD)){
		printf("The buffer is empty.\n");
	}

	temp_getc_offset = pBD->getc_offset;
	pBD->getc_offset = 0;

	while (!b_eob(pBD)){ /* loop until we reach the end of the buffer */
		printf("%c", pBD->cb_head[b_getc(pBD)]);
	}

	pBD->getc_offset = temp_getc_offset;
	printf("\n");

	return pBD->getc_offset + 1;
}

Buffer *b_pack(Buffer * const pBD){
	char **temp_loc;

	temp_loc = &pBD->cb_head;
	pBD->cb_head = realloc(pBD->cb_head, (pBD->getc_offset+1)*sizeof(char));
	if (!pBD->cb_head) return NULL;
	if (temp_loc != &pBD->cb_head){
		pBD->r_flag = SET_R_FLAG; 
	} else { pBD->r_flag = 0; }

	temp_loc = NULL;

	return pBD;
}

char b_rflag(Buffer * const pBD){
	if (!pBD->r_flag && pBD->r_flag != 0) return R_FAIL1; /* check that offset exists */

	return pBD->r_flag;
}

short b_retract(Buffer * const pBD){
	if (!pBD->getc_offset && pBD->getc_offset != 0) return R_FAIL1; /* check that offset exists */
	if (pBD->getc_offset - 1 < 0) return R_FAIL1; /* can't go negative */

	return --pBD->getc_offset;
}

short b_retract_to_mark(Buffer * const pBD){
	if (!pBD->getc_offset && pBD->getc_offset != 0) return R_FAIL1; /* check that offset exists */
	if (!pBD->mark_offset || pBD->mark_offset < 0) return R_FAIL1; /* can't go negative */

	pBD->getc_offset = pBD->mark_offset;
	return pBD->getc_offset;
}

short b_getcoffset(Buffer * const pBD){
	if (!pBD->getc_offset && pBD->getc_offset != 0) return R_FAIL1; /* check that offset exists */

	return pBD->getc_offset;
}

char * b_cbhead(Buffer * const pBD){
	if (pBD->cb_head) return pBD->cb_head;
	return NULL;
}
