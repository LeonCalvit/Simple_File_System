
#include <stdio.h>
#include "softwaredisk.h"

int main(int argc, char const *argv[])
{
    unsigned long bitmaps[SOFTWARE_DISK_BLOCK_SIZE/sizeof(unsigned long)];
    printf("Initializing Disk...\n");
    // wipe disk completely
    init_software_disk();
    // My init is not as elegant as the one shown in class, at least for now...
    printf("Initializing Free Space Tracking... \n");
    //TODO: Create (2) Free space tracking blocks and mark them in use
    write_sd_block(bitmaps,0);
    write_sd_block(bitmaps,1);
    //TODO: Create inode registration blocks and mark them in use

    printf("done.\n");


    return 0;
}
