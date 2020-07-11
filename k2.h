#ifndef K2_H
#define K2_H

extern "C" void k2_params(unsigned long chunkseed, const int *neighbors, int diagonal, int height);
extern "C" unsigned long k2_start(int threads, unsigned long begin, unsigned long end, unsigned long *out);

#endif