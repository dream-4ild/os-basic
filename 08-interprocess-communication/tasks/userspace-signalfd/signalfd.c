#define _GNU_SOURCE

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int pipe_fds[2];

void handle_signal(int sig) {
  if (write(pipe_fds[1], &sig, sizeof(int)) != sizeof(int)) {
    perror("failed write");
    exit(2);
  }
}

int signalfd() {
  if (pipe(pipe_fds) < 0) {
    perror("pipe failed");
    exit(1);
  }

  for (int sig = 0; sig < 32; ++sig) {
    signal(sig, handle_signal);
  }
  return pipe_fds[0];
}
