// Sam Miller and Leon Calvit
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"

//Inode
struct INode
{
    char name[255];
    int directBlock[12];
    int indirectBlock;

} INode;

// main private file type
struct FileInternals
{
    //TODO: More stuff here...
    FILE *fp;

} FileInternals;

// file type used by user code
typedef struct FileInternals *File;

//Globals
FSError fserror;
FileMode mode;
unsigned char* bitVector; //A bitvector is used to efficiently mark blocks as used oravailable
short unsigned int unusedBits; //Number of unused bits at the end of bitVector
int initialized = 0;


//Startup code. 
void init_fs() {
	initialized = 1;
	
	//Initializes bitvector
	//Creates a buffer the same size as a block, then reads blocks into this buffer, and marks the bitvector if the block is available or not

	bitVector = malloc(software_disk_size() / 8 + 1); //Allocates one bit for every block
	unusedBits = software_disk_size() % 8; //If software_disk_size isn't evenly divisble by 8, then this marks how many bits aren't used.
	unsigned char* buffer = calloc(SOFTWARE_DISK_BLOCK_SIZE, sizeof(unsigned char)); //Creates zero initialized bitvector
	unsigned short size;
	for(long i = 0; i < software_disk_size(); i++) {
		if (read_sd_block(buffer, i) == 0) { //Reads block into buffer, throws an error and returns if there was an error in reading the block
			fserror = FS_IO_ERROR;
			fs_print_error();
			return;
		}
		//Reads first two bytes of buffer, which store the number of used bytes of the block, and if that number is greater than 0, sets the appropriate bit in bitVector to true
		for (int j = 0; j < SOFTWARE_DISK_BLOCK_SIZE; j++) {
			size = (unsigned short)(buffer[0] << 8) + (unsigned short)buffer[1];
			if(size > 0) {
				bitVector[i / 8] |= 0b1 << i % 8;
				break;
			}
		}

	}
	free(buffer);
	return;
}

//Returns index of first free block on the software disk.  If no blocks are free, returns -1
long first_free_block()
{
	for (int i = 0; i < software_disk_size() - unusedBits; i++) {
		if (bitVector[i / 8] >> i%8 & 0b1 == 0) {
			return i;
		}
	}
	return -1;
}


//Takes the index of the block, and flips the availability flag on the bitVector for that block
void flip_block_availability(long index)
{
	if (index > software_disk_size()) {
		puts("Hey, don't do that.");
	}
	else {
		bitVector[index / 8] ^= 0b1 << index % 8; //uses xor bitmask to flip desired bit
	}
}

File open_file(char *name, FileMode mode)
{
	if (!initialized) {
		init_fs();
	}
    return NULL;
}

// create and open new file with pathname 'name' and access mode 'mode'.  Current file
// position is set at byte 0.  Returns NULL on error. Always sets 'fserror' global.
File create_file(char *name, FileMode mode)
{
	if (!initialized) {
		init_fs();
	}
    return NULL;
}

// close 'file'.  Always sets 'fserror' global.
void close_file(File file)
{
    return;
}

// read at most 'numbytes' of data from 'file' into 'buf', starting at the
// current file position.  Returns the number of bytes read. If end of file is reached,
// then a return value less than 'numbytes' signals this condition. Always sets
// 'fserror' global.
unsigned long read_file(File file, void *buf, unsigned long numbytes)
{
	if (!initialized) {
		init_fs();
	}
    return 1;
}

// write 'numbytes' of data from 'buf' into 'file' at the current file position.
// Returns the number of bytes written. On an out of space error, the return value may be less than 'numbytes'.
// Always sets 'fserror' global.
unsigned long write_file(File file, void *buf, unsigned long numbytes)
{
	if (!initialized) {
		init_fs();
	}
    return 1;
}

// sets current position in file to 'bytepos', always relative to the
// beginning of file.  Seeks past the current end of file should
// extend the file. Returns 1 on success and 0 on failure.
// Always sets 'fserror' global.
int seek_file(File file, unsigned long bytepos)
{
	if (!initialized) {
		init_fs();
	}
    return 1;
}

// returns the current length of the file in bytes.
// Always sets 'fserror' global.
unsigned long file_length(File file)
{
    return 1;
}

// deletes the file named 'name', if it exists. Returns 1 on success, 0 on failure.
// Always sets 'fserror' global.
int delete_file(char *name)
{
    // Delete file successfully
    if( 1 )
    {
        return 1;
    }
    else
    {
        fserror=FS_FILE_NOT_FOUND;
        return 0;
    }

}

// determines if a file with 'name' exists and returns 1 if it exists, otherwise 0.
// Always sets 'fserror' global.
int file_exists(char *name)
{
	if (!initialized) {
		init_fs();
	}
    return 1;
}

// describe current filesystem error code by printing a descriptive message to standard error.
void fs_print_error(void)
{
    switch (fserror)
    {
    case FS_NONE:
        printf("FS: No error. \n");
        break;
    case FS_OUT_OF_SPACE:
        printf("FS: The operation caused the softwaredisk to fill up. \n");
        break;
    case FS_FILE_NOT_OPEN:
        printf("FS: Attempted read/write/close/etc. on file that isn't open. \n");
        break;
    case FS_FILE_OPEN:
        printf("FS: File is already open. Concurrent opens are not supported and neither is deleting a file that is open.\n");
        break;
    case FS_FILE_NOT_FOUND:
        printf("FS: Attempted open or delete of file that doesn’t exist. \n");
        break;
    case FS_FILE_READ_ONLY:
        printf("FS:  Attempted write to file opened for READ_ONLY. \n");
        break;
    case FS_FILE_ALREADY_EXISTS:
        printf("FS: Attempted creation of file with existing name. \n");
        break;
    case FS_EXCEEDS_MAX_FILE_SIZE:
        printf("FS: Attempted seek or write would exceed max file size. \n");
        break;
    case FS_ILLEGAL_FILENAME:
        printf("FS: filename begins with a null character. \n");
        break;
    case FS_IO_ERROR:
        printf("FS: something really bad happened... ye who enter abandon all hope. \n");
        break;
    default:
        printf("FS: Unknown error code %d.\n", fserror);
        break;
    }
}

// filesystem error code set (set by each filesystem function)
FSError fserror;
