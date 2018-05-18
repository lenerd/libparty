#include "utils.h"

#include "stdio.h"
#include "stdlib.h"

void random_bytes(void* buf, size_t nbytes)
{
    FILE* f = fopen("/dev/urandom", "r");
    /* FILE* f = fopen("/dev/zero", "r"); */
    if (f == NULL)
    {
        perror("fopen(\"/dev/urandom\", \"r\")");
        exit(1);
    }
    size_t read = 0;
    while (read < nbytes)
    {
        size_t ret = fread((unsigned char*)buf + read, 1, nbytes - read, f);
        if (ret == 0)
        {
            fprintf(stderr, "fread from /dev/urandom failed");
            exit(1);
        }
        read += ret;
    }

    fclose(f);
}
