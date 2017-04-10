#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "job.h"

/*
 * a small implementation of a job queue in c
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


	/* now, push the jobs onto the que stack */
	que_push(q, &j1);
	que_push(q, &j2);
	que_push(q, &j3);

	putchar('\n');
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


	/* example of using do_job */

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


	/* example of using pthread to process jobs concurrently */

	int njob = 3;
	pthread_t threads[njob];

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

	j1 = newJob(sum, a);
	j2 = newJob(sum, b);
	j3 = newJob(sum, c);

	que_push(q, &j1);
	que_push(q, &j2);
	que_push(q, &j3);

	while (q->head != NULL) {
		void *ret = do_job(q);
		printf("sum = %g\n", *(double*)ret);
		free(ret);
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
	printf("Hello, %s\n", (char *)arg);
	return NULL;
}


void* print_num(void *arg)
{
	printf("print_num = %d\n", *(int*)arg);
	return NULL;
}


void* mult2(void *arg)
{
	int *ret = (int *)arg;

	*ret *= 2;

	return (void*)ret;
}



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

