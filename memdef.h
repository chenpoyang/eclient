#ifndef _MEMDEF_H_
#define _MEMDEF_H_

typedef enum {
    AGENT_WAIT, 
    AGENT_RUN, 
    AGENT_EXIT
} agent_status_t;

void *agent_dup(void *ptr);
void agent_free(void *ptr);
int agent_cmp(void *ptr, void *key);

void *dlg_dup(void *ptr);
void dlg_free(void *ptr);
int dlg_cmp(void *ptr, void *key);

#endif /* _MEMDEF_H_ */
