#![allow(static_mut_refs)]
pub mod actions;
pub mod pool;
pub mod progress;
pub mod random;
pub mod tracker;
pub mod utils;

use std::{io::Write, time::Duration};

use utils::args_parser::Args;

use crate::{actions::Actions, pool::Pool, progress::ProgressBar, random::Random};

fn main() {
    let args = match Args::parse() {
        Ok(args) => args,
        Err(s) if s == "help" => return,
        s => s.unwrap(),
    };

    let mut output = if args.output.unwrap_str().len() > 0 && args.snap_interval.unwrap_int() > 0 {
        std::fs::OpenOptions::new()
            .create(true)
            .write(true)
            .truncate(true)
            .open(args.output.unwrap_str())
            .ok()
    } else {
        None
    };

    tracker::Tracker::write_header(output.as_mut());
    unsafe {
        // Safe because we are just logging state
        tracker::GLOBAL_TRACKER.write(output.as_mut());
    }

    {
        let rng = Random::seeded(args.seed.unwrap_int() as u64);
        let pool = Pool::new(args.capacity.unwrap_int());

        let mut actions = Actions::new(&args, rng, pool);

        let mut progress = if args.duration.unwrap_time() == 0 {
            eprintln!("ITER {}", args.iterations);
            ProgressBar::from_iterations(args.iterations.unwrap_int())
        } else {
            eprintln!("TIME {}", args.duration);
            ProgressBar::from_duration(Duration::from_secs(args.duration.unwrap_time() as u64))
        };

        progress.display(args.display.unwrap_bool());

        let snap_interval = args.snap_interval.unwrap_int().max(1);
        for i in progress.iter() {
            actions.block_action();

            if (i % snap_interval) == 0 {
                unsafe {
                    // Safe because we are just logging state
                    tracker::GLOBAL_TRACKER.write(output.as_mut());
                }
            }
        }
    }

    unsafe {
        // Safe because we are just logging state
        tracker::GLOBAL_TRACKER.write(output.as_mut());
    }

    if let Some(mut out) = output {
        let _ = out.flush();
    }
}
