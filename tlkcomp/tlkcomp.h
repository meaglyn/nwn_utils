/* tlkcomp.h */


#ifndef TLKCOMP_H_
#define TLKCOMP_H_

//#define DEBUG 0
//#define VERBOSE 1

extern int tlkverbose;
extern int tlkdebug;
extern int tlknotranslate;
extern int tlkoutoforder;
extern int tlkquiet;
extern int tlkrenumber;

extern char tlk_errstr[256];

extern char *tlkcomp_name;
extern char *tlkcomp_version;
extern char *tlkcomp_contact;


/* File header  */
struct tlk_file_header_s {
	/* "TLK " */
	char _tlk[4];
	/* "V3.0" */
	char  _ver[4];
	/* 0 == English */
	unsigned int _lang;

	unsigned int _string_count;
	unsigned int _string_entry_offset;
};

typedef struct tlk_file_header_s tlk_file_header_t;


struct tlk_string_data_element_s {
	/* TEXT_PRESENT 0x0001 
	   If flag is set, there is text specified in the file for this
	   StrRef. Use the OffsetToString and StringSize to
	   determine what the text is.
	   If flag is unset, then this StrRef has no text. Return an
	   empty string.
	*/
	unsigned int _flags;
	char _sound_resref[16];
	unsigned int _unused1; // volume variance
	unsigned int _unused2; // pitch variance
	unsigned int _offset; // offset to string from  _string_entry_offset
	unsigned int _string_size; //length of string not counting null terminator (which is not stored on disk);

	float _soundlength; // unused
}; 

typedef struct tlk_string_data_element_s tlk_string_data_element_t;

struct tlk_element_s {
	tlk_string_data_element_t e;

	// This is the tlk file entry number in this tlk file.
	unsigned int _line;
	struct tlk_element_s * prev;
	struct tlk_element_s * next;

	char * _data; // this is malloced space for the actual string bytes.
};

typedef struct tlk_element_s tlk_element_t;

struct tlkfile_s {
	tlk_file_header_t header;
	
	int count;
	unsigned int data_len; // length of all the string data - not counting nulls

	tlk_element_t * start;
	tlk_element_t * end;
	char * filename;
};
typedef struct tlkfile_s tlkfile_t;

struct tlkformat_s {
	char name[8];
	
	tlkfile_t * (*read)(char * filename);
	int (*write) (tlkfile_t * tlk, char * filename);
	int (*check) (char * filename);
};  

typedef struct tlkformat_s tlkformat_t; 


void tlk_register_format(tlkformat_t * format);
tlkformat_t * tlk_lookup_format(char * name);
char * tlk_find_format(char * filename);
char * tlk_do_read_file(char *name, int *len);
int tlk_do_write_file(char *name, char * buffer, unsigned int len);
tlkfile_t * tlk_readfile(char* filename, char * format);
int tlk_writefile(tlkfile_t * tlk, char* filename, char * format);
void tlkcomp_init();
void tlk_set_errstr(char * err);
char * tlk_get_errstr();
void tlk_print_header(tlk_file_header_t * head);
void tlk_print_raw_entry(tlk_string_data_element_t * elem);
void tlk_print_tlkfile(tlkfile_t * tlk, int all);

// tlkfile routines
tlkfile_t * tlkfile_new_tlkfile(char * name);
void tlkfile_free(tlkfile_t * tlk);
tlk_element_t * tlkfile_new_element(tlk_string_data_element_t *desc, char * string_data, unsigned int line);
int tlkfile_add_element(tlkfile_t * tlk, tlk_element_t * elem);

// tlk on disk format module
void  tlk_format_init_module();

// tls (uncompiled) on disk format module
void  tls_format_init_module();

#endif
