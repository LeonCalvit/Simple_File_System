
#include <stdio.h>
#include "softwaredisk.h"

int main(int argc, char const *argv[])
{
    unsigned char bitmaps[SOFTWARE_DISK_BLOCK_SIZE/sizeof(char)];

    printf("Initializing Disk...\n");
    // wipe disk completely

    init_software_disk();
    // My init is not as elegant as the one shown in class, at least for now...
    printf("Initializing Free Space Tracking... \n");
    //TODO: Create (2) Free space tracking blocks and mark them in use
    write_sd_block(bitmaps,0);
    write_sd_block(bitmaps,1);

    for(int i =0; i<40; i++)
    {
        //write_sd_block(INode,i);
    }

    //TODO: Create inode registration blocks and mark them in use
   // Something like this -> write_sd_block(INode[SOFTWARE_DISK_BLOCK_SIZE/sizeof(INode)],2);


    printf("done.\n");


    return 0;
}
