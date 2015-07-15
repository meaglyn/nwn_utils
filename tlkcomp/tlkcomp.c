// tlkcomp.c

#define _POSIX_SOURCE
#define _GNU_SOURCE
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "tlkcomp.h"


// tlk compiler helper routines

tlkformat_t * formats[4];
int tlk_num_formats = 0;

char tlk_errstr[256];

int tlkverbose = 0;
int tlkdebug = 0;
int tlknotranslate = 0;
int tlkoutoforder = 0;
int tlkquiet = 0;
int tlkrenumber = 0;

char * tlkcomp_version = "tlkcompiler 1.0";

tlkformat_t * tlk_lookup_format(char * name) {
	tlkformat_t * ret = NULL;
	int i; 

	for (i = 0; i < tlk_num_formats; i++) {
		if (!strcmp(formats[i]->name, name))
			return formats[i];
	}


	return ret;
}

void tlk_register_format(tlkformat_t * format) {

	formats[tlk_num_formats] = format;
	tlk_num_formats ++;
}

void tlk_set_errstr(char * err) {
	if (err == NULL) return;
	strncpy(tlk_errstr, err, 255);
	tlk_errstr[255] = 0;
}
char * tlk_get_errstr() {
	if (strlen(tlk_errstr))
		return tlk_errstr;
	return "No Error";
}

void tlkcomp_init() {

	tlk_errstr[0] = 0;
	tlk_format_init_module();
	tls_format_init_module();
}


char * tlk_do_read_file(char *name, int *len) {
	FILE *file;
	char *buffer;
	unsigned long fileLen;

	//Open file
	file = fopen(name, "rb");
	if (!file) {
		char msg[256]; 
		sprintf(msg, "Unable to open file %s", name);
		tlk_set_errstr(msg);
		return NULL;
	}
	
	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer=(char *)malloc(fileLen+1);
	if (!buffer) {
		tlk_set_errstr("No enough memory.");
		fclose(file);
		return NULL;
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);

	if (len)
		*len = fileLen;

	return buffer;
}

int tlk_do_write_file(char *name, char * buffer, unsigned int len) {
	FILE *file;

	//Open file
	file = fopen(name, "w");
	if (!file) {
		char msg[256]; 
		sprintf(msg, "Unable to open output file %s", name);
		tlk_set_errstr(msg);
		return -1;
	}

	int cc = fwrite(buffer,len, 1, file);
	if (cc < 1 || ferror(file))  {
		tlk_set_errstr("Error writing output file.");
		return -1;
	} 
	fsync(fileno(file));
	fclose(file);
	return len;
}

// Read the given file as if it was the fiven format. Returns NULL on error 
tlkfile_t * tlk_readfile(char* filename, char * format) {
	tlkfile_t * ret = NULL;
	tlkformat_t * fmt =  tlk_lookup_format(format);
	if (fmt == NULL) {
		char msg[256]; 
		sprintf(msg, "Unable to find file format %s\n", format);
		tlk_set_errstr(msg);
		return NULL;
	}

	ret = fmt->read(filename);
	if (ret == NULL) {
		// read op will have set errstr if needed
		return NULL;
	}
		
	return ret;

}

int tlk_writefile(tlkfile_t * tlk, char* filename, char * format) {
	tlkformat_t * fmt =  tlk_lookup_format(format);
	if (fmt == NULL) {
		char msg[256]; 
		sprintf(msg, "Unable to find file format %s\n", format);
		tlk_set_errstr(msg);
		return -1;
	}

	return fmt->write(tlk, filename);
}

char * tlk_find_format(char * filename) {
	int i; 

	for (i = 0; i < tlk_num_formats; i++) {
		if(tlkdebug)
			fprintf(stderr, "Checking %s format against %s\n", formats[i]->name, filename); 
		if (!formats[i]->check(filename)) {
			if(tlkdebug)
				fprintf(stderr, "Found %s format for %s\n", formats[i]->name,  filename); 
			return formats[i]->name;
		}
		if(tlkdebug)
			fprintf(stderr, "Failed %s format for %s\n", formats[i]->name,  filename); 
	}

	return NULL;
}


void tlk_print_header(tlk_file_header_t * head) {
	printf("tlk file header : %c%c%c%c  %c%c%c%c\n\tlangauge = %d\tcount = %u\toffset = 0x%x\n", 
	       head->_tlk[0], head->_tlk[1],  head->_tlk[2], head->_tlk[3],
	       head->_ver[0], head->_ver[1],  head->_ver[2], head->_ver[3],
	       head->_lang, head->_string_count, head->_string_entry_offset);
}
void tlk_print_raw_entry(tlk_string_data_element_t * elem) {
	printf("Entry: flags 0x%04x  offset 0x%08x  len %d: ", elem->_flags, elem->_offset, elem->_string_size);
}

void tlk_print_entries(tlkfile_t * tlk) {
	tlk_element_t * elem = tlk->start;
	while (elem != NULL) {
		
		printf("%d:\"%s\"\n", elem->_line, elem->_data);
		elem = elem->next;
	}

}

void tlk_print_tlkfile(tlkfile_t * tlk, int all) {
	printf("Tlkfile %s:  %d valid entries, highest %d ", tlk->filename, tlk->count, tlk->end == NULL ? 0: tlk->end->_line);
	printf(" total data length %d\n", tlk->data_len);
	if (all) {
		tlk_print_entries(tlk);
	}
}

