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
static list_t *_msg_que[MAX_AGENT_NUM];
static pthread_mutex_t _msg_que_mtx[MAX_AGENT_NUM];

static void * 
transfer_data(const int evt, void *base, size_t len, int from, int to);
static int check_agent_id(const int id);

/**
 * @brief   各层的守护线程
 *
 * @param   arg     :   线程的参数
 *
 * @return  NULL, 正常退出
 */
void *trigger_daemon(void *arg)
{
    int start, id, evt, flg_que_empty = 1;
    agent_t *agt = (agent_t*)arg;
    agent_status_t status;
    agent_msg_t *agt_msg = NULL;
    node_t *nx_agt_msg = NULL;
    void *data = NULL;
    size_t len;

    e_debug("trigger_daemon", "new trigger thread start!");
    
    start = 1;
    id = agt->id;
    while (start)
    {
        /* 1):从队列里取出一个请求, 通过信号灯同步, 并释放锁, 以便其他生产者添回请求
         * 2):agent忙碌处理优先级最的请求
         * note:可能阻塞于等待信号灯, 无法退出 */
        
        pthread_mutex_lock(&agt->mtx);

        if (agt->status == AGENT_WAIT)
        {
            /* 防止条件欺骗, 条件为:有请求需处理, 所以需对agt->status进行验证
               陷阱:此处阻赛, 退出问题与业务无关地调用send_signal()作退出 */
            pthread_cond_wait(&agt->cnd, &agt->mtx);
            /* must break, instead of changing the value of start,
               see 'stop_agent(...)' */
            if (AGENT_EXIT == agt->status)
            {
                break;
            }
        }
        
        /* 从队列头取一元素, 删除结点, 但结点的数据不能删除(释放资源),
           获取node_t *nod, nod->data, 再删除队列结点 */
        pthread_mutex_lock(&_msg_que_mtx[id]);

        /* agent的状态由队列里最后一个元素决定, 当最后一个元素出队列, 业务逻辑
           处理结束后, agent的状态仍然是AGENT_RUN, 但此时下一次队列里已没请求,
           所以导致'agt_msg = (agent_msg_t *)(nx_agt_msg->value);'产生
           core dumped, 此时添加多一个断逻辑即可 */
        if (list_length(_msg_que[id]) <= 0)
        {
            pthread_mutex_unlock(&_msg_que_mtx[id]);
            agt->status = AGENT_WAIT;
            pthread_mutex_unlock(&agt->mtx);
            e_debug("trigger_daemon", "agent[%d]'s msg queue is empty", id);
            continue;
        }
        
        nx_agt_msg = list_first(_msg_que[id]);
        /* get the data(agent_msg_t type) from list node */

        agt_msg = (agent_msg_t *)(nx_agt_msg->value); /* core dump, why? */
 
        if (agt_msg == NULL)
        {
            e_error("trigger_daemon", "agt_msg is NULL!");
        }
        
        data = agt_msg->data;
        evt = agt_msg->evt;
        len = agt_msg->len;
        status = agt_msg->status;
        /* pop an element */
        list_del_node(_msg_que[id], nx_agt_msg);
        flg_que_empty = list_length(_msg_que[id]) >= 0 ? 0 : 1;

        pthread_mutex_unlock(&_msg_que_mtx[id]);

        /* set agent's value */
        agt->status = status;
        agt->event = evt;
        agt->len = len;
        agt->userdata = data;
        
        /* 从队列中拿出一个无素并处理, 注意同步 */
        if (agt->status == AGENT_RUN)
        {
            /* 调用相应逻辑层的入口函数 */
            agt->handler(agt->event, agt->userdata, agt->len, agt->cmd);
            /* 如果消息队列为空, 则agent需等待, 否则不断取队列元素并处理 */
            agt->status = flg_que_empty ? AGENT_WAIT : AGENT_RUN;
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

/* 检查agent是否合法 */
static int check_agent_id(const int id)
{
    int legal = 0;

    legal = id >= 0 && id < MAX_AGENT_NUM;
    if (!legal)
    {
        e_error("check_agent", "illegal agent id");
    }

    return legal ? EME_OK : EME_ERR;
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
    agent_t agt, *agt_ptr = NULL;
    agent_msg_t *agt_msg = NULL;
    void *s_base = NULL;
    const int id = recver;
    int chk;

    chk = check_agent_id(id);
    if (chk == EME_ERR)
    {
        return chk;
    }
    
    agt.id = recver;
    nod = list_search_key(agent_lst, &agt);
    if (NULL == nod)
    {
        e_error("send_signal", "no daemon thread %d!", agt.id);
        return -1;
    }
    
    /* transmit data to 'recver agent' from 'from agent' */
    s_base = transfer_data(evt, base, len, from, recver);
    
    /* add msg to meg_que(protect list_t *msg_que, static int msg_id) */
    pthread_mutex_lock(&_msg_que_mtx[id]);
    
    /* add agent msg to its' queue, pending to be dealt */
    agt_msg = (agent_msg_t *)malloc(sizeof(agent_msg_t));
    agt_msg->id = id;
    agt_msg->evt = evt;
    agt_msg->len = len;
    /* for dealing this agent msg, the agent status must be set to AGENT_RUN */
    agt_msg->status = AGENT_RUN;
    agt_msg->data = s_base;
    list_add_node_tail(_msg_que[id], agt_msg);

    pthread_mutex_unlock(&_msg_que_mtx[id]);

    /* get agent_t(protect agent_t) */
    agt_ptr = (agent_t *)(nod->value);

    /* 工作线程已拿到锁或在等待处理信号
       1.工作线程没在等待条件变量, 向队列中添加元素, 不可能死锁
       2.工作线程已拿到锁, 向消息队列中添加请求, 需拿到队列锁:
       a):agent拿到agent锁和队列锁, 用户想拿队列锁, 用户需等待, 但用户等待时间极小,
       agent只有在运行过程中才会拿队列锁进行取元素, 且取元素时间复杂度为O(1) */

    /* the agent's queue has at least one agent msg, wake it up or do nothing */
    if (agt_ptr->status != AGENT_RUN)
    {
        pthread_mutex_lock(&agt_ptr->mtx);
        pthread_cond_signal(&agt_ptr->cnd);
        pthread_mutex_unlock(&agt_ptr->mtx);
    }

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
    int i;

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

    if (NULL == _msg_que[0]) /* 初始化所有事件队列 */
    {
        e_debug("init_agent", "all agent msg queue has been initialized!");
        for (i = 0; i < MAX_AGENT_NUM; ++i)
        {
            _msg_que[i] = list_create();
            if (_msg_que[i] == NULL)
            {
                e_error("init_agent",
                        "initialize agent msg queue[%d] failed!", i);
            }
            pthread_mutex_init(&_msg_que_mtx[i], NULL);
        }
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
    int i;
    
    it = list_get_iterator(agent_lst, LIST_START_HEAD);
    while ((cur = list_next(it)) != NULL)
    {
        agt = (agent_t *)cur->value;
        stop_agent(agt->id, agent_lst);
        pthread_join(agt->thrd, NULL);
    }
    list_release_iterator(it);
    it = NULL;

    list_release(agent_lst);
    for (i = 0; i < MAX_AGENT_NUM; ++i)
    {
        list_release(_msg_que[i]);
        _msg_que[i] = NULL;
        pthread_mutex_destroy(&_msg_que_mtx[i]);
    }
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
