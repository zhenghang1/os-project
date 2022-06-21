#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TRUE 1

int num_of_customers;
int num_of_resources;

int *available;
int **maximum;
int **allocation;
int **need;
int *args;
int *work, *finish;

void init(int argc, char *argv[]);
void clean(void);
void print_state(void);
int read_cmd(void);
int safe_check(void);
int request_resources(int customer_num, int request[]);
int release_resources(int customer_num, int release[]);
void increase_resources(int customer_num, int resources[]);
int decrease_resources(int customer_num, int resources[]);

int main(int argc, char *argv[])
{
    // initialization
    init(argc, argv);
    if (!safe_check())
    {
        printf("Error: initial state is unsafe!\n");
        print_state();
        return 1;
    }
    printf("Banker's Algorithm\n(input command \"q\" to quit the program)\n");

    while (TRUE)
    {
        int mode = read_cmd();
        scanf("%*[^\n]%*c");
        int ret;

        switch (mode)
        {
        // RQ
        case 0:
        {
            ret = request_resources(args[0], args + 1);
            if (!ret)
            {
                printf("The resources have been successfully allocated!\n");
            }
            break;
        }
        // RL
        case 1:
        {
            ret = release_resources(args[0], args + 1);
            if (ret)
            {
                printf("Error: The released resources exceed that has been allocated for customer %d!\n", args[0]);
            }
            else
            {
                printf("The resources have been successfully released!\n");
            }
            break;
        }
        //*
        case 2:
        {
            print_state();
            break;
        }
        // quit
        case 3:
        {
            printf("quit successfully!\n");
            clean();
            return 0;
            break;
        }
        // error
        default:
        {
            printf("Error:invalid command!\n");
            break;
        }
        }
    }
    return 0;
}

void init(int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("Error: invalid arguments!\n");
        exit(1);
    }
    num_of_resources = argc - 1;

    // available initialization
    available = (int *)malloc(sizeof(int) * num_of_resources);
    for (int i = 1; i < argc; ++i)
    {
        available[i - 1] = atoi(argv[i]);
    }
    // fetch num_of_customers
    FILE *fp = fopen("init.txt", "r");
    char *tmp = (char *)(malloc(sizeof(char) * num_of_resources * 10));
    while (!feof(fp))
    {
        char *ret = fgets(tmp, num_of_resources * 10, fp);
        if (ret)
            num_of_customers++;
    }
    free(tmp);
    fclose(fp);

    maximum = (int **)malloc(sizeof(int *) * num_of_customers);
    allocation = (int **)malloc(sizeof(int *) * num_of_customers);
    need = (int **)malloc(sizeof(int *) * num_of_customers);
    for (int i = 0; i < num_of_customers; i++)
    {
        maximum[i] = (int *)malloc(sizeof(int) * num_of_resources);
        allocation[i] = (int *)malloc(sizeof(int) * num_of_resources);
        need[i] = (int *)malloc(sizeof(int) * num_of_resources);
    }

    // read data into maximum
    fp = fopen("init.txt", "r");
    for (int i = 0; i < num_of_customers; i++)
    {
        int j = 0;
        fscanf(fp, "%d", &(maximum[i][j]));
        for (j = 1; j < num_of_resources; j++)
        {
            fscanf(fp, ",%d", &(maximum[i][j]));
        }
    }
    fclose(fp);

    // initialization of allocation and need
    for (int i = 0; i < num_of_customers; i++)
    {
        for (int j = 0; j < num_of_resources; j++)
        {
            allocation[i][j] = 0;
            need[i][j] = maximum[i][j];
        }
    }
    // initialization of work and finish (prepared for safe_check)
    work = (int *)malloc(sizeof(int) * num_of_resources);
    finish = (int *)malloc(sizeof(int) * num_of_customers);

    args = (int *)malloc(sizeof(int) * (num_of_resources + 1));
}

/*
mode 0 represents "RQ"
mode 1 represents "RL"
mode 2 represents "*"
mode 3 represents "quit"
mode -1 represents invalid command
*/
int read_cmd(void)
{
    printf("cmd >>  ");
    char cmd[2];
    scanf("%c", &cmd[0]);
    if (cmd[0] == '\n')
        scanf("%c", &cmd[0]);

    if (cmd[0] == '*')
    {
        return 2;
    }
    else if (cmd[0] == 'R')
    {
        scanf("%c", &cmd[1]);
        if (cmd[1] != 'Q' && cmd[1] != 'L')
            return -1;
    }
    else if (cmd[0] == 'q')
        return 3;
    else
        return -1;

    // RQ & RL
    for (int i = 0; i < num_of_resources + 1; i++)
        scanf(" %d", &args[i]);
    if (cmd[1] == 'Q')
        return 0;
    else
        return 1;
}

void print_state(void)
{
    int i, j;
    printf("The current state is shown as below:\n\n");
    printf("Number of customers:\t%d\n",num_of_customers);
    printf("Number of resources:\t%d\n\n",num_of_resources);
    printf("Available:\n\t\t[ ");
    for (i = 0; i < num_of_resources - 1; i++)
    {
        printf("%d, ", available[i]);
    }
    printf("%d ]\n", available[i]);
    printf("\nMaximum:\n");
    for (i = 0; i < num_of_customers; i++)
    {
        printf("customer %d :\t[ ", i);
        for (j = 0; j < num_of_resources - 1; j++)
        {
            printf("%d, ", maximum[i][j]);
        }
        printf("%d ]\n", maximum[i][j]);
    }
    printf("\nAllocation:\n");
    for (i = 0; i < num_of_customers; i++)
    {
        printf("customer %d :\t[ ", i);
        for (j = 0; j < num_of_resources - 1; j++)
        {
            printf("%d, ", allocation[i][j]);
        }
        printf("%d ]\n", allocation[i][j]);
    }
    printf("\nNeed:\n");
    for (i = 0; i < num_of_customers; i++)
    {
        printf("customer %d :\t[ ", i);
        for (j = 0; j < num_of_resources - 1; j++)
        {
            printf("%d, ", need[i][j]);
        }
        printf("%d ]\n", need[i][j]);
    }
    printf("\n");
}

// return 1 represents safe, 0 represents unsafe
int safe_check(void)
{
    // step 1
    for (int i = 0; i < num_of_resources; i++)
        work[i] = available[i];
    for (int i = 0; i < num_of_customers; i++)
        finish[i] = 0;

    for (int i = 0; i < num_of_customers; i++)
    {
        // step 2
        if (finish[i])
            continue;
        int flag = 0;
        for (int j = 0; j < num_of_resources; j++)
        {
            if (need[i][j] > work[j])
                flag = 1;
        }
        if (flag)
            continue;

        // step 3
        for (int j = 0; j < num_of_resources; j++)
        {
            work[j] += allocation[i][j];
            finish[i] = 1;
            i = 0;
        }
    }
    // step 4
    int unsafe_flag = 0;
    for (int i = 0; i < num_of_customers; i++)
    {
        if (finish[i] == 0)
            unsafe_flag = 1;
    }
    return !unsafe_flag;
}

int request_resources(int customer_num, int request[])
{
    int ret = decrease_resources(customer_num, request);
    if (ret == 0)
    {
        if (safe_check())
            return 0;
        else
            printf("Request failed! The request from customer %d will lead to an unsafe state.\n", customer_num);
    }
    else if (ret == 1)
    {
        printf("Error: The requested resources exceed customer %d's needed resources!\n", customer_num);
    }
    else if (ret == 2)
    {
        printf("Error: The requested resources exceed the currently available ones!\n");
    }

    increase_resources(customer_num, request);
    return -1;
}

void increase_resources(int customer_num, int resources[])
{
    for (int i = 0; i < num_of_resources; i++)
    {
        allocation[customer_num][i] -= resources[i];
        need[customer_num][i] += resources[i];
        available[i] += resources[i];
    }
}
/*
return 0 represents successful
return 1 represents allocation[i][j]>maximum[i][j]
return 2 represents request[i]>available[i]
*/
int decrease_resources(int customer_num, int resources[])
{
    for (int i = 0; i < num_of_resources; i++)
    {
        allocation[customer_num][i] += resources[i];
        need[customer_num][i] -= resources[i];
        available[i] -= resources[i];
    }
    int out_maximum = 0;
    int out_available = 0;
    for (int i = 0; i < num_of_resources; i++)
    {
        if (need[customer_num][i] < 0)
            out_maximum = 1;
        if (available[i] < 0)
            out_available = 1;
    }
    if (out_maximum)
        return 1;
    if (out_available)
        return 2;
    return 0;
}

int release_resources(int customer_num, int release[])
{
    // check
    for (int i = 0; i < num_of_resources; ++i)
        if (release[i] > allocation[customer_num][i])
        {
            return 1;
        }

    increase_resources(customer_num, release);
    return 0;
}

void clean(void)
{
    for (int i = 0; i < num_of_customers; i++)
    {
        free(maximum[i]);
        free(allocation[i]);
        free(need[i]);
    }
    free(maximum);
    free(allocation);
    free(need);
    free(available);
    free(args);
    free(work);
    free(finish);
}