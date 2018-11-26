// Sam Miller and Leon Calvit
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "softwaredisk.h"
#include "filesystem.h"

#pragma warning(disable : 4996)
#define NUM_BLOCKS_IN_INODE 12
#define TOTAL_NUM_INODES 40
#define MAX_NAME_LENGTH 255
unsigned long maxFileSizeCalc = (SOFTWARE_DISK_BLOCK_SIZE - 2) * (NUM_BLOCKS_IN_INODE + ((SOFTWARE_DISK_BLOCK_SIZE - 2) / (sizeof(unsigned long))));
#define MAX_FILE_SIZE maxFileSizeCalc

// ------------------------STRUCTS-------------------------
//Inode
struct INode
{
	File file_ptr;
	char name[MAX_NAME_LENGTH];
	unsigned long directBlock[NUM_BLOCKS_IN_INODE];
	unsigned long indirectBlock; //The index of the block number that stores the indexes of the blocks where the rest of the data is kept.
	unsigned int num_blocks;	 //Total number of blocks used for the file.
	unsigned int num_open;		 //The number of people accessing this file.
} INode;

typedef struct INode *inode;
// main private file type
struct FileInternals
{
	struct INode *node_ptr;
	FileMode mode;
	unsigned long BytePosition; // The byte position of the pointer used in seek_file
	char *name;
	FILE *fp;
} FileInternals;

// file type used by user code
typedef struct FileInternals *File;

//---------------------Globals-------------------------
FSError fserror;
unsigned char *bitVector;	  //A bitvector is used to efficiently mark blocks as used or available
short unsigned int unusedBits; //Number of unused bits at the end of bitVector
int initialized = 0;
struct INode nodes[TOTAL_NUM_INODES]; //Array of statically allocated Inodes
int num_nodes = 0;					  //Number of Inodes currently being used.

//----------------------Helper Functions-------------------

// Init a new file
void init_file(File f)
{
	f->node_ptr = NULL;
	f->mode = Closed;
	f->BytePosition = 0;
	f->name = NULL;
	f->fp = NULL;
}

// Init a new Inode
void init_inode(inode i)
{
	strcpy(i->name, "");
	i->num_blocks = 0;
	i->file_ptr = NULL;
	i->indirectBlock = 0;
	i->num_open = 0;
	for (int l = 0; l < NUM_BLOCKS_IN_INODE; l++)
	{
		i->directBlock[l] = 0;
	}
}

//Startup code.
void init_fs()
{
	//This function assumes that either the disk is blank, or there is valid data on it.  Errors occur if the disk is full of random junk.
	initialized = 1;

	//Initializes bitvector
	//Creates a buffer the same size as a block, then reads blocks into this buffer, and marks the bitvector if the block is available or not

	//Allocates one bit for every block
	bitVector = malloc(software_disk_size() / 8 + 1);

	//If software_disk_size isn't evenly divisible by 8, then this marks how many bits aren't used.
	unusedBits = software_disk_size() % 8;

	unsigned char *buffer = malloc(SOFTWARE_DISK_BLOCK_SIZE);
	unsigned short size;
	if (read_sd_block(buffer, 0) == 0)
	{
		//Reads block into buffer, throws an error and returns if there was an error in reading the block
		fserror = FS_IO_ERROR;
		fs_print_error();
		return;
	}
	size = buffer[0] << 8;
	//printf("Value of first half of size -> %d\n", size);
	size |= buffer[1];
	//printf("Value of size -> %d\n", size);
	
	if (size == 0)
	{
		//Generates bitvector from blocks on disk.
		//Slow, as it has to read the entire disk
		//Should only be used on the very first initialization of the filesystem.
		for (unsigned long i = 0; i < software_disk_size(); i++)
		{
			if (read_sd_block(buffer, i) == 0)
			{
				//Reads block into buffer, throws an error and returns if there was an error in reading the block
				fserror = FS_IO_ERROR;
				fs_print_error();
				return;
			}
			//Reads first two bytes of buffer, which store the number of used bytes of the block, and if that number is greater than 0, sets the appropriate bit in bitVector to true
			size = buffer[0] << 8;
			size |= buffer[1];
			if (size > 0)
			{
				bitVector[i / 8] |= 0b1 << i % 8;
				break;
			}
		}
	}
	else
	{
		for (unsigned short i = 0; i < size; i++)
		{
			//printf("Value of i at this iteration ->%d\n Value of size-> %d\n", i, size);
			bitVector[i] = buffer[i];
		}
		bitVector[0] |= 0b11 << 7;
		read_sd_block(buffer, 1);
		size = buffer[0] << 8;
		size |= buffer[1];
		if (size > 0)
		{
			for (short i = SOFTWARE_DISK_BLOCK_SIZE; i < size + SOFTWARE_DISK_BLOCK_SIZE; i++)
			{
				bitVector[i] = buffer[i];
			}
		}
	}
	//Despite the multiple nested loops, this should run very fast because the maximum number of each loop variable is small.
	for (int i = 0; i < NUM_BLOCKS_IN_INODE; i++)
	{ //Initialize inodes from data on disk
		read_sd_block(buffer, i + 2);
		size = buffer[0] << 8;
		size |= buffer[1];
		if (size == 0)
		{
			break;
		}
		strcpy(nodes[i].name, buffer + 2);
		nodes[i].num_blocks = buffer[MAX_NAME_LENGTH + 2] << 24 | buffer[MAX_NAME_LENGTH + 3] << 16 | buffer[MAX_NAME_LENGTH + 4] << 8 | buffer[MAX_NAME_LENGTH + 5];
		if (nodes[i].num_blocks <= NUM_BLOCKS_IN_INODE)
		{
			for (unsigned int j = 0; j < nodes[i].num_blocks; j++)
			{
				for (int k = 0; k < 8; k++)
				{
					nodes[i].directBlock[j] |= buffer[(2 + MAX_NAME_LENGTH + 4) + j + (8 - k)] << (k * 8); //Reads 8 bytes from disk and converts it into a long.
				}
			}
		}
		else
		{
			for (unsigned int j = 0; j < NUM_BLOCKS_IN_INODE; j++)
			{
				for (int k = 0; k < 8; k++)
				{
					nodes[i].directBlock[j] |= buffer[(2 + MAX_NAME_LENGTH + 4) + j + (8 - k)] << (k * 8); //2 == sizeof(short), 4 == sizeof(int) 8 == sizeof(long)
				}
			}
		}
		nodes[i].indirectBlock = 0; //Make sure all the bits are 0 before doing bitwise stuff to it
		for (int j = 0; j < 8; j++)
		{
			nodes[i].indirectBlock |= buffer[(2 + MAX_NAME_LENGTH + 4 + 8 * NUM_BLOCKS_IN_INODE) + (8 - j)] << (j * 8); //2 == sizeof(short), 4 == sizeof(int) 8 == sizeof(long)
		}
		nodes[i].file_ptr = malloc(sizeof(File));
		init_file(nodes[i].file_ptr);
		nodes[i].file_ptr->name = nodes[i].name;
		nodes[i].file_ptr->node_ptr = &nodes[i];
	}
	free(buffer);
	/*
	The structure of the blocks is as follows:
	2 bytes for the short containing the number of used bytes in the block
	255 bytes for the name of the file
	4 bytes for an integer containing the number of blocks the inode has allocated to it
	8*NUM_BLOCKS_IN_INODE bytes for the array of direct blocks even if not all the blocks are used
	8 bytes for the indirect block containing the array of indexes of the indirect blocks
	For the default NUM_BLOCK_IN_INODE value of 12, there should be 147 bytes remaining of the 512 bytes block
	*/
	return;
}

void write_inode_to_disk(int inode_index)
{
	unsigned short used_bytes = MAX_NAME_LENGTH + sizeof(unsigned int) + sizeof(unsigned long) * NUM_BLOCKS_IN_INODE;
	char *buffer = malloc(SOFTWARE_DISK_BLOCK_SIZE);
	buffer[0] = (used_bytes >> 8) & 0xFF;
	buffer[1] = used_bytes & 0xFF;
	strcpy(buffer + sizeof(unsigned short), nodes[inode_index].name);

	//Write the number of blocks used to disk.
	for (int j = 0; j < sizeof(unsigned int); j++)
	{
		buffer[(sizeof(unsigned short) + MAX_NAME_LENGTH) + j] = (nodes[inode_index].num_blocks >> ((sizeof(unsigned int) - j) * 8)) & 0xFF;
	}

	//Write direct blocks to buffer
	for (int i = 0; i < NUM_BLOCKS_IN_INODE; i++)
	{
		for (int j = 0; j < sizeof(unsigned long); j++)
		{
			buffer[sizeof(unsigned short) + MAX_NAME_LENGTH + sizeof(unsigned int) + i + j] = (nodes[inode_index].directBlock[i] >> ((sizeof(unsigned long) - j) * 8)) & 0xFF;
		}
	}

	//Write index of indirect block storage
	for (int i = 0; i < sizeof(unsigned long); i++)
	{
		buffer[(sizeof(unsigned short) + MAX_NAME_LENGTH) + sizeof(unsigned int) + sizeof(unsigned long) * NUM_BLOCKS_IN_INODE + i] = (nodes[inode_index].indirectBlock >> ((sizeof(unsigned long) - i) * 8)) & 0xFF;
	}

	//Write to disk
	write_sd_block(buffer, inode_index + 2);
	free(buffer);
}

void write_fs_to_disk()
{
	for (unsigned int i = 0; i < NUM_BLOCKS_IN_INODE; i++)
	{
		write_inode_to_disk(i);
	}

	unsigned char *buffer = malloc(SOFTWARE_DISK_BLOCK_SIZE);
	if (software_disk_size() > SOFTWARE_DISK_BLOCK_SIZE - 2)
	{
		//Write first part of bitVector to buffer
		buffer[0] = ((SOFTWARE_DISK_BLOCK_SIZE - 2) >> 8) & 0xFF;
		buffer[1] = (SOFTWARE_DISK_BLOCK_SIZE - 2) & 0xFF;
		for (unsigned int i = 2; i < SOFTWARE_DISK_BLOCK_SIZE; i++)
		{
			buffer[i] = bitVector[i];
		}
		write_sd_block(buffer, 0);
		for (unsigned int i = 0; i < SOFTWARE_DISK_BLOCK_SIZE; i++)
		{
			buffer[i] = 0;
		} //Zero the buffer
		//Write second part of bitVector to buffer
		for (unsigned int i = 2; i < (software_disk_size() / 8) - (SOFTWARE_DISK_BLOCK_SIZE - 2) + 1; i++)
		{
			buffer[i] = bitVector[i];
		}
		buffer[0] = (((software_disk_size() / 8) - (SOFTWARE_DISK_BLOCK_SIZE - 2) + 1) >> 8) & 0xFF;
		buffer[1] = ((software_disk_size() / 8) - (SOFTWARE_DISK_BLOCK_SIZE - 2) + 1) & 0xFF;
		write_sd_block(buffer, 1);
	}
	else
	{
		for (unsigned int i = 2; i < software_disk_size() / 8 + 1; i++)
		{
			buffer[i] = bitVector[i];
		}
		buffer[0] = ((software_disk_size() / 8 + 1) >> 8) & 0xFF;
		buffer[1] = (software_disk_size() / 8 - 2 + 1) & 0xFF;
		write_sd_block(buffer, 0);
	}

	free(buffer);
	return;
}

//Returns index of first free block on the software disk.  If no blocks are free, returns -1
long first_free_block()
{
	for (unsigned long i = TOTAL_NUM_INODES + 2; i < software_disk_size() - unusedBits; i++)
	{
		if ((bitVector[i / 8] >> (i % 8) & 0b1) == 0)
		{
			return i;
		}
	}
	return -1;
}

//Takes the index of the block, and flips the availability flag on the bitVector for that block
void flip_block_availability(unsigned long index)
{
	if (index > software_disk_size())
	{
		puts("Error: Attempted to set availability on nonexistant block.");
	}
	else
	{
		bitVector[index / 8] ^= 0b1 << index % 8; //uses xor bitmask to flip desired bit
	}
}

//Gets the number of used bytes for the specified block
unsigned short get_block_used_bytes(long block_num)
{
	unsigned char* buffer3 = malloc(SOFTWARE_DISK_BLOCK_SIZE);
	if (read_sd_block(buffer3, block_num) == 0)
	{
		//Reads block into buffer, throws an error and returns if there was an error in reading the block
		fserror = FS_IO_ERROR;
		fs_print_error();
		return 0;
	}
	unsigned short size = buffer3[0] << 8;
	size |= buffer3[1];
	free(buffer3);
	return size;
}

//Loads an array of longs with the contents of the indirect block
void get_indirect_block_nums(struct INode *node, unsigned long *buf)
{
	if (node->indirectBlock == 0)
	{
		puts("Error: Indicated block has no indirect blocks.");
		return;
	}
	unsigned long *indirect_block = malloc(SOFTWARE_DISK_BLOCK_SIZE);
	if (read_sd_block(indirect_block, node->indirectBlock) == 0)
	{
		//Reads block into buffer, throws an error and returns if there was an error in reading the block
		fserror = FS_IO_ERROR;
		fs_print_error();
		free(indirect_block);
		return;
	}

	for (unsigned int i = 0; i < (node->num_blocks - NUM_BLOCKS_IN_INODE); i++)
	{ //num_blocks - NUM_BLOCKS_IN_INODE is the number of blocks in the indirect node
		buf[i] = indirect_block[i];
	}

	free(indirect_block);
	return;
}

//Gets the block number of the (num)th block of the file
unsigned long get_block_num_from_file(File file, unsigned int num)
{
	if (num > file->node_ptr->num_blocks)
	{
		fserror = FS_IO_ERROR;
		fs_print_error();
		return 0;
	}
	if (num < NUM_BLOCKS_IN_INODE)
	{
		return file->node_ptr->directBlock[num];
	}

	unsigned long *indirect_blocks = malloc(sizeof(unsigned long) * (file->node_ptr->num_blocks - NUM_BLOCKS_IN_INODE));
	get_indirect_block_nums(file->node_ptr, indirect_blocks);
	unsigned long temp = indirect_blocks[num - NUM_BLOCKS_IN_INODE];
	free(indirect_blocks);
	return temp;
}

//Takes input data of size size, and pads the data to the desired size in the inputted buffer.
//Also places two bytes in the front to indicate how many used bytes there are.
//Size should not be more than two smaller than desired_size
void pad_block(char *input_data, short size, char *buffer, short desired_size)
{
	if (desired_size - 2 < size)
	{
		puts("Hey, don't do that.");
		return;
	}
	buffer[0] = (desired_size >> 8) & 0xFF;
	buffer[1] = desired_size & 0xFF;
	for (int i = 2; i < size + 2; i++)
	{
		buffer[i] = input_data[i - 2];
	}

	return;
}

// Retrieves the next free Inode block number from disk
//What does this do? It doesn't seem to get the next free Inode, and it's easy to get that at nodes[num_nodes]
unsigned long get_next_free_Inode()
{
	return 0;
	inode node;
	init_inode(node);
	for (int i = 3; i <= 2 + TOTAL_NUM_INODES; i++)
	{
		unsigned char *buffer = malloc(SOFTWARE_DISK_BLOCK_SIZE);
		read_sd_block(buffer, i);
	}
}

// ----------------------------------------------------------

//------------------Needed Functions--------------------
File open_file(char *name, FileMode mode)
{
	if (!initialized)
	{
		init_fs();
	}
	return NULL;
}

// create and open new file with pathname 'name' and access mode 'mode'.  Current file position is set at byte 0.  Returns NULL on error. Always sets 'fserror' global.
File create_file(char *name, FileMode mode)
{
	fserror = FS_NONE;
	if (!initialized)
	{
		init_fs();
	}

	int i = 0;
	for (; i < 255; i++)
	{
		if (name[i] == '\0')
		{
			break;
		}
	}
	if (i > 254)
	{
		fserror = FS_ILLEGAL_FILENAME;
		fs_print_error();
		return NULL;
	}

	if (first_free_block() == -1 || num_nodes == TOTAL_NUM_INODES)
	{
		fserror = FS_OUT_OF_SPACE;
		fs_print_error();
		return NULL;
	}
	if (file_exists(name))
	{
		fserror = FS_FILE_ALREADY_EXISTS;
		fs_print_error();
		return NULL;
	}

	// init in volitile file struct
	File f = malloc(sizeof(File));
	f->BytePosition = 0;
	f->mode = mode;
	f->node_ptr = &nodes[num_nodes];
	num_nodes++;
	init_inode(f->node_ptr);
	strcpy(f->node_ptr->name, name);
	f->name = f->node_ptr->name;
	f->node_ptr->file_ptr = f;
	f->node_ptr->num_open = 1;

	f->node_ptr->directBlock[0] = first_free_block();
	flip_block_availability(f->node_ptr->directBlock[0]);
	f->node_ptr->num_blocks = 1;
	//Indirect block is unassigned until needed.

	return f;
}

// close 'file'.  Always sets 'fserror' global.
void close_file(File file)
{
	if (file->mode == Closed)
	{
		fserror = FS_FILE_NOT_OPEN;
		fs_print_error();
	}

	// TODO: write any needed changes to file

	file->mode = Closed;
	return;
}

// read at most 'numbytes' of data from 'file' into 'buf', starting at the
// current file position.  Returns the number of bytes read. If end of file is reached,
// then a return value less than 'numbytes' signals this condition. Always sets
// 'fserror' global.
unsigned long read_file(File file, void *buf, unsigned long numbytes)
{
	fserror = FS_NONE;
	if (!initialized)
	{
		init_fs();
	}
	if (file->mode == Closed) 
	{
		fserror = FS_FILE_NOT_OPEN;
		fs_print_error();
		return 0;
	}
	if (file->BytePosition > file_length(file))
	{
		fserror = FS_IO_ERROR;
		fs_print_error();
		return 0;
	}
	unsigned char* buffer = malloc(SOFTWARE_DISK_BLOCK_SIZE);
	unsigned char* buf2 = &buf;
	
	unsigned long current_pos_in_buf = file->BytePosition % (SOFTWARE_DISK_BLOCK_SIZE - 2); //Position of the cursor in the block
	unsigned long bytes_read = 0;
	unsigned long cur_block_index = file->BytePosition / (SOFTWARE_DISK_BLOCK_SIZE - 2);
	unsigned long cur_block = get_block_num_from_file(file, cur_block_index);
	if(current_pos_in_buf + numbytes < SOFTWARE_DISK_BLOCK_SIZE)
	{//Simple case of reading from only one block
		read_sd_block(buffer, get_block_num_from_file(file, cur_block_index));
		for (int i = current_pos_in_buf + 2; i < 2 + current_pos_in_buf + numbytes; i++)
		{
			((unsigned char*)buf)[bytes_read] = buffer[i];
			bytes_read++;
		}
	}
	else
	{
		
		//Read from first block, which isn't guaranteed to start at the start of a block
		read_sd_block(buffer, get_block_num_from_file(file, cur_block_index));
		for (int i = current_pos_in_buf + 2; i < SOFTWARE_DISK_BLOCK_SIZE; i++) {
			buf2[bytes_read] = buffer[i];
			bytes_read++;
		}
		cur_block_index++;
		cur_block = get_block_num_from_file(file, cur_block_index);
		
		//Read all full blocks in the middle
		while ((numbytes - bytes_read) > SOFTWARE_DISK_BLOCK_SIZE - 2)
		{
			read_sd_block(buffer, get_block_num_from_file(file, cur_block_index));
			for (int j = 2; j < SOFTWARE_DISK_BLOCK_SIZE; j++)
			{
				buf2[bytes_read] = buffer[j];
				bytes_read++;
			}
			cur_block_index++;
			cur_block = get_block_num_from_file(file, cur_block_index);
		}
		read_sd_block(buffer, get_block_num_from_file(file, cur_block));
		//Read last block
		for (int i = 2; bytes_read < numbytes; i++) 
		{
			buf2[bytes_read] = buffer[i];
			bytes_read++;
		}
	}
	free(buffer);
	return bytes_read;
}

// write 'numbytes' of data from 'buf' into 'file' at the current file position.
// Returns the number of bytes written. On an out of space error, the return value may be less than 'numbytes'.
// Always sets 'fserror' global.
unsigned long write_file(File file, void *buf, unsigned long numbytes)
{
	fserror = FS_NONE;
	if (!initialized)
	{
		init_fs();
	}

	//Error checking before any writes occur.
	if (file->mode == Closed)
	{
		fserror = FS_FILE_NOT_OPEN;
		fs_print_error();
		return 0;
	}
	if (file->mode == READ_ONLY)
	{
		fserror = FS_FILE_READ_ONLY;
		fs_print_error();
		return 0;
	}
	
	if (file->BytePosition + numbytes > (SOFTWARE_DISK_BLOCK_SIZE - 2) * (NUM_BLOCKS_IN_INODE + (SOFTWARE_DISK_BLOCK_SIZE - 2) / sizeof(unsigned long)))
	{
		fserror = FS_EXCEEDS_MAX_FILE_SIZE;
		fs_print_error();
		return 0;
	}
	
	//unsigned char *buffer = (unsigned char*)malloc(SOFTWARE_DISK_BLOCK_SIZE);
	unsigned char* buffer = malloc(SOFTWARE_DISK_BLOCK_SIZE);
	unsigned char *buf2 = buf; //Void pointers don't allow pointer arithmetic apparently? This is a simple workaround.
	unsigned long current_pos = file->BytePosition;
	unsigned long buf_pos = 0;
	unsigned long cur_block_index = current_pos / (SOFTWARE_DISK_BLOCK_SIZE - 2); //The index of the block in the inode.
	unsigned long cur_block = get_block_num_from_file(file, cur_block_index);	 //The index of the block on the disk
	//Write where only the first block is affected
	if (file->BytePosition % (SOFTWARE_DISK_BLOCK_SIZE - 2) + numbytes < SOFTWARE_DISK_BLOCK_SIZE)
	{
		unsigned short size = numbytes - (current_pos - file->BytePosition); //Number of bytes for the current block to write
		read_sd_block(buffer, cur_block);
		for (int i = 0; i < size; i++)
		{
			buffer[i + 2] = buf2[i];
		}
		size = (size - (file->BytePosition % (SOFTWARE_DISK_BLOCK_SIZE - 2)) + numbytes) > size ? (size - (file->BytePosition % (SOFTWARE_DISK_BLOCK_SIZE - 2)) + numbytes) : size;
		//set size to the larger of size or the end position of numbytes + byteposition
		//This order of operations is correct.  It is NOT supposed to be size - (byteposition + numbytes)
		buffer[0] = (size >> 8) & 0xFF;
		buffer[1] = size & 0xFF;

		write_sd_block(buffer, cur_block);
		free(buffer);
		return numbytes;
	}
	else
	{ //Every other case.
		//If there aren't enough blocks allocated to the inode to store the write
		while(file->BytePosition + numbytes > file_length(file) + SOFTWARE_DISK_BLOCK_SIZE - get_block_used_bytes(get_block_num_from_file(file, file->node_ptr->num_blocks-1)))
			{
			file->node_ptr->num_blocks++;
			if (file->node_ptr->num_blocks > NUM_BLOCKS_IN_INODE) {
				read_sd_block(buffer, file->node_ptr->indirectBlock);
				unsigned long index = first_free_block();
				flip_block_availability(index);
				for (int j = 0; j < sizeof(unsigned long); j++) {
					buffer[2 + (file->node_ptr->num_blocks - NUM_BLOCKS_IN_INODE) * sizeof(unsigned long)] = (index >> ((sizeof(unsigned long) - j) * 8)) & 0xFF;
				}
				unsigned short size = buffer[0] << 8;
				size |= buffer[1];
				size += 2;
				buffer[0] = (size >> 8) & 0xFF;
				buffer[1] = size & 0xFF;
				write_sd_block(buffer, file->node_ptr->indirectBlock);
			}
			else {
				file->node_ptr->directBlock[file->node_ptr->num_blocks - 1] = first_free_block();
				flip_block_availability(file->node_ptr->directBlock[file->node_ptr->num_blocks - 1]);
			}
		}

		read_sd_block(buffer, cur_block);
		for (int i = current_pos % (SOFTWARE_DISK_BLOCK_SIZE - 2); i < SOFTWARE_DISK_BLOCK_SIZE; i++)
		{
			buffer[i + 2] = buf2[i];
		}
		buffer[0] = ((SOFTWARE_DISK_BLOCK_SIZE - 2) >> 8) & 0xFF;
		buffer[1] = (SOFTWARE_DISK_BLOCK_SIZE - 2) & 0xFF;
		write_sd_block(buffer, cur_block);
		cur_block_index++;
		cur_block = get_block_num_from_file(file, cur_block_index);
		buf_pos += SOFTWARE_DISK_BLOCK_SIZE - 2;
		current_pos += SOFTWARE_DISK_BLOCK_SIZE - 2;
		while (numbytes - (current_pos - file->BytePosition) > SOFTWARE_DISK_BLOCK_SIZE - 2)
		{ //While the write is writing full blocks

			for (int i = 0; i < SOFTWARE_DISK_BLOCK_SIZE; i++)
			{
				buffer[i + 2] = buf2[i + buf_pos];
			}
			buffer[0] = ((SOFTWARE_DISK_BLOCK_SIZE - 2) >> 8) & 0xFF;
			buffer[1] = (SOFTWARE_DISK_BLOCK_SIZE - 2) & 0xFF;
			write_sd_block(buffer, get_block_num_from_file(file, cur_block_index));
			cur_block_index++;
			cur_block = get_block_num_from_file(file, cur_block_index);;
			buf_pos += SOFTWARE_DISK_BLOCK_SIZE - 2;
			current_pos += SOFTWARE_DISK_BLOCK_SIZE - 2;
		}
		for (unsigned int i = 0; i < SOFTWARE_DISK_BLOCK_SIZE; i++)
		{
			buffer[i] = 0;
		}																	 //Zero the buffer
		unsigned short size = numbytes - (current_pos - file->BytePosition); //Number of bytes for the current block to write

		if (size > get_block_used_bytes(cur_block))
		{ //If the remaining bytes to write will increase the size of the file
			buffer[0] = (size >> 8) & 0xFF;
			buffer[1] = size & 0xFF;
			for (int i = 0; i < size; i++)
			{
				buffer[i + 2] = buf2[i + buf_pos];
			}
			write_sd_block(buffer, cur_block);
		}
		else
		{ //If the remaining bytes to write won't increase the size of the file.
			read_sd_block(buffer, cur_block);
			for (int i = 0; i < size; i++)
			{
				buffer[i + 2] = buf2[i + buf_pos];
			}
			write_sd_block(buffer, cur_block);
		}
		write_fs_to_disk();
		free(buffer);
		current_pos += size;
		return current_pos - file->BytePosition;
	}


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
	if (!file_exists(file->name))
	{
		fserror = FS_FILE_NOT_FOUND;
		fs_print_error();
		return 0;
	}
	if (bytepos < file_length(file))
	{
		file->BytePosition = bytepos;
		return 1;
	}
	else
	{
		if (bytepos > MAX_FILE_SIZE)
		{
			fserror = FS_EXCEEDS_MAX_FILE_SIZE;
			fs_print_error();
			return 0;
		}
		//TODO: More complex case of if the bytepos is bigger than the file
	}

	return 1;
}

// returns the current length of the file in bytes.
// Always sets 'fserror' global.
unsigned long file_length(File file)
{
	fserror = FS_NONE;
	//All but the last block use SOFTWARE_DISK_BLOCK_SIZE minus 2 bytes, then add the number of used bytes for the last block
	
	return (file->node_ptr->num_blocks) * (SOFTWARE_DISK_BLOCK_SIZE - 2) + get_block_used_bytes(get_block_num_from_file(file, file->node_ptr->num_blocks - 1));
	
}

// deletes the file named 'name', if it exists. Returns 1 on success, 0 on failure.
// Always sets 'fserror' global.
int delete_file(char *name)
{
	fserror = FS_NONE;
	// Delete file successfully
	if (!file_exists(name))
	{
		fserror = FS_FILE_NOT_FOUND;
		fs_print_error();
		return 0;
	}

	struct INode *node = NULL;
	int i = 0;
	//Find the correct number of the specified INode in the array of inodes.
	//Always finds a value because code above checks that said file exists first.
	//Need to use the index of the INode later for swapping.
	for (; i < num_nodes; i++)
	{
		if (strcmp(name, nodes[i].name) == 0)
		{
			node = &nodes[i];
			break;
		}
	}

	if (node->file_ptr->mode == Closed)
	{
		fserror = FS_FILE_OPEN;
		fs_print_error();
		return 0;
	}

	unsigned char *empty_buffer = calloc(SOFTWARE_DISK_BLOCK_SIZE, 1);
	if (node->num_blocks < NUM_BLOCKS_IN_INODE)
	{ //Block numbers aren't being stored in the indirect nodes, so deleting the INode is simpler;
		//Goes through blocks of the INode and writes zeroes to them

		for (unsigned long j = 0; j < node->num_blocks; j++)
		{
			write_sd_block(empty_buffer, nodes[i].directBlock[j]);
			flip_block_availability(nodes[i].directBlock[j]);
		}
	}
	else
	{ //More complex case

		for (unsigned long j = 0; j < NUM_BLOCKS_IN_INODE; j++)
		{ //Free all the blocks in the direct node
			write_sd_block(empty_buffer, nodes[i].directBlock[j]);
			flip_block_availability(nodes[i].directBlock[j]);
		}
		unsigned long *indirect_blocks = malloc(sizeof(unsigned long) * (node->num_blocks - NUM_BLOCKS_IN_INODE));
		get_indirect_block_nums(node, indirect_blocks);
		for (unsigned long j = 0; j < nodes[i].num_blocks - NUM_BLOCKS_IN_INODE; j++)
		{ //Free all the blocks in the indirect node
			write_sd_block(empty_buffer, indirect_blocks[j]);
			flip_block_availability(indirect_blocks[j]);
		}
	}
	free(empty_buffer);
	free(node->file_ptr);
	init_inode(node);
	nodes[i] = nodes[num_nodes - 1];
	num_nodes--;
	write_fs_to_disk();
	return 1;
}

// determines if a file with 'name' exists and returns 1 if it exists, otherwise 0.
// Always sets 'fserror' global.
int file_exists(char *name)
{
	fserror = FS_NONE;
	if (!initialized)
	{
		init_fs();
	}

	for (int i = 0; i < num_nodes; i++)
	{
		if (strcmp(name, nodes[i].name) == 0)
		{
			return 1;
		}
	}

	return 0;
}

// describes the current filesystem error code by printing a descriptive message to standard error.
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
