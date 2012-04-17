#include <malloc/malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "xattrfunctions.h"


//command forms
#define LISTER 0
#define GETTER 1
#define SETTER 2
#define DELETE 3
#define HELP 4
#define VERSION 5

#define VERSION_NUMBER "0.33"

/*	GENERAL FLOW MODEL:
 
 (A)	Try to identify a command flag,
 (B)	Set all option bits and fail if an unknown flag is encountered,
 (C)	Iterate through remaining arguments, filling up variables in a predictable pattern, calling
 our xattr functions for every file listed.
 (D)	Terminate with a result of either 0 (PASS) or -1 (FAIL).
 
 */
void version() {
	printf("xattr version %s, released under the MIT liscense.\n", VERSION_NUMBER);
}
void usage()	{
	
	printf("xattr [-p|-w|-d] [-s] [-l] [-R] [-C] [-v] [-h] file [file ...]\n");
	printf("Examples:\n");
	printf("xattr file [file ...]\n");
	printf("\tLists the name of all the extended attributes of the given file(s).\n");
	printf("xattr -p [-sl] attr_name file [file...]\n");
	printf("\tDisplays the value of the attribute 'attr_name' of the given file(s).\n");
	printf("xattr -w [-slRC] attr_name attr_value file [file...]\n");
	printf("\tSets the value of the attribute 'attr_name' to 'attr_value' for the given file(s).\n");
	printf("xattr -d [-sl] attr_name file [file...]\n");
	printf("\tDeletes the attribute 'attr_name' of the given file(s).\n\n");
	printf("options:  -s:  do not follow symbolic links.\n");
	printf("          -l:  long format (verbose) output in a key=value format.\n");
	printf("          -R:  replace existing values only.\n");
	printf("          -C:  create new keys only.\n");
	printf("          -v:  version information.\n");
	printf("          -h:  this help.\n");
}


int main (int argc, const char * argv[]) {
	
	
	/*	COMMAND FLAG:
	 
	 The absence of a command flag implies the 'list' command is being issued.
	 The command and option formats are backwards-compatable with the 'xattr' python script shipped with Leopard.
	 The command flag must be the first argument encountered.
	 An unknown (non-command) flag will be assumed to be an option flag.
	 
	 */
	
	int command=LISTER;												//	The command flag default is to list attributes.
	char options='\0';												// A 'char' representing options as 1-bit flags.
	int ch;
	
	while ((ch = getopt(argc, (char * const *)argv, "dpwhvlsRC")) != -1) {
		switch (ch) {
			case 'd':
				command = DELETE;
				break;
			case 'p':
				command = GETTER;
				break;
			case 'w':												
				command = SETTER;
				break;
			case 'h':												
				usage();											// ..the program terminates here.
				return -1;											
			case 'v':												
			case '?':
				version();											// ..the program terminates here.
				return -1;				
				/*	OPTIONS FLAGS:
				 All option flags currently MUST preceed all value arguments, for simplicity of design.
				 
				 Valid options are:
				 
				 l:	Display result in 'long' format.
				 s:	Do NOT follow symbolic links.
				 R:	ONLY re-write existing attributes.
				 C:	ONLY create new attributes.
				 
				 */ 
				
			case 'l':												
				options = options | VERBOSE;				
				break;	
			case 's':												
				options = options | SYMBOLIC;
				break;
			case 'R':												
				options = options | REPLACE;
				break;
			case 'C':												
				options = options | CREATE;
				break;
			default:
				usage();
		}
	}
	argc -= optind;
	argv += optind;

//	if (command == LISTER || n>1 ) usage();
//	break;
//	
	const char *key;
	const char *value;
	int result=0;

	switch (command) {
		case GETTER:
			key = argv[0];
			argc -= 1;
			argv += 1;
			break;
		case SETTER:
			key = argv[0];
			value = argv[1];
			argc -= 2;
			argv += 2;
			break;
		case DELETE:
			key = argv[0];
			argc -= 1;
			argv += 1;
			break;
	}

	while ( argc ) { // remaining args are paths
		switch (command) {
			case LISTER:
				result = displayXattrKeys(argv[0], options);
				break;
			case GETTER:
				result=result | displayXattrValue(argv[0], key, options);
				break;
			case SETTER:
				result=result | setXattrValue(argv[0], key, value, options);
				break;
			case DELETE:
				result=result | deleteXattr(argv[0], key, options);
				break;
			default:
				break;
		}
		argc -= 1;
		argv += 1;
		if (result < 0) {									// if a problem has arisen...
			printf("xattr: error at %s", argv[0]);
			return -1;										// ..the program terminates here.
		}
	}
	return 0;
}
