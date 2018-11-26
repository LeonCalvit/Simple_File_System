#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"
#include "softwaredisk.h"

int main(int argc, char *argv[])
{
    printf("Initializing Disk...\n");
    system("./initfs.out");
    init_software_disk();
    unsigned long maxFileSizeCalc = (SOFTWARE_DISK_BLOCK_SIZE-2) * (12 + ((SOFTWARE_DISK_BLOCK_SIZE-2) / (sizeof(unsigned long))));
    printf("%lu", maxFileSizeCalc);

    return 0;
}