#include <stdio.h>

#include <mimalloc.h>
#include <uv.h>

#include "server.h"
#include "request.h"

#define DEFAULT_PORT 3000

static uv_loop_t *default_loop;

static int _request_handler(request_t *req)
{
  printf("Method: %s\n", llhttp_method_name(llhttp_get_method(req->_parser)));
  printf("URL: %s\n", req->url->data);
  printf("Headers:\n");

  hashtable_it_t it = ht_iterator(req->headers);
  while (hti_next(&it))
  {
    ht_entry_t const *entry = hti_get(&it);

    printf("\t%s: %s\n", entry->key->data, STRING(entry->data)->data);
  }

  printf("Body (%zu): %.*s\n", req->body_size, (int)req->body_size, req->body);

  return 0;
}

int main(int argc, char const *argv[])
{
  uv_replace_allocator(mi_malloc, mi_realloc, mi_calloc, mi_free);
  default_loop = uv_default_loop();

  init_request();

  server_t *server = server_configure("0.0.0.0", "::", DEFAULT_PORT, _request_handler, default_loop);
  if (server == NULL)
    return 1;

  if (!server_listen(server, SOMAXCONN))
    return 1;

  return uv_run(default_loop, UV_RUN_DEFAULT);
}
