#include "pool.hpp"

#include "../../c/utils/args_parser.h"

#include <algorithm>

Pool::Pool(Int capacity) {
    self.blocks = std::deque<Block>();
    self.capacity = capacity;
}

Block &Pool::add_block(usize size, SInt ttl) {
    if (self.count() >= self.capacity) {
        panic("Pool is at capacity. Cannot allocate new blocks.");
    }

    return self.blocks.emplace_back(size, ttl);
}

void Pool::del_block(Policy policy, Random &rng) {
    if (policy == POLICY_NEVER) {
        return;
    }

    if (self.blocks.size() == 0) {
        return;
    }

    switch (policy) {
    case POLICY_LIFO:
        self.blocks.pop_back();
        break;
    case POLICY_FIFO:
        self.blocks.pop_front();
        break;
    case POLICY_RANDOM: {
        Int idx = rng.uniform(0, self.count());
        auto p = self.blocks.begin();
        for (Int i = 0; i < idx; i++, p++)
            ;
        self.blocks.erase(p);
    } break;
    case POLICY_BIG_FIRST: {
        auto it = std::max_element(
            self.blocks.begin(), self.blocks.end(),
            [](Block &lhs, Block &rhs) { return lhs.size < rhs.size; });

        self.blocks.erase(it);
    } break;
    case POLICY_SMALL_FIRST: {
        auto it = std::min_element(
            self.blocks.begin(), self.blocks.end(),
            [](Block &lhs, Block &rhs) { return lhs.size < rhs.size; });

        self.blocks.erase(it);
    } break;
    default:
        panic("Unknown policy");
    }
}

void Pool::update_and_prune() {
    auto it = self.blocks.begin();
    while (it != self.blocks.end()) {
        if (it->ttl > 0) {
            it->ttl--;
        }

        if (it->ttl == 0) {
            log_info(ANSI_COLOR(RED, "Pruning") "    " DBG_BLOCK_STR,
                     size_str(it->size, 2), it->ttl_org);
            it = self.blocks.erase(it);
        } else {
            it++;
        }
    }
}

void Pool::touch_pages(Block &block, Int stride) const {
    Int rnd_val = 0xA5;
    u8 *data = block.data.data();
    for (Int off = 0; off < block.size; off += stride) {
        ((volatile u8 *)data)[off] = static_cast<u8>(rnd_val);
        rnd_val = rnd_val * 0x5DEECE66D + 0xB;
    }
}
