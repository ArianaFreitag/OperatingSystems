/* Ariana Freitag */
/* ECE-357-Operating-Systems */
/* Professor Hakner */
/* Find and Replace Using mmap */

#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#define _GNU_SOURCE

int errCheck(int returnVal, char *message) {
    if (returnVal < 0 ) {
        fprintf(stderr,"%s: %s\n", message, strerror(errno));
        return -1;
    } else {
        return 0;
    }
}

off_t fileSize(int file) {
    struct stat st;
    int fileInfo = fstat(file, &st);

    if (fileInfo == 0) {
        return st.st_size;
    } else {
        errCheck(fileInfo, "unable able to stat file");
        return -1;
    }
}

void replace(void *start, char *target, char *replacement, int size) {
    void *pos;
    int len = strlen(replacement);
    int len2 = strlen(target);

    while ((pos = memmem(start, size, target, len2))) {
        // copy replacement into original map
        memcpy(start, replacement, len);
        // reset size of file
        size -= labs(start - pos);
        start = pos;
    }
}

int main(int argc, char *argv[]) {
    char *targ, *rep, *inFile;
    void *map;
    int fin, cl, unmap, size;
    int i = 0;

    if (argc < 4) {
      fprintf(stderr, "Not enough arguments\n");
      return -1;
    }
    targ = argv[1];
    rep = argv[2];
    // loop through Files
    for (int i = 3; i < argc; i++) {
        // open the file
        inFile = argv[i];
        fin = open(inFile, O_RDWR);
        errCheck(fin, "unable to open input file for reading and writing");
        size = fileSize(fin);
        // set up memory mapping
        map = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fin, 0);
        if (map == MAP_FAILED) {
            fprintf(stderr, "map failed:%s\n", strerror(errno));
            return -1;
        }
        // call find and replace function
        replace(map, targ, rep, size);
        // unmap memory
        unmap = munmap(map, size);
        errCheck(unmap, "error unmapping memory map");
        // close
        cl = close(fin);
        errCheck(cl, "error closing file");
    }
    return 0;
}
