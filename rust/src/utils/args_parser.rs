use std::{env, fmt::Display, str::FromStr};

#[derive(Debug, Clone)]
pub enum Arg {
    Int(usize),
    Float(f64),
    Str(String),
    Bool(bool),
    Size(usize),
    Time(usize),
    Enum(usize, &'static [&'static str]), // (index, &choices)
    IntList(Vec<usize>),
    SizeList(Vec<usize>),
}

impl Display for Arg {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Arg::Int(i) => write!(f, "{}", i),
            Arg::Float(flt) => write!(f, "{}", flt),
            Arg::Str(s) => write!(f, "{}", s),
            Arg::Bool(b) => write!(f, "{}", b),
            Arg::Size(_) => write!(f, "{}", self.size_str()),
            Arg::Time(_) => write!(f, "{}", self.time_str()),
            Arg::Enum(i, items) => write!(f, "{}", items[*i]),
            Arg::IntList(_) => write!(f, "{}", self.list_str()),
            Arg::SizeList(_) => write!(f, "{}", self.list_str()),
        }
    }
}

impl Arg {
    fn size_str(&self) -> String {
        assert!(matches!(self, Self::Size(_)));

        let size = self.unwrap_size();
        if size < (1 << 10) {
            format!("{size}B")
        } else if size < (1 << 20) {
            format!("{:.1}KiB", size as f64 / (1 << 10) as f64)
        } else if size < (1 << 30) {
            format!("{:.1}MiB", size as f64 / (1 << 20) as f64)
        } else if size < (1 << 40) {
            format!("{:.1}GiB", size as f64 / (1 << 30) as f64)
        } else {
            format!("{size}B")
        }
    }

    fn time_str(&self) -> String {
        assert!(matches!(self, Self::Time(_)));

        let size = self.unwrap_time();
        if size < 60 {
            format!("{}s", size)
        } else if (size % 60) == 0 {
            format!("{}min", size / 60)
        } else {
            format!("{}min {}s", size / 60, size % 60)
        }
    }

    fn list_str(&self) -> String {
        assert!(matches!(self, Self::IntList(_) | Self::SizeList(_)));

        match self {
            Self::IntList(list) => {
                format!(
                    "[{}]",
                    list.iter()
                        .enumerate()
                        .map(|(idx, v)| if idx < (list.len() - 1) {
                            format!("{},", v)
                        } else {
                            format!("{}", v)
                        })
                        .collect::<String>()
                )
            }
            Self::SizeList(list) => {
                format!(
                    "[{}]",
                    list.iter()
                        .enumerate()
                        .map(|(idx, v)| if idx < (list.len() - 1) {
                            format!("{},", Arg::Size(*v).size_str())
                        } else {
                            format!("{}", Arg::Size(*v).size_str())
                        })
                        .collect::<String>()
                )
            }
            _ => unreachable!("Arg::XList"),
        }
    }
}

impl Arg {
    pub fn unwrap_int(&self) -> usize {
        match self {
            Self::Int(i) => *i,
            _ => panic!("Argument not Int but {self:?}"),
        }
    }

    pub fn unwrap_float(&self) -> f64 {
        match self {
            Self::Float(f) => *f,
            _ => panic!("Argument not Float but {self:?}"),
        }
    }

    pub fn unwrap_str(&self) -> String {
        match self {
            Self::Str(s) => s.clone(),
            _ => panic!("Argument not Str but {self:?}"),
        }
    }

    pub fn unwrap_bool(&self) -> bool {
        match self {
            Self::Bool(b) => *b,
            _ => panic!("Argument not Bool but {self:?}"),
        }
    }

    pub fn unwrap_size(&self) -> usize {
        match self {
            Self::Size(s) => *s,
            _ => panic!("Argument not Size but {self:?}"),
        }
    }

    pub fn unwrap_time(&self) -> usize {
        match self {
            Self::Time(t) => *t,
            _ => panic!("Argument not Time but {self:?}"),
        }
    }

    pub fn unwrap_enum<E: From<usize>>(&self) -> E {
        match self {
            Self::Enum(i, _) => (*i).into(),
            _ => panic!("Argument not Enum but {self:?}"),
        }
    }

    pub fn unwrap_intlist(&self) -> &Vec<usize> {
        match self {
            Self::IntList(l) => l,
            _ => panic!("Argument not IntList but {self:?}"),
        }
    }

    pub fn unwrap_sizelist(&self) -> &Vec<usize> {
        match self {
            Self::SizeList(l) => l,
            _ => panic!("Argument not SizeList but {self:?}"),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum ArgType {
    Int,
    Float,
    Str,
    Bool,
    Size,
    Time,
    Enum,
    IntList,
    SizeList,
}

impl Arg {
    /// Parse the string `s` into *the* variant you ask for.
    ///
    /// - `kind` tells you which variant to produce.
    /// - `choices` is only consulted when `kind == ArgType::Enum`.
    ///   It must be `Some(&[...])` or you get an error.
    pub fn from_str(
        s: &str,
        kind: ArgType,
        choices: Option<&'static [&'static str]>,
    ) -> Result<Self, String> {
        match kind {
            ArgType::Int => {
                let v = s
                    .parse::<usize>()
                    .map_err(|e| format!("invalid integer `{}`: {}", s, e))?;
                Ok(Arg::Int(v))
            }
            ArgType::Float => {
                let v = s
                    .parse::<f64>()
                    .map_err(|e| format!("invalid float `{}`: {}", s, e))?;
                Ok(Arg::Float(v))
            }
            ArgType::Str => Ok(Arg::Str(s.to_owned())),
            ArgType::Size => {
                let idx = s
                    .char_indices()
                    .skip_while(|(_, c)| c.is_numeric())
                    .map(|(i, _)| i)
                    .next();

                let v = match idx {
                    None => s
                        .parse::<usize>()
                        .map_err(|e| format!("invalid size `{}`: {}", s, e))?,
                    Some(i) => {
                        let size = (&s[..i])
                            .parse::<usize>()
                            .map_err(|e| format!("invalid size `{}`: {}", s, e))?;
                        match &s[i..] {
                            "B" => size,
                            "KB" => size * 1_000,
                            "KiB" => size * (1 << 10),
                            "MB" => size * 1_000_000,
                            "MiB" => size * (1 << 20),
                            "GB" => size * 1_000_000_000,
                            "GiB" => size * (1 << 30),
                            _ => {
                                return Err(format!(
                                    "invalid size unit {}, (supported units B, KB, KiB, MB, MiB, GB, GiB)",
                                    &s[..i]
                                ));
                            }
                        }
                    }
                };

                Ok(Arg::Size(v))
            }
            ArgType::Time => {
                let idx = s
                    .char_indices()
                    .skip_while(|(_, c)| c.is_numeric())
                    .map(|(i, _)| i)
                    .next();

                let v = match idx {
                    None => s
                        .parse::<usize>()
                        .map_err(|e| format!("invalid duration `{}`: {}", s, e))?,
                    Some(i) => {
                        let secs = (&s[..i])
                            .parse::<usize>()
                            .map_err(|e| format!("invalid duration `{}`: {}", s, e))?;
                        match &s[i..] {
                            "s" => secs,
                            "min" => secs * 60,
                            _ => {
                                return Err(format!(
                                    "invalid duration unit {}, (supported units s, min)",
                                    &s[..i]
                                ));
                            }
                        }
                    }
                };

                Ok(Arg::Time(v))
            }
            ArgType::Enum => {
                let choices =
                    choices.ok_or_else(|| "enum parser needs a choices slice".to_string())?;
                match choices.iter().position(|&c| c == s) {
                    Some(idx) => Ok(Arg::Enum(idx, choices)),
                    None => Err(format!("`{}` is not one of {:?}", s, choices)),
                }
            }
            ArgType::IntList => Ok(Arg::IntList(
                s.split(',')
                    .map(|n| Arg::from_str(n, ArgType::Int, None).unwrap().unwrap_int())
                    .collect(),
            )),
            ArgType::SizeList => Ok(Arg::SizeList(
                s.split(',')
                    .map(|n| Arg::from_str(n, ArgType::Size, None).unwrap().unwrap_size())
                    .collect(),
            )),
            ArgType::Bool => {
                unreachable!("ArgType Bool from_str");
            }
        }
    }
}

macro_rules! define_args {
    (
        struct $name:ident {
        $(
            $category:literal [
                $(
                    $field:ident : $ty:ident {
                        $(short: $short:literal,)?
                        long: $long:literal,
                        metavar: $metavar:literal,
                        help: $help:expr
                        $(, options: [ $($option:literal),+ ] )?
                        $(, default: $default:expr )?
                        $(, mutex: [ $($other_field:ident),* ])*
                        $(, check: |$s:ident| $code:block )?
                        $(,)?
                    }
                ),* $(,)?
            ]
        ),* $(,)?
    }) => {

        /// The user‐facing struct
        #[derive(Debug)]
        pub struct $name {
            $(
                $(
                /// `$help`
                pub $field: Arg,
                )*
            )*
        }

        impl std::fmt::Display for $name {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                writeln!(f, "{} {{", stringify!($name))?;
                $($(
                    writeln!(f, "  {}: {},", stringify!($field),self.$field)?;
                )*)*
                write!(f, "}}")
            }
        }

        impl $name {

            pub fn usage(program: &str) {
                println!("Usage: {program} [options]\n\n  <BYTES>  supports units B, KB, KiB, MB, MiB, GB, GiB (e.g. 10KB)\n  <SECS>   supports units s, min (e.g. 1min)\n  <L[...]> comma-separeted list (e.g. 1,2,3,4)");

                $(
                    println!("\n{}:", $category);

                    $(
                        define_args!(@print_arg $ty, $long, $metavar, $help $(, $short)?);
                        define_args!(@print_arg_default $ty $(, $default)? $(, [ $($option),+ ])?);
                    )*
                )*

                define_args!(@print_arg Bool, "help", "", "Show this help text and exit", 'h');
            }

            pub fn check(&mut self) {
                $($(
                    $(
                        let $s = &mut *self;
                        $code
                    )?
                )*)*
            }

            /// Parse from `std::env::args()`
            pub fn parse() -> Result<Self, String> {
                let mut args = env::args();
                let program = args.next().expect("Program name should always exist");
                // intermediate storage
                $(
                    $(
                    let mut $field: Option<Arg> = None;
                    )*
                )*

                while let Some(arg) = args.next() {
                    match arg.as_str() {
                        "-h" | "--help" => {
                            Self::usage(&program);
                            return Err("help".to_string());
                        },
                        $(
                            $(
                            // match either `-x` or `--xxx`
                            x if $(x == concat!("-", $short) ||)? x == concat!("--", $long) => {
                                $field = define_args!(@parse_field_next $ty, args, $long $(, [ $($option),+ ] )?);
                            },
                            x if (ArgType::$ty != ArgType::Bool) && define_args!(@short_match x $(, $short)?) => {
                                $field = define_args!(@parse_field_cur $ty, x, concat!("-", $($short)?) $(, [ $($option),+ ] )?);
                            },
                            x if (ArgType::$ty != ArgType::Bool) && x.starts_with(concat!("--", $long)) => {
                                $field = define_args!(@parse_field_cur $ty, x, concat!("--", $long) $(, [ $($option),+ ] )?);
                            },
                            )*
                        )*
                        other => return Err(format!("Unknown argument `{}`", other)),
                    }
                }


                $($(
                    $(
                        $(
                            if $field.is_some() && $other_field.is_some() {
                                panic!("{} Arguments --{} and --{} are mutually exclusive", $crate::colorize!("[ERROR]"; RED), stringify!($field), stringify!($other_field));
                            }
                        )*
                    )?
                )*)*

                let mut ret = $name {
                    $($(
                        $field: match $field {
                            Some(v) => v,
                            None => define_args!(@field_default $ty, $long $(, $default)? $(, [$($option),+])?),
                        },
                    )*)*
                };

                ret.check();
                Ok(ret)
            }
        }
    };

    (@print_arg $ty:ident, $long:literal, $metavar:literal, $help:literal) => {
        println!("       --{:<13} {:<8} {}", $long, $metavar, $help)
    };
    (@print_arg Enum, $long:literal, $metavar:literal, $help:literal) => {
        println!("       --{:<13} {:<8} {}; one of:", $long, $metavar, $help)
    };
    (@print_arg $ty:ident, $long:literal, $metavar:literal, $help:literal, $short:literal) => {
        println!("  -{},  --{:<13} {:<8} {}", $short, $long, $metavar, $help)
    };
    (@print_arg Enum, $long:literal, $metavar:literal, $help:literal, $short:literal) => {
        println!("  -{},  --{:<13} {:<8} {}; one of:", $short, $long, $metavar, $help)
    };

    (@print_arg_default $ty:ident) => {};
    (@print_arg_default Str, $default:expr) => {
        if ($default != "".to_string()) {
            println!("{:33} (default: {})", "", $default)
        }
    };
    (@print_arg_default IntList, $default:expr) => {
        if ($default.len() > 0) {
            println!("{:33} (default: {})", "", Arg::IntList($default))
        }
    };
    (@print_arg_default SizeList, $default:expr) => {
        if ($default.len() > 0) {
            println!("{:33} (default: {})", "", Arg::SizeList($default))
        }
    };
    (@print_arg_default $ty:ident, $default:expr) => {
        println!("{:33} (default: {})", "", Arg::$ty($default))
    };
    (@print_arg_default Enum, $default:expr, [ $($option:literal),+ ]) => {
        define_args!(@print_index_opts $default; $($option),+)
    };

    (@print_index_opts $default:expr; $($opt:literal),+) => {
        define_args!(@print_index_opts_inner $default; 0; $($opt),+)
    };
    (@print_index_opts_inner $default:expr; $idx:expr; $head:literal) => {
        println!("{:33} - {}{}", "", $head, if ($default) == ($idx) { " (default)" } else { "" } )
    };
    (@print_index_opts_inner $default:expr; $idx:expr; $head:literal, $($tail:literal),+) => {
        define_args!(@print_index_opts_inner $default; $idx; $head);
        define_args!(@print_index_opts_inner $default; $idx + 1; $($tail),+)
    };


    (@short_match $x:ident) => {
        false
    };
    (@short_match $x:ident, $short:literal) => {
        $x.starts_with(concat!("-", $short))
    };

    // Bool flags don’t consume a value; just set `true`.
    (@parse_field_next Bool, $args:ident, $long:expr) => {
        Some(Arg::Bool(true))
    };
    (@parse_field_next Enum, $args:ident, $long:expr, [ $($option:literal),+ ]) => {{
        let val = $args.next()
            .ok_or_else(|| format!("Expected value for `{}`", $long))?;
        let val = Arg::from_str(&val, ArgType::Enum, Some(&[$($option),+]))?;
        Some(val)
    }};
    (@parse_field_next $ty:ident, $args:ident, $long:expr) => {{
        let val = $args.next()
            .ok_or_else(|| format!("Expected value for `{}`", $long))?;
        let val = Arg::from_str(&val, ArgType::$ty, None)?;
        Some(val)
    }};

    (@parse_field_cur Enum, $x:ident, $pfx:expr, [ $($option:literal),+ ]) => {{
        let val = $x.strip_prefix($pfx).expect("Identified flag should always start with it's own prefix");
        let val = val.strip_prefix("=").unwrap_or(val);

        Some(Arg::from_str(val, ArgType::Enum, Some(&[$($option),+]))?)
    }};
    (@parse_field_cur $ty:ident, $x:ident, $pfx:expr) => {{
        let val = $x.strip_prefix($pfx).expect("Identified flag should always start with it's own prefix");
        let val = val.strip_prefix("=").unwrap_or(val);

        Some(Arg::from_str(val, ArgType::$ty, None)?)
    }};

    // No default -> error if missing
    (@field_default $ty:ident, $long:expr) => {
        return Err(format!("Missing required argument `{}`", $long))
    };
    // With default
    (@field_default Enum, $long:expr, $default:expr, [$($option:literal),+]) => {
        Arg::Enum($default, &[$($option),+])
    };
    (@field_default Str, $long:expr, $default:expr) => {
        Arg::Str($default.into())
    };
    (@field_default $ty:ident, $long:expr, $default:expr) => {
        Arg::$ty($default)
    };
}

#[derive(Debug, PartialEq)]
pub enum Policy {
    Lifo,
    Fifo,
    Random,
    BigFirst,
    SmallFirst,
    Never,
}

impl From<usize> for Policy {
    fn from(value: usize) -> Self {
        match value {
            0 => Self::Lifo,
            1 => Self::Fifo,
            2 => Self::Random,
            3 => Self::BigFirst,
            4 => Self::SmallFirst,
            5 => Self::Never,
            _ => panic!("Unknown policy"),
        }
    }
}

impl FromStr for Policy {
    type Err = &'static str;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "lifo" => Ok(Self::Lifo),
            "fifo" => Ok(Self::Fifo),
            "random" => Ok(Self::Random),
            "big-first" => Ok(Self::BigFirst),
            "small-first" => Ok(Self::SmallFirst),
            "never" => Ok(Self::Never),
            _ => Err("Unknown policy"),
        }
    }
}

#[derive(Debug, PartialEq)]
pub enum Distribution {
    Uniform,
    Exp,
    Powerlaw,
}

impl From<usize> for Distribution {
    fn from(value: usize) -> Self {
        match value {
            0 => Self::Uniform,
            1 => Self::Exp,
            2 => Self::Powerlaw,
            _ => panic!("Unknown distribution"),
        }
    }
}

impl FromStr for Distribution {
    type Err = &'static str;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "uniform" => Ok(Self::Uniform),
            "exp" => Ok(Self::Exp),
            "powerlaw" => Ok(Self::Powerlaw),
            _ => Err("Unknown distribution"),
        }
    }
}

#[derive(Debug, PartialEq)]
pub enum Trend {
    None,
    Grow,
    Shrink,
    Saw,
}

impl From<usize> for Trend {
    fn from(value: usize) -> Self {
        match value {
            0 => Self::None,
            1 => Self::Grow,
            2 => Self::Shrink,
            3 => Self::Saw,
            _ => panic!("Unknown trend"),
        }
    }
}

impl FromStr for Trend {
    type Err = &'static str;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "never" => Ok(Self::None),
            "grow" => Ok(Self::Grow),
            "shrink" => Ok(Self::Shrink),
            "saw" => Ok(Self::Saw),
            _ => Err("Unknown trend"),
        }
    }
}

#[derive(Debug, PartialEq)]
pub enum SizeMode {
    Exact,
    Nearest,
}

impl From<usize> for SizeMode {
    fn from(value: usize) -> Self {
        match value {
            0 => Self::Exact,
            1 => Self::Nearest,
            _ => panic!("Unknown size-mode"),
        }
    }
}

impl FromStr for SizeMode {
    type Err = &'static str;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "exact" => Ok(Self::Exact),
            "nearest" => Ok(Self::Nearest),
            _ => Err("Unknown size-mode"),
        }
    }
}

#[derive(Debug, PartialEq)]
pub enum TTLMode {
    Off,
    Fixed,
    List,
}

impl From<usize> for TTLMode {
    fn from(value: usize) -> Self {
        match value {
            0 => Self::Off,
            1 => Self::Fixed,
            2 => Self::List,
            _ => panic!("Unknown ttl-mode"),
        }
    }
}

impl FromStr for TTLMode {
    type Err = &'static str;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "off" => Ok(Self::Off),
            "fixed" => Ok(Self::Fixed),
            "list" => Ok(Self::List),
            _ => Err("Unknown ttl-mode"),
        }
    }
}

define_args! {
    struct Args {
        "General run control" [
            policy: Enum {
                short: 'p',
                long: "policy",
                metavar: "POLICY",
                help: "Free policy",
                options: ["lifo", "fifo", "random", "big-first", "small-first", "never"],
                default: 0,
                check: |this| {
                    if (this.policy.unwrap_enum::<Policy>() == Policy::Never)
                    && (this.ttl_mode.unwrap_enum::<TTLMode>() == TTLMode::Off) {
                        panic!(
                            "{} If --policy is 'never', then --ttl-mode should not be off",
                            crate::colorize!("[ERROR]"; RED)
                        );
                    }
                }
            },
            iterations: Int {
                short: 'n',
                long: "iterations",
                metavar: "N",
                help: "Total alloc/free ops",
                default: 10_000,
                mutex: [duration]
            },
            duration: Time {
                short: 'd',
                long: "duration",
                metavar: "SECS",
                help: "Run for this many seconds (mutually exclusive with --iterations)",
                default: 0
            },
            alloc_freq: Float {
                short: 'f',
                long: "alloc-freq",
                metavar: "F",
                help: "Frequency [0, 1] of allocations when policy not 'never'",
                default: 0.7,
                check: |this| {
                    if this.alloc_freq.unwrap_float() < 0.0 || this.alloc_freq.unwrap_float() > 1.0 {
                        panic!("{} --alloc-freq should be on the interval [0, 1], but is {}", crate::colorize!("[ERROR]"; RED), this.alloc_freq);
                    }
                }
            },
            seed: Int {
                short: 's',
                long: "seed",
                metavar: "N",
                help: "RNG seed (0=time)",
                default: 0,
                check: |this| {
                    if this.seed.unwrap_int() == 0 {
                        this.seed = Arg::Int(
                            std::time::SystemTime::now()
                                .duration_since(std::time::SystemTime::UNIX_EPOCH)
                                .expect("Time goes forward")
                                .as_secs() as usize,
                        );
                    }
                }
            },
        ],
        "Pool sizing" [
            capacity: Int {
                short: 'c',
                long: "capacity",
                metavar: "N",
                help: "Max live blocks",
                default: 10_000
            }
        ],
        "Block-size" [
            min_size: Size {
                short: 'a',
                long: "min-size",
                metavar: "BYTES",
                help: "Min block size",
                default: 16,
                check: |this| {
                    if this.min_size.unwrap_size() > this.max_size.unwrap_size() {
                        this.max_size = Arg::Size(this.min_size.unwrap_size());
                    }
                }
            },
            max_size: Size {
                short: 'A',
                long: "max-size",
                metavar: "BYTES",
                help: "Max block size",
                default: 1 << 20
            },
            size_trend: Enum {
                long: "size-trend",
                metavar: "TREND",
                help: "Set block size mutation through time. If 'none' sizes are random",
                options: ["none", "grow", "shrink", "saw"],
                default: 0,
            },
            size_step: Size {
                long: "size-step",
                metavar: "BYTES",
                help: "Set block mutation size",
                default: 1 << 10,
                check: |this| {
                    if this.size_step.unwrap_size() == 0 {
                        panic!(
                            "{} --size-step should not be zero",
                            crate::colorize!("[ERROR]"; RED)
                        );
                    }
                }
            },
            trend_jitter: Size {
                long: "trend-jitter",
                metavar: "BYTES",
                help: "Add uniform +- jitter to mutiation size",
                default: 0
            },
            size_list: SizeList {
                long: "size-list",
                metavar: "L[BYTES]",
                help: "Only use provided block sizes",
                default: Vec::<usize>::new()
            },
            size_mode: Enum {
                long: "size-mode",
                metavar: "MODE",
                help: "Choose size from list. trend/distribution for nearest",
                options: ["exact", "nearest"],
                default: 0,
            },
            size_weights: IntList {
                long: "size-weights",
                metavar: "L[N]",
                help: "Set weights of size list (in %)",
                default: Vec::<usize>::new(),
                check: |this| {
                    if (this.size_list.unwrap_sizelist().len() > 0)
                    && (this.size_weights.unwrap_intlist().len() > 0) &&
                    (this.size_list.unwrap_sizelist().len() != this.size_weights.unwrap_intlist().len()) {
                        panic!(
                            "{} Number of weights doesn't match to the number of items in size-list. W({}) != L({})",crate::colorize!("[ERROR]"; RED),
                            this.size_weights.unwrap_intlist().len(),
                            this.size_list.unwrap_sizelist().len()
                        );
                    }
                }
            },
        ],
        "Block-size distribution" [
            distribution: Enum {
                short: 'P',
                long: "distribution",
                metavar: "TYPE",
                help: "Size distribution",
                options: ["uniform", "exp", "powerlaw"],
                default: 0,
                check: |this| {
                    if (this.distribution.unwrap_enum::<Distribution>() == Distribution::Powerlaw)
                        && (this.dist_param.unwrap_float() == 0.0)
                    {
                        panic!(
                            "{} Alpha parameter for powerlaw distribution should not be 0",
                            crate::colorize!("[ERROR]"; RED)
                        );
                    }
                }
            },
            dist_param: Float {
                short: 'r',
                long: "dist-param",
                metavar: "F",
                help: "Parameter for non-uniform distributions {exp(lambda), powerlaw(alpha)}",
                default: 1.0
            }
        ],
        "Block lifetime" [
            ttl_mode: Enum {
                long: "ttl-mode",
                metavar: "MODE",
                help: "Set lifetime of blocks",
                options: ["off", "fixed", "list"],
                default: 0
            },
            ttl_fixed: Int {
                long: "ttl-fixed",
                metavar: "N",
                help: "Number of cycles blocks will live",
                default: 10,
                check: |this| {
                    if this.ttl_fixed.unwrap_int() == 0 {
                        panic!(
                            "{} --ttl-fixed should not be zero",
                            crate::colorize!("[ERROR]"; RED)
                        );
                    }
                }
            },
            ttl_list: IntList {
                long: "ttl-list",
                metavar: "L[N]",
                help: "Use provided liftimes uniformly",
                default: Vec::<usize>::new()
            },
            ttl_weights: IntList {
                long: "ttl-weights",
                metavar: "L[N]",
                help: "Set weights for list of lifetimes",
                default: Vec::<usize>::new(),
                check: |this| {
                    if (this.ttl_list.unwrap_intlist().len() > 0)
                    && (this.ttl_weights.unwrap_intlist().len() > 0) &&
                    (this.ttl_list.unwrap_intlist().len() != this.ttl_weights.unwrap_intlist().len()) {
                        panic!(
                            "{} Number of weights doesn't match to the number of items in lifetime list. W({}) != L({})",crate::colorize!("[ERROR]"; RED),
                            this.ttl_weights.unwrap_intlist().len(),
                            this.ttl_list.unwrap_intlist().len()
                        );
                    }
                }
            },
        ],
        "Instrumentation & output" [
            snap_interval: Int {
                short: 'i',
                long: "snap-interval",
                metavar: "N",
                help: "Every N ops, snapshot and log stats",
                default: 1000
            },
            output: Str {
                short: 'o',
                long: "output",
                metavar: "FILE",
                help: "Path to CSV metrics log",
                default: ""
            },
            display: Bool {
                long: "display",
                metavar: "",
                help: "Display a progress bar",
                default: false,
            }
        ]
    }
}
