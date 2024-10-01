#if !defined(_SERVER_H_)
#define _SERVER_H_

#include <stdbool.h>

#include <uv.h>

#include "request.h"

typedef struct server
{
  uv_tcp_t *tcp4, *tcp6;
  request_handler_f handler;
} server_t;

server_t *server_configure(
  char const *ipv4,
  char const *ipv6,
  int port,
  request_handler_f handler,
  uv_loop_t *loop);
void server_destroy(server_t *server);
bool server_listen(server_t *server, int backlog);

#endif // _SERVER_H_
