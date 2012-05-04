//
//  xattr.m
//  xattr
//
//  Created by Dave Carlton on 4/17/12.
//  Copyright (c) 2012 Polymicro Systems. All rights reserved.
//

//#import <exception_defines.h>
#import "xattr.h"

@implementation XATTR {
}

@synthesize URL;
@synthesize options;
@synthesize resultError;
@synthesize attributes;


- (id)initWithURL: (NSURL *) aURL options: (NSUInteger) theOptions {
	self = [super init];
	if (self) {
		URL        = aURL;
		options    = theOptions;
		attributes = [self getKeysValues];
	}
	return self;
}

- (id)initWithPath: (NSString *) thePath options: (NSUInteger) theOptions {
	self = [super init];
	if (self) {
		URL        = [[NSURL alloc] initFileURLWithPath: thePath];
		options    = theOptions;
		attributes = [self getKeysValues];
	}
	return self;
}

// return YES if error was created in resultError
- (BOOL)ifErrno: (NSString *) description {
	if (errno != 0) {
		NSString *errorDescription = nil;

		errorDescription = [description stringByAppendingFormat: @" at path: %@", [URL path]];

		// Make and return custom domain error.
		NSError      *underlyingError = [[NSError alloc] initWithDomain: NSPOSIXErrorDomain
		                                                           code: errno userInfo: nil];
		NSArray      *objArray        = [NSArray arrayWithObjects: errorDescription, underlyingError, [URL path], nil];
		NSArray      *keyArray        = [NSArray arrayWithObjects: NSLocalizedDescriptionKey,
		                                                           NSUnderlyingErrorKey, NSFilePathErrorKey, nil];
		NSDictionary *eDict           = [NSDictionary dictionaryWithObjects: objArray
		                                                            forKeys: keyArray];

		resultError = [[NSError alloc] initWithDomain: NSPOSIXErrorDomain
		                                         code: errno userInfo: eDict];
		return YES;
	}
	return NO;
}

- (NSMutableDictionary *)getKeysValues {
	NSMutableDictionary *xattributes = [NSMutableDictionary dictionaryWithCapacity: 0];

	const char *path = [[URL path] fileSystemRepresentation];
	ssize_t bufferSize = listxattr(path, NULL, 0, options);                        // find out the buffer size.
	if (!bufferSize < 0 || bufferSize == -1) {
		if ([self ifErrno: @"could not get size for keys"])
			return xattributes;
	}

	if (bufferSize == 0)
		return xattributes;

	/*	LISTXATTR:
	The listxattr function fills the allocated text buffer with a list of attribute names,
	seperated by the null character ('\0').
	*/

	char *buffer = malloc((size_t)bufferSize);
	if (buffer == NULL) {
		if ([self ifErrno: @"could not allocate memory for keys"])
			return xattributes;
	}

	int listLength = listxattr(path, buffer, (size_t)bufferSize, options);
	if (listLength == 0)
		return xattributes;
	if (listLength == -1)
		if ([self ifErrno: @"error getting keys"])
			return xattributes;

	NSString *allKeys = [[NSString alloc] initWithBytes: buffer length: (NSUInteger)(bufferSize - 1) encoding: NSUTF8StringEncoding];
	NSArray  *keys    = [allKeys componentsSeparatedByString: @"\0"];

	NSMutableArray *values = [NSMutableArray arrayWithCapacity: 1];

	free(buffer);
	for (NSString *keyString in keys) {
		const char *item = [keyString cStringUsingEncoding: NSUTF8StringEncoding];
		bufferSize = getxattr(path, item, NULL, 0, 0, options);                    
		if (bufferSize) {
			buffer = malloc((size_t)(bufferSize + 1));                                
            // getxattr returns without nullterm, make room...
			if (buffer == NULL)
				if ([self ifErrno: @"could not allocate memory for values"])
					return xattributes;
			buffer[bufferSize] = '\0';                                        // ...add nullterm.
			int valueLength = getxattr(path, item, buffer, (size_t)bufferSize, 0, options);
			if (valueLength) {
				NSData *attrData = [NSData dataWithBytes: buffer length: (NSUInteger)valueLength];
				if (attrData)
					[values addObject: attrData];
				else
					[values addObject: @""];
			}
			else
				[values addObject: @""];
            free(buffer);
		}
		else
			[values addObject: @""];
	}
	xattributes = [NSMutableDictionary dictionaryWithObjects: values forKeys: keys];
	return xattributes;
}

/*	Delete Xattr:
	This function deletes the named extended attribute of the file at path.
	It will return NO if the attribute does not exist, or cannot be deleted.
	The deletion of the attribute is double-checked by a getxattr call that expects an error upon success.
 */
- (BOOL)deleteAttribute: (NSString *) aAttribute {
	if (aAttribute) {
        const char *aPath = [[URL path] cStringUsingEncoding: NSUTF8StringEncoding];
        const char *aKey  = [aAttribute cStringUsingEncoding: NSUTF8StringEncoding];
        if (removexattr(aPath, aKey, options) != 0)
            if ([self ifErrno: [NSString stringWithFormat: @"failed to remove attribute %@", aAttribute]])
                return NO; // error was created
        // double check to see that it does not exist.
        ssize_t size = getxattr(aPath, aKey, NULL, 0, 0, options);                  
        if (size != -1) {
            errno = EPERM;
            [self ifErrno: [NSString stringWithFormat: @"attribute %@ still exists after delete attempt", aAttribute]];
            return NO; // error was created
        }
		[attributes removeObjectForKey: aAttribute];
        return YES; // attribute was deleted
    } else {
		errno = EINVAL;
		[self ifErrno: @"tried to remove nil key for attribute"];
		return NO;
	}
}

- (BOOL)setAttribute: (NSString *) aAttribute value: (NSString *) xValue {
	if (aAttribute) {
        const char *aPath  = [[URL path] cStringUsingEncoding: NSUTF8StringEncoding];
        const char *aKey   = [aAttribute cStringUsingEncoding: NSUTF8StringEncoding];
        const char *aValue = [xValue cStringUsingEncoding: NSUTF8StringEncoding];
        int result = setxattr(aPath, aKey, aValue, (size_t)[xValue length], 0, options);
        if (result == -1) {
            [self ifErrno: [NSString stringWithFormat: @"failure to set attribute %@", aAttribute]];
            return NO;
        }
		[attributes setValue: xValue  forKey:aAttribute ];
        return YES;
    } else {
		errno = EINVAL;
		[self ifErrno: @"tried to set nil key for attribute"];
		return NO;
	}
}

- (BOOL)hasAttribute: (NSString *) string {
	id result;
	@try {
		result = [attributes valueForKey: string];
	}
	@catch (NSException *e) {
		return NO;
	}
    return result == nil ? NO : YES;
}

- (NSData *)getAttribute: (NSString *) aAttribute {
	if ([self hasAttribute: aAttribute])
		return [attributes valueForKey: aAttribute];
	return nil;
}

@end
