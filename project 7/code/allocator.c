#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define TRUE 1
#define MAX 0x3FFFFFFF // 1GB

typedef struct MEM_NODE
{
    int type;   // 1 for allocated memory, 0 for non_allocated memory
    char *name; // process name (if allocated(type=1))
    int begin;  // starting address
    int end;    // ending address
    struct MEM_NODE *next;
} mem_node;

char args[3][100];
char cmd[100];
mem_node *head;

void init(int argc, char *argv[]);
void clean(void);
void request(void);
void release(void);
void compact(void);
// read in command
int read_cmd(void);
// search for an available hole for memory allcation request
mem_node *search_available(int size, char strategy);
// allocate memory in the given hole with the given process name and memory size
void allocate(mem_node *target, char *proc_name, int size);
// merge all the adjecent holes
void merge(void);
// print the current state
void print_state(void);

int main(int argc, char *argv[])
{
    // initialization
    init(argc, argv);
    while (TRUE)
    {
        int mode = read_cmd();
        scanf("%*[^\n]%*c");

        switch (mode)
        {
        // RQ
        case 0:
        {
            request();
            break;
        }
        // RL
        case 1:
        {
            release();
            break;
        }
        // C
        case 2:
        {
            compact();
            break;
        }
        // STAT
        case 3:
        {
            print_state();
            break;
        }

        // exit
        case 4:
        {
            printf("Exit successfully!\n\n");
            clean();
            return 0;
        }
        // error
        default:
        {
            printf("Error:Invalid command!\n\n");
            break;
        }
        }
    }
    return 0;
}

void init(int argc, char *argv[])
{
    if (argc <= 1 || argc >= 3)
    {
        printf("Error: invalid arguments!\n\n");
        exit(1);
    }
    int mem_size = atoi(argv[1]);
    if (mem_size > MAX || mem_size < 0)
    {
        printf("Error: invalid memory size!\n\n");
        exit(1);
    }

    head = (mem_node *)malloc(sizeof(mem_node *) * 10);
    if (!head)
    {
        printf("Error: memory initialization failed!\n\n");
        exit(1);
    }

    head->begin = 0;
    head->end = mem_size - 1;
    head->type = 0;
    head->next = NULL;
}

void request(void)
{
    int alloc_size = atoi(args[1]);
    if (alloc_size > MAX || alloc_size < 0)
    {
        printf("Error: Invalid requested memory size!\n\n");
        return;
    }

    mem_node *target = search_available(alloc_size, args[2][0]);
    if (target == NULL)
        return;

    allocate(target, args[0], alloc_size);
    printf("Request for %s has been satisfied!\n\n", args[0]);
}

void release(void)
{
    char *name = args[0];
    mem_node *tmp = head;
    int flag = 0; // flag=0 means there's no such a block of memory allocated for this process
    while (tmp)
    {
        if (tmp->type == 0)
        {
            tmp = tmp->next;
            continue;
        }
        else if (!strcmp(tmp->name, name))
        {
            tmp->type = 0;
            flag = 1;
        }
        tmp = tmp->next;
    }
    if (flag)
    {
        merge();
        printf("Memory allocated for %s has been released!\n\n", args[0]);
    }
    else
    {
        printf("Error: Release failed, no memory has been allocated for process named %s!\n\n", args[0]);
    }
}

void compact(void)
{
    mem_node *unused = head, *unused_pre;
    mem_node *allocated, *allocated_pre;
    unused_pre = (mem_node *)malloc(sizeof(mem_node *)); // virtual head node
    unused_pre->next = head;
    unused_pre->end = -1;
    mem_node *to_be_free = unused_pre;

    while (TRUE)
    {
        while (unused && unused->type == 1)
        {
            unused_pre = unused;
            unused = unused->next;
        }
        if (unused == NULL) // all memory been allocated, no hole exists
            break;
        ;

        allocated = unused->next;
        allocated_pre = unused;
        while (allocated && allocated->type != 1)
        {
            allocated_pre = allocated;
            allocated = allocated->next;
        }
        if (allocated == NULL)
            break;

        // swap the unused node and the allocated node
        unused_pre->next = allocated;
        allocated_pre->next = unused;
        mem_node *tmp = allocated->next;
        allocated->next = unused->next;
        unused->next = tmp;

        if (unused == head) // in case that head is swapped, update the head pointer
            head = unused_pre->next;

        tmp = unused_pre->next;
        int current_begin = unused_pre->end + 1;
        while (tmp) // update the begin and end of the affected memory nodes
        {
            int size = tmp->end - tmp->begin + 1;
            tmp->begin = current_begin;
            tmp->end = tmp->begin + size - 1;
            current_begin = tmp->end + 1;
            tmp = tmp->next;
        }

        unused = unused_pre->next;
    }

    merge();
    free(to_be_free);
    printf("Compact unused holes successfully!\n\n");
}
/*
mode 0 represents "RQ"
mode 1 represents "RL"
mode 2 represents "C"
mode 3 represents "STAT"
mode 4 represents "X"
mode -1 represents invalid command
*/
int read_cmd(void)
{
    printf("allocator>  ");
    scanf("%s", cmd);

    if (!strcmp(cmd, "RQ"))
    {
        for (int i = 0; i < 3; i++)
            scanf(" %s", args[i]);
        return 0;
    }
    else if (!strcmp(cmd, "RL"))
    {
        scanf(" %s", args[0]);
        return 1;
    }
    else if (!strcmp(cmd, "C"))
    {
        return 2;
    }
    else if (!strcmp(cmd, "STAT"))
    {
        return 3;
    }
    else if (!strcmp(cmd, "X"))
    {
        return 4;
    }
    else
        return -1;
}

mem_node *search_available(int size, char strategy)
{
    mem_node *ptr = head;
    switch (strategy)
    {
    case 'F':
    {
        while (ptr)
        {
            if (ptr->type == 1)
            {
                ptr = ptr->next;
                continue; // already allocated
            }
            int current_size = ptr->end - ptr->begin + 1;
            if (current_size >= size)
                return ptr;
            else
                ptr = ptr->next;
        }
        if (ptr == NULL)
        {
            printf("Error: There's insufficient memory to be allocated, request rejected!\n\n");
            return NULL;
        }
        break;
    }
    case 'B':
    {
        mem_node *current_best_target = NULL;
        int current_best_size = INT_MAX;
        while (ptr)
        {
            if (ptr->type == 1)
            {
                ptr = ptr->next;
                continue; // already allocated
            }
            int current_size = ptr->end - ptr->begin + 1;
            if (current_size >= size)
            {
                if (current_best_size > current_size)
                {
                    current_best_target = ptr;
                    current_best_size = current_size;
                }
            }
            ptr = ptr->next;
        }
        if (current_best_target == NULL)
        {
            printf("Error: There's insufficient memory to be allocated, request rejected!\n\n");
            return NULL;
        }
        return current_best_target;
        break;
    }
    case 'W':
    {
        mem_node *current_worst_target = NULL;
        int current_worst_size = 0;
        while (ptr)
        {
            if (ptr->type == 1)
            {
                ptr = ptr->next;
                continue; // already allocated
            }
            int current_size = ptr->end - ptr->begin + 1;
            if (current_size >= size)
            {
                if (current_worst_size < current_size)
                {
                    current_worst_target = ptr;
                    current_worst_size = current_size;
                }
            }
            ptr = ptr->next;
        }
        if (current_worst_target == NULL)
        {
            printf("Error: There's insufficient memory to be allocated, request rejected!\n\n");
            return NULL;
        }
        return current_worst_target;
        break;
    }
    default:
    {
        printf("Error: invalid allocation strategy argument!\n\n");
        break;
    }
    }
    return NULL;
}

void allocate(mem_node *target, char *proc_name, int size)
{
    mem_node *new_node = (mem_node *)malloc(sizeof(mem_node *));
    target->name = (char *)malloc(sizeof(char) * (strlen(proc_name) + 1));

    // new_node
    new_node->next = target->next;
    target->next = new_node;
    new_node->begin = target->begin + size;
    new_node->end = target->end;
    new_node->type = 0;

    // target
    target->end = target->begin + size - 1;
    target->type = 1;
    strcpy(target->name, proc_name);
}

void merge(void)
{
    if (!head->next)
        return;
    mem_node *p1 = head, *p2 = head->next;

    while (p2)
    {
        if (p1->type == 0 && p2->type == 0)
        {
            p1->next = p2->next;
            p1->end = p2->end;
            free(p2);
            p2 = p1->next;
        }
        else
        {
            p1 = p1->next;
            p2 = p2->next;
        }
    }
}

void print_state(void)
{
    printf("The current state:\n");
    mem_node *tmp = head;
    while (tmp)
    {
        printf("Addresses [ %d : %d ]\t", tmp->begin, tmp->end);
        if (tmp->type)
        {
            printf("Process %s\n", tmp->name);
        }
        else
        {
            printf("Unused\n");
        }
        tmp = tmp->next;
    }
    printf("\n");
}

void clean(void)
{
    while (head)
    {
        mem_node *tmp = head;
        head = head->next;
        free(tmp);
    }
}