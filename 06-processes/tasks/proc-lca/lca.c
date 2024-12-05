#include "lca.h"

#include <stdio.h>
#include <stdlib.h>

pid_t get_parent_pid(pid_t child) {
  char path[256];
  char line[256];
  FILE *status_file;
  int ppid = -1;

  snprintf(path, sizeof(path), "/proc/%d/status", child);

  status_file = fopen(path, "r");
  if (!status_file) {
    perror("fopen");
    return -1;
  }

  while (fgets(line, sizeof(line), status_file)) {
    if (sscanf(line, "PPid: %d", &ppid) == 1) {
      break;
    }
  }

  fclose(status_file);

  return ppid;
}

pid_t find_lca(pid_t x, pid_t y) {
  if (x == y) {
    return x;
  }

  const size_t max_parents = 1024;
  pid_t first_parents[max_parents];
  pid_t second_parents[max_parents];

  first_parents[0] = x;
  second_parents[0] = y;

  int last_1 = 0;
  int last_2 = 0;

  do {
    ++last_1;
    first_parents[last_1] = get_parent_pid(first_parents[last_1 - 1]);

  } while (first_parents[last_1] != 0);

  do {
    ++last_2;
    second_parents[last_2] = get_parent_pid(second_parents[last_2 - 1]);

  } while (second_parents[last_2] != 0);

  for (; last_1 > -1 && last_2 > -1 &&
         first_parents[last_1] == second_parents[last_2];
       --last_1, --last_2) {
  }

  if (last_1 == -1) {
    return x;
  }
  if (last_2 == -1) {
    return y;
  }

  return first_parents[last_1 + 1];
}
