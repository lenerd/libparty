// MIT License
//
// Copyright (c) 2018 Lennart Braun
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "util.h"

#include "stdio.h"
#include "stdlib.h"

void random_bytes(void* buf, size_t nbytes)
{
    FILE* f = fopen("/dev/urandom", "r");
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
