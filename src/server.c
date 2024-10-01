#include "server.h"

#include <mimalloc.h>

static void _alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
  size_t good_size = mi_good_size(suggested_size);
  char *buffer = mi_malloc(good_size);
  if (buffer != NULL)
  {
    buf->base = buffer;
    buf->len = good_size;
  }
}

static void _close_cb(uv_handle_t *handle)
{
  delete_request_handler(handle->data);
  mi_free(handle);
}

static void _read_cb(uv_stream_t *stream, ssize_t nread, uv_buf_t const *buf)
{
  if (nread > 0)
  {
    request_t *req = stream->data;
    enum llhttp_errno err = llhttp_execute(req->_parser, buf->base, nread);

    if (err == HPE_OK)
    {
      printf("Parse success\n");
    }
    else
    {
      fprintf(stderr, "Parser error: %s %s\n", llhttp_errno_name(err), req->_parser->reason);
    }
  }

  // close connection anyways because it's a test yet
  uv_close((uv_handle_t *)stream, _close_cb);
  mi_free(buf->base);
}

static void _conn_cb(uv_stream_t *server, int status)
{
  if (status < 0)
  {
    fprintf(stderr, "Connection error %s\n", uv_strerror(status));
    return;
  }

  uv_tcp_t *client = mi_malloc(sizeof(uv_tcp_t));
  if (client == NULL)
  {
    fprintf(stderr, "Allocation error (tcp_client)\n");
    return;
  }

  server_t *app_server = server->data;
  request_t *req = create_request_handler((uv_stream_t *)client, app_server->handler);
  if (req == NULL)
  {
    mi_free(client);
    return;
  }
  client->data = req;

  int err = uv_tcp_init(server->loop, client);
  if (err != 0)
  {
    delete_request_handler(req);
    mi_free(client);
    fprintf(stderr, "error: %s\n", uv_strerror(err));
    return;
  }

  if (uv_accept(server, (uv_stream_t *)client) == 0)
  {
    uv_read_start((uv_stream_t *)client, _alloc_cb, _read_cb);
  }
  else
  {
    uv_close((uv_handle_t *)client, _close_cb);
  }
}

uv_tcp_t *_tcp_init(uv_loop_t *loop)
{
  uv_tcp_t *tcp = mi_malloc(sizeof(uv_tcp_t));
  if (tcp == NULL)
    return NULL;

  if (uv_tcp_init(loop, tcp))
  {
    mi_free(tcp);
    return NULL;
  }

  return tcp;
}

uv_tcp_t *_configure_tcp_ipv4(uv_loop_t *loop, char const *ipv4, int port)
{
  uv_tcp_t *tcp = _tcp_init(loop);
  if (tcp == NULL)
    return NULL;

  struct sockaddr_in addr;
  if (uv_ip4_addr(ipv4, port, &addr))
  {
    mi_free(tcp);
    return NULL;
  }

  if (uv_tcp_bind(tcp, (struct sockaddr const *)&addr, 0))
  {
    mi_free(tcp);
    return NULL;
  }

  return tcp;
}

uv_tcp_t *_configure_tcp_ipv6(uv_loop_t *loop, char const *ipv6, int port)
{
  uv_tcp_t *tcp = _tcp_init(loop);
  if (tcp == NULL)
    return NULL;

  struct sockaddr_in6 addr;
  if (uv_ip6_addr(ipv6, port, &addr))
  {
    mi_free(tcp);
    return NULL;
  }

  if (uv_tcp_bind(tcp, (struct sockaddr const *)&addr, 0))
  {
    mi_free(tcp);
    return NULL;
  }

  return tcp;
}

#define USE_LOOP_OR_DEFAULT(loop) (loop != NULL ? loop : uv_default_loop())

server_t *server_configure(char const *ipv4, char const *ipv6, int port, request_handler_f handler, uv_loop_t *loop)
{
  if (ipv4 == NULL && ipv6 == NULL)
    return NULL;

  server_t *server = mi_zalloc_small(sizeof(server_t));
  if (server == NULL)
    return NULL;

  if (ipv4 != NULL)
  {
    uv_tcp_t *tcp4 = _configure_tcp_ipv4(USE_LOOP_OR_DEFAULT(loop), ipv4, port);
    if (tcp4 == NULL)
    {
      mi_free(server);
      return NULL;
    }
    server->tcp4 = tcp4;
    tcp4->data = server;
  }

  if (ipv6 != NULL)
  {
    uv_tcp_t *tcp6 = _configure_tcp_ipv6(USE_LOOP_OR_DEFAULT(loop), ipv6, port);
    if (tcp6 == NULL)
    {
      mi_free(server->tcp4);
      mi_free(server);
      return NULL;
    }
    server->tcp6 = tcp6;
    tcp6->data = server;
  }

  server->handler = handler;

  return server;
}

void server_destroy(server_t *server)
{
  if (server == NULL)
    return;

  mi_free(server->tcp4);
  mi_free(server->tcp6);
  mi_free(server);
}

bool server_listen(server_t *server, int backlog)
{
  if (server->tcp4 == NULL && server->tcp6 == NULL)
    return false;

  if (server->tcp4 != NULL)
  {
    if (uv_listen((uv_stream_t *)server->tcp4, backlog, _conn_cb))
      return false;
  }

  if (server->tcp6 != NULL)
  {
    if (uv_listen((uv_stream_t *)server->tcp6, backlog, _conn_cb))
      return false;
  }

  return true;
}
