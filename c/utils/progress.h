#ifndef PROGRESS_H
#define PROGRESS_H

#include "common.h"

typedef enum {
    PROGRESS_MODE_ITERATIONS,
    PROGRESS_MODE_DURATION
} ProgressMode;

typedef struct {
    ProgressMode mode;
    usize target; // iterations count or duration in seconds
    usize current;
    usize width;
    struct timespec start_time;
    struct timespec last_update;
    long update_interval_ms;
    bool displayed;
} ProgressBar;

void progress_from_iterations(ProgressBar *pb, usize iterations);
void progress_from_duration(ProgressBar *pb, usize duration_sec);
void progress_from_iterations_with_width(ProgressBar *pb, usize iterations, usize width);
void progress_from_duration_with_width(ProgressBar *pb, usize duration_sec, usize width);

void progress_update(ProgressBar *pb);
bool progress_is_finished(const ProgressBar *pb);
void progress_finish(ProgressBar *pb);

bool progress_has_next(const ProgressBar *pb);
usize progress_next(ProgressBar *pb);

#endif // PROGRESS_H
