#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "job.h"



/*
 *
 * constructor for job que
 *
 */
struct que_t* que_init(void)
{
	struct que_t *q = malloc(sizeof *q);
	if (q == NULL)
		return NULL;

	q->head = NULL;
	q->tail = NULL;
	q->size = 0;

	pthread_mutex_init(&q->lock, NULL); /* default options */

	return q;
}




/*
 * functions for adding, removing, and getting
 * referrences to jobs
 */

struct job_t* que_push(struct que_t *q, struct job_t *j)
{
	assert(j != NULL);
	struct entry_t *e = malloc(sizeof *e);
	if (e == NULL) {
		perror("Memory allocation failure when creating list entry\n");
	}

	e->job = *j;
	e->next = NULL;

	pthread_mutex_lock(&q->lock);

	if (q->head == NULL) {       /* if first entry */
		q->head = e;
		q->tail = e;
	} else {
		q->tail->next = e;   /* append to tail */
		q->tail = e;
	}

	++q->size;

	pthread_mutex_unlock(&q->lock);
	return j;
}




struct job_t* que_pop(struct que_t *q, struct job_t *j)
{
	pthread_mutex_lock(&q->lock);
	assert(q->head != NULL);


	struct entry_t *top = q->head;
	if (j != NULL) {
		*j = top->job;   /* save the job in j */
	}

	if (q->head == q->tail) { /* if only one element */
		q->head = q->tail = NULL;
	} else {
		q->head = q->head->next;
	}

	free(top);

	pthread_mutex_unlock(&q->lock);
	return j;
}




struct job_t* que_top(struct que_t *q)
{
	pthread_mutex_lock(&q->lock);

	assert(q->head != NULL);
	struct job_t *top = &(q->head->job);

	pthread_mutex_unlock(&q->lock);
	return top;
}




/*
 *
 * functions for executing jobs
 *
 */

void* do_job(struct que_t *q)
{
	void *ret;
	struct job_t j;

	que_pop(q, &j);   /* REMOVES JOB, but saves the job into j */

	ret = (*j.function)(j.args);   /* call to the job's function */
	return ret;
}


pthread_t* start_job_thread(struct que_t *q, pthread_t *thread)
{
	struct job_t j;
	que_pop(q, &j);  /* remove job, save in j */

	pthread_create(thread, NULL, (j.function), (j.args));
	return thread;
}


/* walk the que, doing all jobs in order */
void flush_que(struct que_t *q)
{
	assert(q->head != NULL);
	while (q->head != NULL) {
		do_job(q);    /* calles pop, (removes entry and performs job) */
	}
}




/*
 *
 * miscelaneous functions
 *
 */

int que_is_empty(const struct que_t *q)
{
	if (q->head == NULL) {
		return 1;
	}

	return 0;
}




unsigned int que_size(const struct que_t *q)
{
	return q->size;
}






/*
 * destructor / cleanup
 */

/* does not destroy jobs, only entries and the queue */
void que_destroy(struct que_t *q)
{
	pthread_mutex_lock(&q->lock);

	/* free the entries */
	struct entry_t *current = q->head;
	while (current != NULL) {
		struct entry_t *next = current->next;
		free(current);
		current = next;
	}

	/* list object cleanup */
	pthread_mutex_unlock(&q->lock);
	pthread_mutex_destroy(&q->lock);

	free(q);
}




