use crate::rng::*;

pub const GROUND: u8 = 63;
pub const TARGET: u8 = 17;

pub fn kaktwoos(seed: u64, height: u8, n1: isize, n2: isize, nd: isize, bit: u64) -> (u8, isize) {
	let mut rng = Rng::new(seed);
	let mut map = [GROUND; 1024];
	let mut pos = -1isize;
	let mut top = height;
	'outer: for i in 0..10 {
		if GROUND + TARGET > 9 * (10 - i) + top { break 'outer; }
		let ix = rng.next_int(16) + 8;
		let iz = rng.next_int(16) + 8;
		let ip = (iz * 32 + ix) as usize;
		let iy = rng.next_int((map[ip] as i32 + 1) * 2);
		if iy + 3 <= GROUND as i32 && iy - 3 >= 0 {
			rng.advance(60);
			continue;
		}
		if iy - 3 > top as i32 + 1 {
			for _ in 0..10 {
				rng.advance(6);
				let h = rng.next_int(3) + 1;
				rng.next_int(h);
			}
			continue;
		}
		for _ in 0..10 {
			let x = ix + rng.next_int(8) - rng.next_int(8);
			let y = iy + rng.next_int(4) - rng.next_int(4);
			let z = iz + rng.next_int(8) - rng.next_int(8);
			let p = (z * 32 + x) as usize;
			if pos == -1 && y > GROUND as i32 && y <= height as i32 + 1 {
				if p == n1 as usize { pos = n1; }
				if p == n2 as usize { pos = n2; }
				if p == nd as usize { pos = nd; }
				if pos != -1 {
					if pos != nd && (seed ^ bit) & 16 == 0 { break 'outer; }
					if pos == nd && (seed ^ bit) & 16 != 0 { break 'outer; }
					map[p] = height;
				}
			}
			if y <= map[p] as i32 { continue; }
			let h = rng.next_int(3) + 1;
			let h = rng.next_int(h) + 1;
			if y != map[p] as i32 + 1 { continue; }
			if y == GROUND as i32 + 1 {
				let mut mask = 0;
				if x != 0x00 { mask |= map[p - 0x01] ^ GROUND; }
				if x != 0x1F { mask |= map[p + 0x01] ^ GROUND; }
				if z != 0x00 { mask |= map[p - 0x20] ^ GROUND; }
				if z != 0x1F { mask |= map[p + 0x20] ^ GROUND; }
				if mask != 0 { continue; }
			}
			map[p] += h as u8;
			if map[p] > top {
				top = map[p]
			}
		}
	}
	(top - GROUND, pos)
}

#[test]
fn test_kaktwoos() {
	assert_eq!(kaktwoos(77849775653u64, 75, 344, 840, 856, 16).0, 20);
}