#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum { MAX_ARGS_COUNT = 256, MAX_CHAIN_LINKS_COUNT = 256 };

typedef struct {
  char* command;
  uint64_t argc;
  char* argv[MAX_ARGS_COUNT];
} chain_link_t;

typedef struct {
  uint64_t chain_links_count;
  chain_link_t chain_links[MAX_CHAIN_LINKS_COUNT];
} chain_t;

void create_chain(char* command, chain_t* chain) {
  if (chain == NULL) {
    return;
  }

  chain->chain_links_count = 0;

  bool just_was_pipe = false;

  char buffer[MAX_ARGS_COUNT];
  size_t cnt = 0;

  for (size_t i = 0; i < strlen(command); ++i) {
    if (command[i] == '|') {
      ++chain->chain_links_count;
      just_was_pipe = true;
      if (chain->chain_links_count != MAX_CHAIN_LINKS_COUNT) {
        chain->chain_links[chain->chain_links_count].argc = 0;
      }
      continue;
    }

    if (command[i] == ' ') {
      if (!just_was_pipe) {
        buffer[cnt++] = '\0';

        char* tmp_for_arg = malloc(strlen(buffer) + 1);
        strcpy(tmp_for_arg, buffer);

        chain->chain_links[chain->chain_links_count]
            .argv[chain->chain_links[chain->chain_links_count].argc] =
            tmp_for_arg;

        if (chain->chain_links[chain->chain_links_count].argc == 0) {
          chain->chain_links[chain->chain_links_count].command = tmp_for_arg;
        }

        ++chain->chain_links[chain->chain_links_count].argc;
        cnt = 0;
      }
      continue;
    }

    if (just_was_pipe) {
      just_was_pipe = false;
    }

    buffer[cnt++] = command[i];
  }

  buffer[cnt++] = '\0';

  char* tmp_for_arg = malloc(strlen(buffer) + 1);
  strcpy(tmp_for_arg, buffer);

  chain->chain_links[chain->chain_links_count]
      .argv[chain->chain_links[chain->chain_links_count].argc] = tmp_for_arg;
  if (chain->chain_links[chain->chain_links_count].argc == 0) {
    chain->chain_links[chain->chain_links_count].command = tmp_for_arg;
  }

  ++chain->chain_links[chain->chain_links_count].argc;
  ++chain->chain_links_count;
}

void do_work(chain_link_t* chain_link) {
  chain_link->argv[chain_link->argc] = NULL;
  execvp(chain_link->command, chain_link->argv);
}

void run_chain(chain_t* chain) {
  int pipe_fds[2] = {0, 0};
  int input_fd = STDIN_FILENO;

  for (size_t i = 0; i < chain->chain_links_count; ++i) {
    if (pipe(pipe_fds) < 0) {
      perror("pipe failed");
      exit(1);
    }

    pid_t pid = fork();

    if (pid == 0) {
      // child

      if (dup2(input_fd, STDIN_FILENO) < 0) {
        perror("failed STDIN");
        exit(1);
      }

      if (i < chain->chain_links_count - 1) {
        if (dup2(pipe_fds[1], STDOUT_FILENO) < 0) {
          perror("failed STDOUT");
          exit(1);
        }
      }

      close(pipe_fds[0]);
      close(pipe_fds[1]);

      if (input_fd != STDIN_FILENO) {
        close(input_fd);
      }

      do_work(&chain->chain_links[i]);

      exit(0);
    }

    close(pipe_fds[1]);

    if (input_fd != STDIN_FILENO) {
      close(input_fd);
    }

    input_fd = pipe_fds[0];
  }
}

void destroy_chain(chain_t* chain) {
  for (size_t i = 0; i < chain->chain_links_count; ++i) {
    for (size_t j = 0; j < chain->chain_links[i].argc; ++j) {
      free(chain->chain_links[i].argv[j]);
    }
  }
}

int main(int argc, char* argv[]) {
  chain_t chain;
  create_chain(argv[1], &chain);

  run_chain(&chain);
  destroy_chain(&chain);
  return 0;
}
