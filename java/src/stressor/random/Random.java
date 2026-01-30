package stressor.random;

import java.util.List;

import stressor.utils.ArgsParser.Distribution;

public class Random {
    private static final Random INSTANCE = new Random();

    private long random_state = 0x853c49e6748fea9bL;

    Random() {
    }

    public static Random get() {
        return INSTANCE;
    }

    public void seed(long seed) {
        if (seed == 0) {
            long time = System.currentTimeMillis() / 1000L;
            seed = time;
        }
        random_state = splitmix64(seed);
        if (random_state == 0) {
            random_state = 0x9e3779b97f4a7c15L;
        }
    }

    public boolean coin_flip() {
        return (uniform(0, 2) % 2) == 1;
    }

    public double uniform01() {
        long r = xs64star();
        r = shlu(r, 11);
        assert r >= 0;
        return (double) r * (1.0 / (double) (1L << 53));
    }

    public long uniform(long min, long max) {
        assert min <= max;

        var u = uniform01();
        return min + (long) ((double) (max - min) * u);
    }

    public long exp(long min, long max, double lambda) {
        assert min <= max;
        assert lambda > 0.0;

        var u = uniform01();
        double x = Math.log(1 - u) / lambda;

        return Math.clamp((long) x, min, max);
    }

    public long powerlaw(long min, long max, double alpha) {
        assert min <= max;

        var u = uniform01();
        double x;
        if (Math.abs(alpha - 1.0) < 1e-8) {
            double ratio = (double) max / (double) min;
            x = (double) min * Math.pow(ratio, u);
        } else {
            var exp = 1.0 - alpha;
            var min_e = Math.pow(min, exp);
            var max_e = Math.pow(max, exp);
            var val_e = min_e + u * (max_e - min_e);
            x = Math.pow(val_e, 1.0 / exp);
        }

        return Math.clamp((long) x, min, max);
    }

    public long next(long min, long max, double param,
            Distribution distribution) {
        switch (distribution) {
            case UNIFORM:
                return uniform(min, max);
            case EXP:
                return exp(min, max, param);
            case POWERLAW:
                return powerlaw(min, max, param);
        }

        throw new RuntimeException("Unreachable(Random.next)");
    }

    public long choice(List<Long> l) {
        assert l.size() > 0;
        return l.get((int) this.uniform(0, l.size()));
    }

    public long choice(List<Long> l, List<Long> weights) {
        assert l.size() > 0;

        if (weights.size() == 0) {
            return choice(l);
        }

        long vote = uniform(0, weights.stream().reduce(0L, Long::sum));

        long a = 0;
        int idx = 0;

        for (int i = 0; i < weights.size(); i++) {
            a += weights.get(i);
            if (vote < a) {
                idx = i;
                break;
            }
        }

        return l.get(idx);
    }

    // =============== PRIVATE ===============

    private static long splitmix64(long x) {
        long z = x + 0x9e3779b97f4a7c15L;
        z = (z ^ shlu(z, 30)) * 0xbf58476d1ce4e5b9L;
        z = (z ^ shlu(z, 27)) * 0x94d049bb133111ebL;
        return z ^ shlu(z, 31);
    }

    private long xs64star() {
        long x = random_state;
        x ^= shlu(x, 12);
        x ^= x << 25;
        x ^= shlu(x, 27);
        random_state = x;
        return x * 2685821657736338717L;
    }

    private static long shlu(long x, int shmnt) {
        // long bit = x & (1L << 63);
        // x ^= bit;
        // x = x >> shmnt;
        // if (bit != 0) {
        // x |= 1L << (63 - shmnt);
        // }
        return x >>> shmnt;
    }
}
