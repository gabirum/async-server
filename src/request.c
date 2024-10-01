#include "request.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <mimalloc.h>

static llhttp_settings_t _parser_settings;

static int _url_cb(llhttp_t *parser, const char *at, size_t length)
{
  request_t *req = parser->data;

  if (req->url == NULL)
  {
    req->url = string_new(at, length);
    if (req->url == NULL)
    {
      return -1;
    }
  }
  else
  {
    if (!string_cstr_concatn(req->url, at, length))
    {
      return -1;
    }
  }

  return 0;
}

static int _header_field_cb(llhttp_t *parser, const char *at, size_t length)
{
  request_t *req = parser->data;

  if (req->_hk == NULL)
  {
    req->_hk = string_new(at, length);
    if (req->_hk == NULL)
    {
      return -1;
    }
  }
  else
  {
    if (!string_cstr_concatn(req->_hk, at, length))
    {
      return -1;
    }
  }

  return 0;
}

static int _header_field_complete_cb(llhttp_t *parser)
{
  request_t *req = parser->data;

  ht_entry_t *entry = ht_get(req->headers, req->_hk);
  if (entry != NULL)
  {
    string_delete(req->_hk);
    req->_hk = entry->key;
  }

  return 0;
}

static int _header_value_cb(llhttp_t *parser, const char *at, size_t length)
{
  request_t *req = parser->data;

  if (req->_hd == NULL)
  {
    req->_hd = string_new(at, length);
    if (req->_hd == NULL)
    {
      return -1;
    }
  }
  else
  {
    if (!string_cstr_concatn(req->_hd, at, length))
    {
      return -1;
    }
  }

  return 0;
}

static int _header_value_complete_cb(llhttp_t *parser)
{
  request_t *req = parser->data;

  if (req->_hk == NULL || req->_hd == NULL)
  {
    return -1;
  }

  ht_entry_t *entry = ht_get(req->headers, req->_hk);
  if (entry == NULL)
  {
    if (!ht_set(req->headers, req->_hk, req->_hd))
    {
      return -1;
    }
  }
  else
  {
    if (!string_concat(entry->data, req->_hd))
    {
      return -1;
    }
    string_delete(req->_hd);
  }

  req->_hk = NULL;
  req->_hd = NULL;

  return 0;
}

static int _headers_cb(llhttp_t *parser)
{
  switch (llhttp_get_method(parser))
  {
  case HTTP_HEAD:
  case HTTP_GET:
    return 1;
  default:
    return 0;
  }
}

static int _body_cb(llhttp_t *parser, const char *at, size_t length)
{
  request_t *req = parser->data;

  if (req->body == NULL)
  {
    req->body = mi_malloc(length);
    if (req->body == NULL)
    {
      return -1;
    }
  }
  else
  {
    char *new_body_space = mi_realloc(req->body, req->body_size + length);
    if (new_body_space == NULL)
    {
      return -1;
    }
    req->body = new_body_space;
  }
  memcpy(req->body + req->body_size, at, length);
  req->body_size += length;

  return 0;
}

static int _complete_cb(llhttp_t *parser)
{
  request_t *req = parser->data;

  return req->_handle_request(req);
}

void init_request()
{
  llhttp_settings_init(&_parser_settings);
  _parser_settings.on_url = _url_cb;
  _parser_settings.on_header_field = _header_field_cb;
  _parser_settings.on_header_field_complete = _header_field_complete_cb;
  _parser_settings.on_header_value = _header_value_cb;
  _parser_settings.on_header_value_complete = _header_value_complete_cb;
  _parser_settings.on_headers_complete = _headers_cb;
  _parser_settings.on_body = _body_cb;
  _parser_settings.on_message_complete = _complete_cb;
}

request_t *create_request_handler(uv_stream_t *stream, request_handler_f handler)
{
  request_t *req = mi_zalloc_small(sizeof(request_t));
  if (req == NULL)
    return NULL;

  req->_parser = mi_malloc(sizeof(llhttp_t));
  if (req->_parser == NULL)
  {
    mi_free(req);
    return NULL;
  }
  llhttp_init(req->_parser, HTTP_REQUEST, &_parser_settings);
  req->_parser->data = req;

  req->headers = ht_new(30, .75f);
  if (req->headers == NULL)
  {
    mi_free(req->_parser);
    mi_free(req);
    return NULL;
  }

  req->_stream = stream;
  req->_handle_request = handler;

  return req;
}

void delete_request_handler(request_t *req)
{
  if (req == NULL)
    return;

  mi_free(req->body);
  string_delete(req->url);
  string_delete(req->_hk);
  string_delete(req->_hd);
  ht_delete(req->headers, (ht_cleanup_f)string_delete);
  mi_free(req->_parser);
  mi_free(req);
}
