#if !defined(_REQUEST_H_)
#define _REQUEST_H_

#include <llhttp.h>
#include <uv.h>

#include "collections/string.h"
#include "collections/hashtable.h"

typedef struct request
{
  uv_stream_t *_stream;
  int (*_handle_request)(struct request *req);
  llhttp_t *_parser;
  hashtable_t *headers;
  string_t *url, *_hk, *_hd;
  char *body;
  size_t body_size;
} request_t;

typedef int (*request_handler_f)(request_t *req);

void init_request();

request_t *create_request_handler(uv_stream_t *stream, request_handler_f handler);
void delete_request_handler(request_t *req);

#endif // _REQUEST_H_
