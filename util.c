/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 nghfp9wa7bzq@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/*
  This file contains helper functions for pico-dirtyJtag.
*/

#include "util.h"


// Debug only
#if ( PDJ_DEBUG_LEVEL > 0 )
    void print_buffer(const uint8_t* buffer, const uint16_t bytes)
    {
        // Use a terminating null character to denote an empty string.
        char empty_str = '\0';
        print_buffer_pf(buffer, bytes, &empty_str);
    }

    // Same as the above function,
    // but adds an option for a prefix.
    // It is used to distinguish between functions
    // running on different CPU cores in a multicore application.
    void print_buffer_pf(const uint8_t* buffer, const uint16_t bytes, const char* prefix)
    {
        for (uint16_t i = 0; i < bytes; i++) {
            printf("%s#%d: %02X\n", prefix, i, buffer[i]);
        }
    }
#endif
