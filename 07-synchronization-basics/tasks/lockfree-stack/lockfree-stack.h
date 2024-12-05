#pragma once

#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_BACKOFF 1000000

typedef struct node {
  uintptr_t value;
  struct node *next;
} node_t;

typedef struct lfstack {
  node_t *top;
} lfstack_t;

void exponential_backoff(int attempt) {
    unsigned int backoff_time = rand() % (1 << attempt);
    if (backoff_time > MAX_BACKOFF) {
        backoff_time = MAX_BACKOFF;
    }
    usleep(backoff_time);
}

int lfstack_init(lfstack_t *stack) {
  stack->top = NULL;
  static size_t i = 0;
  if (!i++) {
    srand(time(NULL));
  }
  return 0;  // success
}

int lfstack_push(lfstack_t *stack, uintptr_t value) {
  node_t *new = malloc(sizeof(node_t));
  int attempt = 0;
  while (1) {
    node_t *old = atomic_load(&stack->top);
    new->next = old;
    new->value = value;

    if (atomic_compare_exchange_weak(&stack->top, &old, new)) {
      break;
    }
    exponential_backoff(attempt++);
  }
  return 0;
}

int lfstack_pop(lfstack_t *stack, uintptr_t *value) {
  node_t *top = atomic_load(&stack->top);
  int attempt = 0;
  while (top != NULL) {
    if (atomic_compare_exchange_weak(&stack->top, &top, top->next)) {
      *value = top->value;
      free(top);
      return 0;
    }
    exponential_backoff(attempt++);
  }
  if (value != NULL) {
    *value = 0;
  }
  return 0;  // success
}

int lfstack_destroy(lfstack_t *stack) {
  size_t _;
  while (stack->top != NULL) {
    if (0 != lfstack_pop(stack, &_)) {
      perror("unexpected error during pop");
      return 1;
    }
  }
  return 0;  // success
}
