#include "cfp_mem.h"
#include <stdlib.h>
#include <string.h>

void *cfp_malloc(uint32_t size)
{
    return malloc(size);
}

void cfp_free(void *ptr)
{
    return free(ptr);
}

void *cfp_memcpy(void *dst, const void *src, uint32_t size)
{
    if ((NULL == dst) || (NULL == src))
    {
        return NULL;
    }
    return memcpy(dst, src, size);
}

void *cfp_memset(void *ptr, int32_t val, uint32_t size)
{
    if (NULL == ptr)
    {
        return NULL;
    }
    return memset(ptr, val, size);
}