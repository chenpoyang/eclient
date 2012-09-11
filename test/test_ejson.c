#include <stdio.h>
#include <string.h>
#include "../jsonpro.h"

void print_obj(json_value *val);

int main(int argc, const char *argv[])
{
    char str[1024];
    json_value *val = NULL;

    strcpy(str, "{\"action\":\"login\",\"user\":\"weimade\",\"passwd\":\"123456\"}");
    //while (gets(str) != NULL)
    {
        val = json_parse(str);
        print_obj(val);
    }
    json_value_free(val);

    return 0;
}

void print_obj(json_value *val)
{
    json_value *nx_val;
    if (val->type == json_object)
    {
        puts("it is an json_ojbect type!");
        if (val->u.object.values->name != NULL)
        {
            printf("obj's name = [%s]\r\n", val->u.object.values->name);
        }
        nx_val = val->u.object.values->value;
        if (nx_val->type == json_string)
        {
            printf("a string type, str[%s], obj->obj->value\r\n", nx_val->u.string.ptr);
        }
        else
        {
            printf("not a string type, obj->obj->value\r\n");
        }
    }
    else
    {
        puts("it isn't an json_ojbect type!\r\n");
    }

    if (val->_reserved.next_alloc != NULL)
    {
        print_obj(val->_reserved.next_alloc);
    }
    else
    {
        printf("no parent!\r\n");
    }
}
