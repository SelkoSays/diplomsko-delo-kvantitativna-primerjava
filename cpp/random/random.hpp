#ifndef RANDOM_HPP
#define RANDOM_HPP

#include "../../c/utils/common.h"
#include "../../c/utils/list.h"
#include "../../c/utils/logging.h"

#include "../utils/common.hpp"

struct Random {
  public:
    u64 random_state;

  public:
    Random() : random_state(0x853c49e6748fea9bULL) {}
    Random(u64 seed);

    bool coin_flip(void) { return (uniform(0, 2) % 2) == 1; }

    Int uniform(Int min, Int max);
    Int exponential(Int min, Int max, Float lambda);
    Int powerlaw(Int min, Int max, Float alpha);

    Int next(Int min, Int max, Float param, Distribution distribution);

    Int choice(const IntList &l);
    Int choice(const IntList &l, const IntList &weights);

    static inline Int clamp(Int min, Int max, Int num) {
        dbg_assert(min <= max);
        if (num < min) {
            return min;
        }
        if (num > max) {
            return max;
        }
        return num;
    }

    double uniform01(void) {
        u64 r = xs64star() >> 11;
        return (double)r * (1.0 / (double)(1ULL << 53));
    }

  private:

    u64 splitmix64(u64 x) const {
        uint64_t z = x + 0x9e3779b97f4a7c15ULL;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    u64 xs64star(void) {
        u64 x = self.random_state;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        self.random_state = x;
        return x * 2685821657736338717ULL;
    }
};
#endif // RANDOM_H
