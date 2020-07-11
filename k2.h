#ifndef K2_H
#define K2_H

typedef unsigned long long ull;
extern "C" void k2_params(ull chunkseed, const int *neighbors, int diagonal, int height);
extern "C" ull k2_start(int threads, ull begin, ull end, ull *out);

#ifndef THREAD_WORK_SIZE
#define THREAD_WORK_SIZE 2000000UL
#endif

inline ull k2_start_block(int threads, ull *offset, ull task_end, ull *out) {
	ull end = *offset + THREAD_WORK_SIZE * threads;
	if (end > task_end) end = task_end;
	ull result = k2_start(threads, *offset, end, out);
	*offset = end;
	return result;
}

#endif