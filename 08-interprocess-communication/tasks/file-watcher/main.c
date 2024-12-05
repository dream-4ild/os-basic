#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct Counter {
  char filename[PATH_MAX];
  int counter;
  struct Counter* next;
} Counter;

typedef struct Counters {
  struct Counter* head;
} Counters;

void increment(Counters* counters, char* filename, int value) {
  Counter* current = counters->head;
  while (current != NULL) {
    if (strncmp(current->filename, filename, PATH_MAX) == 0) {
      current->counter += value;
      return;
    }
    current = current->next;
  }
  Counter* new_head = malloc(sizeof(Counter));
  new_head->next = counters->head;
  new_head->counter = value;
  strncpy(new_head->filename, filename, PATH_MAX - 1);
  counters->head = new_head;
}

void print(Counters* counters) {
  Counter* current = counters->head;
  while (current != NULL) {
    printf("%s:%d\n", current->filename, current->counter);
    current = current->next;
  }
}

char* GetPath(pid_t pid, int fd) {
  char* path = malloc(PATH_MAX);
  char buffer[PATH_MAX];
  snprintf(buffer, sizeof(buffer), "/proc/%d/fd/%d", pid, fd);

  ssize_t len = readlink(buffer, path, PATH_MAX - 1);
  if (len == -1) {
    perror("readlink");
    return NULL;
  }
  path[len] = '\0';
  return path;
}

int main(int argc, char* argv[]) {
  Counters* counters = malloc(sizeof(Counters));
  counters->head = NULL;

  pid_t pid = fork();
  if (pid == 0) {
    if (ptrace(PTRACE_TRACEME) < 0) {
      perror("ptrace failed in child");
      exit(1);
    }

    char* args[4] = {argv[1], argv[2], argv[3], NULL};
    if (execvp(argv[1], args) < 0) {
      perror("execvp failed");
      exit(1);
    }
  }

  struct user_regs_struct regs;
  int status;

  do {
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
      break;
    }

    if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
      perror("ptrace2 failed\n");
      exit(3);
    }

    if (regs.orig_rax == 1) {
      char* path = GetPath(pid, regs.rdi);
      if (regs.rax != -38) {
        increment(counters, path, regs.rdx);
      }
    }

    if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL) < 0) {
      perror("ptrace3 failed");
      exit(4);
    }
  } while (WIFSTOPPED(status));

  waitpid(pid, &status, 0);

  print(counters);

  return 0;
}
