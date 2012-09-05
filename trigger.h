#ifndef _TRIGGER_H_
#define _TRIGGER_H_

#include <stddef.h>
#include <time.h>
#include <pthread.h>
#include "memdef.h"
#include "list.h"

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

void *trigger_daemon(void *arg);
void init_agent(int id, const char *name, agent_handler);
void stop_agent(int id, list_t *agt_lst);
void stop_all_agent();
int send_signal(int evt, void *base, size_t len, int from, int recver);
void *app_req_mem(size_t len);
void thread_wait(unsigned int sec);

#endif /* _TRIGGER_H_ */
