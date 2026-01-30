use crate::{
    pool::Pool,
    random::Random,
    utils::args_parser::{Args, Distribution, Policy, SizeMode, TTLMode, Trend},
};

pub struct Actions<'a> {
    cur_block_size: usize,
    args: &'a Args,
    rng: Random,
    pool: Pool,
}

impl<'a> Actions<'a> {
    pub fn new(args: &'a Args, rng: Random, pool: Pool) -> Self {
        let cur_block_size: usize;
        match args.size_trend.unwrap_enum::<Trend>() {
            Trend::None => cur_block_size = 0usize,
            Trend::Grow => cur_block_size = args.min_size.unwrap_size(),
            Trend::Shrink => cur_block_size = args.max_size.unwrap_size(),
            Trend::Saw => cur_block_size = args.min_size.unwrap_size(),
        }
        Self {
            cur_block_size,
            args,
            rng,
            pool,
        }
    }

    pub fn block_action(&mut self) {
        if !matches!(self.args.ttl_mode.unwrap_enum::<TTLMode>(), TTLMode::Off) {
            self.pool.update_and_prune();
        }

        let should_alloc = self.should_alloc();

        if !should_alloc {
            self.pool
                .del_block(self.args.policy.unwrap_enum::<Policy>(), &mut self.rng);
        } else {
            let block_size = self.get_block_size();
            let block_ttl = self.get_block_ttl();
            self.pool.add_block_with_ttl(block_size, block_ttl);
        }
    }
}

impl<'a> Actions<'a> {
    fn trend_block_size(&mut self) -> usize {
        let jitter: isize =
            self.rng
                .uniform(0, self.args.trend_jitter.unwrap_size() * 2) as isize
                - self.args.trend_jitter.unwrap_size() as isize;

        match self.args.size_trend.unwrap_enum::<Trend>() {
            Trend::None => self.rng.next(
                self.args.min_size.unwrap_size(),
                self.args.max_size.unwrap_size(),
                self.args.dist_param.unwrap_float(),
                self.args.distribution.unwrap_enum::<Distribution>(),
            ),
            Trend::Grow => {
                let tmp = self.cur_block_size;
                let mut step = self.args.size_step.unwrap_size() as isize + jitter;
                if step <= 0 {
                    step = 1;
                }

                self.cur_block_size += step as usize;
                self.cur_block_size = self.cur_block_size.clamp(
                    self.args.min_size.unwrap_size(),
                    self.args.max_size.unwrap_size(),
                );
                tmp
            }
            Trend::Shrink => {
                let tmp = self.cur_block_size;
                let mut step = self.args.size_step.unwrap_size() as isize + jitter;
                if step <= 0 {
                    step = 1;
                }

                if step as usize > self.cur_block_size {
                    self.cur_block_size = self.args.min_size.unwrap_size();
                } else {
                    self.cur_block_size -= step as usize;
                }

                self.cur_block_size = self.cur_block_size.clamp(
                    self.args.min_size.unwrap_size(),
                    self.args.max_size.unwrap_size(),
                );
                tmp
            }
            Trend::Saw => {
                let tmp = self.cur_block_size;
                let mut step = self.args.size_step.unwrap_size() as isize + jitter;
                if step <= 0 {
                    step = 1;
                }

                self.cur_block_size += step as usize;
                if self.cur_block_size > self.args.max_size.unwrap_size() {
                    self.cur_block_size = self.args.min_size.unwrap_size();
                }
                tmp
            }
        }
    }

    fn get_closest(list: &Vec<usize>, size: usize) -> usize {
        if list.len() == 0 {
            return size;
        }

        list.iter()
            .map(|n| (*n, (size as isize - *n as isize).abs() as usize))
            .min_by(|n1, n2| n1.1.cmp(&n2.1))
            .expect("List has at least one element")
            .0
    }

    fn get_block_size(&mut self) -> usize {
        let l = self.args.size_list.unwrap_sizelist();
        if l.len() == 0 {
            return self.trend_block_size();
        }

        match self.args.size_mode.unwrap_enum::<SizeMode>() {
            SizeMode::Exact => self.rng.wchoice(l, self.args.size_weights.unwrap_intlist()),
            SizeMode::Nearest => Self::get_closest(l, self.trend_block_size()),
        }
    }

    fn get_block_ttl(&mut self) -> isize {
        match self.args.ttl_mode.unwrap_enum::<TTLMode>() {
            TTLMode::Off => -1,
            TTLMode::Fixed => self.args.ttl_fixed.unwrap_int() as isize,
            TTLMode::List => self.rng.wchoice(
                self.args.ttl_list.unwrap_intlist(),
                self.args.ttl_weights.unwrap_intlist(),
            ) as isize,
        }
    }

    #[inline(always)]
    fn should_alloc(&mut self) -> bool {
        (self.pool.count() < self.pool.capacity)
            && (self.rng.uniform01() < self.args.alloc_freq.unwrap_float())
    }
}
