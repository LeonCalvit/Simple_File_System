# Sam Miller & Leon Calvit

## 1 Design Limitations

+ File Names will be limited at 255 characters
+ Null Characters are not allowed to start the name or be part of file names.
+ The maximum number of blocks supported for data is 7997, (8000 - 2 blocks of Free space management and X blocks of Inode management)
+ Block 0 and Block 1 Will be used by the filesystem to track free space
+ Maximum file size is BLOCKSIZE - 2 * (NUM_BLOCKS_IN_INODE + (BLOCKSIZE/sizeof(long) - 1))
+ The file system currently only supports 40 files at maximum, although this isn't an inherent limitation, merely an arbitrary number
+ Number of used bytes are stored in the first two bytes of each block, both for tracking and to deal with the degenerate case of a user writng all 0s to disk
+ File information is stored from block 2 to block (2+number maximum inodes)
+ Errors occur if the initial data read into the system on its frst running is not all 0.  Format disk first to avoid this.
+ Naturally, any subsequent runs of the program should produce no errors.

## 2 Design Choices

### Strong readers

The system currently disallows any concurrent access of files.

### 2.1 Free Space Tracking

In order to keep track of in use and free blocks I have decided to use Bit Vectors to track boolean values for the use of the blocks
The Bit Vector is generated initially by scanning through the filesystem, and marking every block that has > 0 used bytes with a 1 in the bit vector


### 2.2 File system structure

+ For the overall design of the filesystem I am using a flat (no subdirectories) namespace.
+ The files will be referenced and defined by a Unix style inode based method.