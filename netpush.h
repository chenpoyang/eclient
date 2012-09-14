/* 服务端主动推送事件 */
#ifndef _NETPUSH_H_
#define _NETPUSH_H_

#include <time.h>

#include "common.h"

typedef struct _msg_arrived_t 
{
    char msg[E_MAXLINE];
    char to[E_MINLINE];
    char from[E_MINLINE];
} msg_arrived_t;

#endif /* NETPUSH_H_ */
