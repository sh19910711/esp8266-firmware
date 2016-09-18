#include <stddef.h>
#include <stdint.h>


// memcpy that does 32-bit aligned memory accesses
void aligned_memcpy(void *to, const void *from, size_t size) {
    uint32_t *dst = (uint32_t *) to;
    uint32_t *src = (uint32_t *) from;
    int bitmask = sizeof(uint32_t) - 1;

    int di = (uintptr_t) dst & bitmask;
    int si = (uintptr_t) src & bitmask;
    dst = (uint32_t *) ((uintptr_t) dst & ~bitmask);
    src = (uint32_t *) ((uintptr_t) src & ~bitmask);

    uint32_t d_data = (di == 0)? 0 : *dst;
    uint32_t s_data = *src;
    src++;

    while (size > 0) {
        if (si > 3) {
            s_data = *src;
            src++;
            si = 0;
        }

        if (di > 3) {
            *dst = d_data;
            d_data = 0;
            dst++;
            di = 0;
        }
        d_data |= ((s_data >> (si * 8)) & 0xff) << (di * 8);
        si++;
        di++;
        size--;
    }

    if (di > 0) {
        uint32_t ranges[] = {0xff000000, 0xffff0000, 0xffffff00, 0x00000000};
        *dst = d_data | (*dst & ranges[di - 1]);
    }
}

#ifdef TEST_ALIGNED_MEMCPY
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define VP(x, n) ((void *) ((uintptr_t) (x) + n))

const char aligned_one[] __attribute__((aligned(8)))   =  "Hello!####";
const char unaligned_one[] __attribute__((aligned(8))) = "+Great!####";
const char unaligned_one2[] __attribute__((aligned(8))) = "123THIS IS IT!####";
const char unaligned_one3[] __attribute__((aligned(8))) = "123456789abcdefFOO BAR BAZ:D####";

int main() {
    void *aligned_buf = malloc(128);
    void *unaligned_buf = (void *) ((uintptr_t) malloc(128) + 1);
    memset(aligned_buf,   '@', 16);
    memset(unaligned_buf, '@', 16);

    memset(aligned_buf,   '@', 16);
    aligned_memcpy(aligned_buf, aligned_one, 6);
    printf("buf='%s', src='%s', size=%lu\n", aligned_buf, aligned_one, sizeof(aligned_one));
    assert(!memcmp(aligned_buf, aligned_one, 6));

    memset(aligned_buf,   '@', 16);
    aligned_memcpy(aligned_buf, VP(unaligned_one, 1), 6);
    printf("buf='%s', src='%s', size=%lu\n", aligned_buf, unaligned_one, sizeof(unaligned_one));
    assert(!memcmp(aligned_buf, VP(unaligned_one, 1), 6));

    memset(unaligned_buf,   '@', 16);
    aligned_memcpy(unaligned_buf, VP(unaligned_one2, 3), 11);
    printf("buf='%s', src='%s', size=%lu\n", unaligned_buf, unaligned_one2, sizeof(unaligned_one2));
    assert(!memcmp(unaligned_buf, VP(unaligned_one2, 3), 11));

    memset(aligned_buf,   '@', 16);
    aligned_memcpy(aligned_buf, VP(unaligned_one3, 0xf), 13);
    printf("buf='%s', src='%s', size=%lu\n", aligned_buf, unaligned_one3, sizeof(unaligned_one3));
    assert(!memcmp(aligned_buf, VP(unaligned_one3, 0xf), 13));
}

#endif // TEST_ALIGNED_MEMCPY
