use std::{ptr::addr_of, time::SystemTime};

use crate::utils::args_parser::Distribution;

#[derive(Debug)]
pub struct Random(u64);

impl Random {
    pub fn new() -> Self {
        Self(0x853c49e6748fea9b)
    }

    pub fn seeded(mut seed: u64) -> Self {
        if seed == 0 {
            let time = std::time::SystemTime::now()
                .duration_since(SystemTime::UNIX_EPOCH)
                .expect("Time goes forward")
                .as_secs();
            seed = time ^ (addr_of!(seed).addr() as u64);
        }
        let mut random_state = Self::splitmix64(seed);
        if random_state == 0 {
            random_state = 0x9e3779b97f4a7c15;
        }

        Self(random_state)
    }
}

impl Random {
    pub fn coin_flip(&mut self) -> bool {
        return (self.uniform(0, 2) % 2) == 1;
    }

    pub fn uniform01(&mut self) -> f64 {
        let r = self.xs64star() >> 11;
        return r as f64 * (1.0 / ((1u64 << 53) as f64));
    }

    pub fn uniform(&mut self, min: usize, max: usize) -> usize {
        debug_assert!(min <= max);

        min + ((max - min) as f64 * self.uniform01()) as usize
    }

    pub fn exp(&mut self, min: usize, max: usize, lambda: f64) -> usize {
        debug_assert!(min <= max);
        debug_assert!(lambda > 0.0);

        let u = self.uniform01();
        let x = -(1.0 - u).ln() / lambda;

        return (x as usize).clamp(min, max);
    }

    pub fn powerlaw(&mut self, min: usize, max: usize, alpha: f64) -> usize {
        debug_assert!(min <= max);

        let u: f64 = self.uniform01();
        let x: f64 = if (alpha - 1.0).abs() < 1e-8 {
            let ratio: f64 = max as f64 / min as f64;
            min as f64 * ratio.powf(u)
        } else {
            let exp = 1.0 - alpha;
            let min_e = (min as f64).powf(exp);
            let max_e = (max as f64).powf(exp);
            let val_e = min_e + u * (max_e - min_e);
            val_e.powf(1.0 / exp)
        };
        // clamp just in case of fp drift
        return (x as usize).clamp(min, max);
    }

    pub fn next(
        &mut self,
        min: usize,
        max: usize,
        param: f64,
        distribution: Distribution,
    ) -> usize {
        match distribution {
            Distribution::Uniform => self.uniform(min, max),
            Distribution::Exp => self.exp(min, max, param as f64),
            Distribution::Powerlaw => self.powerlaw(min, max, param as f64),
        }
    }

    pub fn choice(&mut self, l: &Vec<usize>) -> usize {
        debug_assert!(l.len() > 0);

        let idx = self.uniform(0, l.len());
        l[idx]
    }

    pub fn wchoice(&mut self, l: &Vec<usize>, weights: &Vec<usize>) -> usize {
        debug_assert!(l.len() > 0);

        if weights.len() == 0 {
            return self.choice(l);
        }

        let vote = self.uniform(0, weights.iter().sum());
        let mut a = 0;
        let mut idx = 0;
        for (i, w) in weights.iter().enumerate() {
            a += w;
            if vote < a {
                idx = i;
                break;
            }
        }

        l[idx]
    }
}

impl Random {
    fn splitmix64(x: u64) -> u64 {
        let mut z: u64 = x.wrapping_add(0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)).wrapping_mul(0xbf58476d1ce4e5b9);
        z = (z ^ (z >> 27)).wrapping_mul(0x94d049bb133111eb);
        return z ^ (z >> 31);
    }

    fn xs64star(&mut self) -> u64 {
        let mut x = self.0;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        self.0 = x;
        return x.wrapping_mul(2685821657736338717);
    }
}
