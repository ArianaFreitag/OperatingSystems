// Ariana Freitag Problem Set 4
// catgrepmore: Using pipes and handling signals
// Note: Testing worked for file sizes of 140 kb
// Note: Testing failed for file size of 3.4 mb

#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/signal.h>

struct sigaction sigact;
int totalBytes = 0;
int totalFiles = 0;
int signo;

void handler(s) {
  if (s==signo) {
    fprintf(stderr,"%s, Bytes read: %d, Files read: %d \n", strsignal(signo), totalBytes, totalFiles);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[]) {
  // parse commands
  char *inFile;
  char *pattern;
  int i;
  pid_t pidMore, pidGrep;
  int buffSize, fin, cl, rd, wr, left;
  struct sigaction sa;
  signo = SIGINT;
  sa.sa_handler=handler;
  sa.sa_flags =0;
  sigemptyset(&sa.sa_mask);
  if (sigaction(signo,&sa,0) == -1) {
    fprintf(stderr, "sigaction failed:%s\n", strerror(errno));
    return -1;
  }
  signal(SIGPIPE, SIG_IGN);
  buffSize = 4096;
  char *buff = malloc((sizeof(char))*buffSize);
  if (buff == NULL) {
    fprintf(stderr, "Malloc failed:%s\n", strerror(errno));
    return -1;
  }
  // check to make sure enough arguments
  if (argc < 3) {
    fprintf(stderr, "Not enough arguments\n");
    return -1;
  }
  // set pattern
  pattern = argv[1];
  for (i = 0; i < argc - 2; i++) { // loop through input files
    int pipeGrep[2], pipeMore[2];
    if (pipe(pipeGrep) == -1) {
      fprintf(stderr, "Creation of grep pipe failed:%s\n", strerror(errno));
      return -1;
    }
    if (pipe(pipeMore) == -1) {
      fprintf(stderr, "Creation of more pipe failed:%s\n", strerror(errno));
      return -1;
    }
    // set up grep and more pipes
    pidMore = fork();
    if (pidMore == 0) { // child more
      if (dup2(pipeMore[0],STDIN_FILENO) == -1) {
        fprintf(stderr, "redirection of pipeMore stdin failed:%s\n", strerror(errno));
        return -1;
      }
      // make sure to close all fd
      close(pipeGrep[0]);
      close(pipeGrep[1]);
      close(pipeMore[0]);
      close(pipeMore[1]);
      if (execlp("more", "more", NULL) == -1) {
        fprintf(stderr, "execution of more failed: %s\n", strerror(errno));
        return -1;
      }
    } else if (pidMore == -1){
      fprintf(stderr, "fork failed: %s\n", strerror(errno));
      return -1;
    }
    printf("here!!!\n");
    pidGrep = fork();
    if (pidGrep == 0) { // child grep
      if (dup2(pipeGrep[0],STDIN_FILENO) == -1) {
        fprintf(stderr, "redirection of pipeGrep stdin failed:%s\n", strerror(errno));
        return -1;
      }
      if (dup2(pipeMore[1],STDOUT_FILENO) == -1) {
        fprintf(stderr, "redirection of pipeGrep stdout failed:%s\n", strerror(errno));
        return -1;
      }
      // make sure to close all fd
      close(pipeGrep[0]);
      close(pipeGrep[1]);
      close(pipeMore[0]);
      close(pipeMore[1]);
      if (execlp("grep","grep",pattern, NULL) == -1) {
        fprintf(stderr, "execution of more failed: %s\n", strerror(errno));
        return -1;
      }
    } else if (pidGrep == -1){
      fprintf(stderr, "fork failed: %s\n", strerror(errno));
      return -1;
    }
    // get file
    inFile = argv[2+i];
    if (strcmp("-", inFile) == 0) {
      fin = 0; // set to standard input
    } else {
      fin = open(inFile, O_RDONLY);
        if (fin < 0) {
          fprintf(stderr, "Cannot open input file %s for reading: %s\n", inFile, strerror(errno));
          return -1;
        }
    }
    totalFiles++;
    // perform cat
    while ((rd = read(fin, buff, sizeof(char)*buffSize)) > 0) {
      if (rd < 0 ) {
        fprintf(stderr, "Error reading input file %s: %s\n", inFile, strerror(errno));
        return -1;
      } else {
        wr = write(pipeGrep[1], buff, rd);
          if (wr < 0) {
            fprintf(stderr, "Error writing to output pipe: %s\n", strerror(errno));
            return -1;
          } else if (wr < rd) { // partial write check and fix
            fprintf(stderr, "Partial Write Occured on %s: %s\n", inFile, strerror(errno));
            rd = rd - wr;
            buff = buff + rd;
            totalBytes += wr;
            wr = 0;
          } else {
            totalBytes += wr;
          }
      }
    }
    close(fin);
    close(pipeGrep[0]);
    close(pipeGrep[1]);
    close(pipeMore[0]);
    close(pipeMore[1]);
    wait(0);
    wait(0);
    free(buff);
  }
  return 0;
}
