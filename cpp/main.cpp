#ifndef LOG_LEVEL
#define LOG_LEVEL 0
#endif

#include "../c/utils/args_parser.h"
#include "../c/utils/common.h"
#include "../c/utils/logging.h"

#include "utils/common.hpp"
#include "utils/progress.hpp"

#include "actions/actions.hpp"
#include "pool/pool.hpp"
#include "random/random.hpp"
#include "tracker/tracker.hpp"

using Tracker = tracker::Tracker;

Random rng;

int main(int argc, char *argv[]) {
    parse_args(argc, argv, &global_args, specs, spec_count);

    Args args = global_args;

    std::ofstream output;
    if (args.output.as.s != nullptr && args.snap_interval.as.i > 0) {
        output.open(args.output.as.s, std::ios::trunc | std::ios::out);
#if defined(LOG_LEVEL)
        if (!output) {
            perror(ERROR_STR " Could not open file");
        }
#endif
    }

    rng = Random(args.seed.as.i);

    Tracker &tracker = Tracker::instance();
    if (output.is_open()) {
        tracker.writeHeader(output);
        tracker.init();
        tracker.write(output);
    }
    action::init_actions(args);

    {
        Pool pool = Pool(args.capacity.as.i);

        utils::ProgressBar progress =
            utils::ProgressBar::from_iterations(args.iterations.as.i);
        if (args.duration_sec.as.i > 0) {
            progress =
                utils::ProgressBar::from_duration(args.duration_sec.as.i);
        }

        progress.display(args.display.as.b);

        Int interval = (args.snap_interval.as.i > 0) ? args.snap_interval.as.i : 1;
        while (progress.has_next()) {
            usize i = progress.next();
            action::block_action(pool, args, rng);

            if (output.is_open() && ((i % interval) == 0)) {
                tracker.write(output);
            }
        }

        progress.finish();

        free_args(&args);
    }
    
    if (output.is_open()) {
        tracker.write(output);
        output.flush();
        output.close();
    }
    return 0;
}
