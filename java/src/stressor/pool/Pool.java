package stressor.pool;

import java.util.ArrayDeque;
import java.util.Deque;

import stressor.random.Random;
import stressor.tracker.Tracker;
import stressor.utils.ArgsParser.Policy;

public class Pool {
    public static class Block {
        public byte[] data;
        public long ttl;
        public long ttl_org;

        Block(int size) {
            this(size, -1);
        }

        Block(int size, long ttl) {
            this.data = new byte[size];
            this.ttl = ttl;
            this.ttl_org = ttl;
            Tracker.get().addAlloc(size);
        }

        public void tick() {
            if (this.ttl > 0) {
                this.ttl--;
            }
        }

        public boolean alive() {
            return (this.ttl < 0) || (this.ttl > 0);
        }
    }

    public final Deque<Pool.Block> blocks;
    public final int capacity;

    public Pool(int capacity) {
        this.capacity = capacity;
        this.blocks = new ArrayDeque<>(capacity);
    }

    public void deinit() {
        while (!blocks.isEmpty()) {
            var b = blocks.removeLast();
            Tracker.get().removeAlloc(b.data.length);
        }
    }

    public int count() {
        return this.blocks.size();
    }

    public void addBlock(int size) {
        blocks.addLast(new Block(size));
    }

    public void addBlock(int size, long ttl) {
        blocks.addLast(new Block(size, ttl));
    }

    public void delBlock(Policy policy) {
        if (blocks.size() == 0) {
            return;
        }
        
        switch (policy) {
            case LIFO: {
                var b = blocks.removeLast();
                Tracker.get().removeAlloc(b.data.length);
            }
                return;
            case FIFO: {
                var b = blocks.removeFirst();
                Tracker.get().removeAlloc(b.data.length);
            }
                return;
            case RANDOM: {
                int idx = (int) Random.get().uniform(0, blocks.size());
                var it = blocks.iterator();
                int i = 0;
                while (it.hasNext()) {
                    var b = it.next();
                    if (i == idx) {
                        Tracker.get().removeAlloc(b.data.length);
                        it.remove();
                        break;
                    }
                    i++;
                }
                return;
            }
            case BIG_FIRST: {
                var el = blocks.stream().max((lhs, rhs) -> lhs.data.length - rhs.data.length);
                if (el.isPresent()) {
                    Tracker.get().removeAlloc(el.get().data.length);
                    blocks.remove(el.get());
                }
            }
                return;
            case SMALL_FIRST:
                var el = blocks.stream().min((lhs, rhs) -> lhs.data.length - rhs.data.length);
                if (el.isPresent()) {
                    Tracker.get().removeAlloc(el.get().data.length);
                    blocks.remove(el.get());
                }
                return;
            case NEVER:
                return;
        }

        throw new RuntimeException("Unreachable(del_block)");
    }

    public void updateAndPrune() {
        var it = this.blocks.iterator();

        while (it.hasNext()) {
            var b = it.next();
            b.tick();

            if (!b.alive()) {
                Tracker.get().removeAlloc(b.data.length);
                it.remove();
            }
        }
    }
}
