#include "deque.h"
#include "../tracker/tracker.h"
#include "../utils/args_parser.h"

extern Tracker global_tracker;

enum DequeAddOpt { DEQUE_PREPEND, DEQUE_APPEND };
enum DequeRemOpt {
    DEQUE_POP_FRONT,
    DEQUE_POP_BACK,
    DEQUE_POP_IDX,
    DEQUE_POP_NODE
};
enum DequeFindOpt {
    DEQUE_FIND_MIN,
    DEQUE_FIND_MAX,
};

void deque_node_init(DequeNode *node, usize size) {
    if (node == NULL) {
        panic("Provided node is null");
    }

    node->blk.data = tracker_alloc(&global_tracker, size);

    if (node->blk.data == NULL) {
        log_error("Could not allocate block of size %zu", size);
        node->blk.size = 0;
        return;
    }

    node->blk.size = size;
    node->prev = NULL;
    node->next = NULL;
}

void deque_init(Deque *deq) {
    deq->head = NULL;
    deq->tail = NULL;
    deq->count = 0;
}

static Block *deque_add(Deque *deq, usize size, isize ttl,
                        enum DequeAddOpt opt);
static void deque_remove(Deque *deq, usize idx, DequeNode *node,
                         enum DequeRemOpt opt);

Block *deque_prepend(Deque *deq, usize size, isize ttl) {
    return deque_add(deq, size, ttl, DEQUE_PREPEND);
}

Block *deque_append(Deque *deq, usize size, isize ttl) {
    return deque_add(deq, size, ttl, DEQUE_APPEND);
}

static Block *deque_add(Deque *deq, usize size, isize ttl,
                        enum DequeAddOpt opt) {
    if (deq == NULL) {
        panic("Provided deque is null");
    }

    if (size == 0) {
        return NULL;
    }

    DequeNode *dn = calloc(1, sizeof(DequeNode));
    if (dn == NULL) {
        log_error("Could not create deque node");
        return NULL;
    }

    deque_node_init(dn, size);
    if (dn->blk.data == NULL) {
        free(dn);
        return NULL;
    }

    dn->blk.ttl = ttl;
    dn->blk.ttl_org = ttl;

    dbg_assert((deq->head != NULL) ||
               (deq->tail == NULL)); // head(null) => tail(null)
    dbg_assert((deq->tail != NULL) ||
               (deq->head == NULL)); // tail(null) => head(null)

    if (deq->head == NULL) {
        deq->head = dn;
        deq->tail = dn;
    } else if (opt == DEQUE_APPEND) {
        deq->tail->next = dn;
        dn->prev = deq->tail;
        deq->tail = dn;
    } else if (opt == DEQUE_PREPEND) {
        deq->head->prev = dn;
        dn->next = deq->head;
        deq->head = dn;
    } else {
        panic("Unknown deque_add option %d", opt);
    }

    deq->count++;

    return &dn->blk;
}

void deque_pop_front(Deque *deq) {
    deque_remove(deq, 0, NULL, DEQUE_POP_FRONT);
}

void deque_pop_back(Deque *deq) { deque_remove(deq, 0, NULL, DEQUE_POP_BACK); }

void deque_pop(Deque *deq, usize idx) {
    deque_remove(deq, idx, NULL, DEQUE_POP_IDX);
}

void deque_pop_node(Deque *deq, DequeNode *node) {
    deque_remove(deq, 0, node, DEQUE_POP_NODE);
}

static void deque_remove(Deque *deq, usize idx, DequeNode *node,
                         enum DequeRemOpt opt) {
    if (deq == NULL) {
        panic("Provided deque is null");
    }

    if ((opt == DEQUE_POP_IDX) && (idx >= deq->count)) {
        panic("Index out of bounds");
    }

    dbg_assert((deq->head != NULL) ||
               (deq->tail == NULL)); // head(null) => tail(null)
    dbg_assert((deq->tail != NULL) ||
               (deq->head == NULL)); // tail(null) => head(null)
    dbg_assert((deq->head != NULL) ||
               (deq->count == 0)); // head(null) => count(0)

    if (deq->count == 0) {
        log_warn("Trying to remove from empty deque");
        return;
    }

    DequeNode *dn = NULL;

    switch (opt) {
    case DEQUE_POP_FRONT: {
        dn = deq->head;
        deq->head = dn->next;
        if (deq->head != NULL) {
            deq->head->prev = NULL;
        }
    } break;
    case DEQUE_POP_BACK: {
        dn = deq->tail;
        deq->tail = dn->prev;
        if (deq->tail != NULL) {
            deq->tail->next = NULL;
        }
    } break;
    case DEQUE_POP_IDX: {
        if (idx == 0) {
            dn = deq->head;
            deq->head = dn->next;
            if (deq->head != NULL) {
                deq->head->prev = NULL;
            }
        }
        if (idx == (deq->count - 1)) {
            dn = deq->tail;
            deq->tail = dn->prev;
            if (deq->tail != NULL) {
                deq->tail->next = NULL;
            }
        }
        if ((idx > 0) && (idx < (deq->count - 1))) {
            if ((deq->count - idx) < idx) {
                dn = deq->tail;
                for (usize i = deq->count - 1; i > idx; i--) {
                    dn = dn->prev;
                }
            } else {
                dn = deq->head;
                for (usize i = 0; i < idx; i++) {
                    dn = dn->next;
                }
            }

            dbg_assert(dn->prev != NULL);
            dbg_assert(dn->next != NULL);

            dn->prev->next = dn->next;
            dn->next->prev = dn->prev;
        }
    } break;
    case DEQUE_POP_NODE: {
        if (node == NULL) {
            return;
        }

        dn = node;
        if (dn == deq->head) {
            deq->head = dn->next;
            if (deq->head != NULL) {
                deq->head->prev = NULL;
            }
        }
        if (dn == deq->tail) {
            deq->tail = dn->prev;
            if (deq->tail != NULL) {
                deq->tail->next = NULL;
            }
        }

        if (dn->prev != NULL) {
            dn->prev->next = dn->next;
        }
        if (dn->next != NULL) {
            dn->next->prev = dn->prev;
        }
    } break;
    default:
        panic("Unknown deque_remove option %d", opt);
        break;
    }

    if (dn == NULL) {
        panic("DequeNode should not be null at this point");
    }

    deq->count--;

    if (deq->count == 0) {
        deq->head = NULL;
        deq->tail = NULL;
    }

    tracker_free(&global_tracker, dn->blk.data, dn->blk.size);
    dn->prev = NULL;
    dn->next = NULL;
    free(dn);
}

DequeNode *deque_at(Deque *deq, usize idx) {
    if (deq == NULL) {
        panic("deque is null");
    }

    if (idx >= deq->count) {
        panic("Index out of bounds Idx(%zu) >= Count(%zu)", idx, deq->count);
    }

    DequeNode *dn = deq->head;
    for (usize i = 0; i < idx; i++) {
        dn = dn->next;
    }

    return dn;
}

void log_deque(const Deque *dq) {
    DequeNode *dn = dq->head;

    println("Deque [");
    while ((dn != NULL)) {
        println("  Block { data: %p, size: %zu, ttl: %ld, ttl_org: %ld },",
                (void *)dn->blk.data, dn->blk.size, dn->blk.ttl,
                dn->blk.ttl_org);
        dn = dn->next;
    }
    println("]");
}

static DequeNode *deque_find(Deque *deq, enum DequeFindOpt opt);

DequeNode *deque_find_min(Deque *deq) {
    return deque_find(deq, DEQUE_FIND_MIN);
}
DequeNode *deque_find_max(Deque *deq) {
    return deque_find(deq, DEQUE_FIND_MAX);
}

static DequeNode *deque_find(Deque *deq, enum DequeFindOpt opt) {
    if (deq == NULL || deq->count == 0) {
        return NULL;
    }

    DequeNode *node = NULL;
    isize base = -1L;
    DequeNode *dn = deq->head;

    bool flip = (opt == DEQUE_FIND_MIN);
    usize i = 0;
    while ((dn != NULL) && (i < deq->count)) {
        if ((base == -1) || ((((isize)dn->blk.size) > base) ^ flip)) {
            node = dn;
            base = (isize)dn->blk.size;
        }
        dn = dn->next;
        i++;
    }

    return node;
}
