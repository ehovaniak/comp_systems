
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

#include "job.h"
#include "queue.h"
#include "factor.h"

static queue* iqueue;
static queue* oqueue;

static int worker_count = 0;
static pid_t workers[64];
static pid_t printer;

void
work_off_jobs()
{
    while (1) {
        job jj = queue_get(iqueue);
        if (jj.number < 0) {
            break;
        }

        factor(jj.number, &(jj.count), &(jj.factors[0]));

        queue_put(oqueue, jj);
    }
}

void
print_results(int64_t count)
{
    int64_t oks = 0;

    for (int64_t ii = 0; ii < count; ++ii) {
        job res = get_result();

        printf("%ld: ", res.number);
        int64_t prod = 1;
        for (int64_t jj = 0; jj < res.count; ++jj) {
            int64_t xx = res.factors[jj];
            prod *= xx;
            printf("%ld ", xx);
        }
        printf("\n");

        if (prod == res.number) {
            ++oks;
        }
        else {
            fprintf(stderr, "Warning: bad factorization");
        }
    }

    printf("Factored %ld / %ld numbers.\n", oks, count);
}

void
factor_wait_done()
{
    // FIXME: Why is this here?
	// wait for all processes to complete their work and release
	// their resources
	for (int i = 0; i < worker_count; i++)
		waitpid(workers[i], 0, 0);
	waitpid(printer, 0, 0);

	return;
}

void
factor_init(int num_procs, int64_t count)
{
	// create input and output queues
    if (iqueue == 0) iqueue = make_queue();
    if (oqueue == 0) oqueue = make_queue();

    // FIXME: Spawn N worker prows and a printing proc.
    worker_count = num_procs;
    for (int i = 0; i < num_procs; i++) {
    	// spawn a process and keep track of its id
    	workers[i] = fork();
    	if (workers[i] == 0) {
    		//worker_count++;
    		// begin factoring and exit upon completion
    		work_off_jobs();
    		exit(0);
    	}
    }

    // spawn an extra process for printing output to console
    printer = fork();
    if (printer == 0) {
    	print_results(count);
    	exit(0);
    }

    return;
}

void
factor_cleanup()
{
    job done = make_job(-1);

    for (int ii = 0; ii < worker_count; ++ii) {
        submit_job(done);
    }

    // FIXME: Make sure all the workers are done.
    factor_wait_done();

    // free the queue resources
    free_queue(iqueue);
    iqueue = 0;
    free_queue(oqueue);
    oqueue = 0;
}

void
submit_job(job jj)
{
    queue_put(iqueue, jj);
}

job
get_result()
{
    return queue_get(oqueue);
}

static
int64_t
isqrt(int64_t xx)
{
    double yy = ceil(sqrt((double)xx));
    return (int64_t) yy;
}

void
factor(int64_t xx, int64_t* size, int64_t* ys)
{
    int jj = 0;

    while (xx % 2 == 0) {
        ys[jj++] = 2;
        xx /= 2;
    }

    for (int64_t ii = 3; ii <= isqrt(xx); ii += 2) {
        int64_t x1 = xx / ii;
        if (x1 * ii == xx) {
            ys[jj++] = ii;
            xx = x1;
            ii = 1;
        }
    }

    ys[jj++] = xx;
    *size = jj;
}

