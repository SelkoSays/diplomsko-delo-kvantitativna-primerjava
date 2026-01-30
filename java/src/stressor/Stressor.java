package stressor;

import stressor.actions.Actions;
import stressor.pool.Pool;
import stressor.random.Random;
import stressor.utils.ArgsParser;
import stressor.utils.ProgressBar;
import stressor.tracker.Tracker;

import java.io.FileNotFoundException;
import java.io.PrintWriter;

public final class Stressor {

    public static void main(String[] argv) throws Exception {
        ArgsParser.Args args = ArgsParser.parseOrExit(argv);

        Random.get().seed(args.seed.asInt());
        Pool pool = new Pool((int) args.capacity.asInt());
        Actions actions = new Actions(args, pool);

        PrintWriter output = null;
        final Tracker trk = Tracker.get();
        if (args.output != null && args.output.asStr() != null && !args.output.asStr().isEmpty() && args.snap_interval.asInt() > 0) {
            try {
                output = new PrintWriter(args.output.asStr());
            } catch (FileNotFoundException e) {
                System.err.println(e);
                output = null;
            }
            trk.writeHeader(output);
            trk.write(output);
        }

        final long sampleEvery = Math.max(1, args.snap_interval.asInt());

        ProgressBar progress = null;
        if (args.duration_sec.asTime() > 0) {
            progress = ProgressBar.fromDuration(args.duration_sec.asTime());
        } else {
            progress = ProgressBar.fromIterations(args.iterations.asInt());
        }

        progress.display(args.display.asBool());

        for (var i : progress) {
            actions.blockAction();
            if (output != null && (i % sampleEvery == 0)) {
                trk.write(output);
            }
        }

        progress.finish();

        pool.deinit();

        trk.write(output);

        if (output != null) {
            output.flush();
            output.close();
        }
    }
}
