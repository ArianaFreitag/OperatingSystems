#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <string.h>


int errCheck(int returnVal, char *message) {
    if (returnVal < 0 ) {
        fprintf(stderr,"%s: %s\n", message, strerror(errno));
        return -1;
    }
}

int tas(volatile char *lock); // from assembly code

int main(int argc, char *argv[]) {
    long long *mapSafe, *mapUnsafe;
    int numProcess = 4;
    int process[numProcess];
    bool inChild;
    char *mapLock;
    int waiting, i;
    int numIter = 1000000;

    // create shared memory
    mapSafe = (long long *) mmap(0, sizeof(long long), PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mapSafe == MAP_FAILED) {
        fprintf(stderr, "map failed:%s\n", strerror(errno));
        return -1;
    }
    mapUnsafe = (long long *) mmap(0, sizeof(long long), PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mapUnsafe == MAP_FAILED) {
        fprintf(stderr, "map failed:%s\n", strerror(errno));
        return -1;
    }

    // create lock region
    mapLock = (char *) mmap (NULL, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( mapLock == MAP_FAILED) {
        fprintf(stderr, "map failed:%s\n", strerror(errno));
        return -1;
    }

    // fork processes
    for (i = 0; i < numProcess; i++) {
        process[i] = fork();
        errCheck(process[i], "Error: fork failed");
        if (process[i] == 0) {
            inChild = true;
            break;
        } else {
            continue;
        }
    }

    if (inChild) {
        for(i = 0; i < numIter; i++) {
            // no locking
            *mapUnsafe +=1;
        }
        for(i = 0; i < numIter; i++) {
            // do locking
            while (tas(mapLock))
                ;
            // CRITICAL REGION
            *mapSafe +=1;
            // free lock
            *mapLock = 0;
        }
        return 0;

    } else {
        for (i = 0; i < numProcess; i++) {
            waiting = waitpid(process[i], NULL, 0);
            errCheck(waiting, "Error: wait failed");
        }
    }
    // print results
    printf ("Goal: %llu\n",  numProcess * numIter);
    printf ("Value no locking: %llu\n", *mapUnsafe);
    printf ("Value with locking: %llu\n", *mapSafe);

    return 0;
}
