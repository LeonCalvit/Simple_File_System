# Sam Miller

## 1 Design Limitations

+ File Names will be limited at 255 characters
+ Null Characters are not allowed to start the name or be part of file names.
+ The maximum number of blocks supported for data is 7997, (8000 - 2 blocks of Free space management and X blocks of Inode management)
+ Block 0 and Block 1 Will be used by the filesystem to track

## 2 Design Choices

### 2.1 Free Space Tracking

In order to keep track of in use and free blocks I have decided to use ~~LiNkEd LiStS~~ Bit Vectors to track boolean values for the use of the blocks

### 2.2 File system structure

+ For the overall design of the filesystem I am using a flat (no subdirectories) namespace.
+ The files will be referenced and defined by a Unix style inode based method.
+ T