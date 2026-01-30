pub const RESET: &str = "\x1b[0m";
pub const RED: &str = "\x1b[91m";
pub const BRED: &str = "\x1b[101m";
pub const GREEN: &str = "\x1b[92m";
pub const BGREEN: &str = "\x1b[102m";
pub const YELLOW: &str = "\x1b[93m";
pub const BYELLOW: &str = "\x1b[103m";
pub const BLUE: &str = "\x1b[94m";
pub const BBLUE: &str = "\x1b[104m";
pub const CYAN: &str = "\x1b[96m";
pub const BCYAN: &str = "\x1b[106m";
pub const WHITE: &str = "\x1b[97m";
pub const BWHITE: &str = "\x1b[107m";
pub const GRAY: &str = "\x1b[90m";
pub const BGRAY: &str = "\x1b[100m";
pub const BLACK: &str = "\x1b[30m";
pub const BBLACK: &str = "\x1b[40m";
pub const BOLD: &str = "\x1b[1m";
pub const ITALIC: &str = "\x1b[3m";
pub const UNDERLINE: &str = "\x1b[4m";

#[cfg(feature = "no-ansi")]
#[macro_export]
macro_rules! color {
    ($c:ident) => {
        ""
    };
}

#[cfg(not(feature = "no-ansi"))]
#[macro_export]
macro_rules! color {
    ($c:ident) => {
        $crate::utils::ansi_colors::$c
    };
}

#[cfg(feature = "no-ansi")]
#[macro_export]
macro_rules! colorize {
    ($l:expr; $($c:ident),+) => {
        $l
    };
}

#[cfg(not(feature = "no-ansi"))]
#[macro_export]
macro_rules! colorize {
    ($l:expr; $c:ident $(, $cc:ident)*) => {
        $crate::color!($c).to_owned() $(+ $crate::color!($cc))* + ($l) + $crate::color!(RESET)
    };
}
