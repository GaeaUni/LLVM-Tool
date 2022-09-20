#import <CoreFoundation/CoreFoundation.h>
#import <malloc/malloc.h>
#include <cstddef>
#import <string>

std::string entry(const void *arg0) {
    const void *ptr = (const void *)arg0;
    void *zone = (void *)malloc_zone_from_ptr(ptr);
    auto size = (size_t)malloc_size(ptr);
    const char *name = (const char *)malloc_get_zone_name(zone);
    printf("zone = %s, size = %zu", name, size);
    zone;
}