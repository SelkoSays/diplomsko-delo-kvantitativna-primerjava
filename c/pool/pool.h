#ifndef POOL_H
#define POOL_H

#include "../tracker/tracker.h"
#include "../utils/common.h"
#include "deque.h"

typedef struct {
    Deque items;
    Int capacity;
} Pool;

Pool *pool_create(Int capacity);
void pool_init(Pool *pool, Int capacity);
Block *pool_add_block(Pool *pool, usize size);
Block *pool_add_block_with_ttl(Pool *pool, usize size, usize ttl);

void pool_update_and_prune(Pool *pool);
void pool_del_block(Pool *pool, Policy policy);
void pool_free(Pool *pool);
void pool_destroy(Pool **pool);

#endif // POOL_H
