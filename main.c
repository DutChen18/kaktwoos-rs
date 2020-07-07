#include <stdio.h>
#include "k2.h"

#define WORK_SIZE 10000000UL

int main() {
	static unsigned long out[1024 * 1024];
	static int neighbors[] = { 856, 344, 840 };
	k2_params(9567961692053UL, neighbors, 0, 12);
	unsigned long count = 0;
	for (unsigned long i = 4500000000UL; i < 4600000000UL; i += WORK_SIZE) {
		count += k2_start(12, i, i + WORK_SIZE, out + count);
	}
	for (unsigned long i = 0; i < count; i++)
		printf("%lu\n", out[i]);
}