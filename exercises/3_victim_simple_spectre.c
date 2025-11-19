#include <stdint.h>
#include <x86intrin.h>

#include "common.h"

extern void flush(void* addr);

#define SECRET_CANDIDATE CANDIDATE_B
uint8_t __attribute__((aligned(PAGE_SIZE))) probe_buffer[PAGE_SIZE] = {0};

volatile uint8_t authenticated = 0;

static inline uint8_t is_authenticated() {
    return authenticated;
}

static inline void authenticate() {
    authenticated = 1;
}

static inline void deauthenticate() {
    authenticated = 0;
    // Not leaving traces in cache
    flush(&probe_buffer[IDX_A]);
    flush(&probe_buffer[IDX_B]);
}

int cast_vote()
{
    uint8_t auth = is_authenticated();
    // Only cast vote if the victim is authenticated
    // But can we bypass this check using Spectre?
    if (auth) {
        probe_buffer[IDX(SECRET_CANDIDATE)]++;
        return 0;
    }
    return -1;
}

/* You can only call this function in your exploit */
void run_victim(void)
{
    authenticate();
    cast_vote();
    deauthenticate();
}