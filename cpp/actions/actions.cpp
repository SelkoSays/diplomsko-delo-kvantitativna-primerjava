#include "actions.hpp"

#include "../../c/utils/args_parser.h"
#include "../../c/utils/list.h"

static Int block_size_tmp = 0;

namespace action {

void init_actions(const Args &args) {
    switch (args.size_trend.as.e) {
    case TREND_NONE:
        break;
    case TREND_SAW:
    case TREND_GROW:
        block_size_tmp = args.min_size.as.i;
        break;
    case TREND_SHRINK:
        block_size_tmp = args.max_size.as.i;
        break;
    default:
        panic("Unknown trend %u", args.size_trend.as.e);
    }
}

/// @brief Block size according to trend (ignore list)
/// @param args
/// @return Block size
static Int trend_block_size(const Args &args, Random &rng) {
    SInt jitter = (SInt)rng.uniform(0, args.trend_jitter.as.i * 2) -
                  (SInt)args.trend_jitter.as.i;

    switch (args.size_trend.as.e) {
    case TREND_NONE:
        return rng.next(args.min_size.as.i, args.max_size.as.i,
                        args.dist_param.as.f,
                        (Distribution)args.distribution.as.e);
    case TREND_GROW: {
        Int tmp = block_size_tmp;

        SInt step = (SInt)args.size_step.as.i + jitter;
        if (step <= 0) {
            step = 1;
        }
        block_size_tmp += (Int)step;
        block_size_tmp = Random::clamp(args.min_size.as.i, args.max_size.as.i,
                                       block_size_tmp);

        return tmp;
    }
    case TREND_SHRINK: {
        Int tmp = block_size_tmp;
        SInt step = (SInt)args.size_step.as.i + jitter;
        if (step <= 0) {
            step = 1;
        }
        if ((Int)step > block_size_tmp) {
            block_size_tmp = args.min_size.as.i;
        } else {
            block_size_tmp -= (Int)step;
        }
        block_size_tmp = Random::clamp(args.min_size.as.i, args.max_size.as.i,
                                       block_size_tmp);

        return tmp;
    }
    case TREND_SAW: {
        Int tmp = block_size_tmp;

        SInt step = (SInt)args.size_step.as.i + jitter;
        if (step <= 0) {
            step = 1;
        }
        block_size_tmp += (Int)step;
        if (block_size_tmp > args.max_size.as.i) {
            block_size_tmp = args.min_size.as.i;
        }
        return tmp;
    }
    default:
        panic("Unknown trend %u", args.size_trend.as.e);
    }
}

static inline constexpr SInt abs_int(SInt n) { return (n >= 0) ? n : -n; }

static Int get_closest(IntList *list, Int size) {
    IntList l = *list;
    if (l.count == 0) {
        return size;
    }

    SInt min = -1;
    usize idx = (usize)-1;
    for (usize i = 0; i < l.count; i++) {
        SInt v = abs_int((SInt)size - (SInt)l.items[i]);
        if ((min < 0) || (v < min)) {
            min = v;
            idx = i;
        }
    }

    if (min < 0 || ((isize)idx) < 0) {
        panic("Minimum and index should not be less than 0");
    }

    return l.items[idx];
}

/// @brief Block size according to all argumnets
/// @param args
/// @return Block size
static Int get_block_size(const Args &args, Random &rng) {
    IntList l = args.size_list.as.il;
    if (l.count == 0) {
        return trend_block_size(args, rng);
    }

    switch (args.size_mode.as.e) {
    case SIZE_LIST_MODE_EXACT:
        return rng.choice(l, args.size_weights.as.il);
    case SIZE_LIST_MODE_NEAREST: {
        return get_closest(&l, trend_block_size(args, rng));
    } break;
    default:
        panic("Unknown size mode %u", args.size_mode.as.e);
    }
}

static SInt get_block_ttl(const Args &args, Random &rng) {
    switch (args.ttl_mode.as.e) {
    case TTL_OFF:
        return -1L;
    case TTL_FIXED:
        return (SInt)args.ttl_fixed.as.i;
    case TTL_LIST:
        return (SInt)rng.choice(args.ttl_list.as.il, args.ttl_weights.as.il);
    default:
        panic("Unknown ttl mode %d", args.ttl_mode.as.e);
    }
}

static inline constexpr bool should_alloc(const Pool &pool, const Args &args,
                                          Random &rng) {
    return (pool.count() < pool.capacity) &&
           (rng.uniform01() < args.alloc_freq.as.f);
}

void block_action(Pool &pool, const Args &args, Random &rng) {
    if (args.ttl_mode.as.e != TTL_OFF) {
        pool.update_and_prune();
    }

    bool alloc = should_alloc(pool, args, rng);

    if (!alloc) {
        pool.del_block((Policy)args.policy.as.e, rng);
    } else {
        Int block_size = get_block_size(args, rng);
        SInt block_ttl = get_block_ttl(args, rng);
        pool.add_block(block_size, block_ttl);
    }
}

} // namespace action