// Sam Miller and Leon Calvit
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "softwaredisk.h"
#include "filesystem.h"

#define NUM_BLOCKS_IN_INODE 12
#define TOTAL_NUM_INODES 40

//Inode
struct INode
{
    char inUse;
    char name[255];
    int directBlock[NUM_BLOCKS_IN_INODE];
    int indirectBlock; //The index of the block number that stores the indexes of the blocks where the rest of the data is kept.
    int num_blocks; //Total number of blocks used for the file.
} INode;


// main private file type
struct FileInternals
{
  //To Do: Keep track of file node

	struct INode* node;
	FileMode mode;
	unsigned long BytePosition; // The byte position of the pointer used in seek_file
    char open; //If the file is currently open somewhere or not
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
struct INode* nodes[TOTAL_NUM_INODES]; //Array of statically allocated Inodes
int num_nodes = 0;					   //Number of Inodes currently being used.

//Startup code.
void init_fs()
{
	initialized = 1;

	//Initializes bitvector
	//Creates a buffer the same size as a block, then reads blocks into this buffer, and marks the bitvector if the block is available or not

    //Allocates one bit for every block
	bitVector = malloc(software_disk_size() / 8 + 1);

    //If software_disk_size isn't evenly divisible by 8, then this marks how many bits aren't used.
    unusedBits = software_disk_size() % 8;

    unsigned char* buffer = calloc(SOFTWARE_DISK_BLOCK_SIZE, sizeof(unsigned char)); //Creates zero initialized bitvector
	unsigned short size;
	for(long i = 0; i < software_disk_size(); i++)
    {
		if (read_sd_block(buffer, i) == 0)
        {
            //Reads block into buffer, throws an error and returns if there was an error in reading the block
			fserror = FS_IO_ERROR;
			fs_print_error();
			return;
		}
		//Reads first two bytes of buffer, which store the number of used bytes of the block, and if that number is greater than 0, sets the appropriate bit in bitVector to true
		for (int j = 0; j < SOFTWARE_DISK_BLOCK_SIZE; j++)
        {
			size = (unsigned short)(buffer[0] << 8);
			size += (unsigned short)buffer[1];
			if(size > 0)
            {
				bitVector[i / 8] |= 0b1 << i % 8;
				break;
			}
		}
	}
	free(buffer);

	//TODO: Initialize array of Inodes from first blocks of softwaredisk.

	return;
}

//Returns index of first free block on the software disk.  If no blocks are free, returns -1
long first_free_block()
{
	for (int i = 0; i < software_disk_size() - unusedBits; i++)
    {
		if ((bitVector[i / 8] >> i%8 & 0b1) == 0)
        {
			return i;
		}
	}
	return -1;
}


//Takes the index of the block, and flips the availability flag on the bitVector for that block
void flip_block_availability(long index)
{
	if(index > software_disk_size())
    {
		puts("Hey, don't do that.");
	}
	else
    {
		bitVector[index / 8] ^= 0b1 << index % 8; //uses xor bitmask to flip desired bit
	}
}

//Gets the number of used bytes for the specified block
short get_block_used_bytes(long block_num)
{
	unsigned char* buffer = calloc(SOFTWARE_DISK_BLOCK_SIZE, sizeof(unsigned char));
	if (read_sd_block(buffer, block_num) == 0)
	{
		//Reads block into buffer, throws an error and returns if there was an error in reading the block
		fserror = FS_IO_ERROR;
		fs_print_error();
		return 0;
	}
	short size = (unsigned short)(buffer[0] << 8);
	size += (unsigned short)buffer[1];
	free(buffer);
	return size;
}

//Gets the block number of the (num)th block of the file
long get_block_num_from_file(File file, unsigned int num)
{
	
	if (num < NUM_BLOCKS_IN_INODE)
	{
		return file->node->directBlock[num];
	}
	//TODO: if the number of the block requested is greater than the number of blocks in the array direct block,
	//this function needs to read the contents of the indirect block as a sequence of long block numbers, and return the
	//content of the ith-num long in that array
	return 0;
}

File open_file(char *name, FileMode mode)
{
	if (!initialized)
    {
		init_fs();
	}
    return NULL;
}

// create and open new file with pathname 'name' and access mode 'mode'.  Current file
// position is set at byte 0.  Returns NULL on error. Always sets 'fserror' global.
File create_file(char *name, FileMode mode)
{
	if (!initialized)
    {
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
	if (!initialized)
    {
		init_fs();
	}
    return 1;
}

// write 'numbytes' of data from 'buf' into 'file' at the current file position.
// Returns the number of bytes written. On an out of space error, the return value may be less than 'numbytes'.
// Always sets 'fserror' global.
unsigned long write_file(File file, void *buf, unsigned long numbytes)
{
	if (!initialized)
    {
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
	if (!initialized)
    {
		init_fs();
	}
    return 1;
}

// returns the current length of the file in bytes.
// Always sets 'fserror' global.
unsigned long file_length(File file)
{
	//All but the last block use SOFTWARE_DISK_BLOCK_SIZE minus 2 bytes, then add the number of used bytes for the last block
	unsigned long size = (file->node->num_blocks - 1) * (SOFTWARE_DISK_BLOCK_SIZE - 2) + get_block_used_bytes(get_block_num_from_file(file, file->node->num_blocks));
    return size;
}

// deletes the file named 'name', if it exists. Returns 1 on success, 0 on failure.
// Always sets 'fserror' global.
int delete_file(char *name)
{
    // Delete file successfully
    if( !file_exists(name)){
        fserror=FS_FILE_NOT_FOUND;
		fs_print_error();
        return 0;
    }
    
	struct INode* node = NULL;
	int i = 0;
	//Find the correct number of the specified INode in the array of inodes.
	//Always finds a value because code above checks that said file exists first.
	//Need to use the index of the INode later for swapping.
	for(; i < num_nodes; i++) {
		if (strcmp(name, nodes[i]->name) == 0) {
			node = nodes[i];
			break;
		}
	}

	if (nodes[i]->inUse == 'y') {
		fserror = FS_FILE_OPEN;
		fs_print_error();
		return 0;
	}

	if (node->num_blocks < NUM_BLOCKS_IN_INODE) { //Block numbers aren't being stored in the indirect nodes, so deleting the INode is simpler;
		//Goes through blocks of the INode and writes zeroes to them
		unsigned char* empty_buffer = calloc(SOFTWARE_DISK_BLOCK_SIZE, sizeof(unsigned char));
		for (unsigned long j = 0; j < node->num_blocks; j++) {
			write_sd_block(empty_buffer, nodes[i]->directBlock[j]);
			flip_block_availability(nodes[i]->directBlock[j]);
		}
		free(empty_buffer);
		
	}
	else { //More complex case
		//TODO: handle more complex case where node numbers are being stored in the indirect node.
	}
	nodes[i] = nodes[num_nodes - 1];
	num_nodes--;
	return 1;
    
    

}

// determines if a file with 'name' exists and returns 1 if it exists, otherwise 0.
// Always sets 'fserror' global.
int file_exists(char *name)
{
	if (!initialized)
    {
		init_fs();
	}

	for (int i = 0; i < num_nodes; i++) {
		if (strcmp(name, nodes[i]->name) == 0) {
			return 1;
		}
	}

    return 0;
}

//Takes input data of size size, and pads the data to the desired size in the inputted buffer.
//Also places two bytes in the front to indicate how many used bytes there are.
//Size should not be more than two smaller than desired_size

//Still in testing. Don't rely on yet

void pad_block(char * input_data, short size, char* buffer, short desired_size)
{
	if (desired_size - 2 < size) {
		puts("Hey, don't do that.");
		return;
	}
	buffer[0] = (desired_size >> 8) & 0xFF;
	buffer[1] = desired_size & 0xFF;
	for (int i = 2; i < size + 2; i++) {
		buffer[i] = input_data[i - 2];
	}

	return;
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
