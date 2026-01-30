#ifndef DEQUE_H
#define DEQUE_H

#include "../utils/common.h"

#define DBG_BLOCK_STR                                                          \
    ANSI_COLOR(YELLOW, "Block")                                                \
    " { " ANSI_COLOR(GRAY, "size") " = %s, " ANSI_COLOR(GRAY, "ttl") " = %ld " \
                                                                     "}"
#define DBG_BLOCK_STR_BYTE                                                     \
    ANSI_COLOR(YELLOW, "Block")                                                \
    " { " ANSI_COLOR(GRAY, "size") " = %zu, " ANSI_COLOR(GRAY,                 \
                                                         "ttl") " = %ld "      \
                                                                "}"

typedef struct {
    void *data;
    Int size;
    SInt ttl;     // ..0 = INF, 0 = FREE, 1.. = ALIVE
    SInt ttl_org; // original ttl
} Block;

typedef struct DequeNode {
    Block blk;
    struct DequeNode *prev;
    struct DequeNode *next;
} DequeNode;

typedef struct {
    DequeNode *head;
    DequeNode *tail;
    usize count;
} Deque;

void deque_init(Deque *deq);

Block *deque_prepend(Deque *deq, usize size, isize ttl);
Block *deque_append(Deque *deq, usize size, isize ttl);

void deque_pop_front(Deque *deq);
void deque_pop_back(Deque *deq);
void deque_pop(Deque *deq, usize idx);
void deque_pop_node(Deque *deq, DequeNode *node);

DequeNode *deque_at(Deque *deq, usize idx);

DequeNode *deque_find_min(Deque *deq);
DequeNode *deque_find_max(Deque *deq);

void log_deque(const Deque *dq);

#endif // DEQUE_H
