// tlkfile.c

// routines to manage the internal representation of the tlkfile.
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "tlkcomp.h" 

void initialize_header(tlkfile_t * tlk) {
	tlk->header._tlk[0] = 'T';
	tlk->header._tlk[1] = 'L';
	tlk->header._tlk[2] = 'K';
	tlk->header._tlk[3] = ' ';

	tlk->header._ver[0] = 'V';
	tlk->header._ver[1] = '3';
	tlk->header._ver[2] = '.';
	tlk->header._ver[3] = '0';
}

tlkfile_t * tlkfile_new_tlkfile(char * name) {
	tlkfile_t* ret = malloc(sizeof(tlkfile_t));
	if (ret == NULL) 
		return NULL;

	memset(ret, 0,sizeof(tlkfile_t));
	initialize_header(ret);

	ret->filename = malloc(strlen(name) + 1);
	strcpy(ret->filename, name);

	return ret;
}

void tlkfile_free(tlkfile_t * tlk) {

	if (tlk == NULL)
		return;

	tlk_element_t *cur = tlk->start;
	tlk_element_t * tmp;
	while (cur != NULL) {
		if (cur->_data)
			free(cur->_data);
		
		tmp = cur;
		cur = cur->next;
		free(tmp);
	}
	if (tlk->filename)
		free(tlk->filename);
	free(tlk);
}

// Create a new element given the descriptor, string data and line. The string_data should be a 
// malloced null terminated string. It should not be a pointer to the un terminated raw data. 
tlk_element_t * tlkfile_new_element(tlk_string_data_element_t *desc, char * string_data, unsigned int line) {
	tlk_element_t * ret =  malloc(sizeof(tlk_element_t));
	if (ret == NULL) 
		return NULL;

	//memset(ret, 0,sizeof(tlk_element_t));
	memcpy(ret, desc, sizeof(tlk_string_data_element_t));
	ret->prev = NULL;
	ret->next = NULL;

	ret->_line = line;
	ret->_data = string_data;
	       
	return ret;
}

int tlkfile_list_insert(tlkfile_t * tlk, tlk_element_t * elem) {
	if (tlk->start == NULL) {
		tlk->start = elem;
		tlk->end = elem;
		return 0;
	} 
	
	// check for insert at end which will be the common case
	if (elem->_line > tlk->end->_line) {
		elem->prev = tlk->end;
		tlk->end->next = elem;
		tlk->end = elem;
		return 0;
	}
	tlk_element_t *cur = tlk->start;
	while (cur != NULL && cur->_line < elem->_line) {
		cur = cur->next;
	}
	// This should not happen 
	if (cur == NULL) {
		elem->prev = tlk->end;
		tlk->end->next = elem;
		tlk->end = elem;
		return 0;
	} 

	if (cur->_line == elem->_line) {
		char msg[128];
		sprintf(msg, "Error - duplicate tlk entries %d \n", elem->_line);
		tlk_set_errstr(msg);
		return -1;
	}
	// insert before cur
	tlk_element_t *prev = cur->prev;
	elem->next = cur;
	if (cur == tlk->start) 
		tlk->start = elem;
	else if (prev != NULL)
		prev->next = elem;
	else {
		return -1;
	}
	elem->prev = prev;
	cur->prev = elem;

	return 0;
}


int tlkfile_add_element(tlkfile_t * tlk, tlk_element_t * elem) {
	int ret = tlkfile_list_insert(tlk, elem);
	
	if (ret >= 0) { 
		tlk->data_len += elem->e._string_size;
		tlk->count ++;
	}
	return ret;
}
