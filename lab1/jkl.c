#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int pipefd[2];  // 管道描述符
pid_t pid_sender, pid_receiver;  // 发送和接收进程

void SIGUSR2_hander();
void SIGUSR1_hander();
void send();
void receive();
// 父进程信号处理函数
void parent_handler(int sig) {
    printf("\n父进程捕获到中断信号, 即将终止子进程...\n");
    kill(pid_sender, SIGUSR1);
    kill(pid_receiver, SIGUSR2);
}

int main() {
    // 创建管道
    if (pipe(pipefd) < 0) {
        perror("创建管道失败");
        exit(1);
    }

    // 设置父进程信号处理
    signal(SIGINT, parent_handler);

    // 创建发送进程 
    if ((pid_sender = fork()) < 0) {
        perror("创建发送进程失败");
        exit(1);
    } else send();

    // 创建接收进
    if ((pid_receiver = fork()) < 0) {
        perror("创建发送进程失败");
        exit(1);
    } else receive();

    
    // 等待子进程结束
    int status;
    waitpid(pid_sender, &status, 0);
    waitpid(pid_receiver, &status, 0);
    
    printf("Parent Process is Killed!\n");
    return 0;
}

void send() {
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1,SIGUSR1_hander);
    close(pipefd[0]);
    int count = 1;
    while (1) {
    char msg_buf[64];
    snprintf(msg_buf, sizeof(msg_buf), "I send you %d times.", count++);
    write(pipefd[1], msg_buf, strlen(msg_buf) + 1);  
    sleep(1);
    }
}

void receive() {
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1,SIGUSR2_hander);
    close(pipefd[1]);
    char recv_buf[64]; 
    while (1) {
    ssize_t n = read(pipefd[0], recv_buf, sizeof(recv_buf));
    if (n > 0) {
        printf("接收到消息: %s\n", recv_buf);
    }
    }
}

void SIGUSR1_hander(int sig){
    close(pipefd[0]);
    printf("Child Process 1 is Killed by Parent!\n");
    exit(0);
}

void SIGUSR2_hander(int sig){
    close(pipefd[1]);
    printf("Child Process 2 is Killed by Parent!\n");
    exit(0);
}