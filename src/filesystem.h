// main private file type: you implement this in filesystem.c
struct FileInternals;

// file type used by user code
typedef struct FileInternals* File;
#define SOFTWARE_DISK_BLOCK_SIZE 512
// access mode for open_file() and create_file()
typedef enum {
	READ_ONLY, READ_WRITE
} FileMode;

// error codes set in global 'fserror' by filesystem functions
typedef enum  {
  FS_NONE,
  FS_OUT_OF_SPACE,         // the operation caused the software disk to fill up
  FS_FILE_NOT_OPEN,  	   // attempted read/write/close/etc. on file that isn’t open
  FS_FILE_OPEN,      	   // file is already open. Concurrent opens are not
                           // supported and neither is deleting a file that is open.
  FS_FILE_NOT_FOUND, 	   // attempted open or delete of file that doesn’t exist
  FS_FILE_READ_ONLY, 	   // attempted write to file opened for READ_ONLY
  FS_FILE_ALREADY_EXISTS,  // attempted creation of file with existing name
  FS_EXCEEDS_MAX_FILE_SIZE,// seek or write would exceed max file size
  FS_ILLEGAL_FILENAME,     // filename begins with a null character
  FS_IO_ERROR              // something really bad happened
} FSError;

// function prototypes for filesystem API

// open existing file with pathname 'name' and access mode 'mode'.  Current file
// position is set at byte 0.  Returns NULL on error. Always sets 'fserror' global.
File open_file(char *name, FileMode mode);

// create and open new file with pathname 'name' and access mode 'mode'.  Current file
// position is set at byte 0.  Returns NULL on error. Always sets 'fserror' global.
File create_file(char *name, FileMode mode);

// close 'file'.  Always sets 'fserror' global.
void close_file(File file);

// read at most 'numbytes' of data from 'file' into 'buf', starting at the
// current file position.  Returns the number of bytes read. If end of file is reached,
// then a return value less than 'numbytes' signals this condition. Always sets
// 'fserror' global.
unsigned long read_file(File file, void *buf, unsigned long numbytes);

// write 'numbytes' of data from 'buf' into 'file' at the current file position.
// Returns the number of bytes written. On an out of space error, the return value may be
// less than 'numbytes'.  Always sets 'fserror' global.
unsigned long write_file(File file, void *buf, unsigned long numbytes);

// sets current position in file to 'bytepos', always relative to the
// beginning of file.  Seeks past the current end of file should
// extend the file. Returns 1 on success and 0 on failure.  Always
// sets 'fserror' global.
int seek_file(File file, unsigned long bytepos);

// returns the current length of the file in bytes. Always sets 'fserror' global.
unsigned long file_length(File file);

// deletes the file named 'name', if it exists. Returns 1 on success, 0 on failure.
// Always sets 'fserror' global.
int delete_file(char *name);

// determines if a file with 'name' exists and returns 1 if it exists, otherwise 0.
// Always sets 'fserror' global.
int file_exists(char *name);

// describe current filesystem error code by printing a descriptive message to standard
// error.
void fs_print_error(void);

//initialize filesystem with starting data
void init_fs();

//Returns index of the first free block on the software disk.  If no blocks are free, returns -1
long first_free_block();

//Flip the availability flag for the appropriate block in the bitvector
void flip_block_availability(long index);

//Gets the number of used bytes for the specified block
short get_block_used_bytes(long block_num);

//Gets the block number of the (num)th block of the file
long get_block_num_from_file(File file, unsigned int num);

//Takes input data of size size, and pads the data to the desired size in the inputted buffer.
//Also places two bytes in the front to indicate how many used bytes there are.
//Size should not be more than two smaller than desired_size

//Still in testinng. Don't rely on yet

void pad_block(char* input_data, short size, char* buffer, short desired_size);

// filesystem error code set (set by each filesystem function)
extern FSError fserror;

