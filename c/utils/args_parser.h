#ifndef _UTILS_ARGS_PARSER_H_
#define _UTILS_ARGS_PARSER_H_

#include "common.h"

#include "list.h"
#include "logging.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define shift(xs, xs_sz) (assert((xs_sz) > 0), (xs_sz)--, *(xs)++)

#pragma region TYPE DEFS

#define ARGUMENTS                                                              \
    /* General */                                                              \
    A(policy)                                                                  \
    A(iterations)                                                              \
    A(duration_sec)                                                            \
    A(alloc_freq)                                                              \
    A(seed)                                                                    \
    /* Pool */                                                                 \
    A(capacity)                                                                \
    /* Block size */                                                           \
    A(size_trend)                                                              \
    A(size_step)                                                               \
    A(trend_jitter)                                                            \
    A(size_list)                                                               \
    A(size_mode)                                                               \
    A(size_weights)                                                            \
    A(min_size)                                                                \
    A(max_size)                                                                \
    /* Block size distribution */                                              \
    A(distribution)                                                            \
    A(dist_param)                                                              \
    /* Block lifetimes*/                                                       \
    A(ttl_mode)                                                                \
    A(ttl_fixed)                                                               \
    A(ttl_list)                                                                \
    A(ttl_weights)                                                                 \
    /* Instrumentation & output */                                             \
    A(snap_interval)                                                           \
    A(output)                                                                  \
    A(display)

typedef enum {
    ARG_TYPE_INT,       /* expects “N” -> stored in Int* */
    ARG_TYPE_TIME,      /* expects “N” -> stored in Int* */
    ARG_TYPE_SIZE,      /* expects “N” -> stored in Int* */
    ARG_TYPE_FLOAT,     /* expects “F” -> stored in Float* */
    ARG_TYPE_STR,       /* expects “S” -> stored in Str* */
    ARG_TYPE_BOOL,      /* no value; sets a bool* true if present */
    ARG_TYPE_ENUM,      /* expects one of choices[] -> stored in Int* index */
    ARG_TYPE_INT_LIST,  /* expects comma separated vals -> stored in IntList* */
    ARG_TYPE_SIZE_LIST, /* expects comma separated vals -> stored in IntList* */
    ARG_TYPE_COUNT
} ArgType;

typedef struct {
    ArgType t;
    union {
        Int i;      // Int
        Float f;    // Float
        Str s;      // String
        bool b;     // Flag
        u32 e;      // Enum
        IntList il; // Int list
    } as;
} Arg;

#if defined(__cplusplus)
}

#include <array>

constexpr auto __arg_types() {
    std::array<const char *, ARG_TYPE_COUNT> a{};

    a[ARG_TYPE_INT] = "ARG_TYPE_INT";
    a[ARG_TYPE_TIME] = "ARG_TYPE_TIME";
    a[ARG_TYPE_SIZE] = "ARG_TYPE_SIZE";
    a[ARG_TYPE_FLOAT] = "ARG_TYPE_FLOAT";
    a[ARG_TYPE_STR] = "ARG_TYPE_STR";
    a[ARG_TYPE_BOOL] = "ARG_TYPE_BOOL";
    a[ARG_TYPE_ENUM] = "ARG_TYPE_ENUM";
    a[ARG_TYPE_INT_LIST] = "ARG_TYPE_INT_LIST";
    a[ARG_TYPE_SIZE_LIST] = "ARG_TYPE_SIZE_LIST";

    return a;
}

static auto arg_types = __arg_types();

extern "C" {
#else

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static const char *arg_types[] = {
    [ARG_TYPE_INT] = "ARG_TYPE_INT",
    [ARG_TYPE_TIME] = "ARG_TYPE_TIME",
    [ARG_TYPE_SIZE] = "ARG_TYPE_SIZE",
    [ARG_TYPE_FLOAT] = "ARG_TYPE_FLOAT",
    [ARG_TYPE_STR] = "ARG_TYPE_STR",
    [ARG_TYPE_BOOL] = "ARG_TYPE_BOOL",
    [ARG_TYPE_ENUM] = "ARG_TYPE_ENUM",
    [ARG_TYPE_INT_LIST] = "ARG_TYPE_INT_LIST",
    [ARG_TYPE_SIZE_LIST] = "ARG_TYPE_SIZE_LIST",
};
#pragma GCC diagnostic pop

#endif // __cplusplus

#define A(name) Arg name;
typedef struct {
    ARGUMENTS
} Args;
#undef A

#define spec_type(spec) (spec).default_val.t

typedef struct _ArgSpec {
    char short_opt;       /* e.g. 'n' for -n; '\0' if none */
    const char *long_opt; /* e.g. "iterations" for --iterations */
    void (*setter)(Args *, Arg); /* field setting fuction variable */
    bool required;               /* true -> missing -> error */
    const char *metavar;         /* e.g. "N" or "FILE" in usage */
    const char *help;            /* help string */
    const char *category;        /* category */
    
    // for ENUM only: list of valid strings, and count; NULL,count=0 otherwise
    const char *const *choices;
    size_t choices_count;
    /* default value: applied before parsing */
    Arg default_val;
    bool has_default;
} ArgSpec;

#pragma endregion TYPE DEFS

#pragma region PARSE FUNCS

#define A(name) void __args_set_field_##name(Args *args, Arg val);
ARGUMENTS
#undef A

int parse_enum(const char *argval, ArgSpec *spec);
Int parse_duration(const char *argval);
Int parse_size(const char *argval);
Int parse_int(const char *argval);
Float parse_float(const char *argval);
IntList parse_size_list(const char *argval);
IntList parse_int_list(const char *argval);

void parse_argval(const char *argval, Args *args, ArgSpec *spec);

#pragma endregion PARSE FUNCS

extern Args global_args;
extern ArgSpec specs[];
extern usize spec_count;

#pragma region FUNCTIONS

UTILSAPI void print_usage(const char *progname, const ArgSpec *specs,
                          size_t spec_count);

UTILSAPI void parse_args(int argc, char **argv, Args *args, ArgSpec *specs,
                         usize spec_count);

UTILSAPI void free_args(Args *args);

#pragma endregion FUNCTIONS

#pragma region LOGGING

void log_args(const Args *args);
char *size_str(Int size, u8 precision);

#pragma endregion LOGGING

#ifndef NO_UNDEF_ARGS
#undef ARGUMENTS
#endif

#if defined(__cplusplus)
}
#endif

#endif // _UTILS_ARGS_PARSER_H_
