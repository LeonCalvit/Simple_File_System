#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"


int main(int argc, char *argv[])
{
    printf("Initializing Disk...\n");
    system("./initfs.out");

    File f=create_file("BIlly",READ_ONLY);
    write_file(f,"Hello",6);


    return 0;

}