/* ctrlagent层守望护线程, 用于处理所有ctrlagent层队列里的请求, 包括业务逻辑处理,
 * 入口为agent中的handler(const int , void *, size_t, from, to) */
#ifndef _TRIGGER_H_
#define _TRIGGER_H_

#include <stddef.h>
#include <time.h>
#include <pthread.h>
#include "memdef.h"
#include "list.h"

#define MAX_AGENT_NUM 32
#define AGENT_THREAD_DEFAULT_WAIT 1

typedef void (*agent_handler)(int, void *, size_t, int);

typedef struct _agent_t {
    int id;
    pthread_t thrd;
    char *name;
    pthread_mutex_t mtx;
    pthread_cond_t cnd;
    agent_status_t status;
    agent_handler handler;
    int event;
    void *userdata;
    size_t len;
    int cmd;
} agent_t;

/* agent msg */
typedef struct _agent_msg_t {
    int id; /* send to "recver's queue" */
    int evt;
    size_t len;
    agent_status_t status;
    void *data; /* just transmit, no need to free */
} agent_msg_t;

void *trigger_daemon(void *arg);
void init_agent(int id, const char *name, agent_handler);
void stop_agent(int id, list_t *agt_lst);
void stop_all_agent();
int send_signal(int evt, void *base, size_t len, int from, int recver);
void *app_req_mem(size_t len);
void thread_wait(unsigned int sec);

#endif /* _TRIGGER_H_ */
