#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

// Matches your names; CHECK_CLOCK means "high-res" path.
// In C++ we use steady_clock; in C we use clock()/time().
enum TimeCheckOpt { CHECK_CLOCK = 0, CHECK_TIME = 1 };

typedef struct {
    i64 start;               // units depend on check_opt (see below)
    i64 end;                 // ...
    enum TimeCheckOpt check_opt; // CHECK_CLOCK: ns (C++) or clock ticks (C)
                                 // CHECK_TIME: seconds since epoch (C)
} Timer;

static inline void timer_reset(Timer *t) {
    t->start = 0;
    t->end = 0;
    t->check_opt = CHECK_CLOCK;
}

#ifdef __cplusplus
// ---------------- C++ implementation (monotonic, nanoseconds) ----------------
#include <chrono>

static inline void timer_start_for(Timer *t, i64 duration_sec) {
    using clock = std::chrono::steady_clock;
    using ns    = std::chrono::nanoseconds;
    const i64 dur_ns = duration_sec * (i64)1000000000;
    const i64 now_ns =
        std::chrono::duration_cast<ns>(clock::now().time_since_epoch()).count();

    t->check_opt = CHECK_CLOCK;                  // we always use the steady clock
    t->start = now_ns;                           // nanoseconds
    t->end   = now_ns + dur_ns;                  // nanoseconds
}

static inline bool timer_is_finished(Timer *t) {
    using clock = std::chrono::steady_clock;
    using ns    = std::chrono::nanoseconds;
    const i64 now_ns =
        std::chrono::duration_cast<ns>(clock::now().time_since_epoch()).count();

    // With steady_clock, we never fall back; just compare ns.
    return now_ns >= t->end;
}

#else
// ---------------- C implementation (clock()/time() with fallback) ------------
#include <time.h>

static inline void timer_start_for(Timer *t, i64 duration_sec) {
    t->check_opt = CHECK_CLOCK;

    clock_t c = clock();
    if (c != (clock_t)-1) {
        t->start = (i64)c;                                   // clock ticks
        t->end   = t->start + duration_sec * (i64)CLOCKS_PER_SEC;
        return;
    }

    // fallback to wall clock (seconds)
    t->check_opt = CHECK_TIME;
    time_t now = time(NULL);
    t->start = (i64)now;
    t->end   = t->start + duration_sec;
}

static inline bool timer_is_finished(Timer *t) {
    if (t->check_opt == CHECK_CLOCK) {
        clock_t c = clock();
        if (c != (clock_t)-1) {
            return (i64)c >= t->end; // both in clock ticks
        }

        // fallback to seconds if clock() stopped working mid-run
        t->check_opt = CHECK_TIME;
        t->start = t->start / (i64)CLOCKS_PER_SEC;
        t->end   = t->end   / (i64)CLOCKS_PER_SEC;
    }

    time_t now = time(NULL);
    return (i64)now >= t->end; // seconds
}
#endif

#endif /* TIMER_H */
