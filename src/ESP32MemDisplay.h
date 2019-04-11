/*
 * SmartMatrix Library - ESP32 Memory Info
 *
 * Copyright (c) 2018 Louis Beaudoin (Pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _ESP32MEMDISPLAY_H_
#define _ESP32MEMDISPLAY_H_

static void show_esp32_heap_mem(const char *str=NULL) {
    if (str) {
    	printf("%s: %6d bytes total, %6d bytes largest free block\n", str, heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
    } else {
    	printf("Heap/32-bit Memory Available: %6d bytes total, %6d bytes largest free block\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
    }
}


static void show_esp32_dma_mem(const char *str=NULL) {
    if (str) {
        printf("%s: %6d bytes total, %6d bytes largest free block\n", str, heap_caps_get_free_size(MALLOC_CAP_DMA), heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
    } else {
        printf("8-bit/DMA Memory Available  : %6d bytes total, %6d bytes largest free block\n", heap_caps_get_free_size(MALLOC_CAP_DMA), heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
    }
}

static void show_esp32_all_mem(void) {
    show_esp32_heap_mem();
    // 32bit always seems to output the same value as Heap
    // printf("32-bit Memory Available: %d bytes total, %d bytes largest free block\n", heap_caps_get_free_size(MALLOC_CAP_32BIT), heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));

    show_esp32_dma_mem();
    // 8-bit always seems to output the same value as DMA
    //printf("8-bit Accessible Memory Available: %d bytes total, %d bytes largest free block\n", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}

#endif
