#ifndef _LIST_H_
#define _LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LIST_START_HEAD 0
#define LIST_START_TAIL 1
#define MATCH 0 /* for list_search_key() */

typedef struct _node_t {
    struct _node_t *next;
    struct _node_t *prev;
    void *value;
} node_t;

typedef struct _list_iter_t {
    node_t *next;
    int dir;
} list_iter_t;

/**
 * @brief   列表, 对于不同的自定义类型, 需提供拷贝,释放, 比较结点的方式实现
 */
typedef struct _list_t {
    node_t *head;
    node_t *tail;
    void *(*ldup)(void *ptr);   /* 拷贝结点的方式 */
    void (*lfree)(void *ptr);   /* 释放列表结点内存的方式 */
    int (*lcmp)(void *ptr, void *key);  /* 比较结点值的方式 */
    unsigned int len;
} list_t;

/* 宏实现的函数 */
#define list_length(L)              ((L)->len)
#define list_first(L)               ((L)->head)
#define list_last(L)                ((L)->tail)
#define list_prev_node(L)           ((L)->prev)
#define list_next_node(L)           ((L)->next)
#define list_set_ldup_method(L,M)   ((L)->ldup = (M))
#define list_set_lfree_method(L,M)  ((L)->lfree = (M))
#define list_set_lcmp_method(L,M)   ((L)->lcmp = (M))
#define list_get_ldup_method(L)     ((L)->ldup)
#define list_get_lfree_method(L)    ((L)->lfree)
#define list_get_lcmp_method(L)     ((L)->lcmp)

/* 函数原型 */
list_t *list_create();
void list_release(list_t *lst);
list_t *list_add_node_head(list_t *lst, void *value);
list_t *list_add_node_tail(list_t *lst, void *value);
list_t *list_insert_node(list_t *lst, node_t *old_node, void *value, int after);
list_t *list_del_node(list_t *lst, node_t *nod);
list_iter_t *list_get_iterator(list_t *lst, int dir);
node_t *list_next(list_iter_t *iter);
void list_release_iterator(list_iter_t *iter);
list_t *list_dup(list_t *src_lst);
node_t *list_search_key(list_t *lst, void *key);
node_t *list_index(list_t *lst, int index);
void list_rewind(list_t *lst, list_iter_t *iter);
void list_rewind_tail(list_t *lst, list_iter_t *iter);

#ifdef __cplusplus
}
#endif

#endif /* _LIST_H_ */
