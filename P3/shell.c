/* Ariana Freitag */
/* ECE-357-Operating-Systems */
/* Professor Hakner */
/* Simple Shell in C */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
struct rusage usage;
struct timeval start, end;

char *getLine(void) {
   int pos = 0;
   char *buffer = malloc(sizeof(char) * 40000);
   int c;

   while (1) {
      c = getchar();
      if (c == '\n' || c == EOF) {
         buffer[pos] = '\0';
         return buffer;
      } else {
         buffer[pos] = c;
      }
      pos++;
   }
}

char **parse(char *line) {
  // parse line and tokenize into strings
  int pos = 0;
  char **argV = malloc(1024 * sizeof(char));
  char *arg;

  if (!argV) {
    fprintf(stderr, "Error allocating argument vector: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  arg = strtok(line, " ");
  while (arg != NULL) {
    argV[pos] = arg;
    pos++;
    arg = strtok(NULL," ");
  }
  argV[pos] = NULL;
  return argV;
}

int launch(char **args) {
   int n, d;
   int i = 0;
   int pid, wpid;
   int status;
   char *redir;
   int fd;
   char *filename;

   gettimeofday(&start, NULL);
   pid = fork();
   if (pid == 0) {
      for (n = 0; args[n] != NULL; n++) { // loop through arguments
         int check = atoi(args[n]);
         if (args[n][0] == '<') { // redir stdin
            redir = args[n];
            args[n] = NULL;
            filename = redir + 1;
            fd = open(filename,O_WRONLY, 0666);
            if (fd < 0) {
              fprintf(stderr, "Cannot open stdin redirection file %s: %s\n", filename, strerror(errno));
              return 1;
            }
            d = dup2(fd,0);
            if(d<0){
               fprintf(stderr, "unable to redirect file %s: %s\n", filename, strerror(errno));
               return 1;
            }
            close(fd);
            if (n == 0) {
               return 1;
            }
         }
         if( args[n][0] == '>' && args[n][1] != '>') { // redir stdout
            redir = args[n];
            args[n] = NULL;
            filename = redir + 1;
            fd = open(filename,O_CREAT|O_TRUNC|O_WRONLY, 0666);
            if (fd < 0) {
              fprintf(stderr, "Cannot open stdout redirection file %s: %s\n", redir, strerror(errno));
              return 1;
            }
            d = dup2(fd,1);
            if(d<0){
               fprintf(stderr, "unable to redirect file %s: %s\n", filename, strerror(errno));
               return 1;
            }
            close(fd);
            if (n == 0) {
               return 1;
            }
         }
         if(args[n][0] == '2' && args[n][1] == '>' && args[n][2] != '>') { // redir stderr
            redir = args[n];
            args[n] = NULL;
            filename = redir + 2;
            fd = open(filename,O_CREAT|O_TRUNC|O_WRONLY, 0666);
            if (fd < 0) {
              fprintf(stderr, "Cannot open stderr redirection file %s: %s\n", redir, strerror(errno));
              return 1;
            }
            d = dup2(fd,2);
            if(d <0){
               fprintf(stderr, "unable to redirect file %s: %s\n", filename, strerror(errno));
               return 1;
            }
            close(fd);
            if (n == 0) {
               return 1;
            }
         }
         if(args[n][0] == '>' && args[n][1] == '>') { // redir stdout
            redir = args[n];
            args[n] = NULL;
            filename = redir + 2;
            fd = open(filename,O_CREAT|O_APPEND|O_WRONLY, 0666);
            if (fd < 0) {
              fprintf(stderr, "Cannot open stdout redirection file %s: %s\n", redir, strerror(errno));
              return 1;
            }
            d = dup2(fd,1);
            if(d < 0){
               fprintf(stderr, "unable to redirect file %s: %s\n", filename, strerror(errno));
               return 1;
            }
            close(fd);
            if (n == 0) {
               return 1;
            }
         }
         if(args[n][0] == '2' && args[n][1] == '>' && args[n][2] == '>') { // redir stderr
            redir = args[n];
            args[n] = NULL;
            filename = redir + 3;
            fd = open(filename,O_CREAT|O_APPEND|O_WRONLY, 0666);
            if (fd < 0) {
              fprintf(stderr, "Cannot open stderr redirection file %s: %s\n", redir, strerror(errno));
              return 1;
            }
            d = dup2(fd,2);
            if(d <0 ){
               fprintf(stderr, "unable to redirect file %s: %s\n", filename, strerror(errno));
               return 1;
            }
            close(fd);
            if (n == 0 ) {
               return 1;
            }
         }
      }
      int exec = execvp(args[0], args);
      if (exec == -1) {
         fprintf(stderr, "Execution of command %s failed: %s\n", args[0], strerror(errno));
         return 1;
      }
      exit(EXIT_FAILURE);
   } else if (pid == -1){
      fprintf(stderr, "Error creating child process with fork: %s\n", strerror(errno));
      return 1;
   } else { // in parent
      wpid = wait3(&status, WUNTRACED, &usage);
   }
   gettimeofday(&end, NULL);
   fprintf(stderr, "Command returned with the return code: %i\n", WEXITSTATUS(status));
   fprintf(stderr, "consuming %ld.%06u real seconds, %ld.%06u user, %ld.%06u system\n", (end.tv_sec - start.tv_sec), (end.tv_usec -start.tv_usec), usage.ru_utime.tv_sec, usage.ru_utime.tv_usec, usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
   return 1;
}

int execute(char **args) {
   int status;
   pid_t pid, wpid;
   char *ignore;
   char *builtIn[] = {
     "exit",
     "cd",
   };

   ignore = &args[0][0];
   if(ignore[0] == '#') {
      return 1;
   } else if (strcmp(args[0], builtIn[0]) == 0) {
      if (args[1] == '\0') {
         _exit(0);
      } else {
         _exit(atoi(args[1]));
      }
      return 0;
   } else if (strcmp(args[0], builtIn[1]) == 0) {
      if (args[1] == NULL) {
         fprintf(stderr, "Error: expected argument to cd\n");
      } else {
         if (chdir(args[1]) != 0) {
            fprintf(stderr, "Change directory failed: %s\n", strerror(errno));
         }
      }
      return 1;
   } else if (ignore == '\0') {
      fprintf(stderr,"Exited shell due to EOF\n");
      return 0;
   } else {
      status = launch(args);
      return status;
   }
}

int main(int argc, char *argv[]) {
   char *line;
   char **args;
   int status = 1;

   while (status == 1) {
      line = getLine();
      args = parse(line);
      status = execute(args);
   }
   return 0;
}
