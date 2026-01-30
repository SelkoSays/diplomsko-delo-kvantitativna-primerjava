#ifndef RANDOM_H
#define RANDOM_H

#include "../utils/common.h"
#include "../utils/list.h"

static inline Int clamp(Int min, Int max, Int num) {
    dbg_assert(min <= max);

    if (num < min) {return min;}
    if (num > max) {return max;}
    return num;
}

double uniform01(void);

Int next_uniform(Int min, Int max);
Int next_exponential(Int min, Int max, Float lambda);
Int next_powerlaw(Int min, Int max, Float alpha);

void random_seed(u64 seed);

Int next_random(Int min, Int max, Float param, Distribution distribution);

Int random_choice(const IntList* l);
Int random_wchoice(const IntList* l, const IntList* weights);

static inline bool coin_flip(void) { return (next_uniform(0, 2) % 2) == 1; }

#endif // RANDOM_H
