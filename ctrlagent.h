#ifndef _CTRLAGENT_H_
#define _CTRLAGENT_H_

#include <stddef.h>
#include "erequest.h"
#include "netreq.h"

typedef enum {
    DLG_STEP_INIT,
    DLG_STEP_RUN, 
    DLG_STEP_FINISH
} dlg_step_t;

typedef enum {
    DLG_RES_IDLE, 
    DLG_RES_TIMEOUT, 
    DLG_RES_ACK
} dlg_res_t;

/* 会话缓存标识 */
typedef struct _dlg_t {
    int id;
    int flg; /* use or not, eg: 1, 0  */
    int opr; /* g_cmd_info[opr] */
    time_t beg;
    ctrl_req_t *req;
    net_req_t *net_req;
    dlg_step_t step;
    dlg_res_t result;
    void *ack;
    void *data[MAX_DLG_MEMBER];
} dlg_t;

/* 会话生命周期标识 */
extern dlg_t dlg[MAX_DLG_BUF];
typedef void (*cmd_handler)(size_t dlg_idx);

typedef struct _cmd_info_t {
    int cmd;
    char name[32];
    cmd_handler handler;
} cmd_info_t;

/* entrance */
void ctrlagent(int, void *, size_t, int);
void ctrl_elogin(size_t idx);
int validate_dlg(const int idx);

#endif /* _CTRLAGENT_H_ */
