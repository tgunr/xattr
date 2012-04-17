/*
 *  xattrfunctions.h
 *  little_lister
 *
 *  Created by Matthew Lye on 11/01/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 *  Using the xattr library calls.
 */

#include <sys/xattr.h>

//  define the option flags.
#define VERBOSE 0x0020
#define SYMBOLIC XATTR_NOFOLLOW
#define REPLACE XATTR_REPLACE
#define	CREATE XATTR_CREATE

int displayXattrKeys(const char *path, const char options);
int displayXattrValue(const char *path, const char *key, const char options);
int setXattrValue(const char *path, const char *key, const char *value, const char options);
int deleteXattr(const char *path, const char *key, const char options);
