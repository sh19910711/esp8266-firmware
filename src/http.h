#ifndef __HTTP_H__
#define __HTTP_H__

#include "finfo.h"

int do_http_request(const char *method, const char *host, int port,
                    const char *path, const void *headers, size_t headers_size,
                    const void *payload, size_t payload_size,
                    uint8_t **buf, size_t *buf_size, size_t *resp_size,
                    int *offset, bool tls);
int _http_request(const char *method, const char *url, size_t url_size,
                  const void *headers, size_t headers_size,
                  const void *payload, size_t payload_size,
                  void *buf, size_t buf_size, size_t *resp_size);
int http_request(const char *method, String url, String headers,
                 const void *payload, size_t payload_size,
                 void *buf, size_t buf_size, size_t *resp_size);

#endif
