/**
 * Example client program that uses thread pool.
 */

#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"

struct data
{
    int a;
    int b;
};

void add(void *param)
{
    struct data *temp;
    temp = (struct data *)param;

    printf("I add two values %d and %d result = %d\n", temp->a, temp->b, temp->a + temp->b);
}

int main(void)
{
    // create some work to do
    struct data work[30];
    for (int i = 0; i < 30; i++)
    {
        work[i].a = i;
        work[i].b = 30 - i;
    }
    
    int num;
    printf("please input the number of threads:     ");
    scanf("%d",&num);
    // initialize the thread pool
    pool_init(num);
    
    // submit the work to the queue
    for (int i = 0; i < 30; i++)
    {
        pool_submit(&add, &work[i]);
    }

    // may be helpful
    sleep(1);

    pool_shutdown();

    return 0;
}
