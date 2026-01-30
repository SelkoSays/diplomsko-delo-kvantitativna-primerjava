#include "progress.h"

#include <math.h>

static long timespec_diff_ms(const struct timespec *start,
                             const struct timespec *end) {
    long diff_sec = end->tv_sec - start->tv_sec;
    long diff_nsec = end->tv_nsec - start->tv_nsec;
    return diff_sec * 1000L + diff_nsec / 1000000L;
}

static void get_current_time(struct timespec *ts) {
    clock_gettime(CLOCK_MONOTONIC, ts);
}

static void progress_init(ProgressBar *pb, ProgressMode mode, usize target,
                          usize width) {
    pb->mode = mode;
    pb->target = target;
    pb->current = 0;
    pb->width = width;
    pb->update_interval_ms = 100;
    get_current_time(&pb->start_time);
    pb->last_update = pb->start_time;
    pb->displayed = false;
}

void progress_from_iterations(ProgressBar *pb, usize iterations) {
    progress_init(pb, PROGRESS_MODE_ITERATIONS, iterations, 50);
}

void progress_from_duration(ProgressBar *pb, usize duration_sec) {
    progress_init(pb, PROGRESS_MODE_DURATION, duration_sec, 50);
}

void progress_from_iterations_with_width(ProgressBar *pb, usize iterations,
                                         usize width) {
    progress_init(pb, PROGRESS_MODE_ITERATIONS, iterations, width);
}

void progress_from_duration_with_width(ProgressBar *pb, usize duration_sec,
                                       usize width) {
    progress_init(pb, PROGRESS_MODE_DURATION, duration_sec, width);
}

static double progress_get_progress(const ProgressBar *pb) {
    switch (pb->mode) {
    case PROGRESS_MODE_ITERATIONS:
        return pb->target > 0 ? fmin(1.0, (double)pb->current / pb->target)
                              : 1.0;
    case PROGRESS_MODE_DURATION: {
        struct timespec now;
        get_current_time(&now);
        double elapsed_sec = timespec_diff_ms(&pb->start_time, &now) / 1000.0;
        return pb->target > 0 ? fmin(1.0, elapsed_sec / pb->target) : 1.0;
    }
    default:
        return 1.0;
    }
}

bool progress_is_finished(const ProgressBar *pb) {
    switch (pb->mode) {
    case PROGRESS_MODE_ITERATIONS:
        return pb->current >= pb->target;
    case PROGRESS_MODE_DURATION: {
        struct timespec now;
        get_current_time(&now);
        long elapsed_sec = timespec_diff_ms(&pb->start_time, &now) / 1000;
        return elapsed_sec >= (long)pb->target;
    }
    default:
        return true;
    }
}

static void progress_draw(const ProgressBar *pb) {
    if (!pb->displayed) {
        return;
    }

    double progress = progress_get_progress(pb);
    usize percent = (usize)(progress * 100);
    usize filled = (usize)(progress * pb->width);

    struct timespec now;
    get_current_time(&now);
    double elapsed_sec = timespec_diff_ms(&pb->start_time, &now) / 1000.0;
    double rate = elapsed_sec > 0.0 ? pb->current / elapsed_sec : 0.0;

    printf("\r[");
    for (usize i = 0; i < pb->width; i++) {
        if (i < filled) {
            printf("=");
        } else if (i == filled && filled < pb->width) {
            printf(">");
        } else {
            printf(" ");
        }
    }
    printf("] ");

    switch (pb->mode) {
    case PROGRESS_MODE_ITERATIONS:
        printf("%zu/%zu (%3zu%%) ", pb->current, pb->target, percent);
        break;
    case PROGRESS_MODE_DURATION:
        printf("%.1fs/%.1fs (%3zu%%) ", elapsed_sec, (double)pb->target,
               percent);
        break;
    }

    if (rate > 0.0) {
        printf("%.1f/s ", rate);
    }

    switch (pb->mode) {
    case PROGRESS_MODE_ITERATIONS:
        if (rate > 0.0 && pb->current < pb->target) {
            long eta = (long)((pb->target - pb->current) / rate);
            printf("ETA: %02ld:%02ld", eta / 60, eta % 60);
        }
        break;
    case PROGRESS_MODE_DURATION: {
        long remaining_sec = (long)pb->target - (long)elapsed_sec;
        if (remaining_sec < 0)
            remaining_sec = 0;
        printf("ETA: %02ld:%02ld", remaining_sec / 60, remaining_sec % 60);
        break;
    }
    }

    fflush(stdout);
}

void progress_update(ProgressBar *pb) {
    pb->current++;

    struct timespec now;
    get_current_time(&now);

    if (timespec_diff_ms(&pb->last_update, &now) >= pb->update_interval_ms ||
        progress_is_finished(pb)) {
        progress_draw(pb);
        pb->last_update = now;
    }
}

void progress_finish(ProgressBar *pb) {
    if (pb->displayed) {
        progress_draw(pb);
        printf("\n");
    }
}

bool progress_has_next(const ProgressBar *pb) {
    return !progress_is_finished(pb);
}

usize progress_next(ProgressBar *pb) {
    if (progress_is_finished(pb)) {
        return pb->current;
    }
    progress_update(pb);
    return pb->current;
}
