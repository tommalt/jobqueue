#pragma once


/* job_t contains a function pointer and the argument */
struct job_t
{
	void* (*function) (void *args);
	void *args;
};


/* constructor for creating new Job */
#define newJob(function, args) ((struct job_t) {function, args})



/* NOTE: functions should be of the signature:
 * (void*) (*) (void*)
 *
 * ex)
 * void* my_function(void* arg) {
 *
 *	char *whatever = ... do somthing with arg ...
 *
 * 	return (void*)whatever;
 * }
 */



/* entry contains a job and pointer to next job; (it's a list node) */
struct entry_t
{
	struct job_t job;
	struct entry_t *next;
};



/* que is a doubly linked list of entries */
struct que_t
{
	struct entry_t *head;
	struct entry_t *tail;
	unsigned int size;
	pthread_mutex_t lock;
};




struct que_t* que_init(void);              /* initializes a que */

struct job_t* que_push(struct que_t *q, struct job_t *j);     /* adds job to que */

struct job_t* que_pop(struct que_t *q, struct job_t *j);      /* removes job from que */

struct job_t* que_top(struct que_t *q);        /* returns reference to head job */




void* do_job(struct que_t *q);          /* performs job, returning result */

void flush_que(struct que_t *q);        /* performs all jobs, discarding results */




/* misc stuff */
int que_is_empty(const struct que_t *q);         	/* check if empty */

unsigned int que_size(const struct que_t *q);    /* returns number of jobs */



/* cleanup */
/* de-allocates que and entries, but not jobs */
void que_destroy(struct que_t *q);


