#[cfg(not(feature = "log-level-debug"))]
#[macro_export]
macro_rules! debug {
    ($($any:tt)*) => {};
}
#[cfg(not(feature = "log-level-info"))]
#[macro_export]
macro_rules! info {
    ($($any:tt)*) => {};
}
#[cfg(not(feature = "log-level-warn"))]
#[macro_export]
macro_rules! warn {
    ($($any:tt)*) => {};
}
#[cfg(not(feature = "log-level-error"))]
#[macro_export]
macro_rules! error {
    ($($any:tt)*) => {};
}

#[cfg(feature = "log-level-debug")]
#[macro_export]
macro_rules! debug {
    () => {};

    ($($arg:tt)*) => {
        eprintln!("{} {}", $crate::colorize!("[DEBUG]"; YELLOW), format!($($arg)*))
    };
}

#[cfg(feature = "log-level-info")]
#[macro_export]
macro_rules! info {
    () => {};

    ($($arg:tt)*) => {
        println!("{} {}", $crate::colorize!("[INFO]"; CYAN), format!($($arg)*))
    };
}

#[cfg(feature = "log-level-warn")]
#[macro_export]
macro_rules! warn {
    () => {};

    ($($arg:tt)*) => {
        eprintln!("{} {}", $crate::colorize!("[WARN]"; YELLOW), format!($($arg)*))
    };
}

#[cfg(feature = "log-level-error")]
#[macro_export]
macro_rules! error {
    () => {};

    ($($arg:tt)*) => {
        eprintln!("{} {}", $crate::colorize!("[ERROR]"; RED), format!($($arg)*))
    }
}
