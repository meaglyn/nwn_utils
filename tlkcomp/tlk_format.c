// tlk_format.c

/// handle on disk format for tlk files
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "tlkcomp.h"

int tlk_format_validate(char * filename) {
	FILE *file;
	char buffer[64];
	int cc;

	//Open file
	file = fopen(filename, "rb");
	if (!file) {
		return 1;
	}
	cc= fread(buffer, 1, 8, file);
	
	if (cc != 8) {
		fclose(file);
		return 1;
	}

	buffer[8] = 0;
	if (!strncmp(buffer, "TLK V3.0", 8)) {
		fclose(file);
		return 0;
	}

	fclose(file);
	return 1;
}

char * tlk_get_string_from_elem(tlk_string_data_element_t * elem, char * base) {
	int len  = elem->_string_size;
	char * ret;
	if (len <= 0) 
		return NULL;

	ret = malloc (len + 1);
	memcpy(ret, base + elem->_offset, len);
	ret[len] = 0;
	return ret;
}


tlkfile_t * tlk_format_parse_buffer(char * filename, char * buffer, int len) {
	tlkfile_t * ret;
	tlk_file_header_t * tlk_header;
	int i;
	unsigned int line = 0;
	char * str;
	char * string_data_base;
	tlk_string_data_element_t * entries;
	tlk_string_data_element_t * cur;
	tlk_element_t * elem;

	tlk_header = (tlk_file_header_t *) buffer;
	if (tlkdebug)
		tlk_print_header(tlk_header);

	ret = tlkfile_new_tlkfile(filename);
	ret->header._string_count = tlk_header->_string_count;
	ret->header._string_entry_offset = tlk_header->_string_entry_offset;
	ret->header._lang  = tlk_header->_lang;
	//if (tlkdebug)
	//	tlk_print_header(&ret->header);
	
	entries = (tlk_string_data_element_t*) (buffer + sizeof(tlk_file_header_t));
	string_data_base = buffer + tlk_header->_string_entry_offset;

// Process the strings 
	for (i = 0; i < tlk_header->_string_count; i ++) {
		cur = &entries[i];
		// They all seem to be marked present - that's a bug in tlk editor tool it looks like.
		str = NULL;
		if (cur->_flags & 1) {
			str =  tlk_get_string_from_elem(cur, string_data_base);
		}

		if(tlkdebug) {
			printf("Line %d: ", line);
			tlk_print_raw_entry(cur);
			if (str)
				printf("Data = \"%s\"", str);
			printf("\n");
		}
		if (str) {
			elem =  tlkfile_new_element(cur, str, line);
			tlkfile_add_element(ret, elem);
		}


		line ++;
	}
	return ret;
}

tlkfile_t * tlk_format_read(char * filename) {
	int len = 0;
	char * buffer;
	tlkfile_t * ret;

	if ( tlk_format_validate(filename)) {
		tlk_set_errstr("Not valid TLK file format.");
		return NULL;
	}

	buffer =  tlk_do_read_file(filename, &len);
	if (buffer == NULL)
		return NULL; // do_read set errstr

	ret = tlk_format_parse_buffer(filename, buffer, len);
	
	free(buffer);
	return ret;
}

void tlk_format_blank_entry(tlk_string_data_element_t * entry) {
	memset(entry, 0, sizeof(tlk_string_data_element_t));
//	entry->_flags = 0;
//	entry->_offset = 0;
//	entry->_string_size = 0;
}

int tlk_format_add_blank_entries(char * start, int count) {
	int i;
	unsigned int offset = 0;
	for (i = 0; i < count ; i ++ ) {
		tlk_format_blank_entry((tlk_string_data_element_t *)(start + offset));
		offset += sizeof(tlk_string_data_element_t);
	}
	return offset;
}

int tlk_format_fill_data(tlkfile_t * tlk, char * header_buf, char * data_buf, unsigned int data_len) {
	int cur_line = 0;
	int last_line = -1; 
	unsigned int hd_offset = 0;
	unsigned int data_offset = 0;
	int len;
	char * head_buf_cur = header_buf;
	tlk_element_t *cur = tlk->start;
	tlk_string_data_element_t *elem;
	int count = 0;

	if (tlkdebug)
		fprintf(stderr,"tlk_format_fill_data , tlk %p, header_buf %p, data_buf %p, len %d\n", 
			tlk, header_buf, data_buf, data_len);
	while (cur != NULL) {
		cur_line = cur->_line;
		//fprintf(stderr, "  entry %d, line %d, len = %d, last line %d, cur header off set 0x%08x, cur data offset = 0x%08x\n",
		//	count, cur_line,  (int) strlen(cur->_data), last_line, hd_offset, data_offset);
		//fprintf(stderr, "  data \"%s\n", cur->_data);
		if (cur_line > last_line + 1) {
			// Not the next entry fill in blanks
			hd_offset += tlk_format_add_blank_entries(head_buf_cur, cur_line - (last_line + 1));
			head_buf_cur = header_buf + hd_offset;
			//fprintf(stderr, " Added %d blank lines, header offset now 0x%08x\n",  cur_line - (last_line + 1), hd_offset);
		}
		// Copy in the descriptor data and do book keeping.
		//fprintf(stderr,"  entry %d copy element to 0x%08x, line %d\n", count, hd_offset, cur_line);
		// point to descriptor location in output buffer
		elem = (tlk_string_data_element_t*) head_buf_cur;
		memcpy(head_buf_cur, &cur->e, sizeof(tlk_string_data_element_t));
		hd_offset += sizeof(tlk_string_data_element_t);
		head_buf_cur = header_buf + hd_offset;
		last_line = cur_line;
		
		// Now update the descriptor and put the data in place.
		len = strlen(cur->_data);
		if (data_offset + len > data_len) {
			tlk_set_errstr("too much data for allocation - something is wrong.");
			return -1;	
		}
		elem->_string_size = len;
		elem->_offset = data_offset;
		elem->_flags |= 1;
		memcpy(data_buf + data_offset, cur->_data, len);
		data_offset += len;

		count ++;
		cur = cur->next;
		
	}
	return hd_offset;
}


int tlk_format_write(tlkfile_t * tlk, char * filename) {
	int header_size;
	int data_size = 0;
	char *buffer;
	char * data_buf;
	char * header_buf;
	int num_entries;
	int buffer_len;

	if (tlk->count <= 0) {
		tlk_set_errstr("No data to write.");
		return -1;
	}

	if (tlk->end == NULL) {
		tlk_set_errstr("Invalid or corrupted tlk data.");
		return -1;	
	}

	// allocate buffer for header and descriptors
	// calculate size needed from count
	num_entries = tlk->end->_line + 1;
	header_size = sizeof(tlk_file_header_t) + num_entries * sizeof(tlk_string_data_element_t);
	data_size = tlk->data_len;
	buffer_len = header_size + data_size;

	if (tlkdebug) 
		printf("Writing tlk file %s with %d entries, total len %d bytes\n", filename, num_entries, buffer_len );

	buffer = malloc(buffer_len);
	if (buffer == NULL) {
		tlk_set_errstr("ENOMEM");
		return -1;
	}
	header_buf = buffer;
	data_buf = buffer + header_size;

	if (tlkdebug)
		fprintf(stderr, "tlk_format_write allocated header_buf %p, data_buf %p\n", header_buf, data_buf);

	// Put the header in the header_buf and adjust the data start offset.
	memcpy(header_buf, &tlk->header, sizeof(tlk_file_header_t));
	tlk_file_header_t * head = (tlk_file_header_t *) header_buf;
	head->_string_count = num_entries;
	head->_string_entry_offset = header_size;

	if (tlkdebug)
		tlk_print_header(head);
	int cc = tlk_format_fill_data(tlk, header_buf + sizeof(tlk_file_header_t) , data_buf, data_size);
	if (cc < 0) {
		free(buffer);
		return cc;
	}
	if (tlkdebug) {
		fprintf(stderr, "header_buf %p, got header_size = 0x%08x, need 0x%08x\n", header_buf, (int)  cc, (int) (header_size -sizeof(tlk_file_header_t)) );
	}
	if (cc + sizeof(tlk_file_header_t) !=  header_size) {
		fprintf(stderr, "Header offset = 0x%08x , got 0x%08x\n", (int) (header_size -sizeof(tlk_file_header_t) ),(int) cc);
		tlk_set_errstr("header size mismatch.");
		free(buffer);
		return -1;

	}
	tlkfile_t * testtlk =  tlk_format_parse_buffer(filename, buffer, buffer_len);
	if (testtlk == NULL) {
		tlk_set_errstr("Unable to validate output data.");
		free(buffer);
		return -1;
	}
	//tlk_print_tlkfile(testtlk, 1);
	tlkfile_free(testtlk);
	
	cc = tlk_do_write_file(filename, buffer, buffer_len);
	if (cc < 0) {
		free(buffer);
		return -1;
	}
	return 0;
}


tlkformat_t tlk_format = {
	.name = "tlk",
	.read = tlk_format_read,
	.write = tlk_format_write,
	.check = tlk_format_validate
};


void tlk_format_init_module() {
	tlk_register_format(&tlk_format);
}
