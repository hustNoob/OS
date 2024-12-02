#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int pipe_fd[2]; // 管道文件描述符

pid_t child1_pid, child2_pid;

// 父进程信号处理函数
void handle_sigint(int sig) {
    kill(child1_pid, SIGTERM);
    kill(child2_pid, SIGTERM);
}

// 子进程1信号处理函数
void handle_child1_sigterm(int sig) {
    printf("Child Process 1 is Killed by Parent!\n");
    exit(0);
}

// 子进程2信号处理函数
void handle_child2_sigterm(int sig) {
    printf("Child Process 2 is Killed by Parent!\n");
    exit(0);
}

int main() {
    // 创建管道
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 捕捉父进程的中断信号
    signal(SIGINT, handle_sigint);

    // 创建子进程1
    if ((child1_pid = fork()) == 0) {
        // 子进程1
        signal(SIGTERM, handle_child1_sigterm); // 捕捉终止信号
        close(pipe_fd[0]); // 关闭读端
        int count = 1;
        while (1) {
            char message[50];
            snprintf(message, sizeof(message), "I send you %d times.", count++);
            write(pipe_fd[1], message, strlen(message) + 1);
            sleep(1); // 每隔1秒发送一次
        }
    } else if (child1_pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // 创建子进程2
    if ((child2_pid = fork()) == 0) {
        // 子进程2
        signal(SIGTERM, handle_child2_sigterm); // 捕捉终止信号
        close(pipe_fd[1]); // 关闭写端
        char buffer[50];
        while (1) {
            read(pipe_fd[0], buffer, sizeof(buffer));
            printf("Child 2 received: %s\n", buffer);
        }
    } else if (child2_pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // 父进程
    close(pipe_fd[0]); // 父进程不需要读写管道，关闭两端
    close(pipe_fd[1]);
    
    // 等待子进程终止
    waitpid(child1_pid, NULL, 0);
    waitpid(child2_pid, NULL, 0);
    
    printf("Parent Process is Killed!\n");
    return 0;
}
