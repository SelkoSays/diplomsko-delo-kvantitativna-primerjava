#include "pool.h"

#include "../random/random.h"
#include "../utils/args_parser.h"

extern Tracker global_tracker;

Pool *pool_create(Int capacity) {
    Pool *pool = tracker_create(Pool, &global_tracker);

    if (pool == NULL) {
        panic("Could not create pool");
    }

    pool_init(pool, capacity);

    return pool;
}

void pool_init(Pool *pool, Int capacity) {
    // Don't initialize if null
    if (pool == NULL) {
        return;
    }

    deque_init(&pool->items);
    pool->capacity = capacity;
}

static Block *pool_add(Pool *pool, usize size, usize ttl) {
    if (pool == NULL) {
        panic("Provided pool is null");
    }

    if (pool->items.count >= pool->capacity) {
        panic("Pool is at capacity. Cannot allocate new blocks.");
    }

    Block *block = deque_append(&pool->items, size, ttl);

    return block;
}

Block *pool_add_block(Pool *pool, usize size) {
    return pool_add(pool, size, -1);
}

Block *pool_add_block_with_ttl(Pool *pool, usize size, usize ttl) {
    return pool_add(pool, size, ttl);
}

void pool_update_and_prune(Pool *pool) {
    if (pool == NULL) {
        panic("Provided pool is NULL");
    }

    DequeNode *dn = pool->items.head;
    while (dn != NULL) {
        Block *blk = &dn->blk;
        if (blk->ttl > 0) {
            blk->ttl--;
        }
        if (blk->ttl == 0) {
            DequeNode *tmp = dn;
            dn = dn->next;
            deque_pop_node(&pool->items, tmp);
        } else {
            dn = dn->next;
        }
    }
}

void pool_del_block(Pool *pool, Policy policy) {
    if (policy == POLICY_NEVER) {
        return;
    }

    if (pool == NULL) {
        panic("Provided pool is NULL");
    }

    if (pool->items.count == 0) {
        log_warn("Trying to delete block from empty pool");
        return;
    }

    switch (policy) {
    case POLICY_LIFO:
        deque_pop_back(&pool->items);
        break;
    case POLICY_FIFO:
        deque_pop_front(&pool->items);
        break;
    case POLICY_RANDOM:
        deque_pop(&pool->items, (usize)next_uniform(0, pool->items.count));
        break;
    case POLICY_BIG_FIRST: {
        DequeNode *node = deque_find_max(&pool->items);
        if (node != NULL) {
            deque_pop_node(&pool->items, node);
        }
    } break;
    case POLICY_SMALL_FIRST: {
        DequeNode *node = deque_find_min(&pool->items);
        if (node != NULL) {
            deque_pop_node(&pool->items, node);
        }
    } break;
    default:
        panic("Unknown policy");
    }
}

void pool_free(Pool *pool) {
    // Null pointer is valid input
    if (pool == NULL) {
        return;
    }

    // Remove all live blocks
    while (pool->items.count > 0) {
        deque_pop_front(&pool->items);
    }
}

void pool_destroy(Pool **pool) {
    // Null pointer is valid input
    if (pool == NULL) {
        return;
    }

    pool_free(*pool);

    tracker_destroy(&global_tracker, *pool);
    *pool = NULL;
}
