/*
 *  xattrfunctions.c
 *  little_lister
 *
 *  Created by Matthew Lye on 11/01/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 *  Using the xattr library calls.
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/xattr.h>
#include <sys/errno.h>
#include <string.h>
#include "xattrfunctions.h"


/*  Display Xattr Keys:
 
	This function displays the names of the extended attributes posessed by the file at 'path'.
	If the long/verbose option is set, it will call the displayXattrValue function,
	with the long/verbose option, for each extended attribute.
 
 */


int displayXattrKeys(const char *path, const char options) {
	
	int result=0;
	
	/*	BUFFER SIZES:
		The xattr functions return a negative value when they fail, assigning a diagnostic value to 'errno'.
		The xattr functions return a size value if called with a null buffer.
	 */
	
//	ssize_t listxattr(const char *path, char *namebuff, size_t size, int options);
	size_t bufferSize=listxattr(path, NULL, 0, options);						// find out the buffer size.
	
	if (bufferSize < 0 || bufferSize == -1 ) { 
		perror(NULL); 
		return errno; 
	}
	
		/*	LISTXATTR:
			The listxattr function fills the allocated text buffer with a list of attribute names,
			seperated by the null character ('\0').  
			The function returns the number of attributes.  
		 */
		
	char *listBuffer=malloc(bufferSize);
	if (listBuffer == NULL) { 
		perror(NULL); 
		return errno; 
	}	// return NOMEM on malloc fail.
		
	int listLength=listxattr(path, listBuffer, bufferSize, options);	

	int n=0;															// a character scanner counter.
	int nameStart=0;													// the index position of each name.
	
	for (n=0; n < listLength; n++) {
		
		if ( listBuffer[n]=='\0' ) {									// at a null termination in the buffer...

			char *item=&listBuffer[nameStart];							// return a pointer to the name...
			nameStart=n+1;												// and move to the next name.
			
			if ( VERBOSE & options)		
				result=result | displayXattrValue(path,item,options);
			else
				printf("%s: %s\n",path, item);
		}
	}
	
	free(listBuffer);
	return result;
		
}


/*  Display Xattr Value:
 
	This function simply displays the value of the named extended attribute of the file at path.
	If the long/verbose option is set, it will return the name of the extended attribute and the value
	as "name=value", for use in shell scripts.
 
	getxattr does *not* null-terminate the strings it returns.  One must either use the buffer size information,
	or append a NULL.  I've done the latter.
 
 */


int displayXattrValue(const char *path, const char *key, const char options) {
	
	size_t bufferSize=getxattr(path, key, NULL, 0, 0,options);					// check the buffer size.
	//printf("buffer size:  %i\n", bufferSize);
	if (bufferSize < 0 ) { 
		perror(NULL); 
		return errno; 
	}						// return NOMEM on malloc fail.

	char *valueBuffer=malloc(bufferSize + 1);								// returns without nullterm, make room...
	if (valueBuffer == NULL) { 
		perror(NULL); 
		return errno; 
	}
	
	valueBuffer[bufferSize]='\0';										// ...add nullterm.
	
	int valueLength=getxattr(path, key, valueBuffer, bufferSize, 0,options);
	if (valueLength < 0) { 
		perror(NULL); 
		return errno; 
	}
														
	printf("%s: ", path);	
	if ( VERBOSE & options )	
		printf("%s: ", key);						// prepend the attribute name.

	fprintf(stdout,"%s\n",valueBuffer);
	free(valueBuffer);
	return 0;
}

/*	Set Xattr Value:
	This function assigns values to new or existing extended attributes.  It responds to the '-R' and '-C' flags,
	which, respectively, prevent the creation of new attributes, or prevent the alteration of old ones.
	If the long/verbose option is set, displayXattrValue will be called with the long/verbose option.
 */

int setXattrValue(const char *path, const char *key, const char *value, const char options) {
	int result;
	int valueSize=0;														// the size of the value to be assigned.
	for (valueSize=0; value[valueSize]!='\0';valueSize++);					// Does argv[] assure NULL term?

	result = setxattr(path,key,value, valueSize,0, options);
	if ( result < 0) { perror(NULL); return errno; }
	if ( VERBOSE & options ) 
		displayXattrValue(path, key, options);	
	
	return 0;
	
}

/*	Delete Xattr:
	This function deletes the named extended attribute of the file at path.
	It will return an error if the attribute does not exist, or cannot be deleted.
	The deletion of the attribute is double-checked by a getxattr call that expects an error upon success.
 */

int deleteXattr(const char *path, const char *key, const char options){

	if ( removexattr(path, key, options) < 0 ) { perror(NULL); return errno; }
	int result=(getxattr(path, key, NULL, 0, 0, options)<0)?0:-1;			// check to see that it does not exist.

	if ( VERBOSE & options ) {
	
		if( result < 0 )	printf("Could not delete attribute '%s'.\n",key);
		else				printf("'%s' deleted.\n",key);
		
	}
	
	return result;
	
}