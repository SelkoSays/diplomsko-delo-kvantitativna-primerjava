#define NO_UNDEF_ARGS
#include "args_parser.h"

#if defined(ARG_LOG)
#define log(...) log_debug(__VA_ARGS__)
#else
#define log(...)
#endif // ARG_LOG

#include "list.h"
#include "str_util.h"

#pragma region DEFINES

#define arg_int(v)                                                             \
    (Arg) {                                                                    \
        .t = ARG_TYPE_INT, .as = {.i = v }                                     \
    }
#define arg_time(v)                                                            \
    (Arg) {                                                                    \
        .t = ARG_TYPE_TIME, .as = {.i = v }                                    \
    }
#define arg_size(v)                                                            \
    (Arg) {                                                                    \
        .t = ARG_TYPE_SIZE, .as = {.i = v }                                    \
    }
#define arg_float(v)                                                           \
    (Arg) {                                                                    \
        .t = ARG_TYPE_FLOAT, .as = {.f = v }                                   \
    }
#define arg_str(v)                                                             \
    (Arg) {                                                                    \
        .t = ARG_TYPE_STR, .as = {.s = v }                                     \
    }
#define arg_bool(v)                                                            \
    (Arg) {                                                                    \
        .t = ARG_TYPE_BOOL, .as = {.b = v }                                    \
    }
#define arg_enum(v)                                                            \
    (Arg) {                                                                    \
        .t = ARG_TYPE_ENUM, .as = {.e = v }                                    \
    }
#define arg_intlist                                                            \
    (Arg) {                                                                    \
        .t = ARG_TYPE_INT_LIST, .as = {                                        \
            .il = {.items = NULL, .count = 0, .capacity = 0}                   \
        }                                                                      \
    }
#define arg_sizelist                                                           \
    (Arg) {                                                                    \
        .t = ARG_TYPE_SIZE_LIST, .as = {                                       \
            .il = {.items = NULL, .count = 0, .capacity = 0}                   \
        }                                                                      \
    }
#define arg_intlist_v(v)                                                       \
    (Arg) {                                                                    \
        .t = ARG_TYPE_INT_LIST, .as = {.il = v }                               \
    }
#define arg_sizelist_v(v)                                                      \
    (Arg) {                                                                    \
        .t = ARG_TYPE_SIZE_LIST, .as = {.il = v }                              \
    }

#define A(name)                                                                \
    void __args_set_field_##name(Args *args, Arg val) { args->name = val; }
ARGUMENTS
#undef A

#pragma endregion DEFINES

#pragma region GLOBALS

#if defined(__cplusplus)
extern "C" {
#endif

Args global_args = {0};

ArgSpec specs[] = {
    {'p', "policy", __args_set_field_policy, false, "POLICY", "Free policy",
     "General run-control", policies, POLICY_COUNT, arg_enum(POLICY_LIFO),
     true},
    {'n', "iterations", __args_set_field_iterations, false, "N",
     "Total alloc/free ops", "General run-control", NULL, 0, arg_int(10000u),
     true},
    {'d', "duration", __args_set_field_duration_sec, false, "SECS",
     "Run for this many seconds (mutually exclusive with "
     "--iterations)",
     "General run-control", NULL, 0, arg_time(0u), true},
    {'f', "alloc-freq", __args_set_field_alloc_freq, false, "F",
     "Frequency [0, 1] of allocations when policy not 'never'",
     "General run-control", NULL, 0, arg_float(0.7), true},
    {'s', "seed", __args_set_field_seed, false, "N", "RNG seed (0=time)",
     "General run-control", NULL, 0, arg_int(0u), true},

    {'c', "capacity", __args_set_field_capacity, false, "C", "Max live blocks",
     "Pool sizing", NULL, 0, arg_int(10000u), true},

    {'a', "min-size", __args_set_field_min_size, false, "BYTES",
     "Min block size", "Block-size", NULL, 0, arg_size(16u), true},
    {'A', "max-size", __args_set_field_max_size, false, "BYTES",
     "Max block size", "Block-size", NULL, 0, arg_size(1 << 20), true},
    {'\0', "size-trend", __args_set_field_size_trend, false, "TREND",
     "Set block size mutation through time. If 'none' sizes are random",
     "Block-size", trends, TREND_COUNT, arg_enum(TREND_NONE), true},
    {'\0', "size-step", __args_set_field_size_step, false, "BYTES",
     "Set block mutation size", "Block-size", NULL, 0, arg_size(1024u), true},
    {'\0', "trend-jitter", __args_set_field_trend_jitter, false, "BYTES",
     "Add uniform +- jitter to mutiation size", "Block-size", NULL, 0,
     arg_size(0), true},

    {'\0', "size-list", __args_set_field_size_list, false, "L[BYTES]",
     "Only use provided block sizes", "Block-size", NULL, 0, arg_sizelist,
     false},
    {'\0', "size-mode", __args_set_field_size_mode, false, "MODE",
     "Choose size from list. trend/distribution for nearest", "Block-size",
     size_list_modes, SIZE_LIST_MODE_COUNT, arg_enum(SIZE_LIST_MODE_EXACT),
     true},
    {'\0', "size-weights", __args_set_field_size_weights, false, "L[N]",
     "Set weights of size list (in %)", "Block-size", NULL, 0, arg_intlist,
     false},

    {'P', "distribution", __args_set_field_distribution, false, "TYPE",
     "Size distribution", "Block-size distribution", distributions,
     DISTRIBUTION_COUNT, arg_enum(DISTRIBUTION_UNIFORM), true},
    {'r', "dist-param", __args_set_field_dist_param, false, "F",
     "Parameter for non-uniform distributions {exp(lambda), "
     "powerlaw(alpha)}",
     "Block-size distribution", NULL, 0, arg_float(1.0f), true},

    {'\0', "ttl-mode", __args_set_field_ttl_mode, false, "MODE",
     "Set lifetime of blocks", "Block lifetime", ttls, TTL_COUNT,
     arg_enum(TTL_OFF), true},
    {'\0', "ttl-fixed", __args_set_field_ttl_fixed, false, "N",
     "Number of cycles blocks will live", "Block lifetime", NULL, 0,
     arg_int(10), true},
    {'\0', "ttl-list", __args_set_field_ttl_list, false, "L[N]",
     "Use provided liftimes uniformly", "Block lifetime", NULL, 0, arg_intlist,
     true},
    {'\0', "ttl-weights", __args_set_field_ttl_weights, false, "L[N]",
     "Set weights for list of lifetimes", "Block lifetime", NULL, 0,
     arg_intlist, true},

    {'i', "snap-interval", __args_set_field_snap_interval, false, "N",
     "Every N ops, snapshot and log stats", "Instrumentation & output", NULL, 0,
     arg_int(1000u), true},
    {'o', "output", __args_set_field_output, false, "FILE",
     "Path to CSV metrics log", "Instrumentation & output", NULL, 0,
     arg_str(NULL), true},
    {'\0', "display", __args_set_field_display, false, NULL,
     "Display a progress bar", "Instrumentation & output", NULL, 0,
     arg_bool(false), true},
    {'h', "help", NULL, false, NULL, "Show this help text and exit",
     "Instrumentation & output", NULL, 0, arg_bool(false), false},
};

usize spec_count = sizeof(specs) / sizeof(ArgSpec);

#pragma endregion GLOBALS

#pragma region PARSE FUNCS

void parse_argval(const char *argval, Args *args, ArgSpec *spec) {
    if (spec->setter == NULL) {
        panic("ArgSpec does not include setter function");
    }

    switch (spec_type(*spec)) {
    case ARG_TYPE_INT:
        spec->setter(args, arg_int(parse_int(argval)));
        break;
    case ARG_TYPE_TIME:
        spec->setter(args, arg_time(parse_duration(argval)));
        break;
    case ARG_TYPE_SIZE:
        spec->setter(args, arg_size(parse_size(argval)));
        break;
    case ARG_TYPE_FLOAT:
        spec->setter(args, arg_float(parse_float(argval)));
        break;
    case ARG_TYPE_ENUM:
        spec->setter(args, arg_enum(parse_enum(argval, spec)));
        break;
    case ARG_TYPE_INT_LIST:
        spec->setter(args, arg_intlist_v(parse_int_list(argval)));
        break;
    case ARG_TYPE_SIZE_LIST:
        spec->setter(args, arg_sizelist_v(parse_size_list(argval)));
        break;
    case ARG_TYPE_STR:
        spec->setter(args, arg_str(argval));
        break;
    case ARG_TYPE_BOOL:
        spec->setter(args, arg_bool(true));
        break;
    default:
        fatal("Unknown argument type (%d)", spec_type(*spec));
    }
}

int parse_enum(const char *argval, ArgSpec *spec) {
    for (usize i = 0; i < spec->choices_count; i++) {
        if (strcmp(spec->choices[i], argval) == 0) {
            return (int)i;
        }
    }

    fatal("Invalid enum option: %s", argval);
}

Int parse_duration(const char *argval) {
    if (argval == NULL || *argval == '\0') {
        return 0;
    }

    char *endp;
    Int duration = (Int)strtoul(argval, &endp, 0);
    if (argval == endp) {
        fatal("Invalid duration");
    }

    if (*endp != '\0') {
        if (strcmp("s", endp) == 0) {
        } else if (strcmp("min", endp) == 0) {
            duration *= 60;
        } else {
            fatal("Supported units for duration are (s)econds and (min)utes");
        }
    }

    return duration;
}

Int parse_size(const char *argval) {
    if (argval == NULL || *argval == '\0') {
        return 0;
    }

    char *endp;
    Int size = (Int)strtoul(argval, &endp, 0);
    if (argval == endp) {
        fatal("Invalid size");
    }

    if (*endp != '\0') {
        if (strcmp("B", endp) == 0) {
        } else if (strcmp("KB", endp) == 0) {
            size *= (Int)1e3;
        } else if (strcmp("KiB", endp) == 0) {
            size *= 1 << 10;
        } else if (strcmp("MB", endp) == 0) {
            size *= (Int)1e6;
        } else if (strcmp("MiB", endp) == 0) {
            size *= 1 << 20;
        } else if (strcmp("GB", endp) == 0) {
            size *= (Int)1e9;
        } else if (strcmp("GiB", endp) == 0) {
            size *= 1 << 30;
        } else {
            fatal("Supported units for size are B, KB, KiB, MB, MiB, GB, GiB");
        }
    }

    return size;
}

Int parse_int(const char *argval) {
    if (argval == NULL || *argval == '\0') {
        return 0;
    }

    char *endp;
    Int num = (Int)strtoul(argval, &endp, 0);

    if (argval == endp) {
        fatal("Invalid integer");
    }

    return num;
}

Float parse_float(const char *argval) {
    char *endp;
    Float num = (Float)strtod(argval, &endp);

    if (argval == endp) {
        fatal("Invalid float");
    }

    return num;
}

IntList parse_int_list(const char *argval) {
    isize idx = str_find(argval, ',');

    IntList l = {0};

    const char *p = argval;
    while (idx >= 0) {
        list_append(&l, parse_int(p));
        p = &p[idx + 1];
        idx = str_find(p, ',');
    }

    list_append(&l, parse_int(p));
    return l;
}

IntList parse_size_list(const char *argval) {
    isize idx = str_find(argval, ',');

    IntList l = {0};

    char *p = (char *)argval;
    while (idx >= 0) {
        char pp = p[idx];
        p[idx] = '\0';
        list_append(&l, parse_size(p));
        p[idx] = pp;
        p = &p[idx + 1];
        idx = str_find(p, ',');
    }

    list_append(&l, parse_size(p));
    return l;
}

#pragma endregion PARSE FUNCS

#pragma region MAIN FUNCS

static void check_args(Args *args, Int iter);

static char size_buf[1024];
static char prec_buf[16];
char *size_str(Int size, u8 precision) {
    snprintf(prec_buf, ARRAY_LEN(prec_buf), "%%.%df%%s", precision);

    if (size < (1 << 10)) {
        snprintf(size_buf, ARRAY_LEN(size_buf), "%luB", size);
    } else if (size < (1UL << 20)) {
        snprintf(size_buf, ARRAY_LEN(size_buf), prec_buf,
                 ((double)size) / (1UL << 10), "KiB");
    } else if (size < (1UL << 30)) {
        snprintf(size_buf, ARRAY_LEN(size_buf), prec_buf,
                 ((double)size) / (1UL << 20), "MiB");
    } else if (size < (1UL << 40)) {
        snprintf(size_buf, ARRAY_LEN(size_buf), prec_buf,
                 ((double)size) / (1UL << 30), "GiB");
    } else {
        snprintf(size_buf, ARRAY_LEN(size_buf), "%luB", size);
    }

    return size_buf;
}

void print_usage(const char *progname, const ArgSpec *specs,
                 size_t spec_count) {
    println("Usage: %s [options]\n", progname);

    println(
        "  <BYTES>  supports units B, KB, KiB, MB, MiB, GB, GiB (e.g. 10KB)\n"
        "  <SECS>   supports units s, min (e.g. 1min)\n"
        "  <L[...]> comma-separeted list (e.g. 1,2,3,4)");

    const char *last_cat = NULL;
    for (size_t i = 0; i < spec_count; i++) {
        const ArgSpec *s = &specs[i];

        if (!last_cat || strcmp(s->category, last_cat) != 0) {
            println("\n%s:", s->category);
            last_cat = s->category;
        }

        char shortbuf[8] = "";
        if (s->short_opt) {
            snprintf(shortbuf, sizeof(shortbuf), "-%c,", s->short_opt);
        }

        char metavar[16] = "";
        if (spec_type(*s) != ARG_TYPE_BOOL && s->metavar) {
            snprintf(metavar, sizeof(metavar), " %s", s->metavar);
        }

        char enumlist[16] = "";
        if (spec_type(*s) == ARG_TYPE_ENUM && s->choices_count > 0) {
            strcat(enumlist, "; one of:");
        }

        println("  %-4s--%-13s%-9s %s%s", shortbuf, s->long_opt, metavar,
                s->help, enumlist);

        if (s->has_default) {
            switch (spec_type(*s)) {
            case ARG_TYPE_BOOL:
                println("%32s (default: %s)", "",
                        (s->default_val.as.b) ? "true" : "false");
                break;
            case ARG_TYPE_FLOAT:
                println("%32s (default: %.2f)", "", s->default_val.as.f);
                break;
            case ARG_TYPE_INT:
                println("%32s (default: %lu)", "", s->default_val.as.i);
                break;
            case ARG_TYPE_TIME:
                if (s->default_val.as.i < 60) {
                    println("%32s (default: %lus)", "", s->default_val.as.i);
                } else if (s->default_val.as.i % 60 == 0) {
                    println("%32s (default: %lumin)", "",
                            s->default_val.as.i / 60);
                } else {
                    println("%32s (default: %lumin %lus)", "",
                            s->default_val.as.i / 60, s->default_val.as.i % 60);
                }
                break;
            case ARG_TYPE_SIZE:
                println("%32s (default: %s)", "",
                        size_str(s->default_val.as.i, 1));
                break;
            case ARG_TYPE_STR:
                if (s->default_val.as.s != NULL) {
                    println("%32s (default: %s)", "", s->default_val.as.s);
                }
                break;
            case ARG_TYPE_ENUM:
                for (usize i = 0; i < s->choices_count; i++) {
                    println("%32s - %s%s", "", s->choices[i],
                            (i == s->default_val.as.i) ? " (default)" : "");
                }
                break;
            case ARG_TYPE_INT_LIST:
            case ARG_TYPE_SIZE_LIST:
                if (s->default_val.as.il.count > 0) {
                    todo("default int/size list");
                }
                break;
            default:
                panic("Unknown argument type %d", spec_type(*s));
                break;
            }
        }
    }
    println();
}

void parse_args(int argc, char **argv, Args *args, ArgSpec *specs,
                usize spec_count) {

    const char *program = shift(argv, argc);

    // ============================ SET DEFAULTS ============================

    log("Start setting defaults");

    for (usize i = 0; i < spec_count; i++) {
        ArgSpec *spec = &specs[i];
        if (!spec->has_default) {
            continue;
        }
        switch (spec_type(*spec)) {
        case ARG_TYPE_INT:
        case ARG_TYPE_TIME:
        case ARG_TYPE_SIZE:
            log("Setting int|time|size");
            spec->setter(args, arg_int(spec->default_val.as.i));
            break;
        case ARG_TYPE_FLOAT:
            log("Setting float");
            spec->setter(args, arg_float(spec->default_val.as.f));
            break;
        case ARG_TYPE_STR:
            log("Setting str");
            spec->setter(args, arg_str(spec->default_val.as.s));
            break;
        case ARG_TYPE_BOOL:
            log("Setting bool");
            spec->setter(args, arg_bool(spec->default_val.as.b));
            break;
        case ARG_TYPE_ENUM:
            log("Setting enum");
            spec->setter(args, arg_enum(spec->default_val.as.e));
            break;
        case ARG_TYPE_INT_LIST:
        case ARG_TYPE_SIZE_LIST:
            log("Setting int-list|size-list");
            spec->setter(args, spec->default_val);
            break;
        default:
            fatal("Unknown argument type");
            break;
        }
    }

    Int iter = args->iterations.as.i;
    args->iterations = arg_int(0);
    // ======================================================================

    log("Start parsing args");

    while (argc > 0) {
        const char *arg = shift(argv, argc);

        if (*arg != '-') {
            fatal("Unexpected argument %s", arg);
        }

        if ((*(arg + 1) == '-') && strcmp(arg + 2, "help") == 0) {
            print_usage(program, specs, spec_count);
            exit(0);
        }
        if (*(arg + 1) == 'h') {
            print_usage(program, specs, spec_count);
            exit(0);
        }

        usize i = (usize)-1;
        u32 off = 0;
        for (usize j = 0; j < spec_count; j++) {
            if ((*(arg + 1) == '-') &&
                (strcmp(arg + 2, specs[j].long_opt) == 0)) {
                i = j;
                break;
            }
            if ((*(arg + 1) == specs[j].short_opt) && (strlen(arg + 1) == 1)) {
                i = j;
                break;
            }

            usize opt_len = strlen(specs[j].long_opt);
            if ((*(arg + 1) == '-') &&
                (strncmp(arg + 2, specs[j].long_opt, opt_len) == 0)) {
                off = opt_len + 2;
                i = j;
                break;
            }

            if (*(arg + 1) == specs[j].short_opt) {
                off = 2;
                i = j;
                break;
            }
        }

        if (i == (usize)-1) {
            fatal("Unexpected argument %s", arg);
        }

        ArgSpec *spec = &specs[i];

        if ((off == 0) && (spec_type(*spec) != ARG_TYPE_BOOL)) {
            arg = shift(argv, argc);
        } else if (off > 0) {
            arg = arg + off;
            if (*arg == '=') {
                arg++;
            }
        }

        log("Arg value = %s", arg);

        parse_argval(arg, args, spec);
    }

    check_args(args, iter);
}

static void check_args(Args *args, Int iter) {
    if (args->min_size.as.i > args->max_size.as.i) {
        args->max_size = args->min_size;
    }

    if (args->iterations.as.i == 0 && args->duration_sec.as.i == 0) {
        args->iterations.as.i = iter;
    } else if (args->iterations.as.i != 0 && args->duration_sec.as.i != 0) {
        fatal("Arguments --iterations and --duration are mutualy exclusive");
    }

    if (args->seed.as.i == 0) {
        args->seed = arg_int((Int)time(NULL));
    }

    if ((args->distribution.as.e == DISTRIBUTION_POWERLAW) &&
        (args->dist_param.as.f == 0.0f)) {
        fatal("Alpha parameter for powerlaw distribution should not be 0");
    }

    if (((args->ttl_list.as.il.count > 0) && (args->ttl_weights.as.il.count > 0)) &&
        (args->ttl_list.as.il.count != args->ttl_weights.as.il.count)) {
        fatal("Number of weights doesn't match to the number of items in "
              "lifetime list. W(%u) != L(%u)",
              args->ttl_weights.as.il.count, args->ttl_list.as.il.count);
    }

    if (((args->size_list.as.il.count > 0) &&
         (args->size_weights.as.il.count > 0)) &&
        (args->size_list.as.il.count != args->size_weights.as.il.count)) {
        fatal("Number of weights doesn't match to the number of items in "
              "size-list. W(%u) != L(%u)",
              args->size_weights.as.il.count, args->size_list.as.il.count);
    }

    if (args->size_step.as.i == 0) {
        fatal("--size-step should not be zero");
    }

    if (args->ttl_fixed.as.i == 0) {
        fatal("--ttl-fixed should not be zero");
    }

    if ((args->policy.as.e == POLICY_NEVER) &&
        (args->ttl_mode.as.e == TTL_OFF)) {
        fatal("If --policy is 'never', then --ttl-mode should not be off");
    }

    if (args->alloc_freq.as.f < 0 || args->alloc_freq.as.f > 1) {
        fatal("--alloc-freq should be on the interval [0, 1], but is %f",
              args->alloc_freq.as.f);
    }
}

#define A(name)                                                                \
    if (((args->name.t == ARG_TYPE_INT_LIST) ||                                \
         (args->name.t == ARG_TYPE_SIZE_LIST)) &&                              \
        (args->name.as.il.capacity > 0)) {                                     \
        list_free(args->name.as.il);                                           \
    }
void free_args(Args *args) { ARGUMENTS }
#undef A

#pragma endregion MAIN FUNCS

#pragma region LOGGING

static char log_buf[2048];

static char *str_int_list(const IntList *l) {
    if (l == NULL) {
        return NULL;
    }

    snprintf(log_buf, sizeof(log_buf), "[");
    u32 buf_idx = 1;
    if (l->count > 0) {
        int b = snprintf(log_buf + buf_idx, sizeof(log_buf) - buf_idx, "%zu",
                         l->items[0]);
        if (b >= 0) {
            buf_idx += b;
        }
    }
    for (u32 i = 1; i < l->count; i++) {
        int b = -1;
        b = snprintf(log_buf + buf_idx, sizeof(log_buf) - buf_idx, ",%zu",
                     l->items[i]);

        if (b < 1) {
            break;
        }
        buf_idx += b;
    }
    snprintf(log_buf + buf_idx, sizeof(log_buf) - buf_idx, "]");

    return log_buf;
}

void log_args(const Args *args) {
    if (args->policy.as.e >= POLICY_COUNT) {
        log_debug("args.policy = unknown(%u)", args->policy.as.e);
    } else {
        log_debug("args.policy = %s", policies[args->policy.as.e]);
    }
    log_debug("args.iterations = %zu", args->iterations.as.i);
    log_debug("args.duration_sec = %zu", args->duration_sec.as.i);
    log_debug("args.alloc_freq = %f", args->alloc_freq.as.f);
    log_debug("args.seed = %zu", args->seed.as.i);

    log_debug("args.capacity = %zu", args->capacity.as.i);

    log_debug("args.min_size = %zu", args->min_size.as.i);
    log_debug("args.max_size = %zu", args->max_size.as.i);

    if (args->size_trend.as.e >= TREND_COUNT) {
        log_debug("args.size_trend = unknown(%u)", args->size_trend.as.e);
    } else {
        log_debug("args.size_trend = %s", trends[args->size_trend.as.e]);
    }
    log_debug("args.size_step = %zu", args->size_step.as.i);
    log_debug("args.trend_jitter = %zu", args->trend_jitter.as.i);

    log_debug("args.size_list = %s", str_int_list(&args->size_list.as.il));
    if (args->size_mode.as.e >= SIZE_LIST_MODE_COUNT) {
        log_debug("args.size_mode = unknown(%u)", args->size_mode.as.e);
    } else {
        log_debug("args.size_mode = %s", trends[args->size_mode.as.e]);
    }
    log_debug("args.size_weights = %s",
              str_int_list(&args->size_weights.as.il));

    if (args->distribution.as.e >= DISTRIBUTION_COUNT) {
        log_debug("args.distribution = unknown(%u)", args->distribution.as.e);
    } else {
        log_debug("args.distribution = %s",
                  distributions[args->distribution.as.e]);
    }

    log_debug("args.dist_param = %f", args->dist_param.as.f);

    if (args->ttl_mode.as.e >= TTL_COUNT) {
        log_debug("args.ttl_mode = unknown(%u)", args->ttl_mode.as.e);
    } else {
        log_debug("args.ttl_mode = %s", ttls[args->ttl_mode.as.e]);
    }
    log_debug("args.ttl_fixed = %zu", args->ttl_fixed.as.i);
    log_debug("args.ttl_list = %s", str_int_list(&args->ttl_list.as.il));
    log_debug("args.ttl_weights = %s", str_int_list(&args->ttl_weights.as.il));

    log_debug("args.snap_interval = %zu", args->snap_interval.as.i);
    log_debug("args.output = %s", args->output.as.s);
    log_debug("args.display = %d", args->display.as.b);
}

#pragma endregion LOGGING

#if defined(__cplusplus)
}
#endif

#undef log
#undef NO_UNDEF_ARGS
