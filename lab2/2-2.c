#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <pthread.h>
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int a;
int semid;

void P(int semid, int index)
{
    struct sembuf sem;
    sem.sem_num = index;
    sem.sem_op = -1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
    return;
}
void V(int semid, int index)
{
    struct sembuf sem;
    sem.sem_num = index;
    sem.sem_op = 1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
    return;
}

void *subp1(){
    a = 0;
    for(int i = 1; i <= 100; i++) {
        P(semid, 0);
        a = a + i;
        V(semid, 1);
    }
    return NULL;
}

void *subp2(){
    for(int i = 1; i <= 100; i++) {
        P(semid, 1);
        if(a % 2 == 0)
            printf("打印线程1：a=%d\n", a);
        V(semid, 2);
    }
    return NULL;
}

void *subp3(){
    for(int i = 1; i <= 100; i++) {
        P(semid, 2);
        if(a % 2 == 1)
            printf("打印线程2：a=%d\n", a);
        V(semid, 0);
    }
    return NULL;
}

int main() {
    // 创建信号灯
    semid = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);
    if (semid == -1) {
        printf("信号灯集创建失败！\n");
        exit(1);
    }

    // 信号灯赋初值
    union semun sem[3];
    sem[0].val = 1;
    sem[1].val = 0;
    sem[2].val = 0;
    for(int i = 0; i < 3; i++) {
        if(semctl(semid, i, SETVAL, sem[i]) == -1) {
            printf("信号灯%d初始化失败！\n", i);
            exit(1);
        }
    }

    // 创建线程subp1、subp2、subp3
    pthread_t p1, p2, p3;
    if(pthread_create(&p1, NULL, subp1, NULL)) {
        printf("subp1创建失败！\n");
        exit(1);
    }
    if(pthread_create(&p2, NULL, subp2, NULL)) {
        printf("subp2创建失败！\n");
        exit(1);
    }
    if(pthread_create(&p3, NULL, subp3, NULL)) {
        printf("subp3创建失败！\n");
        exit(1);
    }

    // 等待三个线程运行结束
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);

    // 删除信号灯
    semctl(semid, 0, IPC_RMID, NULL);
    
    return 0;
}