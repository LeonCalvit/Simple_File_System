#include <stdio.h>
#include "softwaredisk.h"

int main(int argc, char const *argv[])
{
    printf("Initializing Disk...\n");

    // wipe disk completely
    init_software_disk();
    printf("done.\n");
    return 0;
}
