//
//  xattr.m
//  xattr
//
//  Created by Dave Carlton on 4/17/12.
//  Copyright (c) 2012 Polymicro Systems. All rights reserved.
//

#import "xattr.h"
#include "xattrfunctions.h"

@implementation XATTR {
	NSURL        *URL;
	enum XATTROptions options;
}
@synthesize URL;
@synthesize options;

- (id)initWithURL: (NSURL *) aURL options: (XATTROptions) theOptions {
	self = [super init];
	if (self) {
		URL     = aURL;
		options = theOptions;
		NSMutableArray *keys   = [NSMutableArray arrayWithCapacity: 1];
		NSMutableArray *values = [NSMutableArray arrayWithCapacity: 1];
	}
	return self;
}

- (id)initWithPath: (NSString *) thePath options: (XATTROptions) theOptions {
	self = [super init];
	if (self) {
		URL     = [[NSURL alloc] initFileURLWithPath: thePath];
		options = theOptions;
	}
	return self;
}

- (NSArray *)keys error:(NSError **)anError {
	size_t bufferSize = listxattr([URL path], NULL, 0, options);                        // find out the buffer size.
	if (bufferSize < 0 || bufferSize == -1) {
		if (anError != NULL) {
			NSString     *description = nil;
			NSDictionary *uDict       = nil;
			int errCode;

			if (errno) {
				description = NSLocalizedString(@"listxattr did not return a buffer size", @"");
				errCode     = errno;
			}
			// Make underlying error.
			NSError      *underlyingError = [[[NSError alloc] initWithDomain: NSPOSIXErrorDomain
			                                                            code: errno userInfo: nil] autorelease];
			// Make and return custom domain error.
			NSArray      *objArray        = [NSArray arrayWithObjects: description, underlyingError, path, nil];
			NSArray      *keyArray        = [NSArray arrayWithObjects: NSLocalizedDescriptionKey,
			                                                           NSUnderlyingErrorKey, NSFilePathErrorKey, nil];
			NSDictionary *eDict           = [NSDictionary dictionaryWithObjects: objArray
			                                                            forKeys: keyArray];

			*anError = [[[NSError alloc] initWithDomain: MyCustomErrorDomain
			                                       code: errCode userInfo: eDict] autorelease];
		}
		return nil;
	}

	/*	LISTXATTR:
The listxattr function fills the allocated text buffer with a list of attribute names,
seperated by the null character ('\0').
The function returns the number of attributes.
*/

	char *listBuffer = malloc(bufferSize);
	if (listBuffer == NULL) {
		if (anError != NULL) {
			NSString     *description = nil;
			NSDictionary *uDict       = nil;
			int errCode;

			if (errno == ENOMEM) {
				description = NSLocalizedString(@"Cannot allocate memory", @"");
				errCode     = ENOMEM;
			}
			// Make underlying error.
			NSError      *underlyingError = [[[NSError alloc] initWithDomain: NSPOSIXErrorDomain
			                                                            code: errno userInfo: nil] autorelease];
			// Make and return custom domain error.
			NSArray      *objArray        = [NSArray arrayWithObjects: description, underlyingError, path, nil];
			NSArray      *keyArray        = [NSArray arrayWithObjects: NSLocalizedDescriptionKey,
			                                                           NSUnderlyingErrorKey, NSFilePathErrorKey, nil];
			NSDictionary *eDict           = [NSDictionary dictionaryWithObjects: objArray
			                                                            forKeys: keyArray];

			*anError = [[[NSError alloc] initWithDomain: MyCustomErrorDomain
			                                       code: errCode userInfo: eDict] autorelease];
		}
		return nil;
	}    // return NOMEM on malloc fail.

	int listLength = listxattr(path, listBuffer, bufferSize, options);

	int n         = 0;                                                            // a character scanner counter.
	int nameStart = 0;                                                    // the index position of each name.
	NSMutableArray *attributes
	for (n = 0; n < listLength; n++) {

		if (listBuffer[n] == '\0') {                                    // at a null termination in the buffer...

			char *item = &listBuffer[nameStart];                            // return a pointer to the name...
			nameStart = n + 1;                                                // and move to the next name.
			NSString *keyName = [NSString stringWithCString: item encoding: NSUTF8Encoding];
			[keys addObject: keyName];
		}
	}

	free(listBuffer);
	return keys;
}


@end
