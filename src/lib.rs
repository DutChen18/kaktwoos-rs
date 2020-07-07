mod rng;
mod kaktwoos;

use kaktwoos::*;

use std::os::raw::*;
use std::sync::atomic::*;
use std::sync::*;
use std::slice;
use std::ptr;
use std::thread;

static G_C4: AtomicU64 = AtomicU64::new(0);
static G_C5: AtomicU64 = AtomicU64::new(0);
static G_CH: AtomicU8 = AtomicU8::new(GROUND);
static G_N1: AtomicIsize = AtomicIsize::new(-1);
static G_N2: AtomicIsize = AtomicIsize::new(-1);
static G_ND: AtomicIsize = AtomicIsize::new(-1);

fn run(begin: u64, end: u64, mutex: &Mutex<Vec<u64>>) {
	let c4 = G_C4.load(Ordering::Relaxed);
	let c5 = G_C5.load(Ordering::Relaxed);
	let ch = G_CH.load(Ordering::Relaxed);
	let n1 = G_N1.load(Ordering::Relaxed);
	let n2 = G_N2.load(Ordering::Relaxed);
	let nd = G_ND.load(Ordering::Relaxed);
	for seed in begin..end {
		let seed = (seed << 4) | c4;
		let result = kaktwoos(seed, ch, n1, n2, nd, c5);
		if result.0 >= TARGET {
			let mut guard = mutex.lock().unwrap();
			let vec = &mut *guard;
			let packed = seed | ((result.1 as u64) << 48) | ((result.0 as u64) << 58);
			vec.push(packed);
		}
	}
}

#[no_mangle]
pub extern "C" fn k2_params(cs: c_ulong, nn: *const c_int, di: c_int, ch: c_int) {
	let neighbors = unsafe { slice::from_raw_parts(nn, 3) };
	G_C4.store(cs & 15, Ordering::Relaxed);
	G_C5.store(cs & 16, Ordering::Relaxed);
	G_CH.store(ch as u8 + GROUND, Ordering::Relaxed);
	G_N1.store(if di == 0 { neighbors[2] } else { neighbors[0] } as isize, Ordering::Relaxed);
	G_N2.store(if di == 1 { neighbors[2] } else { neighbors[1] } as isize, Ordering::Relaxed);
	G_ND.store(neighbors[di as usize] as isize, Ordering::Relaxed);
}

#[no_mangle]
pub extern "C" fn k2_start(threads: c_int, begin: c_ulong, end: c_ulong, out: *mut c_ulong) -> c_ulong {
	let arc: Arc<Mutex<Vec<u64>>> = Arc::new(Mutex::new(Vec::new()));
	if threads == 1 {
		run(begin, end, &arc);
	} else {
		let mut thrs = Vec::new();
		let work_size = (end - begin + (threads - 1) as u64) / threads as u64;
		for i in 0..threads {
			let mutex = arc.clone();
			thrs.push(thread::spawn(move || {
				let thread_begin = begin + i as u64 * work_size;
				let thread_end = std::cmp::min(thread_begin + work_size, end);
				run(thread_begin, thread_end, &mutex);
			}));
		}
		for thr in thrs {
			thr.join().unwrap();
		}
	}
	let vec = Arc::try_unwrap(arc).unwrap().into_inner().unwrap();
	unsafe { ptr::copy(vec.as_ptr(), out, vec.len()); }
	vec.len() as c_ulong
}