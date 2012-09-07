#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "common.h"
#include "trigger.h"
#include "elog.h"
#include "list.h"

/* agent_t 列表 */
static list_t *agent_lst = NULL;
static pthread_mutex_t _lst_mtx;
static pthread_attr_t _thrd_attr;

/* 消息队列 */
static list_t *msg_que = NULL;
static pthread_mutex_t _msg_que_mtx;

static void * 
transfer_data(const int evt, void *base, size_t len, int from, int to);

/**
 * @brief   各层的守护线程
 *
 * @param   arg     :   线程的参数
 *
 * @return  NULL, 正常退出
 */
void *trigger_daemon(void *arg)
{
    int start;
    agent_t *agt = (agent_t*)arg;

    e_debug("trigger_daemon", "new trigger thread start!");
    
    start = 1;
    while (start)
    {
        /* 1):从队列里取出一个请求, 通过信号灯同步, 并释放锁, 以便其他生产者添回请求
         * 2):agent忙碌处理优先级最的请求
         * note:可能阻塞于等待信号灯, 无法退出 */
        
        pthread_mutex_lock(&agt->mtx);

        if (agt->status == AGENT_WAIT)
        {
            pthread_cond_wait(&agt->cnd, &agt->mtx);
            if (agt->status == AGENT_RUN)
            {
                /* 调用相应逻辑层的入口函数 */
                agt->handler(agt->event, agt->userdata, agt->len, agt->cmd);
                agt->status = AGENT_WAIT;
            }
            else if (agt->status == AGENT_EXIT)
            {
                start = 0;
                pthread_cond_destroy(&agt->cnd);
                pthread_mutex_destroy(&agt->mtx);
            }
        }

        pthread_mutex_unlock(&agt->mtx);
    }
    
    e_debug("trigger_daemon", "a trigger thread exit!");

    pthread_exit(NULL);
}

/* 各层之间数据的传递 */
static void * 
transfer_data(const int evt, void *base, size_t len, int from, int to)
{
    void *s_base = NULL;

    /* 新的请求或回调到达, 交换数据 */
    s_base = malloc(len);
    /* 必加, 否则数据无法传递 */
    memcpy(s_base, base, len);
    e_debug("store_data","trigger received data from[%d], "
            "will transmit to[%d], size = [%d], event[%d]",
            from, to, len, evt);

    return s_base;
}

/**
 * @brief   evt事件来临, 向线程recver发送消息
 *
 * @param   evt     :   事件
 * @param   base    :   数据的基址
 * @param   len     :   数据的长度
 * @param   recver  :   接收者
 *
 * @return  0, 成功发送;反之, 发送失败
 */
int send_signal(int evt, void *base, size_t len, int from, int recver)
{
    node_t *nod = NULL;
    agent_t agt, *agt_ptr;
    void *s_base = NULL;

    agt.id = recver;
    nod = list_search_key(agent_lst, &agt);
    if (NULL == nod)
    {
        e_error("send_signal", "no daemon thread %d!", agt.id);
        return -1;
    }

    /* add msg to meg_que(protect list_t *msg_que, static int msg_id) */
    pthread_mutex_lock(&_msg_que_mtx);

    pthread_mutex_unlock(&_msg_que_mtx);

    /* 接收信息, 转移到另处一层 */
    s_base = transfer_data(evt, base, len, from, recver);

    /* run agent_t(protect agent_t) */
    agt_ptr = (agent_t *)(nod->value);

    pthread_mutex_lock(&agt_ptr->mtx);

    agt_ptr->len = len;
    agt_ptr->event = evt;
    agt_ptr->userdata = s_base;
    agt_ptr->status = AGENT_RUN;
    pthread_cond_signal(&agt_ptr->cnd);

    pthread_mutex_unlock(&agt_ptr->mtx);

    return EME_OK;
}

void stop_agent(int id, list_t *agt_lst)
{
    agent_t *cur = NULL;
    node_t *nod = NULL;
    list_iter_t *it = NULL;

    it = list_get_iterator(agt_lst, LIST_START_HEAD);
    while ((nod = list_next(it)) != NULL)
    {
        cur = (agent_t*)nod->value;
        if (cur->id == id)
        {
            /* 发送停止信号并清理资源 */
            pthread_mutex_lock(&cur->mtx);
            cur->status = AGENT_EXIT;
            pthread_cond_signal(&cur->cnd);
            pthread_mutex_unlock(&cur->mtx);
            break;
        }
    }
    list_release_iterator(it);
}

/**
 * @brief   初始化一个线程, 定时调用handler
 *
 * @param   id      :   逻辑上的线程id, 一般在commondef.h定义
 * @param   name    :   线程的名字, 可供输出log
 * @param   handler :   线程定时要处理的逻辑, 分别处于各层的逻辑入口
 */
void init_agent(int id, const char *name, agent_handler handler)
{
    agent_t *agt;

    if (agent_lst == NULL)
    {
        agent_lst = list_create();
        agent_lst->ldup = agent_dup;
        agent_lst->lfree = agent_free;
        agent_lst->lcmp = agent_cmp;
        pthread_mutex_init(&_lst_mtx, NULL);
        pthread_attr_init(&_thrd_attr);
        pthread_attr_setdetachstate(&_thrd_attr, PTHREAD_CREATE_JOINABLE);
    }

    if (msg_que == NULL)
    {
        msg_que = list_create();
        pthread_mutex_init(&_msg_que_mtx, NULL);
    }

    agt = (agent_t *)calloc(1, sizeof(agent_t));
    if (NULL == agt)
    {
        e_error("init_agent", "calloc error on init_one_agent!\n");
        return;
    }

    agt->id = id;
    agt->name = (char *)calloc(1, strlen(name) + 1);
    strcpy(agt->name, name);
    agt->status = AGENT_WAIT;
    agt->handler = handler;
    agt->userdata = NULL;
    agt->len = 0;
    pthread_mutex_init(&agt->mtx, NULL);
    pthread_cond_init(&agt->cnd, NULL);

    /* 启动线程 */
    pthread_create(&agt->thrd, &_thrd_attr, trigger_daemon, (void *)agt);
    
    list_add_node_tail(agent_lst, agt);
}

void stop_all_agent()
{
    list_iter_t *it = NULL;
    node_t *cur = NULL;
    agent_t *agt = NULL;
    
    it = list_get_iterator(agent_lst, LIST_START_HEAD);
    while ((cur = list_next(it)) != NULL)
    {
        agt = (agent_t *)cur->value;
        stop_agent(agt->id, agent_lst);
        pthread_join(agt->thrd, NULL);
    }
    list_release_iterator(it);

    list_release(agent_lst);
    list_release(msg_que);
}

/**
 * @brief   当前线程等待sec秒
 *
 * @param   sec     :   等待的秒数
 */
void thread_wait(unsigned int sec)
{
    pthread_mutex_t mtx;
    pthread_cond_t cnd;
    struct timespec ts;

    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&cnd, NULL);
    
    pthread_mutex_lock(&mtx);
    ts.tv_sec = time(NULL) + sec;
    ts.tv_nsec = 0;
    pthread_cond_timedwait(&cnd, &mtx, &ts);
    pthread_mutex_unlock(&mtx);
}

void *app_req_mem(size_t len)
{
    void *base = NULL;

    base = malloc(len);

    if (NULL == base)
    {
        e_error("app_req_mem", "app_req_mem error!");
    }

    return base;
}
