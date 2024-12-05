#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 256

int main(int argc, const char** argv) {
  if (argc == 1) {
    return 1;
  }
  const char* path = argv[1];

  int fd = open(path, O_RDONLY);

  char buffer[BUFFER_SIZE];

  do {
    ssize_t res = read(fd, &buffer, BUFFER_SIZE);
    
    if (res == -1){
      return 1;
    }

    if (res == 0) {
      continue;
    }

    write(STDOUT_FILENO, &buffer, res);

  } while (1);

  return 0;
}
