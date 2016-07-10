//
//  Copyright (c) 2012-2014 Orbotix Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <RobotCommandKit/RKRobotBase.h>

/*! @file */

/*! States used internally for loading a program. */
typedef NS_ENUM(uint8_t, RKOrbBasicProgramLoadState) {
   RKOrbBasicProgramNotLoaded,
   RKOrbBasicProgramLoading,
   RKOrbBasicProgramLoaded
};


/**
 * Class for encapsulating an orbBasic program, and handling the lifecycle the program is loaded, excuted, aborted, and erased. This class will handle loading a file which can be no more than 1024 bytes and the program has to download in 253 byte, so client code does not have to append each chunk. After the program is loaded, the erase method needs be called before loading another program. 
 */
@interface RKOrbBasicProgram : NSObject <RKResponseObserver>

@property ( strong, nonatomic) id<RKRobotBase> robot;

/*! Property to the string representation of the program */
@property ( nonatomic, strong ) NSString *program;
/*! The directory path to where a program resides in the file system. */
@property ( nonatomic, strong ) NSString *path;
/*! The file name of the program on the file system. */
@property ( nonatomic, strong ) NSString *name;
/*! The starting line number when executing the program. This is set to the first line
 number in the program on initialization. */
@property ( nonatomic, assign ) NSInteger startLineNumber;


/*!
 *  Provides the file extension expected for orbBasic program files.
 * @return Always returns the string "orbbas".
 */
+ (NSString *) fileExtension;

/*!
 * Initializes the object with a file a it's directory path.
 * @param filePath The file for an orbBasic program.
 * @param directoryPath The directory that contains the file.
 * @param error An option NSError pointer to set if an error occurs while loading the file.
 * @return The initialized object.
 */
- (id) initWithFilePath:(NSString *) filePath directoryPath:(NSString *) directoryPath error:(NSError **) error;

/*! Saves to a binary file */
-(void)saveToFile;

/*! Deletes the binary file. */
-(void)deleteFile;

/*! Renames the Program. */
-(void)rename:(NSString *)newName;

/*! Loads the program onto Sphero, but does not execute the program. */
- (void) load;
/*! Executes a loaded program, and loads the program if it hasn't already been loaded. */
- (void) execute;
/*! Aborts the current running program. */
- (void) abort;
/*! Erases the program off of Sphero and allows the program to be reloaded. */
- (void) erase;

@end
