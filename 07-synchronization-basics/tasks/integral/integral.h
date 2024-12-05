#pragma once

#include <fcntl.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "wait.h"

typedef double field_t;

typedef field_t func_t(field_t);

typedef struct thread_arg {
  func_t *func;
  field_t left_bound;
  field_t right_bound;
  _Atomic(uint32_t) complete;
} thread_arg_t;

typedef struct par_integrator {
  _Atomic(double) result;
  pthread_t *threads;
  size_t threads_num;
  thread_arg_t *thread_args;
  atomic_bool need_destroy;
  _Atomic(uint32_t) ready_threads_cnt;
  _Atomic(uint32_t) start_calc_cnt;
} par_integrator_t;

typedef struct thread_args {
  par_integrator_t *integrator;
  thread_arg_t *args;
} thread_args_t;

double cacl_pthread(func_t *func, field_t left_bound, field_t right_bound) {
  static const size_t kIntervalCnt = 10000;
  double res = 0;

  const double interval_length = (right_bound - left_bound) / kIntervalCnt;

  for (size_t i = 0; i < kIntervalCnt; ++i) {
    double left_x = left_bound + interval_length * i;

    res += func(left_bound + interval_length * (i + 0.5)) * interval_length;
  }

  return res;
}

void atomic_add(_Atomic(double) *atomic_var, double increment) {
  double current_value, new_value;
  do {
    current_value = atomic_load(atomic_var);
    new_value = current_value + increment;
  } while (
      !atomic_compare_exchange_weak(atomic_var, &current_value, new_value));
}

void *do_work(void *args) {
  thread_args_t *arg = args;
  do {
    atomic_wait(&arg->args->complete, 1);

    if (atomic_load(&arg->integrator->need_destroy)) {
      break;
    }

    if (atomic_load(&arg->args->complete)) {
      continue;
    }

    atomic_fetch_add(&arg->integrator->start_calc_cnt, 1);

    double res = cacl_pthread(arg->args->func, arg->args->left_bound,
                              arg->args->right_bound);

    atomic_add(&arg->integrator->result, res);

    atomic_store(&arg->args->complete, 1);

    atomic_fetch_add(&arg->integrator->ready_threads_cnt, 1);

    atomic_notify_one(&arg->integrator->ready_threads_cnt);
  } while (1);

  free(args);
  return NULL;
}

int par_integrator_init(par_integrator_t *self, size_t threads_num) {
  self->threads_num = threads_num;

  self->thread_args = malloc(sizeof(thread_arg_t) * threads_num);

  self->need_destroy = false;

  self->threads = malloc(threads_num * sizeof(pthread_t));

  for (size_t i = 0; i < self->threads_num; ++i) {
    thread_args_t *data = malloc(sizeof(thread_args_t));

    data->integrator = self;
    data->args = self->thread_args + i;

    self->thread_args[i].complete = 1;

    pthread_create(&self->threads[i], NULL, do_work, data);
  }

  return 0;
}

int par_integrator_start_calc(par_integrator_t *self, func_t *func,
                              field_t left_bound, field_t right_bound) {
  self->result = 0;
  self->ready_threads_cnt = 0;
  self->start_calc_cnt = 0;

  for (size_t i = 0; i < self->threads_num; ++i) {
    double left =
        left_bound + (right_bound - left_bound) / self->threads_num * i;
    double right =
        left_bound + (right_bound - left_bound) / self->threads_num * (i + 1);

    self->thread_args[i].func = func;
    self->thread_args[i].left_bound = left;
    self->thread_args[i].right_bound = right;
    atomic_store(&self->thread_args[i].complete, 0);
  }

  while (atomic_load(&self->start_calc_cnt) < self->threads_num) {
    for (size_t i = 0; i < self->threads_num; ++i) {
      atomic_notify_one(&self->thread_args[i].complete);
    }
  }

  return 0;
}

int par_integrator_get_result(par_integrator_t *self, field_t *result) {
  uint32_t last = atomic_load(&self->ready_threads_cnt);
  while (last != self->threads_num) {
    atomic_wait(&self->ready_threads_cnt, last);
    last = atomic_load(&self->ready_threads_cnt);
  }

  *result = atomic_load(&self->result);
  return 0;
}

int par_integrator_destroy(par_integrator_t *self) {
  self->need_destroy = true;

  for (size_t i = 0; i < self->threads_num; ++i) {
    atomic_store(&self->thread_args[i].complete, 0);
    atomic_notify_one(&self->thread_args[i].complete);
    pthread_join(self->threads[i], NULL);
  }

  free(self->threads);
  free(self->thread_args);

  return 0;
}
