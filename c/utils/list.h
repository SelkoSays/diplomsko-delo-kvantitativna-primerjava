#ifndef LIST_H
#define LIST_H

#include "common.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

#ifndef LIST_INIT_CAP
#define LIST_INIT_CAP 16
#endif

#define list_reserve(l, expected_capacity)                                     \
    do {                                                                       \
        if ((expected_capacity) > (l)->capacity) {                             \
            if ((l)->capacity == 0) {                                          \
                (l)->capacity = LIST_INIT_CAP;                                 \
            }                                                                  \
            while ((expected_capacity) > (l)->capacity) {                      \
                (l)->capacity *= 2;                                            \
            }                                                                  \
            (l)->items =                                                       \
                realloc((l)->items, (l)->capacity * sizeof(*(l)->items));      \
            assert((l)->items != NULL && "Buy more RAM lol");                  \
        }                                                                      \
    } while (0)

// Append an item to a dynamic array
#define list_append(l, item)                                                   \
    do {                                                                       \
        list_reserve((l), (l)->count + 1);                                     \
        (l)->items[(l)->count++] = (item);                                     \
    } while (0)

#define list_free(l) free((l).items)

// Append several items to a dynamic array
#define list_append_many(l, new_items, new_items_count)                        \
    do {                                                                       \
        list_reserve((l), (l)->count + (new_items_count));                     \
        memcpy((l)->items + (l)->count, (new_items),                           \
               (new_items_count) * sizeof(*(l)->items));                       \
        (l)->count += (new_items_count);                                       \
    } while (0)

#define list_resize(l, new_size)                                               \
    do {                                                                       \
        list_reserve((l), new_size);                                           \
        (l)->count = (new_size);                                               \
    } while (0)

#define list_last(l) (l)->items[(assert((l)->count > 0), (l)->count - 1)]
#define list_remove_unordered(l, i)                                            \
    do {                                                                       \
        size_t j = (i);                                                        \
        assert(j < (l)->count);                                                \
        (l)->items[j] = (l)->items[--(l)->count];                              \
    } while (0)


typedef struct {
    Int *items;
    u32 count;
    u32 capacity;
} IntList;

#if defined(LIST_REMOVE_PREFIX)

#define reserve(l, expected_capacity) list_reserve(l, expected_capacity)
#define append(l, item) list_append(l, item)
#define append_many(l, new_items, new_items_count)                             \
    list_append_many(l, new_items, new_items_count)
#define resize(l, new_size) list_resize(l, new_size)
#define last(l) list_last(l)
#define remove_unordered(l, i) list_remove_unordered(l, i)

#endif // LIST_REMOVE_PREFIX

#if defined(__cplusplus)
}
#endif

#endif // LIST_H
