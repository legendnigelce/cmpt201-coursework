// client.c
/*
Questions to answer at top of client.c:
(You should not need to change the code in client.c)
1. What is the address of the server it is trying to connect to (IP address and
port number). IP address: 127.0.0.1 Port number: 8000

2. Is it UDP or TCP? How do you know?
It is TCP because socket() has the type SOCK_STREAM. As well, it uses connect(),
and UDP doesn't use that.

3. The client is going to send some data to the server. Where does it get this
data from? How can you tell in the code? It gets the data from the user. You can
tell because it reads the data from STDIN_FILENO, and writes that data to the
file.

4. How does the client program end? How can you tell that in the code?
The client program ends when there is nothing left to read. You can tell that in
the code because the condition in the loop is that the amount of bytes read has
to be more than 1. If there is nothing left to read then the loop ends and the
program ends.
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8000
#define BUF_SIZE 64
#define ADDR "127.0.0.1"

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main() {
  struct sockaddr_in addr;
  int sfd;
  ssize_t num_read;
  char buf[BUF_SIZE];

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, ADDR, &addr.sin_addr) <= 0) {
    handle_error("inet_pton");
  }

  int res = connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (res == -1) {
    handle_error("connect");
  }

  while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 1) {
    if (write(sfd, buf, num_read) != num_read) {
      handle_error("write");
    }
    printf("Just sent %zd bytes.\n", num_read);
  }

  if (num_read == -1) {
    handle_error("read");
  }

  close(sfd);
  exit(EXIT_SUCCESS);
}

// server.c
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 64
#define PORT 8000
#define LISTEN_BACKLOG 32

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

// Shared counters for: total # messages, and counter of clients (used for
// assigning client IDs)
int total_message_count = 0;
int client_id_counter = 1;

// Mutexs to protect above global state.
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_id_mutex = PTHREAD_MUTEX_INITIALIZER;

struct client_info {
  int cfd;
  int client_id;
};

void *handle_client(void *arg) {
  struct client_info *client = (struct client_info *)arg;

  printf("New client created! ID %d on socket FD %d\n", client->client_id,
         client->cfd);
  char buf[BUF_SIZE];
  int num_read;
  while ((num_read = read(client->cfd, buf, BUF_SIZE)) > 0) {
    buf[num_read - 1] = '\0';
    pthread_mutex_lock(&count_mutex);
    total_message_count++;
    printf("Msg #\t%d; Client ID %d: %s\n", total_message_count,
           client->client_id, buf);
    pthread_mutex_unlock(&count_mutex);
  }

  close(client->cfd);
  printf("Ending thread for client %d\n", client->client_id);

  return NULL;
}

int main() {
  struct sockaddr_in addr;
  int sfd;

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
    handle_error("bind");
  }

  if (listen(sfd, LISTEN_BACKLOG) == -1) {
    handle_error("listen");
  }

  for (;;) {
    int cfd = accept(sfd, NULL, NULL);
    if (cfd > 0) {
      pthread_t t;
      struct client_info *new =
          (struct client_info *)malloc(sizeof(struct client_info));
      new->cfd = cfd;
      pthread_mutex_lock(&client_id_mutex);
      new->client_id = client_id_counter;
      client_id_counter++;
      pthread_mutex_unlock(&client_id_mutex);
      pthread_create(&t, NULL, handle_client, (void *)new);
      pthread_detach(t);
    }
  }

  if (close(sfd) == -1) {
    handle_error("close");
  }

  return 0;
}
