#ifndef _UTILS_COMMON_H_
#define _UTILS_COMMON_H_

#define UTILSAPI

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ansi_colors.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int32_t i32;
typedef int64_t i64;
typedef size_t usize;
typedef ssize_t isize;

typedef u64 Int;
typedef i64 SInt;
typedef double Float;
typedef const char *Str;

#define print(fmt, ...) printf((fmt), ##__VA_ARGS__)
#define fprint(file, fmt, ...) fprintf((file), (fmt), ##__VA_ARGS__)
#define println(...) __println_helper("" __VA_ARGS__)
#define __println_helper(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define fprintln(file, ...) __fprintln_helper(file, "" __VA_ARGS__)
#define __fprintln_helper(file, fmt, ...)                                      \
    fprintf((file), fmt "\n", ##__VA_ARGS__)

#define eprint(fmt, ...) fprintf(stderr, (fmt), ##__VA_ARGS__)
#define eprintln(...) __eprintln_helper("" __VA_ARGS__)
#define __eprintln_helper(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

#define fatal(...)                                                             \
    do {                                                                       \
        eprintln(ANSI_BOLD ANSI_RED "[ERROR]" ANSI_RESET " " __VA_ARGS__);     \
        abort();                                                               \
    } while (0)

#define panic(...)                                                             \
    do {                                                                       \
        __panic_at_position;                                                   \
        eprintln(" -> " __VA_ARGS__);                                          \
        abort();                                                               \
    } while (0)

#define __panic_at_position                                                    \
    eprintln(ANSI_BOLD ANSI_RED "[ERROR]" ANSI_RESET " Panicked at " __FILE__  \
                                ":%u",                                         \
             __LINE__)

#define todo(...)                                                              \
    do {                                                                       \
        __todo_at_position;                                                    \
        eprintln(" -> " __VA_ARGS__);                                          \
        abort();                                                               \
    } while (0)

#define __todo_at_position                                                     \
    eprintln(ANSI_BOLD ANSI_YELLOW "[TODO]" ANSI_RESET                         \
                                   " Panicked at " __FILE__ ":%u",             \
             __LINE__)

#if !defined(__OPTIMIZE__) || defined(KEEP_DBG_ASSERT)
#define dbg_assert(expr)                                                       \
    do {                                                                       \
        if (!(expr)) {                                                         \
            panic("Assertion failed for: " #expr);                             \
        }                                                                      \
    } while (0)
#else
#define dbg_assert(expr)
#endif

#define assert_eq(expr1, expr2)                                                \
    do {                                                                       \
        if ((expr1) != (expr2)) {                                              \
            panic("Assertion failed for: " #expr1 " == " #expr2);              \
        }                                                                      \
    } while (0)

#define assert_neq(expr1, expr2)                                               \
    do {                                                                       \
        if ((expr1) == (expr2)) {                                              \
            panic("Assertion failed for: " #expr1 " != " #expr2);              \
        }                                                                      \
    } while (0)

typedef enum {
    POLICY_LIFO,
    POLICY_FIFO,
    POLICY_RANDOM,
    POLICY_BIG_FIRST,
    POLICY_SMALL_FIRST,
    POLICY_NEVER,
    POLICY_COUNT
} Policy;

#if defined(__cplusplus)
}

#include <array>

inline constexpr auto __policies = []() constexpr {
    std::array<const char *, POLICY_COUNT> p{};

    p[POLICY_LIFO] = "lifo";
    p[POLICY_FIFO] = "fifo";
    p[POLICY_RANDOM] = "random";
    p[POLICY_BIG_FIRST] = "big-first";
    p[POLICY_SMALL_FIRST] = "small-first";
    p[POLICY_NEVER] = "never";

    return p;
}();

inline constexpr auto policies = __policies.data();

extern "C" {
#else

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static const char *policies[] = {[POLICY_LIFO] = "lifo",
                                 [POLICY_FIFO] = "fifo",
                                 [POLICY_RANDOM] = "random",
                                 [POLICY_BIG_FIRST] = "big-first",
                                 [POLICY_SMALL_FIRST] = "small-first",
                                 [POLICY_NEVER] = "never"};

#endif // __cplusplus

typedef enum {
    DISTRIBUTION_UNIFORM,
    DISTRIBUTION_EXP,
    DISTRIBUTION_POWERLAW,
    DISTRIBUTION_COUNT
} Distribution;

#if defined(__cplusplus)
}

#include <array>

inline constexpr auto __distributions = []() constexpr {
    std::array<const char *, DISTRIBUTION_COUNT> d{};

    d[DISTRIBUTION_UNIFORM] = "uniform";
    d[DISTRIBUTION_EXP] = "exp";
    d[DISTRIBUTION_POWERLAW] = "powerlaw";

    return d;
}();

inline constexpr auto distributions = __distributions.data();

extern "C" {
#else

static const char *distributions[] = {[DISTRIBUTION_UNIFORM] = "uniform",
                                      [DISTRIBUTION_EXP] = "exp",
                                      [DISTRIBUTION_POWERLAW] = "powerlaw"};

#endif // __cplusplus

typedef enum {
    TREND_NONE,
    TREND_GROW,
    TREND_SHRINK,
    TREND_SAW,
    TREND_COUNT,
} Trend;

#if defined(__cplusplus)
}

#include <array>

inline constexpr auto __trends = []() constexpr {
    std::array<const char *, TREND_COUNT> t{};

    t[TREND_NONE] = "none";
    t[TREND_GROW] = "grow";
    t[TREND_SHRINK] = "shrink";
    t[TREND_SAW] = "saw";

    return t;
}();

inline constexpr auto trends = __trends.data();

extern "C" {
#else

static const char *trends[] = {
    [TREND_NONE] = "none",
    [TREND_GROW] = "grow",
    [TREND_SHRINK] = "shrink",
    [TREND_SAW] = "saw",
};

#endif // __cplusplus

typedef enum {
    SIZE_LIST_MODE_EXACT,
    SIZE_LIST_MODE_NEAREST,
    SIZE_LIST_MODE_COUNT,
} SizeListMode;

#if defined(__cplusplus)
}

#include <array>

inline constexpr auto __size_list_modes = []() constexpr {
    std::array<const char *, SIZE_LIST_MODE_COUNT> m{};

    m[SIZE_LIST_MODE_EXACT] = "exact";
    m[SIZE_LIST_MODE_NEAREST] = "nearest";

    return m;
}();

inline constexpr auto size_list_modes = __size_list_modes.data();

extern "C" {
#else

static const char *size_list_modes[] = {
    [SIZE_LIST_MODE_EXACT] = "exact",
    [SIZE_LIST_MODE_NEAREST] = "nearest",
};

#endif // __cplusplus

typedef enum {
    TTL_OFF,
    TTL_FIXED,
    TTL_LIST,
    TTL_COUNT,
} Lifetime;

#if defined(__cplusplus)
}

#include <array>

inline constexpr auto __ttls = []() constexpr {
    std::array<const char *, TTL_COUNT> l{};

    l[TTL_OFF] = "off";
    l[TTL_FIXED] = "fixed";
    l[TTL_LIST] = "list";

    return l;
}();

inline constexpr auto ttls = __ttls.data();

extern "C" {
#else

static const char *ttls[] = {
    [TTL_OFF] = "off",
    [TTL_FIXED] = "fixed",
    [TTL_LIST] = "list",
};

#pragma GCC diagnostic pop

#endif // __cplusplus

#if defined(__cplusplus)
}
#endif

#endif // _UTILS_COMMON_H_
