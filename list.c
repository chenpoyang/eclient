#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "list.h"

list_t *list_create()
{
    list_t *lst;

    if ((lst = malloc(sizeof(*lst))) == NULL)
    {
        return NULL;
    }
    lst->head = lst->tail = NULL;
    lst->ldup = NULL;  /* 复制一个列表的方式 */
    lst->lfree = NULL; /* 列表释放节点内存的方式 */
    lst->lcmp = NULL;  /* 列表比较结点的方式 */
    lst->len = 0;

    return lst;
}

/**
 * @brief   释放整个列的内存, 并根不同的结点类型释放内
 *
 * @param   lst
 */
void list_release(list_t *lst)
{
    size_t len;
    node_t *cur, *next;

    cur = lst->head;
    len = lst->len;
    while (len-- > 0)
    {
        next = cur->next;
        /* 列表根据不同的结点, 以不同的方式释内存 */
        if (lst->lfree != NULL)
        {
            lst->lfree(cur->value);
        }
        free(cur);
        cur = next;
    }
    free(lst);
}

/**
 * @brief   将结点插到列表头
 *
 * @param   lst     :   列表
 * @param   value   :   结点的值, 将新建一个结点并插入列表中
 *
 * @return  
 */
list_t *list_add_node_head(list_t *lst, void *value)
{
    node_t *nde;

    nde = (node_t*)calloc(1, sizeof(node_t));
    if (NULL == nde)
    {
        return NULL;
    }
    nde->value = value;
    if (lst->len == 0)
    {
        lst->head = lst->tail = nde;
        nde->prev = nde->next = NULL;
    }
    else
    {
        nde->prev = NULL;
        nde->next = lst->head;
        lst->head->prev = nde;
        lst->head = nde;
    }
    ++lst->len;

    return lst;
}

/**
 * @brief   将结点插到列表尾
 *
 * @param   lst     :   列表
 * @param   value   :   结点的值, 将新建一个结点并插入列表中
 *
 * @return  
 */
list_t *list_add_node_tail(list_t *lst, void *value)
{
    node_t *nde;

    nde = (node_t*)calloc(1, sizeof(node_t));
    if (NULL == nde)
    {
        return NULL;
    }
    nde->value = value;
    if (0 == lst->len)
    {
        lst->head = lst->tail = nde;
        nde->prev = nde->next = NULL;
    }
    else
    {
        nde->next = NULL;
        nde->prev = lst->tail;
        lst->tail->next = nde;
        lst->tail = nde;
    }
    ++lst->len;

    return lst;
}


/**
 * @brief   将一值为value的结点插入到列表中old_node结点的前或后
 *
 * @param   lst         :   新结点的列表
 * @param   old_node    :   原来的结点
 * @param   value       :   新结点所带的值
 * @param   after       :   当after为真, 插到结点old_node后面, 后之, 前面
 *
 * @return  
 */
list_t *list_insert_node(list_t *lst, node_t *old_node, void *value, int after)
{
    node_t *nde;

    nde = (node_t*)calloc(1, sizeof(node_t));
    if (NULL == nde)
    {
        return NULL;
    }
    nde->value = value;
    if (after)  /* 将新结点nde插入到结点old_node的后面 */
    {
        nde->next = old_node->next;
        nde->prev = old_node;
        if (lst->tail == old_node)
        {
            lst->tail = nde;
        }
    }
    else
    {
        nde->next = old_node;
        nde->prev = old_node->prev;
        if (lst->head == old_node)
        {
            lst->head = nde;
        }
    }

    if (nde->prev != NULL)
    {
        nde->prev->next = nde;
    }
    if (nde->next != NULL)
    {
        nde->next->prev = nde;
    }
    ++lst->len;

    return lst;
}


/**
 * @brief   从列表中删除结点nod, 并根据不同的结点类型释放内存
 *
 * @param   lst     :   目标列表
 * @param   nod     :   要删除的结点
 *
 * @return  操作完后的列表
 */
list_t *list_del_node(list_t *lst, node_t *nod)
{
    if (NULL == nod->prev)  /* 头结点 */
    {
        lst->head = nod->next;
    }
    else
    {
        nod->prev->next = nod->next;
    }

    if (NULL == nod->next)  /* 尾结点 */
    {
        lst->tail = nod->prev;
    }
    else
    {
        nod->next->prev = nod->prev;
    }

    if (lst->lfree != NULL)
    {
        lst->lfree(nod->value);
    }
    free(nod);
    --lst->len;

    return lst;
}


/**
 * @brief   获取列表的迭带器
 *
 * @param   lst     :   目标列表
 * @param   dir     :   迭带方向

 * @return  迭带器
 */
list_iter_t *list_get_iterator(list_t *lst, int dir)
{
    list_iter_t *it;

    it = (list_iter_t*)calloc(1, sizeof(list_iter_t));
    if (NULL == it)
    {
        printf("list_get_iterator():calloc error!\n");
        return NULL;
    }
    if (dir == LIST_START_HEAD)
    {
        it->next = lst->head;
    }
    else
    {
        it->next = lst->tail;
    }
    it->dir = dir;

    return it;
}

/**
 * @brief   使用迭带器返加列表的下一个元素
 *
 * @param   iter    :   迭带器
 *
 * @return  迭带的结点
 */
node_t *list_next(list_iter_t *iter)
{
    node_t *cur = NULL;

    cur = iter->next;
    if (cur != NULL)
    {
        if (iter->dir == LIST_START_HEAD)
        {
            iter->next = cur->next;
        }
        else
        {
            iter->next = cur->prev;
        }
    }
    return cur;
}

void list_release_iterator(list_iter_t *iter)
{
    free(iter);
    iter = NULL;
}

/**
 * @brief   拷贝原列表src_lst
 *
 * @param   src_lst     :   源列表
 *
 * @return  返回拷贝后的列表
 */
list_t *list_dup(list_t *src_lst)
{
    list_t *cp_lst = NULL;
    list_iter_t *it = NULL;
    node_t *nod = NULL;
    void *value = NULL;

    if ((cp_lst = list_create()) == NULL)
    {
        return NULL;
    }

    cp_lst->ldup = src_lst->ldup;
    cp_lst->lfree = src_lst->lfree;
    cp_lst->lcmp = src_lst->lcmp;

    it = list_get_iterator(src_lst, LIST_START_HEAD);
    while ((nod = list_next(it)) != NULL)
    {
        if (cp_lst->ldup != NULL)   /* 不同结点复制方式不同 */
        {
            value = cp_lst->ldup(nod->value);
            if (NULL == value)
            {
                list_release(cp_lst);
                list_release_iterator(it);

                return NULL;
            }
        }
        else
        {
            value = nod->value;
        }

        if (list_add_node_head(cp_lst, value) == NULL)
        {
            list_release(cp_lst);
            list_release_iterator(it);

            return NULL;
        }
    }

    list_release_iterator(it);

    return cp_lst;
}

node_t *list_search_key(list_t *lst, void *key)
{
    list_iter_t *it = NULL;
    node_t *nod = NULL;

    it = list_get_iterator(lst, LIST_START_HEAD);
    while ((nod = list_next(it)) != NULL)
    {
        if (lst->lcmp != NULL)  /* 根据不同的结点类型采用不同的比较方式 */
        {
            if (lst->lcmp(nod->value, key) == MATCH)
            {
                list_release_iterator(it);
                return nod;
            }
        }
        else
        {
            if (nod->value == key)
            {
                list_release_iterator(it);
                return nod;
            }
        }
    }
    list_release_iterator(it);

    return NULL;

}

node_t *list_index(list_t *lst, int index)
{
    node_t *nde = NULL;
    
    if (index < 0)  /* 从尾开始找, 最后一个结点下标为 -1, 倒数第二为-2 */
    {
        index = (-index) - 1;
        nde = lst->tail;
        while (--index >= 0 && nde)
        {
            nde = nde->prev;
        }
    }
    else
    {
        nde = lst->head;
        while (--index >= 0 && nde)
        {
            nde = nde->next;
        }
    }
    
    return nde;
}

void list_rewind(list_t *lst, list_iter_t *iter)
{
    iter->next = lst->head;
    iter->dir = LIST_START_HEAD;
}

void list_rewind_tail(list_t *lst, list_iter_t *iter)
{
    iter->next = lst->tail;
    iter->dir = LIST_START_TAIL;
}
