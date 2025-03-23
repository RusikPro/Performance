#ifndef __UTILS__CACHE_H__
#define __UTILS__CACHE_H__

#include <cstddef>

/*----------------------------------------------------------------------------*/

// Flush the cache by reading/writing a large memory block.
void flushCache ()
{
    // Allocate a buffer that's much larger than the L1/L2/L3 caches.
    // Adjust the size as needed (here we use 50 MB).
    const std::size_t size = 50 * 1024 * 1024;
    volatile char* buffer = new volatile char[ size ];

    // Touch the buffer in steps (cache line size is usually 64 bytes).
    for (size_t i = 0; i < size; i += 64) {
        buffer[i] = static_cast<char>(i % 256);
    }

    // Prevent the compiler from optimizing the loop away.
    asm volatile("" : : "r"(buffer) : "memory");

    // Clean up.
    delete[] buffer;
}

/*----------------------------------------------------------------------------*/

#endif // __UTILS__CACHE_H__
