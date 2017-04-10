#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "job.h"

/*
 * a small implementation of a job queue in c
 *
 *
 * the first part shows how to add jobs, and flush
 * the job queue.
 *
 * next, how to execute jobs in order, saving their
 * return values
 *
 * then, running each job on its own thread,
 * again, saving the return value of the job
 *
 *
 */



/* functions to be executed by each job object */
void* say_something(void *arg);

void* print_num(void *arg);

void* mult2(void *arg);

void* sum(void *arg); /* sum an array */

int main()
{
	/* initialize a new queue */
	struct que_t *q = que_init();


	/* first, create some jobs
	 * the job constructor takes a function pointer with the signature:
	 *
	 * (void*) (*) (void*)
	 *
	 * and and argument of type: void*
	 *
	 * the newJob constructor is a pre-processor macro
	 * the argument does not have to be explicitly cast to
	 * (void*). (notice that msg1, msg2 ... are not cast).
	 */

	char *msg1 = "from job one!";
	char *msg2 = "from job two!";
	char *msg3 = "from job three!";


	struct job_t j1 = newJob(say_something, msg1);
	struct job_t j2 = newJob(say_something, msg2);
	struct job_t j3 = newJob(say_something, msg3);


	/* now, push the jobs onto the que */
	que_push(q, &j1);
	que_push(q, &j2);
	que_push(q, &j3);

	putchar('\n');

	/*
	 * executing the jobs
	 *
	 */
	flush_que(q);  /* flush_que will execute all of the jobs
			  ignoring their return values.

			  to keep return values, use `do_job` in a loop
			  (see below) */


	char *msg4 = "World!";
	j1 = newJob(say_something, msg4);    /* job variables can be reused */
	que_push(q, &j1);

	int num = 5;
	que_push(q, &newJob(print_num, &num));  /* jobs can be initialized in-place like so */

	flush_que(q);



	/*
	 *
	 * example of using do_job, no que_flush
	 * (executing jobs and getting the return values)
	 *
	 * task : pass number as input, double it in-place
	 */

	int x = 5;
	int y = 10;
	j1 = newJob(mult2, &x);  /* multiply x and y by 2 */
	j2 = newJob(mult2, &y);

	que_push(q, &j1);
	que_push(q, &j2);

	printf("\n\nBefore executing jobs ...\n");
	printf("x = %d, y = %d\n\n", x, y);

	printf("return values from jobs ...\n");
	while (q->head != NULL) {
		/* do job performs a job, and removes
		 * it from the queue */

		void *ret = do_job(q);
		printf("Result = %d\n", *(int*)ret);
	}

	printf("\nAfterwards ... \n");
	printf("x = %d, y = %d\n", x, y);



	/*
	 *
	 * example of using pthread to process jobs concurrently
	 *
	 *
	 * task : take the sum of three arrays
	 */

	double a[10];
	double b[10];
	double c[10];

	/* generating dummy data */
	int i;
	for (i = 0; i < 10; ++i) {
		a[i] = i * 2.0 + 3.14 - (-12.0 * 0.5);
		b[i] = i * 200.0 + (i % 2) / 11.5;
		c[i] = i + i + i / 3.0;
	}

	/* creating the jobs */
	j1 = newJob(sum, a);
	j2 = newJob(sum, b);
	j3 = newJob(sum, c);


	/* add to the que */
	que_push(q, &j1);
	que_push(q, &j2);
	que_push(q, &j3);


	int njob = 3;
	pthread_t threads[njob];


	/* start each job on their own thread */
	/* to get the return value, use pthread_join (see below) */

	pthread_t *pt = (threads+0); /* pointer to first thread */

	while (q->head != NULL) {
		start_job_thread(q, pt);   /* saves pointer to thread in pt */
		++pt;
	}


	double results[njob];
	results[0] = 0;
	results[1] = 0;
	results[2] = 0;

	double *sum;
	for (i = 0; i < njob; ++i, ++sum) {
		pthread_join(threads[i], (void*)&sum);
		results[i] = *sum;
		printf("sum of array %d = %g\n", i, results[i]);
		free(sum); /* summation term malloc'd in this job
			      must be free'd each time */
	}




	/* clean up the queue, deleting any entries in the queue */
	que_destroy(q);

	putchar('\n');
	return 0;
}



/*
 *
 * functions to be executed by each job object
 *
 */

void* say_something(void *arg)
{
	assert(arg != NULL);
	printf("Hello, %s\n", (char *)arg);
	return NULL;
}



void* print_num(void *arg)
{
	assert(arg != NULL);
	printf("print_num = %d\n", *(int*)arg);
	return NULL;
}



/* multiply by two, saving inplace */
void* mult2(void *arg)
{
	assert(arg != NULL);
	int *ret = (int *)arg;

	*ret *= 2;

	return (void*)ret;
}




/* NOTE: the sum term in this function must be
 * free'd after each call.
 * see the pthread example in main
 */
void* sum(void *arg)
{
	double *array = (double *)arg;

	double *sum = malloc(sizeof *sum);
	*sum = 0;

	int i;
	for (i = 0; i < 10; ++i) {
		*sum += array[i];
	}

	return (void *)sum;
}

