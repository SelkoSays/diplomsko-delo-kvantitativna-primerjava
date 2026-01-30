package stressor.utils;

public class AnsiColors {
    public static final String ESC = "\u001b";

    // ================= COLOR =================

    public static final String RESET = ESC + "[0m";
    public static final String RED = ESC + "[91m";
    public static final String BRED = ESC + "[101m";
    public static final String GREEN = ESC + "[92m";
    public static final String BGREEN = ESC + "[102m";
    public static final String YELLOW = ESC + "[93m";
    public static final String BYELLOW = ESC + "[103m";
    public static final String BLUE = ESC + "[94m";
    public static final String BBLUE = ESC + "[104m";
    public static final String CYAN = ESC + "[96m";
    public static final String BCYAN = ESC + "[106m";
    public static final String WHITE = ESC + "[97m";
    public static final String BWHITE = ESC + "[107m";
    public static final String GRAY = ESC + "[90m";
    public static final String BGRAY = ESC + "[100m";
    public static final String BLACK = ESC + "[30m";
    public static final String BBLACK = ESC + "[40m";

    // ================= STYLE =================

    public static final String BOLD = ESC + "[1m";
    public static final String ITALIC = ESC + "[3m";
    public static final String UNDERLINE = ESC + "[4m";

    public static String colorize(String str, String... colors) {
        StringBuilder sb = new StringBuilder();
        for (var color : colors) {
            sb.append(color);
        }
        sb.append(str);
        sb.append(RESET);
        return sb.toString();
    }
}
