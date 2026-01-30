#ifndef ANSI_COLORS_H
#define ANSI_COLORS_H

#define ANSI_COLOR(color, str) ANSI_##color str ANSI_RESET

#if defined(NO_ANSI)

#define ANSI_RESET
#define ANSI_RED
#define ANSI_BRED
#define ANSI_GREEN
#define ANSI_BGREEN
#define ANSI_YELLOW
#define ANSI_BYELLOW
#define ANSI_BLUE
#define ANSI_BBLUE
#define ANSI_CYAN
#define ANSI_BCYAN
#define ANSI_WHITE
#define ANSI_BWHITE
#define ANSI_GRAY
#define ANSI_BGRAY
#define ANSI_GREY
#define ANSI_BGREY
#define ANSI_BLACK
#define ANSI_BBLACK

#define ANSI_BOLD
#define ANSI_ITALIC
#define ANSI_UNDERLINE

#else

#define ANSI_RESET "\x1b[0m"
#define ANSI_RED "\x1b[91m"
#define ANSI_BRED "\x1b[101m"
#define ANSI_GREEN "\x1b[92m"
#define ANSI_BGREEN "\x1b[102m"
#define ANSI_YELLOW "\x1b[93m"
#define ANSI_BYELLOW "\x1b[103m"
#define ANSI_BLUE "\x1b[94m"
#define ANSI_BBLUE "\x1b[104m"
#define ANSI_CYAN "\x1b[96m"
#define ANSI_BCYAN "\x1b[106m"
#define ANSI_WHITE "\x1b[97m"
#define ANSI_BWHITE "\x1b[107m"
#define ANSI_GRAY "\x1b[90m"
#define ANSI_BGRAY "\x1b[100m"
#define ANSI_GREY ANSI_GRAY
#define ANSI_BGREY ANSI_BGRAY
#define ANSI_BLACK "\x1b[30m"
#define ANSI_BBLACK "\x1b[40m"

#define ANSI_BOLD "\x1b[1m"
#define ANSI_ITALIC "\x1b[3m"
#define ANSI_UNDERLINE "\x1b[4m"

#endif // NO_ANSI

#endif // ANSI_COLORS_H
