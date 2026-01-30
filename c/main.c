#include "utils/common.h"
#include "utils/logging.h"

#include "pool/pool.h"
#include "random/random.h"
#include "utils/args_parser.h"
#include "utils/list.h"
#include "actions/actions.h"
#include "utils/progress.h"

extern Tracker global_tracker;

int main(int argc, char *argv[]) {
    parse_args(argc, argv, &global_args, specs, spec_count);

    Args args = global_args;

    FILE *output = NULL;
    if (args.output.as.s != NULL && args.snap_interval.as.i > 0) {
        output = fopen(args.output.as.s, "w+");
#if defined(LOG_LEVEL)
        if (output == NULL) {
            perror(ERROR_STR " Could not open file");
        }
#endif
    }

    tracker_write_header(output);
    tracker_write(&global_tracker, output);

    random_seed(args.seed.as.i);

    Pool pool;
    pool_init(&pool, args.capacity.as.i);
    init_actions(&args);

    ProgressBar progress;
    if (args.duration_sec.as.i > 0) {
        progress_from_duration(&progress, args.duration_sec.as.i);
    } else {
        progress_from_iterations(&progress, args.iterations.as.i);
    }

    progress.displayed = args.display.as.b;

    Int interval = (args.snap_interval.as.i > 0) ? args.snap_interval.as.i : 1;
    usize i = 0;
    while (progress_has_next(&progress)) {
        i = progress_next(&progress);
        block_action(&pool, &args);

        if ((i % interval) == 0) {
            tracker_write(&global_tracker, output);
        }
    }

    progress_finish(&progress);
    pool_free(&pool);
    free_args(&args);
    tracker_write(&global_tracker, output);

    if (output != NULL) {
        fflush(output);
        fclose(output);
    }
    return 0;
}
