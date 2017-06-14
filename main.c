
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#include "int128.h"
#include "factor.h"
#include "ivec.h"



int
main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage:\n");
        printf("  ./main threads start count\n");
        return 1;
    }

    const int num_threads = atoi(argv[1]);
    //assert(threads == 1);

    const int128_t start = atoh(argv[2]);
    const int64_t  count = atol(argv[3]);

    pthread_t threads_list[num_threads];

    // FIXME: Maybe we're spawning threads in init?
    factor_init();

    for (int64_t i = 0; i < num_threads; i++) {
    	int err = pthread_create(&threads_list[i], 0, run_jobs, NULL);
    	if (err) {
			printf("Error! 'pthread_create' returned code %d!\n", err);
			factor_cleanup();
			return 0;
		}
    }

    for (int64_t ii = 0; ii < count; ++ii) {
    	//printf("entering jobs\n");
        factor_job* job = make_job(start + ii);
        submit_job(job);
    }
    close_iqueue();

    // FIXME: This should be (threads) seperate threads.
    //run_jobs();

    int64_t oks = 0;

    // FIXME: This should probably be while ((job = get_result()))
    for (int64_t ii = 0; ii < count; ++ii) {
        factor_job* job = get_result();

        print_int128(job->number);
        printf(": ");
        print_ivec(job->factors);

        ivec* ys = job->factors;
        
        int128_t prod = 1;
        for (int ii = 0; ii < ys->len; ++ii) {
            prod *= ys->data[ii];
        }

        if (prod == job->number) {
            ++oks;
        }
        else {
            fprintf(stderr, "Warning: bad factorization");
        }

        free_job(job);
    }
    close_oqueue();
    printf("Factored %ld / %ld numbers.\n", oks, count);

    for (int64_t i = 0; i < num_threads; i++) {
    	int err = pthread_join(threads_list[i], NULL);
    	if (err) {
    		printf("Error! 'pthread_join' returned code %d!\n", err);
    		factor_cleanup();
    		return 0;
    	}
    }

    factor_cleanup();
    //for (int64_t i = 0; i < num_threads; i++)
    //	pthread_join(threads_list[i], NULL);
    // FIXME: We should have joined all spawned threads.

    return 0;
}
