// tlkcomui.c
// commanline interface to tlkcomp


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"

#include "tlkcomp.h"

void do_usage() {
	fprintf(stderr, "Usage:\n" 
       "\tnwntlkcomp [-drnv] [-i <fmt>] [-f <fmt] -o <outfile> <infile>\n\n"
   
       "\tWhere outfile is named with an appropriate extension. Infile\n" 
       "\ttype will be detected if possible.\n\n"

       "\t-i <fmt> : specify the format of the input file (\"tlk\" or \"tls\")\n"
       "\t      Will error if file is not of specificied format.\n\n"

       "\t-f <fmt> : specify the format of the output file (\"tlk\" or \"tls\")\n"
       "\t      Will produce output file in given format regardless of \n"
       "\t      outfile's extension.\n\n"

	"\t-n: Do not use high numbers for translating the user entry number.\n"
	"\tThis produces <1><1>: entry instead of <1><16,777,217:entry. This is\n" 
	"\tuseful for decompiling dialog.tlk.\n\n"

	"\t-v: enable a bit mor everbose output, number of entries etc.\n\n"
	"\t-r: enable automatic renumbering of lines set to 0. Will be given immediate next line number. Use with caution.\n\n"

	"\t-d: enable debug output, lots of data you probably don't care about.\n");

}


int main (int argc, char ** argv) {

	char * filename = NULL;
	char * outfile = NULL;
	char * infmt = NULL;
	char * outfmt = NULL;
	char c;
	
	// Read options 
	while ((c = getopt (argc, argv, "df:i:no:qrv")) != -1)
         switch (c)
           {
           case 'd':
		   tlkdebug = 1;
		   break;
           case 'f':
		   outfmt = optarg;
		   break;
           case 'i':
		   infmt = optarg;
		   break;
	   case 'n':
		   tlknotranslate = 1;
		   break;	   
	   case 'o':
		   outfile = optarg;
		   break;
	   case 'q':
		   tlkquiet = 1;
		   break; 
	   case 'r':
		   tlkrenumber = 1;
		   break;
	   case 'v':
		   tlkverbose = 1;
		   break; 
           case '?':
		   if (optopt == 'f' || optopt == 'i' || optopt == 'o')
			   fprintf (stderr, "Option -%c requires an argument.\n", optopt);
		   else if (isprint (optopt))
			   fprintf (stderr, "Unknown option `-%c'.\n", optopt);
		   else
			   fprintf (stderr,
				    "Unknown option character `\\x%x'.\n",
				    optopt);
		   do_usage();
		   return 1;
           default:
		   do_usage();
		   return 1;
           }
    
	filename = argv[optind];
	
	if (tlkquiet && tlkverbose) 
		tlkquiet = 0;

	if (!tlkquiet) { 
		printf("%s:", tlkcomp_version);
		fflush(stdout);
	}

	if (filename == NULL) {
		fprintf(stderr, "No input file given.\n");
		do_usage();
		return 1;
	}

	if (outfile == NULL) {
		fprintf(stderr, "No output file given.\n");
		do_usage();
		return 1;	
	}

	tlkcomp_init();
	// Determine input and output formats (from file names or options?)
	if (infmt == NULL) {
		if(tlkverbose)
			fprintf(stderr, "Trying to find format of input file \"%s\"\n", filename);

		infmt = tlk_find_format(filename);
		if(infmt == NULL) {
			fprintf(stderr,"Unsupported input file format for \"%s\"\n", filename);
			return 1;
		}	  
	} 

	if (outfmt == NULL) {
		// pull of extension from outfile and if present see if it's a valid format.
		char * t = outfile + (strlen(outfile) - 4);
		if (*t == '.') {
			t++;
			tlkformat_t * f =  tlk_lookup_format(t);
			if (f != NULL) {
				outfmt = f->name;
			}
		} 
	}
	if (outfmt == NULL) {
		fprintf(stderr,"Unable to determine output file format for \"%s\"\n", outfile);
		return 1;	
	}
	tlkformat_t * inf =  tlk_lookup_format(infmt);
	if (inf == NULL) {
		fprintf(stderr,"Unsupported input file format \"%s\"\n", infmt);
		return 1;	  	
	}
	tlkformat_t * outf =  tlk_lookup_format(outfmt);
	if (outf == NULL) {
		fprintf(stderr,"Unsupported output file format \"%s\"\n", outfmt);
		return 1;	  	
	}
	

	// if verbose print what we are going to do.
	if (!tlkquiet) {printf("running with infile %s (%s) , outfile %s (%s) %s %s\n", filename, infmt, outfile, outfmt, 
			       ( tlkverbose ? "verbose" : ""), (tlkdebug ? "debug": "") );
		fflush(stdout);
	}

	// read infile in infile format
	tlkfile_t * tlk =  tlk_readfile(filename, inf->name);
	if (!tlk) {
		fprintf(stderr, "Unable to handle file %s: %s\n", filename, tlk_get_errstr());
		return 1;
	}

	// write to outfile in outfile format
	if (tlkverbose || tlkdebug) 
		tlk_print_tlkfile(tlk, tlkdebug);
	
	int cc = tlk_writefile(tlk, outfile, outf->name);
	if (cc < 0) {
		fprintf(stderr, "Unable to write file %s: %s\n", outfile, tlk_get_errstr());
		return 1;
	}
	return 0;
}
