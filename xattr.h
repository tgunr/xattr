//
//  xattr.h
//  xattr
//
//  Created by Dave Carlton on 4/17/12.
//  Copyright (c) 2012 Polymicro Systems. All rights reserved.
//
#include <sys/xattr.h>
#import <Cocoa/Cocoa.h>

enum XATTROptions {
	nofollow = 0x0001,     /* Don't follow symbolic links */
	/* Options for setxattr calls */
	create   = 0x0002,     /* set the value, fail if attr already exists */
	replace  = 0x0004,     /* set the value, fail if attr does not exist */

/* Set this to bypass authorization checking (eg. if doing auth-related work) */
	nosecurity = 0x0008,

/* Set this to bypass the default extended attribute file (dot-underscore file) */
	nodefault = 0x0010,

/* option for f/getxattr() and f/listxattr() to expose the HFS Compression extended attributes */
	showcompressions = 0x0020

};

@interface XATTR : NSObject {
    NSURL *URL;
    enum XATTROptions options;
    NSError *resultError;
	NSMutableDictionary *attributes;
}

@property(nonatomic, strong) NSURL *URL;
@property(nonatomic, assign) enum XATTROptions options;
@property(nonatomic, strong) NSError *resultError;
@property(nonatomic, strong) NSMutableDictionary *attributes;


- (instancetype)initWithURL: (NSURL *) aURL options: (enum XATTROptions) theOptions NS_DESIGNATED_INITIALIZER;
- (instancetype)initWithPath: (NSString *) thePath options: (enum XATTROptions) theOptions NS_DESIGNATED_INITIALIZER;

- (BOOL)ifErrno: (NSString *) description;

@property (NS_NONATOMIC_IOSONLY, getter=getKeysValues, readonly, copy) NSMutableDictionary *keysValues;

- (BOOL)deleteAttribute: (NSString *) aAttribute;

- (BOOL)setAttribute: (NSString *) aAttribute value: (NSString *) xValue;

- (BOOL)hasAttribute: (NSString *) string;

- (NSData *)getAttribute: (NSString *) aAttribute;

@end
