#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct StorageItem {
  char key[PATH_MAX];
  char value[PATH_MAX];
  struct StorageItem* next;
} StorageItem;

typedef struct Storage {
  struct StorageItem* head;
} Storage;

StorageItem* find(Storage* storage, char* key) {
  StorageItem* current = storage->head;
  while (current != NULL) {
    if (strncmp(current->key, key, PATH_MAX) == 0) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

void set(Storage* storage, char* key, char* value) {
  StorageItem* element = find(storage, key);
  if (element == NULL) {
    element = malloc(sizeof(StorageItem));
    strncpy(element->key, key, PATH_MAX - 1);
    element->next = storage->head;
    storage->head = element;
  }
  strncpy(element->value, value, PATH_MAX - 1);
}

char* get(Storage* storage, char* key) {
  StorageItem* element = find(storage, key);
  if (element == NULL) {
    return "";
  } else {
    return element->value;
  }
}

void process_client(Storage* storage, int fd) {
  char buffer[128];
  int res = read(fd, buffer, sizeof(buffer));
  if (res < 0) {
    perror("read failed");
    return;
  }

  buffer[res] = '\0';

  static const char* kGet = "get";
  static const size_t kMethodLen = 3;

  if (res < kMethodLen + 1) {
    return;
  }

  if (memcmp(buffer, kGet, strlen(kGet)) == 0) {
    // GET
    char* value = get(storage, buffer + kMethodLen + 1);
    if (dprintf(fd, "%s\n", value) < 0) {
      perror("write1 failed");
      return;
    }

  } else {
    // SET
    char key[1024];
    size_t i = 0;
    for (; buffer[kMethodLen + 1 + i] != ' '; ++i) {
      key[i] = buffer[kMethodLen + 1 + i];
    }

    key[i] = '\0';

    char value[1024];

    ++i;

    strcpy(value, buffer + kMethodLen + 1 + i);

    set(storage, key, value);
  }
}

int main(int argc, char* argv[]) {
  Storage* storage = malloc(sizeof(Storage));
  storage->head = NULL;
  int server_port = atoi(argv[1]);

  int epoll_fd = epoll_create1(0);

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("socket");
    exit(1);
  }

  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(server_port),
      .sin_addr.s_addr = INADDR_ANY,
  };

  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &(int){1},
             sizeof(int));

  if (bind(socket_fd, (struct sockaddr*)(&addr), sizeof(addr)) < 0) {
    perror("bind");
    exit(5);
  }

  if (listen(socket_fd, SOMAXCONN) < 0) {
    perror("listen");
    exit(7);
  }

  int flags = fcntl(socket_fd, F_GETFD);
  flags |= O_NONBLOCK;
  fcntl(socket_fd, F_SETFD, flags);

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = socket_fd;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev);

  struct epoll_event events[10];
  for (;;) {
    int n = epoll_wait(epoll_fd, events, 10, -1);

    for (int i = 0; i < n; ++i) {
      if (events[i].data.fd == socket_fd) {
        int client_fd = accept(socket_fd, NULL, NULL);
        if (client_fd < 0) {
          perror("accept");
          exit(6);
        }

        int flags = fcntl(client_fd, F_GETFD);
        flags |= O_NONBLOCK;
        fcntl(client_fd, F_SETFD, flags);

        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
      } else {
        process_client(storage, events[i].data.fd);
      }
    }
  }

  shutdown(socket_fd, SHUT_RDWR);
  close(socket_fd);

  return 0;
}
