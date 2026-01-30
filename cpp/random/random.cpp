#include "random.hpp"

#include <cmath>

Random::Random(u64 seed) {
    if (seed == 0) {
        seed = (u64)time(NULL) ^ (uintptr_t)&seed;
    }
    random_state = splitmix64(seed);
    if (random_state == 0) {
        random_state = 0x9e3779b97f4a7c15ULL;
    }
}

Int Random::uniform(Int min, Int max) {
    dbg_assert(min <= max);

    Float u = uniform01(); // in [0,1)
    return min + static_cast<Int>((max - min) * u);
}

Int Random::exponential(Int min, Int max, Float lambda) {
    dbg_assert(min <= max);
    dbg_assert(lambda != 0.0f);

    Float u = uniform01();
    Float x = -std::log(1.0 - u) / lambda; // in [0, inf)

    return clamp(min, max, static_cast<Int>(x));
}

Int Random::powerlaw(Int min, Int max, Float alpha) {
    dbg_assert(min <= max);

    Float u = uniform01();
    Float x;
    if (std::fabs(alpha - 1.0) < 1e-8) {
        Float ratio = static_cast<Float>(max) / min;
        x = min * std::pow(ratio, u);
    } else {
        Float exp = 1.0 - alpha;
        Float min_e = std::pow(static_cast<Float>(min), exp);
        Float max_e = std::pow(static_cast<Float>(max), exp);
        Float val_e = min_e + u * (max_e - min_e);
        x = std::pow(val_e, 1.0 / exp);
    }
    // clamp just in case
    return clamp(min, max, static_cast<Int>(x));
}

Int Random::next(Int min, Int max, Float param, Distribution distribution) {
    switch (distribution) {
    case DISTRIBUTION_UNIFORM:
        return uniform(min, max);
    case DISTRIBUTION_EXP:
        return exponential(min, max, param);
    case DISTRIBUTION_POWERLAW:
        return powerlaw(min, max, param);
    default:
        fatal("Unknown distribution");
    }
}

Int Random::choice(const IntList &l) {
    dbg_assert(l.count > 0);

    Int idx = self.uniform(0, l.count);
    return l.items[idx];
}

static inline constexpr Int sum(const IntList &l) {
    Int acc = 0;
    for (u32 i = 0; i < l.count; i++) {
        acc += l.items[i];
    }
    return acc;
}

Int Random::choice(const IntList &l, const IntList &weights) {
    dbg_assert(l.count > 0);

    if (weights.count == 0) {
        return self.choice(l);
    }

    Int vote = self.uniform(0, sum(weights));
    Int a = 0;
    u32 idx = 0;
    for (u32 i = 0; i < weights.count; i++) {
        a += weights.items[i];
        if (vote < a) {
            idx = i;
            break;
        }
    }

    return l.items[idx];
}
