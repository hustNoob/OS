#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <pthread.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int all;
int num = 0;
int semid;


void P(int semid, int index) {
    struct sembuf sem;
    sem.sem_num = index;
    sem.sem_op = -1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
    return;
}

void V(int semid, int index) {
    struct sembuf sem;
    sem.sem_num = index;
    sem.sem_op = 1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
    return;
}

void *subp(void* id) {
    long pthread = (long) id;
    while (num < all) {
        P(semid, 0);
        if (num < all) {
            num++;
            printf("线程%ld售票一张，剩余票数：%d\n", pthread, all-num);
            V(semid, 0);
            if (num == all) {
                printf("已售票数：%d\n", num);
                break;
            }
        } else {
            V(semid, 0);
            break;
        }
        usleep(1000);
    }
    return NULL;
}

int main() {
    // 获取输入
    int n = 0;
    printf("飞机票总数：");
    scanf("%d", &all);
    printf("创建售票线程个数：");
    scanf("%d", &n);

    // 创建信号灯
    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        printf("信号灯集创建失败！\n");
        exit(1);
    }

    // 信号灯赋初值
    union semun sem;
    sem.val = 1;
    if (semctl(semid, 0, SETVAL, sem) == -1) {
        printf("信号灯0初始化失败！\n");
        exit(1);
    }

    // 创建多个线程
    pthread_t p[n];
    for (int i = 0; i < n; i++) {
        if (pthread_create(&p[i], NULL, subp, (void*)(i + 1))) {
            printf("pthread%d创建失败！\n", i + 1);
            exit(1);
        }
    }

    // 等待线程运行结束
    for (int k = 0; k < n; k++) {
        if (pthread_join(p[k], NULL) == -1) {
            printf("pthread%d结束失败！\n", k + 1);
            exit(1);
        }
    }

    // 删除信号灯
    if (semctl(semid, 0, IPC_RMID, NULL) == -1) {
        printf("信号灯集删除失败！\n");
        exit(1);
    }

    return 0;
}