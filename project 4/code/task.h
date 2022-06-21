/**
 * Representation of a task in the system.
 */

#ifndef TASK_H
#define TASK_H

// representation of a task
typedef struct task {
    char *name;
    int tid;
    int priority;
    int burst;

    int init_burst;             //记录初始的burst（用于waiting time计算）
    int arrive_time;            //到达时间
    int finish_time;            //完成时间
    int responce_time;          //在第一次运行后，记录响应时间
    int been_executed;          //记录是否为第一次运行
} Task;

#endif
