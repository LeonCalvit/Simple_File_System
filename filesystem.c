// Sam Miller and Sam Miller
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "filesystem.h"

//

// main private file type
struct FileInternals{
    FILE *fp;


}FileInternals;

// file type used by user code
typedef struct FileInternals* File;



FSError fserror;
FileMode mode;

File open_file(char *name, FileMode mode){

}

// create and open new file with pathname 'name' and access mode 'mode'.  Current file
// position is set at byte 0.  Returns NULL on error. Always sets 'fserror' global.
File create_file(char *name, FileMode mode){

}

// close 'file'.  Always sets 'fserror' global.
void close_file(File file){

}

// read at most 'numbytes' of data from 'file' into 'buf', starting at the
// current file position.  Returns the number of bytes read. If end of file is reached,
// then a return value less than 'numbytes' signals this condition. Always sets
// 'fserror' global.
unsigned long read_file(File file, void *buf, unsigned long numbytes){

}

// write 'numbytes' of data from 'buf' into 'file' at the current file position.
// Returns the number of bytes written. On an out of space error, the return value may be
// less than 'numbytes'.  Always sets 'fserror' global.
unsigned long write_file(File file, void *buf, unsigned long numbytes){

}

// sets current position in file to 'bytepos', always relative to the
// beginning of file.  Seeks past the current end of file should
// extend the file. Returns 1 on success and 0 on failure.  Always
// sets 'fserror' global.
int seek_file(File file, unsigned long bytepos){

}

// returns the current length of the file in bytes. Always sets 'fserror' global.
unsigned long file_length(File file){

}

// deletes the file named 'name', if it exists. Returns 1 on success, 0 on failure.
// Always sets 'fserror' global.
int delete_file(char *name){

}

// determines if a file with 'name' exists and returns 1 if it exists, otherwise 0.
// Always sets 'fserror' global.
int file_exists(char *name){

}

// describe current filesystem error code by printing a descriptive message to standard
// error.
void fs_print_error(void){
    switch (fserror){
    case FS_NONE:
        printf("FS: No error. \n");
        break;
    case FS_OUT_OF_SPACE:
        printf("FS: The operation casued the softwaredisk to fill up. \n");
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

