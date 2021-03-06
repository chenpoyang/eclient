/* UI层请求的入口, 如e_login_t和e_register_t都用ctrl_req_t包装,
 * 统一发送到ctrlagent层处理 */
#ifndef _EREQUEST_H_
#define _EREQUEST_H_

#include "common.h"

typedef enum {
    CMD_LOGIN,
    CMD_REGISTER,
    CMD_SEND_MSG
} enum_ctrl_t;

typedef struct _e_login_t {
    char usr[E_MINLINE];
    char pwd[E_MINLINE];
} e_login_t;

typedef struct _e_register_t {
    char usr[E_MINLINE];
    char pwd[E_MINLINE];
    char repwd[E_MINLINE];
} e_register_t;

typedef struct _e_send_msg_t {
    int type;
    char msg[E_MAXLINE];
    char to[E_MINLINE];
} e_send_msg_t;

typedef struct _ctrl_req_t {
    int cmd;
    void *req;
} ctrl_req_t;

typedef struct _ctrl_notify_t {
    int cmd;
    void *nty;
} ctrl_notify_t;

int e_login(const char *usr, const char *pwd);
int e_register(const char *usr, const char *pwd, const char *repwd);
int e_snd_msg(int type, const char *msg, const char *to);

#endif /* _EREQUEST_H_ */
