package stressor.actions;

import stressor.pool.Pool;
import stressor.random.Random;
import stressor.utils.ArgsParser.*;

import java.util.List;

public class Actions {
    private long curBlockSize;
    private final Args args;
    private final Pool pool;

    public Actions(Args args, Pool pool) {
        this.args = args;
        this.pool = pool;

        // Initialize current block size based on trend
        Trend trend = args.size_trend.asEnum();
        switch (trend) {
            case NONE:
                this.curBlockSize = 0;
                break;
            case GROW:
                this.curBlockSize = args.min_size.asSize();
                break;
            case SHRINK:
                this.curBlockSize = args.max_size.asSize();
                break;
            case SAW:
                this.curBlockSize = args.min_size.asSize();
                break;
            default:
                this.curBlockSize = 0;
        }
    }

    public void blockAction() {
        TTLMode ttlMode = args.ttl_mode.asEnum();
        if (ttlMode != TTLMode.OFF) {
            pool.updateAndPrune();
        }

        boolean shouldAlloc = shouldAlloc();

        if (!shouldAlloc) {
            Policy policy = args.policy.asEnum();
            pool.delBlock(policy);
        } else {
            int blockSize = (int) getBlockSize();
            long blockTtl = getBlockTtl();
            pool.addBlock(blockSize, blockTtl);
        }
    }

    private long trendBlockSize() {
        long jitterRange = args.trend_jitter.asSize() * 2;
        long jitter = Random.get().uniform(0, jitterRange) - args.trend_jitter.asSize();

        Trend trend = args.size_trend.asEnum();
        Distribution dist = args.distribution.asEnum();

        switch (trend) {
            case NONE:
                return Random.get().next(
                        args.min_size.asSize(),
                        args.max_size.asSize(),
                        args.dist_param.asFloat(),
                        dist);

            case GROW: {
                long tmp = curBlockSize;
                long step = args.size_step.asSize() + jitter;
                if (step <= 0) {
                    step = 1;
                }

                curBlockSize += step;
                curBlockSize = clamp(
                        curBlockSize,
                        args.min_size.asSize(),
                        args.max_size.asSize());
                return tmp;
            }

            case SHRINK: {
                long tmp = curBlockSize;
                long step = args.size_step.asSize() + jitter;
                if (step <= 0) {
                    step = 1;
                }

                curBlockSize -= step;
                if (curBlockSize < 0) {
                    curBlockSize = 0;
                }
                curBlockSize = clamp(
                        curBlockSize,
                        args.min_size.asSize(),
                        args.max_size.asSize());
                return tmp;
            }

            case SAW: {
                long tmp = curBlockSize;
                long step = args.size_step.asSize() + jitter;
                if (step <= 0) {
                    step = 1;
                }

                curBlockSize += step;
                if (curBlockSize > args.max_size.asSize()) {
                    curBlockSize = args.min_size.asSize();
                }
                return tmp;
            }

            default:
                throw new IllegalStateException("Unknown trend: " + trend);
        }
    }

    private static long getClosest(List<Long> list, long size) {
        if (list.isEmpty()) {
            return size;
        }

        long closest = list.get(0);
        long minDiff = Math.abs(size - closest);

        for (Long value : list) {
            long diff = Math.abs(size - value);
            if (diff < minDiff) {
                minDiff = diff;
                closest = value;
            }
        }

        return closest;
    }

    private long getBlockSize() {
        List<Long> sizeList = args.size_list.asSizeList();
        if (sizeList.isEmpty()) {
            return trendBlockSize();
        }

        SizeListMode mode = args.size_mode.asEnum();
        switch (mode) {
            case EXACT:
                return Random.get().choice(sizeList, args.size_weights.asIntList());
            case NEAREST:
                return getClosest(sizeList, trendBlockSize());
            default:
                throw new IllegalStateException("Unknown size mode: " + mode);
        }
    }

    private long getBlockTtl() {
        TTLMode ttlMode = args.ttl_mode.asEnum();

        switch (ttlMode) {
            case OFF:
                return -1;
            case FIXED:
                return args.ttl_fixed.asInt();
            case LIST:
                return Random.get().choice(
                        args.ttl_list.asIntList(),
                        args.ttl_weights.asIntList());
            default:
                throw new IllegalStateException("Unknown TTL mode: " + ttlMode);
        }
    }

    private boolean shouldAlloc() {
        return (pool.count() < pool.capacity) && (Random.get().uniform01() < args.alloc_freq.asFloat());
    }

    private static long clamp(long value, long min, long max) {
        if (value < min)
            return min;
        if (value > max)
            return max;
        return value;
    }
}