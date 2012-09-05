#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define ID_ONE      1
#define ID_TWO      2
#define THREAD_EXIT 3
#define THREAD_RUN  1
#define THREAD_WAIT 0

pthread_attr_t thrd_attr;
pthread_mutex_t mtx_one;
pthread_mutex_t mtx_two;
pthread_cond_t cnd_one;
pthread_cond_t cnd_two;
int one_run, two_run;
int tmp_cnt;
int main_started;

void message_handler(const char *str);

void init();
void uninit();
void * agent_one(void *arg);
void * agent_two(void *arg);
void agent_send(int receiver, int cmd);

/* 信号处理 */
void sigint_handler(int sig);

int main(int argc, const char *argv[])
{
    int recver, cmd;
    struct sigaction sa;
    pthread_t thrd_one, thrd_two;

    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) < 0)
    {
        perror("sigaction!");
        return -1;
    }

    init();
    pthread_create(&thrd_one, &thrd_attr, agent_one, NULL);
    pthread_create(&thrd_two, &thrd_attr, agent_two, NULL);

    main_started = 1;
    while (main_started)
    {
        printf("Enter Receiver(1, 2) and Cmd(1[run], 3[exit]), ([0, 0] to exit):");
        scanf("%d %d", &recver, &cmd);
        if (recver + cmd > 0)
        {
            agent_send(recver, cmd);
        }
        else
        {
            main_started = 0;
            agent_send(ID_ONE, THREAD_EXIT);
            agent_send(ID_TWO, THREAD_EXIT);
        }
    }

    uninit();
    return 0;
}

void sigint_handler(int sig)
{
    agent_send(ID_ONE, THREAD_EXIT);
    agent_send(ID_TWO, THREAD_EXIT);
    main_started = 0;
}

void uninit()
{
    pthread_attr_destroy(&thrd_attr);
    pthread_mutex_destroy(&mtx_one);
    pthread_mutex_destroy(&mtx_two);
    pthread_cond_destroy(&cnd_one);
    pthread_cond_destroy(&cnd_two);
}

void init()
{
    pthread_mutex_init(&mtx_one, NULL);
    pthread_mutex_init(&mtx_two, NULL);
    pthread_cond_init(&cnd_one, NULL);
    pthread_cond_init(&cnd_two, NULL);
    pthread_attr_init(&thrd_attr);
    pthread_attr_setdetachstate(&thrd_attr, PTHREAD_CREATE_JOINABLE);
}

void agent_send(int receiver, int cmd)
{
    switch (receiver)
    {
        case ID_ONE:
            pthread_mutex_lock(&mtx_one);
            one_run = cmd;
            pthread_cond_signal(&cnd_one);
            pthread_mutex_unlock(&mtx_one);
            break;
        case ID_TWO:
            pthread_mutex_lock(&mtx_two);
            two_run = cmd;
            pthread_cond_signal(&cnd_two);
            pthread_mutex_unlock(&mtx_two);
            break;
    }
}

void * agent_two(void *arg)
{
    int started = 1;
    
    while (started)
    {
        pthread_mutex_lock(&mtx_two);

        if (!two_run)
        {
            pthread_cond_wait(&cnd_two, &mtx_two);
            if(two_run == THREAD_RUN)
            {
                message_handler("agent_two's turn!");
                two_run = THREAD_WAIT;
            }
            else if (two_run == THREAD_EXIT)
            {
                started = 0;
            }
            else
            {
                two_run = THREAD_WAIT;
            }
        }

        pthread_mutex_unlock(&mtx_two);
    }
    write(0, "agent_two exit!\n", 16);

    pthread_exit(NULL);
}

void * agent_one(void *arg)
{
    int started = 1;

    while (started)
    {
        pthread_mutex_lock(&mtx_one);

        /* 等待触发agent_one的条件 */
        if (!one_run)
        {
            pthread_cond_wait(&cnd_one, &mtx_one);
            if (one_run == THREAD_RUN)
            {
                message_handler("agent_one's turn!");
                one_run = THREAD_WAIT;
            }
            else if (one_run == THREAD_EXIT)
            {
                started = 0;
            }
            else
            {
                one_run = THREAD_WAIT;
            }
        }

        pthread_mutex_unlock(&mtx_one);
    }
    write(0, "agent_one exit!\n", 16);

    pthread_exit(NULL);
}

void message_handler(const char *str)
{
    puts(str);
}
