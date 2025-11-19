#include <stdint.h>
#include <x86intrin.h>
#include <stdio.h>
#include "common.h"

extern void flush(void* addr);

// Secret array will be located just after public in memory
uint8_t public_arr[16] __attribute__((section(".data.public_arr"))) = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t secret[] __attribute__((section(".data.secret"))) = "1597";
size_t secret_len = sizeof(secret) - 1;
uint8_t shared_array[256 * PROBE_SCALE];

uint8_t dummy = 0;
volatile uint8_t temp = 0;

volatile size_t public_size = (size_t) sizeof(public_arr);
volatile size_t *public_size_ptr = &public_size;
volatile size_t **public_size_ptr_ptr = &public_size_ptr;
volatile size_t ***public_size_ptr_ptr_ptr = &public_size_ptr_ptr;

void flush_sizes(void) {
    flush((void*)&public_size);
    flush((void*)&public_size_ptr);
    flush((void*)&public_size_ptr_ptr);
    flush((void*)&public_size_ptr_ptr_ptr);
}

__attribute__((noinline))
void warm_up_victim(size_t idx) {
    __asm__ volatile ("" ::: "memory");
    temp = secret[idx];
}

__attribute__((noinline))
void run_victim(size_t idx) {
    __asm__ volatile ("" ::: "memory");

    if (idx < ***public_size_ptr_ptr_ptr) {
        uint8_t value = public_arr[idx];
        shared_array[PROBE_INDEX(value)]++;
        dummy ^= value;
    }
}