// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <pthread.h>

#define PORT	8080
#define MAXLINE 1024

struct _NetworkContext {
  int sock;
  struct sockaddr_in server_addr;
};

typedef struct _NetworkContext NetworkContext;

NetworkContext *network_init(uint16_t port, in_addr_t server_addr)
{
  NetworkContext *ctx = malloc(sizeof(NetworkContext));

  if (!ctx) goto _error1;

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    perror("socket creation failed");
    goto _error2;
  }
  ctx->sock = sock;

  memset(&ctx->server_addr, 0, sizeof(ctx->server_addr));

  ctx->server_addr.sin_family = AF_INET;
  ctx->server_addr.sin_port = htons(port);
  ctx->server_addr.sin_addr.s_addr = htonl(server_addr);

  return ctx;

_error2:
  free(ctx);
_error1:
  return NULL;
}


ssize_t network_send(const NetworkContext *network_context,
                     const char *buffer,
                     size_t buffer_size)
{
  ssize_t len = sendto(network_context->sock,
                       (const char *) buffer,
                       buffer_size,
                       0,
                       (const struct sockaddr *) &network_context->server_addr,
                       sizeof(network_context->server_addr));
  if (len != buffer_size) {
    perror("Can't send packet");
  }
  return len;
}

ssize_t network_recv(const NetworkContext *network_context,
                     char *buffer,
                     size_t buffer_size)
{
  struct sockaddr_in servaddr;
  socklen_t len = sizeof(servaddr);

  memcpy(&servaddr, &network_context->server_addr, sizeof(servaddr));

  ssize_t n = recvfrom(network_context->sock,
                       (char *) buffer,
                       buffer_size,
                       0,
                       (struct sockaddr *) &servaddr,
                       &len);

  return n;
}


int main() {
    char buffer[MAXLINE];

    NetworkContext *network_context = network_init(PORT, INADDR_LOOPBACK);

    if (!network_context) {
      printf("Ошибка инициализации сети\n");
      return -1;
    }

    int is_writer;
    printf("Читатель - 0, писатель - 1: ");
    scanf("%d", &is_writer);

    // Сообщаем серверу о нашей роли
    uint8_t role_byte = !!is_writer;
    network_send(network_context, (char *) &role_byte, 1);

    while (1) {
      if (is_writer) {
        printf("Enter your message: ");
        fgets(buffer, MAXLINE - 1, stdin);
        buffer[MAXLINE - 1] = '\0';

        network_send(network_context, buffer, strlen(buffer));
      } else {
        int n = network_recv(network_context, buffer, MAXLINE - 1);
        if (n != -1) {
          buffer[n] = '\0';
          printf("Server : %s\n", buffer);
        }
      }
    }
    return 0;
}
