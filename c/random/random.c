#include "random.h"

#include <math.h>

static u64 random_state = 0x853c49e6748fea9bULL;

static u64 splitmix64(u64 x) {
    uint64_t z = x + 0x9e3779b97f4a7c15ULL;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static u64 xs64star(void) {
    u64 x = random_state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    random_state = x;
    return x * 2685821657736338717ULL;
}

void random_seed(uint64_t seed) {
    if (seed == 0) {
        seed = (u64)time(NULL) ^ (uintptr_t)&seed;
    }
    random_state = splitmix64(seed);
    if (random_state == 0)
        random_state = 0x9e3779b97f4a7c15ULL;
}

double uniform01(void) {
    u64 r = xs64star() >> 11;
    return (double)r * (1.0 / (double)(1ULL << 53));
}

Int next_uniform(Int min, Int max) {
    dbg_assert(min <= max);

    return min + (Int)((max - min) * uniform01());
}

Int next_exponential(Int min, Int max, Float lambda) {
    dbg_assert(min <= max);
    dbg_assert(lambda != 0.0f);

    Float u = uniform01();
    Float x = -log(1.0 - u) / lambda; // in [0, inf)

    return clamp(min, max, (Int)x);
}

Int next_powerlaw(Int min, Int max, Float alpha) {
    dbg_assert(min <= max);

    Float u = uniform01();
    Float x;
    if (fabs(alpha - 1.0) < 1e-8) {
        Float ratio = (Float)max / min;
        x = min * pow(ratio, u);
    } else {
        Float exp = 1.0 - alpha;
        Float min_e = pow((Float)min, exp);
        Float max_e = pow((Float)max, exp);
        Float val_e = min_e + u * (max_e - min_e);
        x = pow(val_e, 1.0 / exp);
    }
    // clamp just in case of fp drift
    return clamp(min, max, (Int)x);
}

Int next_random(Int min, Int max, Float param, Distribution distribution) {
    switch (distribution) {
    case DISTRIBUTION_UNIFORM:
        return next_uniform(min, max);
    case DISTRIBUTION_EXP:
        return next_exponential(min, max, param);
    case DISTRIBUTION_POWERLAW:
        return next_powerlaw(min, max, param);
    default:
        fatal("Unknown distribution");
    }
}

Int random_choice(const IntList *l) {
    dbg_assert(l->count > 0);

    Int idx = next_uniform(0, l->count);
    return l->items[idx];
}

static Int sum(const IntList *l) {
    Int acc = 0;
    for (u32 i = 0; i < l->count; i++) {
        acc += l->items[i];
    }
    return acc;
}

Int random_wchoice(const IntList *l, const IntList *weights) {
    dbg_assert(l->count > 0);

    if (weights->count == 0) {
        return random_choice(l);
    }

    Int vote = next_uniform(0, sum(weights));
    Int a = 0;
    u32 idx = 0;
    for (u32 i = 0; i < weights->count; i++) {
        a += weights->items[i];
        if (vote < a) {
            idx = i;
            break;
        }
    }

    return l->items[idx];
}
