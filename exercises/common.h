#ifndef CACHEUTILS_H_INC
#define CACHEUTILS_H_INC

// For exercise 2
#define PROBE_SCALE 5493

// For exercise 1 and 3
#define PAGE_SIZE   0x1000
#define LINE_SIZE   64
#define CANDIDATE_A      2
#define CANDIDATE_B      15

#define IDX(X) (X*LINE_SIZE+2)
#define IDX_A IDX(CANDIDATE_A)
#define IDX_B IDX(CANDIDATE_B)

#endif /* CACHEUTILS_H_INC */