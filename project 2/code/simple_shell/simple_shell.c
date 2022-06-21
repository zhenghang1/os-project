/**
 * Simple shell interface program.
 *
 * Operating System Concepts - Tenth Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_LINE 80 /* 80 chars per line, per command */

int main(void)
{
    char *args[MAX_LINE / 2 + 1]; /* command line (of 80) has max of 40 arguments */
    int should_run = 1;
    char input_tmp[MAX_LINE / 2 + 1][20];
    int arg_count = 0;
    char *history_cmd[MAX_LINE / 2 + 1];
    int have_history = 0;
    int history_arg_count = 0;
    int wait_flag = 1;
    char tmp_str[MAX_LINE / 2 + 1][20];
    char *out_file;
    char *in_file;
    int redir_out_flag = 0;
    int redir_in_flag = 0;
    int pipe_flag = 0;
    int pipe_count = 0;
    char *pipe_arg[MAX_LINE / 2 + 1];
    int pipe_fd[2];

    while (should_run)
    {
        printf("osh>");
        fflush(stdout);
        arg_count = 0;
        redir_out_flag = 0;
        redir_in_flag = 0;
        wait_flag = 1;
        pipe_flag = 0;

        //初始化清空args
        for (int i = 0; i < MAX_LINE / 2 + 1; i++)
        {
            args[i] = NULL;
        }
        if (!have_history)
        {
            for (int i = 0; i < MAX_LINE / 2 + 1; i++)
            {
                history_cmd[i] = NULL;
            }
        }
        //读入指令
        char ch;
        while (scanf("%s", input_tmp[arg_count]))
        {
            args[arg_count] = input_tmp[arg_count];
            arg_count++;
            scanf("%c", &ch);
            if (ch == '\n')
                break;
        }

        //判断exit
        if (strcmp(args[0], "exit") == 0)
        {
            should_run = 0;
            continue;
        }
        //判断并执行 !!
        if (strcmp(args[0], "!!") == 0)
        {
            if (!have_history)
            {
                printf("No commands in history!\n");
                continue;
            }
            else
            {
                printf("osh>");
                fflush(stdout);
                for (int i = 0; i < history_arg_count; i++)
                {
                    strcpy(tmp_str[i], history_cmd[i]);
                    args[i] = tmp_str[i];
                    printf("%s", args[i]);
                    printf("%s", " ");
                }
                printf("%c", '\n');
                arg_count = history_arg_count;
            }
        }
        else
        {
            for (int i = 0; i < arg_count; i++)
            {
                strcpy(tmp_str[i], args[i]);
                history_cmd[i] = tmp_str[i];
            }
            history_arg_count = arg_count;
            have_history = 1;
        }
        //判断wait
        if (strcmp(args[arg_count - 1], "&") == 0)
        {
            args[arg_count - 1] = NULL;
            arg_count--;
            wait_flag = 0;
        }

        //判断重定向
        if (arg_count > 1 && strcmp(args[arg_count - 2], ">") == 0)
        {
            redir_out_flag = 1; // redir_out_flag=1表示输出重定向
            out_file = args[arg_count - 1];
            args[arg_count - 1] = NULL;
            args[arg_count - 2] = NULL;
            arg_count -= 2;
        }
        if (arg_count > 1 && strcmp(args[arg_count - 2], "<") == 0)
        {
            redir_in_flag = 1; // redir_in_flag=1表示输出重定向
            in_file = args[arg_count - 1];
            args[arg_count - 1] = NULL;
            args[arg_count - 2] = NULL;
            arg_count -= 2;
        }

        //判断pipe
        for (int i = 0; i < arg_count; i++)
        {
            if (strcmp(args[i], "|") == 0)
            {
                pipe_flag = 1;
                pipe_count = arg_count - i - 1;
                arg_count = i;
                for (int j = 0; j < MAX_LINE / 2 + 1; j++)
                {
                    pipe_arg[j] = NULL;
                }
                for (int j = 0; j < pipe_count; j++)
                {
                    pipe_arg[j] = args[i + 1];
                    args[i + 1] = NULL;
                }
                args[i] = NULL;
                break;
            }
        }

        //创建新进程
        pid_t pid;
        pid = fork();
        if (pid < 0)
        {
            printf("Fork Failed\n");
            return 1;
        }
        else if (pid == 0) // child
        {
            //重定向执行
            int fd;
            if (redir_out_flag == 1)
            { //输出重定向
                fd = open(out_file, O_CREAT | O_RDWR, S_IRWXU);
                if (fd == -1)
                {
                    printf("Create file Failed\n");
                    continue;
                }
                dup2(fd, STDOUT_FILENO);
            }
            if (redir_in_flag == 1)
            { //输入重定向
                fd = open(in_file, O_CREAT | O_RDONLY, S_IRUSR);
                dup2(fd, STDIN_FILENO);
            }

            //执行pipe
            if (pipe_flag == 1)
            {
                int pipe_result = pipe(pipe_fd);
                if (pipe_result == -1)
                {
                    printf("Pipe Create Failed!\n");
                }

                pid_t pipe_pid = fork();
                if (pipe_pid < 0)
                {
                    printf("Pipe Fork Failed\n");
                    return 1;
                }
                else if (pipe_pid == 0)
                {
                    close(pipe_fd[0]);
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    execvp(args[0], args);
                    //close(pipe_fd[1]);
                    exit(0);
                }
                else
                {
                    close(pipe_fd[1]);
                    dup2(pipe_fd[0], STDIN_FILENO);
                    execvp(pipe_arg[0], pipe_arg);
                    //close(pipe_fd[0]);
                    wait(NULL);
                }
            }
            else
            {
                execvp(args[0], args);
            }
            exit(0);
        }
        else // parent
        {
            if (wait_flag)
            {
                wait(NULL);
            }
        }
        /**
         * After reading user input, the steps are:
         * (1) fork a child process
         * (2) the child process will invoke execvp()
         * (3) if command includes &, parent and child will run concurrently
         */
    }

    return 0;
}
