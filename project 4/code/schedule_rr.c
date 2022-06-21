#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

struct node *head = NULL;
int tid = 0;
int clock = 0;

void add(char *name, int priority, int burst)
{
    Task *tsk = NULL;
    tsk = (Task *)malloc(sizeof(Task));
    tsk->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(tsk->name, name);
    tsk->priority = priority;
    tsk->burst = burst;
    tsk->tid = __sync_fetch_and_add(&tid, 1);
    tsk->been_executed = 0;
    tsk->init_burst = burst;
    tsk->arrive_time = clock;

    insert(&head, tsk);
}

void schedule()
{
    int total_turnaround = 0;
    int total_wait = 0;
    int total_responce = 0;
    int task_count = 0;
    while (head)
    {
        struct node *nodeptr = head;
        while (nodeptr->next) //找到最先进入队列的task
        {
            nodeptr = nodeptr->next;
        }
        Task *tsk = nodeptr->task;

        if (tsk->been_executed == 0) //检查是否首次被执行，更新response time
        {
            tsk->been_executed = 1;
            task_count++;
            tsk->responce_time = clock - tsk->arrive_time;
        }
        
        if (tsk->burst > QUANTUM)         //判断一个时间片是否可以完成该task
        {
            clock += QUANTUM;
            tsk->burst -= QUANTUM;
            run(tsk, QUANTUM);
            delete(&head, tsk);
            insert(&head, tsk);
            continue;
        }
        else
        {
            run(tsk, tsk->burst);
            clock += tsk->burst;

            tsk->finish_time = clock;
            delete (&head, tsk);

            int turnaround = tsk->finish_time - tsk->arrive_time; //每个task执行结束后，将时间累加到总时间
            total_turnaround += turnaround;
            total_wait += (turnaround - tsk->init_burst);
            total_responce += tsk->responce_time;

            free(tsk->name);
            free(tsk);
        }
    }

    // calculate the statistics
    double aver_turnaround = ((double)total_turnaround) / task_count;
    double aver_wait = ((double)total_wait) / task_count;
    double aver_responce = ((double)total_responce) / task_count;
    printf("\n");
    printf("For the total %d tasks:\n", task_count);
    printf("The Average Turnaround Time is:     %lf \n", aver_turnaround);
    printf("The Average Waiting Time is:     %lf \n", aver_wait);
    printf("The Average Responce Time is:     %lf \n", aver_responce);
}