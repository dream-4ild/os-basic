#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

bool is_same_file(const char* lhs_path, const char* rhs_path) {
  struct stat res1;
  int res = stat(lhs_path, &res1);
  if (res == -1) {
    return false;
  }
  struct stat res2;
  res = stat(rhs_path, &res2);
  if (res == -1) {
    return false;
  }
  return res1.st_ino == res2.st_ino;
}

int main(int argc, const char* argv[]) {
  if (argc != 3) {
    return 1;
  }
  bool res = is_same_file(argv[1], argv[2]);
  if (res) {
    printf("yes\n");
  } else {
    printf("no\n");
  }
  return 0;
}
