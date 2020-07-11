#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include "k2.h"

#define WORK_SIZE 10000000UL

double time()
{
	using namespace std::chrono;
	auto now = high_resolution_clock::now().time_since_epoch();
	return (double) duration_cast<milliseconds>(now).count() / 1000;
}

int main(int argc, char **argv)
{
	static unsigned long long out[1024 * 1024];
	static int neighbors[] = { 856, 344, 840 };
	k2_params(9567961692053UL, neighbors, 0, 12);
	if (argc < 2) {
		printf("usage: %s [threads]\n", argv[0]);
		return EXIT_FAILURE;
	}
	int threads = atoi(argv[1]);
	unsigned long long count = 0;
	unsigned long long start = 4500000000UL;
	unsigned long long total = 1500000000UL;
	double start_time = time();
	for (unsigned long long i = start; i < start + total; i += WORK_SIZE) {
		count += k2_start(threads, i, i + WORK_SIZE, out + count);
	}
	double end_time = time();
	double elapsed = end_time - start_time;
	double per_sec = total / elapsed;
	printf("%.2lfs %.2lfm/s\n", elapsed, per_sec / 1000000.0);
	return EXIT_SUCCESS;
}