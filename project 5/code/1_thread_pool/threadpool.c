/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include "threadpool.h"

#define TRUE 1

// this represents work that has to be
// completed by a thread in the pool
typedef struct
{
    void (*function)(void *p);
    void *data;
} task;

// the work queue
struct task_node
{
    task tsk;
    struct task_node *next;
};
struct task_node *head, *tail;

pthread_mutex_t mutex;
sem_t *sem;

int num_of_threads;
int *thread_index;
int shutdown;

// the worker bee
pthread_t *bee;

// insert a task into the queue
// returns 0 if successful or 1 otherwise,
int enqueue(task t)
{
    struct task_node *tmp = (struct task_node *)malloc(sizeof(struct task_node));
    if (tmp == NULL)
    {
        printf("malloc error and enqueue failed!\n");
        return 1;
    }

    tmp->tsk = t;
    tmp->next = NULL;
    if (tail == NULL) // first task
    {
        head = tmp;
        tail = tmp;
    }
    else // others
    {
        tail->next = tmp;
        tail = tail->next;
    }
    // printf("enqueue seccessful\n");
    return 0;
}

// remove a task from the queue
task dequeue()
{
    if (head == NULL)
    {
        printf("waiting queue empty and dequeue failed!\n");
        return;
    }
    task worktodo = head->tsk;
    struct task_node *tmp = head;
    head = head->next;
    if (head == NULL)
        tail = NULL;
    free(tmp);
    // printf("dequeue seccessful\n");
    return worktodo;
}

// the worker thread in the thread pool
void *worker(void *param)
{
    while (TRUE)
    {
        sem_wait(sem); // decrease the semaphore
        if (shutdown)
            break;

        pthread_mutex_lock(&mutex);
        task worktodo = dequeue();
        pthread_mutex_unlock(&mutex);

        // execute the task
        printf("In thread %d:   ", *((int *)param));
        execute(worktodo.function, worktodo.data);
    }

    pthread_exit(0);
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *p), void *p)
{
    task worktodo;
    worktodo.function = somefunction;
    worktodo.data = p;

    int res;
    pthread_mutex_lock(&mutex);
    res = enqueue(worktodo);
    pthread_mutex_unlock(&mutex);

    if (res == 1)
    {
        printf("task submission to pool failed!\n");
        return 1;
    }
    else
    {
        sem_post(sem); // increase the semaphore
        return 0;
    }
}

// initialize the thread pool
void pool_init(int num)
{
    num_of_threads = num;
    bee = (pthread_t *)malloc(sizeof(pthread_t) * num_of_threads);
    thread_index = (int *)malloc(sizeof(int) * num_of_threads);

    head = tail = NULL;
    shutdown = 0;

    // initialize mutual-exclusion locks and semaphores
    int ret = pthread_mutex_init(&mutex, NULL);
    if (ret)
    {
        printf("mutex initialization failed!\n");
        return;
    }
    sem = sem_open("QUEUE_SEM", O_CREAT, 0666, 0);
    if (sem == SEM_FAILED)
    {
        printf("semaphore initialization failed!\n");
        return;
    }

    // create the threads
    for (int i = 0; i < num_of_threads; i++)
    {
        thread_index[i] = i;
        ret = pthread_create(&bee[i], NULL, worker, &thread_index[i]);
    }
}

// shutdown the thread pool
void pool_shutdown(void)
{
    shutdown = 1;

    // release sem so that threads can exit
    for (int i = 0; i < num_of_threads; i++)
    {
        sem_post(sem);
    }

    //  join the threads
    for (int i = 0; i < num_of_threads; i++)
    {
        pthread_join(bee[i], NULL);
    }

    // destroy mutex and semaphore
    pthread_mutex_destroy(&mutex);
    sem_destroy(sem);

    free(bee);
    free(thread_index);
}
