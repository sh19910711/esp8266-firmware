#ifndef __HTTP_H__
#define __HTTP_H__

#include "finfo.h"

ferr_t do_http_request(const char *host, int port, const char *method,
                       const char *path, const void *headers, const void *payload,
                       size_t payload_size, uint8_t **buf, size_t *buf_size,
                       int *offset);
ferr_t http_request(const char *host, int port, const char *method,
                    const char *path, const void *headers, const void *payload,
                    size_t payload_size, void *buf, size_t buf_size);
ferr_t xeof_http_request(const char *host, int port, const char *method,
                         const char *path, const void *headers, const void *payload,
                         size_t payload_size, void *buf, size_t buf_size);

#endif
