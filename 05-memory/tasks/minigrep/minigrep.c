#include <dirent.h>
#include <fcntl.h>
#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

void rec(pcre* regex, const char* path);

void process_dir(pcre* regex, const char* path, DIR* dir_ptr) {
  struct dirent* entry;

  while ((entry = readdir(dir_ptr)) != NULL) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      char full_path[PATH_MAX];
      snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

      rec(regex, full_path);
    }
  }

  closedir(dir_ptr);
}

void process_file(pcre* regex, const char* path) {
  int fd = open(path, O_RDONLY);

  if (fd < 0) {
    perror("failed to open file");
  }

  size_t file_size = lseek(fd, 0, SEEK_END);

  char* file_ptr = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

  close(fd);

  int ovector[3];
  int rc, start_offset = 0;

  char buffer[4098];
  size_t i = 0;
  size_t line_count = 0;

  for (char* ptr = file_ptr; (void*)ptr < ((void*)file_ptr) + file_size;
       ++ptr) {
    if (*ptr == '\n') {
      buffer[i] = '\0';

      i = 0;
      ++line_count;

      while ((rc = pcre_exec(regex, NULL, buffer, strlen(buffer), start_offset,
                             0, ovector, 3)) >= 0) {
        for (int j = 0; j < rc; j++) {
          printf("%s:%ld: %s\n", path, line_count, buffer);
        }
        start_offset = ovector[1];
      }
      start_offset = 0;
    } else {
      buffer[i++] = *ptr;
    }
  }
}

void rec(pcre* regex, const char* path) {
  DIR* dir_ptr = opendir(path);

  if (dir_ptr) {
    process_dir(regex, path, dir_ptr);
  } else {
    process_file(regex, path);
  }
}

int main(int argc, char** argv) {
  const char* regex = argv[1];
  const char* path = argv[2];

  const char* error;
  int erroffset = 0;

  pcre* re = pcre_compile(regex, 0, &error, &erroffset, NULL);

  if (!re) {
    if (write(2, error, strlen(error)) < 0) {
      perror("couldn't write to stderr");
    }
    return 1;
  }

  rec(re, path);

  pcre_free(re);
  return 0;
}
