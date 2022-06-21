#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "buffer.h"

#define TRUE 1

sem_t empty, full;
pthread_mutex_t mutex;

// producer thread
void *producer(void *param)
{
    buffer_item item;
    int index = *(int *)param;
    while (TRUE)
    {
        /* sleep for a random period of time */
        sleep(rand() % 3);

        /* generate a random number */
        item = rand();

        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        /* critical section */
        if (insert_item(item))
        {
            fprintf(stderr, "buffer is full, producer %d 's insertion failed!\n", index);
            exit(1);
        }
        else
            printf("producer %d produced %d \n", index, item);
        /* critical section */
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
    }
    pthread_exit(0);
}

// consumer thread
void *consumer(void *param)
{
    buffer_item item;
    int index = *(int *)param;
    while (TRUE)
    {
        /* sleep for a random period of time */
        sleep(rand() % 3);

        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        /* critical section */
        if (remove_item(&item))
        {
            fprintf(stderr, "buffer is empty, consumer %d 's deletion failed!\n", index);
            exit(1);
        }
        else
            printf("consumer %d consumed %d \n", index, item);
        /* critical section */
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Error: invalid arguments.\n");
        return 1;
    }
    int time = atoi(argv[1]);
    int num_of_producers = atoi(argv[2]);
    int num_of_consumers = atoi(argv[3]);

    // buffer initialization
    buffer_init();

    // mutex and semaphore initialization
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    pthread_t *producers = (pthread_t *)malloc(sizeof(pthread_t) * num_of_producers);
    pthread_t *consumers = (pthread_t *)malloc(sizeof(pthread_t) * num_of_consumers);
    for (int i = 0; i < num_of_producers; ++i)
    {
        int producer_index = i;
        pthread_create(&producers[i], NULL, &producer, &producer_index);
    }
    for (int i = 0; i < num_of_consumers; ++i)
    {
        int consumer_index = i;
        pthread_create(&consumers[i], NULL, &consumer, &consumer_index);
    }

    //sleep before termination
    sleep(time);

    // termination
    for (int i = 0; i < num_of_producers; ++i)
        pthread_cancel(producers[i]);
    for (int i = 0; i < num_of_consumers; ++i)
        pthread_cancel(consumers[i]);

    // destroy mutex and semaphore
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    free(producers);
    free(consumers);
    return 0;
}