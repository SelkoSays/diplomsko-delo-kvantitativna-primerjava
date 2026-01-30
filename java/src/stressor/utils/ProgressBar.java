package stressor.utils;

import java.util.Iterator;
import java.util.NoSuchElementException;

public final class ProgressBar implements Iterator<Long>, Iterable<Long> {

    public enum Mode {
        ITERATIONS, DURATION
    }

    private final Mode mode;
    private final long target; // iterations count or duration in seconds
    private long current;
    private final int width;
    private final long startTime;
    private long lastUpdate;
    private final long updateInterval;
    private boolean displayed;

    public static ProgressBar fromIterations(long iterations) {
        return new ProgressBar(Mode.ITERATIONS, iterations, 50);
    }

    public static ProgressBar fromDuration(long durationSeconds) {
        return new ProgressBar(Mode.DURATION, durationSeconds, 50);
    }

    public static ProgressBar fromIterations(long iterations, int width) {
        return new ProgressBar(Mode.ITERATIONS, iterations, width);
    }

    public static ProgressBar fromDuration(long durationSeconds, int width) {
        return new ProgressBar(Mode.DURATION, durationSeconds, width);
    }

    public ProgressBar display(boolean display) {
        this.displayed = display;
        return this;
    }

    private ProgressBar(Mode mode, long target, int width) {
        this.mode = mode;
        this.target = target;
        this.current = 0;
        this.width = width;
        this.startTime = System.nanoTime();
        this.lastUpdate = System.nanoTime();
        this.updateInterval = 100_000_000L; // 100ms in nanoseconds
        this.displayed = false;
    }

    public void update() {
        current++;
        long now = System.nanoTime();

        if (now - lastUpdate >= updateInterval || isFinished()) {
            draw();
            lastUpdate = now;
        }
    }

    public boolean isFinished() {
        switch (mode) {
            case ITERATIONS:
                return current >= target;
            case DURATION:
                long elapsedSec = (System.nanoTime() - startTime) / 1_000_000_000L;
                return elapsedSec >= target;
            default:
                return true;
        }
    }

    public void finish() {
        if (this.displayed) {
            draw();
            System.out.println();
        }
    }

    private double getProgress() {
        switch (mode) {
            case ITERATIONS:
                return target > 0 ? Math.min(1.0, (double) current / target) : 1.0;
            case DURATION:
                double elapsedSec = (System.nanoTime() - startTime) / 1_000_000_000.0;
                return target > 0 ? Math.min(1.0, elapsedSec / target) : 1.0;
            default:
                return 1.0;
        }
    }

    private void draw() {
        if (!this.displayed) {
            return;
        }

        double progress = getProgress();
        int percent = (int) (progress * 100);
        int filled = (int) (progress * width);

        double elapsedSec = (System.nanoTime() - startTime) / 1_000_000_000.0;
        double rate = elapsedSec > 0.0 ? current / elapsedSec : 0.0;

        System.out.print("\r[");
        for (int i = 0; i < width; i++) {
            if (i < filled) {
                System.out.print("=");
            } else if (i == filled && filled < width) {
                System.out.print(">");
            } else {
                System.out.print(" ");
            }
        }
        System.out.print("] ");

        switch (mode) {
            case ITERATIONS:
                System.out.printf("%d/%d (%3d%%) ", current, target, percent);
                break;
            case DURATION:
                System.out.printf("%.1fs/%.1fs (%3d%%) ", elapsedSec, (double) target, percent);
                break;
        }

        if (rate > 0.0) {
            System.out.printf("%.1f/s ", rate);
        }

        switch (mode) {
            case ITERATIONS:
                if (rate > 0.0 && current < target) {
                    long eta = (long) ((target - current) / rate);
                    System.out.printf("ETA: %02d:%02d", eta / 60, eta % 60);
                }
                break;
            case DURATION:
                long remainingSec = Math.max(0, target - (long) elapsedSec);
                System.out.printf("ETA: %02d:%02d", remainingSec / 60, remainingSec % 60);
                break;
        }

        System.out.flush();
    }

    @Override
    public Iterator<Long> iterator() {
        return this;
    }

    @Override
    public boolean hasNext() {
        return !isFinished();
    }

    @Override
    public Long next() {
        if (isFinished()) {
            throw new NoSuchElementException();
        }
        update();
        return current;
    }
}
