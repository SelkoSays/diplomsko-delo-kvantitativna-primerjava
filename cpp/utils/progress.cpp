#include "progress.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>

namespace utils {

ProgressBar::ProgressBar(ProgressMode mode, usize target, usize width)
    : mode(mode), target(target), current(0), width(width),
      start_time(std::chrono::steady_clock::now()),
      last_update(std::chrono::steady_clock::now()),
      update_interval(std::chrono::milliseconds(100)), displayed(false) {}

ProgressBar ProgressBar::from_iterations(usize iterations) {
    return ProgressBar(ProgressMode::Iterations, iterations, 50);
}

ProgressBar ProgressBar::from_duration(usize duration_sec) {
    return ProgressBar(ProgressMode::Duration, duration_sec, 50);
}

ProgressBar ProgressBar::from_iterations(usize iterations, usize width) {
    return ProgressBar(ProgressMode::Iterations, iterations, width);
}

ProgressBar ProgressBar::from_duration(usize duration_sec, usize width) {
    return ProgressBar(ProgressMode::Duration, duration_sec, width);
}

double ProgressBar::get_progress() const {
    switch (mode) {
    case ProgressMode::Iterations:
        return target > 0 ? std::min(1.0, static_cast<double>(current) / target)
                          : 1.0;
    case ProgressMode::Duration: {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time);
        double elapsed_sec = elapsed.count() / 1000.0;
        return target > 0 ? std::min(1.0, elapsed_sec / target) : 1.0;
    }
    default:
        return 1.0;
    }
}

bool ProgressBar::is_finished() const {
    switch (mode) {
    case ProgressMode::Iterations:
        return current >= target;
    case ProgressMode::Duration: {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - start_time);
        return elapsed.count() >= static_cast<long>(target);
    }
    default:
        return true;
    }
}

void ProgressBar::draw() const {
    if (!self.displayed) {
        return;
    }

    double progress = get_progress();
    usize percent = static_cast<usize>(progress * 100);
    usize filled = static_cast<usize>(progress * width);

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);
    double elapsed_sec = elapsed.count() / 1000.0;
    double rate = elapsed_sec > 0.0 ? current / elapsed_sec : 0.0;

    std::cout << "\r[";
    for (usize i = 0; i < width; i++) {
        if (i < filled) {
            std::cout << "=";
        } else if (i == filled && filled < width) {
            std::cout << ">";
        } else {
            std::cout << " ";
        }
    }
    std::cout << "] ";

    switch (mode) {
    case ProgressMode::Iterations:
        std::cout << current << "/" << target << " (" << std::setw(3) << percent
                  << "%) ";
        break;
    case ProgressMode::Duration:
        std::cout << std::fixed << std::setprecision(1) << elapsed_sec << "s/"
                  << static_cast<double>(target) << "s (" << std::setw(3)
                  << percent << "%) ";
        break;
    }

    if (rate > 0.0) {
        std::cout << std::fixed << std::setprecision(1) << rate << "/s ";
    }

    switch (mode) {
    case ProgressMode::Iterations:
        if (rate > 0.0 && current < target) {
            long eta = static_cast<long>((target - current) / rate);
            std::cout << "ETA: " << std::setfill('0') << std::setw(2)
                      << eta / 60 << ":" << std::setw(2) << eta % 60;
        }
        break;
    case ProgressMode::Duration: {
        long remaining_sec =
            static_cast<long>(target) - static_cast<long>(elapsed_sec);
        if (remaining_sec < 0)
            remaining_sec = 0;
        std::cout << "ETA: " << std::setfill('0') << std::setw(2)
                  << remaining_sec / 60 << ":" << std::setw(2)
                  << remaining_sec % 60;
        break;
    }
    }

    std::cout << std::setfill(' ');
    std::cout.flush();
}

void ProgressBar::update() {
    current++;

    auto now = std::chrono::steady_clock::now();

    if (now - last_update >= update_interval || is_finished()) {
        draw();
        last_update = now;
    }
}

void ProgressBar::finish() {
    if (self.displayed) {
        draw();
        std::cout << std::endl;
    }
}

bool ProgressBar::has_next() const { return !is_finished(); }

usize ProgressBar::next() {
    if (is_finished()) {
        return current;
    }
    update();
    return current;
}

} // namespace utils