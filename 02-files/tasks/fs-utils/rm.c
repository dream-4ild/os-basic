#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILENAME_LEN 256

enum Error{
  NeedROption = 2,
  TooLongPath = 3,
};

void DeleteNode(const char* path, bool rec_enable) {
  struct stat file;
  lstat(path, &file);

  if (file.st_mode & S_IFDIR) {
    if (!rec_enable) {
      errno = NeedROption;
      return;
    }

    DIR* dir;
    if ((dir = opendir(path)) != NULL) {
      struct dirent* ent;
      while ((ent = readdir(dir)) != NULL) {
        static const char* kCurrentDir = ".";
        static const char* kParentDir = "..";

        if (strcmp(ent->d_name, kCurrentDir) != 0 &&
            strcmp(ent->d_name, kParentDir) != 0) {
          char path_to_file[MAX_FILENAME_LEN];
          if (strlen(path) + strlen(ent->d_name) >= MAX_FILENAME_LEN) {
            errno = TooLongPath;
            break;
          }
          strcpy(path_to_file, path);
          if (path_to_file[strlen(path_to_file) - 1] != '/') {
            strcat(path_to_file, "/");
          }
          strcat(path_to_file, ent->d_name);
          DeleteNode(path_to_file, rec_enable);
          if (errno != 0) {
            break;
          }
        }
      }

      closedir(dir);
      rmdir(path);
      if (errno != 0) {
        perror("remove dir failed");
        return;
      }
    }
  } else {
    unlink(path);
    if (errno != 0) {
      perror("remove file failed");
    }
  }
}

int main(int argc, char** argv) {
  // declare options
  const char* kAvailArgs = "r";

  bool rec_enable = false;

  int opt;
  while ((opt = getopt(argc, argv, kAvailArgs)) != -1) {
    switch (opt) {
      case 'r': {
        rec_enable = true;
        break;
      }
      default:
        exit(1);
    }
  }

  for (; argv[optind] != NULL; ++optind) {
    DeleteNode(argv[optind], rec_enable);
    if (errno != 0) {
      exit(errno);
    }
  }

  return 0;
}
