#ifndef K2_H
#define K2_H

extern "C" void k2_params(unsigned long long chunkseed, const int *neighbors, int diagonal, int height);
extern "C" unsigned long long k2_start(int threads, unsigned long long begin, unsigned long long end, unsigned long long *out);

#endif