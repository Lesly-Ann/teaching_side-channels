#include <stdint.h>
#include <x86intrin.h>

#include "common.h"

uint8_t __attribute__((aligned(PAGE_SIZE))) probe_buffer[PAGE_SIZE] = {0};

/* Secret candidate can be 'a' or 'b' */
volatile uint8_t secret_candidate = 'a';

void cast_vote(uint8_t candidate)
{
    if (candidate == 'a')
        probe_buffer[IDX_A]++;
    else
        probe_buffer[IDX_B]++;
}

void run_victim(void)
{
    cast_vote(secret_candidate);
}