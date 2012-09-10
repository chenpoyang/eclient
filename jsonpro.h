/* json数据形式的自定义协议 */
#ifndef _JSONPRO_H_
#define _JSONPRO_H_

typedef struct _json_obj_t json_obj_t;
typedef struct _json_array_t json_array_t;
typedef struct _json_element_t json_element_t;

struct _json_element_t 
{
    int flag; /* determinate combination */
    
    union /* json name */
    {
        json_obj_t *name_obj;
        json_array_t *name_array;
        char *name_str;
        int name_bool;
    }jname;

    union/* json value */
    {
        json_obj_t *value_obj;
        json_array_t *value_array;
        char *value_str;
        int value_bool;
    }jvalue;
};

struct _json_obj_t {
    json_element_t elem;
    json_element_t *next;
    char *jobj_str;
};

struct _json_array_t {
    json_element_t *ary;
    char *jarray_str;
    int len;
};

#endif
