package stressor.utils;

public class Logger {
    static enum LogLevel {
        DEBUG(0),
        INFO(1),
        WARN(2),
        ERROR(3),
        NO_LOG(4);

        private final int lvl;

        LogLevel(final int lvl) {
            this.lvl = lvl;
        }

        public int lvl() {
            return this.lvl;
        }
    }

    static final LogLevel LOG_LEVEL = LogLevel.DEBUG;
    static final boolean NO_ANSI = false;

    static final String DEBUG_STR = (NO_ANSI) ? "[DEBUG]" : (AnsiColors.YELLOW + "[DEBUG]" + AnsiColors.RESET);
    static final String INFO_STR = (NO_ANSI) ? "[INFO]" : (AnsiColors.CYAN + "[INFO]" + AnsiColors.RESET);
    static final String WARN_STR = (NO_ANSI) ? "[WARN]" : (AnsiColors.YELLOW + "[WARN]" + AnsiColors.RESET);
    static final String ERROR_STR = (NO_ANSI) ? "[ERROR]" : (AnsiColors.RED + "[ERROR]" + AnsiColors.RESET);

    public static void debug(String fmt, Object... args) {
        if (LOG_LEVEL.lvl() <= 0) {
            System.err.printf(DEBUG_STR + " " + fmt + "%n", args);
        }
    }

    public static void info(String fmt, Object... args) {
        if (LOG_LEVEL.lvl() <= 1) {
            System.err.printf(INFO_STR + " " + fmt + "%n", args);
        }
    }

    public static void warn(String fmt, Object... args) {
        if (LOG_LEVEL.lvl() <= 2) {
            System.err.printf(WARN_STR + " " + fmt + "%n", args);
        }
    }

    public static void error(String fmt, Object... args) {
        if (LOG_LEVEL.lvl() <= 3) {
            System.err.printf(ERROR_STR + " " + fmt + "%n", args);
        }
    }
}
