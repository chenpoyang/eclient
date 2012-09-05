#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memdef.h"
#include "trigger.h"

void *agent_dup(void *ptr)
{
    agent_t *agt = (agent_t *)ptr;
    agent_t *cp_agt = NULL;
    int len;

    cp_agt = (agent_t*)malloc(sizeof(agent_t));
    cp_agt->id = agt->id;

    len = strlen(agt->name);
    cp_agt->name = (char *)calloc(1, len + 1);
    strncpy(cp_agt->name, agt->name, len);
    cp_agt->name[len] = '\0';

    cp_agt->status = AGENT_WAIT;
    cp_agt->handler = NULL;
    cp_agt->event = agt->event;
    cp_agt->userdata = malloc(agt->len);
    cp_agt->len = agt->len;
    memcpy(cp_agt->userdata, agt->userdata, agt->len);
    cp_agt->cmd = agt->cmd;

    return cp_agt;
}

void agent_free(void *ptr)
{
    agent_t *agt = (agent_t *)ptr;

    free(agt->name);
    free(agt);
}

int agent_cmp(void *ptr, void *key)
{
    agent_t *ag_a, *ag_b;

    ag_a = (agent_t*)ptr;
    ag_b = (agent_t*)key;

    if (ag_a->id != ag_b->id)
    {
        return ag_a->id > ag_b->id ? 1 : -1;
    }
    else
    {
        return 0;
    }
}

void *dlg_dup(void *ptr)
{
    /* TODO */

    return ptr;
}

void dlg_free(void *ptr)
{
    /* TODO */
}

int dlg_cmp(void *ptr, void *key)
{
    /* TODO */
    return 0;
}
