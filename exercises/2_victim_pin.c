#include <stdint.h>
#include <x86intrin.h>
#include <stdio.h>
#include "common.h"

// Secret array will be located just after public in memory
uint8_t public_arr[16] __attribute__((section(".data.public_arr"))) = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t secret[] __attribute__((section(".data.secret"))) = "1597";
size_t secret_len = sizeof(secret) - 1;
uint8_t shared_array[256 * PROBE_SCALE];

void run_victim(size_t idx) {
    __asm__ volatile ("" ::: "memory");
    // Notice the out of bound access here
    // It can be used to encode secret in the cache
    uint8_t v = public_arr[idx];
    shared_array[v * PROBE_SCALE]++;
}