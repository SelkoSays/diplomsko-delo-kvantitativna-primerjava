#ifndef PROGRESS_HPP
#define PROGRESS_HPP

#include "../../c/utils/common.h"
#include "common.hpp"
#include <chrono>
#include <cstdio>

namespace utils {

enum class ProgressMode { Iterations, Duration };

class ProgressBar {
  private:
    ProgressMode mode;
    usize target;
    usize current;
    usize width;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point last_update;
    std::chrono::milliseconds update_interval;
    bool displayed;

    void draw() const;
    double get_progress() const;

  public:
    static ProgressBar from_iterations(usize iterations);
    static ProgressBar from_duration(usize duration_sec);
    static ProgressBar from_iterations(usize iterations, usize width);
    static ProgressBar from_duration(usize duration_sec, usize width);

    ProgressBar &display(bool display) {
        self.displayed = display;
        return self;
    }

    void update();
    bool is_finished() const;
    void finish();

    bool has_next() const;
    usize next();

  private:
    ProgressBar(ProgressMode mode, usize target, usize width);
};

} // namespace utils

#endif // PROGRESS_HPP