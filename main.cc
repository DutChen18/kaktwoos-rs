#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <cstring>
#include <inttypes.h>
#include "k2.h"

#ifdef _WIN64
#include "boinc_win.h"
#else
#ifdef _WIN32
#include "boinc_win.h"
#endif
#endif

#include "boinc_api.h"

#define MAX_SEED_BUFFER_SIZE (0x10000)

double time()
{
	using namespace std::chrono;
	auto now = high_resolution_clock::now().time_since_epoch();
	return (double)duration_cast<milliseconds>(now).count() / 1000;
}

int main(int argc, char **argv)
{

	BOINC_OPTIONS options;

	boinc_options_defaults(options);
	options.normal_thread_priority = true;
	boinc_init_options(&options);

	int threads = 0;
	unsigned long long start = 0;
	unsigned long long end = 0;
	unsigned long long chunkSeed = 0;
	int neighbor1 = 0;
	int neighbor2 = 0;
	int neighbor3 = 0;
	int diagonalIndex = 0;
	int height = 0;

	struct checkpoint_vars
	{
		unsigned long long offset;
		unsigned long long start;
		int block;
		double elapsed_chkpoint;
		int total_seed_count;
	};

	if (argc % 2 != 1)
	{
		fprintf(stderr, "Failed to parse arguments\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 1; i < argc; i += 2)
	{
		const char *param = argv[i];
		if (strcmp(param, "-t") == 0 || strcmp(param, "--threads") == 0)
		{
			threads = atoi(argv[i + 1]);
		}
		else if (strcmp(param, "-s") == 0 || strcmp(param, "--start") == 0)
		{
			sscanf(argv[i + 1], "%llu", &start);
		}
		else if (strcmp(param, "-e") == 0 || strcmp(param, "--end") == 0)
		{
			sscanf(argv[i + 1], "%llu", &end);
		}
		else if (strcmp(param, "-cs") == 0 || strcmp(param, "--chunkseed") == 0)
		{
			sscanf(argv[i + 1], "%llu", &chunkSeed);
		}
		else if (strcmp(param, "-n1") == 0 || strcmp(param, "--neighbor1") == 0)
		{
			neighbor1 = atoi(argv[i + 1]);
		}
		else if (strcmp(param, "-n2") == 0 || strcmp(param, "--neighbor2") == 0)
		{
			neighbor2 = atoi(argv[i + 1]);
		}
		else if (strcmp(param, "-n3") == 0 || strcmp(param, "--neighbor3") == 0)
		{
			neighbor3 = atoi(argv[i + 1]);
		}
		else if (strcmp(param, "-di") == 0 || strcmp(param, "--diagonalindex") == 0)
		{
			diagonalIndex = atoi(argv[i + 1]);
		}
		else if (strcmp(param, "-ch") == 0 || strcmp(param, "--cactusheight") == 0)
		{
			height = atoi(argv[i + 1]);
		}
		else
		{
			fprintf(stderr, "Unknown parameter: %s\n", param);
		}
	}

	fprintf(stderr, "Received work unit: %llu\n", chunkSeed);
	fprintf(stderr, "Data: n1: %d, n2: %d, n3: %d, di: %d, ch: %d\n",
			neighbor1,
			neighbor2,
			neighbor3,
			diagonalIndex,
			height);

	static unsigned long long found_seeds[MAX_SEED_BUFFER_SIZE];
	static int neighbors[] = {neighbor1, neighbor2, neighbor3};

	k2_params(chunkSeed, neighbors, diagonalIndex, height);

	unsigned long long total = end - start;
	unsigned long long total_seed_count = 0;
	unsigned long long offset = start;
	unsigned long long block = 0;
	int progcheck = 0;

	FILE *checkpoint_data = boinc_fopen("kaktpoint.txt", "rb");
	unsigned long long elapsed_chkpoint = 0;

	if (!checkpoint_data)
	{
		fprintf(stderr, "No checkpoint to load\n");
	}
	else
	{

		boinc_begin_critical_section();
		struct checkpoint_vars data_store;

		fread(&data_store, sizeof(data_store), 1, checkpoint_data);
		offset = data_store.offset;
		start = data_store.start;
		block = data_store.block;
		elapsed_chkpoint = data_store.elapsed_chkpoint;
		total_seed_count = data_store.total_seed_count;

		fread(found_seeds, sizeof(cl_ulong), total_seed_count, checkpoint_data);

		fprintf(stderr, "Checkpoint loaded, task time %llu s \n", elapsed_chkpoint);
		fclose(checkpoint_data);
		boinc_end_critical_section();
	}

	double start_time = time();
	for (; offset < end;)
	{
		unsigned long long count = k2_start_block(threads, &offset, end, found_seeds + total_seed_count);
		progcheck++;
		block++;

		for (unsigned long long j = 0; j < count; j++)
		{
            fprintf(stderr,"    Found seed: %llu, %llu, height: %d\n",
                    found_seeds[total_seed_count],
                    found_seeds[total_seed_count] & ((1ULL << 48ULL) - 1ULL),
                    (int)(found_seeds[total_seed_count] >> 58ULL));

            fprintf(stderr, "%llu\n", found_seeds[total_seed_count]);
			total_seed_count++;
		}

		if (progcheck > 125 || boinc_time_to_checkpoint())
		{ // 200 for 0.2bil seeds before checkpoint
			boinc_begin_critical_section(); // Boinc should not interrupt this
			double frac = (double(offset - start) / (total));
			boinc_fraction_done(frac);

			progcheck = 0;

			boinc_delete_file("kaktpoint.txt");
			FILE *checkpoint_data = boinc_fopen("kaktpoint.txt", "wb");

			struct checkpoint_vars data_store;
			data_store.offset = offset;
			data_store.start = start;
			data_store.block = block;
			data_store.elapsed_chkpoint = (elapsed_chkpoint + (double)(time() - start_time));
			data_store.total_seed_count = total_seed_count;

			fwrite(&data_store, sizeof(data_store), 1, checkpoint_data);
			fwrite(found_seeds, sizeof(cl_ulong), total_seed_count, checkpoint_data);

			fclose(checkpoint_data);

			double fraction_done = ((offset - start) / (total));
			boinc_fraction_done(fraction_done);

			boinc_end_critical_section();
			boinc_checkpoint_completed(); // Checkpointing completed
		}
	}

	fflush(stderr);
	double end_time = time();
	double elapsed = elapsed_chkpoint + end_time - start_time;
	double per_sec = total / elapsed;
	fprintf(stderr, "\nSpeed (%.2lfm/s * %i t): %.2lfm/s\n", ((per_sec / 1000000.0) / threads), threads, per_sec / 1000000.0 );
	fprintf(stderr, "Done\n");
	fprintf(stderr, "Processed: %llu seeds in %.2lfs seconds\n", total, elapsed);
	
    fprintf(stderr, "Found seeds: \n");

    for (unsigned long i = 0; i < total_seed_count; i++) {
        fprintf(stderr, "    %llu\n", found_seeds[i]);
    }
	fflush(stderr);
	boinc_finish(0);
}
