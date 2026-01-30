// progress.rs
use std::io::{self, Write};
use std::time::{Duration, Instant};

#[derive(Debug)]
pub enum ProgressMode {
    Iterations(usize),
    Duration(Duration),
}

pub struct ProgressBar {
    pub mode: ProgressMode,
    current: usize,
    width: usize,
    pub start_time: Instant,
    last_update: Instant,
    update_interval: Duration,
    displayed: bool,
}

impl ProgressBar {
    pub fn from_iterations(total: usize) -> Self {
        Self::new(ProgressMode::Iterations(total))
    }

    pub fn from_duration(duration: Duration) -> Self {
        Self::new(ProgressMode::Duration(duration))
    }

    fn new(mode: ProgressMode) -> Self {
        Self {
            mode,
            current: 0,
            width: 50,
            start_time: Instant::now(),
            last_update: Instant::now(),
            update_interval: Duration::from_millis(100),
            displayed: false,
        }
    }

    pub fn with_width(mut self, width: usize) -> Self {
        self.width = width;
        self
    }

    pub fn display(&mut self, display: bool) {
        self.displayed = display;
    }

    pub fn update(&mut self) {
        self.current += 1;
        let now = Instant::now();

        if now.duration_since(self.last_update) >= self.update_interval || self.is_finished() {
            self.draw();
            self.last_update = now;
        }
    }

    pub fn is_finished(&self) -> bool {
        match self.mode {
            ProgressMode::Iterations(total) => self.current >= total,
            ProgressMode::Duration(duration) => self.start_time.elapsed() >= duration,
        }
    }

    pub fn finish(&mut self) {
        if self.displayed {
            self.draw();
            println!();
        }
    }

    fn get_progress(&self) -> f64 {
        match self.mode {
            ProgressMode::Iterations(total) => {
                if total > 0 {
                    (self.current as f64 / total as f64).min(1.0)
                } else {
                    1.0
                }
            }
            ProgressMode::Duration(duration) => {
                let elapsed = self.start_time.elapsed();
                if duration.as_secs_f64() > 0.0 {
                    (elapsed.as_secs_f64() / duration.as_secs_f64()).min(1.0)
                } else {
                    1.0
                }
            }
        }
    }

    fn draw(&self) {
        if !self.displayed {
            return;
        }

        let progress = self.get_progress();
        let percent = (progress * 100.0) as usize;
        let filled = (progress * self.width as f64) as usize;

        let elapsed = self.start_time.elapsed();
        let rate = if elapsed.as_secs_f64() > 0.0 {
            self.current as f64 / elapsed.as_secs_f64()
        } else {
            0.0
        };

        print!("\r[");
        for i in 0..self.width {
            if i < filled {
                print!("=");
            } else if i == filled && filled < self.width {
                print!(">");
            } else {
                print!(" ");
            }
        }
        print!("] ");

        match self.mode {
            ProgressMode::Iterations(total) => {
                print!("{}/{} ({:3}%) ", self.current, total, percent);
            }
            ProgressMode::Duration(duration) => {
                let elapsed = self.start_time.elapsed();
                print!(
                    "{:.1}s/{:.1}s ({:3}%) ",
                    elapsed.as_secs_f64(),
                    duration.as_secs_f64(),
                    percent
                );
            }
        }

        if rate > 0.0 {
            print!("{:.1}/s ", rate);
        }

        match self.mode {
            ProgressMode::Iterations(total) if rate > 0.0 && self.current < total => {
                let eta = Duration::from_secs_f64((total - self.current) as f64 / rate);
                print!("ETA: {:02}:{:02}", eta.as_secs() / 60, eta.as_secs() % 60);
            }
            ProgressMode::Duration(duration) => {
                let remaining = duration.saturating_sub(self.start_time.elapsed());
                print!(
                    "ETA: {:02}:{:02}",
                    remaining.as_secs() / 60,
                    remaining.as_secs() % 60
                );
            }
            _ => {}
        }

        io::stdout().flush().unwrap();
    }

    pub fn iter(&mut self) -> ProgressBarIterator<'_> {
        ProgressBarIterator {
            bar: self,
            diplayed_finish: false,
        }
    }
}

impl Iterator for ProgressBar {
    type Item = usize;

    fn next(&mut self) -> Option<Self::Item> {
        if self.is_finished() {
            None
        } else {
            self.update();
            Some(self.current)
        }
    }
}

pub struct ProgressBarIterator<'a> {
    bar: &'a mut ProgressBar,
    diplayed_finish: bool,
}

impl<'a> Iterator for ProgressBarIterator<'a> {
    type Item = usize;

    fn next(&mut self) -> Option<Self::Item> {
        if self.bar.is_finished() {
            if !self.diplayed_finish {
                self.diplayed_finish = true;
                self.bar.finish();
            }
            None
        } else {
            self.bar.update();
            Some(self.bar.current)
        }
    }
}
