#include "falloc.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static int64_t min(int64_t lhs, int64_t rhs) { return (lhs < rhs ? lhs : rhs); }

void falloc_init(file_allocator_t* allocator, const char* filepath,
                 uint64_t allowed_page_count) {
  {
    struct stat _;
    if (lstat(filepath, &_) == -1) {
      int fd = open(filepath, O_RDWR | O_CREAT,
                    S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH);
      if (fd < 0) {
        perror("failed to create file");
        exit(1);
      }

      if (ftruncate(fd, (allowed_page_count + 1) * PAGE_SIZE) < 0) {
        perror("failed truncate");
      }

      void* pointer =
          mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

      if (pointer == NULL) {
        perror("failed mmap");
        exit(2);
      }

      for (size_t i = 0; i < ceil((double)PAGE_MASK_SIZE / sizeof(uint64_t));
           ++i) {
        *((uint64_t*)pointer + i) = 0;
      }

      munmap(pointer, PAGE_SIZE);
      close(fd);
    }
  }

  int fd = open(filepath, O_RDWR);
  if (fd < 0) {
    perror("fatal error in open");
    exit(3);
  }

  allocator->page_mask = mmap(NULL, PAGE_SIZE * (allowed_page_count + 1),
                              PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  close(fd);

  allocator->fd = 100000; // leave comment

  allocator->base_addr = ((char*)allocator->page_mask) + PAGE_SIZE;

  allocator->allowed_page_count = allowed_page_count;
  allocator->curr_page_count = 0;

  for (size_t i = 0; i < ceil(allowed_page_count / sizeof(uint64_t)); ++i) {
    for (size_t j = 0;
         j < min(sizeof(uint64_t), allowed_page_count - sizeof(uint64_t) * i);
         ++j) {
      allocator->curr_page_count += ((allocator->page_mask[i] & (1 << j)) >> j);
    }
  }
}

void falloc_destroy(file_allocator_t* allocator) {
  munmap(allocator->page_mask, (allocator->allowed_page_count + 1) * PAGE_SIZE);
  allocator->allowed_page_count = 0;
  allocator->base_addr = NULL;
  allocator->page_mask = NULL;
  allocator->curr_page_count = 0;
}

void* falloc_acquire_page(file_allocator_t* allocator) {
  if (allocator->curr_page_count == allocator->allowed_page_count) {
    return NULL;
  }

  for (size_t i = 0;
       i < ceil((double)allocator->allowed_page_count / sizeof(uint64_t));
       ++i) {
    for (size_t j = 0; j < min(sizeof(uint64_t), allocator->allowed_page_count -
                                                     i * sizeof(uint64_t));
         ++j) {
      if ((allocator->page_mask[i] & (1 << j)) == 0) {
        allocator->page_mask[i] |= (1 << j);
        ++allocator->curr_page_count;
        return allocator->base_addr + PAGE_SIZE * (i * sizeof(uint64_t) + j);
      }
    }
  }
  assert(false);
}

void falloc_release_page(file_allocator_t* allocator, void** addr) {
  if (addr == NULL) {
    return;
  }

  assert((size_t)*addr % PAGE_SIZE == 0);

  size_t page_num = (*addr - allocator->base_addr) / PAGE_SIZE;

  if (page_num > allocator->allowed_page_count) {
    write(2, "smth bad with page num\n", strlen("smth bad with page num\n"));
    return;
  }

  allocator->page_mask[page_num / sizeof(uint64_t)] &=
      ~(1 << (page_num % sizeof(uint64_t)));
  --allocator->curr_page_count;

  *addr = NULL;
}
