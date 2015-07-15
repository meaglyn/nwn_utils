
#include "stdio.h"

#include "tlkcomp.h"

int main (int argc, char ** argv) {
       char * filename = "test1.tlk";
       char * outfile = "out.tlk";
       tlkfile_t * tlk; 
       
       if (argc > 1) {
	       filename = argv[1];
       }
       if (argc > 2) {
	       outfile = argv[2];
       }

       tlkverbose = 1;
       tlkdebug = 1;

       tlkcomp_init();
       tlk =  tlk_readfile(filename, "tlk");
       if (!tlk) {
	       fprintf(stderr, "Unable to handle file %s: %s\n", filename, tlk_get_errstr());
		return 1;
	}
	if (tlkverbose)
		tlk_print_tlkfile(tlk, 1);

	int cc = tlk_writefile(tlk, outfile, "tlk");
	if (cc < 0) 
		fprintf(stderr, "Unable to write file %s: %s\n", outfile, tlk_get_errstr());
		return 1;

	return 0;
}
