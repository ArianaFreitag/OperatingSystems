#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

int main(int argc, char *argv[]){
  clock_t begin = clock();
  // initialized variables
  char *inFile;
  char *outFile;
  int buffSize, opt, inputNum, optindGlobal, fout, fin, cl, rd, wr, left;
  int i;
  buffSize = 4096; // set to standard buffer size if no buffer given
  // take in arguments, set inFile, outFile, and buffSize
  while ((opt = getopt(argc, argv, "b:o:")) != -1) {
    switch(opt) {
      case 'b': // buffer size given
        buffSize = atoi(optarg);
        printf("Buffer size: %i\n", buffSize);
        break;
      case 'o': // output file given
        outFile = optarg;
        printf("Output File: %s\n", outFile);
        break;
      default:
        printf("Error input argument");
    }
  }
  // number of inputs
  inputNum = argc - optind;
  // create buffer
  char *buff = malloc((sizeof(char))*buffSize);
  // open and create output file for W only
  if (outFile) {
    fout = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if (fout < 0) {
      fprintf(stderr, "Cannot open output file %s for writing: %s\n", outFile, strerror(errno));
      return -1;
      }
    } else {
      fout = 1; // sets output to standard output
    }
  // set to use standard input
  bool useStandardIn = (inputNum == 0);
  if(inputNum == 0) {
    inputNum = 1;
  }
  // loop through files, read and write to output file
  for (i = 0; i < inputNum; i++) {
    // get files
    if (useStandardIn){
      fin = 0;
    } else {
      inFile = argv[optind+i];
      if (strcmp("-", inFile) == 0) {
        fin = 0; // set to standard input
      } else {
        fin = open(inFile, O_RDONLY);
          if (fin < 0) {
            fprintf(stderr, "Cannot open input file %s for reading: %s\n", inFile, strerror(errno));
            return -1;
          }
      }
    }
    // open files
    if (fin == 0) {
      printf("standard input used\n");
    }
    // read files and write to output
    while ((rd = read(fin, buff, sizeof(char)*buffSize)) > 0) {
      if (rd < 0 ) {
        fprintf(stderr, "Error reading input file %s: %s\n", inFile, strerror(errno));
        return -1;
      } else {
        wr = write(fout, buff, rd);
          if (wr < 0) {
            fprintf(stderr, "Error writing to output file %s: %s\n", outFile, strerror(errno));
            return -1;
          } else if (wr < rd) { // partial write check and fix
            fprintf(stderr, "Partial Write Occured on %s: %s\n", inFile, strerror(errno));
            rd = rd - wr;
            buff = buff + rd;
            wr = 0;
          }
      }
    }
    // close input files
    if (fin != 0) {
      cl = close(fin);
      if (cl < 0) {
        fprintf(stderr, "Error closing input files\n", strerror(errno));
        return -1;
      }
    }
  }
  // close output file
  if (fout != 1) {
    cl = close(fout);
    if (cl < 0) {
      fprintf(stderr, "Error closing output\n", strerror(errno));
      return -1;
    }
  }
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("Time Spent:%f\n", time_spent);
  return 0;
}
