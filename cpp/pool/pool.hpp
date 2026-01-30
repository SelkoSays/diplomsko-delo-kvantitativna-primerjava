#ifndef POOL_HPP
#define POOL_HPP

#include "../../c/utils/common.h"
#include "../tracker/tracker.hpp"

#include "../random/random.hpp"
#include "../utils/common.hpp"

#include <queue>

#define DBG_BLOCK_STR                                                          \
    ANSI_COLOR(YELLOW, "Block")                                                \
    " { " ANSI_COLOR(GRAY, "size") " = %s, " ANSI_COLOR(GRAY, "ttl") " = %ld " \
                                                                     "}"

namespace block {
template <class ByteAlloc = std::allocator<u8>> struct Block {
    using allocator_type = ByteAlloc;

    std::vector<u8, ByteAlloc> data;
    Int size;
    SInt ttl;
    SInt ttl_org;

    Block(std::allocator_arg_t, const ByteAlloc &a, Int sz)
        : Block(a, sz, -1) {}

    Block(std::allocator_arg_t, const ByteAlloc &a, Int sz, SInt ttl_)
        : data(a), size(sz), ttl(ttl_), ttl_org(ttl_) {
        data.resize(size, 0);
    }

    explicit Block(Int sz) : Block(sz, -1) {}
    explicit Block(Int sz, SInt ttl_)
        : data(), size(sz), ttl(ttl_), ttl_org(ttl_) {
        data.resize(size, 0);
    }
};
} // namespace block

using Block = block::Block<tracker::TrackingAllocator<u8>>;

struct Pool {
    std::deque<Block> blocks;
    Int capacity;

  public:
    Pool(Int capacity);
    Pool(Pool &) = delete;
    Pool(const Pool &) = delete;

    Block &add_block(usize size, SInt ttl = -1L);
    void del_block(Policy policy, Random &rng);

    void update_and_prune();

    Block &operator[](usize idx) {
        if (idx >= this->capacity) {
            panic("Index out of bounds (idx(%zu) >= capacity(%zu))", idx,
                  this->capacity);
        }

        return this->blocks[idx];
    }

    usize count() const { return this->blocks.size(); }
};

#endif // POOL_HPP
