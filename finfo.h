#ifndef __FINFO_H__
#define __FINFO_H__

typedef int ferr_t;
enum {
    BERR_OK = 1,
    BERR_CONNECT = 2,
    BERR_NOMEM = 3,
};

struct firmware_info {
    void (*printchar)(const char ch);
    int (*read_adc)();
    ferr_t (*http_request)(const char *host, int port, const char *method,
                           const char *path, const void *headers, const void *payload,
                           size_t payload_size, void *buf, size_t buf_size);
};


#endif
