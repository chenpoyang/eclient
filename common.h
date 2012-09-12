#ifndef _COMMON_H_
#define _COMMON_H_

#include <stddef.h>
#include <assert.h>
#include "elog.h"

/* 逻辑线程的id */
#define CTRL_AGENT_ID   1
#define NET_AGENT_ID    2

/* ctrlagent层的事件类型 */
#define CTRL_REQ_EVT    1
#define CTRL_NOTIFY_EVT 2

/* netagent层的事件类型 */
#define NET_REQ_EVT     3
#define NET_NOTIFY_EVT  4

#define E_MAXLINE 2048
#define E_MINLINE 128
#define MAX_DLG_MEMBER 10
#define MAX_DLG_BUF 512
#define DLG_BUF_NOT_USE 0
#define DLG_BUF_USED 1

typedef enum _eme_error_t 
{
    EME_OK = 0,
    EME_ERR = -1,
    EME_TIMEOUT = 100,
} eme_error_t;

typedef void (*eclient_listener)(void *, size_t);

#endif /* _COMMON_H_ */
