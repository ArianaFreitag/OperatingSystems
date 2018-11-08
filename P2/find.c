/* Ariana Freitag */
/* ECE-357-Operating-Systems */
/* Professor Hakner */
/* Recursive Filesystem Lister in C */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

int scanDir(char *directory) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    struct tm fdate;
    mode_t mode;
    char path[512], link[512], date[20], perm[10];

    // open directory
    if (!(dir = opendir(directory))) {
        fprintf(stderr, "Warning: Cannot open directory %s for reading: %s\n", directory, strerror(errno));
        return -1;
    }
    // check for errors with reading directory
    if (readdir(dir) == NULL) {
      fprintf(stderr, "Error reading directory: %s: %s\n", directory, strerror(errno));
      return -1;
    }
    // loop through directory
    while ((entry = readdir(dir)) != NULL) {
        // create paths
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);
        if (lstat(path, &st) < 0) {
          fprintf(stderr, "Cannot stat file %s: %s\n", path, strerror(errno));
          return -1;
        }
        // set mode
        mode = st.st_mode;
        perm[0] = '-';
        perm[1] = (mode & S_IRUSR) ? 'r' : '-';
        perm[2] = (mode & S_IWUSR) ? 'w' : '-';
        perm[3] = (mode & S_IXUSR) ? 'x' : '-';
        perm[4] = (mode & S_IRGRP) ? 'r' : '-';
        perm[5] = (mode & S_IWGRP) ? 'w' : '-';
        perm[6] = (mode & S_IXGRP) ? 'x' : '-';
        perm[7] = (mode & S_IROTH) ? 'r' : '-';
        perm[8] = (mode & S_IWOTH) ? 'w' : '-';
        perm[9] = (mode & S_IXOTH) ? 'x' : '-';
        // get date
        strftime(date, 20, "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
        // output the contents based on either directory, file, or symbolic link
        if (S_ISDIR(mode)) {
          if ((strcmp(entry->d_name, "..") == 0) || (strcmp(entry->d_name, ".") == 0)) {
            if (strcmp(entry->d_name, ".") == 0) {
                perm[0] = 'd';
                printf("%llu %lli %s %hu %u %u %lli %s %s \n", st.st_ino, st.st_blocks, perm, st.st_nlink, st.st_uid, st.st_gid, st.st_size, date, directory);
            }
          } else {
            scanDir(path);
          }
        } else if (S_ISREG(mode)) {
            perm[0] = '-';
            printf("%llu %lli %s %hu %u %u %lli %s %s\n", st.st_ino, st.st_blocks, perm, st.st_nlink, st.st_uid, st.st_gid, st.st_size, date, path);
        } else if (S_ISLNK(mode)) {
            perm[0] = 'l';
            ssize_t length = readlink(path, link, 511);
            if (length < 0) {
              fprintf(stderr, "Cannot read path of symbolic link %s: %s\n", path, strerror(errno));
            } else {
              link[length] = '\0';
            }
            printf("%llu %lli %s %hu %u %u %lli %s %s -> %s\n", st.st_ino, st.st_blocks, perm, st.st_nlink, st.st_uid, st.st_gid, st.st_size, date, path, link);
        } else {
            fprintf(stderr, "This type of file is null: %s\n", strerror(errno));
        }
  }
  // close directory
  if (closedir(dir) < 0) {
    fprintf(stderr, "Can't close directory %s: %s\n", directory, strerror(errno));
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
    char *start;
    if (argc > 2) {
        fprintf(stderr, "Warning: Too many arguments: %s\n", strerror(errno));
    } else if (argc == 2) {
        start = argv[1];
    } else {
      start = ".";
    }
    scanDir(start);
    return 0;
}
