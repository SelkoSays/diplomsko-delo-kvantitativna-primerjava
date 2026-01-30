use std::collections::VecDeque;

use crate::random::Random;
use crate::tracker::GLOBAL_TRACKER;
use crate::utils::args_parser::Policy;

pub struct Block {
    data: Vec<u8>,
    ttl: isize,
    ttl_org: isize,
}

impl Drop for Block {
    fn drop(&mut self) {
        unsafe {
            GLOBAL_TRACKER.remove_alloc(self.data.capacity());
        }
    }
}

impl Block {
    pub fn new(size: usize) -> Self {
        Self::with_ttl(size, -1)
    }

    pub fn with_ttl(size: usize, ttl: isize) -> Self {
        unsafe {
            GLOBAL_TRACKER.add_alloc(size);
        }
        Self {
            data: vec![0; size],
            ttl,
            ttl_org: ttl,
        }
    }

    #[inline(always)]
    pub fn tick(&mut self) {
        if self.ttl > 0 {
            self.ttl -= 1;
        }
    }

    #[inline(always)]
    pub const fn alive(&self) -> bool {
        self.ttl < 0 || self.ttl > 0
    }

    #[inline(always)]
    pub const fn ttl_org(&self) -> isize {
        self.ttl_org
    }
}

pub struct Pool {
    blocks: VecDeque<Block>,
    pub capacity: usize,
}

impl Pool {
    pub fn new(capacity: usize) -> Self {
        Self {
            blocks: VecDeque::with_capacity(capacity),
            capacity: capacity,
        }
    }

    pub fn add_block(&mut self, size: usize) {
        if self.blocks.len() >= self.capacity {
            return;
        }
        self.blocks.push_back(Block::new(size));
    }

    pub fn add_block_with_ttl(&mut self, size: usize, ttl: isize) {
        if self.blocks.len() >= self.capacity {
            return;
        }
        self.blocks.push_back(Block::with_ttl(size, ttl));
    }

    pub fn del_block(&mut self, policy: Policy, rng: &mut Random) {
        match policy {
            Policy::Lifo => self.blocks.pop_back(),
            Policy::Fifo => self.blocks.pop_front(),
            Policy::Random => self.blocks.remove(rng.uniform(0, self.blocks.len())),
            Policy::BigFirst => self.blocks.remove(
                self.blocks
                    .iter()
                    .enumerate()
                    .max_by_key(|(_, b)| b.data.capacity())
                    .map(|(i, _)| i)
                    .unwrap_or(0),
            ),
            Policy::SmallFirst => self.blocks.remove(
                self.blocks
                    .iter()
                    .enumerate()
                    .min_by_key(|(_, b)| b.data.capacity())
                    .map(|(i, _)| i)
                    .unwrap_or(0),
            ),
            Policy::Never => None,
        };
    }

    pub fn update_and_prune(&mut self) {
        let mut i = 0usize;
        while i < self.blocks.len() {
            self.blocks[i].tick();

            if !self.blocks[i].alive() {
                self.blocks.remove(i);
            } else {
                i += 1;
            }
        }
    }

    pub fn count(&self) -> usize {
        self.blocks.len()
    }
}
