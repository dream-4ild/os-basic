#include "storage.h"

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static uint64_t min(uint64_t first, uint64_t second) {
  return (first < second ? first : second);
}

void reverse(char* buff, ssize_t len) {
  for (ssize_t i = 0; i < len / 2; ++i) {
    char tmp = buff[i];
    buff[i] = buff[len - i - 1];
    buff[len - i - 1] = tmp;
  }
}

void uint_to_string(uint64_t value, char* dest) {
  ssize_t offset = 0;
  while (value) {
    dest[offset++] = '0' + value % 10;
    value /= 10;
  }
  reverse(dest, offset);
  dest[offset++] = '\0';
}

void separate_path(const char* path, char* buff) {
  int buffer_offset = 0;
  for (int i = 0; i < strlen(path);) {
    uint64_t need_write = min(SUBDIR_NAME_SIZE, strlen(path) - i);
    memcpy(buff + buffer_offset, path + i, need_write);
    buffer_offset += need_write;
    i += need_write;
    if (need_write == SUBDIR_NAME_SIZE) {
      buff[buffer_offset++] = '/';
    }
  }

  if (strlen(path) % SUBDIR_NAME_SIZE == 0) {
    buff[buffer_offset++] = '@';
  }
  buff[buffer_offset++] = '\0';
}

int prepare_path(const char* root_dir, storage_key_t key) {
  char buff[3 * MAX_KEY_SIZE];  // magic constant max_path_with_"/" (*2 должно было хватить)
  strcpy(&buff[0], root_dir);

  separate_path(key, &buff[strlen(root_dir)]);

  char new_buff[3 * MAX_KEY_SIZE];
  memset(new_buff, 0, 3 * MAX_KEY_SIZE);

  for (ssize_t i = 0; buff[i] != '\0' || (new_buff[i] = '\0', false); ++i) {
    new_buff[i] = buff[i];
    if (new_buff[i] == '/') {
      new_buff[i + 1] = '\0';
      struct stat _;
      if (stat(new_buff, &_) == 0) {
        continue;
      }
      if (mkdir(new_buff, (S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH)) == -1) {
        perror("failed created dir");
        return -1;
      }
    }
  }

  return open(new_buff, O_RDWR | O_APPEND | O_CREAT,
              S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH);
}

void return_by_version(int fd, uint64_t* buff_for_version, char* buff_for_key,
                       version_t* need_version) {
  char elem;
  int buff_offset = 0;
  uint64_t version = 0;
  bool was_space = false;

  while (read(fd, &elem, 1) != 0) {
    if (elem == '\n') {
      *buff_for_version = version;
      if (buff_for_key != NULL) {
        buff_for_key[buff_offset] = '\0';
      }
      if (need_version != NULL && *need_version == version) {
        return;
      }

      buff_offset = 0;
      version = 0;
      was_space = false;
      continue;
    }

    if (elem == ' ' && !was_space) {
      was_space = true;
      continue;
    }

    if (elem >= '0' && elem <= '9' && !was_space) {
      version *= 10;
      version += elem - '0';
      continue;
    }
    if (buff_for_key != NULL) {
      buff_for_key[buff_offset++] = elem;
    }
  }

}

void storage_init(storage_t* storage, const char* root_path) {
  if (storage == NULL) {
    return;
  }

  storage->root_path = malloc(strlen(root_path) + 2);
  strcpy(storage->root_path, root_path);
  if (root_path[strlen(root_path)] != '/') {
    storage->root_path[strlen(root_path)] = '/';
    storage->root_path[strlen(root_path) + 1] = '\0';
  }
}

void storage_destroy(storage_t* storage) {
  if (storage == NULL || storage->root_path == NULL) {
    return;
  }

  free(storage->root_path);
}

version_t storage_set(storage_t* storage, storage_key_t key,
                      storage_value_t value) {
  if (storage == NULL || key == NULL) {
    return -1;
  }

  int fd = prepare_path(storage->root_path, key);
  if (fd == -1) {
    perror("failed to open file1");
    return -1;
  }

  uint64_t version = 0;
  return_by_version(fd, &version, NULL, NULL);

  char buff[10];  // magic constant
  uint_to_string(version + 1, buff);
  write(fd, buff, strlen(buff));
  write(fd, " ", 1);
  write(fd, value, strlen(value));
  write(fd, "\n", 1);

  close(fd);

  return version + 1;
}

version_t storage_get(storage_t* storage, storage_key_t key,
                      returned_value_t returned_value) {
  if (storage == NULL || key == NULL || returned_value == NULL) {
    return 0;
  }

  int fd = prepare_path(storage->root_path, key);
  if (fd == -1) {
    perror("failed to open file2");
  }

  uint64_t res = 0;
  return_by_version(fd, &res, returned_value, NULL);

  close(fd);
  return res;
}

version_t storage_get_by_version(storage_t* storage, storage_key_t key,
                                 version_t version,
                                 returned_value_t returned_value) {
  if (storage == NULL || key == NULL || returned_value == NULL) {
    return 0;
  }

  int fd = prepare_path(storage->root_path, key);
  if (fd == -1) {
    perror("failed to open file3");
  }

  uint64_t res = -1;
  return_by_version(fd, &res, returned_value, &version);

  close(fd);
  return res;
}