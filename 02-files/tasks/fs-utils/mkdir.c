#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DEFAULT_MODE (S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH)

enum Error { NeedPOption = 2, AlreadyExist, FailedCreateDir };

int Make(const char *path, const bool parent_enable, uint32_t mode) {
  struct stat _;

  if (stat(path, &_) != -1) {
    errno = AlreadyExist;
    return AlreadyExist;
  }

  char curr_path[strlen(path) + 1];

  char buff[strlen(path) + 1];
  memcpy(buff, path, strlen(path) + 1);

  for (int i = strlen(path) - 1; (buff[i] == '/') && (buff[i] = '\0', true);
       --i) {
  }

  for (size_t i = 0; (buff[i] != '\0') || (curr_path[i + 1] = '\0', false);
       ++i) {
    if (buff[i] == '/' && i != 0) {
      curr_path[i] = '\0';

      if (stat(curr_path, &_) == -1) {
        if (!parent_enable) {
          printf("need -p to create : %s\n", curr_path);
          errno = NeedPOption;
          return NeedPOption;
        }
        errno = 0;
        int res = mkdir(curr_path, 755);
        if (res != 0) {
          perror("Failed create directory1");
          return FailedCreateDir;
        }
      }
    }
    curr_path[i] = buff[i];
    curr_path[i + 1] = '\0';
  }

  int res = mkdir(curr_path, mode);
  if (res != 0) {
    perror("Failed create directory2");
    return 1;
  }

  return 0;
}

int main(int argc, char **argv) {
  // declare options
  const char *kAvailArgs = "pm:";

  char *mask = NULL;
  bool parent_enable = false;

  static struct option long_opt[] = {{"mode", 1, 0, 'm'}, {"p", 0, 0, 'p'}};
  int opt;
  while ((opt = getopt_long(argc, argv, kAvailArgs,
                            (const struct option *)&long_opt, NULL)) != -1) {
    switch (opt) {
      case 'p': {
        parent_enable = true;
        break;
      }
      case 'm': {
        mask = optarg;
        break;
      }
      default:
        exit(1);
    }
  }

  for (; argv[optind] != NULL; ++optind) {
    int res = Make(argv[optind], parent_enable,
                   mask == NULL ? DEFAULT_MODE : strtoull(mask, 0L, 8));
    if (res != 0) {
      exit(errno);
    }
  }

  return 0;
}
