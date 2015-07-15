// tls_format.c

/// handle on disk format for uncompiled tlk files (called tls).
// For getline
#define _GNU_SOURCE 
#include "unistd.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "tlkcomp.h"


struct tls_data_element_s {
	tlk_string_data_element_t e;
	int line;
	char * data;
};
typedef struct tls_data_element_s tls_data_element_t;


const char * TLS_FILE_HEADER = "#TLS V1.0 Uncompiled TLK source#\n";

int tls_format_validate(char * filename) {
	FILE *file;
	char *lineptr = NULL;
	int cc;
	size_t len;

	//Open file
	file = fopen(filename, "r");
	if (!file) {
		return 1;
	}
	cc = getline(&lineptr, &len, file);
	if (cc < 0) {
		fclose(file);
		if (lineptr)
			free(lineptr);
		return 1;
	}

	if(tlkdebug)
		fprintf(stderr, "tls validate file %s, got len %d line \"%s\"\n", filename, (int) cc, lineptr);
	if (cc != strlen(TLS_FILE_HEADER)) {
		fclose(file);
		if (lineptr)
			free(lineptr);
		return 1;	
	}

	// Success!
	if (!strncmp(lineptr, TLS_FILE_HEADER , cc)) {
		fclose(file);
		if (lineptr)
			free(lineptr);
		return 0;
	}

	fclose(file);
	if (lineptr)
		free(lineptr);
	return 1;
}


#define MIN_CHUNK 128

// read a line from the given buffer. Uses terminator to determine line (usually called with '\n'). 
// *lineptr will should be null or malloc'd space. Will be allocated and reallocated as needed to
// fit the line. *n will return the length of the allocation, and if *lineptr is non-null should contain
// the initial size. Return value is number of bytes written, not including NULL terminator but counting the 
// line terminator character. 
// Offset is the starting location in buffer to read from. buflen is total length of the buffer (used for eof). 
int tls_getstr (char **lineptr, size_t *n, char * buffer, size_t buflen, char terminator, size_t offset) {
	int nchars_avail;		/* Allocated but unused chars in *LINEPTR.  */
	char *read_pos;		/* Where we're reading into *LINEPTR. */
	int ret;
	char *buf_pos;
	char *buf_end = buffer + buflen;
	
	if (!lineptr || !n || !buffer) {
		tlk_set_errstr("Invalid arguments to tls_getstr");//errno = EINVAL;
		return -1;
	}
	
	if (!*lineptr) {
		*n = MIN_CHUNK;
		*lineptr = malloc (*n);
		if (!*lineptr) {
			tlk_set_errstr("ENOMEM");//errno = ENOMEM;
			return -1;
		}
	}
	
	nchars_avail = *n;
	read_pos = *lineptr;
	buf_pos = buffer + offset;
	buf_end = buffer + buflen;
	
	if (buf_pos >= buf_end) {
		return 0;
	}
	
	if (tlkdebug)
		fprintf(stderr, "DBG: offset = %d, buflen = %d\n", (int) offset, (int) buflen);
	for (;;) {		
		register int c = *buf_pos;
		//	fprintf(stderr,"got char \'%c\' at pos %d\n", (char) c,  (int) (buf_pos - buffer));
		buf_pos++;

		/* We always want at least one char left in the buffer, since we
		   always (unless we get an error while reading the first char)
		   NUL-terminate the line buffer.  */
		
		//assert((*lineptr + *n) == (read_pos + nchars_avail));
		if (nchars_avail < 2) {
			if (*n > MIN_CHUNK)
				*n *= 2;
			else
				*n += MIN_CHUNK;
			
			nchars_avail = *n + *lineptr - read_pos;
			*lineptr = realloc (*lineptr, *n);
			if (!*lineptr) {
				tlk_set_errstr("ENOMEM");//errno = ENOMEM;
				return -1;
			}
			read_pos = *n - nchars_avail + *lineptr;
			//assert((*lineptr + *n) == (read_pos + nchars_avail));
		}
		
		if (buf_pos > buf_end) {
			fprintf(stderr, "DBG: curpos = %d, buflen = %d\n", (int) (buf_pos - buffer), (int) (buf_end - buffer));
			/* Return partial line, if any.  */
			if (read_pos == *lineptr)
				return 0;
			else
				break;
		}
		
		*read_pos++ = c;
		nchars_avail--;
		
		if (c == terminator)
			/* Return the line.  */
			break;
	}
	
	/* Done - NUL terminate and return the number of chars read.  */
	*read_pos = '\0';
	
	ret = read_pos - *lineptr;
	return ret;
}

tls_data_element_t * tls_new_element(char *in_str, int line) {
	tls_data_element_t * ret = malloc(sizeof(tls_data_element_t));
	int size=  strlen(in_str);

	memset(ret, 0, sizeof(tls_data_element_t));
	ret->e._flags = 1;
	ret->e._string_size = size;
	
	ret->line = line;
	ret->data = malloc(size + 1);
	strncpy(ret->data, in_str, size);
	ret->data[size] = 0;
	
	if(tlkdebug)
		fprintf(stderr, "New tls elem, \"%s\", len %d\n", ret->data, ret->e._string_size);

	return ret;
}

int tls_output_tlk_element(tlk_element_t * elem, char * buffer) {
	if (tlknotranslate)
			return sprintf(buffer, "<%d><%d>:%s\n", elem->_line, elem->_line, elem->_data);
	return sprintf(buffer, "<%d><%d>:%s\n", elem->_line, elem->_line + 0x1000000, elem->_data);
}
void tls_add_to_element(tls_data_element_t * tls, char *in_str) {

	int size = strlen(in_str);
	char * old = tls->data;
	int old_size = tls->e._string_size;

	tls->data = malloc(size + old_size + 2);
	sprintf(tls->data, "%s\n%s", old, in_str);
	//strncpy(tls->data, old, old_size + 1); // take null terminator as well.
	//strcat(tls->data, in_str);
	tls->e._string_size = size + old_size +1; // new size includes the added \n but not the null
	free(old);
}


tlkfile_t * tls_format_parse_buffer(char * filename, char * buffer, int len) {
	int cc;
	char *linep;
	size_t n;
	size_t buf_offset = 0;
	int line_num = 0; // file line number for error messages
	int s;
	int l, ll;
	char * tmp_buf;
	tls_data_element_t * cur_elem = NULL;
	int last_line_no = -1; // last tlk entry number 
	tlkfile_t * ret;
	tlk_element_t * e;
	char * t;

	// Read the header line off first. and dispose of it.
	linep = NULL;
	cc = tls_getstr (&linep, &n, buffer, len, '\n', buf_offset);
	if (cc < 0) {
		tlk_set_errstr("Not valid TLS file format.");
		return NULL;	
	}
	buf_offset += cc;
	line_num ++;

	if(tlkdebug)
		fprintf(stderr,"%s", linep);
	free(linep);
	
	ret = tlkfile_new_tlkfile(filename);
	

	// Read each line of buffer
	for (;;) {
		linep = NULL;
		cc = tls_getstr (&linep, &n, buffer, len, '\n', buf_offset);
		if (cc < 0) {
			tlk_set_errstr("Not valid TLS file format.");
			return NULL;	
		}
		if (cc == 0) {
			if (tlkdebug)
				fprintf(stderr,"Got EOF\n");
			// End any in-progress element	
			if(cur_elem != NULL) {
				e = tlkfile_new_element(&cur_elem->e, cur_elem->data, cur_elem->line);
				if (tlkfile_add_element(ret, e) < 0)
					return NULL;
				
				free(cur_elem);
				cur_elem = NULL;
			}
			break;
		}
		buf_offset += cc;
		line_num ++;
		
		if(tlkdebug) {
			fprintf(stderr,"%s", linep);
			fprintf(stderr,"len = %d\n", (int)strlen(linep));
		}
		tmp_buf = malloc (cc + 1);
		s = sscanf(linep, "<%d><%d>:%s", &l, &ll, tmp_buf);
		if(tlkdebug)
			fprintf(stderr, "Got s = %d, l= %d, ll = %d, cur_elem = %p, last_line_no = %d \n", s, l, ll, cur_elem, last_line_no);
		if (s != 3) {
			// error if no current element
			if (cur_elem == NULL) {
				char msg[128];
				sprintf(msg, "Error - invalid line at %s:%d\n", filename, line_num);
				tlk_set_errstr(msg);
				return NULL;
			}
			// continue current element if there is one
			linep[cc-1] = 0;
			tls_add_to_element(cur_elem, linep);
			if (tlkdebug) 
				fprintf(stderr, "Adding to line %d\n", cur_elem->line);
		} else {
			// end current element
			if(cur_elem != NULL) {
				e = tlkfile_new_element(&cur_elem->e, cur_elem->data, cur_elem->line);
				if (tlkfile_add_element(ret, e) < 0)
					return NULL;

				free(cur_elem);
				cur_elem = NULL;
			}
			// start new element with this info.
			// Get the rest of the string from the line
			t = strstr(linep, ">:");
			if (t == NULL) {
				// error
				return NULL;
			}
			t += 2;
			memset(tmp_buf,0,cc + 1);
			// Don't keep the newline
			strncpy(tmp_buf, t, cc - (t - linep) -1); 
			if (l == 0 && tlkrenumber) {
				l = last_line_no + 1;	
				if (tlkdebug) 
					fprintf(stderr, "Renumbering line to  %d\n", l);
			}
			if (l == last_line_no) {
				char msg[128];
				sprintf(msg, "Error - duplicate TLS entry %d at line %d of input file %s", l, line_num, filename);
				tlk_set_errstr(msg);
				return NULL;
			}
			
			if (l < last_line_no && !tlkoutoforder) {
				char msg[128];
				sprintf(msg, "Error - out of order TLS entry %d at line %d of input file %s", l, line_num, filename);
				tlk_set_errstr(msg);
				return NULL;
			}

			cur_elem = tls_new_element(tmp_buf, l);
			last_line_no = l;
			if(tlkdebug) {
				if (tlknotranslate)
					fprintf(stderr, "Got tlk entry %d (%d)\n", last_line_no, ll == 0 ? last_line_no : ll);
				else
					fprintf(stderr, "Got tlk entry %d (%d)\n", last_line_no, ll == 0 ? last_line_no + 0x1000000 : ll);
			}
		}
		free(tmp_buf);
		free(linep);
	}

	return ret;
}

tlkfile_t * tls_format_read(char * filename) {
	int len = 0;
	char * buffer;
	tlkfile_t * ret;

	if ( tls_format_validate(filename)) {
		tlk_set_errstr("Not valid TLS file format.");
		return NULL;
	}

	buffer =  tlk_do_read_file(filename, &len);
	if (buffer == NULL)
		return NULL; // do_read set errstr

	ret = tls_format_parse_buffer(filename, buffer, len);
	
	free(buffer);
	return ret;
}

int tls_format_write(tlkfile_t * tlk, char * filename) {
	int size;
	char * buffer;
	char * cur_pos;
	tlk_element_t * elem;
	int cc;

	if (tlk->count <= 0) {
		tlk_set_errstr("No data to write.");
		return -1;
	}

	if (tlk->end == NULL) {
		tlk_set_errstr("Invalid or corrupted tlk data.");
		return -1;	
	}

	size = tlk->data_len + tlk->count * 28 + strlen(TLS_FILE_HEADER) ;

	buffer = malloc(size);
	if (buffer == NULL) {
		tlk_set_errstr("ENOMEM");
		return -1;
	}
	memset(buffer, 0, size);
	cur_pos = buffer;
	
	cur_pos += sprintf(cur_pos, "%s", TLS_FILE_HEADER);

	elem = tlk->start;
	while(elem != NULL) {
		cur_pos += tls_output_tlk_element(elem, cur_pos);
		elem = elem->next;
	}
        cc= tlk_do_write_file(filename, buffer, cur_pos - buffer);
	return cc;
}


tlkformat_t tls_format = {
	.name = "tls",
	.read = tls_format_read,
	.write = tls_format_write,
	.check = tls_format_validate
};


void tls_format_init_module() {
	tlk_register_format(&tls_format);
}
