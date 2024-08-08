// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT	8080
#define MAXLINE 1024

struct _NetworkContext {
  int sock;
  struct sockaddr_in addr;
};

typedef struct _NetworkContext NetworkContext;


NetworkContext *network_init(uint16_t port)
{
  NetworkContext *ctx = malloc(sizeof(NetworkContext));
  if (!ctx) goto _error1;

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    perror("socket creation failed");
    goto _error2;
  }
  ctx->sock = sock;

  memset(&ctx->addr, 0, sizeof(ctx->addr));

  ctx->addr.sin_family = AF_INET;
  ctx->addr.sin_addr.s_addr = INADDR_ANY;
  ctx->addr.sin_port = htons(port);

  int bind_status = bind(ctx->sock,
                         (const struct sockaddr *) &ctx->addr,
                         sizeof(ctx->addr));
  if (bind_status < 0)
  {
    perror("bind failed");
    goto _error3;
  }

  return ctx;

_error3:
  close(ctx->sock);
_error2:
  free(ctx);
_error1:
  return NULL;
}

void network_fini(NetworkContext *ctx)
{
  if (ctx) {
    if (ctx->sock >= 0) close(ctx->sock);
    free(ctx);
  }
}

int main() {
    char buffer[MAXLINE];
    struct sockaddr_in cliaddr, reader_addr;

    NetworkContext *network_context = network_init(PORT);
    if (!network_context) {
      printf("Ошибка инициализации сети\n");
      return -1;
    }

    int len, n;

    memset(&cliaddr, 0, sizeof(cliaddr));

    while (1) {
      len = sizeof(cliaddr); //len is value/resuslt

      n = recvfrom(network_context->sock,
                   (char *) buffer,
                   MAXLINE,
                   MSG_WAITALL,
                   (struct sockaddr *) &cliaddr,
                   &len);

      if (n == 1) {
        int is_writer = buffer[0];
        if (!is_writer) {
          printf("Задан новый читатель!\n");
          memcpy(&reader_addr, &cliaddr, sizeof(cliaddr));
        }
        continue;
      }

      buffer[n] = '\0';

      printf("Client : %s\n", buffer);

      sendto(network_context->sock,
             (const char *)buffer,
             strlen(buffer),
             0,
             (const struct sockaddr *) &reader_addr,
             sizeof(reader_addr));
    }

    return 0;
}
