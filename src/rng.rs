pub const MOD: u64 = 1u64 << 48;
pub const MUL: u64 = 25214903917u64;
pub const ADD: u64 = 11u64;

pub struct Rng {
	seed: u64,
}

impl Rng {
	pub fn new(seed: u64) -> Rng {
		Rng { seed }
	}

	pub fn advance(&mut self, count: u64) {
		let mut add = 0u64;
		let mut mul = 1u64;
		for _ in 0..count {
			add = (add.wrapping_mul(MUL).wrapping_add(ADD)) % MOD;
			mul = (mul.wrapping_mul(MUL)                  ) % MOD;
		}
		self.seed = (self.seed.wrapping_mul(mul).wrapping_add(add)) % MOD;
	}
	
	pub fn next(&mut self, bits: u8) -> i32 {
		self.advance(1);
		(self.seed >> (48 - bits)) as i32
	}

	pub fn next_int(&mut self, bound: i32) -> i32 {
		assert!(bound > 0);
		if bound & -bound == bound {
			return (self.next(31) as u64 * bound as u64 >> 31) as i32;
		}
		loop {
			let bits = self.next(31);
			let val = bits % bound;
			if bits - val + (bound - 1) >= 0 {
				return val;
			}
		}
	}
}