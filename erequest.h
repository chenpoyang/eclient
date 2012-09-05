#ifndef _EREQUEST_H_
#define _EREQUEST_H_

#include "common.h"

typedef enum {
    CMD_LOGIN,
} enum_ctrl_t;

typedef struct _e_login_t {
    char usr[E_MINLINE];
    char pwd[E_MINLINE];
} e_login_t;

typedef struct _ctrl_req_t {
    int cmd;
    void *req;
} ctrl_req_t;

typedef struct _ctrl_notify_t {
    int cmd;
    void *nty;
} ctrl_notify_t;

int e_login(const char *usr, const char *pwd);

#endif /* _EREQUEST_H_ */
