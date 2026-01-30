package stressor.utils;

import java.util.*;
import java.util.function.Consumer;
import java.util.stream.Collectors;

public final class ArgsParser {

    public interface EnumParse {
        public EnumParse fromIndex(int idx);

        public EnumParse fromString(String name);
    }

    // ==================== ENUMS ====================
    public enum Policy implements EnumParse {
        LIFO(0, "lifo"),
        FIFO(1, "fifo"),
        RANDOM(2, "random"),
        BIG_FIRST(3, "big-first"),
        SMALL_FIRST(4, "small-first"),
        NEVER(5, "never");

        private final int index;
        private final String name;

        Policy(int index, String name) {
            this.index = index;
            this.name = name;
        }

        @Override
        public Policy fromIndex(int index) {
            for (Policy p : values()) {
                if (p.index == index)
                    return p;
            }
            throw new IllegalArgumentException("Unknown policy index: " + index);
        }

        @Override
        public Policy fromString(String name) {
            for (Policy p : values()) {
                if (p.name.equals(name))
                    return p;
            }
            throw new IllegalArgumentException("Unknown policy: " + name);
        }

        @Override
        public String toString() {
            return name;
        }
    }

    public enum Distribution implements EnumParse {
        UNIFORM(0, "uniform"),
        EXP(1, "exp"),
        POWERLAW(2, "powerlaw");

        private final int index;
        private final String name;

        Distribution(int index, String name) {
            this.index = index;
            this.name = name;
        }

        @Override
        public Distribution fromIndex(int index) {
            for (Distribution d : values()) {
                if (d.index == index)
                    return d;
            }
            throw new IllegalArgumentException("Unknown distribution index: " + index);
        }

        @Override
        public Distribution fromString(String name) {
            for (Distribution d : values()) {
                if (d.name.equals(name))
                    return d;
            }
            throw new IllegalArgumentException("Unknown distribution: " + name);
        }

        @Override
        public String toString() {
            return name;
        }
    }

    public enum Trend implements EnumParse {
        NONE(0, "none"),
        GROW(1, "grow"),
        SHRINK(2, "shrink"),
        SAW(3, "saw");

        private final int index;
        private final String name;

        Trend(int index, String name) {
            this.index = index;
            this.name = name;
        }

        @Override
        public Trend fromIndex(int index) {
            for (Trend t : values()) {
                if (t.index == index)
                    return t;
            }
            throw new IllegalArgumentException("Unknown trend index: " + index);
        }

        @Override
        public Trend fromString(String name) {
            for (Trend t : values()) {
                if (t.name.equals(name))
                    return t;
            }
            throw new IllegalArgumentException("Unknown trend: " + name);
        }

        @Override
        public String toString() {
            return name;
        }
    }

    public enum SizeListMode implements EnumParse {
        EXACT(0, "exact"),
        NEAREST(1, "nearest");

        private final int index;
        private final String name;

        SizeListMode(int index, String name) {
            this.index = index;
            this.name = name;
        }

        @Override
        public SizeListMode fromIndex(int index) {
            for (SizeListMode m : values()) {
                if (m.index == index)
                    return m;
            }
            throw new IllegalArgumentException("Unknown size mode index: " + index);
        }

        @Override
        public SizeListMode fromString(String name) {
            for (SizeListMode m : values()) {
                if (m.name.equals(name))
                    return m;
            }
            throw new IllegalArgumentException("Unknown size mode: " + name);
        }

        @Override
        public String toString() {
            return name;
        }
    }

    public enum TTLMode implements EnumParse {
        OFF(0, "off"),
        FIXED(1, "fixed"),
        LIST(2, "list");

        private final int index;
        private final String name;

        TTLMode(int index, String name) {
            this.index = index;
            this.name = name;
        }

        @Override
        public TTLMode fromIndex(int index) {
            for (TTLMode t : values()) {
                if (t.index == index)
                    return t;
            }
            throw new IllegalArgumentException("Unknown TTL mode index: " + index);
        }

        @Override
        public TTLMode fromString(String name) {
            for (TTLMode t : values()) {
                if (t.name.equals(name))
                    return t;
            }
            throw new IllegalArgumentException("Unknown TTL mode: " + name);
        }

        @Override
        public String toString() {
            return name;
        }
    }

    // ==================== ARG TYPE ====================
    public static class Arg {
        public enum Type {
            INT, TIME, SIZE, FLOAT, STR, BOOL, ENUM, INT_LIST, SIZE_LIST
        }

        private final Type type;
        private final Object value;

        private Arg(Type type, Object value) {
            this.type = type;
            this.value = value;
        }

        public Object getValue() {
            return this.value;
        }

        public static Arg intArg(long value) {
            return new Arg(Type.INT, value);
        }

        public static Arg timeArg(long value) {
            return new Arg(Type.TIME, value);
        }

        public static Arg sizeArg(long value) {
            return new Arg(Type.SIZE, value);
        }

        public static Arg floatArg(double value) {
            return new Arg(Type.FLOAT, value);
        }

        public static Arg strArg(String value) {
            return new Arg(Type.STR, value);
        }

        public static Arg boolArg(boolean value) {
            return new Arg(Type.BOOL, value);
        }

        public static Arg enumArg(Enum<?> value) {
            return new Arg(Type.ENUM, value);
        }

        public static Arg intListArg(List<Long> value) {
            return new Arg(Type.INT_LIST, new ArrayList<>(value));
        }

        public static Arg sizeListArg(List<Long> value) {
            return new Arg(Type.SIZE_LIST, new ArrayList<>(value));
        }

        public long asInt() {
            if (type != Type.INT)
                throw new IllegalStateException("Not an INT");
            return (Long) value;
        }

        public long asTime() {
            if (type != Type.TIME)
                throw new IllegalStateException("Not a TIME");
            return (Long) value;
        }

        public long asSize() {
            if (type != Type.SIZE)
                throw new IllegalStateException("Not a SIZE");
            return (Long) value;
        }

        public double asFloat() {
            if (type != Type.FLOAT)
                throw new IllegalStateException("Not a FLOAT");
            return (Double) value;
        }

        public String asStr() {
            if (type != Type.STR)
                throw new IllegalStateException("Not a STR");
            return (String) value;
        }

        public boolean asBool() {
            if (type != Type.BOOL)
                throw new IllegalStateException("Not a BOOL");
            return (Boolean) value;
        }

        @SuppressWarnings("unchecked")
        public <E extends Enum<E>> E asEnum() {
            if (type != Type.ENUM)
                throw new IllegalStateException("Not an ENUM");
            return (E) value;
        }

        @SuppressWarnings("unchecked")
        public List<Long> asIntList() {
            if (type != Type.INT_LIST)
                throw new IllegalStateException("Not an INT_LIST");
            return (List<Long>) value;
        }

        @SuppressWarnings("unchecked")
        public List<Long> asSizeList() {
            if (type != Type.SIZE_LIST)
                throw new IllegalStateException("Not a SIZE_LIST");
            return (List<Long>) value;
        }

        @Override
        public String toString() {
            switch (type) {
                case SIZE:
                    return sizeStr((Long) value);
                case TIME:
                    return timeStr((Long) value);
                case INT_LIST:
                case SIZE_LIST:
                    return listStr();
                default:
                    return String.valueOf(value);
            }
        }

        private String listStr() {
            List<Long> list = (type == Type.INT_LIST) ? asIntList() : asSizeList();
            if (type == Type.SIZE_LIST) {
                return "[" + list.stream()
                        .map(ArgsParser::sizeStr)
                        .collect(Collectors.joining(",")) + "]";
            } else {
                return "[" + list.stream()
                        .map(String::valueOf)
                        .collect(Collectors.joining(",")) + "]";
            }
        }
    }

    // ==================== ARGS CLASS ====================
    public static final class Args {
        // General run-control
        public Arg policy = Arg.enumArg(Policy.LIFO);
        public Arg iterations = Arg.intArg(10000);
        public Arg duration_sec = Arg.timeArg(0);
        public Arg alloc_freq = Arg.floatArg(0.7f);
        public Arg seed = Arg.intArg(0);

        // Pool sizing
        public Arg capacity = Arg.intArg(10000);

        // Block-size
        public Arg min_size = Arg.sizeArg(16);
        public Arg max_size = Arg.sizeArg(1 << 20);
        public Arg size_trend = Arg.enumArg(Trend.NONE);
        public Arg size_step = Arg.sizeArg(1024);
        public Arg trend_jitter = Arg.sizeArg(0);

        public Arg size_list = Arg.sizeListArg(new ArrayList<>());
        public Arg size_mode = Arg.enumArg(SizeListMode.EXACT);
        public Arg size_weights = Arg.intListArg(new ArrayList<>());

        // Block-size distribution
        public Arg distribution = Arg.enumArg(Distribution.UNIFORM);
        public Arg dist_param = Arg.floatArg(1.0f);

        // Block lifetime
        public Arg ttl_mode = Arg.enumArg(TTLMode.OFF);
        public Arg ttl_fixed = Arg.intArg(10);
        public Arg ttl_list = Arg.intListArg(new ArrayList<>());
        public Arg ttl_weights = Arg.intListArg(new ArrayList<>());

        // Instrumentation & output
        public Arg snap_interval = Arg.intArg(1000);
        public Arg output = Arg.strArg(null);
        public Arg display = Arg.boolArg(false);

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("Args {\n");
            sb.append("  policy: ").append(policy).append(",\n");
            sb.append("  iterations: ").append(iterations).append(",\n");
            sb.append("  duration_sec: ").append(duration_sec).append(",\n");
            sb.append("  alloc_freq: ").append(snap_interval).append(",\n");
            sb.append("  seed: ").append(seed).append(",\n");
            sb.append("  capacity: ").append(capacity).append(",\n");
            sb.append("  min_size: ").append(min_size).append(",\n");
            sb.append("  max_size: ").append(max_size).append(",\n");
            sb.append("  size_trend: ").append(size_trend).append(",\n");
            sb.append("  size_step: ").append(size_step).append(",\n");
            sb.append("  trend_jitter: ").append(trend_jitter).append(",\n");
            sb.append("  size_list: ").append(size_list).append(",\n");
            sb.append("  size_mode: ").append(size_mode).append(",\n");
            sb.append("  size_weights: ").append(size_weights).append(",\n");
            sb.append("  distribution: ").append(distribution).append(",\n");
            sb.append("  dist_param: ").append(dist_param).append(",\n");
            sb.append("  ttl_mode: ").append(ttl_mode).append(",\n");
            sb.append("  ttl_fixed: ").append(ttl_fixed).append(",\n");
            sb.append("  ttl_list: ").append(ttl_list).append(",\n");
            sb.append("  ttl_weights: ").append(ttl_weights).append(",\n");
            sb.append("  snap_interval: ").append(snap_interval).append(",\n");
            sb.append("  output: ").append(output).append(",\n");
            sb.append("  display: ").append(display).append(",\n");
            sb.append("}");
            return sb.toString();
        }
    }

    // ==================== ARG SPEC ====================
    static final class ArgSpec {
        final char shortOpt;
        final String longOpt;
        final Arg.Type type;
        final Consumer<Arg> setter;
        final boolean required;
        final String metavar;
        final String help;
        public String category;
        final String[] choices;
        final Arg defaultValue;
        final boolean hasDefault;
        EnumParse enumParser;

        ArgSpec(char shortOpt, String longOpt, Consumer<Arg> setter,
                boolean required, String metavar, String help,
                String[] choices, Arg defaultValue, boolean hasDefault) {
            this.shortOpt = shortOpt;
            this.longOpt = longOpt;
            this.type = (defaultValue != null) ? defaultValue.type : Arg.Type.BOOL;
            this.setter = setter;
            this.required = required;
            this.metavar = metavar;
            this.help = help;
            this.category = "";
            this.choices = choices;
            this.defaultValue = defaultValue;
            this.hasDefault = hasDefault;
            this.enumParser = null;
        }

        ArgSpec(char shortOpt, String longOpt, Consumer<Arg> setter,
                boolean required, String metavar, String help,
                String[] choices, Arg defaultValue, boolean hasDefault, EnumParse parser) {
            this(shortOpt, longOpt, setter, required, metavar, help, choices, defaultValue, hasDefault);
            this.enumParser = parser;
        }

        public ArgSpec setCategory(String category) {
            this.category = category;
            return this;
        }
    }

    static final class ArgSpecs implements Iterable<ArgSpec> {
        private final Map<String, List<ArgSpec>> groupedSpecs;
        private final List<String> groups;
        private String currentGroup;

        ArgSpecs() {
            groupedSpecs = new HashMap<>();
            groups = new ArrayList<>();
            currentGroup = "";
        }

        public void add(ArgSpec spec) {
            groupedSpecs.get(currentGroup).add(spec.setCategory(currentGroup));
        }

        public void changeGroup(String group) {
            currentGroup = group;
            groupedSpecs.put(group, new ArrayList<>());
            groups.add(group);
        }

        @Override
        public Iterator<ArgSpec> iterator() {
            return new ArgSpecs.ArgSpecIterator(this);
        }

        static class ArgSpecIterator implements Iterator<ArgSpec> {
            ArgSpecs specs;
            int groupIdx;
            int specIdx;
            boolean moved;

            ArgSpecIterator(ArgSpecs specs) {
                this.specs = specs;
                this.groupIdx = 0;
                this.specIdx = -1;
                this.moved = false;
            }

            @Override
            public boolean hasNext() {
                if (moved) {
                    return true;
                }

                if (this.groupIdx >= specs.groups.size()) {
                    return false;
                }

                var gSpecs = specs.groupedSpecs.get(specs.groups.get(groupIdx));
                if ((specIdx + 1) < gSpecs.size()) {
                    specIdx++;
                    moved = true;
                    return true;
                }

                while ((this.groupIdx + 1) < specs.groups.size()) {
                    this.groupIdx++;
                    this.specIdx = 0;

                    gSpecs = specs.groupedSpecs.get(specs.groups.get(groupIdx));
                    if (gSpecs.size() == 0) {
                        continue;
                    }
                    moved = true;
                    return true;
                }

                return false;
            }

            @Override
            public ArgSpec next() {
                if (!hasNext()) {
                    throw new NoSuchElementException("No more argument spesification");
                }
                moved = false;
                return specs.groupedSpecs.get(specs.groups.get(groupIdx)).get(specIdx);
            }

        }
    }

    // ==================== SPECS DEFINITION ====================
    private static ArgSpecs buildSpecs(Args args) {
        ArgSpecs specs = new ArgSpecs();

        // General run-control
        specs.changeGroup("General run-control");
        specs.add(new ArgSpec('p', "policy", v -> args.policy = v, false,
                "POLICY",
                "Free policy",
                new String[] { "lifo", "fifo", "random", "big-first", "small-first", "never" },
                Arg.enumArg(Policy.LIFO), true, Policy.LIFO));

        specs.add(new ArgSpec('n', "iterations", v -> args.iterations = v, false, "N", "Total alloc/free ops", null,
                Arg.intArg(10000), true));

        specs.add(new ArgSpec('d', "duration", v -> args.duration_sec = v, false, "SECS",
                "Run for this many seconds (mutually exclusive with --iterations)", null, Arg.timeArg(0), true));

        specs.add(new ArgSpec('f', "alloc-freq", v -> args.alloc_freq = v, false, "F",
                "Frequency [0, 1] of allocations when policy not 'never'", null, Arg.floatArg(0.7f), true));

        specs.add(new ArgSpec('s', "seed", v -> args.seed = v, false, "N", "RNG seed (0=time)", null, Arg.intArg(0),
                true));

        // Pool sizing
        specs.changeGroup("Pool sizing");
        specs.add(new ArgSpec('c', "capacity", v -> args.capacity = v, false, "C", "Max live blocks", null,
                Arg.intArg(10000), true));

        // Block-size
        specs.changeGroup("Block-size");
        specs.add(new ArgSpec('a', "min-size", v -> args.min_size = v, false, "BYTES", "Min block size", null,
                Arg.sizeArg(16), true));

        specs.add(new ArgSpec('A', "max-size", v -> args.max_size = v, false, "BYTES", "Max block size", null,
                Arg.sizeArg(1 << 20), true));

        specs.add(new ArgSpec('\0', "size-trend", v -> args.size_trend = v,
                false, "TREND",
                "Set block size mutation through time. If 'none' sizes are random",
                new String[] { "none", "grow", "shrink", "saw" }, Arg.enumArg(Trend.NONE), true, Trend.NONE));

        specs.add(new ArgSpec('\0', "size-step", v -> args.size_step = v, false, "BYTES", "Set block mutation size",
                null, Arg.sizeArg(1024), true));

        specs.add(new ArgSpec('\0', "trend-jitter", v -> args.trend_jitter = v, false, "BYTES",
                "Add uniform +- jitter to mutation size", null, Arg.sizeArg(0), true));

        specs.add(new ArgSpec('\0', "size-list", v -> args.size_list = v, false, "L[BYTES]",
                "Only use provided block sizes", null, Arg.sizeListArg(new ArrayList<>()), false));

        specs.add(new ArgSpec('\0', "size-mode",
                v -> args.size_mode = v, false, "MODE",
                "Choose size from list. trend/distribution for nearest", new String[] { "exact", "nearest" },
                Arg.enumArg(SizeListMode.EXACT), true, SizeListMode.EXACT));

        specs.add(new ArgSpec('\0', "size-weights", v -> args.size_weights = v, false, "L[N]",
                "Set weights of size list (in %)", null, Arg.intListArg(new ArrayList<>()), false));

        // Block-size distribution
        specs.changeGroup("Block-size distribution");
        specs.add(new ArgSpec('P', "distribution",
                v -> args.distribution = v, false, "TYPE",
                "Size distribution",
                new String[] { "uniform", "exp", "powerlaw" }, Arg.enumArg(Distribution.UNIFORM), true,
                Distribution.UNIFORM));

        specs.add(new ArgSpec('r', "dist-param", v -> args.dist_param = v, false, "F",
                "Parameter for non-uniform distributions {exp(lambda), powerlaw(alpha)}", null, Arg.floatArg(1.0f),
                true));

        // Block lifetime
        specs.changeGroup("Block lifetime");
        specs.add(new ArgSpec('\0', "ttl-mode", v -> args.ttl_mode = v,
                false, "MODE", "Set lifetime of blocks",
                new String[] { "off", "fixed", "list" }, Arg.enumArg(TTLMode.OFF), true, TTLMode.OFF));

        specs.add(new ArgSpec('\0', "ttl-fixed", v -> args.ttl_fixed = v, false, "N",
                "Number of cycles blocks will live", null, Arg.intArg(10), true));

        specs.add(new ArgSpec('\0', "ttl-list", v -> args.ttl_list = v, false, "L[N]",
                "Use provided lifetimes uniformly", null, Arg.intListArg(new ArrayList<>()), true));

        specs.add(new ArgSpec('\0', "ttl-weights", v -> args.ttl_weights = v, false, "L[N]",
                "Set weights for list of lifetimes", null, Arg.intListArg(new ArrayList<>()), true));

        // Instrumentation & output
        specs.changeGroup("Instrumentation & output");
        specs.add(new ArgSpec('i', "snap-interval", v -> args.snap_interval = v, false, "N",
                "Every N ops, snapshot and log stats", null, Arg.intArg(1000), true));

        specs.add(new ArgSpec('o', "output", v -> args.output = v, false, "FILE", "Path to CSV metrics log", null,
                Arg.strArg(null), true));

        specs.add(new ArgSpec('\0', "display", v -> args.display = v, false, null, "Verbose logging", null,
                Arg.boolArg(false), true));

        specs.add(new ArgSpec('h', "help", null, false, null, "Show this help text and exit", null, Arg.boolArg(false),
                false));

        return specs;
    }

    // ==================== USAGE ====================
    public static void printUsage(String progname) {
        System.out.printf("Usage: %s [options]\n\n", progname);
        System.out.println("  <BYTES>  supports units B, KB, KiB, MB, MiB, GB, GiB (e.g. 10KB)");
        System.out.println("  <SECS>   supports units s, min (e.g. 1min)");
        System.out.println("  <L[...]> comma-separated list (e.g. 1,2,3,4)");

        Args dummy = new Args();
        ArgSpecs specs = buildSpecs(dummy);
        String lastCategory = null;

        for (ArgSpec spec : specs) {
            if (!spec.category.equals(lastCategory)) {
                System.out.println("\n" + spec.category + ":");
                lastCategory = spec.category;
            }

            String shortPart = (spec.shortOpt != '\0')
                    ? String.format("-%c,", spec.shortOpt)
                    : "    ";
            String metavarPart = (spec.metavar != null)
                    ? " " + spec.metavar
                    : "";
            String enumPart = (spec.type == Arg.Type.ENUM && spec.choices != null)
                    ? "; one of:"
                    : "";

            System.out.printf("  %-4s--%-13s%-9s %s%s\n",
                    shortPart, spec.longOpt, metavarPart, spec.help, enumPart);

            if (spec.hasDefault) {
                printDefault(spec);
            }
        }
        System.out.println();
    }

    private static void printDefault(ArgSpec spec) {
        switch (spec.type) {
            case BOOL:
                System.out.printf("%32s (default: %s)\n", "",
                        spec.defaultValue.asBool() ? "true" : "false");
                break;
            case FLOAT:
                System.out.printf("%32s (default: %.2f)\n", "", spec.defaultValue.asFloat());
                break;
            case INT:
                System.out.printf("%32s (default: %d)\n", "", spec.defaultValue.asInt());
                break;
            case TIME:
                System.out.printf("%32s (default: %s)\n", "", timeStr(spec.defaultValue.asTime()));
                break;
            case SIZE:
                System.out.printf("%32s (default: %s)\n", "", sizeStr(spec.defaultValue.asSize()));
                break;
            case STR:
                if (spec.defaultValue.asStr() != null) {
                    System.out.printf("%32s (default: %s)\n", "", spec.defaultValue.asStr());
                }
                break;
            case ENUM:
                if (spec.choices != null) {
                    Enum<?> defaultEnum = spec.defaultValue.asEnum();
                    for (int i = 0; i < spec.choices.length; i++) {
                        boolean isDefault = spec.choices[i].equals(defaultEnum.toString());
                        System.out.printf("%32s - %s%s\n", "", spec.choices[i],
                                isDefault ? " (default)" : "");
                    }
                }
                break;
            case INT_LIST:
            case SIZE_LIST:
                if (!spec.defaultValue.asIntList().isEmpty()) {
                    System.out.printf("%32s (default: %s)\n", "", spec.defaultValue);
                }
                break;
        }
    }

    // ==================== PARSING ====================
    public static Args parseArgs(String[] argv) {
        Args args = new Args();
        ArgSpecs specs = buildSpecs(args);

        // Build lookup maps
        Map<String, ArgSpec> byLong = new HashMap<>();
        Map<Character, ArgSpec> byShort = new HashMap<>();
        for (ArgSpec spec : specs) {
            byLong.put(spec.longOpt, spec);
            if (spec.shortOpt != '\0') {
                byShort.put(spec.shortOpt, spec);
            }
        }

        // Set defaults first
        for (ArgSpec spec : specs) {
            if (spec.hasDefault && spec.setter != null) {
                spec.setter.accept(spec.defaultValue);
            }
        }

        // Save original iterations default
        long iterDefault = args.iterations.asInt();
        args.iterations = Arg.intArg(0);

        // Parse arguments
        for (int i = 0; i < argv.length; i++) {
            String arg = argv[i];

            if (!arg.startsWith("-")) {
                throw new IllegalArgumentException("Unexpected argument: " + arg);
            }

            if ("--help".equals(arg) || "-h".equals(arg)) {
                printUsage("stressor");
                System.exit(0);
            }

            ArgSpec spec = null;
            String value = null;

            if (arg.startsWith("--")) {
                // Long option
                String longOpt = arg.substring(2);
                int eqIdx = longOpt.indexOf('=');

                if (eqIdx >= 0) {
                    value = longOpt.substring(eqIdx + 1);
                    longOpt = longOpt.substring(0, eqIdx);
                }

                spec = byLong.get(longOpt);
                if (spec == null) {
                    throw new IllegalArgumentException("Unknown option: --" + longOpt);
                }

                if (spec.type != Arg.Type.BOOL && value == null) {
                    if (i + 1 >= argv.length) {
                        throw new IllegalArgumentException("Option requires a value: --" + longOpt);
                    }
                    value = argv[++i];
                }
            } else if (arg.length() == 2 && arg.charAt(0) == '-') {
                // Short option
                char shortOpt = arg.charAt(1);
                spec = byShort.get(shortOpt);
                if (spec == null) {
                    throw new IllegalArgumentException("Unknown option: -" + shortOpt);
                }

                if (spec.type != Arg.Type.BOOL) {
                    if (i + 1 >= argv.length) {
                        throw new IllegalArgumentException("Option requires a value: -" + shortOpt);
                    }
                    value = argv[++i];
                }
            } else if (arg.startsWith("-") && arg.length() > 2) {
                // Short option with value attached
                char shortOpt = arg.charAt(1);
                spec = byShort.get(shortOpt);
                if (spec == null) {
                    throw new IllegalArgumentException("Unknown option: -" + shortOpt);
                }
                value = arg.substring(2);
                if (value.startsWith("=")) {
                    value = value.substring(1);
                }
            } else {
                throw new IllegalArgumentException("Invalid argument format: " + arg);
            }

            // Parse and set the value
            if (spec.setter != null) {
                Arg parsedValue = parseValue(value, spec);
                spec.setter.accept(parsedValue);
            }
        }

        // Check and adjust arguments
        checkArgs(args, iterDefault);

        return args;
    }

    private static void checkArgs(Args args, long iterDefault) {
        if (args.min_size.asSize() > args.max_size.asSize()) {
            args.max_size = Arg.sizeArg(args.min_size.asSize());
        }

        if (args.iterations.asInt() == 0 && args.duration_sec.asTime() == 0) {
            args.iterations = Arg.intArg(iterDefault);
        } else if (args.iterations.asInt() != 0 && args.duration_sec.asTime() != 0) {
            throw new IllegalArgumentException("Arguments --iterations and --duration are mutually exclusive");
        }

        if (args.seed.asInt() == 0) {
            args.seed = Arg.intArg(System.currentTimeMillis() / 1000L);
        }

        if (args.distribution.<Distribution>asEnum() == Distribution.POWERLAW && args.dist_param.asFloat() == 0.0f) {
            throw new IllegalArgumentException("Alpha parameter for powerlaw distribution should not be 0");
        }

        if (!args.ttl_list.asIntList().isEmpty() && !args.ttl_weights.asIntList().isEmpty() &&
                args.ttl_list.asIntList().size() != args.ttl_weights.asIntList().size()) {
            throw new IllegalArgumentException(String.format(
                    "Number of weights doesn't match the number of items in lifetime list. W(%d) != L(%d)",
                    args.ttl_weights.asIntList().size(), args.ttl_list.asIntList().size()));
        }

        if (!args.size_list.asSizeList().isEmpty() && !args.size_weights.asIntList().isEmpty() &&
                args.size_list.asSizeList().size() != args.size_weights.asIntList().size()) {
            throw new IllegalArgumentException(String.format(
                    "Number of weights doesn't match the number of items in size-list. W(%d) != L(%d)",
                    args.size_weights.asIntList().size(), args.size_list.asSizeList().size()));
        }

        if (args.size_step.asSize() == 0) {
            throw new IllegalArgumentException("--size-step should not be zero");
        }

        if (args.ttl_fixed.asInt() == 0) {
            throw new IllegalArgumentException("--ttl-fixed should not be zero");
        }

        if (args.policy.<Policy>asEnum() == Policy.NEVER && args.ttl_mode.<TTLMode>asEnum() == TTLMode.OFF) {
            throw new IllegalArgumentException("If --policy is 'never', then --ttl-mode should not be off");
        }

        if (args.alloc_freq.asFloat() < 0 || args.alloc_freq.asFloat() > 1) {
            throw new IllegalArgumentException(String.format("--alloc-freq should be on the interval [0, 1], but is %f",
                    args.alloc_freq.asFloat()));
        }
    }

    private static Arg parseValue(String value, ArgSpec spec) {
        switch (spec.type) {
            case BOOL:
                return Arg.boolArg(true);
            case INT:
                return Arg.intArg(parseInt(value));
            case TIME:
                return Arg.timeArg(parseDuration(value));
            case SIZE:
                return Arg.sizeArg(parseSize(value));
            case FLOAT:
                return Arg.floatArg(parseFloat(value));
            case STR:
                return Arg.strArg(value);
            case ENUM:
                return parseEnum(value, spec);
            case INT_LIST:
                return Arg.intListArg(parseIntList(value));
            case SIZE_LIST:
                return Arg.sizeListArg(parseSizeList(value));
            default:
                throw new IllegalArgumentException("Unknown argument type: " + spec.type);
        }
    }

    // ==================== PARSE HELPERS ====================
    private static long parseInt(String s) {
        if (s == null || s.isEmpty()) {
            return 0;
        }
        try {
            if (s.startsWith("0x") || s.startsWith("0X")) {
                return Long.parseUnsignedLong(s.substring(2), 16);
            }
            return Long.parseLong(s);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid integer: " + s);
        }
    }

    private static double parseFloat(String s) {
        try {
            return Double.parseDouble(s);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid float: " + s);
        }
    }

    private static long parseDuration(String s) {
        if (s == null || s.isEmpty()) {
            return 0;
        }

        int idx = 0;
        while (idx < s.length() && Character.isDigit(s.charAt(idx))) {
            idx++;
        }

        String numPart = s.substring(0, idx);
        String unit = s.substring(idx);
        long value = parseInt(numPart);

        switch (unit) {
            case "":
            case "s":
                return value;
            case "min":
                return value * 60;
            default:
                throw new IllegalArgumentException("Supported units for duration are (s)econds and (min)utes");
        }
    }

    private static long parseSize(String s) {
        if (s == null || s.isEmpty()) {
            return 0;
        }

        int idx = 0;
        while (idx < s.length() && Character.isDigit(s.charAt(idx))) {
            idx++;
        }

        String numPart = s.substring(0, idx);
        String unit = s.substring(idx);
        long value = parseInt(numPart);

        switch (unit) {
            case "":
            case "B":
                return value;
            case "KB":
                return value * 1000L;
            case "KiB":
                return value * (1L << 10);
            case "MB":
                return value * 1_000_000L;
            case "MiB":
                return value * (1L << 20);
            case "GB":
                return value * 1_000_000_000L;
            case "GiB":
                return value * (1L << 30);
            default:
                throw new IllegalArgumentException("Supported units for size are B, KB, KiB, MB, MiB, GB, GiB");
        }
    }

    private static Arg parseEnum(String value, ArgSpec spec) {
        if (spec.choices == null) {
            throw new IllegalArgumentException("No choices provided for enum");
        }

        if (spec.enumParser == null) {
            throw new IllegalArgumentException("Argument enum does not have parse function");
        }

        if (Arrays.asList(spec.choices).contains(value)) {
            return Arg.enumArg((Enum<?>) spec.enumParser.fromString(value));
        }

        throw new IllegalArgumentException("Invalid enum option: " + value);
    }

    private static List<Long> parseIntList(String s) {
        List<Long> list = new ArrayList<>();
        if (s == null || s.isEmpty()) {
            return list;
        }

        String[] parts = s.split(",");
        for (String part : parts) {
            list.add(parseInt(part.trim()));
        }
        return list;
    }

    private static List<Long> parseSizeList(String s) {
        List<Long> list = new ArrayList<>();
        if (s == null || s.isEmpty()) {
            return list;
        }

        String[] parts = s.split(",");
        for (String part : parts) {
            list.add(parseSize(part.trim()));
        }
        return list;
    }

    // ==================== FORMAT HELPERS ====================
    private static String sizeStr(long size) {
        if (size < (1L << 10)) {
            return size + "B";
        } else if (size < (1L << 20)) {
            return String.format("%.1fKiB", size / (double) (1L << 10));
        } else if (size < (1L << 30)) {
            return String.format("%.1fMiB", size / (double) (1L << 20));
        } else if (size < (1L << 40)) {
            return String.format("%.1fGiB", size / (double) (1L << 30));
        } else {
            return size + "B";
        }
    }

    private static String timeStr(long time) {
        if (time < 60) {
            return time + "s";
        } else if (time % 60 == 0) {
            return (time / 60) + "min";
        } else {
            return (time / 60) + "min " + (time % 60) + "s";
        }
    }

    // ==================== MAIN ENTRY POINTS ====================
    public static Args parseOrExit(String[] argv) {
        try {
            return parseArgs(argv);
        } catch (IllegalArgumentException e) {
            System.err.println(AnsiColors.colorize("[ERROR] ", AnsiColors.RED) + e.getMessage());
            System.exit(2);
            return null;
        }
    }

    // ==================== LOGGING ====================
    public static void logArgs(Args args) {
        System.out.println("args.policy = " + args.policy.asEnum());
        System.out.println("args.iterations = " + args.iterations.asInt());
        System.out.println("args.duration_sec = " + args.duration_sec.asTime());
        System.out.println("args.alloc_freq = " + args.alloc_freq.asFloat());
        System.out.println("args.seed = " + args.seed.asInt());
        System.out.println("args.capacity = " + args.capacity.asInt());
        System.out.println("args.min_size = " + args.min_size.asSize());
        System.out.println("args.max_size = " + args.max_size.asSize());
        System.out.println("args.size_trend = " + args.size_trend.asEnum());
        System.out.println("args.size_step = " + args.size_step.asSize());
        System.out.println("args.trend_jitter = " + args.trend_jitter.asSize());
        System.out.println("args.size_list = " + args.size_list);
        System.out.println("args.size_mode = " + args.size_mode.asEnum());
        System.out.println("args.size_weights = " + args.size_weights);
        System.out.println("args.distribution = " + args.distribution.asEnum());
        System.out.println("args.dist_param = " + args.dist_param.asFloat());
        System.out.println("args.ttl_mode = " + args.ttl_mode.asEnum());
        System.out.println("args.ttl_fixed = " + args.ttl_fixed.asInt());
        System.out.println("args.ttl_list = " + args.ttl_list);
        System.out.println("args.ttl_weights = " + args.ttl_weights);
        System.out.println("args.snap_interval = " + args.snap_interval.asInt());
        System.out.println("args.output = " + args.output.asStr());
        System.out.println("args.display = " + args.display.asBool());
    }
}
